/*************************************************************************
 *
 *  $RCSfile: helpagentdispatcher.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:21:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef __FRAMEWORK_DISPATCH_HELPAGENTDISPATCHER_HXX_
#include <dispatch/helpagentdispatcher.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _SV_HELP_HXX
#include <vcl/help.hxx>
#endif

#ifndef _COMPHELPER_GUARDING_HXX_
#include <comphelper/guarding.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_HELPOPT_HXX
#include <svtools/helpopt.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
//........................................................................
namespace framework
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;

    //====================================================================
    //= HelpAgentDispatcher
    //====================================================================
    //--------------------------------------------------------------------
    HelpAgentDispatcher::HelpAgentDispatcher( const Reference< XFrame >& _rxParentFrame )
        :ThreadHelpBase(&Application::GetSolarMutex())
        ,m_pContainerWindow(NULL)
        ,m_pAgentWindow(NULL)
        ,m_xParentFrame(_rxParentFrame)
    {
        OSL_ENSURE(m_xParentFrame.is(), "HelpAgentDispatcher::HelpAgentDispatcher: invalid parent frame!");
    }

    //--------------------------------------------------------------------
    HelpAgentDispatcher::~HelpAgentDispatcher()
    {
        osl_incrementInterlockedCount( &m_refCount );
            // we may create new references to ourself below, so ensure the dtor is not called twice ....
        closeAgentWindow();
        if(m_xAutoCloseTimer.isValid())
            m_xAutoCloseTimer->setListener(NULL);
    }

    //--------------------------------------------------------------------
    sal_Bool HelpAgentDispatcher::ensureContainerWindow()
    {
        if (m_pContainerWindow)
            return sal_True;

        if (!m_xParentFrame.is())
        {
            OSL_ENSURE(sal_False, "HelpAgentDispatcher::ensureContainerWindow: have no explicit container window and no frame to obtain an implicit one!");
            // error condition, already asserted in the ctor
            return sal_False;
        }

        Reference< XWindow > xContainer = m_xParentFrame->getContainerWindow();
        implConstruct(xContainer);

        return (NULL != m_pContainerWindow);
    }

    //--------------------------------------------------------------------
    void HelpAgentDispatcher::implConstruct( const Reference< XWindow >& _rxContainer )
    {
        OSL_ENSURE(!m_pContainerWindow, "HelpAgentDispatcher::implConstruct: not to be called twice!");
        OSL_ENSURE(_rxContainer.is(), "HelpAgentDispatcher::implConstruct: invalid container window given!");

        m_pContainerWindow = VCLUnoHelper::GetWindow(_rxContainer);
        OSL_ENSURE(!_rxContainer.is() || (NULL != m_pContainerWindow), "HelpAgentDispatcher::implConstruct: could not get the implementation of the container!");
    }

    //--------------------------------------------------------------------
    void SAL_CALL HelpAgentDispatcher::dispatch( const URL& _rURL, const Sequence< PropertyValue >& _rArgs ) throw (RuntimeException)
    {
        ResetableGuard aGuard(m_aLock);
        switchURL(_rURL);
    }

    //--------------------------------------------------------------------
    void SAL_CALL HelpAgentDispatcher::addStatusListener( const Reference< XStatusListener >& _rxListener, const URL& _rURL ) throw (RuntimeException)
    {
        // this is pretty simple: we accept _all_ URLs, and we accept them _always_. So simply notify the listener
        // of the initial "available" state and then do nothing.
        if (_rxListener.is())
        {
            FeatureStateEvent aEvent;
            aEvent.FeatureURL = _rURL;
            aEvent.IsEnabled = sal_True;
            aEvent.Requery = sal_False;
            _rxListener->statusChanged(aEvent);
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL HelpAgentDispatcher::removeStatusListener( const Reference< XStatusListener >& _rxListener, const URL& _rURL ) throw (RuntimeException)
    {
        // nothing to do. see addStatusListener
    }

    //--------------------------------------------------------------------
    void SAL_CALL HelpAgentDispatcher::windowResized( const WindowEvent& _rSource ) throw (RuntimeException)
    {
        positionAgentWindow();
    }

    //--------------------------------------------------------------------
    void SAL_CALL HelpAgentDispatcher::windowMoved( const WindowEvent& _rSource ) throw (RuntimeException)
    {
        // not interested in
    }

    //--------------------------------------------------------------------
    void SAL_CALL HelpAgentDispatcher::windowShown( const EventObject& _rSource ) throw (RuntimeException)
    {
        // not interested in
    }

    //--------------------------------------------------------------------
    void SAL_CALL HelpAgentDispatcher::windowHidden( const EventObject& _rSource ) throw (RuntimeException)
    {
        // not interested in
    }

    //--------------------------------------------------------------------
    void SAL_CALL HelpAgentDispatcher::disposing( const EventObject& _rSource ) throw (RuntimeException)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xSelfHold( static_cast< ::com::sun::star::frame::XDispatch* >(this), ::com::sun::star::uno::UNO_QUERY );

        // not interested in case the container window is closed (this should be handled by our owner)

        // interested in case our agent window is closed (we're the only instance allowed to close it)
        if (m_pAgentWindow)
        {
            Reference< XWindow > xSource(_rSource.Source, UNO_QUERY);
            Reference< XWindow > xAgentWindow = VCLUnoHelper::GetInterface(m_pAgentWindow);
            if (xSource.get() == xAgentWindow.get())
            {   // somebody closed my agent window, but it was not me
                agentClosedExternally();
            }
        }
    }

    //--------------------------------------------------------------------
    void HelpAgentDispatcher::agentClosedExternally()
    {
        ResetableGuard aGuard(m_aLock);
        stopAutoCloseTimer();
        m_pAgentWindow = NULL;
    }

    //--------------------------------------------------------------------
    sal_Bool HelpAgentDispatcher::approveURLRequest(const URL& _rURL)
    {
        SvtHelpOptions aHelpOptions;
        sal_Int32 nAllowedToIgnore = aHelpOptions.getAgentIgnoreURLCounter(_rURL.Complete);
        return nAllowedToIgnore > 0;
    }

    //--------------------------------------------------------------------
    void HelpAgentDispatcher::switchURL(const URL& _rURL)
    {
        if (!approveURLRequest(_rURL))
            // silently drop the request
            return;

        // show our agent window
        ensureAgentWindow();

        // stop the expiration timer for the old URL
        stopAutoCloseTimer();

        // save the URL
        m_sCurrentURL = _rURL.Complete;

        // start the expiration timer for the new URL
        startAutoCloseTimer();
    }

    //--------------------------------------------------------------------
    void HelpAgentDispatcher::stopAutoCloseTimer()
    {
        if (!m_xAutoCloseTimer.isValid())
            return;

        m_xAutoCloseTimer->stop();
    }

    //--------------------------------------------------------------------
    void HelpAgentDispatcher::startAutoCloseTimer()
    {
        ::vos::TTimeValue aAutoCloseTimeout( SvtHelpOptions().GetHelpAgentTimeoutPeriod(), 0 );
        if (!m_xAutoCloseTimer.isValid())
        {

            m_xAutoCloseTimer = new OTimerHelper(aAutoCloseTimeout);
            m_xAutoCloseTimer->setListener(this);
        }

        m_xAutoCloseTimer->setRemainingTime(aAutoCloseTimeout);
        m_xAutoCloseTimer->start();
    }

    //--------------------------------------------------------------------
    void HelpAgentDispatcher::helpRequested()
    {
        ResetableGuard aGuard(m_aLock);

        // FIRST stop the timer
        stopAutoCloseTimer();

        // reset the ignore counter for this URL
        SvtHelpOptions().resetAgentIgnoreURLCounter(m_sCurrentURL);

        Help* pApplicationHelp = Application::GetHelp();
        OSL_ENSURE(pApplicationHelp, "HelpAgentDispatcher::helpRequested: no help system available!");
        if (pApplicationHelp)
            pApplicationHelp->Start( m_sCurrentURL, NULL );

        aGuard.unlock();
        closeAgentWindow();
    }

    //--------------------------------------------------------------------
    void HelpAgentDispatcher::closeAgent()
    {
        // the hint has been ignored by the user (click the closer)
        markURLIgnored(m_sCurrentURL);
        // close the window
        closeAgentWindow();
    }

    //--------------------------------------------------------------------
    void HelpAgentDispatcher::timerExpired()
    {
        // the hint has been ignored by the user
        markURLIgnored(m_sCurrentURL);
        // close the window
        closeAgentWindow();
    }

    //--------------------------------------------------------------------
    void HelpAgentDispatcher::markURLIgnored( const ::rtl::OUString& _rURL )
    {
        SvtHelpOptions().decAgentIgnoreURLCounter(_rURL);
    }

    //--------------------------------------------------------------------
    void HelpAgentDispatcher::closeAgentWindow()
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xSelfHold( static_cast< ::com::sun::star::frame::XDispatch* >(this), ::com::sun::star::uno::UNO_QUERY );

        // now acquire the SolarMutex ...
        ::vos::OGuard aSolarGuard(Application::GetSolarMutex());
        // ... and our own mutex
        ResetableGuard aGuard(m_aLock);

        stopAutoCloseTimer();

        if (!m_pAgentWindow)
            return;

        if (m_pContainerWindow)
        {
            Reference< XWindow > xContainer = VCLUnoHelper::GetInterface(m_pContainerWindow);
            OSL_ENSURE(xContainer.is(), "HelpAgentDispatcher::closeAgentWindow: no UNO interface for the container window!");
            if (xContainer.is())
                xContainer->removeWindowListener(this);
        }

        if (m_pAgentWindow)
        {
            Reference< XWindow > xAgentWindow = VCLUnoHelper::GetInterface(m_pAgentWindow);
            OSL_ENSURE(xAgentWindow.is(), "HelpAgentDispatcher::closeAgentWindow: no UNO interface for the agent window!");
            if (xAgentWindow.is())
                xAgentWindow->removeWindowListener(this);
        }

        delete m_pAgentWindow;
        m_pAgentWindow = NULL;
    }

    //--------------------------------------------------------------------
    void HelpAgentDispatcher::positionAgentWindow()
    {
        OSL_ENSURE(m_pContainerWindow, "HelpAgentDispatcher::positionAgentWindow: please use ensureContainerWindow!");
        OSL_ENSURE(m_pAgentWindow, "HelpAgentDispatcher::positionAgentWindow: to be called with an existing agent window only!");
        OSL_ENSURE(m_pAgentWindow->GetParent() == m_pContainerWindow, "HelpAgentDispatcher::positionAgentWindow: invalid window hierarchy!");

        const Size aContainerSize = m_pContainerWindow->GetSizePixel();
        const Size aAgentSize = m_pAgentWindow->getPreferredSizePixel();

        const Point aAgentPos   ( aContainerSize.Width() - aAgentSize.Width()
                                , aContainerSize.Height() - aAgentSize.Height() );

        // TODO: use a surrogate if the container window is too small to contain the full-sized agent window

        m_pAgentWindow->SetPosSizePixel(aAgentPos, aAgentSize);
    }

    //--------------------------------------------------------------------
    sal_Bool HelpAgentDispatcher::ensureAgentWindow()
    {
        if (m_pAgentWindow)
            return sal_True;

        ::vos::OGuard aSolarGuard(Application::GetSolarMutex());
        if (!ensureContainerWindow())
            return sal_False;

        // create it
        m_pAgentWindow = new ::svt::HelpAgentWindow(m_pContainerWindow);
        m_pAgentWindow->setCallback(this);

        // add as listener at the agent window in case it is closed by the user (and not by us ourself)
        Reference< XWindow > xAgentWindow = VCLUnoHelper::GetInterface(m_pAgentWindow);
        OSL_ENSURE(xAgentWindow.is(), "HelpAgentDispatcher::ensureAgentWindow: no UNO interface for the agent window!");
        if (xAgentWindow.is())
            xAgentWindow->addWindowListener(this);

        // add as window listener to the container window so we can maintain the property position of the agent window
        Reference< XWindow > xContainer = VCLUnoHelper::GetInterface(m_pContainerWindow);
        OSL_ENSURE(xContainer.is(), "HelpAgentDispatcher::ensureAgentWindow: no container window interface!");
        if (xContainer.is())
            xContainer->addWindowListener(this);

        // position it
        positionAgentWindow();

        // show it
        if (m_pContainerWindow->IsVisible())
            m_pAgentWindow->Show();

        return sal_True;
    }

//........................................................................
}   // namespace framework
//........................................................................

