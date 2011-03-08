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
//_______________________________________________
// my own includes

#include <services/sessionlistener.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/resetableguard.hxx>
#include <protocols.h>
#include <services.h>

#include <osl/thread.h>


#include <vcl/svapp.hxx>
#include <tools/urlobj.hxx>
#include <tools/tempfile.hxx>
#include <unotools/tempfile.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/util/URL.hpp>
#include <osl/time.h>
#include <comphelper/processfactory.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/internaloptions.hxx>
#include <stdio.h>
//_______________________________________________
// interface includes
#include <com/sun/star/uno/Any.hxx>

#include <com/sun/star/uno/Sequence.hxx>
//_______________________________________________
// includes of other projects

//_______________________________________________
// namespace

using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace com::sun::star::frame;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;

using ::rtl::OUString;
using ::rtl::OString;
namespace framework{

//_______________________________________________
// non exported const

//_______________________________________________
// non exported definitions

//_______________________________________________
// declarations

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
}

SessionListener::~SessionListener()
{
    if (m_rSessionManager.is())
    {
        css::uno::Reference< XSessionManagerListener> me(this);
        m_rSessionManager->removeSessionManagerListener(me);
    }
}

void SessionListener::StoreSession( sal_Bool bAsync )
{
    ResetableGuard aGuard(m_aLock);
    try
    {
        // xd create SERVICENAME_AUTORECOVERY -> XDispatch
        // xd->dispatch("vnd.sun.star.autorecovery:/doSessionSave, async=bAsync
        // on stop event m_rSessionManager->saveDone(this); in case of asynchronous call
        // in case of synchronous call the caller should do saveDone() call himself!

        css::uno::Reference< XDispatch > xDispatch(m_xSMGR->createInstance(SERVICENAME_AUTORECOVERY), UNO_QUERY_THROW);
        css::uno::Reference< XURLTransformer > xURLTransformer(m_xSMGR->createInstance(SERVICENAME_URLTRANSFORMER), UNO_QUERY_THROW);
        URL aURL;
        aURL.Complete = OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.star.autorecovery:/doSessionSave"));
        xURLTransformer->parseStrict(aURL);

        // in case of asynchronous call the notification will trigger saveDone()
        if ( bAsync )
            xDispatch->addStatusListener(this, aURL);

        Sequence< PropertyValue > args(1);
        args[0] = PropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("DispatchAsynchron")),-1,makeAny(bAsync),PropertyState_DIRECT_VALUE);
        xDispatch->dispatch(aURL, args);
    } catch (com::sun::star::uno::Exception& e) {
        OString aMsg = OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8);
        OSL_ENSURE(sal_False, aMsg.getStr());
        // save failed, but tell manager to go on if we havent yet dispatched the request
        // in case of synchronous saving the notification is done by the caller
        if ( bAsync && m_rSessionManager.is() )
            m_rSessionManager->saveDone(this);
    }
}

void SessionListener::QuitSessionQuietly()
{
    ResetableGuard aGuard(m_aLock);
    try
    {
        // xd create SERVICENAME_AUTORECOVERY -> XDispatch
        // xd->dispatch("vnd.sun.star.autorecovery:/doSessionQuietQuit, async=false
        // it is done synchronously to avoid conflict with normal quit process

        css::uno::Reference< XDispatch > xDispatch(m_xSMGR->createInstance(SERVICENAME_AUTORECOVERY), UNO_QUERY_THROW);
        css::uno::Reference< XURLTransformer > xURLTransformer(m_xSMGR->createInstance(SERVICENAME_URLTRANSFORMER), UNO_QUERY_THROW);
        URL aURL;
        aURL.Complete = OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.star.autorecovery:/doSessionQuietQuit"));
        xURLTransformer->parseStrict(aURL);

        Sequence< PropertyValue > args(1);
        args[0] = PropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("DispatchAsynchron")),-1,makeAny(sal_False),PropertyState_DIRECT_VALUE);
        xDispatch->dispatch(aURL, args);
    } catch (com::sun::star::uno::Exception& e) {
        OString aMsg = OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8);
        OSL_ENSURE(sal_False, aMsg.getStr());
    }
}

void SAL_CALL SessionListener::disposing(const com::sun::star::lang::EventObject&) throw (RuntimeException)
{
}

void SAL_CALL SessionListener::initialize(const Sequence< Any  >& args)
    throw (RuntimeException)
{

    OUString aSMgr(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.SessionManagerClient"));
    if (args.getLength() > 0)
    {
        NamedValue v;
        for (int i = 0; i < args.getLength(); i++)
        {
            if (args[i] >>= v)
            {
                if (v.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("SessionManagerName")))
                    v.Value >>= aSMgr;
                else if (v.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("SessionManager")))
                    v.Value >>= m_rSessionManager;
                else if (v.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("AllowUserInteractionOnQuit")))
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
   if (event.FeatureURL.Complete.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.autorecovery:/doSessionRestore")))
    {
        if (event.FeatureDescriptor.compareToAscii("update")==0)
            m_bRestored = sal_True; // a document was restored
        // if (event.FeatureDescriptor.compareToAscii("stop")==0)

    }
    else if (event.FeatureURL.Complete.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.autorecovery:/doSessionSave")))
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
    ResetableGuard aGuard(m_aLock);
    m_bRestored = sal_False;
    try {
        css::uno::Reference< XDispatch > xDispatch(m_xSMGR->createInstance(SERVICENAME_AUTORECOVERY), UNO_QUERY_THROW);

        URL aURL;
        aURL.Complete = OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.star.autorecovery:/doSessionRestore"));
        css::uno::Reference< XURLTransformer > xURLTransformer(m_xSMGR->createInstance(SERVICENAME_URLTRANSFORMER), UNO_QUERY_THROW);
        xURLTransformer->parseStrict(aURL);
        Sequence< PropertyValue > args;
        xDispatch->addStatusListener(this, aURL);
        xDispatch->dispatch(aURL, args);
        m_bRestored = sal_True;

    } catch (com::sun::star::uno::Exception& e) {
        OString aMsg = OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8);
        OSL_ENSURE(sal_False, aMsg.getStr());
    }

    return m_bRestored;
}


void SAL_CALL SessionListener::doSave( sal_Bool bShutdown, sal_Bool /*bCancelable*/ )
    throw (RuntimeException)
{
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
            m_bTerminated = xDesktop->terminate();

            if ( m_rSessionManager.is() )
            {
                // false means that the application closing has been cancelled
                if ( !m_bTerminated )
                    m_rSessionManager->cancelShutdown();
                else
                    m_rSessionManager->interactionDone( this );
            }
        }
        catch( css::uno::Exception& )
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
    // set the state back
    m_bSessionStoreRequested = sal_False; // there is no need to protect it with mutex
}

void SessionListener::doQuit()
    throw (RuntimeException)
{
    if ( m_bSessionStoreRequested && !m_bTerminated )
    {
        // let the session be closed quietly in this case
        QuitSessionQuietly();
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
