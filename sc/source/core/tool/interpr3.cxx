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

#include <tools/solar.h>
#include <stdlib.h>
#include <string.h>

#include <interpre.hxx>
#include <global.hxx>
#include <compiler.hxx>
#include <formulacell.hxx>
#include <document.hxx>
#include <dociter.hxx>
#include <matrixoperators.hxx>
#include <scmatrix.hxx>

#include <math.h>
#include <cassert>
#include <memory>
#include <set>
#include <vector>
#include <algorithm>
#include <comphelper/random.hxx>
#include <osl/diagnose.h>

using ::std::vector;
using namespace formula;

/// Two columns of data should be sortable with GetSortArray() and QuickSort()
// This is an arbitrary limit.
#define MAX_COUNT_DOUBLE_FOR_SORT (MAXROWCOUNT * 2)

const double ScInterpreter::fMaxGammaArgument = 171.624376956302;  // found experimental
const double fMachEps = ::std::numeric_limits<double>::epsilon();

class ScDistFunc
{
public:
    virtual double GetValue(double x) const = 0;

protected:
    ~ScDistFunc() {}
};

//  iteration for inverse distributions

//template< class T > double lcl_IterateInverse( const T& rFunction, double x0, double x1, bool& rConvError )

/** u*w<0.0 fails for values near zero */
static bool lcl_HasChangeOfSign( double u, double w )
{
    return (u < 0.0 && w > 0.0) || (u > 0.0 && w < 0.0);
}

static double lcl_IterateInverse( const ScDistFunc& rFunction, double fAx, double fBx, bool& rConvError )
{
    rConvError = false;
    const double fYEps = 1.0E-307;
    const double fXEps = ::std::numeric_limits<double>::epsilon();

    OSL_ENSURE(fAx<fBx, "IterateInverse: wrong interval");

    //  find enclosing interval

    double fAy = rFunction.GetValue(fAx);
    double fBy = rFunction.GetValue(fBx);
    double fTemp;
    unsigned short nCount;
    for (nCount = 0; nCount < 1000 && !lcl_HasChangeOfSign(fAy,fBy); nCount++)
    {
        if (fabs(fAy) <= fabs(fBy))
        {
            fTemp = fAx;
            fAx += 2.0 * (fAx - fBx);
            if (fAx < 0.0)
                fAx = 0.0;
            fBx = fTemp;
            fBy = fAy;
            fAy = rFunction.GetValue(fAx);
        }
        else
        {
            fTemp = fBx;
            fBx += 2.0 * (fBx - fAx);
            fAx = fTemp;
            fAy = fBy;
            fBy = rFunction.GetValue(fBx);
        }
    }

    if (fAy == 0.0)
        return fAx;
    if (fBy == 0.0)
        return fBx;
    if (!lcl_HasChangeOfSign( fAy, fBy))
    {
        rConvError = true;
        return 0.0;
    }
    // inverse quadric interpolation with additional brackets
    // set three points
    double fPx = fAx;
    double fPy = fAy;
    double fQx = fBx;
    double fQy = fBy;
    double fRx = fAx;
    double fRy = fAy;
    double fSx = 0.5 * (fAx + fBx); // potential next point
    bool bHasToInterpolate = true;
    nCount = 0;
    while ( nCount < 500 && fabs(fRy) > fYEps &&
            (fBx-fAx) > ::std::max( fabs(fAx), fabs(fBx)) * fXEps )
    {
        if (bHasToInterpolate)
        {
            if (fPy!=fQy && fQy!=fRy && fRy!=fPy)
            {
                fSx = fPx * fRy * fQy / (fRy-fPy) / (fQy-fPy)
                    + fRx * fQy * fPy / (fQy-fRy) / (fPy-fRy)
                    + fQx * fPy * fRy / (fPy-fQy) / (fRy-fQy);
                bHasToInterpolate = (fAx < fSx) && (fSx < fBx); // inside the brackets?
            }
            else
                bHasToInterpolate = false;
        }
        if(!bHasToInterpolate)
        {
            fSx = 0.5 * (fAx + fBx);
            // reset points
            fQx = fBx; fQy = fBy;
            bHasToInterpolate = true;
        }
        // shift points for next interpolation
        fPx = fQx; fQx = fRx; fRx = fSx;
        fPy = fQy; fQy = fRy; fRy = rFunction.GetValue(fSx);
        // update brackets
        if (lcl_HasChangeOfSign( fAy, fRy))
        {
            fBx = fRx; fBy = fRy;
        }
        else
        {
            fAx = fRx; fAy = fRy;
        }
        // if last iteration brought to small advance, then do bisection next
        // time, for safety
        bHasToInterpolate = bHasToInterpolate && (fabs(fRy) * 2.0 <= fabs(fQy));
        ++nCount;
    }
    return fRx;
}

// General functions

void ScInterpreter::ScNoName()
{
    PushError(FormulaError::NoName);
}

void ScInterpreter::ScBadName()
{
    short nParamCount = GetByte();
    while (nParamCount-- > 0)
    {
        PopError();
    }
    PushError( FormulaError::NoName);
}

double ScInterpreter::phi(double x)
{
    return  0.39894228040143268 * exp(-(x * x) / 2.0);
}

double ScInterpreter::integralPhi(double x)
{ // Using gauss(x)+0.5 has severe cancellation errors for x<-4
    return 0.5 * ::rtl::math::erfc(-x * 0.7071067811865475); // * 1/sqrt(2)
}

double ScInterpreter::taylor(const double* pPolynom, sal_uInt16 nMax, double x)
{
    double nVal = pPolynom[nMax];
    for (short i = nMax-1; i >= 0; i--)
    {
        nVal = pPolynom[i] + (nVal * x);
    }
    return nVal;
}

double ScInterpreter::gauss(double x)
{

    double xAbs = fabs(x);
    sal_uInt16 xShort = static_cast<sal_uInt16>(::rtl::math::approxFloor(xAbs));
    double nVal = 0.0;
    if (xShort == 0)
    {
        static const double t0[] =
        { 0.39894228040143268, -0.06649038006690545,  0.00997355701003582,
         -0.00118732821548045,  0.00011543468761616, -0.00000944465625950,
          0.00000066596935163, -0.00000004122667415,  0.00000000227352982,
          0.00000000011301172,  0.00000000000511243, -0.00000000000021218 };
        nVal = taylor(t0, 11, (xAbs * xAbs)) * xAbs;
    }
    else if (xShort <= 2)
    {
        static const double t2[] =
        { 0.47724986805182079,  0.05399096651318805, -0.05399096651318805,
          0.02699548325659403, -0.00449924720943234, -0.00224962360471617,
          0.00134977416282970, -0.00011783742691370, -0.00011515930357476,
          0.00003704737285544,  0.00000282690796889, -0.00000354513195524,
          0.00000037669563126,  0.00000019202407921, -0.00000005226908590,
         -0.00000000491799345,  0.00000000366377919, -0.00000000015981997,
         -0.00000000017381238,  0.00000000002624031,  0.00000000000560919,
         -0.00000000000172127, -0.00000000000008634,  0.00000000000007894 };
        nVal = taylor(t2, 23, (xAbs - 2.0));
    }
    else if (xShort <= 4)
    {
        static const double t4[] =
       { 0.49996832875816688,  0.00013383022576489, -0.00026766045152977,
         0.00033457556441221, -0.00028996548915725,  0.00018178605666397,
        -0.00008252863922168,  0.00002551802519049, -0.00000391665839292,
        -0.00000074018205222,  0.00000064422023359, -0.00000017370155340,
         0.00000000909595465,  0.00000000944943118, -0.00000000329957075,
         0.00000000029492075,  0.00000000011874477, -0.00000000004420396,
         0.00000000000361422,  0.00000000000143638, -0.00000000000045848 };
        nVal = taylor(t4, 20, (xAbs - 4.0));
    }
    else
    {
        static const double asympt[] = { -1.0, 1.0, -3.0, 15.0, -105.0 };
        nVal = 0.5 + phi(xAbs) * taylor(asympt, 4, 1.0 / (xAbs * xAbs)) / xAbs;
    }
    if (x < 0.0)
        return -nVal;
    else
        return nVal;
}

//  #i26836# new gaussinv implementation by Martin Eitzenberger <m.eitzenberger@unix.net>

double ScInterpreter::gaussinv(double x)
{
    double q,t,z;

    q=x-0.5;

    if(fabs(q)<=.425)
    {
        t=0.180625-q*q;

        z=
        q*
        (
            (
                (
                    (
                        (
                            (
                                (
                                    t*2509.0809287301226727+33430.575583588128105
                                )
                                *t+67265.770927008700853
                            )
                            *t+45921.953931549871457
                        )
                        *t+13731.693765509461125
                    )
                    *t+1971.5909503065514427
                )
                *t+133.14166789178437745
            )
            *t+3.387132872796366608
        )
        /
        (
            (
                (
                    (
                        (
                            (
                                (
                                    t*5226.495278852854561+28729.085735721942674
                                )
                                *t+39307.89580009271061
                            )
                            *t+21213.794301586595867
                        )
                        *t+5394.1960214247511077
                    )
                    *t+687.1870074920579083
                )
                *t+42.313330701600911252
            )
            *t+1.0
        );

    }
    else
    {
        if(q>0) t=1-x;
        else        t=x;

        t=sqrt(-log(t));

        if(t<=5.0)
        {
            t+=-1.6;

            z=
            (
                (
                    (
                        (
                            (
                                (
                                    (
                                        t*7.7454501427834140764e-4+0.0227238449892691845833
                                    )
                                    *t+0.24178072517745061177
                                )
                                *t+1.27045825245236838258
                            )
                            *t+3.64784832476320460504
                        )
                        *t+5.7694972214606914055
                    )
                    *t+4.6303378461565452959
                )
                *t+1.42343711074968357734
            )
            /
            (
                (
                    (
                        (
                            (
                                (
                                    (
                                        t*1.05075007164441684324e-9+5.475938084995344946e-4
                                    )
                                    *t+0.0151986665636164571966
                                )
                                *t+0.14810397642748007459
                            )
                            *t+0.68976733498510000455
                        )
                        *t+1.6763848301838038494
                    )
                    *t+2.05319162663775882187
                )
                *t+1.0
            );

        }
        else
        {
            t+=-5.0;

            z=
            (
                (
                    (
                        (
                            (
                                (
                                    (
                                        t*2.01033439929228813265e-7+2.71155556874348757815e-5
                                    )
                                    *t+0.0012426609473880784386
                                )
                                *t+0.026532189526576123093
                            )
                            *t+0.29656057182850489123
                        )
                        *t+1.7848265399172913358
                    )
                    *t+5.4637849111641143699
                )
                *t+6.6579046435011037772
            )
            /
            (
                (
                    (
                        (
                            (
                                (
                                    (
                                        t*2.04426310338993978564e-15+1.4215117583164458887e-7
                                    )
                                    *t+1.8463183175100546818e-5
                                )
                                *t+7.868691311456132591e-4
                            )
                            *t+0.0148753612908506148525
                        )
                        *t+0.13692988092273580531
                    )
                    *t+0.59983220655588793769
                )
                *t+1.0
            );

        }

        if(q<0.0) z=-z;
    }

    return z;
}

double ScInterpreter::Fakultaet(double x)
{
    x = ::rtl::math::approxFloor(x);
    if (x < 0.0)
        return 0.0;
    else if (x == 0.0)
        return 1.0;
    else if (x <= 170.0)
    {
        double fTemp = x;
        while (fTemp > 2.0)
        {
            fTemp--;
            x *= fTemp;
        }
    }
    else
        SetError(FormulaError::NoValue);
    return x;
}

double ScInterpreter::BinomKoeff(double n, double k)
{
    // this method has been duplicated as BinomialCoefficient()
    // in scaddins/source/analysis/analysishelper.cxx

    double nVal = 0.0;
    k = ::rtl::math::approxFloor(k);
    if (n < k)
        nVal = 0.0;
    else if (k == 0.0)
        nVal = 1.0;
    else
    {
        nVal = n/k;
        n--;
        k--;
        while (k > 0.0)
        {
            nVal *= n/k;
            k--;
            n--;
        }

    }
    return nVal;
}

// The algorithm is based on lanczos13m53 in lanczos.hpp
// in math library from http://www.boost.org
/** you must ensure fZ>0
    Uses a variant of the Lanczos sum with a rational function. */
static double lcl_getLanczosSum(double fZ)
{
    static const double fNum[13] ={
        23531376880.41075968857200767445163675473,
        42919803642.64909876895789904700198885093,
        35711959237.35566804944018545154716670596,
        17921034426.03720969991975575445893111267,
        6039542586.35202800506429164430729792107,
        1439720407.311721673663223072794912393972,
        248874557.8620541565114603864132294232163,
        31426415.58540019438061423162831820536287,
        2876370.628935372441225409051620849613599,
        186056.2653952234950402949897160456992822,
        8071.672002365816210638002902272250613822,
        210.8242777515793458725097339207133627117,
        2.506628274631000270164908177133837338626
        };
    static const double fDenom[13] = {
        0,
        39916800,
        120543840,
        150917976,
        105258076,
        45995730,
        13339535,
        2637558,
        357423,
        32670,
        1925,
        66,
        1
        };
    // Horner scheme
    double fSumNum;
    double fSumDenom;
    int nI;
    if (fZ<=1.0)
    {
        fSumNum = fNum[12];
        fSumDenom = fDenom[12];
        for (nI = 11; nI >= 0; --nI)
        {
            fSumNum *= fZ;
            fSumNum += fNum[nI];
            fSumDenom *= fZ;
            fSumDenom += fDenom[nI];
        }
    }
    else
    // Cancel down with fZ^12; Horner scheme with reverse coefficients
    {
        double fZInv = 1/fZ;
        fSumNum = fNum[0];
        fSumDenom = fDenom[0];
        for (nI = 1; nI <=12; ++nI)
        {
            fSumNum *= fZInv;
            fSumNum += fNum[nI];
            fSumDenom *= fZInv;
            fSumDenom += fDenom[nI];
        }
    }
    return fSumNum/fSumDenom;
}

// The algorithm is based on tgamma in gamma.hpp
// in math library from http://www.boost.org
/** You must ensure fZ>0; fZ>171.624376956302 will overflow. */
static double lcl_GetGammaHelper(double fZ)
{
    double fGamma = lcl_getLanczosSum(fZ);
    const double fg = 6.024680040776729583740234375;
    double fZgHelp = fZ + fg - 0.5;
    // avoid intermediate overflow
    double fHalfpower = pow( fZgHelp, fZ / 2 - 0.25);
    fGamma *= fHalfpower;
    fGamma /= exp(fZgHelp);
    fGamma *= fHalfpower;
    if (fZ <= 20.0 && fZ == ::rtl::math::approxFloor(fZ))
        fGamma = ::rtl::math::round(fGamma);
    return fGamma;
}

// The algorithm is based on tgamma in gamma.hpp
// in math library from http://www.boost.org
/** You must ensure fZ>0 */
static double lcl_GetLogGammaHelper(double fZ)
{
    const double fg = 6.024680040776729583740234375;
    double fZgHelp = fZ + fg - 0.5;
    return log( lcl_getLanczosSum(fZ)) + (fZ-0.5) * log(fZgHelp) - fZgHelp;
}

/** You must ensure non integer arguments for fZ<1 */
double ScInterpreter::GetGamma(double fZ)
{
    const double fLogPi = log(F_PI);
    const double fLogDblMax = log( ::std::numeric_limits<double>::max());

    if (fZ > fMaxGammaArgument)
    {
        SetError(FormulaError::IllegalFPOperation);
        return HUGE_VAL;
    }

    if (fZ >= 1.0)
        return lcl_GetGammaHelper(fZ);

    if (fZ >= 0.5)  // shift to x>=1 using Gamma(x)=Gamma(x+1)/x
        return lcl_GetGammaHelper(fZ+1) / fZ;

    if (fZ >= -0.5) // shift to x>=1, might overflow
    {
        double fLogTest = lcl_GetLogGammaHelper(fZ+2) - rtl::math::log1p(fZ) - log( fabs(fZ));
        if (fLogTest >= fLogDblMax)
        {
            SetError( FormulaError::IllegalFPOperation);
            return HUGE_VAL;
        }
        return lcl_GetGammaHelper(fZ+2) / (fZ+1) / fZ;
    }
    // fZ<-0.5
    // Use Euler's reflection formula: gamma(x)= pi/ ( gamma(1-x)*sin(pi*x) )
    double fLogDivisor = lcl_GetLogGammaHelper(1-fZ) + log( fabs( ::rtl::math::sin( F_PI*fZ)));
    if (fLogDivisor - fLogPi >= fLogDblMax)     // underflow
        return 0.0;

    if (fLogDivisor<0.0)
        if (fLogPi - fLogDivisor > fLogDblMax)  // overflow
        {
            SetError(FormulaError::IllegalFPOperation);
            return HUGE_VAL;
        }

    return exp( fLogPi - fLogDivisor) * ((::rtl::math::sin( F_PI*fZ) < 0.0) ? -1.0 : 1.0);
}

/** You must ensure fZ>0 */
double ScInterpreter::GetLogGamma(double fZ)
{
    if (fZ >= fMaxGammaArgument)
        return lcl_GetLogGammaHelper(fZ);
    if (fZ >= 1.0)
        return log(lcl_GetGammaHelper(fZ));
    if (fZ >= 0.5)
        return log( lcl_GetGammaHelper(fZ+1) / fZ);
    return lcl_GetLogGammaHelper(fZ+2) - rtl::math::log1p(fZ) - log(fZ);
}

double ScInterpreter::GetFDist(double x, double fF1, double fF2)
{
    double arg = fF2/(fF2+fF1*x);
    double alpha = fF2/2.0;
    double beta = fF1/2.0;
    return GetBetaDist(arg, alpha, beta);
}

double ScInterpreter::GetTDist( double T, double fDF, int nType )
{
    switch ( nType )
    {
        case 1 : // 1-tailed T-distribution
            return 0.5 * GetBetaDist( fDF / ( fDF + T * T ), fDF / 2.0, 0.5 );
        case 2 : // 2-tailed T-distribution
            return GetBetaDist( fDF / ( fDF + T * T ), fDF / 2.0, 0.5);
        case 3 : // left-tailed T-distribution (probability density function)
            return pow( 1 + ( T * T / fDF ), -( fDF + 1 ) / 2 ) / ( sqrt( fDF ) * GetBeta( 0.5, fDF / 2.0 ) );
        case 4 : // left-tailed T-distribution (cumulative distribution function)
            double X = fDF / ( T * T + fDF );
            double R = 0.5 * GetBetaDist( X, 0.5 * fDF, 0.5 );
            return ( T < 0 ? R : 1 - R );
    }
    SetError( FormulaError::IllegalArgument );
    return HUGE_VAL;
}

// for LEGACY.CHIDIST, returns right tail, fDF=degrees of freedom
/** You must ensure fDF>0.0 */
double ScInterpreter::GetChiDist(double fX, double fDF)
{
    if (fX <= 0.0)
        return 1.0; // see ODFF
    else
        return GetUpRegIGamma( fDF/2.0, fX/2.0);
}

// ready for ODF 1.2
// for ODF CHISQDIST; cumulative distribution function, fDF=degrees of freedom
// returns left tail
/** You must ensure fDF>0.0 */
double ScInterpreter::GetChiSqDistCDF(double fX, double fDF)
{
    if (fX <= 0.0)
        return 0.0; // see ODFF
    else
        return GetLowRegIGamma( fDF/2.0, fX/2.0);
}

double ScInterpreter::GetChiSqDistPDF(double fX, double fDF)
{
    // you must ensure fDF is positive integer
    double fValue;
    if (fX <= 0.0)
        return 0.0; // see ODFF
    if (fDF*fX > 1391000.0)
    {
        // intermediate invalid values, use log
        fValue = exp((0.5*fDF - 1) * log(fX*0.5) - 0.5 * fX - log(2.0) - GetLogGamma(0.5*fDF));
    }
    else // fDF is small in most cases, we can iterate
    {
        double fCount;
        if (fmod(fDF,2.0)<0.5)
        {
            // even
            fValue = 0.5;
            fCount = 2.0;
        }
        else
        {
            fValue = 1/sqrt(fX*2*F_PI);
            fCount = 1.0;
        }
        while ( fCount < fDF)
        {
            fValue *= (fX / fCount);
            fCount += 2.0;
        }
        if (fX>=1425.0) // underflow in e^(-x/2)
            fValue = exp(log(fValue)-fX/2);
        else
            fValue *= exp(-fX/2);
    }
    return fValue;
}

void ScInterpreter::ScChiSqDist()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 2, 3 ) )
        return;
    bool bCumulative;
    if (nParamCount == 3)
        bCumulative = GetBool();
    else
        bCumulative = true;
    double fDF = ::rtl::math::approxFloor(GetDouble());
    if (fDF < 1.0)
        PushIllegalArgument();
    else
    {
        double fX = GetDouble();
        if (bCumulative)
            PushDouble(GetChiSqDistCDF(fX,fDF));
        else
            PushDouble(GetChiSqDistPDF(fX,fDF));
    }
}

void ScInterpreter::ScChiSqDist_MS()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 3, 3 ) )
        return;
    bool bCumulative = GetBool();
    double fDF = ::rtl::math::approxFloor( GetDouble() );
    if ( fDF < 1.0 || fDF > 1E10 )
        PushIllegalArgument();
    else
    {
        double fX = GetDouble();
        if ( fX < 0 )
            PushIllegalArgument();
        else
        {
            if ( bCumulative )
                PushDouble( GetChiSqDistCDF( fX, fDF ) );
            else
                PushDouble( GetChiSqDistPDF( fX, fDF ) );
        }
    }
}

