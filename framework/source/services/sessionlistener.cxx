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

#include <sal/types.h>

#include <services/desktop.hxx>
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
#include <cppuhelper/implbase.hxx>
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

/// @HTML
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
typedef cppu::WeakImplHelper<
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

    // restore handling
    bool m_bRestored;

    bool m_bSessionStoreRequested;

    bool m_bAllowUserInteractionOnQuit;
    bool m_bTerminated;

    // in case of synchronous call the caller should do saveDone() call himself!
    void StoreSession( bool bAsync );

    // let session quietly close the documents, remove lock files, store configuration and etc.
    void QuitSessionQuietly();

public:
    explicit SessionListener(const css::uno::Reference< css::uno::XComponentContext >& xContext);

    virtual ~SessionListener() override;

    virtual OUString SAL_CALL getImplementationName() override
    {
        return OUString("com.sun.star.comp.frame.SessionListener");
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return {"com.sun.star.frame.SessionListener"};
    }

    virtual void SAL_CALL disposing(const css::lang::EventObject&) override;

    // XInitialization
    virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any  >& args) override;

    // XSessionManagerListener
    virtual void SAL_CALL doSave( sal_Bool bShutdown, sal_Bool bCancelable ) override;
    virtual void SAL_CALL approveInteraction( sal_Bool bInteractionGranted ) override;
   virtual void SAL_CALL shutdownCanceled() override;
   virtual sal_Bool SAL_CALL doRestore() override;

    // XSessionManagerListener2
    virtual void SAL_CALL doQuit() override;

    // XStatusListener
    virtual void SAL_CALL statusChanged(const css::frame::FeatureStateEvent& event) override;
};

SessionListener::SessionListener(const css::uno::Reference< css::uno::XComponentContext >& rxContext )
        : m_xContext( rxContext )
        , m_bRestored( false )
        , m_bSessionStoreRequested( false )
        , m_bAllowUserInteractionOnQuit( false )
        , m_bTerminated( false )
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

void SessionListener::StoreSession( bool bAsync )
{
    SAL_INFO("fwk.session", "SessionListener::StoreSession");
    osl::MutexGuard g(m_aMutex);
    try
    {
        // xd create SERVICENAME_AUTORECOVERY -> frame::XDispatch
        // xd->dispatch("vnd.sun.star.autorecovery:/doSessionSave, async=bAsync
        // on stop event m_rSessionManager->saveDone(this); in case of asynchronous call
        // in case of synchronous call the caller should do saveDone() call himself!

        css::uno::Reference< frame::XDispatch > xDispatch = css::frame::theAutoRecovery::get( m_xContext );
        css::uno::Reference< XURLTransformer > xURLTransformer = URLTransformer::create( m_xContext );
        URL aURL;
        aURL.Complete = "vnd.sun.star.autorecovery:/doSessionSave";
        xURLTransformer->parseStrict(aURL);

        // in case of asynchronous call the notification will trigger saveDone()
        if ( bAsync )
            xDispatch->addStatusListener(this, aURL);

        Sequence< PropertyValue > args(1);
        args[0] = PropertyValue(OUString("DispatchAsynchron"),-1,makeAny(bAsync),PropertyState_DIRECT_VALUE);
        xDispatch->dispatch(aURL, args);
    } catch (const css::uno::Exception& e) {
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
    osl::MutexGuard g(m_aMutex);
    try
    {
        // xd create SERVICENAME_AUTORECOVERY -> frame::XDispatch
        // xd->dispatch("vnd.sun.star.autorecovery:/doSessionQuietQuit, async=false
        // it is done synchronously to avoid conflict with normal quit process

        css::uno::Reference< frame::XDispatch > xDispatch = css::frame::theAutoRecovery::get( m_xContext );
        css::uno::Reference< XURLTransformer > xURLTransformer = URLTransformer::create( m_xContext );
        URL aURL;
        aURL.Complete = "vnd.sun.star.autorecovery:/doSessionQuietQuit";
        xURLTransformer->parseStrict(aURL);

        Sequence< PropertyValue > args(1);
        args[0] = PropertyValue(OUString("DispatchAsynchron"),-1,makeAny(false),PropertyState_DIRECT_VALUE);
        xDispatch->dispatch(aURL, args);
    } catch (const css::uno::Exception& e) {
        SAL_WARN("fwk.session",e.Message);
    }
}

void SAL_CALL SessionListener::disposing(const css::lang::EventObject& Source)
{
    SAL_INFO("fwk.session", "SessionListener::disposing");
    if (Source.Source == m_rSessionManager) {
        m_rSessionManager.clear();
    }
}

void SAL_CALL SessionListener::initialize(const Sequence< Any  >& args)
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

    SAL_INFO("fwk.session.debug", "  m_bAllowUserInteractionOnQuit = " << (m_bAllowUserInteractionOnQuit ? "true" : "false"));
    if (!m_rSessionManager.is())
        m_rSessionManager = css::uno::Reference< frame::XSessionManagerClient >
            (m_xContext->getServiceManager()->createInstanceWithContext(aSMgr, m_xContext), UNO_QUERY);

    if (m_rSessionManager.is())
    {
        m_rSessionManager->addSessionManagerListener(this);
    }
}

