/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"
#include <helper/vclstatusindicator.hxx>

//-----------------------------------------------
// includes of own modules
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>

//-----------------------------------------------
// includes of interfaces

//-----------------------------------------------
// includes of external modules

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif
#include <vcl/svapp.hxx>

//-----------------------------------------------
// namespace

namespace framework {

//-----------------------------------------------
// definitions

//-----------------------------------------------
DEFINE_XINTERFACE_1(VCLStatusIndicator                           ,
                    OWeakObject                                  ,
                    DIRECT_INTERFACE(css::task::XStatusIndicator))

//-----------------------------------------------
VCLStatusIndicator::VCLStatusIndicator(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR        ,
                                       const css::uno::Reference< css::awt::XWindow >&               xParentWindow)
    : ThreadHelpBase     (&Application::GetSolarMutex())
    , ::cppu::OWeakObject(                             )
    , m_xSMGR            (xSMGR                        )
    , m_xParentWindow    (xParentWindow                )
    , m_pStatusBar       (0                            )
    , m_nRange           (0                            )
    , m_nValue           (0                            )
{
    if (!m_xParentWindow.is())
        throw css::uno::RuntimeException(
                ::rtl::OUString::createFromAscii("Cant work without a parent window!"),
                static_cast< css::task::XStatusIndicator* >(this));
}

//-----------------------------------------------
VCLStatusIndicator::~VCLStatusIndicator()
{
}

//-----------------------------------------------
void SAL_CALL VCLStatusIndicator::start(const ::rtl::OUString& sText ,
                                              sal_Int32        nRange)
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::awt::XWindow > xParentWindow = m_xParentWindow;
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    // SOLAR SAFE -> ----------------------------
    ::vos::OClearableGuard aSolarLock(Application::GetSolarMutex());

    Window* pParentWindow = VCLUnoHelper::GetWindow(xParentWindow);
    if (!m_pStatusBar)
        m_pStatusBar = new StatusBar(pParentWindow, WB_3DLOOK|WB_BORDER);

    VCLStatusIndicator::impl_recalcLayout(m_pStatusBar, pParentWindow);

    m_pStatusBar->Show();
    m_pStatusBar->StartProgressMode(sText);
    m_pStatusBar->SetProgressValue(0);

    // force repaint!
    pParentWindow->Show();
    pParentWindow->Invalidate(INVALIDATE_CHILDREN);
    pParentWindow->Flush();

    aSolarLock.clear();
    // <- SOLAR SAFE ----------------------------

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);
    m_sText  = sText;
    m_nRange = nRange;
    m_nValue = 0;
    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
void SAL_CALL VCLStatusIndicator::reset()
    throw(css::uno::RuntimeException)
{
    // SOLAR SAFE -> ----------------------------
    ::vos::OClearableGuard aSolarLock(Application::GetSolarMutex());
    if (m_pStatusBar)
    {
        m_pStatusBar->SetProgressValue(0);
        m_pStatusBar->SetText(String());
    }
    aSolarLock.clear();
    // <- SOLAR SAFE ----------------------------
}

//-----------------------------------------------
void SAL_CALL VCLStatusIndicator::end()
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);
    m_sText  = ::rtl::OUString();
    m_nRange = 0;
    m_nValue = 0;
    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    // SOLAR SAFE -> ----------------------------
    ::vos::OClearableGuard aSolarLock(Application::GetSolarMutex());
    if (m_pStatusBar)
    {
        m_pStatusBar->EndProgressMode();
        m_pStatusBar->Show(sal_False);

        delete m_pStatusBar;
        m_pStatusBar = 0;
    }
    aSolarLock.clear();
    // <- SOLAR SAFE ----------------------------
}

//-----------------------------------------------
void SAL_CALL VCLStatusIndicator::setText(const ::rtl::OUString& sText)
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);
    m_sText = sText;
    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    // SOLAR SAFE -> ----------------------------
    ::vos::OClearableGuard aSolarLock(Application::GetSolarMutex());
    if (m_pStatusBar)
        m_pStatusBar->SetText(sText);
    aSolarLock.clear();
    // <- SOLAR SAFE ----------------------------
}

//-----------------------------------------------
void SAL_CALL VCLStatusIndicator::setValue(sal_Int32 nValue)
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    if (nValue <= m_nRange)
        m_nValue = nValue;
    else
        m_nValue = m_nRange;

    sal_Int32 nRange = m_nRange;
              nValue = m_nValue;

    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    // normalize value to fit the range of 0-100 %
    sal_uInt16 nPercent = sal::static_int_cast< sal_uInt16 >(
        ::std::min(
            ((nValue*100) / ::std::max(nRange,(sal_Int32)1)), (sal_Int32)100));

    // SOLAR SAFE -> ----------------------------
    ::vos::OClearableGuard aSolarLock(Application::GetSolarMutex());
    if (m_pStatusBar)
        m_pStatusBar->SetProgressValue(nPercent);
    aSolarLock.clear();
    // <- SOLAR SAFE ----------------------------
}

//-----------------------------------------------
void VCLStatusIndicator::impl_recalcLayout(Window* pStatusBar   ,
                                           Window* pParentWindow)
{
    if (
        (!pStatusBar   ) ||
        (!pParentWindow)
       )
       return;

    Size aParentSize = pParentWindow->GetSizePixel();
    pStatusBar->SetPosSizePixel(0,
                                0,
                                aParentSize.Width(),
                                aParentSize.Height());
}

} // namespace framework
