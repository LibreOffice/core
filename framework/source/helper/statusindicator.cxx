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
    throw(css::uno::RuntimeException, std::exception)
{
    if (comphelper::LibreOfficeKit::isActive())
    {
        m_nRange = nRange;
        m_nLastCallbackPercent = -1;

        comphelper::LibreOfficeKit::statusIndicatorStart();
    }

    css::uno::Reference< css::task::XStatusIndicatorFactory > xFactory(m_xFactory);
    if (xFactory.is())
    {
        StatusIndicatorFactory* pFactory = static_cast<StatusIndicatorFactory*>(xFactory.get());
        pFactory->start(this, sText, nRange);
    }
}

void SAL_CALL StatusIndicator::end()
    throw(css::uno::RuntimeException, std::exception)
{
    if (comphelper::LibreOfficeKit::isActive())
    {
        comphelper::LibreOfficeKit::statusIndicatorFinish();
    }

    css::uno::Reference< css::task::XStatusIndicatorFactory > xFactory(m_xFactory);
    if (xFactory.is())
    {
        StatusIndicatorFactory* pFactory = static_cast<StatusIndicatorFactory*>(xFactory.get());
        pFactory->end(this);
    }
}

void SAL_CALL StatusIndicator::reset()
    throw(css::uno::RuntimeException, std::exception)
{
    if (comphelper::LibreOfficeKit::isActive())
        return;

    css::uno::Reference< css::task::XStatusIndicatorFactory > xFactory(m_xFactory);
    if (xFactory.is())
    {
        StatusIndicatorFactory* pFactory = static_cast<StatusIndicatorFactory*>(xFactory.get());
        pFactory->reset(this);
    }
}

void SAL_CALL StatusIndicator::setText(const OUString& sText)
    throw(css::uno::RuntimeException, std::exception)
{
    if (comphelper::LibreOfficeKit::isActive())
        return;

    css::uno::Reference< css::task::XStatusIndicatorFactory > xFactory(m_xFactory);
    if (xFactory.is())
    {
        StatusIndicatorFactory* pFactory = static_cast<StatusIndicatorFactory*>(xFactory.get());
        pFactory->setText(this, sText);
    }
}

void SAL_CALL StatusIndicator::setValue(sal_Int32 nValue)
    throw(css::uno::RuntimeException, std::exception)
{
    if (comphelper::LibreOfficeKit::isActive())
    {
        int nPercent = (100*nValue)/m_nRange;
        if (nPercent != m_nLastCallbackPercent)
        {
            comphelper::LibreOfficeKit::statusIndicatorSetValue(nPercent);
            m_nLastCallbackPercent = nPercent;
        }
        return;
    }

    css::uno::Reference< css::task::XStatusIndicatorFactory > xFactory(m_xFactory);
    if (xFactory.is())
    {
        StatusIndicatorFactory* pFactory = static_cast<StatusIndicatorFactory*>(xFactory.get());
        pFactory->setValue(this, nValue);
    }
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
