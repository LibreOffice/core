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

#include <helper/vclstatusindicator.hxx>

#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>

namespace framework {

VCLStatusIndicator::VCLStatusIndicator(const css::uno::Reference< css::awt::XWindow >&               xParentWindow)
    : m_xParentWindow    (xParentWindow                )
    , m_pStatusBar       (nullptr                            )
    , m_nRange           (0                            )
    , m_nValue           (0                            )
{
    if (!m_xParentWindow.is())
        throw css::uno::RuntimeException(
                "Can't work without a parent window!",
                static_cast< css::task::XStatusIndicator* >(this));
}

VCLStatusIndicator::~VCLStatusIndicator()
{
}

void SAL_CALL VCLStatusIndicator::start(const OUString& sText ,
                                              sal_Int32        nRange)
{
    SolarMutexGuard aSolarGuard;

    VclPtr<vcl::Window> pParentWindow = VCLUnoHelper::GetWindow(m_xParentWindow);
    if (!m_pStatusBar)
        m_pStatusBar = VclPtr<StatusBar>::Create(pParentWindow, WB_3DLOOK|WB_BORDER);

    VCLStatusIndicator::impl_recalcLayout(m_pStatusBar, pParentWindow);

    m_pStatusBar->Show();
    m_pStatusBar->StartProgressMode(sText);
    m_pStatusBar->SetProgressValue(0);

    // force repaint!
    pParentWindow->Show();
    pParentWindow->Invalidate(InvalidateFlags::Children);
    pParentWindow->Flush();

    m_nRange = nRange;
    m_nValue = 0;
}

void SAL_CALL VCLStatusIndicator::reset()
{
    SolarMutexGuard aSolarGuard;
    if (m_pStatusBar)
    {
        m_pStatusBar->SetProgressValue(0);
        m_pStatusBar->SetText(OUString());
    }
}

void SAL_CALL VCLStatusIndicator::end()
{
    SolarMutexGuard aSolarGuard;

    m_nRange = 0;
    m_nValue = 0;

    if (m_pStatusBar)
    {
        m_pStatusBar->EndProgressMode();
        m_pStatusBar->Show(false);

        m_pStatusBar.disposeAndClear();
    }
}

void SAL_CALL VCLStatusIndicator::setText(const OUString& sText)
{
    SolarMutexGuard aSolarGuard;
    if (m_pStatusBar)
        m_pStatusBar->SetText(sText);
}

void SAL_CALL VCLStatusIndicator::setValue(sal_Int32 nValue)
{
    SolarMutexGuard aSolarGuard;

    if (nValue <= m_nRange)
        m_nValue = nValue;
    else
        m_nValue = m_nRange;

    sal_Int32 nRange = m_nRange;
    nValue = m_nValue;

    // normalize value to fit the range of 0-100%
    sal_uInt16 nPercent = sal::static_int_cast< sal_uInt16 >(
        ::std::min(
            ((nValue*100) / ::std::max(nRange,sal_Int32(1))), sal_Int32(100)));

    if (m_pStatusBar)
        m_pStatusBar->SetProgressValue(nPercent);
}

void VCLStatusIndicator::impl_recalcLayout(vcl::Window* pStatusBar   ,
                                           vcl::Window const * pParentWindow)
{
    if (
        (!pStatusBar   ) ||
        (!pParentWindow)
       )
       return;

    Size aParentSize = pParentWindow->GetSizePixel();
    pStatusBar->setPosSizePixel(0,
                                0,
                                aParentSize.Width(),
                                aParentSize.Height());
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
