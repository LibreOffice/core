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

#include "doceventnotifier.hxx"
#include "scriptdocument.hxx"

#include <com/sun/star/frame/theGlobalEventBroadcaster.hpp>

#include <vcl/svapp.hxx>

#include <tools/diagnose_ex.h>

#include <comphelper/processfactory.hxx>
#include <boost/noncopyable.hpp>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

namespace basctl
{

    using ::com::sun::star::document::XDocumentEventBroadcaster;
    using ::com::sun::star::document::XDocumentEventListener;
    using ::com::sun::star::document::DocumentEvent;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::frame::theGlobalEventBroadcaster;
    using ::com::sun::star::uno::UNO_QUERY;

    namespace csslang = ::com::sun::star::lang;

    // DocumentEventNotifier::Impl

    typedef ::cppu::WeakComponentImplHelper<   XDocumentEventListener
                                           >   DocumentEventNotifier_Impl_Base;

    enum ListenerAction
    {
        RegisterListener,
        RemoveListener
    };

    /** impl class for DocumentEventNotifier
    */
    class DocumentEventNotifier::Impl   :public ::boost::noncopyable
                                        ,public ::cppu::BaseMutex
                                        ,public DocumentEventNotifier_Impl_Base
    {
    public:
        Impl (DocumentEventListener&, Reference<XModel> const& rxDocument);
        virtual ~Impl ();

        // XDocumentEventListener
        virtual void SAL_CALL documentEventOccured( const DocumentEvent& Event ) throw (RuntimeException, std::exception) override;

        // XEventListener
        virtual void SAL_CALL disposing( const csslang::EventObject& Event ) throw (RuntimeException, std::exception) override;

        // ComponentHelper
        virtual void SAL_CALL disposing() override;

    private:
        /// determines whether the instance is already disposed
        bool    impl_isDisposed_nothrow() const { return m_pListener == nullptr; }

        /// disposes the instance
        void    impl_dispose_nothrow();

        /// registers or revokes the instance as listener at the global event broadcaster
        void    impl_listenerAction_nothrow( ListenerAction _eAction );

    private:
        DocumentEventListener*  m_pListener;
        Reference< XModel >     m_xModel;
    };

    DocumentEventNotifier::Impl::Impl (DocumentEventListener& rListener, Reference<XModel> const& rxDocument) :
        DocumentEventNotifier_Impl_Base(m_aMutex),
        m_pListener(&rListener),
        m_xModel(rxDocument)
    {
        osl_atomic_increment( &m_refCount );
        impl_listenerAction_nothrow( RegisterListener );
        osl_atomic_decrement( &m_refCount );
    }

    DocumentEventNotifier::Impl::~Impl ()
    {
        if ( !impl_isDisposed_nothrow() )
        {
            acquire();
            dispose();
        }
    }

    void SAL_CALL DocumentEventNotifier::Impl::documentEventOccured( const DocumentEvent& _rEvent ) throw (RuntimeException, std::exception)
    {
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        OSL_PRECOND( !impl_isDisposed_nothrow(), "DocumentEventNotifier::Impl::notifyEvent: disposed, but still getting events?" );
        if ( impl_isDisposed_nothrow() )
            return;

        Reference< XModel > xDocument( _rEvent.Source, UNO_QUERY );
        OSL_ENSURE( xDocument.is(), "DocumentEventNotifier::Impl::notifyEvent: illegal source document!" );
        if ( !xDocument.is() )
            return;

        struct EventEntry
        {
            const sal_Char* pEventName;
            void (DocumentEventListener::*listenerMethod)( const ScriptDocument& _rDocument );
        };
        EventEntry aEvents[] = {
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

        for ( size_t i=0; i < SAL_N_ELEMENTS( aEvents ); ++i )
        {
            if ( !_rEvent.EventName.equalsAscii( aEvents[i].pEventName ) )
                continue;

            ScriptDocument aDocument( xDocument );
            {
                // the listener implementations usually require the SolarMutex, so lock it here.
                // But ensure the proper order of locking the solar and the own mutex
                aGuard.clear();
                SolarMutexGuard aSolarGuard;
                ::osl::MutexGuard aGuard2( m_aMutex );

                if ( impl_isDisposed_nothrow() )
                    // somebody took the chance to dispose us -> bail out
                    return;

                (m_pListener->*aEvents[i].listenerMethod)( aDocument );
            }
            break;
        }
    }

    void SAL_CALL DocumentEventNotifier::Impl::disposing( const csslang::EventObject& /*Event*/ ) throw (RuntimeException, std::exception)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( !impl_isDisposed_nothrow() )
            impl_dispose_nothrow();
    }

    void SAL_CALL DocumentEventNotifier::Impl::disposing()
    {
        impl_listenerAction_nothrow( RemoveListener );
        impl_dispose_nothrow();
    }

    void DocumentEventNotifier::Impl::impl_dispose_nothrow()
    {
        m_pListener = nullptr;
        m_xModel.clear();
    }

    void DocumentEventNotifier::Impl::impl_listenerAction_nothrow( ListenerAction _eAction )
    {
        try
        {
            Reference< XDocumentEventBroadcaster > xBroadcaster;
            if ( m_xModel.is() )
                xBroadcaster.set( m_xModel, UNO_QUERY_THROW );
            else
            {
                Reference< css::uno::XComponentContext > aContext(
                    comphelper::getProcessComponentContext() );
                xBroadcaster = theGlobalEventBroadcaster::get(aContext);
            }

            void ( SAL_CALL XDocumentEventBroadcaster::*listenerAction )( const Reference< XDocumentEventListener >& ) =
                ( _eAction == RegisterListener ) ? &XDocumentEventBroadcaster::addDocumentEventListener : &XDocumentEventBroadcaster::removeDocumentEventListener;
            (xBroadcaster.get()->*listenerAction)( this );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
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
