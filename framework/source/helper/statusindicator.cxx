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

#include <comphelper/lok.hxx>
#include <helper/statusindicator.hxx>

namespace framework{

StatusIndicator::StatusIndicator(StatusIndicatorFactory* pFactory)
    : m_xFactory(pFactory)
    , m_nRange(100)
    , m_nLastCallbackPercent(-1)
{
}

StatusIndicator::~StatusIndicator()
{
}

void SAL_CALL StatusIndicator::start(const OUString& sText ,
                                     sal_Int32       nRange)
{
    if (comphelper::LibreOfficeKit::isActive())
    {
        m_nRange = nRange;
        m_nLastCallbackPercent = -1;

        comphelper::LibreOfficeKit::statusIndicatorStart();
    }
#if !defined(IOS) && !defined(ANDROID)
    css::uno::Reference< css::task::XStatusIndicatorFactory > xFactory(m_xFactory);
    if (xFactory.is())
    {
        StatusIndicatorFactory* pFactory = static_cast<StatusIndicatorFactory*>(xFactory.get());
        pFactory->start(this, sText, nRange);
    }
#else
    (void) sText;
#endif
}

void SAL_CALL StatusIndicator::end()
{
    if (comphelper::LibreOfficeKit::isActive())
    {
        comphelper::LibreOfficeKit::statusIndicatorFinish();
    }
#if !defined(IOS) && !defined(ANDROID)
    css::uno::Reference< css::task::XStatusIndicatorFactory > xFactory(m_xFactory);
    if (xFactory.is())
    {
        StatusIndicatorFactory* pFactory = static_cast<StatusIndicatorFactory*>(xFactory.get());
        pFactory->end(this);
    }
#endif
}

void SAL_CALL StatusIndicator::reset()
{
    if (comphelper::LibreOfficeKit::isActive())
        return;
#if !defined(IOS) && !defined(ANDROID)
    css::uno::Reference< css::task::XStatusIndicatorFactory > xFactory(m_xFactory);
    if (xFactory.is())
    {
        StatusIndicatorFactory* pFactory = static_cast<StatusIndicatorFactory*>(xFactory.get());
        pFactory->reset(this);
    }
#endif
}

void SAL_CALL StatusIndicator::setText(const OUString& sText)
{
    if (comphelper::LibreOfficeKit::isActive())
        return;
#if !defined(IOS) && !defined(ANDROID)
    css::uno::Reference< css::task::XStatusIndicatorFactory > xFactory(m_xFactory);
    if (xFactory.is())
    {
        StatusIndicatorFactory* pFactory = static_cast<StatusIndicatorFactory*>(xFactory.get());
        pFactory->setText(this, sText);
    }
#else
    (void) sText;
#endif
}

void SAL_CALL StatusIndicator::setValue(sal_Int32 nValue)
{
    if (comphelper::LibreOfficeKit::isActive())
    {
        int nPercent = (100*nValue)/m_nRange;
        if (nPercent >= m_nLastCallbackPercent)
        {
            comphelper::LibreOfficeKit::statusIndicatorSetValue(nPercent);
            m_nLastCallbackPercent = nPercent;
        }
        return;
    }
#if !defined(IOS) && !defined(ANDROID)
    css::uno::Reference< css::task::XStatusIndicatorFactory > xFactory(m_xFactory);
    if (xFactory.is())
    {
        StatusIndicatorFactory* pFactory = static_cast<StatusIndicatorFactory*>(xFactory.get());
        pFactory->setValue(this, nValue);
    }
#endif
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
