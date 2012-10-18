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

#include <services/sessionlistener.hxx>
#include <services/desktop.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/resetableguard.hxx>
#include <protocols.h>
#include <services.h>

#include <osl/thread.h>

#include <vcl/svapp.hxx>
#include <unotools/tempfile.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/AutoRecovery.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/util/URL.hpp>
#include <osl/time.h>
#include <comphelper/processfactory.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/internaloptions.hxx>
#include <stdio.h>

#include <com/sun/star/uno/Any.hxx>

#include <com/sun/star/uno/Sequence.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace com::sun::star::frame;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;

using ::rtl::OUString;
using ::rtl::OString;

namespace framework{

//***********************************************
// XInterface, XTypeProvider, XServiceInfo

DEFINE_XINTERFACE_6(
        SessionListener,
        OWeakObject,
        DIRECT_INTERFACE(css::lang::XTypeProvider),
        DIRECT_INTERFACE(css::lang::XInitialization),
        DIRECT_INTERFACE(css::frame::XSessionManagerListener),
        DIRECT_INTERFACE(css::frame::XSessionManagerListener2),
        DIRECT_INTERFACE(css::frame::XStatusListener),
        DIRECT_INTERFACE(css::lang::XServiceInfo))

DEFINE_XTYPEPROVIDER_5(
        SessionListener,
        css::lang::XTypeProvider,
        css::lang::XInitialization,
        css::frame::XSessionManagerListener2,
        css::frame::XStatusListener,
        css::lang::XServiceInfo)

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE(
       SessionListener,
       cppu::OWeakObject,
       SERVICENAME_SESSIONLISTENER,
       IMPLEMENTATIONNAME_SESSIONLISTENER)

DEFINE_INIT_SERVICE(SessionListener,
                    {
                        /* Add special code for initialization here, if you have to use your own instance
                           during your ctor is still in progress! */
                    }
                   )

SessionListener::SessionListener(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR )
        : ThreadHelpBase      (&Application::GetSolarMutex())
        , OWeakObject         (                             )
        , m_xSMGR             (xSMGR                        )
        , m_bRestored( sal_False )
        , m_bSessionStoreRequested( sal_False )
        , m_bAllowUserInteractionOnQuit( sal_False )
        , m_bTerminated( sal_False )
{
    SAL_INFO("fwk.session", "SessionListener::SessionListener");
}

SessionListener::~SessionListener()
{
    SAL_INFO("fwk.session", "SessionListener::~SessionListener");
    if (m_rSessionManager.is())
    {
        css::uno::Reference< XSessionManagerListener> me(this);
        m_rSessionManager->removeSessionManagerListener(me);
    }
}

void SessionListener::StoreSession( sal_Bool bAsync )
{
    SAL_INFO("fwk.session", "SessionListener::StoreSession");
    ResetableGuard aGuard(m_aLock);
    try
    {
        // xd create SERVICENAME_AUTORECOVERY -> XDispatch
        // xd->dispatch("vnd.sun.star.autorecovery:/doSessionSave, async=bAsync
        // on stop event m_rSessionManager->saveDone(this); in case of asynchronous call
        // in case of synchronous call the caller should do saveDone() call himself!

        css::uno::Reference< XDispatch > xDispatch = css::frame::AutoRecovery::create( ::comphelper::getComponentContext(m_xSMGR) );
        css::uno::Reference< XURLTransformer > xURLTransformer = URLTransformer::create( ::comphelper::getComponentContext(m_xSMGR) );
        URL aURL;
        aURL.Complete = OUString("vnd.sun.star.autorecovery:/doSessionSave");
        xURLTransformer->parseStrict(aURL);

        // in case of asynchronous call the notification will trigger saveDone()
        if ( bAsync )
            xDispatch->addStatusListener(this, aURL);

        Sequence< PropertyValue > args(1);
        args[0] = PropertyValue(OUString("DispatchAsynchron"),-1,makeAny(bAsync),PropertyState_DIRECT_VALUE);
        xDispatch->dispatch(aURL, args);
    } catch (const com::sun::star::uno::Exception& e) {
        OString aMsg = OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8);
        OSL_FAIL(aMsg.getStr());
        // save failed, but tell manager to go on if we havent yet dispatched the request
        // in case of synchronous saving the notification is done by the caller
        if ( bAsync && m_rSessionManager.is() )
            m_rSessionManager->saveDone(this);
    }
}