void ScInterpreter::ScGamma()
{
    double x = GetDouble();
    if (x <= 0.0 && x == ::rtl::math::approxFloor(x))
        PushIllegalArgument();
    else
    {
        double fResult = GetGamma(x);
        if (nGlobalError != FormulaError::NONE)
        {
            PushError( nGlobalError);
            return;
        }
        PushDouble(fResult);
    }
}

void ScInterpreter::ScLogGamma()
{
    double x = GetDouble();
    if (x > 0.0)    // constraint from ODFF
        PushDouble( GetLogGamma(x));
    else
        PushIllegalArgument();
}

double ScInterpreter::GetBeta(double fAlpha, double fBeta)
{
    double fA;
    double fB;
    if (fAlpha > fBeta)
    {
        fA = fAlpha; fB = fBeta;
    }
    else
    {
        fA = fBeta; fB = fAlpha;
    }
    if (fA+fB < fMaxGammaArgument) // simple case
        return GetGamma(fA)/GetGamma(fA+fB)*GetGamma(fB);
    // need logarithm
    // GetLogGamma is not accurate enough, back to Lanczos for all three
    // GetGamma and arrange factors newly.
    const double fg = 6.024680040776729583740234375; //see GetGamma
    double fgm = fg - 0.5;
    double fLanczos = lcl_getLanczosSum(fA);
    fLanczos /= lcl_getLanczosSum(fA+fB);
    fLanczos *= lcl_getLanczosSum(fB);
    double fABgm = fA+fB+fgm;
    fLanczos *= sqrt((fABgm/(fA+fgm))/(fB+fgm));
    double fTempA = fB/(fA+fgm); // (fA+fgm)/fABgm = 1 / ( 1 + fB/(fA+fgm))
    double fTempB = fA/(fB+fgm);
    double fResult = exp(-fA * ::rtl::math::log1p(fTempA)
                            -fB * ::rtl::math::log1p(fTempB)-fgm);
    fResult *= fLanczos;
    return fResult;
}

// Same as GetBeta but with logarithm
double ScInterpreter::GetLogBeta(double fAlpha, double fBeta)
{
    double fA;
    double fB;
    if (fAlpha > fBeta)
    {
        fA = fAlpha; fB = fBeta;
    }
    else
    {
        fA = fBeta; fB = fAlpha;
    }
    const double fg = 6.024680040776729583740234375; //see GetGamma
    double fgm = fg - 0.5;
    double fLanczos = lcl_getLanczosSum(fA);
    fLanczos /= lcl_getLanczosSum(fA+fB);
    fLanczos *= lcl_getLanczosSum(fB);
    double fLogLanczos = log(fLanczos);
    double fABgm = fA+fB+fgm;
    fLogLanczos += 0.5*(log(fABgm)-log(fA+fgm)-log(fB+fgm));
    double fTempA = fB/(fA+fgm); // (fA+fgm)/fABgm = 1 / ( 1 + fB/(fA+fgm))
    double fTempB = fA/(fB+fgm);
    double fResult = -fA * ::rtl::math::log1p(fTempA)
                        -fB * ::rtl::math::log1p(fTempB)-fgm;
    fResult += fLogLanczos;
    return fResult;
}

// beta distribution probability density function
double ScInterpreter::GetBetaDistPDF(double fX, double fA, double fB)
{
    // special cases
    if (fA == 1.0) // result b*(1-x)^(b-1)
    {
        if (fB == 1.0)
            return 1.0;
        if (fB == 2.0)
            return -2.0*fX + 2.0;
        if (fX == 1.0 && fB < 1.0)
        {
            SetError(FormulaError::IllegalArgument);
            return HUGE_VAL;
        }
        if (fX <= 0.01)
            return fB + fB * ::rtl::math::expm1((fB-1.0) * ::rtl::math::log1p(-fX));
        else
            return fB * pow(0.5-fX+0.5,fB-1.0);
    }
    if (fB == 1.0) // result a*x^(a-1)
    {
        if (fA == 2.0)
            return fA * fX;
        if (fX == 0.0 && fA < 1.0)
        {
            SetError(FormulaError::IllegalArgument);
            return HUGE_VAL;
        }
        return fA * pow(fX,fA-1);
    }
    if (fX <= 0.0)
    {
        if (fA < 1.0 && fX == 0.0)
        {
            SetError(FormulaError::IllegalArgument);
            return HUGE_VAL;
        }
        else
            return 0.0;
    }
    if (fX >= 1.0)
    {
        if (fB < 1.0 && fX == 1.0)
        {
            SetError(FormulaError::IllegalArgument);
            return HUGE_VAL;
        }
        else
            return 0.0;
    }

    // normal cases; result x^(a-1)*(1-x)^(b-1)/Beta(a,b)
    const double fLogDblMax = log( ::std::numeric_limits<double>::max());
    const double fLogDblMin = log( ::std::numeric_limits<double>::min());
    double fLogY = (fX < 0.1) ? ::rtl::math::log1p(-fX) : log(0.5-fX+0.5);
    double fLogX = log(fX);
    double fAm1LogX = (fA-1.0) * fLogX;
    double fBm1LogY = (fB-1.0) * fLogY;
    double fLogBeta = GetLogBeta(fA,fB);
    // check whether parts over- or underflow
    if (   fAm1LogX < fLogDblMax  && fAm1LogX > fLogDblMin
        && fBm1LogY < fLogDblMax  && fBm1LogY > fLogDblMin
        && fLogBeta < fLogDblMax  && fLogBeta > fLogDblMin
        && fAm1LogX + fBm1LogY < fLogDblMax && fAm1LogX + fBm1LogY > fLogDblMin)
        return pow(fX,fA-1.0) * pow(0.5-fX+0.5,fB-1.0) / GetBeta(fA,fB);
    else // need logarithm;
        // might overflow as a whole, but seldom, not worth to pre-detect it
        return exp( fAm1LogX + fBm1LogY - fLogBeta);
}

/*
                x^a * (1-x)^b
    I_x(a,b) = ----------------  * result of ContFrac
                a * Beta(a,b)
*/
static double lcl_GetBetaHelperContFrac(double fX, double fA, double fB)
{   // like old version
    double a1, b1, a2, b2, fnorm, cfnew, cf;
    a1 = 1.0; b1 = 1.0;
    b2 = 1.0 - (fA+fB)/(fA+1.0)*fX;
    if (b2 == 0.0)
    {
        a2 = 0.0;
        fnorm = 1.0;
        cf = 1.0;
    }
    else
    {
        a2 = 1.0;
        fnorm = 1.0/b2;
        cf = a2*fnorm;
    }
    cfnew = 1.0;
    double rm = 1.0;

    const double fMaxIter = 50000.0;
    // loop security, normal cases converge in less than 100 iterations.
    // FIXME: You will get so much iterations for fX near mean,
    // I do not know a better algorithm.
    bool bfinished = false;
    do
    {
        const double apl2m = fA + 2.0*rm;
        const double d2m = rm*(fB-rm)*fX/((apl2m-1.0)*apl2m);
        const double d2m1 = -(fA+rm)*(fA+fB+rm)*fX/(apl2m*(apl2m+1.0));
        a1 = (a2+d2m*a1)*fnorm;
        b1 = (b2+d2m*b1)*fnorm;
        a2 = a1 + d2m1*a2*fnorm;
        b2 = b1 + d2m1*b2*fnorm;
        if (b2 != 0.0)
        {
            fnorm = 1.0/b2;
            cfnew = a2*fnorm;
            bfinished = (fabs(cf-cfnew) < fabs(cf)*fMachEps);
        }
        cf = cfnew;
        rm += 1.0;
    }
    while (rm < fMaxIter && !bfinished);
    return cf;
}

// cumulative distribution function, normalized
double ScInterpreter::GetBetaDist(double fXin, double fAlpha, double fBeta)
{
// special cases
    if (fXin <= 0.0)  // values are valid, see spec
        return 0.0;
    if (fXin >= 1.0)  // values are valid, see spec
        return 1.0;
    if (fBeta == 1.0)
        return pow(fXin, fAlpha);
    if (fAlpha == 1.0)
    //            1.0 - pow(1.0-fX,fBeta) is not accurate enough
        return -::rtl::math::expm1(fBeta * ::rtl::math::log1p(-fXin));
    //FIXME: need special algorithm for fX near fP for large fA,fB
    double fResult;
    // I use always continued fraction, power series are neither
    // faster nor more accurate.
    double fY = (0.5-fXin)+0.5;
    double flnY = ::rtl::math::log1p(-fXin);
    double fX = fXin;
    double flnX = log(fXin);
    double fA = fAlpha;
    double fB = fBeta;
    bool bReflect = fXin > fAlpha/(fAlpha+fBeta);
    if (bReflect)
    {
        fA = fBeta;
        fB = fAlpha;
        fX = fY;
        fY = fXin;
        flnX = flnY;
        flnY = log(fXin);
    }
    fResult = lcl_GetBetaHelperContFrac(fX,fA,fB);
    fResult = fResult/fA;
    double fP = fA/(fA+fB);
    double fQ = fB/(fA+fB);
    double fTemp;
    if (fA > 1.0 && fB > 1.0 && fP < 0.97 && fQ < 0.97) //found experimental
        fTemp = GetBetaDistPDF(fX,fA,fB)*fX*fY;
    else
        fTemp = exp(fA*flnX + fB*flnY - GetLogBeta(fA,fB));
    fResult *= fTemp;
    if (bReflect)
        fResult = 0.5 - fResult + 0.5;
    if (fResult > 1.0) // ensure valid range
        fResult = 1.0;
    if (fResult < 0.0)
        fResult = 0.0;
    return fResult;
}

void ScInterpreter::ScBetaDist()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 3, 6 ) ) // expanded, see #i91547#
        return;
    double fLowerBound, fUpperBound;
    double alpha, beta, x;
    bool bIsCumulative;
    if (nParamCount == 6)
        bIsCumulative = GetBool();
    else
        bIsCumulative = true;
    if (nParamCount >= 5)
        fUpperBound = GetDouble();
    else
        fUpperBound = 1.0;
    if (nParamCount >= 4)
        fLowerBound = GetDouble();
    else
        fLowerBound = 0.0;
    beta = GetDouble();
    alpha = GetDouble();
    x = GetDouble();
    double fScale = fUpperBound - fLowerBound;
    if (fScale <= 0.0 || alpha <= 0.0 || beta <= 0.0)
    {
        PushIllegalArgument();
        return;
    }
    if (bIsCumulative) // cumulative distribution function
    {
        // special cases
        if (x < fLowerBound)
        {
            PushDouble(0.0); return; //see spec
        }
        if (x > fUpperBound)
        {
            PushDouble(1.0); return; //see spec
        }
        // normal cases
        x = (x-fLowerBound)/fScale;  // convert to standard form
        PushDouble(GetBetaDist(x, alpha, beta));
        return;
    }
    else // probability density function
    {
        if (x < fLowerBound || x > fUpperBound)
        {
            PushDouble(0.0);
            return;
        }
        x = (x-fLowerBound)/fScale;
        PushDouble(GetBetaDistPDF(x, alpha, beta)/fScale);
        return;
    }
}

/**
  Microsoft version has parameters in different order
  Also, upper and lowerbound are optional and have default values
  and different constraints apply.
  Basically, function is identical with ScInterpreter::ScBetaDist()
*/
void ScInterpreter::ScBetaDist_MS()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 4, 6 ) )
        return;
    double fLowerBound, fUpperBound;
    double alpha, beta, x;
    bool bIsCumulative;
    if (nParamCount == 6)
        fUpperBound = GetDouble();
    else
        fUpperBound = 1.0;
    if (nParamCount >= 5)
        fLowerBound = GetDouble();
    else
        fLowerBound = 0.0;
    bIsCumulative = GetBool();
    beta = GetDouble();
    alpha = GetDouble();
    x = GetDouble();
    if (alpha <= 0.0 || beta <= 0.0 || x < fLowerBound || x > fUpperBound)
    {
        PushIllegalArgument();
        return;
    }
    double fScale = fUpperBound - fLowerBound;
    if (bIsCumulative) // cumulative distribution function
    {
        x = (x-fLowerBound)/fScale;  // convert to standard form
        PushDouble(GetBetaDist(x, alpha, beta));
        return;
    }
    else // probability density function
    {
        x = (x-fLowerBound)/fScale;
        PushDouble(GetBetaDistPDF(x, alpha, beta)/fScale);
        return;
    }
}

void ScInterpreter::ScPhi()
{
    PushDouble(phi(GetDouble()));
}

void ScInterpreter::ScGauss()
{
    PushDouble(gauss(GetDouble()));
}

void ScInterpreter::ScFisher()
{
    double fVal = GetDouble();
    if (fabs(fVal) >= 1.0)
        PushIllegalArgument();
    else
        PushDouble( ::rtl::math::atanh( fVal));
}

void ScInterpreter::ScFisherInv()
{
    PushDouble( tanh( GetDouble()));
}

void ScInterpreter::ScFact()
{
    double nVal = GetDouble();
    if (nVal < 0.0)
        PushIllegalArgument();
    else
        PushDouble(Fakultaet(nVal));
}

void ScInterpreter::ScCombin()
{
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        double k = ::rtl::math::approxFloor(GetDouble());
        double n = ::rtl::math::approxFloor(GetDouble());
        if (k < 0.0 || n < 0.0 || k > n)
            PushIllegalArgument();
        else
            PushDouble(BinomKoeff(n, k));
    }
}

void ScInterpreter::ScCombinA()
{
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        double k = ::rtl::math::approxFloor(GetDouble());
        double n = ::rtl::math::approxFloor(GetDouble());
        if (k < 0.0 || n < 0.0 || k > n)
            PushIllegalArgument();
        else
            PushDouble(BinomKoeff(n + k - 1, k));
    }
}

void ScInterpreter::ScPermut()
{
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        double k = ::rtl::math::approxFloor(GetDouble());
        double n = ::rtl::math::approxFloor(GetDouble());
        if (n < 0.0 || k < 0.0 || k > n)
            PushIllegalArgument();
        else if (k == 0.0)
            PushInt(1);     // (n! / (n - 0)!) == 1
        else
        {
            double nVal = n;
            for (sal_uLong i = static_cast<sal_uLong>(k)-1; i >= 1; i--)
                nVal *= n-static_cast<double>(i);
            PushDouble(nVal);
        }
    }
}

void ScInterpreter::ScPermutationA()
{
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        double k = ::rtl::math::approxFloor(GetDouble());
        double n = ::rtl::math::approxFloor(GetDouble());
        if (n < 0.0 || k < 0.0)
            PushIllegalArgument();
        else
            PushDouble(pow(n,k));
    }
}

double ScInterpreter::GetBinomDistPMF(double x, double n, double p)
// used in ScB and ScBinomDist
// preconditions: 0.0 <= x <= n, 0.0 < p < 1.0;  x,n integral although double
{
    double q = (0.5 - p) + 0.5;
    double fFactor = pow(q, n);
    if (fFactor <=::std::numeric_limits<double>::min())
    {
        fFactor = pow(p, n);
        if (fFactor <= ::std::numeric_limits<double>::min())
            return GetBetaDistPDF(p, x+1.0, n-x+1.0)/(n+1.0);
        else
        {
            sal_uInt32 max = static_cast<sal_uInt32>(n - x);
            for (sal_uInt32 i = 0; i < max && fFactor > 0.0; i++)
                fFactor *= (n-i)/(i+1)*q/p;
            return fFactor;
        }
    }
    else
    {
        sal_uInt32 max = static_cast<sal_uInt32>(x);
        for (sal_uInt32 i = 0; i < max && fFactor > 0.0; i++)
            fFactor *= (n-i)/(i+1)*p/q;
        return fFactor;
    }
}

static double lcl_GetBinomDistRange(double n, double xs,double xe,
            double fFactor /* q^n */, double p, double q)
//preconditions: 0.0 <= xs < xe <= n; xs,xe,n integral although double
{
    sal_uInt32 i;
    double fSum;
    // skip summands index 0 to xs-1, start sum with index xs
    sal_uInt32 nXs = static_cast<sal_uInt32>( xs );
    for (i = 1; i <= nXs && fFactor > 0.0; i++)
        fFactor *= (n-i+1)/i * p/q;
    fSum = fFactor; // Summand xs
    sal_uInt32 nXe = static_cast<sal_uInt32>(xe);
    for (i = nXs+1; i <= nXe && fFactor > 0.0; i++)
    {
        fFactor *= (n-i+1)/i * p/q;
        fSum += fFactor;
    }
    return std::min(fSum,1.0);
}

void ScInterpreter::ScB()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 3, 4 ) )
        return ;
    if (nParamCount == 3)   // mass function
    {
        double x = ::rtl::math::approxFloor(GetDouble());
        double p = GetDouble();
        double n = ::rtl::math::approxFloor(GetDouble());
        if (n < 0.0 || x < 0.0 || x > n || p < 0.0 || p > 1.0)
            PushIllegalArgument();
        else if (p == 0.0)
            PushDouble( (x == 0.0) ? 1.0 : 0.0 );
        else if ( p == 1.0)
            PushDouble( (x == n) ? 1.0 : 0.0);
        else
            PushDouble(GetBinomDistPMF(x,n,p));
    }
    else
    {   // nParamCount == 4
        double xe = ::rtl::math::approxFloor(GetDouble());
        double xs = ::rtl::math::approxFloor(GetDouble());
        double p = GetDouble();
        double n = ::rtl::math::approxFloor(GetDouble());
        double q = (0.5 - p) + 0.5;
        bool bIsValidX = ( 0.0 <= xs && xs <= xe && xe <= n);
        if ( bIsValidX && 0.0 < p && p < 1.0)
        {
            if (xs == xe)       // mass function
                PushDouble(GetBinomDistPMF(xs,n,p));
            else
            {
                double fFactor = pow(q, n);
                if (fFactor > ::std::numeric_limits<double>::min())
                    PushDouble(lcl_GetBinomDistRange(n,xs,xe,fFactor,p,q));
                else
                {
                    fFactor = pow(p, n);
                    if (fFactor > ::std::numeric_limits<double>::min())
                    {
                        // sum from j=xs to xe {(n choose j) * p^j * q^(n-j)}
                        // = sum from i = n-xe to n-xs { (n choose i) * q^i * p^(n-i)}
                        PushDouble(lcl_GetBinomDistRange(n,n-xe,n-xs,fFactor,q,p));
                    }
                    else
                        PushDouble(GetBetaDist(q,n-xe,xe+1.0)-GetBetaDist(q,n-xs+1,xs) );
                }
            }
        }
        else
        {
            if ( bIsValidX ) // not(0<p<1)
            {
                if ( p == 0.0 )
                    PushDouble( (xs == 0.0) ? 1.0 : 0.0 );
                else if ( p == 1.0 )
                    PushDouble( (xe == n) ? 1.0 : 0.0 );
                else
                    PushIllegalArgument();
            }
            else
                PushIllegalArgument();
        }
    }
}

void ScInterpreter::ScBinomDist()
{
    if ( MustHaveParamCount( GetByte(), 4 ) )
    {
        bool bIsCum   = GetBool();     // false=mass function; true=cumulative
        double p      = GetDouble();
        double n      = ::rtl::math::approxFloor(GetDouble());
        double x      = ::rtl::math::approxFloor(GetDouble());
        double q = (0.5 - p) + 0.5;           // get one bit more for p near 1.0
        if (n < 0.0 || x < 0.0 || x > n || p < 0.0 || p > 1.0)
        {
            PushIllegalArgument();
            return;
        }
        if ( p == 0.0)
        {
            PushDouble( (x==0.0 || bIsCum) ? 1.0 : 0.0 );
            return;
        }
        if ( p == 1.0)
        {
            PushDouble( (x==n) ? 1.0 : 0.0);
            return;
        }
        if (!bIsCum)
            PushDouble( GetBinomDistPMF(x,n,p));
        else
        {
            if (x == n)
                PushDouble(1.0);
            else
            {
                double fFactor = pow(q, n);
                if (x == 0.0)
                    PushDouble(fFactor);
                else if (fFactor <= ::std::numeric_limits<double>::min())
                {
                    fFactor = pow(p, n);
                    if (fFactor <= ::std::numeric_limits<double>::min())
                        PushDouble(GetBetaDist(q,n-x,x+1.0));
                    else
                    {
                        if (fFactor > fMachEps)
                        {
                            double fSum = 1.0 - fFactor;
                            sal_uInt32 max = static_cast<sal_uInt32> (n - x) - 1;
                            for (sal_uInt32 i = 0; i < max && fFactor > 0.0; i++)
                            {
                                fFactor *= (n-i)/(i+1)*q/p;
                                fSum -= fFactor;
                            }
                            PushDouble( (fSum < 0.0) ? 0.0 : fSum );
                        }
                        else
                            PushDouble(lcl_GetBinomDistRange(n,n-x,n,fFactor,q,p));
                    }
                }
                else
                    PushDouble( lcl_GetBinomDistRange(n,0.0,x,fFactor,p,q)) ;
            }
        }
    }
}

