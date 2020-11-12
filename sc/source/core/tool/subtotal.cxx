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
#include <cfloat>

bool SubTotal::SafePlus(double& fVal1, double fVal2)
{
    bool bOk = true;
    SAL_MATH_FPEXCEPTIONS_OFF();
    fVal1 += fVal2;
    if (!std::isfinite(fVal1))
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
    if (!std::isfinite(fVal1))
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
    if (!std::isfinite(fVal1))
    {
        bOk = false;
        fVal1 = DBL_MAX;
    }
    return bOk;
}

void ScFunctionData::update(double fNewVal)
{
    if (mbError)
        return;

    switch (meFunc)
    {
        case SUBTOTAL_FUNC_SUM:
            if (!SubTotal::SafePlus(getValueRef(), fNewVal))
                mbError = true;
            break;
        case SUBTOTAL_FUNC_PROD:
            if (getCountRef() == 0) // copy first value (nVal is initialized to 0)
            {
                getValueRef() = fNewVal;
                getCountRef() = 1; // don't care about further count
            }
            else if (!SubTotal::SafeMult(getValueRef(), fNewVal))
                mbError = true;
            break;
        case SUBTOTAL_FUNC_CNT:
        case SUBTOTAL_FUNC_CNT2:
            ++getCountRef();
            break;
        case SUBTOTAL_FUNC_SELECTION_COUNT:
            getCountRef() += fNewVal;
            break;
        case SUBTOTAL_FUNC_AVE:
            if (!SubTotal::SafePlus(getValueRef(), fNewVal))
                mbError = true;
            else
                ++getCountRef();
            break;
        case SUBTOTAL_FUNC_MAX:
            if (getCountRef() == 0) // copy first value (nVal is initialized to 0)
            {
                getValueRef() = fNewVal;
                getCountRef() = 1; // don't care about further count
            }
            else if (fNewVal > getValueRef())
                getValueRef() = fNewVal;
            break;
        case SUBTOTAL_FUNC_MIN:
            if (getCountRef() == 0) // copy first value (nVal is initialized to 0)
            {
                getValueRef() = fNewVal;
                getCountRef() = 1; // don't care about further count
            }
            else if (fNewVal < getValueRef())
                getValueRef() = fNewVal;
            break;
        case SUBTOTAL_FUNC_VAR:
        case SUBTOTAL_FUNC_STD:
        case SUBTOTAL_FUNC_VARP:
        case SUBTOTAL_FUNC_STDP:
            maWelford.update(fNewVal);
            break;
        default:
            // unhandled unknown
            mbError = true;
    }
}

double ScFunctionData::getResult()
{
    if (mbError)
        return 0.0;

    double fRet = 0.0;
    switch (meFunc)
    {
        case SUBTOTAL_FUNC_CNT:
        case SUBTOTAL_FUNC_CNT2:
        case SUBTOTAL_FUNC_SELECTION_COUNT:
            fRet = getCountRef();
            break;
        case SUBTOTAL_FUNC_SUM:
        case SUBTOTAL_FUNC_MAX:
        case SUBTOTAL_FUNC_MIN:
            // Note that nVal is 0.0 for MAX and MIN if nCount==0, that's also
            // how it is defined in ODFF.
            fRet = getValueRef();
            break;
        case SUBTOTAL_FUNC_PROD:
            fRet = (getCountRef() > 0) ? getValueRef() : 0.0;
            break;
        case SUBTOTAL_FUNC_AVE:
            if (getCountRef() == 0)
                mbError = true;
            else
                fRet = getValueRef() / getCountRef();
            break;
        case SUBTOTAL_FUNC_VAR:
        case SUBTOTAL_FUNC_STD:
            if (maWelford.getCount() < 2)
                mbError = true;
            else
            {
                fRet = maWelford.getVarianceSample();
                if (fRet < 0.0)
                    mbError = true;
                else if (meFunc == SUBTOTAL_FUNC_STD)
                    fRet = sqrt(fRet);
            }
            break;
        case SUBTOTAL_FUNC_VARP:
        case SUBTOTAL_FUNC_STDP:
            if (maWelford.getCount() < 1)
                mbError = true;
            else if (maWelford.getCount() == 1)
                fRet = 0.0;
            else
            {
                fRet = maWelford.getVariancePopulation();
                if (fRet < 0.0)
                    mbError = true;
                else if (meFunc == SUBTOTAL_FUNC_STDP)
                    fRet = sqrt(fRet);
            }
            break;
        default:
            assert(!"unhandled unknown");
            mbError = true;
            break;
    }
    if (mbError)
        fRet = 0.0;
    return fRet;
}

void WelfordRunner::update(double fVal)
{
    ++mnCount;
    const double fDelta = fVal - mfMean;
    mfMean += fDelta / mnCount;
    mfM2 += fDelta * (fVal - mfMean);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
