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

#include "documenteventnotifier.hxx"

#include <com/sun/star/frame/DoubleInitializationException.hpp>

#include <comphelper/asyncnotification.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <cppuhelper/weak.hxx>
#include <tools/diagnose_ex.h>

namespace dbaccess
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::frame::DoubleInitializationException;
    using ::com::sun::star::document::XDocumentEventListener;
    using ::com::sun::star::document::DocumentEvent;
    using ::com::sun::star::frame::XController2;

    using namespace ::com::sun::star;

    // DocumentEventHolder
    typedef ::comphelper::EventHolder< DocumentEvent >  DocumentEventHolder;

    // DocumentEventNotifier_Impl
    class DocumentEventNotifier_Impl : public ::comphelper::IEventProcessor
    {
        oslInterlockedCount                                     m_refCount;
        ::cppu::OWeakObject&                                    m_rDocument;
        ::osl::Mutex&                                           m_rMutex;
        bool                                                    m_bInitialized;
        bool                                                    m_bDisposed;
        ::std::shared_ptr<::comphelper::AsyncEventNotifierAutoJoin> m_pEventBroadcaster;
        ::comphelper::OInterfaceContainerHelper2                      m_aLegacyEventListeners;
        ::comphelper::OInterfaceContainerHelper2                      m_aDocumentEventListeners;

    public:
        DocumentEventNotifier_Impl( ::cppu::OWeakObject& _rBroadcasterDocument, ::osl::Mutex& _rMutex )
            :m_refCount( 0 )
            ,m_rDocument( _rBroadcasterDocument )
            ,m_rMutex( _rMutex )
            ,m_bInitialized( false )
            ,m_bDisposed( false )
            ,m_aLegacyEventListeners( _rMutex )
            ,m_aDocumentEventListeners( _rMutex )
        {
        }

        // IEventProcessor
        virtual void SAL_CALL acquire() throw () override;
        virtual void SAL_CALL release() throw () override;

        void addLegacyEventListener( const Reference< document::XEventListener >& Listener )
        {
            m_aLegacyEventListeners.addInterface( Listener );
        }

        void removeLegacyEventListener( const Reference< document::XEventListener >& Listener )
        {
            m_aLegacyEventListeners.removeInterface( Listener );
        }

        void addDocumentEventListener( const Reference< XDocumentEventListener >& Listener )
        {
            m_aDocumentEventListeners.addInterface( Listener );
        }

        void removeDocumentEventListener( const Reference< XDocumentEventListener >& Listener )
        {
            m_aDocumentEventListeners.removeInterface( Listener );
        }

        void disposing();

        void onDocumentInitialized();

        void    notifyDocumentEvent( const OUString& EventName, const Reference< XController2 >& ViewController,
                    const Any& Supplement )
        {
            impl_notifyEvent_nothrow( DocumentEvent(
                m_rDocument, EventName, ViewController, Supplement ) );
        }

        void    notifyDocumentEventAsync( const OUString& EventName, const Reference< XController2 >& ViewController,
                    const Any& Supplement )
        {
            impl_notifyEventAsync_nothrow( DocumentEvent(
                m_rDocument, EventName, ViewController, Supplement ) );
        }

    protected:
        virtual ~DocumentEventNotifier_Impl()
        {
        }

        // IEventProcessor
        virtual void processEvent( const ::comphelper::AnyEvent& _rEvent ) override;

    private:
        void impl_notifyEvent_nothrow( const DocumentEvent& _rEvent );
        void impl_notifyEventAsync_nothrow( const DocumentEvent& _rEvent );
    };

    void SAL_CALL DocumentEventNotifier_Impl::acquire() throw ()
    {
        osl_atomic_increment( &m_refCount );
    }

    void SAL_CALL DocumentEventNotifier_Impl::release() throw ()
    {
        if ( 0 == osl_atomic_decrement( &m_refCount ) )
            delete this;
    }

    void DocumentEventNotifier_Impl::disposing()
    {
        // SYNCHRONIZED ->
        // cancel any pending asynchronous events
        ::osl::ResettableMutexGuard aGuard( m_rMutex );
        if (m_pEventBroadcaster)
        {
            m_pEventBroadcaster->removeEventsForProcessor( this );
            m_pEventBroadcaster->terminate();
                //TODO: a protocol is missing how to join with the thread before
                // exit(3), to ensure the thread is no longer relying on any
                // infrastructure while that infrastructure is being shut down
                // in atexit handlers; simply calling join here leads to
                // deadlock, as this thread holds the solar mutex while the
                // other thread is typically blocked waiting for the solar mutex
                // For now, use newAutoJoinAsyncEventNotifier which is
                // better than nothing.
            m_pEventBroadcaster.reset();
        }

        lang::EventObject aEvent( m_rDocument );
        aGuard.clear();
        // <-- SYNCHRONIZED

        m_aLegacyEventListeners.disposeAndClear( aEvent );
        m_aDocumentEventListeners.disposeAndClear( aEvent );

        // SYNCHRONIZED ->
        aGuard.reset();
        m_bDisposed = true;
        // <-- SYNCHRONIZED
    }

    void DocumentEventNotifier_Impl::onDocumentInitialized()
    {
        if ( m_bInitialized )
            throw DoubleInitializationException();

        m_bInitialized = true;
        if (m_pEventBroadcaster)
        {
            // there are already pending asynchronous events
            ::comphelper::AsyncEventNotifierAutoJoin::launch(m_pEventBroadcaster);
        }
    }

    void DocumentEventNotifier_Impl::impl_notifyEvent_nothrow( const DocumentEvent& _rEvent )
    {
        OSL_PRECOND( m_bInitialized,
            "DocumentEventNotifier_Impl::impl_notifyEvent_nothrow: only to be called when the document is already initialized!" );
        try
        {
            document::EventObject aLegacyEvent( _rEvent.Source, _rEvent.EventName );
            m_aLegacyEventListeners.notifyEach( &document::XEventListener::notifyEvent, aLegacyEvent );
        }
        catch(const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        try
        {
            m_aDocumentEventListeners.notifyEach( &XDocumentEventListener::documentEventOccured, _rEvent );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    void DocumentEventNotifier_Impl::impl_notifyEventAsync_nothrow( const DocumentEvent& _rEvent )
    {
        if (!m_pEventBroadcaster)
        {
            m_pEventBroadcaster = ::comphelper::AsyncEventNotifierAutoJoin
                ::newAsyncEventNotifierAutoJoin("DocumentEventNotifier");
            if ( m_bInitialized )
            {
                // start processing the events if and only if we (our document, respectively) are
                // already initialized
                ::comphelper::AsyncEventNotifierAutoJoin::launch(m_pEventBroadcaster);
            }
        }
        m_pEventBroadcaster->addEvent( new DocumentEventHolder( _rEvent ), this );
    }

    void DocumentEventNotifier_Impl::processEvent( const ::comphelper::AnyEvent& _rEvent )
    {
        // beware, this is called from the notification thread
        {
            ::osl::MutexGuard aGuard( m_rMutex );
            if  ( m_bDisposed )
                return;
        }
        const DocumentEventHolder& rEventHolder = dynamic_cast< const DocumentEventHolder& >( _rEvent );
        impl_notifyEvent_nothrow( rEventHolder.getEventObject() );
    }

    // DocumentEventNotifier
    DocumentEventNotifier::DocumentEventNotifier( ::cppu::OWeakObject& _rBroadcasterDocument, ::osl::Mutex& _rMutex )
        :m_pImpl( new DocumentEventNotifier_Impl( _rBroadcasterDocument, _rMutex ) )
    {
    }

    DocumentEventNotifier::~DocumentEventNotifier()
    {
    }

    void DocumentEventNotifier::disposing()
    {
        m_pImpl->disposing();
    }

    void DocumentEventNotifier::onDocumentInitialized()
    {
        m_pImpl->onDocumentInitialized();
    }

    void DocumentEventNotifier::addLegacyEventListener( const Reference< document::XEventListener >& Listener )
    {
        m_pImpl->addLegacyEventListener( Listener );
    }

    void DocumentEventNotifier::removeLegacyEventListener( const Reference< document::XEventListener >& Listener )
    {
        m_pImpl->removeLegacyEventListener( Listener );
    }

    void DocumentEventNotifier::addDocumentEventListener( const Reference< XDocumentEventListener >& Listener )
    {
        m_pImpl->addDocumentEventListener( Listener );
    }

    void DocumentEventNotifier::removeDocumentEventListener( const Reference< XDocumentEventListener >& Listener )
    {
        m_pImpl->removeDocumentEventListener( Listener );
    }

    void DocumentEventNotifier::notifyDocumentEvent( const OUString& EventName,
        const Reference< XController2 >& ViewController, const Any& Supplement )
    {
        m_pImpl->notifyDocumentEvent( EventName, ViewController, Supplement );
    }

    void DocumentEventNotifier::notifyDocumentEventAsync( const OUString& EventName,
        const Reference< XController2 >& ViewController, const Any& Supplement )
    {
        m_pImpl->notifyDocumentEventAsync( EventName, ViewController, Supplement );
    }

} // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