void ScInterpreter::ScCritBinom()
{
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        double alpha  = GetDouble();
        double p      = GetDouble();
        double n      = ::rtl::math::approxFloor(GetDouble());
        if (n < 0.0 || alpha < 0.0 || alpha > 1.0 || p < 0.0 || p > 1.0)
            PushIllegalArgument();
        else if ( alpha == 0.0 )
            PushDouble( 0.0 );
        else if ( alpha == 1.0 )
            PushDouble( p == 0 ? 0.0 : n );
        else
        {
            double fFactor;
            double q = (0.5 - p) + 0.5;           // get one bit more for p near 1.0
            if ( q > p )                          // work from the side where the cumulative curve is
            {
                // work from 0 upwards
                fFactor = pow(q,n);
                if (fFactor > ::std::numeric_limits<double>::min())
                {
                    double fSum = fFactor;
                    sal_uInt32 max = static_cast<sal_uInt32> (n), i;
                    for (i = 0; i < max && fSum < alpha; i++)
                    {
                        fFactor *= (n-i)/(i+1)*p/q;
                        fSum += fFactor;
                    }
                    PushDouble(i);
                }
                else
                {
                    // accumulate BinomDist until accumulated BinomDist reaches alpha
                    double fSum = 0.0;
                    sal_uInt32 max = static_cast<sal_uInt32> (n), i;
                    for (i = 0; i < max && fSum < alpha; i++)
                    {
                        const double x = GetBetaDistPDF( p, ( i + 1 ), ( n - i + 1 ) )/( n + 1 );
                        if ( nGlobalError == FormulaError::NONE )
                        {
                            fSum += x;
                        }
                        else
                        {
                            PushNoValue();
                            return;
                        }
                    }
                    PushDouble( i - 1 );
                }
            }
            else
            {
                // work from n backwards
                fFactor = pow(p, n);
                if (fFactor > ::std::numeric_limits<double>::min())
                {
                    double fSum = 1.0 - fFactor;
                    sal_uInt32 max = static_cast<sal_uInt32> (n), i;
                    for (i = 0; i < max && fSum >= alpha; i++)
                    {
                        fFactor *= (n-i)/(i+1)*q/p;
                        fSum -= fFactor;
                    }
                    PushDouble(n-i);
                }
                else
                {
                    // accumulate BinomDist until accumulated BinomDist reaches alpha
                    double fSum = 0.0;
                    sal_uInt32 max = static_cast<sal_uInt32> (n), i;
                    alpha = 1 - alpha;
                    for (i = 0; i < max && fSum < alpha; i++)
                    {
                        const double x = GetBetaDistPDF( q, ( i + 1 ), ( n - i + 1 ) )/( n + 1 );
                        if ( nGlobalError == FormulaError::NONE )
                        {
                            fSum += x;
                        }
                        else
                        {
                            PushNoValue();
                            return;
                        }
                    }
                    PushDouble( n - i + 1 );
                }
            }
        }
    }
}

void ScInterpreter::ScNegBinomDist()
{
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        double p = GetDouble();                            // probability
        double s = ::rtl::math::approxFloor(GetDouble());  // No of successes
        double f = ::rtl::math::approxFloor(GetDouble());  // No of failures
        if ((f + s) <= 1.0 || p < 0.0 || p > 1.0)
            PushIllegalArgument();
        else
        {
            double q = 1.0 - p;
            double fFactor = pow(p,s);
            for (double i = 0.0; i < f; i++)
                fFactor *= (i+s)/(i+1.0)*q;
            PushDouble(fFactor);
        }
    }
}

void ScInterpreter::ScNegBinomDist_MS()
{
    if ( MustHaveParamCount( GetByte(), 4 ) )
    {
        bool bCumulative = GetBool();
        double p = GetDouble();                            // probability
        double s = ::rtl::math::approxFloor(GetDouble());  // No of successes
        double f = ::rtl::math::approxFloor(GetDouble());  // No of failures
        if ( s < 1.0 || f < 0.0 || p < 0.0 || p > 1.0 )
            PushIllegalArgument();
        else
        {
            double q = 1.0 - p;
            if ( bCumulative )
                PushDouble( 1.0 - GetBetaDist( q, f + 1, s ) );
            else
            {
                double fFactor = pow( p, s );
                for ( double i = 0.0; i < f; i++ )
                    fFactor *= ( i + s ) / ( i + 1.0 ) * q;
                PushDouble( fFactor );
            }
        }
    }
}

void ScInterpreter::ScNormDist( int nMinParamCount )
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, nMinParamCount, 4 ) )
        return;
    bool bCumulative = nParamCount != 4 || GetBool();
    double sigma = GetDouble();                 // standard deviation
    double mue = GetDouble();                   // mean
    double x = GetDouble();                     // x
    if (sigma <= 0.0)
    {
        PushIllegalArgument();
        return;
    }
    if (bCumulative)
        PushDouble(integralPhi((x-mue)/sigma));
    else
        PushDouble(phi((x-mue)/sigma)/sigma);
}

void ScInterpreter::ScLogNormDist( int nMinParamCount ) //expanded, see #i100119# and fdo72158
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, nMinParamCount, 4 ) )
        return;
    bool bCumulative = nParamCount != 4 || GetBool(); // cumulative
    double sigma = nParamCount >= 3 ? GetDouble() : 1.0; // standard deviation
    double mue = nParamCount >= 2 ? GetDouble() : 0.0;   // mean
    double x = GetDouble();                              // x
    if (sigma <= 0.0)
    {
        PushIllegalArgument();
        return;
    }
    if (bCumulative)
    { // cumulative
        if (x <= 0.0)
            PushDouble(0.0);
        else
            PushDouble(integralPhi((log(x)-mue)/sigma));
    }
    else
    { // density
        if (x <= 0.0)
            PushIllegalArgument();
        else
            PushDouble(phi((log(x)-mue)/sigma)/sigma/x);
    }
}

void ScInterpreter::ScStdNormDist()
{
    PushDouble(integralPhi(GetDouble()));
}

void ScInterpreter::ScStdNormDist_MS()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 2 ) )
        return;
    bool bCumulative = GetBool();                        // cumulative
    double x = GetDouble();                              // x

    if ( bCumulative )
        PushDouble( integralPhi( x ) );
    else
        PushDouble( exp( - pow( x, 2 ) / 2 ) / sqrt( 2 * F_PI ) );
}

void ScInterpreter::ScExpDist()
{
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        double kum    = GetDouble();                    // 0 or 1
        double lambda = GetDouble();                    // lambda
        double x      = GetDouble();                    // x
        if (lambda <= 0.0)
            PushIllegalArgument();
        else if (kum == 0.0)                        // density
        {
            if (x >= 0.0)
                PushDouble(lambda * exp(-lambda*x));
            else
                PushInt(0);
        }
        else                                        // distribution
        {
            if (x > 0.0)
                PushDouble(1.0 - exp(-lambda*x));
            else
                PushInt(0);
        }
    }
}

void ScInterpreter::ScTDist()
{
    if ( !MustHaveParamCount( GetByte(), 3 ) )
        return;
    double fFlag = ::rtl::math::approxFloor(GetDouble());
    double fDF   = ::rtl::math::approxFloor(GetDouble());
    double T     = GetDouble();
    if (fDF < 1.0 || T < 0.0 || (fFlag != 1.0 && fFlag != 2.0) )
    {
        PushIllegalArgument();
        return;
    }
    PushDouble( GetTDist( T, fDF, static_cast<int>(fFlag) ) );
}

void ScInterpreter::ScTDist_T( int nTails )
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;
    double fDF = ::rtl::math::approxFloor( GetDouble() );
    double fT  = GetDouble();
    if ( fDF < 1.0 || ( nTails == 2 && fT < 0.0 ) )
    {
        PushIllegalArgument();
        return;
    }
    double fRes = GetTDist( fT, fDF, nTails );
    if ( nTails == 1 && fT < 0.0 )
        PushDouble( 1.0 - fRes ); // tdf#105937, right tail, negative X
    else
        PushDouble( fRes );
}

void ScInterpreter::ScTDist_MS()
{
    if ( !MustHaveParamCount( GetByte(), 3 ) )
        return;
    bool   bCumulative = GetBool();
    double fDF = ::rtl::math::approxFloor( GetDouble() );
    double T   = GetDouble();
    if ( fDF < 1.0 )
    {
        PushIllegalArgument();
        return;
    }
    PushDouble( GetTDist( T, fDF, ( bCumulative ? 4 : 3 ) ) );
}

void ScInterpreter::ScFDist()
{
    if ( !MustHaveParamCount( GetByte(), 3 ) )
        return;
    double fF2 = ::rtl::math::approxFloor(GetDouble());
    double fF1 = ::rtl::math::approxFloor(GetDouble());
    double fF  = GetDouble();
    if (fF < 0.0 || fF1 < 1.0 || fF2 < 1.0 || fF1 >= 1.0E10 || fF2 >= 1.0E10)
    {
        PushIllegalArgument();
        return;
    }
    PushDouble(GetFDist(fF, fF1, fF2));
}

void ScInterpreter::ScFDist_LT()
{
    int nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 3, 4 ) )
        return;
    bool bCum;
    if ( nParamCount == 3 )
        bCum = true;
    else if ( IsMissing() )
    {
        bCum = true;
        Pop();
    }
    else
        bCum = GetBool();
    double fF2 = ::rtl::math::approxFloor( GetDouble() );
    double fF1 = ::rtl::math::approxFloor( GetDouble() );
    double fF  = GetDouble();
    if ( fF < 0.0 || fF1 < 1.0 || fF2 < 1.0 || fF1 >= 1.0E10 || fF2 >= 1.0E10 )
    {
        PushIllegalArgument();
        return;
    }
    if ( bCum )
    {
        // left tail cumulative distribution
        PushDouble( 1.0 - GetFDist( fF, fF1, fF2 ) );
    }
    else
    {
        // probability density function
        PushDouble( pow( fF1 / fF2, fF1 / 2 ) * pow( fF, ( fF1 / 2 ) - 1 ) /
                    ( pow( ( 1 + ( fF * fF1 / fF2 ) ), ( fF1 + fF2 ) / 2 ) *
                      GetBeta( fF1 / 2, fF2 / 2 ) ) );
    }
}

void ScInterpreter::ScChiDist( bool bODFF )
{
    double fResult;
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;
    double fDF  = ::rtl::math::approxFloor(GetDouble());
    double fChi = GetDouble();
    if ( fDF < 1.0 // x<=0 returns 1, see ODFF1.2 6.18.11
       || ( !bODFF && fChi < 0 ) ) // Excel does not accept negative fChi
    {
        PushIllegalArgument();
        return;
    }
    fResult = GetChiDist( fChi, fDF);
    if (nGlobalError != FormulaError::NONE)
    {
        PushError( nGlobalError);
        return;
    }
    PushDouble(fResult);
}

void ScInterpreter::ScWeibull()
{
    if ( MustHaveParamCount( GetByte(), 4 ) )
    {
        double kum   = GetDouble();                 // 0 or 1
        double beta  = GetDouble();                 // beta
        double alpha = GetDouble();                 // alpha
        double x     = GetDouble();                 // x
        if (alpha <= 0.0 || beta <= 0.0 || x < 0.0)
            PushIllegalArgument();
        else if (kum == 0.0)                        // Density
            PushDouble(alpha/pow(beta,alpha)*pow(x,alpha-1.0)*
                       exp(-pow(x/beta,alpha)));
        else                                        // Distribution
            PushDouble(1.0 - exp(-pow(x/beta,alpha)));
    }
}

void ScInterpreter::ScPoissonDist( bool bODFF )
{
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, ( bODFF ? 2 : 3 ), 3 ) )
    {
        bool bCumulative = nParamCount != 3 || GetBool();         // default cumulative
        double lambda    = GetDouble();                           // Mean
        double x         = ::rtl::math::approxFloor(GetDouble()); // discrete distribution
        if (lambda <= 0.0 || x < 0.0)
            PushIllegalArgument();
        else if (!bCumulative)                            // Probability mass function
        {
            if (lambda >712.0)    // underflow in exp(-lambda)
            {   // accuracy 11 Digits
                PushDouble( exp(x*log(lambda)-lambda-GetLogGamma(x+1.0)));
            }
            else
            {
                double fPoissonVar = 1.0;
                for ( double f = 0.0; f < x; ++f )
                    fPoissonVar *= lambda / ( f + 1.0 );
                PushDouble( fPoissonVar * exp( -lambda ) );
            }
        }
        else                                // Cumulative distribution function
        {
            if (lambda > 712.0)  // underflow in exp(-lambda)
            {   // accuracy 12 Digits
                PushDouble(GetUpRegIGamma(x+1.0,lambda));
            }
            else
            {
                if (x >= 936.0) // result is always indistinguishable from 1
                    PushDouble (1.0);
                else
                {
                    double fSummand = exp(-lambda);
                    double fSum = fSummand;
                    int nEnd = sal::static_int_cast<int>( x );
                    for (int i = 1; i <= nEnd; i++)
                    {
                        fSummand = (fSummand * lambda)/static_cast<double>(i);
                        fSum += fSummand;
                    }
                    PushDouble(fSum);
                }
            }
        }
    }
}

/** Local function used in the calculation of the hypergeometric distribution.
 */
static void lcl_PutFactorialElements( ::std::vector< double >& cn, double fLower, double fUpper, double fBase )
{
    for ( double i = fLower; i <= fUpper; ++i )
    {
        double fVal = fBase - i;
        if ( fVal > 1.0 )
            cn.push_back( fVal );
    }
}

/** Calculates a value of the hypergeometric distribution.

    @see #i47296#

    This function has an extra argument bCumulative,
    which only calculates the non-cumulative distribution and
    which is optional in Calc and mandatory with Excel's HYPGEOM.DIST()

    @see fdo#71722
    @see tdf#102948, make Calc function ODFF1.2-compliant
    @see tdf#117041, implement note at bottom of ODFF1.2 par.6.18.37
 */
void ScInterpreter::ScHypGeomDist( int nMinParamCount )
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, nMinParamCount, 5 ) )
        return;

    bool bCumulative = ( nParamCount == 5 && GetBool() );
    double N = ::rtl::math::approxFloor(GetDouble());
    double M = ::rtl::math::approxFloor(GetDouble());
    double n = ::rtl::math::approxFloor(GetDouble());
    double x = ::rtl::math::approxFloor(GetDouble());

    if ( (x < 0.0) || (n < x) || (N < n) || (N < M) || (M < 0.0) )
    {
        PushIllegalArgument();
        return;
    }

    double fVal = 0.0;

    for ( int i = ( bCumulative ? 0 : x ); i <= x && nGlobalError == FormulaError::NONE; i++ )
    {
        if ( (n - i <= N - M) && (i <= M) )
            fVal +=  GetHypGeomDist( i, n, M, N );
    }

    PushDouble( fVal );
}

/** Calculates a value of the hypergeometric distribution.

    The algorithm is designed to avoid unnecessary multiplications and division
    by expanding all factorial elements (9 of them).  It is done by excluding
    those ranges that overlap in the numerator and the denominator.  This allows
    for a fast calculation for large values which would otherwise cause an overflow
    in the intermediate values.

    @see #i47296#
 */
double ScInterpreter::GetHypGeomDist( double x, double n, double M, double N )
{
    const size_t nMaxArraySize = 500000; // arbitrary max array size

    std::vector<double> cnNumer, cnDenom;

    size_t nEstContainerSize = static_cast<size_t>( x + ::std::min( n, M ) );
    size_t nMaxSize = ::std::min( cnNumer.max_size(), nMaxArraySize );
    if ( nEstContainerSize > nMaxSize )
    {
        PushNoValue();
        return 0;
    }
    cnNumer.reserve( nEstContainerSize + 10 );
    cnDenom.reserve( nEstContainerSize + 10 );

    // Trim coefficient C first
    double fCNumVarUpper = N - n - M + x - 1.0;
    double fCDenomVarLower = 1.0;
    if ( N - n - M + x >= M - x + 1.0 )
    {
        fCNumVarUpper = M - x - 1.0;
        fCDenomVarLower = N - n - 2.0*(M - x) + 1.0;
    }

    double fCNumLower = N - n - fCNumVarUpper;
    double fCDenomUpper = N - n - M + x + 1.0 - fCDenomVarLower;

    double fDNumVarLower = n - M;

    if ( n >= M + 1.0 )
    {
        if ( N - M < n + 1.0 )
        {
            // Case 1

            if ( N - n < n + 1.0 )
            {
                // no overlap
                lcl_PutFactorialElements( cnNumer, 0.0, fCNumVarUpper, N - n );
                lcl_PutFactorialElements( cnDenom, 0.0, N - n - 1.0, N );
            }
            else
            {
                // overlap
                OSL_ENSURE( fCNumLower < n + 1.0, "ScHypGeomDist: wrong assertion" );
                lcl_PutFactorialElements( cnNumer, N - 2.0*n, fCNumVarUpper, N - n );
                lcl_PutFactorialElements( cnDenom, 0.0, n - 1.0, N );
            }

            OSL_ENSURE( fCDenomUpper <= N - M, "ScHypGeomDist: wrong assertion" );

            if ( fCDenomUpper < n - x + 1.0 )
                // no overlap
                lcl_PutFactorialElements( cnNumer, 1.0, N - M - n + x, N - M + 1.0 );
            else
            {
                // overlap
                lcl_PutFactorialElements( cnNumer, 1.0, N - M - fCDenomUpper, N - M + 1.0 );

                fCDenomUpper = n - x;
                fCDenomVarLower = N - M - 2.0*(n - x) + 1.0;
            }
        }
        else
        {
            // Case 2

            if ( n > M - 1.0 )
            {
                // no overlap
                lcl_PutFactorialElements( cnNumer, 0.0, fCNumVarUpper, N - n );
                lcl_PutFactorialElements( cnDenom, 0.0, M - 1.0, N );
            }
            else
            {
                lcl_PutFactorialElements( cnNumer, M - n, fCNumVarUpper, N - n );
                lcl_PutFactorialElements( cnDenom, 0.0, n - 1.0, N );
            }

            OSL_ENSURE( fCDenomUpper <= n, "ScHypGeomDist: wrong assertion" );

            if ( fCDenomUpper < n - x + 1.0 )
                // no overlap
                lcl_PutFactorialElements( cnNumer, N - M - n + 1.0, N - M - n + x, N - M + 1.0 );
            else
            {
                lcl_PutFactorialElements( cnNumer, N - M - n + 1.0, N - M - fCDenomUpper, N - M + 1.0 );
                fCDenomUpper = n - x;
                fCDenomVarLower = N - M - 2.0*(n - x) + 1.0;
            }
        }

        OSL_ENSURE( fCDenomUpper <= M, "ScHypGeomDist: wrong assertion" );
    }
    else
    {
        if ( N - M < M + 1.0 )
        {
            // Case 3

            if ( N - n < M + 1.0 )
            {
                // No overlap
                lcl_PutFactorialElements( cnNumer, 0.0, fCNumVarUpper, N - n );
                lcl_PutFactorialElements( cnDenom, 0.0, N - M - 1.0, N );
            }
            else
            {
                lcl_PutFactorialElements( cnNumer, N - n - M, fCNumVarUpper, N - n );
                lcl_PutFactorialElements( cnDenom, 0.0, n - 1.0, N );
            }

            if ( n - x + 1.0 > fCDenomUpper )
                // No overlap
                lcl_PutFactorialElements( cnNumer, 1.0, N - M - n + x, N - M + 1.0 );
            else
            {
                // Overlap
                lcl_PutFactorialElements( cnNumer, 1.0, N - M - fCDenomUpper, N - M + 1.0 );

                fCDenomVarLower = N - M - 2.0*(n - x) + 1.0;
                fCDenomUpper = n - x;
            }
        }
        else
        {
            // Case 4

            OSL_ENSURE( M >= n - x, "ScHypGeomDist: wrong assertion" );
            OSL_ENSURE( M - x <= N - M + 1.0, "ScHypGeomDist: wrong assertion" );

            if ( N - n < N - M + 1.0 )
            {
                // No overlap
                lcl_PutFactorialElements( cnNumer, 0.0, fCNumVarUpper, N - n );
                lcl_PutFactorialElements( cnDenom, 0.0, M - 1.0, N );
            }
            else
            {
                // Overlap
                OSL_ENSURE( fCNumLower <= N - M + 1.0, "ScHypGeomDist: wrong assertion" );
                lcl_PutFactorialElements( cnNumer, M - n, fCNumVarUpper, N - n );
                lcl_PutFactorialElements( cnDenom, 0.0, n - 1.0, N );
            }

            if ( n - x + 1.0 > fCDenomUpper )
                // No overlap
                lcl_PutFactorialElements( cnNumer, N - 2.0*M + 1.0, N - M - n + x, N - M + 1.0 );
            else if ( M >= fCDenomUpper )
            {
                lcl_PutFactorialElements( cnNumer, N - 2.0*M + 1.0, N - M - fCDenomUpper, N - M + 1.0 );

                fCDenomUpper = n - x;
                fCDenomVarLower = N - M - 2.0*(n - x) + 1.0;
            }
            else
            {
                OSL_ENSURE( M <= fCDenomUpper, "ScHypGeomDist: wrong assertion" );
                lcl_PutFactorialElements( cnDenom, fCDenomVarLower, N - n - 2.0*M + x,
                        N - n - M + x + 1.0 );

                fCDenomUpper = n - x;
                fCDenomVarLower = N - M - 2.0*(n - x) + 1.0;
            }
        }

        OSL_ENSURE( fCDenomUpper <= n, "ScHypGeomDist: wrong assertion" );

        fDNumVarLower = 0.0;
    }

    double nDNumVarUpper   = fCDenomUpper < x + 1.0 ? n - x - 1.0     : n - fCDenomUpper - 1.0;
    double nDDenomVarLower = fCDenomUpper < x + 1.0 ? fCDenomVarLower : N - n - M + 1.0;
    lcl_PutFactorialElements( cnNumer, fDNumVarLower, nDNumVarUpper, n );
    lcl_PutFactorialElements( cnDenom, nDDenomVarLower, N - n - M + x, N - n - M + x + 1.0 );

    ::std::sort( cnNumer.begin(), cnNumer.end() );
    ::std::sort( cnDenom.begin(), cnDenom.end() );
    auto it1 = cnNumer.rbegin(), it1End = cnNumer.rend();
    auto it2 = cnDenom.rbegin(), it2End = cnDenom.rend();

    double fFactor = 1.0;
    for ( ; it1 != it1End || it2 != it2End; )
    {
        double fEnum = 1.0, fDenom = 1.0;
        if ( it1 != it1End )
            fEnum  = *it1++;
        if ( it2 != it2End )
            fDenom = *it2++;
        fFactor *= fEnum / fDenom;
    }

    return fFactor;
}

