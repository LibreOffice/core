/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "bessel.hxx"
#include "analysishelper.hxx"

#include <rtl/math.hxx>

using ::com::sun::star::lang::IllegalArgumentException;
using ::com::sun::star::sheet::NoConvergenceException;

namespace sca {
namespace analysis {

const double f_PI       = 3.1415926535897932385;
const double f_PI_DIV_2 = f_PI / 2.0;
const double f_PI_DIV_4 = f_PI / 4.0;
const double f_2_DIV_PI = 2.0 / f_PI;


// BESSEL J


/*  The BESSEL function, first kind, unmodified:
    The algorithm follows
    http://www.reference-global.com/isbn/978-3-11-020354-7
    Numerical Mathematics 1 / Numerische Mathematik 1,
    An algorithm-based introduction / Eine algorithmisch orientierte Einfuehrung
    Deuflhard, Peter; Hohmann, Andreas
    Berlin, New York (Walter de Gruyter) 2008
    4. ueberarb. u. erw. Aufl. 2008
    eBook ISBN: 978-3-11-020355-4
    Chapter 6.3.2 , algorithm 6.24
    The source is in German.
    The BesselJ-function is a special case of the adjoint summation with
    a_k = 2*(k-1)/x for k=1,...
    b_k = -1, for all k, directly substituted
    m_0=1, m_k=2 for k even, and m_k=0 for k odd, calculated on the fly
    alpha_k=1 for k=N and alpha_k=0 otherwise
*/

double BesselJ( double x, sal_Int32 N ) throw (IllegalArgumentException, NoConvergenceException)

{
    if( N < 0 )
        throw IllegalArgumentException();
    if (x==0.0)
        return (N==0) ? 1.0 : 0.0;

    /*  The algorithm works only for x>0, therefore remember sign. BesselJ
        with integer order N is an even function for even N (means J(-x)=J(x))
        and an odd function for odd N (means J(-x)=-J(x)).*/
    double fSign = (N % 2 == 1 && x < 0) ? -1.0 : 1.0;
    double fX = fabs(x);

    const double fMaxIteration = 9000000.0; //experimental, for to return in < 3 seconds
    double fEstimateIteration = fX * 1.5 + N;
    bool bAsymptoticPossible = pow(fX,0.4) > N;
    if (fEstimateIteration > fMaxIteration)
    {
        if (bAsymptoticPossible)
            return fSign * sqrt(f_2_DIV_PI/fX)* cos(fX-N*f_PI_DIV_2-f_PI_DIV_4);
        else
            throw NoConvergenceException();
    }

    double epsilon = 1.0e-15; // relative error
    bool bHasfound = false;
    double k= 0.0;
    // e_{-1} = 0; e_0 = alpha_0 / b_2
    double  u ; // u_0 = e_0/f_0 = alpha_0/m_0 = alpha_0

    // first used with k=1
    double m_bar;         // m_bar_k = m_k * f_bar_{k-1}
    double g_bar;         // g_bar_k = m_bar_k - a_{k+1} + g_{k-1}
    double g_bar_delta_u; // g_bar_delta_u_k = f_bar_{k-1} * alpha_k
                          // - g_{k-1} * delta_u_{k-1} - m_bar_k * u_{k-1}
    // f_{-1} = 0.0; f_0 = m_0 / b_2 = 1/(-1) = -1
    double g = 0.0;       // g_0= f_{-1} / f_0 = 0/(-1) = 0
    double delta_u = 0.0; // dummy initialize, first used with * 0
    double f_bar = -1.0;  // f_bar_k = 1/f_k, but only used for k=0

    if (N==0)
    {
        //k=0; alpha_0 = 1.0
        u = 1.0; // u_0 = alpha_0
        // k = 1.0; at least one step is necessary
        // m_bar_k = m_k * f_bar_{k-1} ==> m_bar_1 = 0.0
        g_bar_delta_u = 0.0;    // alpha_k = 0.0, m_bar = 0.0; g= 0.0
        g_bar = - 2.0/fX;       // k = 1.0, g = 0.0
        delta_u = g_bar_delta_u / g_bar;
        u = u + delta_u ;       // u_k = u_{k-1} + delta_u_k
        g = -1.0 / g_bar;       // g_k=b_{k+2}/g_bar_k
        f_bar = f_bar * g;      // f_bar_k = f_bar_{k-1}* g_k
        k = 2.0;
        // From now on all alpha_k = 0.0 and k > N+1
    }
    else
    {   // N >= 1 and alpha_k = 0.0 for k<N
        u=0.0; // u_0 = alpha_0
        for (k =1.0; k<= N-1; k = k + 1.0)
        {
            m_bar=2.0 * fmod(k-1.0, 2.0) * f_bar;
            g_bar_delta_u = - g * delta_u - m_bar * u; // alpha_k = 0.0
            g_bar = m_bar - 2.0*k/fX + g;
            delta_u = g_bar_delta_u / g_bar;
            u = u + delta_u;
            g = -1.0/g_bar;
            f_bar=f_bar * g;
        }
        // Step alpha_N = 1.0
        m_bar=2.0 * fmod(k-1.0, 2.0) * f_bar;
        g_bar_delta_u = f_bar - g * delta_u - m_bar * u; // alpha_k = 1.0
        g_bar = m_bar - 2.0*k/fX + g;
        delta_u = g_bar_delta_u / g_bar;
        u = u + delta_u;
        g = -1.0/g_bar;
        f_bar = f_bar * g;
        k = k + 1.0;
    }
    // Loop until desired accuracy, always alpha_k = 0.0
    do
    {
        m_bar = 2.0 * fmod(k-1.0, 2.0) * f_bar;
        g_bar_delta_u = - g * delta_u - m_bar * u;
        g_bar = m_bar - 2.0*k/fX + g;
        delta_u = g_bar_delta_u / g_bar;
        u = u + delta_u;
        g = -1.0/g_bar;
        f_bar = f_bar * g;
        bHasfound = (fabs(delta_u)<=fabs(u)*epsilon);
        k = k + 1.0;
    }
    while (!bHasfound && k <= fMaxIteration);
    if (bHasfound)
        return u * fSign;
    else
        throw NoConvergenceException(); // unlikely to happen
}


// BESSEL I


/*  The BESSEL function, first kind, modified:

                     inf                                  (x/2)^(n+2k)
        I_n(x)  =  SUM   TERM(n,k)   with   TERM(n,k) := --------------
                     k=0                                   k! (n+k)!

    No asymptotic approximation used, see issue 43040.
 */

double BesselI( double x, sal_Int32 n ) throw( IllegalArgumentException, NoConvergenceException )
{
    const sal_Int32 nMaxIteration = 2000;
    const double fXHalf = x / 2.0;
    if( n < 0 )
        throw IllegalArgumentException();

    double fResult = 0.0;

    /*  Start the iteration without TERM(n,0), which is set here.

            TERM(n,0) = (x/2)^n / n!
     */
    sal_Int32 nK = 0;
    double fTerm = 1.0;
    // avoid overflow in Fak(n)
    for( nK = 1; nK <= n; ++nK )
    {
        fTerm = fTerm / static_cast< double >( nK ) * fXHalf;
    }
    fResult = fTerm;    // Start result with TERM(n,0).
    if( fTerm != 0.0 )
    {
        nK = 1;
        const double fEpsilon = 1.0E-15;
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
        fTerm = fTerm * fXHalf / static_cast<double>(nK) * fXHalf / static_cast<double>(nK+n);
        fResult += fTerm;
        nK++;
        }
        while( (fabs( fTerm ) > fabs(fResult) * fEpsilon) && (nK < nMaxIteration) );

    }
    return fResult;
}


double Besselk0( double fNum ) throw( IllegalArgumentException, NoConvergenceException )
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


double Besselk1( double fNum ) throw( IllegalArgumentException, NoConvergenceException )
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


double BesselK( double fNum, sal_Int32 nOrder ) throw( IllegalArgumentException, NoConvergenceException )
{
    switch( nOrder )
    {
        case 0:     return Besselk0( fNum );
        case 1:     return Besselk1( fNum );
        default:
        {
            double      fTox = 2.0 / fNum;
            double      fBkm = Besselk0( fNum );
            double      fBk = Besselk1( fNum );

            for( sal_Int32 n = 1 ; n < nOrder ; n++ )
            {
                const double fBkp = fBkm + double( n ) * fTox * fBk;
                fBkm = fBk;
                fBk = fBkp;
            }

            return fBk;
        }
    }
}


// BESSEL Y


/*  The BESSEL function, second kind, unmodified:
    The algorithm for order 0 and for order 1 follows
    http://www.reference-global.com/isbn/978-3-11-020354-7
    Numerical Mathematics 1 / Numerische Mathematik 1,
    An algorithm-based introduction / Eine algorithmisch orientierte Einfuehrung
    Deuflhard, Peter; Hohmann, Andreas
    Berlin, New York (Walter de Gruyter) 2008
    4. ueberarb. u. erw. Aufl. 2008
    eBook ISBN: 978-3-11-020355-4
    Chapter 6.3.2 , algorithm 6.24
    The source is in German.
    See #i31656# for a commented version of the implementation, attachment #desc6
    http://www.openoffice.org/nonav/issues/showattachment.cgi/63609/Comments%20to%20the%20implementation%20of%20the%20Bessel%20functions.odt
*/

double Bessely0( double fX ) throw( IllegalArgumentException, NoConvergenceException )
{
    if (fX <= 0)
        throw IllegalArgumentException();
    const double fMaxIteration = 9000000.0; // should not be reached
    if (fX > 5.0e+6) // iteration is not considerable better then approximation
        return sqrt(1/f_PI/fX)
                *(rtl::math::sin(fX)-rtl::math::cos(fX));
    const double epsilon = 1.0e-15;
    const double EulerGamma = 0.57721566490153286060;
    double alpha = log(fX/2.0)+EulerGamma;
    double u = alpha;

    double k = 1.0;
    double g_bar_delta_u = 0.0;
    double g_bar = -2.0 / fX;
    double delta_u = g_bar_delta_u / g_bar;
    double g = -1.0/g_bar;
    double f_bar = -1 * g;

    double sign_alpha = 1.0;
    bool bHasFound = false;
    k = k + 1;
    do
    {
        double km1mod2 = fmod(k-1.0, 2.0);
        double m_bar = (2.0*km1mod2) * f_bar;
        if (km1mod2 == 0.0)
            alpha = 0.0;
        else
        {
           alpha = sign_alpha * (4.0/k);
           sign_alpha = -sign_alpha;
        }
        g_bar_delta_u = f_bar * alpha - g * delta_u - m_bar * u;
        g_bar = m_bar - (2.0*k)/fX + g;
        delta_u = g_bar_delta_u / g_bar;
        u = u+delta_u;
        g = -1.0 / g_bar;
        f_bar = f_bar*g;
        bHasFound = (fabs(delta_u)<=fabs(u)*epsilon);
        k=k+1;
    }
    while (!bHasFound && k<fMaxIteration);
    if (bHasFound)
        return u*f_2_DIV_PI;
    else
        throw NoConvergenceException(); // not likely to happen
}

// See #i31656# for a commented version of this implementation, attachment #desc6
// http://www.openoffice.org/nonav/issues/showattachment.cgi/63609/Comments%20to%20the%20implementation%20of%20the%20Bessel%20functions.odt
double Bessely1( double fX ) throw( IllegalArgumentException, NoConvergenceException )
{
    if (fX <= 0)
        throw IllegalArgumentException();
    const double fMaxIteration = 9000000.0; // should not be reached
    if (fX > 5.0e+6) // iteration is not considerable better then approximation
        return - sqrt(1/f_PI/fX)
                *(rtl::math::sin(fX)+rtl::math::cos(fX));
    const double epsilon = 1.0e-15;
    const double EulerGamma = 0.57721566490153286060;
    double alpha = 1.0/fX;
    double f_bar = -1.0;
    double u = alpha;
    double k = 1.0;
    alpha = 1.0 - EulerGamma - log(fX/2.0);
    double g_bar_delta_u = -alpha;
    double g_bar = -2.0 / fX;
    double delta_u = g_bar_delta_u / g_bar;
    u = u + delta_u;
    double g = -1.0/g_bar;
    f_bar = f_bar * g;
    double sign_alpha = -1.0;
    bool bHasFound = false;
    k = k + 1.0;
    do
    {
        double km1mod2 = fmod(k-1.0,2.0);
        double m_bar = (2.0*km1mod2) * f_bar;
        double q = (k-1.0)/2.0;
        if (km1mod2 == 0.0) // k is odd
        {
           alpha = sign_alpha * (1.0/q + 1.0/(q+1.0));
           sign_alpha = -sign_alpha;
        }
        else
            alpha = 0.0;
        g_bar_delta_u = f_bar * alpha - g * delta_u - m_bar * u;
        g_bar = m_bar - (2.0*k)/fX + g;
        delta_u = g_bar_delta_u / g_bar;
        u = u+delta_u;
        g = -1.0 / g_bar;
        f_bar = f_bar*g;
        bHasFound = (fabs(delta_u)<=fabs(u)*epsilon);
        k=k+1;
    }
    while (!bHasFound && k<fMaxIteration);
    if (bHasFound)
        return -u*2.0/f_PI;
    else
        throw NoConvergenceException();
}

double BesselY( double fNum, sal_Int32 nOrder ) throw( IllegalArgumentException, NoConvergenceException )
{
    switch( nOrder )
    {
        case 0:     return Bessely0( fNum );
        case 1:     return Bessely1( fNum );
        default:
        {
            double      fTox = 2.0 / fNum;
            double      fBym = Bessely0( fNum );
            double      fBy = Bessely1( fNum );

            for( sal_Int32 n = 1 ; n < nOrder ; n++ )
            {
                const double fByp = double( n ) * fTox * fBy - fBym;
                fBym = fBy;
                fBy = fByp;
            }

            return fBy;
        }
    }
}

} // namespace analysis
} // namespace sca

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