void SAL_CALL SessionListener::statusChanged(const frame::FeatureStateEvent& event)
{
   SAL_INFO("fwk.session", "SessionListener::statusChanged");

   SAL_INFO("fwk.session.debug", "  ev.Feature = " << event.FeatureURL.Complete <<
                                 ", ev.Descript = " << event.FeatureDescriptor);
   if ( event.FeatureURL.Complete == "vnd.sun.star.autorecovery:/doSessionRestore" )
    {
        if (event.FeatureDescriptor == "update")
            m_bRestored = true; // a document was restored

    }
    else if ( event.FeatureURL.Complete == "vnd.sun.star.autorecovery:/doAutoSave" )
    {   // the "doSessionSave" was never set, look to framework/source/services/autorecovery.cxx
        //  it always testing but never setting (enum AutoRecovery::E_SESSION_SAVE)
        if (event.FeatureDescriptor == "update")
        {
            if (m_rSessionManager.is())
                m_rSessionManager->saveDone(this); // done with save
        }
    }
}

sal_Bool SAL_CALL SessionListener::doRestore()
{
    SAL_INFO("fwk.session", "SessionListener::doRestore");
    osl::MutexGuard g(m_aMutex);
    m_bRestored = false;
    try {
        css::uno::Reference< frame::XDispatch > xDispatch = css::frame::theAutoRecovery::get( m_xContext );

        URL aURL;
        aURL.Complete = "vnd.sun.star.autorecovery:/doSessionRestore";
        css::uno::Reference< XURLTransformer > xURLTransformer(URLTransformer::create(m_xContext));
        xURLTransformer->parseStrict(aURL);
        Sequence< PropertyValue > args;
        xDispatch->addStatusListener(this, aURL);
        xDispatch->dispatch(aURL, args);
        m_bRestored = true;

    } catch (const css::uno::Exception& e) {
        SAL_WARN("fwk.session",e.Message);
    }

    return m_bRestored;
}

void SAL_CALL SessionListener::doSave( sal_Bool bShutdown, sal_Bool /*bCancelable*/ )
{
    SAL_INFO("fwk.session", "SessionListener::doSave");

    SAL_INFO("fwk.session.debug", "  m_bAllowUserInteractionOnQuit = " << (m_bAllowUserInteractionOnQuit ? "true" : "false") <<
                                  ", bShutdown = " << (bShutdown ? "true" : "false"));
    if (bShutdown)
    {
        m_bSessionStoreRequested = true; // there is no need to protect it with mutex
        if ( m_bAllowUserInteractionOnQuit && m_rSessionManager.is() )
            m_rSessionManager->queryInteraction( static_cast< css::frame::XSessionManagerListener* >( this ) );
        else
            StoreSession( true );
    }
    // we don't have anything to do so tell the session manager we're done
    else if( m_rSessionManager.is() )
        m_rSessionManager->saveDone( this );
}

void SAL_CALL SessionListener::approveInteraction( sal_Bool bInteractionGranted )
{
    SAL_INFO("fwk.session", "SessionListener::approveInteraction");
    // do AutoSave as the first step
    osl::MutexGuard g(m_aMutex);

    if ( bInteractionGranted )
    {
        // close the office documents in normal way
        try
        {
            // first of all let the session be stored to be sure that we lose no information
            StoreSession( false );

            css::uno::Reference< css::frame::XDesktop2 > xDesktop = css::frame::Desktop::create( m_xContext );
            // honestly: how many implementations of XDesktop will we ever have?
            // so casting this directly to the implementation
            Desktop* pDesktop(dynamic_cast<Desktop*>(xDesktop.get()));
            if(pDesktop)
            {
                SAL_INFO("fwk.session", " XDesktop is a framework::Desktop -- good.");
                m_bTerminated = pDesktop->terminateQuickstarterToo();
            }
            else
            {
                SAL_WARN("fwk.session", " XDesktop is not a framework::Desktop -- this should never happen.");
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
            StoreSession( true );
            m_rSessionManager->interactionDone( this );
        }

        if ( m_rSessionManager.is() && m_bTerminated )
            m_rSessionManager->saveDone(this);
    }
    else
    {
        StoreSession( true );
    }
}

void SessionListener::shutdownCanceled()
{
    SAL_INFO("fwk.session", "SessionListener::shutdownCanceled");
    // set the state back
    m_bSessionStoreRequested = false; // there is no need to protect it with mutex

    if ( m_rSessionManager.is() )
        m_rSessionManager->saveDone(this);
}

void SessionListener::doQuit()
{
    SAL_INFO("fwk.session", "SessionListener::doQuit");
    if ( m_bSessionStoreRequested && !m_bTerminated )
    {
        // let the session be closed quietly in this case
        QuitSessionQuietly();
    }
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_frame_SessionListener_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    SAL_INFO("fwk.session", "com_sun_star_comp_frame_SessionListener_get_implementation");

    return cppu::acquire(new SessionListener(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