void ScInterpreter::ScGammaDist( bool bODFF )
{
    sal_uInt8 nMinParamCount = ( bODFF ? 3 : 4 );
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, nMinParamCount, 4 ) )
        return;
    bool bCumulative;
    if (nParamCount == 4)
        bCumulative = GetBool();
    else
        bCumulative = true;
    double fBeta = GetDouble();                 // scale
    double fAlpha = GetDouble();                // shape
    double fX = GetDouble();                    // x
    if ((!bODFF && fX < 0) || fAlpha <= 0.0 || fBeta <= 0.0)
        PushIllegalArgument();
    else
    {
        if (bCumulative)                        // distribution
            PushDouble( GetGammaDist( fX, fAlpha, fBeta));
        else                                    // density
            PushDouble( GetGammaDistPDF( fX, fAlpha, fBeta));
    }
}

void ScInterpreter::ScNormInv()
{
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        double sigma = GetDouble();
        double mue   = GetDouble();
        double x     = GetDouble();
        if (sigma <= 0.0 || x < 0.0 || x > 1.0)
            PushIllegalArgument();
        else if (x == 0.0 || x == 1.0)
            PushNoValue();
        else
            PushDouble(gaussinv(x)*sigma + mue);
    }
}

void ScInterpreter::ScSNormInv()
{
    double x = GetDouble();
    if (x < 0.0 || x > 1.0)
        PushIllegalArgument();
    else if (x == 0.0 || x == 1.0)
        PushNoValue();
    else
        PushDouble(gaussinv(x));
}

void ScInterpreter::ScLogNormInv()
{
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 1, 3 ) )
    {
        double fSigma = ( nParamCount == 3 ? GetDouble() : 1.0 );  // Stddev
        double fMue = ( nParamCount >= 2 ? GetDouble() : 0.0 );    // Mean
        double fP = GetDouble();                                   // p
        if ( fSigma <= 0.0 || fP <= 0.0 || fP >= 1.0 )
            PushIllegalArgument();
        else
            PushDouble( exp( fMue + fSigma * gaussinv( fP ) ) );
    }
}

class ScGammaDistFunction : public ScDistFunc
{
    ScInterpreter&  rInt;
    double          fp, fAlpha, fBeta;

public:
            ScGammaDistFunction( ScInterpreter& rI, double fpVal, double fAlphaVal, double fBetaVal ) :
                rInt(rI), fp(fpVal), fAlpha(fAlphaVal), fBeta(fBetaVal) {}

    virtual ~ScGammaDistFunction() {}

    double  GetValue( double x ) const override  { return fp - rInt.GetGammaDist(x, fAlpha, fBeta); }
};

void ScInterpreter::ScGammaInv()
{
    if ( !MustHaveParamCount( GetByte(), 3 ) )
        return;
    double fBeta  = GetDouble();
    double fAlpha = GetDouble();
    double fP = GetDouble();
    if (fAlpha <= 0.0 || fBeta <= 0.0 || fP < 0.0 || fP >= 1.0 )
    {
        PushIllegalArgument();
        return;
    }
    if (fP == 0.0)
        PushInt(0);
    else
    {
        bool bConvError;
        ScGammaDistFunction aFunc( *this, fP, fAlpha, fBeta );
        double fStart = fAlpha * fBeta;
        double fVal = lcl_IterateInverse( aFunc, fStart*0.5, fStart, bConvError );
        if (bConvError)
            SetError(FormulaError::NoConvergence);
        PushDouble(fVal);
    }
}

class ScBetaDistFunction : public ScDistFunc
{
    ScInterpreter&  rInt;
    double          fp, fAlpha, fBeta;

public:
            ScBetaDistFunction( ScInterpreter& rI, double fpVal, double fAlphaVal, double fBetaVal ) :
                rInt(rI), fp(fpVal), fAlpha(fAlphaVal), fBeta(fBetaVal) {}

    virtual ~ScBetaDistFunction() {}

    double  GetValue( double x ) const override  { return fp - rInt.GetBetaDist(x, fAlpha, fBeta); }
};

void ScInterpreter::ScBetaInv()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 3, 5 ) )
        return;
    double fP, fA, fB, fAlpha, fBeta;
    if (nParamCount == 5)
        fB = GetDouble();
    else
        fB = 1.0;
    if (nParamCount >= 4)
        fA = GetDouble();
    else
        fA = 0.0;
    fBeta  = GetDouble();
    fAlpha = GetDouble();
    fP     = GetDouble();
    if (fP < 0.0 || fP > 1.0 || fA >= fB || fAlpha <= 0.0 || fBeta <= 0.0)
    {
        PushIllegalArgument();
        return;
    }

    bool bConvError;
    ScBetaDistFunction aFunc( *this, fP, fAlpha, fBeta );
    // 0..1 as range for iteration so it isn't extended beyond the valid range
    double fVal = lcl_IterateInverse( aFunc, 0.0, 1.0, bConvError );
    if (bConvError)
        PushError( FormulaError::NoConvergence);
    else
        PushDouble(fA + fVal*(fB-fA));                  // scale to (A,B)
}

// Note: T, F, and Chi are
// monotonically decreasing,
// therefore 1-Dist as function

class ScTDistFunction : public ScDistFunc
{
    ScInterpreter&  rInt;
    double          fp, fDF;
    int const       nT;

public:
            ScTDistFunction( ScInterpreter& rI, double fpVal, double fDFVal, int nType ) :
                rInt( rI ), fp( fpVal ), fDF( fDFVal ), nT( nType ) {}

    virtual ~ScTDistFunction() {}

    double  GetValue( double x ) const override  { return fp - rInt.GetTDist( x, fDF, nT ); }
};

void ScInterpreter::ScTInv( int nType )
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;
    double fDF  = ::rtl::math::approxFloor(GetDouble());
    double fP = GetDouble();
    if (fDF < 1.0 || fP <= 0.0 || fP > 1.0 )
    {
        PushIllegalArgument();
        return;
    }
    if ( nType == 4 ) // left-tailed cumulative t-distribution
    {
        if ( fP == 1.0 )
            PushIllegalArgument();
        else if ( fP < 0.5 )
            PushDouble( -GetTInv( 1 - fP, fDF, nType ) );
        else
            PushDouble( GetTInv( fP, fDF, nType ) );
    }
    else
        PushDouble( GetTInv( fP, fDF, nType ) );
};

double ScInterpreter::GetTInv( double fAlpha, double fSize, int nType )
{
    bool bConvError;
    ScTDistFunction aFunc( *this, fAlpha, fSize, nType );
    double fVal = lcl_IterateInverse( aFunc, fSize * 0.5, fSize, bConvError );
    if (bConvError)
        SetError(FormulaError::NoConvergence);
    return fVal;
}

class ScFDistFunction : public ScDistFunc
{
    ScInterpreter&  rInt;
    double          fp, fF1, fF2;

public:
            ScFDistFunction( ScInterpreter& rI, double fpVal, double fF1Val, double fF2Val ) :
                rInt(rI), fp(fpVal), fF1(fF1Val), fF2(fF2Val) {}

    virtual ~ScFDistFunction() {}

    double  GetValue( double x ) const override  { return fp - rInt.GetFDist(x, fF1, fF2); }
};

void ScInterpreter::ScFInv()
{
    if ( !MustHaveParamCount( GetByte(), 3 ) )
        return;
    double fF2 = ::rtl::math::approxFloor(GetDouble());
    double fF1 = ::rtl::math::approxFloor(GetDouble());
    double fP  = GetDouble();
    if (fP <= 0.0 || fF1 < 1.0 || fF2 < 1.0 || fF1 >= 1.0E10 || fF2 >= 1.0E10 || fP > 1.0)
    {
        PushIllegalArgument();
        return;
    }

    bool bConvError;
    ScFDistFunction aFunc( *this, fP, fF1, fF2 );
    double fVal = lcl_IterateInverse( aFunc, fF1*0.5, fF1, bConvError );
    if (bConvError)
        SetError(FormulaError::NoConvergence);
    PushDouble(fVal);
}

void ScInterpreter::ScFInv_LT()
{
    if ( !MustHaveParamCount( GetByte(), 3 ) )
        return;
    double fF2 = ::rtl::math::approxFloor(GetDouble());
    double fF1 = ::rtl::math::approxFloor(GetDouble());
    double fP  = GetDouble();
    if (fP <= 0.0 || fF1 < 1.0 || fF2 < 1.0 || fF1 >= 1.0E10 || fF2 >= 1.0E10 || fP > 1.0)
    {
        PushIllegalArgument();
        return;
    }

    bool bConvError;
    ScFDistFunction aFunc( *this, ( 1.0 - fP ), fF1, fF2 );
    double fVal = lcl_IterateInverse( aFunc, fF1*0.5, fF1, bConvError );
    if (bConvError)
        SetError(FormulaError::NoConvergence);
    PushDouble(fVal);
}

class ScChiDistFunction : public ScDistFunc
{
    ScInterpreter&  rInt;
    double          fp, fDF;

public:
            ScChiDistFunction( ScInterpreter& rI, double fpVal, double fDFVal ) :
                rInt(rI), fp(fpVal), fDF(fDFVal) {}

    virtual ~ScChiDistFunction() {}

    double  GetValue( double x ) const override  { return fp - rInt.GetChiDist(x, fDF); }
};

void ScInterpreter::ScChiInv()
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;
    double fDF  = ::rtl::math::approxFloor(GetDouble());
    double fP = GetDouble();
    if (fDF < 1.0 || fP <= 0.0 || fP > 1.0 )
    {
        PushIllegalArgument();
        return;
    }

    bool bConvError;
    ScChiDistFunction aFunc( *this, fP, fDF );
    double fVal = lcl_IterateInverse( aFunc, fDF*0.5, fDF, bConvError );
    if (bConvError)
        SetError(FormulaError::NoConvergence);
    PushDouble(fVal);
}

/***********************************************/
class ScChiSqDistFunction : public ScDistFunc
{
    ScInterpreter&  rInt;
    double          fp, fDF;

public:
            ScChiSqDistFunction( ScInterpreter& rI, double fpVal, double fDFVal ) :
                rInt(rI), fp(fpVal), fDF(fDFVal) {}

    virtual ~ScChiSqDistFunction() {}

    double  GetValue( double x ) const override  { return fp - rInt.GetChiSqDistCDF(x, fDF); }
};

void ScInterpreter::ScChiSqInv()
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;
    double fDF  = ::rtl::math::approxFloor(GetDouble());
    double fP = GetDouble();
    if (fDF < 1.0 || fP < 0.0 || fP >= 1.0 )
    {
        PushIllegalArgument();
        return;
    }

    bool bConvError;
    ScChiSqDistFunction aFunc( *this, fP, fDF );
    double fVal = lcl_IterateInverse( aFunc, fDF*0.5, fDF, bConvError );
    if (bConvError)
        SetError(FormulaError::NoConvergence);
    PushDouble(fVal);
}

void ScInterpreter::ScConfidence()
{
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        double n     = ::rtl::math::approxFloor(GetDouble());
        double sigma = GetDouble();
        double alpha = GetDouble();
        if (sigma <= 0.0 || alpha <= 0.0 || alpha >= 1.0 || n < 1.0)
            PushIllegalArgument();
        else
            PushDouble( gaussinv(1.0-alpha/2.0) * sigma/sqrt(n) );
    }
}

void ScInterpreter::ScConfidenceT()
{
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        double n     = ::rtl::math::approxFloor(GetDouble());
        double sigma = GetDouble();
        double alpha = GetDouble();
        if (sigma <= 0.0 || alpha <= 0.0 || alpha >= 1.0 || n < 1.0)
            PushIllegalArgument();
        else if (n == 1.0) // for interoperability with Excel
            PushError(FormulaError::DivisionByZero);
        else
            PushDouble( sigma * GetTInv( alpha, n - 1, 2 ) / sqrt( n ) );
    }
}

void ScInterpreter::ScZTest()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 2, 3 ) )
        return;
    double sigma = 0.0, x;
    if (nParamCount == 3)
    {
        sigma = GetDouble();
        if (sigma <= 0.0)
        {
            PushIllegalArgument();
            return;
        }
    }
    x = GetDouble();

    double fSum    = 0.0;
    double fSumSqr = 0.0;
    double fVal;
    double rValCount = 0.0;
    switch (GetStackType())
    {
        case svDouble :
        {
            fVal = GetDouble();
            fSum    += fVal;
            fSumSqr += fVal*fVal;
            rValCount++;
        }
        break;
        case svSingleRef :
        {
            ScAddress aAdr;
            PopSingleRef( aAdr );
            ScRefCellValue aCell(*pDok, aAdr);
            if (aCell.hasNumeric())
            {
                fVal = GetCellValue(aAdr, aCell);
                fSum += fVal;
                fSumSqr += fVal*fVal;
                rValCount++;
            }
        }
        break;
        case svRefList :
        case svDoubleRef :
        {
            short nParam = 1;
            size_t nRefInList = 0;
            while (nParam-- > 0)
            {
                ScRange aRange;
                FormulaError nErr = FormulaError::NONE;
                PopDoubleRef( aRange, nParam, nRefInList);
                ScValueIterator aValIter( pDok, aRange, mnSubTotalFlags );
                if (aValIter.GetFirst(fVal, nErr))
                {
                    fSum += fVal;
                    fSumSqr += fVal*fVal;
                    rValCount++;
                    while ((nErr == FormulaError::NONE) && aValIter.GetNext(fVal, nErr))
                    {
                        fSum += fVal;
                        fSumSqr += fVal*fVal;
                        rValCount++;
                    }
                    SetError(nErr);
                }
            }
        }
        break;
        case svMatrix :
        case svExternalSingleRef:
        case svExternalDoubleRef:
        {
            ScMatrixRef pMat = GetMatrix();
            if (pMat)
            {
                SCSIZE nCount = pMat->GetElementCount();
                if (pMat->IsNumeric())
                {
                    for ( SCSIZE i = 0; i < nCount; i++ )
                    {
                        fVal= pMat->GetDouble(i);
                        fSum += fVal;
                        fSumSqr += fVal * fVal;
                        rValCount++;
                    }
                }
                else
                {
                    for (SCSIZE i = 0; i < nCount; i++)
                        if (!pMat->IsStringOrEmpty(i))
                        {
                            fVal= pMat->GetDouble(i);
                            fSum += fVal;
                            fSumSqr += fVal * fVal;
                            rValCount++;
                        }
                }
            }
        }
        break;
        default : SetError(FormulaError::IllegalParameter); break;
    }
    if (rValCount <= 1.0)
        PushError( FormulaError::DivisionByZero);
    else
    {
        double mue = fSum/rValCount;

        if (nParamCount != 3)
        {
            sigma = (fSumSqr - fSum*fSum/rValCount)/(rValCount-1.0);
            if (sigma == 0.0)
            {
                PushError(FormulaError::DivisionByZero);
                return;
            }
            PushDouble(0.5 - gauss((mue-x)/sqrt(sigma/rValCount)));
        }
        else
            PushDouble(0.5 - gauss((mue-x)*sqrt(rValCount)/sigma));
    }
}

bool ScInterpreter::CalculateTest(bool _bTemplin
                                  ,const SCSIZE nC1, const SCSIZE nC2,const SCSIZE nR1,const SCSIZE nR2
                                  ,const ScMatrixRef& pMat1,const ScMatrixRef& pMat2
                                  ,double& fT,double& fF)
{
    double fCount1  = 0.0;
    double fCount2  = 0.0;
    double fSum1    = 0.0;
    double fSumSqr1 = 0.0;
    double fSum2    = 0.0;
    double fSumSqr2 = 0.0;
    double fVal;
    SCSIZE i,j;
    for (i = 0; i < nC1; i++)
        for (j = 0; j < nR1; j++)
        {
            if (!pMat1->IsStringOrEmpty(i,j))
            {
                fVal = pMat1->GetDouble(i,j);
                fSum1    += fVal;
                fSumSqr1 += fVal * fVal;
                fCount1++;
            }
        }
    for (i = 0; i < nC2; i++)
        for (j = 0; j < nR2; j++)
        {
            if (!pMat2->IsStringOrEmpty(i,j))
            {
                fVal = pMat2->GetDouble(i,j);
                fSum2    += fVal;
                fSumSqr2 += fVal * fVal;
                fCount2++;
            }
        }
    if (fCount1 < 2.0 || fCount2 < 2.0)
    {
        PushNoValue();
        return false;
    } // if (fCount1 < 2.0 || fCount2 < 2.0)
    if ( _bTemplin )
    {
        double fS1 = (fSumSqr1-fSum1*fSum1/fCount1)/(fCount1-1.0)/fCount1;
        double fS2 = (fSumSqr2-fSum2*fSum2/fCount2)/(fCount2-1.0)/fCount2;
        if (fS1 + fS2 == 0.0)
        {
            PushNoValue();
            return false;
        }
        fT = fabs(fSum1/fCount1 - fSum2/fCount2)/sqrt(fS1+fS2);
        double c = fS1/(fS1+fS2);
    //  GetTDist is calculated via GetBetaDist and also works with non-integral
    // degrees of freedom. The result matches Excel
        fF = 1.0/(c*c/(fCount1-1.0)+(1.0-c)*(1.0-c)/(fCount2-1.0));
    }
    else
    {
        //  according to Bronstein-Semendjajew
        double fS1 = (fSumSqr1 - fSum1*fSum1/fCount1) / (fCount1 - 1.0);    // Variance
        double fS2 = (fSumSqr2 - fSum2*fSum2/fCount2) / (fCount2 - 1.0);
        fT = fabs( fSum1/fCount1 - fSum2/fCount2 ) /
             sqrt( (fCount1-1.0)*fS1 + (fCount2-1.0)*fS2 ) *
             sqrt( fCount1*fCount2*(fCount1+fCount2-2)/(fCount1+fCount2) );
        fF = fCount1 + fCount2 - 2;
    }
    return true;
}
void ScInterpreter::ScTTest()
{
    if ( !MustHaveParamCount( GetByte(), 4 ) )
        return;
    double fTyp   = ::rtl::math::approxFloor(GetDouble());
    double fTails = ::rtl::math::approxFloor(GetDouble());
    if (fTails != 1.0 && fTails != 2.0)
    {
        PushIllegalArgument();
        return;
    }

    ScMatrixRef pMat2 = GetMatrix();
    ScMatrixRef pMat1 = GetMatrix();
    if (!pMat1 || !pMat2)
    {
        PushIllegalParameter();
        return;
    }
    double fT, fF;
    SCSIZE nC1, nC2;
    SCSIZE nR1, nR2;
    SCSIZE i, j;
    pMat1->GetDimensions(nC1, nR1);
    pMat2->GetDimensions(nC2, nR2);
    if (fTyp == 1.0)
    {
        if (nC1 != nC2 || nR1 != nR2)
        {
            PushIllegalArgument();
            return;
        }
        double fCount   = 0.0;
        double fSum1    = 0.0;
        double fSum2    = 0.0;
        double fSumSqrD = 0.0;
        double fVal1, fVal2;
        for (i = 0; i < nC1; i++)
            for (j = 0; j < nR1; j++)
            {
                if (!pMat1->IsStringOrEmpty(i,j) && !pMat2->IsStringOrEmpty(i,j))
                {
                    fVal1 = pMat1->GetDouble(i,j);
                    fVal2 = pMat2->GetDouble(i,j);
                    fSum1    += fVal1;
                    fSum2    += fVal2;
                    fSumSqrD += (fVal1 - fVal2)*(fVal1 - fVal2);
                    fCount++;
                }
            }
        if (fCount < 1.0)
        {
            PushNoValue();
            return;
        }
        double fSumD = fSum1 - fSum2;
        double fDivider = fCount*fSumSqrD - fSumD*fSumD;
        if ( fDivider == 0.0 )
        {
            PushError(FormulaError::DivisionByZero);
            return;
        }
        fT = fabs(fSumD) * sqrt((fCount-1.0) / fDivider);
        fF = fCount - 1.0;
    }
    else if (fTyp == 2.0)
    {
        if (!CalculateTest(false,nC1, nC2,nR1, nR2,pMat1,pMat2,fT,fF))
            return;     // error was pushed
    }
    else if (fTyp == 3.0)
    {
        if (!CalculateTest(true,nC1, nC2,nR1, nR2,pMat1,pMat2,fT,fF))
            return;     // error was pushed
    }
    else
    {
        PushIllegalArgument();
        return;
    }
    PushDouble( GetTDist( fT, fF, static_cast<int>(fTails) ) );
}

