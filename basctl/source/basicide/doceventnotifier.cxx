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

#include <doceventnotifier.hxx>
#include <basctl/scriptdocument.hxx>

#include <com/sun/star/frame/theGlobalEventBroadcaster.hpp>

#include <vcl/svapp.hxx>

#include <comphelper/compbase.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <comphelper/processfactory.hxx>

namespace basctl
{

    using ::com::sun::star::document::XDocumentEventBroadcaster;
    using ::com::sun::star::document::XDocumentEventListener;
    using ::com::sun::star::document::DocumentEvent;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::frame::theGlobalEventBroadcaster;
    using ::com::sun::star::uno::UNO_QUERY;

    // DocumentEventNotifier::Impl

    typedef ::comphelper::WeakComponentImplHelper<   XDocumentEventListener
                                           >   DocumentEventNotifier_Impl_Base;

    namespace {

    enum ListenerAction
    {
        RegisterListener,
        RemoveListener
    };

    }

    /** impl class for DocumentEventNotifier
    */
    class DocumentEventNotifier::Impl : public DocumentEventNotifier_Impl_Base
    {
    public:
        // noncopyable
        Impl(const Impl&) = delete;
        Impl& operator=(const Impl&) = delete;

        Impl (DocumentEventListener&, Reference<XModel> const& rxDocument);
        virtual ~Impl () override;

        // XDocumentEventListener
        virtual void SAL_CALL documentEventOccured( const DocumentEvent& Event ) override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Event ) override;

        // WeakComponentImplHelper
        virtual void disposing(std::unique_lock<std::mutex>&) override;

    private:
        /// determines whether the instance is already disposed
        bool    impl_isDisposed_nothrow(std::unique_lock<std::mutex>& /*rGuard*/) const { return m_pListener == nullptr; }

        /// disposes the instance
        void    impl_dispose_nothrow(std::unique_lock<std::mutex>& rGuard);

        /// registers or revokes the instance as listener at the global event broadcaster
        void    impl_listenerAction_nothrow( std::unique_lock<std::mutex>& rGuard, ListenerAction _eAction );

