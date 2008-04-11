/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bessel.cxx,v $
 * $Revision: 1.6 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "bessel.hxx"
#include "analysishelper.hxx"

#include <rtl/math.hxx>

using ::com::sun::star::lang::IllegalArgumentException;

namespace sca {
namespace analysis {

// ============================================================================

const double f_PI       = 3.1415926535897932385;
const double f_2_PI     = 2.0 * f_PI;
const double f_PI_DIV_2 = f_PI / 2.0;
const double f_PI_DIV_4 = f_PI / 4.0;
const double f_2_DIV_PI = 2.0 / f_PI;

const double THRESHOLD  = 30.0;     // Threshold for usage of approximation formula.
const double MAXEPSILON = 1e-10;    // Maximum epsilon for end of iteration.
const sal_Int32 MAXITER = 100;      // Maximum number of iterations.


// ============================================================================
// BESSEL J
// ============================================================================

/*  The BESSEL function, first kind, unmodified:

                     inf                                  (-1)^k (x/2)^(n+2k)
        J_n(x)  =  SUM   TERM(n,k)   with   TERM(n,k) := ---------------------
                     k=0                                       k! (n+k)!

    Approximation for the BESSEL function, first kind, unmodified, for great x:

        J_n(x)  ~  sqrt( 2 / (PI x) ) cos( x - n PI/2 - PI/4 )  for  x>=0.
 */

// ----------------------------------------------------------------------------

double BesselJ( double x, sal_Int32 n ) throw( IllegalArgumentException )
{
    if( n < 0 )
        throw IllegalArgumentException();

    double fResult = 0.0;
    if( fabs( x ) <= THRESHOLD )
    {
        /*  Start the iteration without TERM(n,0), which is set here.

                TERM(n,0) = (x/2)^n / n!
         */
        double fTerm = pow( x / 2.0, (double)n ) / Fak( n );
        sal_Int32 nK = 1;   // Start the iteration with k=1.
        fResult = fTerm;    // Start result with TERM(n,0).

        const double fSqrX = x * x / -4.0;

        do
        {
            /*  Calculation of TERM(n,k) from TERM(n,k-1):

                                   (-1)^k (x/2)^(n+2k)
                    TERM(n,k)  =  ---------------------
                                        k! (n+k)!

                                   (-1)(-1)^(k-1) (x/2)^2 (x/2)^(n+2(k-1))
                               =  -----------------------------------------
                                           k (k-1)! (n+k) (n+k-1)!

                                   -(x/2)^2     (-1)^(k-1) (x/2)^(n+2(k-1))
                               =  ---------- * -----------------------------
                                    k(n+k)            (k-1)! (n+k-1)!

                                   -(x^2/4)
                               =  ---------- TERM(n,k-1)
                                    k(n+k)
             */
            fTerm *= fSqrX;    // defined above as -(x^2/4)
            fTerm /= (nK * (nK + n));
            fResult += fTerm;
        }
        while( (fabs( fTerm ) > MAXEPSILON) && (++nK < MAXITER) );
    }
    else
    {
        /*  Approximation for the BESSEL function, first kind, unmodified:

                J_n(x)  ~  sqrt( 2 / (PI x) ) cos( x - n PI/2 - PI/4 )  for  x>=0.

            The BESSEL function J_n with n IN {0,2,4,...} is axially symmetric at
            x=0, means J_n(x) = J_n(-x). Therefore the approximation for x<0 is:

                J_n(x)  =  J_n(|x|)  for  x<0  and  n IN {0,2,4,...}.

            The BESSEL function J_n with n IN {1,3,5,...} is point-symmetric at
            x=0, means J_n(x) = -J_n(-x). Therefore the approximation for x<0 is:

                J_n(x)  =  -J_n(|x|)  for  x<0  and  n IN {1,3,5,...}.
         */
        double fXAbs = fabs( x );
        fResult = sqrt( f_2_DIV_PI / fXAbs ) * cos( fXAbs - n * f_PI_DIV_2 - f_PI_DIV_4 );
        if( (n & 1) && (x < 0.0) )
            fResult = -fResult;
    }
    return fResult;
}


// ============================================================================
// BESSEL I
// ============================================================================

/*  The BESSEL function, first kind, modified:

                     inf                                  (x/2)^(n+2k)
        I_n(x)  =  SUM   TERM(n,k)   with   TERM(n,k) := --------------
                     k=0                                   k! (n+k)!

    Approximation for the BESSEL function, first kind, modified, for great x:

        I_n(x)  ~  e^x / sqrt( 2 PI x )  for  x>=0.
 */

// ----------------------------------------------------------------------------

double BesselI( double x, sal_Int32 n ) throw( IllegalArgumentException )
{
    if( n < 0 )
        throw IllegalArgumentException();

    double fResult = 0.0;
    if( fabs( x ) <= THRESHOLD )
    {
        /*  Start the iteration without TERM(n,0), which is set here.

                TERM(n,0) = (x/2)^n / n!
         */
        double fTerm = pow( x / 2.0, (double)n ) / Fak( n );
        sal_Int32 nK = 1;   // Start the iteration with k=1.
        fResult = fTerm;    // Start result with TERM(n,0).

        const double fSqrX = x * x / 4.0;

        do
        {
            /*  Calculation of TERM(n,k) from TERM(n,k-1):

                                   (x/2)^(n+2k)
                    TERM(n,k)  =  --------------
                                    k! (n+k)!

                                   (x/2)^2 (x/2)^(n+2(k-1))
                               =  --------------------------
                                   k (k-1)! (n+k) (n+k-1)!

                                   (x/2)^2     (x/2)^(n+2(k-1))
                               =  --------- * ------------------
                                   k(n+k)      (k-1)! (n+k-1)!

                                   x^2/4
                               =  -------- TERM(n,k-1)
                                   k(n+k)
             */
            fTerm *= fSqrX;    // defined above as x^2/4
            fTerm /= (nK * (nK + n));
            fResult += fTerm;
        }
        while( (fabs( fTerm ) > MAXEPSILON) && (++nK < MAXITER) );
    }
    else
    {
        /*  Approximation for the BESSEL function, first kind, modified:

                I_n(x)  ~  e^x / sqrt( 2 PI x )  for  x>=0.

            The BESSEL function I_n with n IN {0,2,4,...} is axially symmetric at
            x=0, means I_n(x) = I_n(-x). Therefore the approximation for x<0 is:

                I_n(x)  =  I_n(|x|)  for  x<0  and  n IN {0,2,4,...}.

            The BESSEL function I_n with n IN {1,3,5,...} is point-symmetric at
            x=0, means I_n(x) = -I_n(-x). Therefore the approximation for x<0 is:

                I_n(x)  =  -I_n(|x|)  for  x<0  and  n IN {1,3,5,...}.
         */
        double fXAbs = fabs( x );
        fResult = exp( fXAbs ) / sqrt( f_2_PI * fXAbs );
        if( (n & 1) && (x < 0.0) )
            fResult = -fResult;
    }
    return fResult;
}


// ============================================================================

double Besselk0( double fNum ) throw( IllegalArgumentException )
{
    double  fRet;

    if( fNum <= 2.0 )
    {
        double  fNum2 = fNum * 0.5;
        double  y = fNum2 * fNum2;

        fRet = -log( fNum2 ) * BesselI( fNum, 0 ) +
                ( -0.57721566 + y * ( 0.42278420 + y * ( 0.23069756 + y * ( 0.3488590e-1 +
                    y * ( 0.262698e-2 + y * ( 0.10750e-3 + y * 0.74e-5 ) ) ) ) ) );
    }
    else
    {
        double  y = 2.0 / fNum;

        fRet = exp( -fNum ) / sqrt( fNum ) * ( 1.25331414 + y * ( -0.7832358e-1 +
                y * ( 0.2189568e-1 + y * ( -0.1062446e-1 + y * ( 0.587872e-2 +
                y * ( -0.251540e-2 + y * 0.53208e-3 ) ) ) ) ) );
    }

    return fRet;
}


double Besselk1( double fNum ) throw( IllegalArgumentException )
{
    double  fRet;

    if( fNum <= 2.0 )
    {
        double  fNum2 = fNum * 0.5;
        double  y = fNum2 * fNum2;

        fRet = log( fNum2 ) * BesselI( fNum, 1 ) +
                ( 1.0 + y * ( 0.15443144 + y * ( -0.67278579 + y * ( -0.18156897 + y * ( -0.1919402e-1 +
                    y * ( -0.110404e-2 + y * ( -0.4686e-4 ) ) ) ) ) ) )
                / fNum;
    }
    else
    {
        double  y = 2.0 / fNum;

        fRet = exp( -fNum ) / sqrt( fNum ) * ( 1.25331414 + y * ( 0.23498619 +
                y * ( -0.3655620e-1 + y * ( 0.1504268e-1 + y * ( -0.780353e-2 +
                y * ( 0.325614e-2 + y * ( -0.68245e-3 ) ) ) ) ) ) );
    }

    return fRet;
}


double BesselK( double fNum, sal_Int32 nOrder ) throw( IllegalArgumentException )
{
    switch( nOrder )
    {
        case 0:     return Besselk0( fNum );
        case 1:     return Besselk1( fNum );
        default:
        {
            double      fBkp;

            double      fTox = 2.0 / fNum;
            double      fBkm = Besselk0( fNum );
            double      fBk = Besselk1( fNum );

            for( sal_Int32 n = 1 ; n < nOrder ; n++ )
            {
                fBkp = fBkm + double( n ) * fTox * fBk;
                fBkm = fBk;
                fBk = fBkp;
            }

            return fBk;
        }
    }
}


double Bessely0( double fNum ) throw( IllegalArgumentException )
{
    double  fRet;

    if( fNum < 8.0 )
    {
        double  y = fNum * fNum;

        double  f1 = -2957821389.0 + y * ( 7062834065.0 + y * ( -512359803.6 +
                    y * ( 10879881.29 + y * ( -86327.92757 + y * 228.4622733 ) ) ) );

        double  f2 = 40076544269.0 + y * ( 745249964.8 + y * ( 7189466.438 +
                    y * ( 47447.26470 + y * ( 226.1030244 + y ) ) ) );

        fRet = f1 / f2 + 0.636619772 * BesselJ( fNum, 0 ) * log( fNum );
    }
    else
    {
        double  z = 8.0 / fNum;
        double  y = z * z;
        double  xx = fNum - 0.785398164;

        double  f1 = 1.0 + y * ( -0.1098628627e-2 + y * ( 0.2734510407e-4 +
                        y * ( -0.2073370639e-5 + y * 0.2093887211e-6 ) ) );

        double  f2 = -0.1562499995e-1 + y * ( 0.1430488765e-3 +
                        y * ( -0.6911147651e-5 + y * ( 0.7621095161e-6 +
                        y * ( -0.934945152e-7 ) ) ) );

        fRet = sqrt( 0.636619772 / fNum ) * ( sin( xx ) * f1 + z * cos( xx ) * f2 );
    }

    return fRet;
}


double Bessely1( double fNum ) throw( IllegalArgumentException )
{
    double  fRet;

    if( fNum < 8.0 )
    {
        double  y = fNum * fNum;

        double  f1 = fNum * ( -0.4900604943e13 + y * ( 0.1275274390e13 +
                        y * ( -0.5153438139e11 + y * ( 0.7349264551e9 +
                        y * ( -0.4237922726e7 + y * 0.8511937935e4 ) ) ) ) );

        double  f2 = 0.2499580570e14 + y * ( 0.4244419664e12 +
                        y * ( 0.3733650367e10 + y * ( 0.2245904002e8 +
                        y * ( 0.1020426050e6 + y * ( 0.3549632885e3 + y ) ) ) ) );

        fRet = f1 / f2 + 0.636619772 * ( BesselJ( fNum, 1 ) * log( fNum ) - 1.0 / fNum );
    }
    else
    {
#if 0
        // #i12430# don't know the intention of this piece of code...
        double  z = 8.0 / fNum;
        double  y = z * z;
        double  xx = fNum - 2.356194491;

        double  f1 = 1.0 + y * ( 0.183105e-2 + y * ( -0.3516396496e-4 +
                        y * ( 0.2457520174e-5 + y * ( -0.240337019e6 ) ) ) );

        double  f2 = 0.04687499995 + y * ( -0.2002690873e-3 +
                        y * ( 0.8449199096e-5 + y * ( -0.88228987e-6 +
                        y * 0.105787412e-6 ) ) );

        fRet = sqrt( 0.636619772 / fNum ) * ( sin( xx ) * f1 + z * cos( xx ) * f2 );
#endif
        // #i12430# ...but this seems to work much better.
        fRet = sqrt( 0.636619772 / fNum ) * sin( fNum - 2.356194491 );
    }

    return fRet;
}


double BesselY( double fNum, sal_Int32 nOrder ) throw( IllegalArgumentException )
{
    switch( nOrder )
    {
        case 0:     return Bessely0( fNum );
        case 1:     return Bessely1( fNum );
        default:
        {
            double      fByp;

            double      fTox = 2.0 / fNum;
            double      fBym = Bessely0( fNum );
            double      fBy = Bessely1( fNum );

            for( sal_Int32 n = 1 ; n < nOrder ; n++ )
            {
                fByp = double( n ) * fTox * fBy - fBym;
                fBym = fBy;
                fBy = fByp;
            }

            return fBy;
        }
    }
}


// ============================================================================

} // namespace analysis
} // namespace sca