void ScInterpreter::ScFTest()
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;
    ScMatrixRef pMat2 = GetMatrix();
    ScMatrixRef pMat1 = GetMatrix();
    if (!pMat1 || !pMat2)
    {
        PushIllegalParameter();
        return;
    }
    SCSIZE nC1, nC2;
    SCSIZE nR1, nR2;
    pMat1->GetDimensions(nC1, nR1);
    pMat2->GetDimensions(nC2, nR2);
    double fCount1  = 0.0;
    double fCount2  = 0.0;
    double fSum1    = 0.0;
    double fSumSqr1 = 0.0;
    double fSum2    = 0.0;
    double fSumSqr2 = 0.0;

    std::vector<sc::op::Op> aOp;
    aOp.emplace_back(sc::op::Op(0.0, [](double& rAccum, double fVal){rAccum += fVal;}));
    aOp.emplace_back(sc::op::Op(0.0, [](double& rAccum, double fVal){rAccum += fVal * fVal;}));

    auto aVal1 = pMat1->Collect(aOp);
    fSum1 = aVal1[0].mfFirst + aVal1[0].mfRest;
    fSumSqr1 = aVal1[1].mfFirst + aVal1[1].mfRest;
    fCount1 = aVal1[2].mnCount;

    auto aVal2 = pMat2->Collect(aOp);
    fSum2 = aVal2[0].mfFirst + aVal2[0].mfRest;
    fSumSqr2 = aVal2[1].mfFirst + aVal2[1].mfRest;
    fCount2 = aVal2[2].mnCount;

    if (fCount1 < 2.0 || fCount2 < 2.0)
    {
        PushNoValue();
        return;
    }
    double fS1 = (fSumSqr1-fSum1*fSum1/fCount1)/(fCount1-1.0);
    double fS2 = (fSumSqr2-fSum2*fSum2/fCount2)/(fCount2-1.0);
    if (fS1 == 0.0 || fS2 == 0.0)
    {
        PushNoValue();
        return;
    }
    double fF, fF1, fF2;
    if (fS1 > fS2)
    {
        fF = fS1/fS2;
        fF1 = fCount1-1.0;
        fF2 = fCount2-1.0;
    }
    else
    {
        fF = fS2/fS1;
        fF1 = fCount2-1.0;
        fF2 = fCount1-1.0;
    }
    double fFcdf = GetFDist(fF, fF1, fF2);
    PushDouble(2.0*std::min(fFcdf, 1.0 - fFcdf));
}

void ScInterpreter::ScChiTest()
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;
    ScMatrixRef pMat2 = GetMatrix();
    ScMatrixRef pMat1 = GetMatrix();
    if (!pMat1 || !pMat2)
    {
        PushIllegalParameter();
        return;
    }
    SCSIZE nC1, nC2;
    SCSIZE nR1, nR2;
    pMat1->GetDimensions(nC1, nR1);
    pMat2->GetDimensions(nC2, nR2);
    if (nR1 != nR2 || nC1 != nC2)
    {
        PushIllegalArgument();
        return;
    }
    double fChi = 0.0;
    bool bEmpty = true;
    for (SCSIZE i = 0; i < nC1; i++)
    {
        for (SCSIZE j = 0; j < nR1; j++)
        {
            if (!(pMat1->IsEmpty(i,j) || pMat2->IsEmpty(i,j)))
            {
                bEmpty = false;
                if (!pMat1->IsStringOrEmpty(i,j) && !pMat2->IsStringOrEmpty(i,j))
                {
                    double fValX = pMat1->GetDouble(i,j);
                    double fValE = pMat2->GetDouble(i,j);
                    if ( fValE == 0.0 )
                    {
                        PushError(FormulaError::DivisionByZero);
                        return;
                    }
                    // These fTemp values guard against a failure when compiled
                    // with optimization (using g++ 4.8.2 on tinderbox 71-TDF),
                    // where ((fValX - fValE) * (fValX - fValE)) with
                    // fValE==1e+308 should had produced Infinity but did
                    // not, instead the result of divide() then was 1e+308.
                    volatile double fTemp1 = (fValX - fValE) * (fValX - fValE);
                    double fTemp2 = fTemp1;
                    fChi += sc::divide( fTemp2, fValE);
                }
                else
                {
                    PushIllegalArgument();
                    return;
                }
            }
        }
    }
    if ( bEmpty )
    {
        // not in ODFF1.2, but for interoperability with Excel
        PushIllegalArgument();
        return;
    }
    double fDF;
    if (nC1 == 1 || nR1 == 1)
    {
        fDF = static_cast<double>(nC1*nR1 - 1);
        if (fDF == 0.0)
        {
            PushNoValue();
            return;
        }
    }
    else
        fDF = static_cast<double>(nC1-1)*static_cast<double>(nR1-1);
    PushDouble(GetChiDist(fChi, fDF));
}

void ScInterpreter::ScKurt()
{
    double fSum,fCount,vSum;
    std::vector<double> values;
    if ( !CalculateSkew(fSum,fCount,vSum,values) )
        return;

    // ODF 1.2 constraints: # of numbers >= 4
    if (fCount < 4.0)
    {
        // for interoperability with Excel
        PushError( FormulaError::DivisionByZero);
        return;
    }

    double fMean = fSum / fCount;

    for (double v : values)
        vSum += (v - fMean) * (v - fMean);

    double fStdDev = sqrt(vSum / (fCount - 1.0));
    double xpower4 = 0.0;

    if (fStdDev == 0.0)
    {
        PushError( FormulaError::DivisionByZero);
        return;
    }

    for (double v : values)
    {
        double dx = (v - fMean) / fStdDev;
        xpower4 = xpower4 + (dx * dx * dx * dx);
    }

    double k_d = (fCount - 2.0) * (fCount - 3.0);
    double k_l = fCount * (fCount + 1.0) / ((fCount - 1.0) * k_d);
    double k_t = 3.0 * (fCount - 1.0) * (fCount - 1.0) / k_d;

    PushDouble(xpower4 * k_l - k_t);
}

void ScInterpreter::ScHarMean()
{
    short nParamCount = GetByte();
    double nVal = 0.0;
    double nValCount = 0.0;
    ScAddress aAdr;
    ScRange aRange;
    size_t nRefInList = 0;
    while ((nGlobalError == FormulaError::NONE) && (nParamCount-- > 0))
    {
        switch (GetStackType())
        {
            case svDouble    :
            {
                double x = GetDouble();
                if (x > 0.0)
                {
                    nVal += 1.0/x;
                    nValCount++;
                }
                else
                    SetError( FormulaError::IllegalArgument);
                break;
            }
            case svSingleRef :
            {
                PopSingleRef( aAdr );
                ScRefCellValue aCell(*pDok, aAdr);
                if (aCell.hasNumeric())
                {
                    double x = GetCellValue(aAdr, aCell);
                    if (x > 0.0)
                    {
                        nVal += 1.0/x;
                        nValCount++;
                    }
                    else
                        SetError( FormulaError::IllegalArgument);
                }
                break;
            }
            case svDoubleRef :
            case svRefList :
            {
                FormulaError nErr = FormulaError::NONE;
                PopDoubleRef( aRange, nParamCount, nRefInList);
                double nCellVal;
                ScValueIterator aValIter( pDok, aRange, mnSubTotalFlags );
                if (aValIter.GetFirst(nCellVal, nErr))
                {
                    if (nCellVal > 0.0)
                    {
                        nVal += 1.0/nCellVal;
                        nValCount++;
                    }
                    else
                        SetError( FormulaError::IllegalArgument);
                    SetError(nErr);
                    while ((nErr == FormulaError::NONE) && aValIter.GetNext(nCellVal, nErr))
                    {
                        if (nCellVal > 0.0)
                        {
                            nVal += 1.0/nCellVal;
                            nValCount++;
                        }
                        else
                            SetError( FormulaError::IllegalArgument);
                    }
                    SetError(nErr);
                }
            }
            break;
            case svMatrix :
            case svExternalSingleRef:
            case svExternalDoubleRef:
            {
                ScMatrixRef pMat = GetMatrix();
                if (pMat)
                {
                    SCSIZE nCount = pMat->GetElementCount();
                    if (pMat->IsNumeric())
                    {
                        for (SCSIZE nElem = 0; nElem < nCount; nElem++)
                        {
                            double x = pMat->GetDouble(nElem);
                            if (x > 0.0)
                            {
                                nVal += 1.0/x;
                                nValCount++;
                            }
                            else
                                SetError( FormulaError::IllegalArgument);
                        }
                    }
                    else
                    {
                        for (SCSIZE nElem = 0; nElem < nCount; nElem++)
                            if (!pMat->IsStringOrEmpty(nElem))
                            {
                                double x = pMat->GetDouble(nElem);
                                if (x > 0.0)
                                {
                                    nVal += 1.0/x;
                                    nValCount++;
                                }
                                else
                                    SetError( FormulaError::IllegalArgument);
                            }
                    }
                }
            }
            break;
            default : SetError(FormulaError::IllegalParameter); break;
        }
    }
    if (nGlobalError == FormulaError::NONE)
        PushDouble( nValCount / nVal );
    else
        PushError( nGlobalError);
}

void ScInterpreter::ScGeoMean()
{
    short nParamCount = GetByte();
    double nVal = 0.0;
    double nValCount = 0.0;
    ScAddress aAdr;
    ScRange aRange;

    size_t nRefInList = 0;
    while ((nGlobalError == FormulaError::NONE) && (nParamCount-- > 0))
    {
        switch (GetStackType())
        {
            case svDouble    :
            {
                double x = GetDouble();
                if (x > 0.0)
                {
                    nVal += log(x);
                    nValCount++;
                }
                else if ( x == 0.0 )
                {
                    // value of 0 means that function result will be 0
                    while ( nParamCount-- > 0 )
                        PopError();
                    PushDouble( 0.0 );
                    return;
                }
                else
                    SetError( FormulaError::IllegalArgument);
                break;
            }
            case svSingleRef :
            {
                PopSingleRef( aAdr );
                ScRefCellValue aCell(*pDok, aAdr);
                if (aCell.hasNumeric())
                {
                    double x = GetCellValue(aAdr, aCell);
                    if (x > 0.0)
                    {
                        nVal += log(x);
                        nValCount++;
                    }
                    else if ( x == 0.0 )
                    {
                        // value of 0 means that function result will be 0
                        while ( nParamCount-- > 0 )
                            PopError();
                        PushDouble( 0.0 );
                        return;
                    }
                    else
                        SetError( FormulaError::IllegalArgument);
                }
                break;
            }
            case svDoubleRef :
            case svRefList :
            {
                FormulaError nErr = FormulaError::NONE;
                PopDoubleRef( aRange, nParamCount, nRefInList);
                double nCellVal;
                ScValueIterator aValIter( pDok, aRange, mnSubTotalFlags );
                if (aValIter.GetFirst(nCellVal, nErr))
                {
                    if (nCellVal > 0.0)
                    {
                        nVal += log(nCellVal);
                        nValCount++;
                    }
                    else if ( nCellVal == 0.0 )
                    {
                        // value of 0 means that function result will be 0
                        while ( nParamCount-- > 0 )
                            PopError();
                        PushDouble( 0.0 );
                        return;
                    }
                    else
                        SetError( FormulaError::IllegalArgument);
                    SetError(nErr);
                    while ((nErr == FormulaError::NONE) && aValIter.GetNext(nCellVal, nErr))
                    {
                        if (nCellVal > 0.0)
                        {
                            nVal += log(nCellVal);
                            nValCount++;
                        }
                        else if ( nCellVal == 0.0 )
                        {
                            // value of 0 means that function result will be 0
                            while ( nParamCount-- > 0 )
                                PopError();
                            PushDouble( 0.0 );
                            return;
                        }
                        else
                            SetError( FormulaError::IllegalArgument);
                    }
                    SetError(nErr);
                }
            }
            break;
            case svMatrix :
            case svExternalSingleRef:
            case svExternalDoubleRef:
            {
                ScMatrixRef pMat = GetMatrix();
                if (pMat)
                {
                    SCSIZE nCount = pMat->GetElementCount();
                    if (pMat->IsNumeric())
                    {
                        for (SCSIZE ui = 0; ui < nCount; ui++)
                        {
                            double x = pMat->GetDouble(ui);
                            if (x > 0.0)
                            {
                                nVal += log(x);
                                nValCount++;
                            }
                            else if ( x == 0.0 )
                            {
                                // value of 0 means that function result will be 0
                                while ( nParamCount-- > 0 )
                                    PopError();
                                PushDouble( 0.0 );
                                return;
                            }
                            else
                                SetError( FormulaError::IllegalArgument);
                        }
                    }
                    else
                    {
                        for (SCSIZE ui = 0; ui < nCount; ui++)
                        {
                            if (!pMat->IsStringOrEmpty(ui))
                            {
                                double x = pMat->GetDouble(ui);
                                if (x > 0.0)
                                {
                                    nVal += log(x);
                                    nValCount++;
                                }
                                else if ( x == 0.0 )
                                {
                                    // value of 0 means that function result will be 0
                                    while ( nParamCount-- > 0 )
                                        PopError();
                                    PushDouble( 0.0 );
                                    return;
                                }
                                else
                                    SetError( FormulaError::IllegalArgument);
                            }
                        }
                    }
                }
            }
            break;
            default : SetError(FormulaError::IllegalParameter); break;
        }
    }
    if (nGlobalError == FormulaError::NONE)
        PushDouble(exp(nVal / nValCount));
    else
        PushError( nGlobalError);
}

void ScInterpreter::ScStandard()
{
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        double sigma = GetDouble();
        double mue   = GetDouble();
        double x     = GetDouble();
        if (sigma < 0.0)
            PushError( FormulaError::IllegalArgument);
        else if (sigma == 0.0)
            PushError( FormulaError::DivisionByZero);
        else
            PushDouble((x-mue)/sigma);
    }
}
bool ScInterpreter::CalculateSkew(double& fSum,double& fCount,double& vSum,std::vector<double>& values)
{
    short nParamCount = GetByte();
    if ( !MustHaveParamCountMin( nParamCount, 1 )  )
        return false;

    fSum   = 0.0;
    fCount = 0.0;
    vSum   = 0.0;
    double fVal = 0.0;
    ScAddress aAdr;
    ScRange aRange;
    size_t nRefInList = 0;
    while (nParamCount-- > 0)
    {
        switch (GetStackType())
        {
            case svDouble :
            {
                fVal = GetDouble();
                fSum += fVal;
                values.push_back(fVal);
                fCount++;
            }
                break;
            case svSingleRef :
            {
                PopSingleRef( aAdr );
                ScRefCellValue aCell(*pDok, aAdr);
                if (aCell.hasNumeric())
                {
                    fVal = GetCellValue(aAdr, aCell);
                    fSum += fVal;
                    values.push_back(fVal);
                    fCount++;
                }
            }
            break;
            case svDoubleRef :
            case svRefList :
            {
                PopDoubleRef( aRange, nParamCount, nRefInList);
                FormulaError nErr = FormulaError::NONE;
                ScValueIterator aValIter( pDok, aRange, mnSubTotalFlags );
                if (aValIter.GetFirst(fVal, nErr))
                {
                    fSum += fVal;
                    values.push_back(fVal);
                    fCount++;
                    SetError(nErr);
                    while ((nErr == FormulaError::NONE) && aValIter.GetNext(fVal, nErr))
                    {
                        fSum += fVal;
                        values.push_back(fVal);
                        fCount++;
                    }
                    SetError(nErr);
                }
            }
            break;
            case svMatrix :
            case svExternalSingleRef:
            case svExternalDoubleRef:
            {
                ScMatrixRef pMat = GetMatrix();
                if (pMat)
                {
                    SCSIZE nCount = pMat->GetElementCount();
                    if (pMat->IsNumeric())
                    {
                        for (SCSIZE nElem = 0; nElem < nCount; nElem++)
                        {
                            fVal = pMat->GetDouble(nElem);
                            fSum += fVal;
                            values.push_back(fVal);
                            fCount++;
                        }
                    }
                    else
                    {
                        for (SCSIZE nElem = 0; nElem < nCount; nElem++)
                            if (!pMat->IsStringOrEmpty(nElem))
                            {
                                fVal = pMat->GetDouble(nElem);
                                fSum += fVal;
                                values.push_back(fVal);
                                fCount++;
                            }
                    }
                }
            }
            break;
            default :
                SetError(FormulaError::IllegalParameter);
            break;
        }
    }

    if (nGlobalError != FormulaError::NONE)
    {
        PushError( nGlobalError);
        return false;
    } // if (nGlobalError != FormulaError::NONE)
    return true;
}

void ScInterpreter::CalculateSkewOrSkewp( bool bSkewp )
{
    double fSum, fCount, vSum;
    std::vector<double> values;
    if (!CalculateSkew( fSum, fCount, vSum, values))
        return;
     // SKEW/SKEWP's constraints: they require at least three numbers
    if (fCount < 3.0)
    {
        // for interoperability with Excel
        PushError(FormulaError::DivisionByZero);
        return;
    }

    double fMean = fSum / fCount;

    for (double v : values)
        vSum += (v - fMean) * (v - fMean);

    double fStdDev = sqrt( vSum / (bSkewp ? fCount : (fCount - 1.0)));
    double xcube = 0.0;

    if (fStdDev == 0)
    {
        PushIllegalArgument();
        return;
    }

    for (double v : values)
    {
        double dx = (v - fMean) / fStdDev;
        xcube = xcube + (dx * dx * dx);
    }

    if (bSkewp)
        PushDouble( xcube / fCount );
    else
        PushDouble( ((xcube * fCount) / (fCount - 1.0)) / (fCount - 2.0) );
}

void ScInterpreter::ScSkew()
{
    CalculateSkewOrSkewp( false );
}

void ScInterpreter::ScSkewp()
{
    CalculateSkewOrSkewp( true );
}

double ScInterpreter::GetMedian( vector<double> & rArray )
{
    size_t nSize = rArray.size();
    if (nSize == 0 || nGlobalError != FormulaError::NONE)
    {
        SetError( FormulaError::NoValue);
        return 0.0;
    }

    // Upper median.
    size_t nMid = nSize / 2;
    vector<double>::iterator iMid = rArray.begin() + nMid;
    ::std::nth_element( rArray.begin(), iMid, rArray.end());
    if (nSize & 1)
        return *iMid;   // Lower and upper median are equal.
    else
    {
        double fUp = *iMid;
        // Lower median.
        iMid = ::std::max_element( rArray.begin(), rArray.begin() + nMid);
        return (fUp + *iMid) / 2;
    }
}

void ScInterpreter::ScMedian()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCountMin( nParamCount, 1 )  )
        return;
    vector<double> aArray;
    GetNumberSequenceArray( nParamCount, aArray, false );
    PushDouble( GetMedian( aArray));
}

double ScInterpreter::GetPercentile( vector<double> & rArray, double fPercentile )
{
    size_t nSize = rArray.size();
    if (nSize == 1)
        return rArray[0];
    else
    {
        size_t nIndex = static_cast<size_t>(::rtl::math::approxFloor( fPercentile * (nSize-1)));
        double fDiff = fPercentile * (nSize-1) - ::rtl::math::approxFloor( fPercentile * (nSize-1));
        OSL_ENSURE(nIndex < nSize, "GetPercentile: wrong index(1)");
        vector<double>::iterator iter = rArray.begin() + nIndex;
        ::std::nth_element( rArray.begin(), iter, rArray.end());
        if (fDiff == 0.0)
            return *iter;
        else
        {
            OSL_ENSURE(nIndex < nSize-1, "GetPercentile: wrong index(2)");
            double fVal = *iter;
            iter = ::std::min_element( rArray.begin() + nIndex + 1, rArray.end());
            return fVal + fDiff * (*iter - fVal);
        }
    }
}

double ScInterpreter::GetPercentileExclusive( vector<double> & rArray, double fPercentile )
{
    size_t nSize1 = rArray.size() + 1;
    if ( rArray.empty() || nSize1 == 1 || nGlobalError != FormulaError::NONE)
    {
        SetError( FormulaError::NoValue );
        return 0.0;
    }
    if ( fPercentile * nSize1 < 1.0 || fPercentile * nSize1 > static_cast<double>( nSize1 - 1 ) )
    {
        SetError( FormulaError::IllegalParameter );
        return 0.0;
    }

    size_t nIndex = static_cast<size_t>(::rtl::math::approxFloor( fPercentile * nSize1 - 1 ));
    double fDiff = fPercentile *  nSize1 - 1 - ::rtl::math::approxFloor( fPercentile * nSize1 - 1 );
    OSL_ENSURE(nIndex < ( nSize1 - 1 ), "GetPercentile: wrong index(1)");
    vector<double>::iterator iter = rArray.begin() + nIndex;
    ::std::nth_element( rArray.begin(), iter, rArray.end());
    if (fDiff == 0.0)
        return *iter;
    else
    {
        OSL_ENSURE(nIndex < nSize1, "GetPercentile: wrong index(2)");
        double fVal = *iter;
        iter = ::std::min_element( rArray.begin() + nIndex + 1, rArray.end());
        return fVal + fDiff * (*iter - fVal);
    }
}

