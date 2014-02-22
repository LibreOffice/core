/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <sal/types.h>

#include <services/desktop.hxx>
#include <classes/filtercache.hxx>
#include <protocols.h>
#include <general.h>

#include <vcl/svapp.hxx>
#include <unotools/tempfile.hxx>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/theAutoRecovery.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XSessionManagerListener2.hpp>
#include <com/sun/star/frame/XSessionManagerClient.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/util/URL.hpp>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/pathoptions.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

using namespace css;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace com::sun::star::beans;
using namespace framework;

namespace {


/** @short  implements flat/deep detection of file/stream formats and provides
            further read/write access to the global office type configuration.

    @descr  Using of this class makes it possible to get information about the
            format type of a given URL or stream. The returned internal type name
            can be used to get more information about this format. Further this
            class provides full access to the configuration data and following
            implementations will support some special query modes.

    @author     as96863

    @docdate    10.03.2003 by as96863

    @todo       <ul>
                    <li>implementation of query mode</li>
                    <li>simple restore mechanism of last consistent cache state,
                        if flush failed</li>
                </ul>
 */
typedef cppu::WeakImplHelper4<
    css::lang::XInitialization,
    css::frame::XSessionManagerListener2,
    css::frame::XStatusListener,
    css::lang::XServiceInfo> SessionListener_BASE;

class SessionListener : public SessionListener_BASE
{
private:
    osl::Mutex m_aMutex;

    /** reference to the uno service manager, which created this service.
        It can be used to create own needed helper services. */
    css::uno::Reference< css::uno::XComponentContext > m_xContext;

    css::uno::Reference< css::frame::XSessionManagerClient > m_rSessionManager;

    
    sal_Bool m_bRestored;

    sal_Bool m_bSessionStoreRequested;

    sal_Bool m_bAllowUserInteractionOnQuit;
    sal_Bool m_bTerminated;


    
    void StoreSession( sal_Bool bAsync );

    
    void QuitSessionQuietly();

public:
    SessionListener( const css::uno::Reference< css::uno::XComponentContext >& xContext );

    virtual ~SessionListener();

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException)
    {
        return OUString("com.sun.star.comp.frame.SessionListener");
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException)
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException)
    {
        css::uno::Sequence< OUString > aSeq(1);
        aSeq[0] = OUString("com.sun.star.frame.SessionListener");
        return aSeq;
    }

    virtual void SAL_CALL disposing(const com::sun::star::lang::EventObject&) throw (css::uno::RuntimeException);

    
    virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any  >& args) throw (css::uno::RuntimeException);

    
    virtual void SAL_CALL doSave( sal_Bool bShutdown, sal_Bool bCancelable )
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL approveInteraction( sal_Bool bInteractionGranted )
        throw (css::uno::RuntimeException);
   virtual void SAL_CALL shutdownCanceled()
        throw (css::uno::RuntimeException);
   virtual sal_Bool SAL_CALL doRestore()
        throw (css::uno::RuntimeException);

    
    virtual void SAL_CALL doQuit()
        throw (::com::sun::star::uno::RuntimeException);

    
    virtual void SAL_CALL statusChanged(const com::sun::star::frame::FeatureStateEvent& event)
        throw (css::uno::RuntimeException);

    void doSaveImpl( sal_Bool bShutdown, sal_Bool bCancelable ) throw (css::uno::RuntimeException);
};

SessionListener::SessionListener(const css::uno::Reference< css::uno::XComponentContext >& rxContext )
        : m_xContext( rxContext )
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
    osl::MutexGuard g(m_aMutex);
    try
    {
        
        
        
        

        css::uno::Reference< frame::XDispatch > xDispatch = css::frame::theAutoRecovery::get( m_xContext );
        css::uno::Reference< XURLTransformer > xURLTransformer = URLTransformer::create( m_xContext );
        URL aURL;
        aURL.Complete = "vnd.sun.star.autorecovery:/doSessionSave";
        xURLTransformer->parseStrict(aURL);

        
        if ( bAsync )
            xDispatch->addStatusListener(this, aURL);

        Sequence< PropertyValue > args(1);
        args[0] = PropertyValue(OUString("DispatchAsynchron"),-1,makeAny(bAsync),PropertyState_DIRECT_VALUE);
        xDispatch->dispatch(aURL, args);
    } catch (const com::sun::star::uno::Exception& e) {
        SAL_WARN("fwk.session",e.Message);
        
        
        if ( bAsync && m_rSessionManager.is() )
            m_rSessionManager->saveDone(this);
    }
}

