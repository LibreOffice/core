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

#include <utility>
#include <xmloff/ProgressBarHelper.hxx>

#include <sal/log.hxx>

using namespace ::com::sun::star;

const sal_Int32 nDefaultProgressBarRange = 1000000;
const float fProgressStep = 0.5;

ProgressBarHelper::ProgressBarHelper(css::uno::Reference < css::task::XStatusIndicator> xTempStatusIndicator,
                                    const bool bTempStrict)
: m_xStatusIndicator(std::move(xTempStatusIndicator))
, m_nRange(nDefaultProgressBarRange)
, m_nReference(100)
, m_nValue(0)
, m_fOldPercent(0.0)
, m_bStrict(bTempStrict)
, m_bRepeat(true)
#ifdef DBG_UTIL
, m_bFailure(false)
#endif
{
}

ProgressBarHelper::~ProgressBarHelper()
{
}

void ProgressBarHelper::ChangeReference(sal_Int32 nNewReference)
{
    if((nNewReference <= 0) || (nNewReference == m_nReference))
        return;

    if (m_nReference)
    {
        double fPercent(static_cast<double>(nNewReference) / m_nReference);
        double fValue(m_nValue * fPercent);
        m_nValue = static_cast<sal_Int32>(fValue);
        m_nReference = nNewReference;
    }
    else
    {
        m_nReference = nNewReference;
        m_nValue = 0;
    }
}

void ProgressBarHelper::SetValue(sal_Int32 nTempValue)
{
    if (!m_xStatusIndicator.is() || (m_nReference <= 0))
        return;

    if ((nTempValue >= m_nValue) && (!m_bStrict || (nTempValue <= m_nReference)))
    {
        // #91317# no progress bar with values > 100%
        if (nTempValue > m_nReference)
        {
            if (!m_bRepeat)
                m_nValue = m_nReference;
            else
            {
                m_xStatusIndicator->reset();
                m_nValue = 0;
            }
        }
        else
            m_nValue = nTempValue;

        double fValue(m_nValue);
        double fNewValue ((fValue * m_nRange) / m_nReference);

        double fPercent((fNewValue * 100) / m_nRange);
        if (fPercent >= (m_fOldPercent + fProgressStep) || fPercent < m_fOldPercent)
        {
            m_xStatusIndicator->setValue(static_cast<sal_Int32>(fNewValue));
            m_fOldPercent = fPercent;
        }
    }
#ifdef DBG_UTIL
    else if (!m_bFailure)
    {
        SAL_WARN("xmloff", "tried to set a wrong value (" << nTempValue << ") on the progressbar");
        m_bFailure = true;
    }
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