void ScInterpreter::ScPercentile( bool bInclusive )
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;
    double alpha = GetDouble();
    if ( bInclusive ? ( alpha < 0.0 || alpha > 1.0 ) : ( alpha <= 0.0 || alpha >= 1.0 ) )
    {
        PushIllegalArgument();
        return;
    }
    vector<double> aArray;
    GetNumberSequenceArray( 1, aArray, false );
    if ( aArray.empty() || nGlobalError != FormulaError::NONE )
    {
        SetError( FormulaError::NoValue );
        return;
    }
    if ( bInclusive )
        PushDouble( GetPercentile( aArray, alpha ));
    else
        PushDouble( GetPercentileExclusive( aArray, alpha ));
}

void ScInterpreter::ScQuartile( bool bInclusive )
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;
    double fFlag = ::rtl::math::approxFloor(GetDouble());
    if ( bInclusive ? ( fFlag < 0.0 || fFlag > 4.0 ) : ( fFlag <= 0.0 || fFlag >= 4.0 ) )
    {
        PushIllegalArgument();
        return;
    }
    vector<double> aArray;
    GetNumberSequenceArray( 1, aArray, false );
    if ( aArray.empty() || nGlobalError != FormulaError::NONE )
    {
        SetError( FormulaError::NoValue );
        return;
    }
    if ( bInclusive )
        PushDouble( fFlag == 2.0 ? GetMedian( aArray ) : GetPercentile( aArray, 0.25 * fFlag ) );
    else
        PushDouble( fFlag == 2.0 ? GetMedian( aArray ) : GetPercentileExclusive( aArray, 0.25 * fFlag ) );
}

void ScInterpreter::ScModalValue()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCountMin( nParamCount, 1 ) )
        return;
    vector<double> aSortArray;
    GetSortArray( nParamCount, aSortArray, nullptr, false, false );
    SCSIZE nSize = aSortArray.size();
    if (nSize == 0 || nGlobalError != FormulaError::NONE)
        PushNoValue();
    else
    {
        SCSIZE nMaxIndex = 0, nMax = 1, nCount = 1;
        double nOldVal = aSortArray[0];
        SCSIZE i;
        for ( i = 1; i < nSize; i++)
        {
            if (aSortArray[i] == nOldVal)
                nCount++;
            else
            {
                nOldVal = aSortArray[i];
                if (nCount > nMax)
                {
                    nMax = nCount;
                    nMaxIndex = i-1;
                }
                nCount = 1;
            }
        }
        if (nCount > nMax)
        {
            nMax = nCount;
            nMaxIndex = i-1;
        }
        if (nMax == 1 && nCount == 1)
            PushNoValue();
        else if (nMax == 1)
            PushDouble(nOldVal);
        else
            PushDouble(aSortArray[nMaxIndex]);
    }
}

void ScInterpreter::ScModalValue_MS( bool bSingle )
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCountMin( nParamCount, 1 ) )
        return;
    vector<double> aArray;
    GetNumberSequenceArray( nParamCount, aArray, false );
    vector< double > aSortArray( aArray );
    QuickSort( aSortArray, nullptr );
    SCSIZE nSize = aSortArray.size();
    if ( nSize == 0 || nGlobalError != FormulaError::NONE )
        PushNoValue();
    else
    {
        SCSIZE nMax = 1, nCount = 1;
        double nOldVal = aSortArray[ 0 ];
        vector< double > aResultArray( 1 );
        SCSIZE i;
        for ( i = 1; i < nSize; i++ )
        {
            if ( aSortArray[ i ] == nOldVal )
                nCount++;
            else
            {
                if ( nCount >= nMax && nCount > 1 )
                {
                    if ( nCount > nMax )
                    {
                        nMax = nCount;
                        if ( aResultArray.size() != 1 )
                            vector< double >( 1 ).swap( aResultArray );
                        aResultArray[ 0 ] = nOldVal;
                    }
                    else
                        aResultArray.emplace_back( nOldVal );
                }
                nOldVal = aSortArray[ i ];
                nCount = 1;
            }
        }
        if ( nCount >= nMax && nCount > 1 )
        {
            if ( nCount > nMax )
                vector< double >().swap( aResultArray );
            aResultArray.emplace_back( nOldVal );
        }
        if ( nMax == 1 && nCount == 1 )
            PushNoValue();
        else if ( nMax == 1 )
            PushDouble( nOldVal ); // there is only 1 result, no reordering needed
        else
        {
            // sort resultArray according to ordering of aArray
            vector< vector< double > > aOrder;
            aOrder.resize( aResultArray.size(), vector< double >( 2 ) );
            for ( i = 0; i < aResultArray.size(); i++ )
            {
                for ( SCSIZE j = 0; j < nSize; j++ )
                {
                    if ( aArray[ j ] == aResultArray[ i ] )
                    {
                        aOrder[ i ][ 0 ] = aResultArray[ i ];
                        aOrder[ i ][ 1 ] = j;
                        break;
                    }
                }
            }
            sort( aOrder.begin(), aOrder.end(), []( const std::vector< double >& lhs,
                                                    const std::vector< double >& rhs )
                                                    { return lhs[ 1 ] < rhs[ 1 ]; } );

            if ( bSingle )
                PushDouble( aOrder[ 0 ][ 0 ] );
            else
            {
                // put result in correct order in aResultArray
                for ( i = 0; i < aResultArray.size(); i++ )
                    aResultArray[ i ] = aOrder[ i ][ 0 ];
                ScMatrixRef pResMatrix = GetNewMat( 1, aResultArray.size(), true );
                pResMatrix->PutDoubleVector( aResultArray, 0, 0 );
                PushMatrix( pResMatrix );
            }
        }
    }
}

void ScInterpreter::CalculateSmallLarge(bool bSmall)
{
    if ( !MustHaveParamCount( GetByte(), 2 )  )
        return;

    SCSIZE nCol = 0, nRow = 0;
    auto aArray = GetTopNumberArray(nCol, nRow);
    auto aArraySize = aArray.size();
    if (aArraySize == 0 || nGlobalError != FormulaError::NONE)
    {
        PushNoValue();
        return;
    }
    assert(aArraySize == nCol * nRow);
    for (double fArg : aArray)
    {
        double f = ::rtl::math::approxFloor(fArg);
        if (f < 1.0)
        {
            PushIllegalArgument();
            return;
        }
    }

    std::vector<SCSIZE> aRankArray;
    aRankArray.reserve(aArraySize);
    std::transform(aArray.begin(), aArray.end(), std::back_inserter(aRankArray),
                   [](double f) { return static_cast<SCSIZE>(f); });

    auto itMaxRank = std::max_element(aRankArray.begin(), aRankArray.end());
    assert(itMaxRank != aRankArray.end());
    SCSIZE k = *itMaxRank;

    vector<double> aSortArray;
    GetNumberSequenceArray(1, aSortArray, false );
    SCSIZE nSize = aSortArray.size();
    if (nSize == 0 || nGlobalError != FormulaError::NONE || nSize < k)
        PushNoValue();
    else if (aArraySize == 1)
    {
        vector<double>::iterator iPos = aSortArray.begin() + (bSmall ? k-1 : nSize-k);
        ::std::nth_element( aSortArray.begin(), iPos, aSortArray.end());
        PushDouble( *iPos);
    }
    else
    {
        std::set<SCSIZE> aIndices;
        for (SCSIZE n : aRankArray)
            aIndices.insert(bSmall ? n-1 : nSize-n);
        // We can spare sorting when the total number of ranks is small enough.
        // Find only the elements at given indices if, arbitrarily, the index size is
        // smaller than 1/3 of the haystack array's size; just sort it squarely, otherwise.
        if (aIndices.size() < nSize/3)
        {
            auto itBegin = aSortArray.begin();
            for (SCSIZE i : aIndices)
            {
                auto it = aSortArray.begin() + i;
                std::nth_element(itBegin, it, aSortArray.end());
                itBegin = ++it;
            }
        }
        else
            std::sort(aSortArray.begin(), aSortArray.end());

        aArray.clear();
        for (SCSIZE n : aRankArray)
            aArray.push_back(aSortArray[bSmall ? n-1 : nSize-n]);
        ScMatrixRef pResult = GetNewMat(nCol, nRow, aArray);
        PushMatrix(pResult);
    }
}

void ScInterpreter::ScLarge()
{
    CalculateSmallLarge(false);
}

void ScInterpreter::ScSmall()
{
    CalculateSmallLarge(true);
}

void ScInterpreter::ScPercentrank( bool bInclusive )
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 2, 3 ) )
        return;
    double fSignificance = ( nParamCount == 3 ? ::rtl::math::approxFloor( GetDouble() ) : 3.0 );
    if ( fSignificance < 1.0 )
    {
        PushIllegalArgument();
        return;
    }
    double fNum = GetDouble();
    vector<double> aSortArray;
    GetSortArray( 1, aSortArray, nullptr, false, false );
    SCSIZE nSize = aSortArray.size();
    if ( nSize == 0 || nGlobalError != FormulaError::NONE )
        PushNoValue();
    else
    {
        if ( fNum < aSortArray[ 0 ] || fNum > aSortArray[ nSize - 1 ] )
            PushNoValue();
        else
        {
            double fRes;
            if ( nSize == 1 )
                fRes = 1.0;            // fNum == aSortArray[ 0 ], see test above
            else
                fRes = GetPercentrank( aSortArray, fNum, bInclusive );
            if ( fRes != 0.0 )
            {
                double fExp = ::rtl::math::approxFloor( log10( fRes ) ) + 1.0 - fSignificance;
                fRes = ::rtl::math::round( fRes * pow( 10, -fExp ) ) / pow( 10, -fExp );
            }
            PushDouble( fRes );
        }
    }
}

double ScInterpreter::GetPercentrank( ::std::vector<double> & rArray, double fVal, bool bInclusive )
{
    SCSIZE nSize = rArray.size();
    double fRes;
    if ( fVal == rArray[ 0 ] )
    {
        if ( bInclusive )
            fRes = 0.0;
        else
            fRes = 1.0 / static_cast<double>( nSize + 1 );
    }
    else
    {
        SCSIZE nOldCount = 0;
        double fOldVal = rArray[ 0 ];
        SCSIZE i;
        for ( i = 1; i < nSize && rArray[ i ] < fVal; i++ )
        {
            if ( rArray[ i ] != fOldVal )
            {
                nOldCount = i;
                fOldVal = rArray[ i ];
            }
        }
        if ( rArray[ i ] != fOldVal )
            nOldCount = i;
        if ( fVal == rArray[ i ] )
        {
            if ( bInclusive )
                fRes = div( nOldCount, nSize - 1 );
            else
                fRes = static_cast<double>( i + 1 ) / static_cast<double>( nSize + 1 );
        }
        else
        {
            //  nOldCount is the count of smaller entries
            //  fVal is between rArray[ nOldCount - 1 ] and rArray[ nOldCount ]
            //  use linear interpolation to find a position between the entries
            if ( nOldCount == 0 )
            {
                OSL_FAIL( "should not happen" );
                fRes = 0.0;
            }
            else
            {
                double fFract = ( fVal - rArray[ nOldCount - 1 ] ) /
                    ( rArray[ nOldCount ] - rArray[ nOldCount - 1 ] );
                if ( bInclusive )
                    fRes = div( static_cast<double>( nOldCount - 1 ) + fFract, nSize - 1 );
                else
                    fRes = ( static_cast<double>(nOldCount) + fFract ) / static_cast<double>( nSize + 1 );
            }
        }
    }
    return fRes;
}

void ScInterpreter::ScTrimMean()
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;
    double alpha = GetDouble();
    if (alpha < 0.0 || alpha >= 1.0)
    {
        PushIllegalArgument();
        return;
    }
    vector<double> aSortArray;
    GetSortArray( 1, aSortArray, nullptr, false, false );
    SCSIZE nSize = aSortArray.size();
    if (nSize == 0 || nGlobalError != FormulaError::NONE)
        PushNoValue();
    else
    {
        sal_uLong nIndex = static_cast<sal_uLong>(::rtl::math::approxFloor(alpha*static_cast<double>(nSize)));
        if (nIndex % 2 != 0)
            nIndex--;
        nIndex /= 2;
        OSL_ENSURE(nIndex < nSize, "ScTrimMean: wrong index");
        double fSum = 0.0;
        for (SCSIZE i = nIndex; i < nSize-nIndex; i++)
            fSum += aSortArray[i];
        PushDouble(fSum/static_cast<double>(nSize-2*nIndex));
    }
}

std::vector<double> ScInterpreter::GetTopNumberArray( SCSIZE& rCol, SCSIZE& rRow )
{
    std::vector<double> aArray;
    switch (GetStackType())
    {
        case svDouble:
            aArray.push_back(PopDouble());
            rCol = rRow = 1;
        break;
        case svSingleRef:
        {
            ScAddress aAdr;
            PopSingleRef(aAdr);
            ScRefCellValue aCell(*pDok, aAdr);
            if (aCell.hasNumeric())
            {
                aArray.push_back(GetCellValue(aAdr, aCell));
                rCol = rRow = 1;
            }
        }
        break;
        case svDoubleRef:
        {
            ScRange aRange;
            PopDoubleRef(aRange, true);
            if (nGlobalError != FormulaError::NONE)
                break;

            // give up unless the start and end are in the same sheet
            if (aRange.aStart.Tab() != aRange.aEnd.Tab())
            {
                SetError(FormulaError::IllegalParameter);
                break;
            }

            // the range already is in order
            assert(aRange.aStart.Col() <= aRange.aEnd.Col());
            assert(aRange.aStart.Row() <= aRange.aEnd.Row());
            rCol = aRange.aEnd.Col() - aRange.aStart.Col() + 1;
            rRow = aRange.aEnd.Row() - aRange.aStart.Row() + 1;
            aArray.reserve(rCol * rRow);

            FormulaError nErr = FormulaError::NONE;
            double fCellVal;
            ScValueIterator aValIter(pDok, aRange, mnSubTotalFlags);
            if (aValIter.GetFirst(fCellVal, nErr))
            {
                do
                    aArray.push_back(fCellVal);
                while (aValIter.GetNext(fCellVal, nErr) && nErr == FormulaError::NONE);
            }
            if (aArray.size() != rCol * rRow)
            {
                aArray.clear();
                SetError(nErr);
            }
        }
        break;
        case svMatrix:
        case svExternalSingleRef:
        case svExternalDoubleRef:
        {
            ScMatrixRef pMat = GetMatrix();
            if (!pMat)
                break;

            if (pMat->IsNumeric())
            {
                SCSIZE nCount = pMat->GetElementCount();
                aArray.reserve(nCount);
                for (SCSIZE i = 0; i < nCount; ++i)
                    aArray.push_back(pMat->GetDouble(i));
                pMat->GetDimensions(rCol, rRow);
            }
            else
                SetError(FormulaError::IllegalParameter);
        }
        break;
        default:
            SetError(FormulaError::IllegalParameter);
        break;
    }
    return aArray;
}

void ScInterpreter::GetNumberSequenceArray( sal_uInt8 nParamCount, vector<double>& rArray, bool bConvertTextInArray )
{
    ScAddress aAdr;
    ScRange aRange;
    short nParam = nParamCount;
    size_t nRefInList = 0;
    ReverseStack( nParamCount );
    while (nParam-- > 0)
    {
        const StackVar eStackType = GetStackType();
        switch (eStackType)
        {
            case svDouble :
                rArray.push_back( PopDouble());
            break;
            case svSingleRef :
            {
                PopSingleRef( aAdr );
                ScRefCellValue aCell(*pDok, aAdr);
                if (aCell.hasNumeric())
                    rArray.push_back(GetCellValue(aAdr, aCell));
            }
            break;
            case svDoubleRef :
            case svRefList :
            {
                PopDoubleRef( aRange, nParam, nRefInList);
                if (nGlobalError != FormulaError::NONE)
                    break;

                aRange.PutInOrder();
                SCSIZE nCellCount = aRange.aEnd.Col() - aRange.aStart.Col() + 1;
                nCellCount *= aRange.aEnd.Row() - aRange.aStart.Row() + 1;
                rArray.reserve( rArray.size() + nCellCount);

                FormulaError nErr = FormulaError::NONE;
                double fCellVal;
                ScValueIterator aValIter( pDok, aRange, mnSubTotalFlags );
                if (aValIter.GetFirst( fCellVal, nErr))
                {
                    rArray.push_back( fCellVal);
                    SetError(nErr);
                    while ((nErr == FormulaError::NONE) && aValIter.GetNext( fCellVal, nErr))
                        rArray.push_back( fCellVal);
                    SetError(nErr);
                }
            }
            break;
            case svMatrix :
            case svExternalSingleRef:
            case svExternalDoubleRef:
            {
                ScMatrixRef pMat = GetMatrix();
                if (!pMat)
                    break;

                SCSIZE nCount = pMat->GetElementCount();
                rArray.reserve( rArray.size() + nCount);
                if (pMat->IsNumeric())
                {
                    for (SCSIZE i = 0; i < nCount; ++i)
                        rArray.push_back( pMat->GetDouble(i));
                }
                else if (bConvertTextInArray && eStackType == svMatrix)
                {
                    for (SCSIZE i = 0; i < nCount; ++i)
                    {
                        if ( pMat->IsValue( i ) )
                            rArray.push_back( pMat->GetDouble(i));
                        else
                        {
                            // tdf#88547 try to convert string to (date)value
                            OUString aStr = pMat->GetString( i ).getString();
                            if ( aStr.getLength() > 0 )
                            {
                                FormulaError nErr = nGlobalError;
                                nGlobalError = FormulaError::NONE;
                                double fVal = ConvertStringToValue( aStr );
                                if ( nGlobalError == FormulaError::NONE )
                                {
                                    rArray.push_back( fVal );
                                    nGlobalError = nErr;
                                }
                                else
                                {
                                    rArray.push_back( CreateDoubleError( FormulaError::NoValue));
                                    // Propagate previous error if any, else
                                    // the current #VALUE! error.
                                    if (nErr != FormulaError::NONE)
                                        nGlobalError = nErr;
                                    else
                                        nGlobalError = FormulaError::NoValue;
                                }
                            }
                        }
                    }
                }
                else
                {
                    for (SCSIZE i = 0; i < nCount; ++i)
                    {
                        if ( pMat->IsValue( i ) )
                            rArray.push_back( pMat->GetDouble(i));
                    }
                }
            }
            break;
            default :
                PopError();
                SetError( FormulaError::IllegalParameter);
            break;
        }
        if (nGlobalError != FormulaError::NONE)
            break;  // while
    }
    // nParam > 0 in case of error, clean stack environment and obtain earlier
    // error if there was one.
    while (nParam-- > 0)
        PopError();
}

void ScInterpreter::GetSortArray( sal_uInt8 nParamCount, vector<double>& rSortArray, vector<long>* pIndexOrder, bool bConvertTextInArray, bool bAllowEmptyArray )
{
    GetNumberSequenceArray( nParamCount, rSortArray, bConvertTextInArray );
    if (rSortArray.size() > MAX_COUNT_DOUBLE_FOR_SORT)
        SetError( FormulaError::MatrixSize);
    else if ( rSortArray.empty() )
    {
        if ( bAllowEmptyArray )
            return;
        SetError( FormulaError::NoValue);
    }

    if (nGlobalError == FormulaError::NONE)
        QuickSort( rSortArray, pIndexOrder);
}

static void lcl_QuickSort( long nLo, long nHi, vector<double>& rSortArray, vector<long>* pIndexOrder )
{
    // If pIndexOrder is not NULL, we assume rSortArray.size() == pIndexOrder->size().

    using ::std::swap;

    if (nHi - nLo == 1)
    {
        if (rSortArray[nLo] > rSortArray[nHi])
        {
            swap(rSortArray[nLo],  rSortArray[nHi]);
            if (pIndexOrder)
                swap(pIndexOrder->at(nLo), pIndexOrder->at(nHi));
        }
        return;
    }

    long ni = nLo;
    long nj = nHi;
    do
    {
        double fLo = rSortArray[nLo];
        while (ni <= nHi && rSortArray[ni] < fLo) ni++;
        while (nj >= nLo && fLo < rSortArray[nj]) nj--;
        if (ni <= nj)
        {
            if (ni != nj)
            {
                swap(rSortArray[ni],  rSortArray[nj]);
                if (pIndexOrder)
                    swap(pIndexOrder->at(ni), pIndexOrder->at(nj));
            }

            ++ni;
            --nj;
        }
    }
    while (ni < nj);

    if ((nj - nLo) < (nHi - ni))
    {
        if (nLo < nj) lcl_QuickSort(nLo, nj, rSortArray, pIndexOrder);
        if (ni < nHi) lcl_QuickSort(ni, nHi, rSortArray, pIndexOrder);
    }
    else
    {
        if (ni < nHi) lcl_QuickSort(ni, nHi, rSortArray, pIndexOrder);
        if (nLo < nj) lcl_QuickSort(nLo, nj, rSortArray, pIndexOrder);
    }
}

void ScInterpreter::QuickSort( vector<double>& rSortArray, vector<long>* pIndexOrder )
{
    long n = static_cast<long>(rSortArray.size());

    if (pIndexOrder)
    {
        pIndexOrder->clear();
        pIndexOrder->reserve(n);
        for (long i = 0; i < n; ++i)
            pIndexOrder->push_back(i);
    }

    if (n < 2)
        return;

    size_t nValCount = rSortArray.size();
    for (size_t i = 0; (i + 4) <= nValCount-1; i += 4)
    {
        size_t nInd = comphelper::rng::uniform_size_distribution(0, nValCount-2);
        ::std::swap( rSortArray[i], rSortArray[nInd]);
        if (pIndexOrder)
            ::std::swap( pIndexOrder->at(i), pIndexOrder->at(nInd));
    }

    lcl_QuickSort(0, n-1, rSortArray, pIndexOrder);
}

