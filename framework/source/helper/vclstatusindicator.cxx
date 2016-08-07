/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
#include <toolkit/helper/vclunohelper.hxx>
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
                ::rtl::OUString::createFromAscii("Can't work without a parent window!"),
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
