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

#include <xmloff/ProgressBarHelper.hxx>

#include <osl/diagnose.h>

using namespace ::com::sun::star;

const sal_Int32 nDefaultProgressBarRange = 1000000;
const float fProgressStep = 0.5;

ProgressBarHelper::ProgressBarHelper(const css::uno::Reference < css::task::XStatusIndicator>& xTempStatusIndicator,
                                    const bool bTempStrict)
: xStatusIndicator(xTempStatusIndicator)
, nRange(nDefaultProgressBarRange)
, nReference(100)
, nValue(0)
, fOldPercent(0.0)
, bStrict(bTempStrict)
, bRepeat(true)
#ifdef DBG_UTIL
, bFailure(false)
#endif
{
}

ProgressBarHelper::~ProgressBarHelper()
{
}

void ProgressBarHelper::ChangeReference(sal_Int32 nNewReference)
{
    if((nNewReference <= 0) || (nNewReference == nReference))
        return;

    if (nReference)
    {
        double fPercent(static_cast<double>(nNewReference) / nReference);
        double fValue(nValue * fPercent);
        nValue = static_cast<sal_Int32>(fValue);
        nReference = nNewReference;
    }
    else
    {
        nReference = nNewReference;
        nValue = 0;
    }
}

void ProgressBarHelper::SetValue(sal_Int32 nTempValue)
{
    if (!xStatusIndicator.is() || (nReference <= 0))
        return;

    if ((nTempValue >= nValue) && (!bStrict || (nTempValue <= nReference)))
    {
        // #91317# no progress bar with values > 100%
        if (nTempValue > nReference)
        {
            if (!bRepeat)
                nValue = nReference;
            else
            {
                xStatusIndicator->reset();
                nValue = 0;
            }
        }
        else
            nValue = nTempValue;

        double fValue(nValue);
        double fNewValue ((fValue * nRange) / nReference);

        double fPercent((fNewValue * 100) / nRange);
        if (fPercent >= (fOldPercent + fProgressStep) || fPercent < fOldPercent)
        {
            xStatusIndicator->setValue(static_cast<sal_Int32>(fNewValue));
            fOldPercent = fPercent;
        }
    }
#ifdef DBG_UTIL
    else if (!bFailure)
    {
        OSL_FAIL("tried to set a wrong value on the progressbar");
        bFailure = true;
    }
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