void ScInterpreter::ScRank( bool bAverage )
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 2, 3 ) )
        return;
    bool bAscending;
    if ( nParamCount == 3 )
        bAscending = GetBool();
    else
        bAscending = false;

    vector<double> aSortArray;
    GetSortArray( 1, aSortArray, nullptr, false, false );
    double fVal = GetDouble();
    SCSIZE nSize = aSortArray.size();
    if ( nSize == 0 || nGlobalError != FormulaError::NONE )
        PushNoValue();
    else
    {
        if ( fVal < aSortArray[ 0 ] || fVal > aSortArray[ nSize - 1 ] )
            PushNoValue();
        else
        {
            double fLastPos = 0;
            double fFirstPos = -1.0;
            bool bFinished = false;
            SCSIZE i;
            for (i = 0; i < nSize && !bFinished; i++)
            {
                if ( aSortArray[ i ] == fVal )
                {
                    if ( fFirstPos < 0 )
                        fFirstPos = i + 1.0;
                }
                else
                {
                    if ( aSortArray[ i ] > fVal )
                    {
                        fLastPos = i;
                        bFinished = true;
                    }
                }
            }
            if ( !bFinished )
                fLastPos = i;
            if ( !bAverage )
            {
                if ( bAscending )
                    PushDouble( fFirstPos );
                else
                    PushDouble( nSize + 1.0 - fLastPos );
            }
            else
            {
                if ( bAscending )
                    PushDouble( ( fFirstPos + fLastPos ) / 2.0 );
                else
                    PushDouble( nSize + 1.0 - ( fFirstPos + fLastPos ) / 2.0 );
            }
        }
    }
}

void ScInterpreter::ScAveDev()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCountMin( nParamCount, 1 ) )
        return;
    sal_uInt16 SaveSP = sp;
    double nMiddle = 0.0;
    double rVal = 0.0;
    double rValCount = 0.0;
    ScAddress aAdr;
    ScRange aRange;
    short nParam = nParamCount;
    size_t nRefInList = 0;
    while (nParam-- > 0)
    {
        switch (GetStackType())
        {
            case svDouble :
                rVal += GetDouble();
                rValCount++;
                break;
            case svSingleRef :
            {
                PopSingleRef( aAdr );
                ScRefCellValue aCell(*pDok, aAdr);
                if (aCell.hasNumeric())
                {
                    rVal += GetCellValue(aAdr, aCell);
                    rValCount++;
                }
            }
            break;
            case svDoubleRef :
            case svRefList :
            {
                FormulaError nErr = FormulaError::NONE;
                double nCellVal;
                PopDoubleRef( aRange, nParam, nRefInList);
                ScValueIterator aValIter( pDok, aRange, mnSubTotalFlags );
                if (aValIter.GetFirst(nCellVal, nErr))
                {
                    rVal += nCellVal;
                    rValCount++;
                    SetError(nErr);
                    while ((nErr == FormulaError::NONE) && aValIter.GetNext(nCellVal, nErr))
                    {
                        rVal += nCellVal;
                        rValCount++;
                    }
                    SetError(nErr);
                }
            }
            break;
            case svMatrix :
            case svExternalSingleRef:
            case svExternalDoubleRef:
            {
                ScMatrixRef pMat = GetMatrix();
                if (pMat)
                {
                    SCSIZE nCount = pMat->GetElementCount();
                    if (pMat->IsNumeric())
                    {
                        for (SCSIZE nElem = 0; nElem < nCount; nElem++)
                        {
                            rVal += pMat->GetDouble(nElem);
                            rValCount++;
                        }
                    }
                    else
                    {
                        for (SCSIZE nElem = 0; nElem < nCount; nElem++)
                            if (!pMat->IsStringOrEmpty(nElem))
                            {
                                rVal += pMat->GetDouble(nElem);
                                rValCount++;
                            }
                    }
                }
            }
            break;
            default :
                SetError(FormulaError::IllegalParameter);
            break;
        }
    }
    if (nGlobalError != FormulaError::NONE)
    {
        PushError( nGlobalError);
        return;
    }
    nMiddle = rVal / rValCount;
    sp = SaveSP;
    rVal = 0.0;
    nParam = nParamCount;
    nRefInList = 0;
    while (nParam-- > 0)
    {
        switch (GetStackType())
        {
            case svDouble :
                rVal += fabs(GetDouble() - nMiddle);
                break;
            case svSingleRef :
            {
                PopSingleRef( aAdr );
                ScRefCellValue aCell(*pDok, aAdr);
                if (aCell.hasNumeric())
                    rVal += fabs(GetCellValue(aAdr, aCell) - nMiddle);
            }
            break;
            case svDoubleRef :
            case svRefList :
            {
                FormulaError nErr = FormulaError::NONE;
                double nCellVal;
                PopDoubleRef( aRange, nParam, nRefInList);
                ScValueIterator aValIter( pDok, aRange, mnSubTotalFlags );
                if (aValIter.GetFirst(nCellVal, nErr))
                {
                    rVal += (fabs(nCellVal - nMiddle));
                    while (aValIter.GetNext(nCellVal, nErr))
                         rVal += fabs(nCellVal - nMiddle);
                }
            }
            break;
            case svMatrix :
            case svExternalSingleRef:
            case svExternalDoubleRef:
            {
                ScMatrixRef pMat = GetMatrix();
                if (pMat)
                {
                    SCSIZE nCount = pMat->GetElementCount();
                    if (pMat->IsNumeric())
                    {
                        for (SCSIZE nElem = 0; nElem < nCount; nElem++)
                        {
                            rVal += fabs(pMat->GetDouble(nElem) - nMiddle);
                        }
                    }
                    else
                    {
                        for (SCSIZE nElem = 0; nElem < nCount; nElem++)
                        {
                            if (!pMat->IsStringOrEmpty(nElem))
                                rVal += fabs(pMat->GetDouble(nElem) - nMiddle);
                        }
                    }
                }
            }
            break;
            default : SetError(FormulaError::IllegalParameter); break;
        }
    }
    PushDouble(rVal / rValCount);
}

void ScInterpreter::ScDevSq()
{
    auto VarResult = []( double fVal, size_t /*nValCount*/ )
    {
        return fVal;
    };
    GetStVarParams( false /*bTextAsZero*/, VarResult);
}

void ScInterpreter::ScProbability()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 3, 4 ) )
        return;
    double fUp, fLo;
    fUp = GetDouble();
    if (nParamCount == 4)
        fLo = GetDouble();
    else
        fLo = fUp;
    if (fLo > fUp)
    {
        double fTemp = fLo;
        fLo = fUp;
        fUp = fTemp;
    }
    ScMatrixRef pMatP = GetMatrix();
    ScMatrixRef pMatW = GetMatrix();
    if (!pMatP || !pMatW)
        PushIllegalParameter();
    else
    {
        SCSIZE nC1, nC2;
        SCSIZE nR1, nR2;
        pMatP->GetDimensions(nC1, nR1);
        pMatW->GetDimensions(nC2, nR2);
        if (nC1 != nC2 || nR1 != nR2 || nC1 == 0 || nR1 == 0 ||
            nC2 == 0 || nR2 == 0)
            PushNA();
        else
        {
            double fSum = 0.0;
            double fRes = 0.0;
            bool bStop = false;
            double fP, fW;
            for ( SCSIZE i = 0; i < nC1 && !bStop; i++ )
            {
                for (SCSIZE j = 0; j < nR1 && !bStop; ++j )
                {
                    if (pMatP->IsValue(i,j) && pMatW->IsValue(i,j))
                    {
                        fP = pMatP->GetDouble(i,j);
                        fW = pMatW->GetDouble(i,j);
                        if (fP < 0.0 || fP > 1.0)
                            bStop = true;
                        else
                        {
                            fSum += fP;
                            if (fW >= fLo && fW <= fUp)
                                fRes += fP;
                        }
                    }
                    else
                        SetError( FormulaError::IllegalArgument);
                }
            }
            if (bStop || fabs(fSum -1.0) > 1.0E-7)
                PushNoValue();
            else
                PushDouble(fRes);
        }
    }
}

void ScInterpreter::ScCorrel()
{
    // This is identical to ScPearson()
    ScPearson();
}

void ScInterpreter::ScCovarianceP()
{
    CalculatePearsonCovar( false, false, false );
}

void ScInterpreter::ScCovarianceS()
{
    CalculatePearsonCovar( false, false, true );
}

void ScInterpreter::ScPearson()
{
    CalculatePearsonCovar( true, false, false );
}

void ScInterpreter::CalculatePearsonCovar( bool _bPearson, bool _bStexy, bool _bSample )
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;
    ScMatrixRef pMat1 = GetMatrix();
    ScMatrixRef pMat2 = GetMatrix();
    if (!pMat1 || !pMat2)
    {
        PushIllegalParameter();
        return;
    }
    SCSIZE nC1, nC2;
    SCSIZE nR1, nR2;
    pMat1->GetDimensions(nC1, nR1);
    pMat2->GetDimensions(nC2, nR2);
    if (nR1 != nR2 || nC1 != nC2)
    {
        PushIllegalArgument();
        return;
    }
    /* #i78250#
     * (sum((X-MeanX)(Y-MeanY)))/N equals (SumXY)/N-MeanX*MeanY mathematically,
     * but the latter produces wrong results if the absolute values are high,
     * for example above 10^8
     */
    double fCount           = 0.0;
    double fSumX            = 0.0;
    double fSumY            = 0.0;

    for (SCSIZE i = 0; i < nC1; i++)
    {
        for (SCSIZE j = 0; j < nR1; j++)
        {
            if (!pMat1->IsStringOrEmpty(i,j) && !pMat2->IsStringOrEmpty(i,j))
            {
                double fValX = pMat1->GetDouble(i,j);
                double fValY = pMat2->GetDouble(i,j);
                fSumX += fValX;
                fSumY += fValY;
                fCount++;
            }
        }
    }
    if (fCount < (_bStexy ? 3.0 : (_bSample ? 2.0 : 1.0)))
        PushNoValue();
    else
    {
        double fSumDeltaXDeltaY = 0.0; // sum of (ValX-MeanX)*(ValY-MeanY)
        double fSumSqrDeltaX    = 0.0; // sum of (ValX-MeanX)^2
        double fSumSqrDeltaY    = 0.0; // sum of (ValY-MeanY)^2
        const double fMeanX = fSumX / fCount;
        const double fMeanY = fSumY / fCount;
        for (SCSIZE i = 0; i < nC1; i++)
        {
            for (SCSIZE j = 0; j < nR1; j++)
            {
                if (!pMat1->IsStringOrEmpty(i,j) && !pMat2->IsStringOrEmpty(i,j))
                {
                    const double fValX = pMat1->GetDouble(i,j);
                    const double fValY = pMat2->GetDouble(i,j);
                    fSumDeltaXDeltaY += (fValX - fMeanX) * (fValY - fMeanY);
                    if ( _bPearson )
                    {
                        fSumSqrDeltaX    += (fValX - fMeanX) * (fValX - fMeanX);
                        fSumSqrDeltaY    += (fValY - fMeanY) * (fValY - fMeanY);
                    }
                }
            }
        }
        if ( _bPearson )
        {
            if (fSumSqrDeltaX == 0.0 || ( !_bStexy && fSumSqrDeltaY == 0.0) )
                PushError( FormulaError::DivisionByZero);
            else if ( _bStexy )
                PushDouble( sqrt( (fSumSqrDeltaY - fSumDeltaXDeltaY *
                            fSumDeltaXDeltaY / fSumSqrDeltaX) / (fCount-2)));
            else
                PushDouble( fSumDeltaXDeltaY / sqrt( fSumSqrDeltaX * fSumSqrDeltaY));
        }
        else
        {
            if ( _bSample )
                PushDouble( fSumDeltaXDeltaY / ( fCount - 1 ) );
            else
                PushDouble( fSumDeltaXDeltaY / fCount);
        }
    }
}

void ScInterpreter::ScRSQ()
{
    // Same as ScPearson()*ScPearson()
    ScPearson();
    if (nGlobalError == FormulaError::NONE)
    {
        switch (GetStackType())
        {
            case svDouble:
                {
                    double fVal = PopDouble();
                    PushDouble( fVal * fVal);
                }
                break;
            default:
                PopError();
                PushNoValue();
        }
    }
}

void ScInterpreter::ScSTEYX()
{
    CalculatePearsonCovar( true, true, false );
}
void ScInterpreter::CalculateSlopeIntercept(bool bSlope)
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;
    ScMatrixRef pMat1 = GetMatrix();
    ScMatrixRef pMat2 = GetMatrix();
    if (!pMat1 || !pMat2)
    {
        PushIllegalParameter();
        return;
    }
    SCSIZE nC1, nC2;
    SCSIZE nR1, nR2;
    pMat1->GetDimensions(nC1, nR1);
    pMat2->GetDimensions(nC2, nR2);
    if (nR1 != nR2 || nC1 != nC2)
    {
        PushIllegalArgument();
        return;
    }
    // #i78250# numerical stability improved
    double fCount           = 0.0;
    double fSumX            = 0.0;
    double fSumY            = 0.0;

    for (SCSIZE i = 0; i < nC1; i++)
    {
        for (SCSIZE j = 0; j < nR1; j++)
        {
            if (!pMat1->IsStringOrEmpty(i,j) && !pMat2->IsStringOrEmpty(i,j))
            {
                double fValX = pMat1->GetDouble(i,j);
                double fValY = pMat2->GetDouble(i,j);
                fSumX += fValX;
                fSumY += fValY;
                fCount++;
            }
        }
    }
    if (fCount < 1.0)
        PushNoValue();
    else
    {
        double fSumDeltaXDeltaY = 0.0; // sum of (ValX-MeanX)*(ValY-MeanY)
        double fSumSqrDeltaX    = 0.0; // sum of (ValX-MeanX)^2
        double fMeanX = fSumX / fCount;
        double fMeanY = fSumY / fCount;
        for (SCSIZE i = 0; i < nC1; i++)
        {
            for (SCSIZE j = 0; j < nR1; j++)
            {
                if (!pMat1->IsStringOrEmpty(i,j) && !pMat2->IsStringOrEmpty(i,j))
                {
                    double fValX = pMat1->GetDouble(i,j);
                    double fValY = pMat2->GetDouble(i,j);
                    fSumDeltaXDeltaY += (fValX - fMeanX) * (fValY - fMeanY);
                    fSumSqrDeltaX    += (fValX - fMeanX) * (fValX - fMeanX);
                }
            }
        }
        if (fSumSqrDeltaX == 0.0)
            PushError( FormulaError::DivisionByZero);
        else
        {
            if ( bSlope )
                PushDouble( fSumDeltaXDeltaY / fSumSqrDeltaX);
            else
                PushDouble( fMeanY - fSumDeltaXDeltaY / fSumSqrDeltaX * fMeanX);
        }
    }
}

void ScInterpreter::ScSlope()
{
    CalculateSlopeIntercept(true);
}

void ScInterpreter::ScIntercept()
{
    CalculateSlopeIntercept(false);
}

void ScInterpreter::ScForecast()
{
    if ( !MustHaveParamCount( GetByte(), 3 ) )
        return;
    ScMatrixRef pMat1 = GetMatrix();
    ScMatrixRef pMat2 = GetMatrix();
    if (!pMat1 || !pMat2)
    {
        PushIllegalParameter();
        return;
    }
    SCSIZE nC1, nC2;
    SCSIZE nR1, nR2;
    pMat1->GetDimensions(nC1, nR1);
    pMat2->GetDimensions(nC2, nR2);
    if (nR1 != nR2 || nC1 != nC2)
    {
        PushIllegalArgument();
        return;
    }
    double fVal = GetDouble();
    // #i78250# numerical stability improved
    double fCount           = 0.0;
    double fSumX            = 0.0;
    double fSumY            = 0.0;

    for (SCSIZE i = 0; i < nC1; i++)
    {
        for (SCSIZE j = 0; j < nR1; j++)
        {
            if (!pMat1->IsStringOrEmpty(i,j) && !pMat2->IsStringOrEmpty(i,j))
            {
                double fValX = pMat1->GetDouble(i,j);
                double fValY = pMat2->GetDouble(i,j);
                fSumX += fValX;
                fSumY += fValY;
                fCount++;
            }
        }
    }
    if (fCount < 1.0)
        PushNoValue();
    else
    {
        double fSumDeltaXDeltaY = 0.0; // sum of (ValX-MeanX)*(ValY-MeanY)
        double fSumSqrDeltaX    = 0.0; // sum of (ValX-MeanX)^2
        double fMeanX = fSumX / fCount;
        double fMeanY = fSumY / fCount;
        for (SCSIZE i = 0; i < nC1; i++)
        {
            for (SCSIZE j = 0; j < nR1; j++)
            {
                if (!pMat1->IsStringOrEmpty(i,j) && !pMat2->IsStringOrEmpty(i,j))
                {
                    double fValX = pMat1->GetDouble(i,j);
                    double fValY = pMat2->GetDouble(i,j);
                    fSumDeltaXDeltaY += (fValX - fMeanX) * (fValY - fMeanY);
                    fSumSqrDeltaX    += (fValX - fMeanX) * (fValX - fMeanX);
                }
            }
        }
        if (fSumSqrDeltaX == 0.0)
            PushError( FormulaError::DivisionByZero);
        else
            PushDouble( fMeanY + fSumDeltaXDeltaY / fSumSqrDeltaX * (fVal - fMeanX));
    }
}

static void lcl_roundUpNearestPow2(SCSIZE& nNum, SCSIZE& nNumBits)
{
    // Find the least power of 2 that is less than or equal to nNum.
    SCSIZE nPow2(1);
    nNumBits = std::numeric_limits<SCSIZE>::digits;
    nPow2 <<= (nNumBits - 1);
    while (nPow2 >= 1)
    {
        if (nNum & nPow2)
            break;

        --nNumBits;
        nPow2 >>= 1;
    }

    if (nPow2 != nNum)
        nNum = nPow2 ? (nPow2 << 1) : 1;
    else
        --nNumBits;
}

static SCSIZE lcl_bitReverse(SCSIZE nIn, SCSIZE nBound)
{
    SCSIZE nOut = 0;
    for (SCSIZE nMask = 1; nMask < nBound; nMask <<= 1)
    {
        nOut <<= 1;

        if (nIn & nMask)
            nOut |= 1;
    }

    return nOut;
}

// Computes and stores twiddle factors for computing DFT later.
struct ScTwiddleFactors
{
    ScTwiddleFactors(SCSIZE nN, bool bInverse) :
        mfWReal(nN),
        mfWImag(nN),
        mnN(nN),
        mbInverse(bInverse)
    {}

    void Compute();

    void Conjugate()
    {
        mbInverse = !mbInverse;
        for (SCSIZE nIdx = 0; nIdx < mnN; ++nIdx)
            mfWImag[nIdx] = -mfWImag[nIdx];
    }

    std::vector<double> mfWReal;
    std::vector<double> mfWImag;
    SCSIZE mnN;
    bool mbInverse;
};

void ScTwiddleFactors::Compute()
{
    mfWReal.resize(mnN);
    mfWImag.resize(mnN);

    double nW = (mbInverse ? 2 : -2)*F_PI/static_cast<double>(mnN);

    if (mnN == 1)
    {
        mfWReal[0] = 1.0;
        mfWImag[0] = 0.0;
    }
    else if (mnN == 2)
    {
        mfWReal[0] = 1;
        mfWImag[0] = 0;

        mfWReal[1] = -1;
        mfWImag[1] = 0;
    }
    else if (mnN == 4)
    {
        mfWReal[0] = 1;
        mfWImag[0] = 0;

        mfWReal[1] = 0;
        mfWImag[1] = (mbInverse ? 1.0 : -1.0);

        mfWReal[2] = -1;
        mfWImag[2] = 0;

        mfWReal[3] = 0;
        mfWImag[3] = (mbInverse ? -1.0 : 1.0);
    }
    else if ((mnN % 4) == 0)
    {
        const SCSIZE nQSize = mnN >> 2;
        // Compute cos of the start quadrant.
        // This is the first quadrant if mbInverse == true, else it is the fourth quadrant.
        for (SCSIZE nIdx = 0; nIdx <= nQSize; ++nIdx)
            mfWReal[nIdx] = cos(nW*static_cast<double>(nIdx));

        if (mbInverse)
        {
            const SCSIZE nQ1End = nQSize;
            // First quadrant
            for (SCSIZE nIdx = 0; nIdx <= nQ1End; ++nIdx)
                mfWImag[nIdx] = mfWReal[nQ1End-nIdx];

            // Second quadrant
            const SCSIZE nQ2End = nQ1End << 1;
            for (SCSIZE nIdx = nQ1End+1; nIdx <= nQ2End; ++nIdx)
            {
                mfWReal[nIdx] = -mfWReal[nQ2End - nIdx];
                mfWImag[nIdx] =  mfWImag[nQ2End - nIdx];
            }

            // Third quadrant
            const SCSIZE nQ3End = nQ2End + nQ1End;
            for (SCSIZE nIdx = nQ2End+1; nIdx <= nQ3End; ++nIdx)
            {
                mfWReal[nIdx] = -mfWReal[nIdx - nQ2End];
                mfWImag[nIdx] = -mfWImag[nIdx - nQ2End];
            }

            // Fourth Quadrant
            for (SCSIZE nIdx = nQ3End+1; nIdx < mnN; ++nIdx)
            {
                mfWReal[nIdx] =  mfWReal[mnN - nIdx];
                mfWImag[nIdx] = -mfWImag[mnN - nIdx];
            }
        }
        else
        {
            const SCSIZE nQ4End = nQSize;
            const SCSIZE nQ3End = nQSize << 1;
            const SCSIZE nQ2End = nQ3End + nQSize;

            // Fourth quadrant.
            for (SCSIZE nIdx = 0; nIdx <= nQ4End; ++nIdx)
                mfWImag[nIdx] = -mfWReal[nQ4End - nIdx];

            // Third quadrant.
            for (SCSIZE nIdx = nQ4End+1; nIdx <= nQ3End; ++nIdx)
            {
                mfWReal[nIdx] = -mfWReal[nQ3End - nIdx];
                mfWImag[nIdx] =  mfWImag[nQ3End - nIdx];
            }

            // Second quadrant.
            for (SCSIZE nIdx = nQ3End+1; nIdx <= nQ2End; ++nIdx)
            {
                mfWReal[nIdx] = -mfWReal[nIdx - nQ3End];
                mfWImag[nIdx] = -mfWImag[nIdx - nQ3End];
            }

            // First quadrant.
            for (SCSIZE nIdx = nQ2End+1; nIdx < mnN; ++nIdx)
            {
                mfWReal[nIdx] =  mfWReal[mnN - nIdx];
                mfWImag[nIdx] = -mfWImag[mnN - nIdx];
            }
        }
    }
    else
    {
        for (SCSIZE nIdx = 0; nIdx < mnN; ++nIdx)
        {
            double fAngle = nW*static_cast<double>(nIdx);
            mfWReal[nIdx] = cos(fAngle);
            mfWImag[nIdx] = sin(fAngle);
        }
    }
}

