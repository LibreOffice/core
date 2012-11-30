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


#include <rtl/logfile.hxx>
#include "interpre.hxx"

double const fHalfMachEps = 0.5 * ::std::numeric_limits<double>::epsilon();

// The idea how this group of gamma functions is calculated, is
// based on the Cephes library
// online http://www.moshier.net/#Cephes [called 2008-02]

/** You must ensure fA>0.0 && fX>0.0
    valid results only if fX > fA+1.0
    uses continued fraction with odd items */
double ScInterpreter::GetGammaContFraction( double fA, double fX )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "sc", "er", "ScInterpreter::GetGammaContFraction" );

    double const fBigInv = ::std::numeric_limits<double>::epsilon();
    double const fBig = 1.0/fBigInv;
    double fCount = 0.0;
    double fNum = 0.0;  // dummy value
    double fY = 1.0 - fA;
    double fDenom = fX + 2.0-fA;
    double fPk = 0.0;   // dummy value
    double fPkm1 = fX + 1.0;
    double fPkm2 = 1.0;
    double fQk = 1.0;   // dummy value
    double fQkm1 = fDenom * fX;
    double fQkm2 = fX;
    double fApprox = fPkm1/fQkm1;
    bool bFinished = false;
    double fR = 0.0;    // dummy value
    do
    {
        fCount = fCount +1.0;
        fY = fY+ 1.0;
        fNum = fY * fCount;
        fDenom = fDenom +2.0;
        fPk = fPkm1 * fDenom  -  fPkm2 * fNum;
        fQk = fQkm1 * fDenom  -  fQkm2 * fNum;
        if (fQk != 0.0)
        {
            fR = fPk/fQk;
            bFinished = (fabs( (fApprox - fR)/fR ) <= fHalfMachEps);
            fApprox = fR;
        }
        fPkm2 = fPkm1;
        fPkm1 = fPk;
        fQkm2 = fQkm1;
        fQkm1 = fQk;
        if (fabs(fPk) > fBig)
        {
            // reduce a fraction does not change the value
            fPkm2 = fPkm2 * fBigInv;
            fPkm1 = fPkm1 * fBigInv;
            fQkm2 = fQkm2 * fBigInv;
            fQkm1 = fQkm1 * fBigInv;
        }
    } while (!bFinished && fCount<10000);
    // most iterations, if fX==fAlpha+1.0; approx sqrt(fAlpha) iterations then
    if (!bFinished)
    {
        SetError(errNoConvergence);
    }
    return fApprox;
}

/** You must ensure fA>0.0 && fX>0.0
    valid results only if fX <= fA+1.0
    uses power series */
double ScInterpreter::GetGammaSeries( double fA, double fX )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "sc", "er", "ScInterpreter::GetGammaSeries" );
    double fDenomfactor = fA;
    double fSummand = 1.0/fA;
    double fSum = fSummand;
    int nCount=1;
    do
    {
        fDenomfactor = fDenomfactor + 1.0;
        fSummand = fSummand * fX/fDenomfactor;
        fSum = fSum + fSummand;
        nCount = nCount+1;
    } while ( fSummand/fSum > fHalfMachEps && nCount<=10000);
    // large amount of iterations will be carried out for huge fAlpha, even
    // if fX <= fAlpha+1.0
    if (nCount>10000)
    {
        SetError(errNoConvergence);
    }
    return fSum;
}

/** You must ensure fA>0.0 && fX>0.0) */
double ScInterpreter::GetLowRegIGamma( double fA, double fX )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "sc", "er", "ScInterpreter::GetLowRegIGamma" );
    double fLnFactor = fA * log(fX) - fX - GetLogGamma(fA);
    double fFactor = exp(fLnFactor);    // Do we need more accuracy than exp(ln()) has?
    if (fX>fA+1.0)  // includes fX>1.0; 1-GetUpRegIGamma, continued fraction
        return 1.0 - fFactor * GetGammaContFraction(fA,fX);
    else            // fX<=1.0 || fX<=fA+1.0, series
        return fFactor * GetGammaSeries(fA,fX);
}

/** You must ensure fA>0.0 && fX>0.0) */
double ScInterpreter::GetUpRegIGamma( double fA, double fX )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "sc", "er", "ScInterpreter::GetUpRegIGamma" );

    double fLnFactor= fA*log(fX)-fX-GetLogGamma(fA);
    double fFactor = exp(fLnFactor); //Do I need more accuracy than exp(ln()) has?;
    if (fX>fA+1.0) // includes fX>1.0
            return fFactor * GetGammaContFraction(fA,fX);
    else //fX<=1 || fX<=fA+1, 1-GetLowRegIGamma, series
            return 1.0 -fFactor * GetGammaSeries(fA,fX);
}

/** Gamma distribution, probability density function.
    fLambda is "scale" parameter
    You must ensure fAlpha>0.0 and fLambda>0.0 */
double ScInterpreter::GetGammaDistPDF( double fX, double fAlpha, double fLambda )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "sc", "er", "ScInterpreter::GetGammaDistPDF" );
    if (fX < 0.0)
        return 0.0;     // see ODFF
    else if (fX == 0)
        // in this case 0^0 isn't zero
    {
        if (fAlpha < 1.0)
        {
            SetError(errDivisionByZero);  // should be #DIV/0
            return HUGE_VAL;
        }
        else if (fAlpha == 1)
        {
            return (1.0 / fLambda);
        }
        else
        {
            return 0.0;
        }
    }
    else
    {
        double fXr = fX / fLambda;
        // use exp(ln()) only for large arguments because of less accuracy
        if (fXr > 1.0)
        {
            const double fLogDblMax = log( ::std::numeric_limits<double>::max());
            if (log(fXr) * (fAlpha-1.0) < fLogDblMax && fAlpha < fMaxGammaArgument)
            {
                return pow( fXr, fAlpha-1.0) * exp(-fXr) / fLambda / GetGamma(fAlpha);
            }
            else
            {
                return exp( (fAlpha-1.0) * log(fXr) - fXr - log(fLambda) - GetLogGamma(fAlpha));
            }
        }
        else    // fXr near to zero
        {
            if (fAlpha<fMaxGammaArgument)
            {
                return pow( fXr, fAlpha-1.0) * exp(-fXr) / fLambda / GetGamma(fAlpha);
            }
            else
            {
                return pow( fXr, fAlpha-1.0) * exp(-fXr) / fLambda / exp( GetLogGamma(fAlpha));
            }
        }
    }
}

/** Gamma distribution, cumulative distribution function.
    fLambda is "scale" parameter
    You must ensure fAlpha>0.0 and fLambda>0.0 */
double ScInterpreter::GetGammaDist( double fX, double fAlpha, double fLambda )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "sc", "er", "ScInterpreter::GetGammaDist" );
    if (fX <= 0.0)
        return 0.0;
    else
        return GetLowRegIGamma( fAlpha, fX / fLambda);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
