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
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/util/URL.hpp>
#include <osl/time.h>
#include <comphelper/processfactory.hxx>
#include <unotools/pathoptions.hxx>
#include <stdio.h>

#include <com/sun/star/uno/Any.hxx>

#include <com/sun/star/uno/Sequence.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace com::sun::star::frame;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;


namespace framework{

//***********************************************
// XInterface, XTypeProvider, XServiceInfo

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE_2(
       SessionListener,
       cppu::OWeakObject,
       "com.sun.star.frame.SessionListener",
       IMPLEMENTATIONNAME_SESSIONLISTENER)

DEFINE_INIT_SERVICE(SessionListener,
                    {
                        /* Add special code for initialization here, if you have to use your own instance
                           during your ctor is still in progress! */
                    }
                   )

SessionListener::SessionListener(const css::uno::Reference< css::uno::XComponentContext >& rxContext )
        : ThreadHelpBase      (&Application::GetSolarMutex())
        , m_xContext          (rxContext                    )
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

        css::uno::Reference< XDispatch > xDispatch = css::frame::AutoRecovery::create( m_xContext );
        css::uno::Reference< XURLTransformer > xURLTransformer = URLTransformer::create( m_xContext );
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
        SAL_WARN("fwk.session",e.Message);
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

        css::uno::Reference< XDispatch > xDispatch = css::frame::AutoRecovery::create( m_xContext );
        css::uno::Reference< XURLTransformer > xURLTransformer = URLTransformer::create( m_xContext );
        URL aURL;
        aURL.Complete = OUString("vnd.sun.star.autorecovery:/doSessionQuietQuit");
        xURLTransformer->parseStrict(aURL);

        Sequence< PropertyValue > args(1);
        args[0] = PropertyValue(OUString("DispatchAsynchron"),-1,makeAny(sal_False),PropertyState_DIRECT_VALUE);
        xDispatch->dispatch(aURL, args);
    } catch (const com::sun::star::uno::Exception& e) {
        SAL_WARN("fwk.session",e.Message);
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
    if ( (args.getLength() == 1) && (args[0] >>= m_bAllowUserInteractionOnQuit) )
       ;// do nothing
    else if (args.getLength() > 0)
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
            (m_xContext->getServiceManager()->createInstanceWithContext(aSMgr, m_xContext), UNO_QUERY);

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
        css::uno::Reference< XDispatch > xDispatch = css::frame::AutoRecovery::create( m_xContext );

        URL aURL;
        aURL.Complete = OUString("vnd.sun.star.autorecovery:/doSessionRestore");
        css::uno::Reference< XURLTransformer > xURLTransformer(URLTransformer::create(m_xContext));
        xURLTransformer->parseStrict(aURL);
        Sequence< PropertyValue > args;
        xDispatch->addStatusListener(this, aURL);
        xDispatch->dispatch(aURL, args);
        m_bRestored = sal_True;

    } catch (const com::sun::star::uno::Exception& e) {
        SAL_WARN("fwk.session",e.Message);
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

            css::uno::Reference< css::frame::XDesktop2 > xDesktop = css::frame::Desktop::create( m_xContext );
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