// A radix-2 decimation in time FFT algorithm for complex valued input.
class ScComplexFFT2
{
public:
    // rfArray.size() would always be even and a power of 2. (asserted in prepare())
    // rfArray's first half contains the real parts and the later half contains the imaginary parts.
    ScComplexFFT2(std::vector<double>& raArray, bool bInverse, bool bPolar, ScTwiddleFactors& rTF, bool bSubSampleTFs = false, bool bDisableNormalize = false) :
        mrArray(raArray),
        mfWReal(rTF.mfWReal),
        mfWImag(rTF.mfWImag),
        mnPoints(raArray.size()/2),
        mnStages(0),
        mbInverse(bInverse),
        mbPolar(bPolar),
        mbDisableNormalize(bDisableNormalize),
        mbSubSampleTFs(bSubSampleTFs)
    {}

    void Compute();

private:

    void prepare();

    double getReal(SCSIZE nIdx)
    {
        return mrArray[nIdx];
    }

    void setReal(double fVal, SCSIZE nIdx)
    {
        mrArray[nIdx] = fVal;
    }

    double getImag(SCSIZE nIdx)
    {
        return mrArray[mnPoints + nIdx];
    }

    void setImag(double fVal, SCSIZE nIdx)
    {
        mrArray[mnPoints + nIdx] = fVal;
    }

    SCSIZE getTFactorIndex(SCSIZE nPtIndex, SCSIZE nTfIdxScaleBits)
    {
        return ( ( nPtIndex << nTfIdxScaleBits ) & ( mnPoints - 1 ) ); // (x & (N-1)) is same as (x % N) but faster.
    }

    void computeFly(SCSIZE nTopIdx, SCSIZE nBottomIdx, SCSIZE nWIdx1, SCSIZE nWIdx2)
    {
        if (mbSubSampleTFs)
        {
            nWIdx1 <<= 1;
            nWIdx2 <<= 1;
        }

        const double x1r = getReal(nTopIdx);
        const double x2r = getReal(nBottomIdx);

        const double& w1r = mfWReal[nWIdx1];
        const double& w1i = mfWImag[nWIdx1];

        const double& w2r = mfWReal[nWIdx2];
        const double& w2i = mfWImag[nWIdx2];

        const double x1i = getImag(nTopIdx);
        const double x2i = getImag(nBottomIdx);

        setReal(x1r + x2r*w1r - x2i*w1i, nTopIdx);
        setImag(x1i + x2i*w1r + x2r*w1i, nTopIdx);

        setReal(x1r + x2r*w2r - x2i*w2i, nBottomIdx);
        setImag(x1i + x2i*w2r + x2r*w2i, nBottomIdx);
    }

    std::vector<double>& mrArray;
    std::vector<double>& mfWReal;
    std::vector<double>& mfWImag;
    SCSIZE mnPoints;
    SCSIZE mnStages;
    bool mbInverse:1;
    bool mbPolar:1;
    bool mbDisableNormalize:1;
    bool mbSubSampleTFs:1;
};

void ScComplexFFT2::prepare()
{
    SCSIZE nPoints = mnPoints;
    lcl_roundUpNearestPow2(nPoints, mnStages);
    assert(nPoints == mnPoints);

    // Reorder array by bit-reversed indices.
    for (SCSIZE nIdx = 0; nIdx < mnPoints; ++nIdx)
    {
        SCSIZE nRevIdx = lcl_bitReverse(nIdx, mnPoints);
        if (nIdx < nRevIdx)
        {
            double fTmp = getReal(nIdx);
            setReal(getReal(nRevIdx), nIdx);
            setReal(fTmp, nRevIdx);

            fTmp = getImag(nIdx);
            setImag(getImag(nRevIdx), nIdx);
            setImag(fTmp, nRevIdx);
        }
    }
}

static void lcl_normalize(std::vector<double>& rCmplxArray, bool bScaleOnlyReal)
{
    const SCSIZE nPoints = rCmplxArray.size()/2;
    const double fScale = 1.0/static_cast<double>(nPoints);

    // Scale the real part
    for (SCSIZE nIdx = 0; nIdx < nPoints; ++nIdx)
        rCmplxArray[nIdx] *= fScale;

    if (!bScaleOnlyReal)
    {
        const SCSIZE nLen = nPoints*2;
        for (SCSIZE nIdx = nPoints; nIdx < nLen; ++nIdx)
            rCmplxArray[nIdx] *= fScale;
    }
}

static void lcl_convertToPolar(std::vector<double>& rCmplxArray)
{
    const SCSIZE nPoints = rCmplxArray.size()/2;
    double fMag, fPhase, fR, fI;
    for (SCSIZE nIdx = 0; nIdx < nPoints; ++nIdx)
    {
        fR = rCmplxArray[nIdx];
        fI = rCmplxArray[nPoints+nIdx];
        fPhase = atan2(fI, fR);
        fMag = sqrt(fR*fR + fI*fI);

        rCmplxArray[nIdx] = fMag;
        rCmplxArray[nPoints+nIdx] = fPhase;
    }
}

void ScComplexFFT2::Compute()
{
    prepare();

    const SCSIZE nFliesInStage = mnPoints/2;
    for (SCSIZE nStage = 0; nStage < mnStages; ++nStage)
    {
        const SCSIZE nTFIdxScaleBits = mnStages - nStage - 1;  // Twiddle factor index's scale factor in bits.
        const SCSIZE nFliesInGroup = SCSIZE(1) << nStage;
        const SCSIZE nGroups = nFliesInStage/nFliesInGroup;
        const SCSIZE nFlyWidth = nFliesInGroup;
        for (SCSIZE nGroup = 0, nFlyTopIdx = 0; nGroup < nGroups; ++nGroup)
        {
            for (SCSIZE nFly = 0; nFly < nFliesInGroup; ++nFly, ++nFlyTopIdx)
            {
                SCSIZE nFlyBottomIdx = nFlyTopIdx + nFlyWidth;
                SCSIZE nWIdx1 = getTFactorIndex(nFlyTopIdx, nTFIdxScaleBits);
                SCSIZE nWIdx2 = getTFactorIndex(nFlyBottomIdx, nTFIdxScaleBits);

                computeFly(nFlyTopIdx, nFlyBottomIdx, nWIdx1, nWIdx2);
            }

            nFlyTopIdx += nFlyWidth;
        }
    }

    if (mbPolar)
        lcl_convertToPolar(mrArray);

    // Normalize after converting to polar, so we have a chance to
    // save O(mnPoints) flops.
    if (mbInverse && !mbDisableNormalize)
        lcl_normalize(mrArray, mbPolar);
}

// Bluestein's algorithm or chirp z-transform algorithm that can be used to
// compute DFT of a complex valued input of any length N in O(N lgN) time.
class ScComplexBluesteinFFT
{
public:

    ScComplexBluesteinFFT(std::vector<double>& rArray, bool bReal, bool bInverse, bool bPolar, bool bDisableNormalize = false) :
        mrArray(rArray),
        mnPoints(rArray.size()/2), // rArray should have space for imaginary parts even if real input.
        mbReal(bReal),
        mbInverse(bInverse),
        mbPolar(bPolar),
        mbDisableNormalize(bDisableNormalize)
    {}

    void Compute();

private:
    std::vector<double>& mrArray;
    const SCSIZE mnPoints;
    bool mbReal:1;
    bool mbInverse:1;
    bool mbPolar:1;
    bool mbDisableNormalize:1;
};

void ScComplexBluesteinFFT::Compute()
{
    std::vector<double> aRealScalars(mnPoints);
    std::vector<double> aImagScalars(mnPoints);
    double fW = (mbInverse ? 2 : -2)*F_PI/static_cast<double>(mnPoints);
    for (SCSIZE nIdx = 0; nIdx < mnPoints; ++nIdx)
    {
        double fAngle = 0.5*fW*static_cast<double>(nIdx*nIdx);
        aRealScalars[nIdx] = cos(fAngle);
        aImagScalars[nIdx] = sin(fAngle);
    }

    SCSIZE nMinSize = mnPoints*2 - 1;
    SCSIZE nExtendedLength = nMinSize, nTmp = 0;
    lcl_roundUpNearestPow2(nExtendedLength, nTmp);
    std::vector<double> aASignal(nExtendedLength*2); // complex valued
    std::vector<double> aBSignal(nExtendedLength*2); // complex valued

    double fReal, fImag;
    for (SCSIZE nIdx = 0; nIdx < mnPoints; ++nIdx)
    {
        // Real part of A signal.
        aASignal[nIdx] = mrArray[nIdx]*aRealScalars[nIdx] + (mbReal ? 0.0 : -mrArray[mnPoints+nIdx]*aImagScalars[nIdx]);
        // Imaginary part of A signal.
        aASignal[nExtendedLength + nIdx] = mrArray[nIdx]*aImagScalars[nIdx] + (mbReal ? 0.0 : mrArray[mnPoints+nIdx]*aRealScalars[nIdx]);

        // Real part of B signal.
        aBSignal[nIdx] = fReal = aRealScalars[nIdx];
        // Imaginary part of B signal.
        aBSignal[nExtendedLength + nIdx] = fImag = -aImagScalars[nIdx]; // negative sign because B signal is the conjugation of the scalars.

        if (nIdx)
        {
            // B signal needs a mirror of its part in 0 < n < mnPoints at the tail end.
            aBSignal[nExtendedLength - nIdx] = fReal;
            aBSignal[(nExtendedLength<<1) - nIdx] = fImag;
        }
    }

    {
        ScTwiddleFactors aTF(nExtendedLength, false /*not inverse*/);
        aTF.Compute();

        // Do complex-FFT2 of both A and B signal.
        ScComplexFFT2 aFFT2A(aASignal, false /*not inverse*/, false /*no polar*/, aTF, false /*no subsample*/, true /* disable normalize */);
        aFFT2A.Compute();

        ScComplexFFT2 aFFT2B(aBSignal, false /*not inverse*/, false /*no polar*/, aTF, false /*no subsample*/, true /* disable normalize */);
        aFFT2B.Compute();

        double fAR, fAI, fBR, fBI;
        for (SCSIZE nIdx = 0; nIdx < nExtendedLength; ++nIdx)
        {
            fAR = aASignal[nIdx];
            fAI = aASignal[nExtendedLength + nIdx];
            fBR = aBSignal[nIdx];
            fBI = aBSignal[nExtendedLength + nIdx];

            // Do point-wise product inplace in A signal.
            aASignal[nIdx] = fAR*fBR - fAI*fBI;
            aASignal[nExtendedLength + nIdx] = fAR*fBI + fAI*fBR;
        }

        // Do complex-inverse-FFT2 of aASignal.
        aTF.Conjugate();
        ScComplexFFT2 aFFT2AI(aASignal, true /*inverse*/, false /*no polar*/, aTF); // Need normalization here.
        aFFT2AI.Compute();
    }

    // Point-wise multiply with scalars.
    for (SCSIZE nIdx = 0; nIdx < mnPoints; ++nIdx)
    {
        fReal = aASignal[nIdx];
        fImag = aASignal[nExtendedLength + nIdx];
        mrArray[nIdx] = fReal*aRealScalars[nIdx] - fImag*aImagScalars[nIdx]; // no conjugation needed here.
        mrArray[mnPoints + nIdx] = fReal*aImagScalars[nIdx] + fImag*aRealScalars[nIdx];
    }

    // Normalize/Polar operations
    if (mbPolar)
        lcl_convertToPolar(mrArray);

    // Normalize after converting to polar, so we have a chance to
    // save O(mnPoints) flops.
    if (mbInverse && !mbDisableNormalize)
        lcl_normalize(mrArray, mbPolar);
}

// Computes DFT of an even length(N) real-valued input by using a
// ScComplexFFT2 if N == 2^k for some k or else by using a ScComplexBluesteinFFT
// with an complex valued input of length = N/2.
class ScRealFFT
{
public:

    ScRealFFT(std::vector<double>& rInArray, std::vector<double>& rOutArray, bool bInverse, bool bPolar) :
        mrInArray(rInArray),
        mrOutArray(rOutArray),
        mbInverse(bInverse),
        mbPolar(bPolar)
    {}

    void Compute();

private:
    std::vector<double>& mrInArray;
    std::vector<double>& mrOutArray;
    bool mbInverse:1;
    bool mbPolar:1;
};

void ScRealFFT::Compute()
{
    // input length has to be even to do this optimization.
    assert(mrInArray.size() % 2 == 0);
    assert(mrInArray.size()*2 == mrOutArray.size());
    // nN is the number of points in the complex-fft input
    // which will be half of the number of points in real array.
    const SCSIZE nN = mrInArray.size()/2;
    if (nN == 0)
    {
        mrOutArray[0] = mrInArray[0];
        mrOutArray[1] = 0.0;
        return;
    }

    // work array should be the same length as mrInArray
    std::vector<double> aWorkArray(nN*2);
    for (SCSIZE nIdx = 0; nIdx < nN; ++nIdx)
    {
        SCSIZE nDoubleIdx = 2*nIdx;
        // Use even elements as real part
        aWorkArray[nIdx] = mrInArray[nDoubleIdx];
        // and odd elements as imaginary part of the contrived complex sequence.
        aWorkArray[nN+nIdx] = mrInArray[nDoubleIdx+1];
    }

    ScTwiddleFactors aTFs(nN*2, mbInverse);
    aTFs.Compute();
    SCSIZE nNextPow2 = nN, nTmp = 0;
    lcl_roundUpNearestPow2(nNextPow2, nTmp);

    if (nNextPow2 == nN)
    {
        ScComplexFFT2 aFFT2(aWorkArray, mbInverse, false /*disable polar*/,
                            aTFs, true /*subsample tf*/, true /*disable normalize*/);
        aFFT2.Compute();
    }
    else
    {
        ScComplexBluesteinFFT aFFT(aWorkArray, false /*complex input*/, mbInverse, false /*disable polar*/,
                                   true /*disable normalize*/);
        aFFT.Compute();
    }

    // Post process aWorkArray to populate mrOutArray

    const SCSIZE nTwoN = 2*nN, nThreeN = 3*nN;
    double fY1R, fY2R, fY1I, fY2I, fResR, fResI, fWR, fWI;
    for (SCSIZE nIdx = 0; nIdx < nN; ++nIdx)
    {
        const SCSIZE nIdxRev = nIdx ? (nN - nIdx) : 0;
        fY1R = aWorkArray[nIdx];
        fY2R = aWorkArray[nIdxRev];
        fY1I = aWorkArray[nN + nIdx];
        fY2I = aWorkArray[nN + nIdxRev];
        fWR  = aTFs.mfWReal[nIdx];
        fWI  = aTFs.mfWImag[nIdx];

        // mrOutArray has length = 4*nN
        // Real part of the final output (only half of the symmetry around Nyquist frequency)
        // Fills the first quarter.
        mrOutArray[nIdx] = fResR = 0.5*(
            fY1R + fY2R +
            fWR * (fY1I + fY2I) +
            fWI * (fY1R - fY2R) );
        // Imaginary part of the final output (only half of the symmetry around Nyquist frequency)
        // Fills the third quarter.
        mrOutArray[nTwoN + nIdx] = fResI = 0.5*(
            fY1I - fY2I +
            fWI * (fY1I + fY2I) -
            fWR * (fY1R - fY2R) );

        // Fill the missing 2 quarters using symmetry argument.
        if (nIdx)
        {
            // Fills the 2nd quarter.
            mrOutArray[nN + nIdxRev] = fResR;
            // Fills the 4th quarter.
            mrOutArray[nThreeN + nIdxRev] = -fResI;
        }
        else
        {
            mrOutArray[nN] = fY1R - fY1I;
            mrOutArray[nThreeN] = 0.0;
        }
    }

    // Normalize/Polar operations
    if (mbPolar)
        lcl_convertToPolar(mrOutArray);

    // Normalize after converting to polar, so we have a chance to
    // save O(mnPoints) flops.
    if (mbInverse)
        lcl_normalize(mrOutArray, mbPolar);
}

using ScMatrixGenerator = ScMatrixRef(SCSIZE, SCSIZE, std::vector<double>&);

// Generic FFT class that decides which FFT implementation to use.
class ScFFT
{
public:

    ScFFT(ScMatrixRef& pMat, bool bReal, bool bInverse, bool bPolar) :
        mpInputMat(pMat),
        mbReal(bReal),
        mbInverse(bInverse),
        mbPolar(bPolar)
    {}

    ScMatrixRef Compute(std::function<ScMatrixGenerator>& rMatGenFunc);

private:
    ScMatrixRef& mpInputMat;
    bool mbReal:1;
    bool mbInverse:1;
    bool mbPolar:1;
};

ScMatrixRef ScFFT::Compute(std::function<ScMatrixGenerator>& rMatGenFunc)
{
    std::vector<double> aArray;
    mpInputMat->GetDoubleArray(aArray);
    SCSIZE nPoints = mbReal ? aArray.size() : (aArray.size()/2);
    if (nPoints == 1)
    {
        mpInputMat->Resize(2, 1, 0.0);
        return mpInputMat;
    }

    if (mbReal && (nPoints % 2) == 0)
    {
        std::vector<double> aOutArray(nPoints*2);
        ScRealFFT aFFT(aArray, aOutArray, mbInverse, mbPolar);
        aFFT.Compute();
        return rMatGenFunc(2, nPoints, aOutArray);
    }

    SCSIZE nNextPow2 = nPoints, nTmp = 0;
    lcl_roundUpNearestPow2(nNextPow2, nTmp);
    if (nNextPow2 == nPoints && !mbReal)
    {
        ScTwiddleFactors aTF(nPoints, mbInverse);
        aTF.Compute();
        ScComplexFFT2 aFFT2(aArray, mbInverse, mbPolar, aTF);
        aFFT2.Compute();
        return rMatGenFunc(2, nPoints, aArray);
    }

    if (mbReal)
        aArray.resize(nPoints*2, 0.0);
    ScComplexBluesteinFFT aFFT(aArray, mbReal, mbInverse, mbPolar);
    aFFT.Compute();
    return rMatGenFunc(2, nPoints, aArray);
}

void ScInterpreter::ScFourier()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 2, 4 ) )
        return;

    bool bInverse = false;
    bool bPolar = false;

    if (nParamCount == 4)
        bPolar = GetBool();

    if (nParamCount > 2)
    {
        if (IsMissing())
            Pop();
        else
            bInverse = GetBool();
    }

    bool bGroupedByColumn = GetBool();

    ScMatrixRef pInputMat = GetMatrix();
    if (!pInputMat)
    {
        PushIllegalParameter();
        return;
    }

    SCSIZE nC, nR;
    pInputMat->GetDimensions(nC, nR);

    if ((bGroupedByColumn && nC > 2) || (!bGroupedByColumn && nR > 2))
    {
        // There can be no more than 2 columns (real, imaginary) if data grouped by columns.
        // and no more than 2 rows if data is grouped by rows.
        PushIllegalArgument();
        return;
    }

    if (!pInputMat->IsNumeric())
    {
        PushNoValue();
        return;
    }

    bool bRealInput = true;
    if (!bGroupedByColumn)
    {
        pInputMat->MatTrans(*pInputMat);
        bRealInput = (nR == 1);
    }
    else
    {
        bRealInput = (nC == 1);
    }

    ScFFT aFFT(pInputMat, bRealInput, bInverse, bPolar);
    std::function<ScMatrixGenerator> aFunc = [this](SCSIZE nCol, SCSIZE nRow, std::vector<double>& rVec) -> ScMatrixRef
    {
        return this->GetNewMat(nCol, nRow, rVec);
    };
    ScMatrixRef pOut = aFFT.Compute(aFunc);
    PushMatrix(pOut);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