void SessionListener::QuitSessionQuietly()
{
    SAL_INFO("fwk.session", "SessionListener::QuitSessionQuietly");
    osl::MutexGuard g(m_aMutex);
    try
    {
        
        
        

        css::uno::Reference< frame::XDispatch > xDispatch = css::frame::theAutoRecovery::get( m_xContext );
        css::uno::Reference< XURLTransformer > xURLTransformer = URLTransformer::create( m_xContext );
        URL aURL;
        aURL.Complete = "vnd.sun.star.autorecovery:/doSessionQuietQuit";
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
       ;
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
        m_rSessionManager = css::uno::Reference< frame::XSessionManagerClient >
            (m_xContext->getServiceManager()->createInstanceWithContext(aSMgr, m_xContext), UNO_QUERY);

    if (m_rSessionManager.is())
    {
        m_rSessionManager->addSessionManagerListener(this);
    }
}

void SAL_CALL SessionListener::statusChanged(const frame::FeatureStateEvent& event)
    throw (css::uno::RuntimeException)
{
   SAL_INFO("fwk.session", "SessionListener::statusChanged");
   if ( event.FeatureURL.Complete == "vnd.sun.star.autorecovery:/doSessionRestore" )
    {
        if (event.FeatureDescriptor.equalsAscii("update"))
            m_bRestored = sal_True; 

    }
    else if ( event.FeatureURL.Complete == "vnd.sun.star.autorecovery:/doSessionSave" )
    {
        if (event.FeatureDescriptor.equalsAscii("stop"))
        {
            if (m_rSessionManager.is())
                m_rSessionManager->saveDone(this); 
        }
    }
}


sal_Bool SAL_CALL SessionListener::doRestore()
    throw (RuntimeException)
{
    SAL_INFO("fwk.session", "SessionListener::doRestore");
    osl::MutexGuard g(m_aMutex);
    m_bRestored = sal_False;
    try {
        css::uno::Reference< frame::XDispatch > xDispatch = css::frame::theAutoRecovery::get( m_xContext );

        URL aURL;
        aURL.Complete = "vnd.sun.star.autorecovery:/doSessionRestore";
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
        m_bSessionStoreRequested = sal_True; 
        if ( m_bAllowUserInteractionOnQuit && m_rSessionManager.is() )
            m_rSessionManager->queryInteraction( static_cast< css::frame::XSessionManagerListener* >( this ) );
        else
            StoreSession( sal_True );
    }
    
    else if( m_rSessionManager.is() )
        m_rSessionManager->saveDone( this );
}

void SAL_CALL SessionListener::approveInteraction( sal_Bool bInteractionGranted )
    throw (RuntimeException)
{
    SAL_INFO("fwk.session", "SessionListener::approveInteraction");
    
    osl::MutexGuard g(m_aMutex);

    if ( bInteractionGranted )
    {
        
        try
        {
            
            StoreSession( sal_False );

            css::uno::Reference< css::frame::XDesktop2 > xDesktop = css::frame::Desktop::create( m_xContext );
            
            
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
    
    m_bSessionStoreRequested = sal_False; 
}

void SessionListener::doQuit()
    throw (RuntimeException)
{
    SAL_INFO("fwk.session", "SessionListener::doQuit");
    if ( m_bSessionStoreRequested && !m_bTerminated )
    {
        
        QuitSessionQuietly();
    }
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_frame_SessionListener_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SessionListener(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
