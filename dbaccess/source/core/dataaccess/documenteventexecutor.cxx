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

#include "documenteventexecutor.hxx"

#include <com/sun/star/document/XDocumentEventBroadcaster.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>

#include <comphelper/namedvaluecollection.hxx>
#include <cppuhelper/weakref.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

namespace dbaccess
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::WeakReference;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::document::XDocumentEventBroadcaster;
    using ::com::sun::star::document::XEventsSupplier;
    using ::com::sun::star::container::XNameAccess;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::util::URLTransformer;
    using ::com::sun::star::util::XURLTransformer;
    using ::com::sun::star::frame::XDispatchProvider;
    using ::com::sun::star::frame::XDispatch;
    using ::com::sun::star::util::URL;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::frame::XController;
    using ::com::sun::star::document::DocumentEvent;

    using namespace ::com::sun::star;

    // DocumentEventExecutor_Data
    struct DocumentEventExecutor_Data
    {
        WeakReference< XEventsSupplier >    xDocument;
        Reference< XURLTransformer >        xURLTransformer;

        DocumentEventExecutor_Data( const Reference< XEventsSupplier >& _rxDocument )
            :xDocument( _rxDocument )
        {
        }
    };

    namespace
    {
        static void lcl_dispatchScriptURL_throw( DocumentEventExecutor_Data& _rDocExecData,
            const OUString& _rScriptURL, const DocumentEvent& _rTrigger )
        {
            Reference< XModel > xDocument( _rDocExecData.xDocument.get(), UNO_QUERY_THROW );

            Reference< XController > xController( xDocument->getCurrentController() );
            Reference< XDispatchProvider > xDispProv;
            if ( xController.is() )
                xDispProv.set( xController->getFrame(), UNO_QUERY );
            if ( !xDispProv.is() )
            {
                OSL_FAIL( "lcl_dispatchScriptURL_throw: no controller/frame? How should I dispatch?" );
                return;
            }

            URL aScriptURL;
            aScriptURL.Complete = _rScriptURL;
            if ( _rDocExecData.xURLTransformer.is() )
                _rDocExecData.xURLTransformer->parseStrict( aScriptURL );

            // unfortunately, executing a script can trigger all kind of complex stuff, and unfortunately, not
            // every component involved into this properly cares for thread safety. To be on the safe side,
            // we lock the solar mutex here.
            SolarMutexGuard aSolarGuard;

            Reference< XDispatch > xDispatch( xDispProv->queryDispatch( aScriptURL, OUString(), 0 ) );
            if ( !xDispatch.is() )
            {
                OSL_FAIL( "lcl_dispatchScriptURL_throw: no dispatcher for the script URL!" );
                return;
            }

            PropertyValue aEventParam;
            aEventParam.Value <<= _rTrigger;
            Sequence< PropertyValue > aDispatchArgs( &aEventParam, 1 );
            xDispatch->dispatch( aScriptURL, aDispatchArgs );
        }
    }

    // DocumentEventExecutor
    DocumentEventExecutor::DocumentEventExecutor( const Reference<XComponentContext> & _rContext,
            const Reference< XEventsSupplier >& _rxDocument )
        :m_pData( new DocumentEventExecutor_Data( _rxDocument ) )
    {
        Reference< XDocumentEventBroadcaster > xBroadcaster( _rxDocument, UNO_QUERY_THROW );

        osl_atomic_increment( &m_refCount );
        {
            xBroadcaster->addDocumentEventListener( this );
        }
        osl_atomic_decrement( &m_refCount );

        try
        {
            m_pData->xURLTransformer = URLTransformer::create(_rContext);
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    DocumentEventExecutor::~DocumentEventExecutor()
    {
    }

    void SAL_CALL DocumentEventExecutor::documentEventOccured( const DocumentEvent& _Event ) throw (RuntimeException)
    {
        Reference< XEventsSupplier > xEventsSupplier( m_pData->xDocument.get(), UNO_QUERY );
        if ( !xEventsSupplier.is() )
        {
            OSL_FAIL( "DocumentEventExecutor::documentEventOccurred: no document anymore, but still being notified?" );
            return;
        }

        Reference< XModel > xDocument( xEventsSupplier, UNO_QUERY_THROW );

        try
        {
            Reference< XNameAccess > xDocEvents( xEventsSupplier->getEvents().get(), UNO_SET_THROW );
            if ( !xDocEvents->hasByName( _Event.EventName ) )
            {
                // this is worth an assertion: We are listener at the very same document which we just asked
                // for its events. So when EventName is fired, why isn't it supported by xDocEvents?
                OSL_FAIL( "DocumentEventExecutor::documentEventOccurred: an unsupported event is notified!" );
                return;
            }

            const ::comphelper::NamedValueCollection aScriptDescriptor( xDocEvents->getByName( _Event.EventName ) );

            OUString sEventType;
            bool bScriptAssigned = aScriptDescriptor.get_ensureType( "EventType", sEventType );

            OUString sScript;
            bScriptAssigned = bScriptAssigned && aScriptDescriptor.get_ensureType( "Script", sScript );

            if ( !bScriptAssigned )
                // no script is assigned to this event
                return;

            bool bDispatchScriptURL = ( sEventType == "Script" || sEventType == "Service" );
            bool bNonEmptyScript = !sScript.isEmpty();

            OSL_ENSURE( bDispatchScriptURL && bNonEmptyScript,
                "DocumentEventExecutor::documentEventOccurred: invalid/unsupported script descriptor" );

            if ( bDispatchScriptURL && bNonEmptyScript )
            {
                lcl_dispatchScriptURL_throw( *m_pData, sScript, _Event );
            }
        }
        catch( const RuntimeException& ) { throw; }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    void SAL_CALL DocumentEventExecutor::disposing( const lang::EventObject& /*_Source*/ ) throw (RuntimeException)
    {
        // not interested in
    }

} // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