void SessionListener::QuitSessionQuietly()
{
    SAL_INFO("fwk.session", "SessionListener::QuitSessionQuietly");
    ResetableGuard aGuard(m_aLock);
    try
    {
        // xd create SERVICENAME_AUTORECOVERY -> XDispatch
        // xd->dispatch("vnd.sun.star.autorecovery:/doSessionQuietQuit, async=false
        // it is done synchronously to avoid conflict with normal quit process

        css::uno::Reference< XDispatch > xDispatch = css::frame::AutoRecovery::create( ::comphelper::getComponentContext(m_xSMGR) );
        css::uno::Reference< XURLTransformer > xURLTransformer = URLTransformer::create( ::comphelper::getComponentContext(m_xSMGR) );
        URL aURL;
        aURL.Complete = OUString("vnd.sun.star.autorecovery:/doSessionQuietQuit");
        xURLTransformer->parseStrict(aURL);

        Sequence< PropertyValue > args(1);
        args[0] = PropertyValue(OUString("DispatchAsynchron"),-1,makeAny(sal_False),PropertyState_DIRECT_VALUE);
        xDispatch->dispatch(aURL, args);
    } catch (const com::sun::star::uno::Exception& e) {
        OString aMsg = OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8);
        OSL_FAIL(aMsg.getStr());
    }
}

void SAL_CALL SessionListener::disposing(const com::sun::star::lang::EventObject&) throw (RuntimeException)
{
    SAL_INFO("fwk.session", "SessionListener::disposing");
}

void SAL_CALL SessionListener::initialize(const Sequence< Any  >& args)
    throw (RuntimeException)
{
    SAL_INFO("fwk.session", "SessionListener::initialize");

    OUString aSMgr("com.sun.star.frame.SessionManagerClient");
    if (args.getLength() > 0)
    {
        NamedValue v;
        for (int i = 0; i < args.getLength(); i++)
        {
            if (args[i] >>= v)
            {
                if ( v.Name == "SessionManagerName" )
                    v.Value >>= aSMgr;
                else if ( v.Name == "SessionManager" )
                    v.Value >>= m_rSessionManager;
                else if ( v.Name == "AllowUserInteractionOnQuit" )
                    v.Value >>= m_bAllowUserInteractionOnQuit;
            }
        }
    }
    if (!m_rSessionManager.is())
        m_rSessionManager = css::uno::Reference< XSessionManagerClient >
            (m_xSMGR->createInstance(aSMgr), UNO_QUERY);

    if (m_rSessionManager.is())
    {
        m_rSessionManager->addSessionManagerListener(this);
    }
}

void SAL_CALL SessionListener::statusChanged(const FeatureStateEvent& event)
    throw (css::uno::RuntimeException)
{
   SAL_INFO("fwk.session", "SessionListener::statusChanged");
   if ( event.FeatureURL.Complete == "vnd.sun.star.autorecovery:/doSessionRestore" )
    {
        if (event.FeatureDescriptor.compareToAscii("update")==0)
            m_bRestored = sal_True; // a document was restored

    }
    else if ( event.FeatureURL.Complete == "vnd.sun.star.autorecovery:/doSessionSave" )
    {
        if (event.FeatureDescriptor.compareToAscii("stop")==0)
        {
            if (m_rSessionManager.is())
                m_rSessionManager->saveDone(this); // done with save
        }
    }
}


