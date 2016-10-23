/*
 * Copyright (c) 2015-2015 Texas Instruments Incorporated
 *
 * Copyright (c) 1985, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* @(#)exp.c	8.1 (Berkeley) 6/4/93 */


/* EXP(X)
 * RETURN THE EXPONENTIAL OF X
 * DOUBLE PRECISION (IEEE 53 bits, VAX D FORMAT 56 BITS)
 * CODED IN C BY K.C. NG, 1/19/85;
 * REVISED BY K.C. NG on 2/6/85, 2/15/85, 3/7/85, 3/24/85, 4/16/85, 6/14/86.
 *
 * Required system supported functions:
 *	scalb(x,n)
 *	copysign(x,y)
 *	finite(x)
 *
 * Method:
 *	1. Argument Reduction: given the input x, find r and integer k such
 *	   that
 *	                   x = k*ln2 + r,  |r| <= 0.5*ln2 .
 *	   r will be represented as r := z+c for better accuracy.
 *
 *	2. Compute exp(r) by
 *
 *		exp(r) = 1 + r + r*R1/(2-R1),
 *	   where
 *		R1 = x - x^2*(p1+x^2*(p2+x^2*(p3+x^2*(p4+p5*x^2)))).
 *
 *	3. exp(x) = 2^k * exp(r) .
 *
 * Special cases:
 *	exp(INF) is INF, exp(NaN) is NaN;
 *	exp(-INF)=  0;
 *	for finite argument, only exp(0)=1 is exact.
 *
 * Accuracy:
 *	exp(x) returns the exponential of x nearly rounded. In a test run
 *	with 1,156,000 random arguments on a VAX, the maximum observed
 *	error was 0.869 ulps (units in the last place).
 */

#include <float.h>
#define __BSD_VISIBLE 1
#include "mathimpl.h"

#if LDBL_MANT_DIG >= 53

_DATA_ACCESS static const __ieee64_t p1 = C(0x1.555555555553ep-3);
_DATA_ACCESS static const __ieee64_t p2 = C(-0x1.6c16c16bebd93p-9);
_DATA_ACCESS static const __ieee64_t p3 = C(0x1.1566aaf25de2cp-14);
_DATA_ACCESS static const __ieee64_t p4 = C(-0x1.bbd41c5d26bf1p-20);
_DATA_ACCESS static const __ieee64_t p5 = C(0x1.6376972bea4d0p-25);
_DATA_ACCESS static const __ieee64_t ln2hi = C(0x1.62e42fee00000p-1);
_DATA_ACCESS static const __ieee64_t ln2lo = C(0x1.a39ef35793c76p-33);
_DATA_ACCESS static const __ieee64_t lnhuge = C(0x1.6602b15b7ecf2p9);
_DATA_ACCESS static const __ieee64_t lntiny = C(-0x1.77af8ebeae354p9);
_DATA_ACCESS static const __ieee64_t invln2 = C(0x1.71547652b82fep0);

/* returns exp(r = x + c) for |c| < |x| with no overlap.  */

__ieee64_t __exp__D(__ieee64_t x, __ieee64_t c)
{
	__ieee64_t  z,hi,lo;
	int k;

	if (x != x)	/* x is NaN */
		return(x);
	if ( x <= lnhuge ) {
		if ( x >= lntiny ) {

		    /* argument reduction : x --> x - k*ln2 */
			z = invln2*x;
			k = z + F(copysign)(.5, x);

		    /* express (x+c)-k*ln2 as hi-lo and let x=hi-lo rounded */

			hi=(x-k*ln2hi);			/* Exact. */
			x= hi - (lo = k*ln2lo-c);
		    /* return 2^k*[1+x+x*c/(2+c)]  */
			z=x*x;
			c= x - z*(p1+z*(p2+z*(p3+z*(p4+z*p5))));
			c = (x*c)/(2.0-c);

			return F(scalbn)(1.+(hi-(lo - c)), k);
		}
		/* end of x > lntiny */

		else
		     /* exp(-big#) underflows to zero */
                     if(isfinite(x)) return(F(scalbn)(1.0,-5000));

		     /* exp(-INF) is zero */
		     else return(0.0);
	}
	/* end of x < lnhuge */

	else
	/* exp(INF) is INF, exp(+big#) overflows to INF */
	    return( isfinite(x) ?  F(scalbn)(1.0,5000)  : x);
}

#endif /* LDBL_MANT_DIG >= 53 */
