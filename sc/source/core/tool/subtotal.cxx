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

#include <subtotal.hxx>
#include <interpre.hxx>

bool SubTotal::SafePlus(double& fVal1, double fVal2)
{
    bool bOk = true;
    SAL_MATH_FPEXCEPTIONS_OFF();
    fVal1 += fVal2;
    if (!::rtl::math::isFinite(fVal1))
    {
        bOk = false;
        if (fVal2 > 0.0)
            fVal1 = DBL_MAX;
        else
            fVal1 = -DBL_MAX;
    }
    return bOk;
}

bool SubTotal::SafeMult(double& fVal1, double fVal2)
{
    bool bOk = true;
    SAL_MATH_FPEXCEPTIONS_OFF();
    fVal1 *= fVal2;
    if (!::rtl::math::isFinite(fVal1))
    {
        bOk = false;
        fVal1 = DBL_MAX;
    }
    return bOk;
}

bool SubTotal::SafeDiv(double& fVal1, double fVal2)
{
    bool bOk = true;
    SAL_MATH_FPEXCEPTIONS_OFF();
    fVal1 /= fVal2;
    if (!::rtl::math::isFinite(fVal1))
    {
        bOk = false;
        fVal1 = DBL_MAX;
    }
    return bOk;
}

void ScFunctionData::update( double fNewVal )
{
    if (bError)
        return;

    switch (eFunc)
    {
        case SUBTOTAL_FUNC_SUM:
            if (!SubTotal::SafePlus(nVal, fNewVal))
                bError = true;
        break;
        case SUBTOTAL_FUNC_PROD:
            if (nCount == 0)    // copy first value (nVal is initialized to 0)
            {
                nVal = fNewVal;
                nCount = 1;     // don't care about further count
            }
            else if (!SubTotal::SafeMult(nVal, fNewVal))
                bError = true;
        break;
        case SUBTOTAL_FUNC_CNT:
        case SUBTOTAL_FUNC_CNT2:
            ++nCount;
        break;
        case SUBTOTAL_FUNC_AVE:
            if (!SubTotal::SafePlus(nVal, fNewVal))
                bError = true;
            else
                ++nCount;
        break;
        case SUBTOTAL_FUNC_MAX:
            if (nCount == 0)    // copy first value (nVal is initialized to 0)
            {
                nVal = fNewVal;
                nCount = 1;     // don't care about further count
            }
            else if (fNewVal > nVal)
                nVal = fNewVal;
        break;
        case SUBTOTAL_FUNC_MIN:
            if (nCount == 0)    // copy first value (nVal is initialized to 0)
            {
                nVal = fNewVal;
                nCount = 1;     // don't care about further count
            }
            else if (fNewVal < nVal)
                nVal = fNewVal;
        break;
        case SUBTOTAL_FUNC_VAR:
        case SUBTOTAL_FUNC_STD:
        case SUBTOTAL_FUNC_VARP:
        case SUBTOTAL_FUNC_STDP:
            maWelford.update( fNewVal);
        break;
        default:
            // unhandled unknown
            bError = true;
    }
}

double ScFunctionData::getResult()
{
    if (bError)
        return 0.0;

    double fRet = 0.0;
    switch (eFunc)
    {
        case SUBTOTAL_FUNC_CNT:
        case SUBTOTAL_FUNC_CNT2:
            fRet = nCount;
        break;
        case SUBTOTAL_FUNC_SUM:
        case SUBTOTAL_FUNC_MAX:
        case SUBTOTAL_FUNC_MIN:
            // Note that nVal is 0.0 for MAX and MIN if nCount==0, that's also
            // how it is defined in ODFF.
            fRet = nVal;
        break;
        case SUBTOTAL_FUNC_PROD:
            fRet = (nCount > 0) ? nVal : 0.0;
        break;
        case SUBTOTAL_FUNC_AVE:
            if (nCount == 0)
                bError = true;
            else
                fRet = nVal / nCount;
        break;
        case SUBTOTAL_FUNC_VAR:
        case SUBTOTAL_FUNC_STD:
            if (maWelford.getCount() < 2)
                bError = true;
            else
            {
                fRet = maWelford.getVarianceSample();
                if (fRet < 0.0)
                    bError = true;
                else if (eFunc == SUBTOTAL_FUNC_STD)
                    fRet = sqrt( fRet);
            }
        break;
        case SUBTOTAL_FUNC_VARP:
        case SUBTOTAL_FUNC_STDP:
            if (maWelford.getCount() < 1)
                bError = true;
            else if (maWelford.getCount() == 1)
                fRet = 0.0;
            else
            {
                fRet = maWelford.getVariancePopulation();
                if (fRet < 0.0)
                    bError = true;
                else if (eFunc == SUBTOTAL_FUNC_STDP)
                    fRet = sqrt( fRet);
            }
        break;
        default:
            assert(!"unhandled unknown");
            bError = true;
        break;
    }
    if (bError)
        fRet = 0.0;
    return fRet;
}

void WelfordRunner::update( double fVal )
{
    ++nCount;
    const double fDelta = fVal - fMean;
    fMean += fDelta / nCount;
    fM2 += fDelta * (fVal - fMean);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