sal_Bool SAL_CALL SessionListener::doRestore()
    throw (RuntimeException)
{
    SAL_INFO("fwk.session", "SessionListener::doRestore");
    ResetableGuard aGuard(m_aLock);
    m_bRestored = sal_False;
    try {
        css::uno::Reference< XDispatch > xDispatch = css::frame::AutoRecovery::create( ::comphelper::getComponentContext(m_xSMGR) );

        URL aURL;
        aURL.Complete = OUString("vnd.sun.star.autorecovery:/doSessionRestore");
        css::uno::Reference< XURLTransformer > xURLTransformer(URLTransformer::create(::comphelper::getComponentContext(m_xSMGR)));
        xURLTransformer->parseStrict(aURL);
        Sequence< PropertyValue > args;
        xDispatch->addStatusListener(this, aURL);
        xDispatch->dispatch(aURL, args);
        m_bRestored = sal_True;

    } catch (const com::sun::star::uno::Exception& e) {
        OString aMsg = OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8);
        OSL_FAIL(aMsg.getStr());
    }

    return m_bRestored;
}


void SAL_CALL SessionListener::doSave( sal_Bool bShutdown, sal_Bool /*bCancelable*/ )
    throw (RuntimeException)
{
    SAL_INFO("fwk.session", "SessionListener::doSave");
    if (bShutdown)
    {
        m_bSessionStoreRequested = sal_True; // there is no need to protect it with mutex
        if ( m_bAllowUserInteractionOnQuit && m_rSessionManager.is() )
            m_rSessionManager->queryInteraction( static_cast< css::frame::XSessionManagerListener* >( this ) );
        else
            StoreSession( sal_True );
    }
    // we don't have anything to do so tell the session manager we're done
    else if( m_rSessionManager.is() )
        m_rSessionManager->saveDone( this );
}

void SAL_CALL SessionListener::approveInteraction( sal_Bool bInteractionGranted )
    throw (RuntimeException)
{
    SAL_INFO("fwk.session", "SessionListener::approveInteraction");
    // do AutoSave as the first step
    ResetableGuard aGuard(m_aLock);

    if ( bInteractionGranted )
    {
        // close the office documents in normal way
        try
        {
            // first of all let the session be stored to be sure that we lose no information
            StoreSession( sal_False );

            css::uno::Reference< css::frame::XDesktop > xDesktop( m_xSMGR->createInstance(SERVICENAME_DESKTOP), css::uno::UNO_QUERY_THROW);
            // honestly: how many implementations of XDesktop will we ever have?
            // so casting this directly to the implementation
            Desktop* pDesktop(dynamic_cast<Desktop*>(xDesktop.get()));
            if(pDesktop)
            {
                SAL_INFO("fwk.session", "XDesktop is a framework::Desktop -- good.");
                m_bTerminated = pDesktop->terminateQuickstarterToo();
            }
            else
            {
                SAL_WARN("fwk.session", "XDesktop is not a framework::Desktop -- this should never happen.");
                m_bTerminated = xDesktop->terminate();
            }

            if ( m_rSessionManager.is() )
            {
                // false means that the application closing has been cancelled
                if ( !m_bTerminated )
                    m_rSessionManager->cancelShutdown();
                else
                    m_rSessionManager->interactionDone( this );
            }
        }
        catch( const css::uno::Exception& )
        {
            StoreSession( sal_True );
            m_rSessionManager->interactionDone( this );
        }

        if ( m_rSessionManager.is() )
            m_rSessionManager->saveDone(this);
    }
    else
    {
        StoreSession( sal_True );
    }
}

void SessionListener::shutdownCanceled()
    throw (RuntimeException)
{
    SAL_INFO("fwk.session", "SessionListener::shutdownCanceled");
    // set the state back
    m_bSessionStoreRequested = sal_False; // there is no need to protect it with mutex
}

void SessionListener::doQuit()
    throw (RuntimeException)
{
    SAL_INFO("fwk.session", "SessionListener::doQuit");
    if ( m_bSessionStoreRequested && !m_bTerminated )
    {
        // let the session be closed quietly in this case
        QuitSessionQuietly();
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