    private:
        DocumentEventListener*  m_pListener;
        Reference< XModel >     m_xModel;
    };

    DocumentEventNotifier::Impl::Impl (DocumentEventListener& rListener, Reference<XModel> const& rxDocument) :
        m_pListener(&rListener),
        m_xModel(rxDocument)
    {
        std::unique_lock aGuard(m_aMutex);
        osl_atomic_increment( &m_refCount );
        impl_listenerAction_nothrow( aGuard, RegisterListener );
        osl_atomic_decrement( &m_refCount );
    }

    DocumentEventNotifier::Impl::~Impl ()
    {
        disposeOnDestruct();
    }

    void SAL_CALL DocumentEventNotifier::Impl::documentEventOccured( const DocumentEvent& _rEvent )
    {
        std::unique_lock aGuard( m_aMutex );

        OSL_PRECOND( !impl_isDisposed_nothrow(aGuard), "DocumentEventNotifier::Impl::notifyEvent: disposed, but still getting events?" );
        if ( impl_isDisposed_nothrow(aGuard) )
            return;

        Reference< XModel > xDocument( _rEvent.Source, UNO_QUERY );
        OSL_ENSURE( xDocument.is(), "DocumentEventNotifier::Impl::notifyEvent: illegal source document!" );
        if ( !xDocument.is() )
            return;

        struct EventEntry
        {
            const char* pEventName;
            void (DocumentEventListener::*listenerMethod)( const ScriptDocument& _rDocument );
        };
        static EventEntry const aEvents[] = {
            { "OnNew",          &DocumentEventListener::onDocumentCreated },
            { "OnLoad",         &DocumentEventListener::onDocumentOpened },
            { "OnSave",         &DocumentEventListener::onDocumentSave },
            { "OnSaveDone",     &DocumentEventListener::onDocumentSaveDone },
            { "OnSaveAs",       &DocumentEventListener::onDocumentSaveAs },
            { "OnSaveAsDone",   &DocumentEventListener::onDocumentSaveAsDone },
            { "OnUnload",       &DocumentEventListener::onDocumentClosed },
            { "OnTitleChanged", &DocumentEventListener::onDocumentTitleChanged },
            { "OnModeChanged",  &DocumentEventListener::onDocumentModeChanged }
        };

        for (EventEntry const & aEvent : aEvents)
        {
            if ( !_rEvent.EventName.equalsAscii( aEvent.pEventName ) )
                continue;

            // Listener implementations require that we hold the mutex, but to avoid lock ordering issues,
            // we need to take the solar mutex before we take our own mutex.
            aGuard.unlock();

            // Listener implements require that we hold the solar mutex.
            SolarMutexGuard aSolarGuard;

            // Take the lock again, so we can check our local fields.
            aGuard.lock();
            if ( impl_isDisposed_nothrow(aGuard) )
                // somebody took the chance to dispose us -> bail out
                return;
            DocumentEventListener* pListener = m_pListener;
            ScriptDocument aDocument( xDocument );
            // We cannot call the listener while holding our mutex because the listener
            // call might trigger an event which call back into us.
            aGuard.unlock();

            (pListener->*aEvent.listenerMethod)( aDocument );

            break;
        }
    }

    void SAL_CALL DocumentEventNotifier::Impl::disposing( const css::lang::EventObject& /*Event*/ )
    {
        SolarMutexGuard aSolarGuard;
        std::unique_lock aGuard( m_aMutex );

        if ( !impl_isDisposed_nothrow(aGuard) )
            impl_dispose_nothrow(aGuard);
    }

    void DocumentEventNotifier::Impl::disposing(std::unique_lock<std::mutex>& rGuard)
    {
        impl_listenerAction_nothrow( rGuard, RemoveListener );
        impl_dispose_nothrow(rGuard);
    }

    void DocumentEventNotifier::Impl::impl_dispose_nothrow(std::unique_lock<std::mutex>& /*rGuard*/)
    {
        m_pListener = nullptr;
        m_xModel.clear();
    }

    void DocumentEventNotifier::Impl::impl_listenerAction_nothrow( std::unique_lock<std::mutex>& rGuard, ListenerAction _eAction )
    {
        try
        {
            Reference< XDocumentEventBroadcaster > xBroadcaster;
            if ( m_xModel.is() )
                xBroadcaster.set( m_xModel, UNO_QUERY_THROW );
            else
            {
                const Reference< css::uno::XComponentContext >& aContext(
                    comphelper::getProcessComponentContext() );
                xBroadcaster = theGlobalEventBroadcaster::get(aContext);
            }

            void ( SAL_CALL XDocumentEventBroadcaster::*listenerAction )( const Reference< XDocumentEventListener >& ) =
                ( _eAction == RegisterListener ) ? &XDocumentEventBroadcaster::addDocumentEventListener : &XDocumentEventBroadcaster::removeDocumentEventListener;

            rGuard.unlock();
            (xBroadcaster.get()->*listenerAction)( this );
            rGuard.lock();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("basctl.basicide");
        }
    }

    // DocumentEventNotifier

    DocumentEventNotifier::DocumentEventNotifier (DocumentEventListener& rListener, Reference<XModel> const& rxDocument) :
        m_pImpl(new Impl(rListener, rxDocument))
    { }

    DocumentEventNotifier::DocumentEventNotifier (DocumentEventListener& rListener) :
        m_pImpl(new Impl(rListener, Reference<XModel>()))
    { }

    DocumentEventNotifier::~DocumentEventNotifier()
    {
    }

    void DocumentEventNotifier::dispose()
    {
        m_pImpl->dispose();
    }

    // DocumentEventListener

    DocumentEventListener::~DocumentEventListener()
    {
    }

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
