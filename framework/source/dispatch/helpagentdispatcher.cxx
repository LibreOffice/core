/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <dispatch/helpagentdispatcher.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <svtools/helpopt.hxx>
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>

namespace css = ::com::sun::star;

//........................................................................
namespace framework
{

//-----------------------------------------------
DEFINE_XINTERFACE_4(HelpAgentDispatcher                         ,
                    OWeakObject                                 ,
                    DIRECT_INTERFACE (css::lang::XTypeProvider ),
                    DIRECT_INTERFACE (css::frame::XDispatch    ),
                    DIRECT_INTERFACE (css::awt::XWindowListener),
                    DIRECT_INTERFACE (css::lang::XEventListener))

//-----------------------------------------------
DEFINE_XTYPEPROVIDER_2(HelpAgentDispatcher     ,
                       css::lang::XTypeProvider,
                       css::frame::XDispatch   )

//--------------------------------------------------------------------
HelpAgentDispatcher::HelpAgentDispatcher( const css::uno::Reference< css::frame::XFrame >& xParentFrame)
    : ThreadHelpBase    (&Application::GetSolarMutex())
    , m_sCurrentURL     (                             )
    , m_xContainerWindow(                             )
    , m_xAgentWindow    (                             )
    , m_aTimer          (                             )
    , m_xSelfHold       (                             )
{
    // It's required that this class has to be contructed with a valid frame.
    // And "valid" means: the frame must already bound to a valid container window.
    m_xContainerWindow = xParentFrame->getContainerWindow();
}

//--------------------------------------------------------------------
HelpAgentDispatcher::~HelpAgentDispatcher()
{
    implts_stopTimer();
    implts_ignoreCurrentURL();

    // Needed ... because it was create as "new VCLWindow()" ! Such windows must be disposed explicitly.
    css::uno::Reference< css::lang::XComponent > xAgentWindow(m_xAgentWindow, css::uno::UNO_QUERY);
    if (xAgentWindow.is())
        xAgentWindow->dispose();
}

//--------------------------------------------------------------------
void SAL_CALL HelpAgentDispatcher::dispatch(const css::util::URL&                                  aURL ,
                                            const css::uno::Sequence< css::beans::PropertyValue >&)
    throw(css::uno::RuntimeException)
{
    // silently drop the request if the new URL was marked to be ignored next time.
    sal_Int32 nAllowedToIgnore = SvtHelpOptions().getAgentIgnoreURLCounter(aURL.Complete);
    if (nAllowedToIgnore < 1)
        return;

    // stop the expiration timer for the old URL
    // The timer will add the old URL to the list of ignorable URLs.
    // So m_sCurrentURL must be set AFTER the timer was stopped !!!
    implts_stopTimer();

    // SAFE ->
    WriteGuard aWriteLock(m_aLock);
    m_sCurrentURL = aURL.Complete;
    aWriteLock.unlock();
    // <- SAFE

    // start the expiration timer for the new URL
    implts_startTimer();

    // make sure the agent window is shown
    implts_showAgentWindow();
}

//--------------------------------------------------------------------
void SAL_CALL HelpAgentDispatcher::addStatusListener(const css::uno::Reference< css::frame::XStatusListener >&,
                                                     const css::util::URL&)
    throw(css::uno::RuntimeException)
{
    // no status available
}

//--------------------------------------------------------------------
void SAL_CALL HelpAgentDispatcher::removeStatusListener(const css::uno::Reference< css::frame::XStatusListener >&,
                                                        const css::util::URL&)
    throw(css::uno::RuntimeException)
{
    // no status available
}

//--------------------------------------------------------------------
void SAL_CALL HelpAgentDispatcher::windowResized(const css::awt::WindowEvent&)
    throw(css::uno::RuntimeException)
{
    implts_positionAgentWindow();
}

//--------------------------------------------------------------------
void SAL_CALL HelpAgentDispatcher::windowMoved(const css::awt::WindowEvent&)
    throw(css::uno::RuntimeException)
{
    implts_positionAgentWindow();
}

//--------------------------------------------------------------------
void SAL_CALL HelpAgentDispatcher::windowShown(const css::lang::EventObject&)
    throw(css::uno::RuntimeException)
{
    implts_showAgentWindow();
}

//--------------------------------------------------------------------
void SAL_CALL HelpAgentDispatcher::windowHidden(const css::lang::EventObject&)
    throw(css::uno::RuntimeException)
{
    implts_hideAgentWindow();
}

//--------------------------------------------------------------------
void SAL_CALL HelpAgentDispatcher::disposing(const css::lang::EventObject& aEvent)
    throw(css::uno::RuntimeException)
{
    // SAFE ->
    WriteGuard aWriteLock(m_aLock);

    // Already disposed ?!
    if (! m_xContainerWindow.is())
        return;
    // Wrong broadcaster ?!
    if (aEvent.Source != m_xContainerWindow)
        return;

    css::uno::Reference< css::uno::XInterface > xSelfHoldUntilMethodEnds(static_cast< css::frame::XDispatch* >(this), css::uno::UNO_QUERY_THROW);
    m_xSelfHold.clear();

    aWriteLock.unlock();
    // <- SAFE

    implts_stopTimer();
    implts_hideAgentWindow();
    implts_ignoreCurrentURL();

    // SAFE ->
    aWriteLock.lock();
    m_xContainerWindow.clear();
    css::uno::Reference< css::lang::XComponent > xAgentWindow(m_xAgentWindow, css::uno::UNO_QUERY);
    m_xAgentWindow.clear();
    aWriteLock.unlock();
    // <- SAFE

    // Needed ... because it was create as "new VCLWindow()" ! Such windows must be disposed explicitly.
    if (xAgentWindow.is())
        xAgentWindow->dispose();
}

//--------------------------------------------------------------------
void HelpAgentDispatcher::helpRequested()
{
    implts_stopTimer();
    implts_hideAgentWindow();
    implts_acceptCurrentURL();
}

//-----------------------------------------------
void HelpAgentDispatcher::closeAgent()
{
    implts_stopTimer();
    implts_hideAgentWindow();
    implts_ignoreCurrentURL();
}

//--------------------------------------------------------------------
void HelpAgentDispatcher::implts_acceptCurrentURL()
{
    // SAFE ->
    WriteGuard aWriteLock(m_aLock);

    ::rtl::OUString sAcceptedURL  = m_sCurrentURL;
                    m_sCurrentURL = ::rtl::OUString();

    aWriteLock.unlock();
    // <- SAFE

    // We must make sure that this URL isnt marked as ignored by the user.
    // Otherwhise the user wont see the corresponding help content in the future.
    SvtHelpOptions().resetAgentIgnoreURLCounter(sAcceptedURL);

    // show the right help content
    // SOLAR SAFE ->
    {
        SolarMutexGuard aSolarLock;
        Help* pHelp = Application::GetHelp();
        if (pHelp)
            pHelp->Start(sAcceptedURL, NULL);
    }
    // <- SOLAR SAFE
}

//--------------------------------------------------------------------
void HelpAgentDispatcher::implts_ignoreCurrentURL()
{
    // SAFE ->
    WriteGuard aWriteLock(m_aLock);

    ::rtl::OUString sIgnoredURL   = m_sCurrentURL;
                    m_sCurrentURL = ::rtl::OUString();

    aWriteLock.unlock();
    // <- SAFE

    if (sIgnoredURL.getLength())
        SvtHelpOptions().decAgentIgnoreURLCounter(sIgnoredURL);
}

//--------------------------------------------------------------------
void HelpAgentDispatcher::implts_stopTimer()
{
    // SAFE ->
    WriteGuard aWriteLock(m_aLock);
    m_xSelfHold.clear();
    aWriteLock.unlock();
    // <- SAFE

    // SOLAR SAFE ->
    // Timer access needs no "own lock" ! It lives if we live ...
    // But it requires locking of the solar mutex ... because it's a vcl based timer.
    {
        SolarMutexGuard aSolarLock;
        if (! m_aTimer.IsActive())
            return;
        m_aTimer.Stop();
    }
    // <- SOLAR SAFE
}

//--------------------------------------------------------------------
void HelpAgentDispatcher::implts_startTimer()
{
    // SOLAR SAFE ->
    // Timer access needs no "own lock" ! It lives if we live ...
    // But it requires locking of the solar mutex ... because it's a vcl based timer.
    {
        SolarMutexGuard aSolarLock;
        if (m_aTimer.IsActive())
            return;
    }
    // <- SOLAR SAFE

    // SAFE ->
    // Timer uses pointer to this help agent dispatcher ...
    // But normaly we are ref counted. So we must make sure that this
    // dispatcher isnt killed during the timer runs .-)
    WriteGuard aWriteLock(m_aLock);
    m_xSelfHold = css::uno::Reference< css::uno::XInterface >(static_cast< css::frame::XDispatch* >(this), css::uno::UNO_QUERY_THROW);
    aWriteLock.unlock();
    // <- SAFE

    sal_Int32 nTime = SvtHelpOptions().GetHelpAgentTimeoutPeriod();

    // SOLAR SAFE ->
    // Timer access needs no "own lock" ! It lives if we live ...
    // But it requires locking of the solar mutex ... because it's a vcl based timer.
    {
        SolarMutexGuard aSolarLock;
        m_aTimer.SetTimeout(nTime*1000); // sec => ms !
        m_aTimer.Start();
    }
}

//-----------------------------------------------
IMPL_LINK(HelpAgentDispatcher, implts_timerExpired, void*,)
{
    // This method is called by using a pointer to us.
    // But we must be aware that we can be destroyed hardly
    // if our uno reference will be gone!
    // => Hold this object alive till this method finish its work.
    // SAFE ->
    WriteGuard aWriteLock(m_aLock);
    css::uno::Reference< css::uno::XInterface > xSelfHoldUntilMethodEnds(static_cast< css::frame::XDispatch* >(this), css::uno::UNO_QUERY_THROW);
    m_xSelfHold.clear();
    aWriteLock.unlock();
    // <- SAFE

    implts_hideAgentWindow();
    implts_ignoreCurrentURL();

    return 0;
}

//--------------------------------------------------------------------
void HelpAgentDispatcher::implts_showAgentWindow()
{
    // SAFE ->
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::awt::XWindow2 > xContainerWindow(m_xContainerWindow, css::uno::UNO_QUERY_THROW);
    aReadLock.unlock();
    // <- SAFE

    css::uno::Reference< css::awt::XWindow > xAgentWindow = implts_ensureAgentWindow();

    if (
        (xContainerWindow.is()        ) &&
        (xAgentWindow.is()            ) &&
        (xContainerWindow->isVisible())
       )
    {
        // make sure that agent window resists at the right place .-)
        implts_positionAgentWindow();
        xAgentWindow->setVisible(sal_True);
    }
}

//--------------------------------------------------------------------
void HelpAgentDispatcher::implts_hideAgentWindow()
{
    css::uno::Reference< css::awt::XWindow > xAgentWindow = implts_ensureAgentWindow();
    if (xAgentWindow.is())
        xAgentWindow->setVisible(sal_False);
}

//--------------------------------------------------------------------
void HelpAgentDispatcher::implts_positionAgentWindow()
{
    // SAFE ->
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::awt::XWindow > xContainerWindow = m_xContainerWindow;
    aReadLock.unlock();
    // <- SAFE

    css::uno::Reference< css::awt::XWindow > xAgentWindow = implts_ensureAgentWindow();
    if (
        (! xContainerWindow.is())  ||
        (! xAgentWindow.is()    )
       )
        return;

          ::svt::HelpAgentWindow* pAgentWindow   = (::svt::HelpAgentWindow*)VCLUnoHelper::GetWindow(xAgentWindow);
    const css::awt::Rectangle     aContainerSize = xContainerWindow->getPosSize();
    const Size                    aAgentSize     = pAgentWindow->getPreferredSizePixel();

    sal_Int32 nW = aAgentSize.Width() ;
    sal_Int32 nH = aAgentSize.Height();

    if (nW < 1)
        nW = 100;
    if (nH < 1)
        nH = 100;

    sal_Int32 nX = aContainerSize.Width  - nW;
    sal_Int32 nY = aContainerSize.Height - nH;

    // TODO: use a surrogate if the container window is too small to contain the full-sized agent window
    xAgentWindow->setPosSize(nX, nY, nW, nH, css::awt::PosSize::POSSIZE);
}

//--------------------------------------------------------------------
css::uno::Reference< css::awt::XWindow > HelpAgentDispatcher::implts_ensureAgentWindow()
{
    // SAFE ->
    ReadGuard aReadLock(m_aLock);
    if (m_xAgentWindow.is())
        return m_xAgentWindow;
    css::uno::Reference< css::awt::XWindow > xContainerWindow = m_xContainerWindow;
    aReadLock.unlock();
    // <- SAFE

    if (!xContainerWindow.is())
        return css::uno::Reference< css::awt::XWindow >();

    ::svt::HelpAgentWindow* pAgentWindow = 0;
    // SOLAR SAFE ->
    {
        SolarMutexGuard aSolarLock;
        // create the agent window
        Window* pContainerWindow = VCLUnoHelper::GetWindow(xContainerWindow);
                pAgentWindow     = new ::svt::HelpAgentWindow(pContainerWindow);
        pAgentWindow->setCallback(this);
    }
    // <- SOLAR SAFE

    // SAFE ->
    WriteGuard aWriteLock(m_aLock);
    m_xAgentWindow = VCLUnoHelper::GetInterface(pAgentWindow);
    css::uno::Reference< css::awt::XWindow > xAgentWindow = m_xAgentWindow;
    aWriteLock.unlock();
    // <- SAFE

    // add as window listener to the container window so we can maintain the property position of the agent window
    xContainerWindow->addWindowListener(this);

    // SOLAR SAFE ->
    {
        SolarMutexGuard aSolarLock;
        // establish callback for our internal used timer.
        // Note: Its only active, if the timer will be started ...
        m_aTimer.SetTimeoutHdl(LINK(this, HelpAgentDispatcher, implts_timerExpired));
    }
    // <- SOLAR SAFE

    return xAgentWindow;
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
