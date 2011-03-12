/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
#include "precompiled_dbaccess.hxx"

#include "documenteventexecutor.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/document/XDocumentEventBroadcaster.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
/** === end UNO includes === **/

#include <comphelper/componentcontext.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <cppuhelper/weakref.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

namespace dbaccess
{

    /** === begin UNO using === **/
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
    using ::com::sun::star::document::XDocumentEventBroadcaster;
    using ::com::sun::star::document::XEventsSupplier;
    using ::com::sun::star::container::XNameAccess;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::util::XURLTransformer;
    using ::com::sun::star::frame::XDispatchProvider;
    using ::com::sun::star::frame::XDispatch;
    using ::com::sun::star::util::URL;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::frame::XController;
    using ::com::sun::star::document::DocumentEvent;
    /** === end UNO using === **/
    using namespace ::com::sun::star;

    //====================================================================
    //= DocumentEventExecutor_Data
    //====================================================================
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
            const ::rtl::OUString& _rScriptURL, const DocumentEvent& _rTrigger )
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

            Reference< XDispatch > xDispatch( xDispProv->queryDispatch( aScriptURL, ::rtl::OUString(), 0 ) );
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

    //====================================================================
    //= DocumentEventExecutor
    //====================================================================
    DocumentEventExecutor::DocumentEventExecutor( const ::comphelper::ComponentContext& _rContext,
            const Reference< XEventsSupplier >& _rxDocument )
        :m_pData( new DocumentEventExecutor_Data( _rxDocument ) )
    {
        Reference< XDocumentEventBroadcaster > xBroadcaster( _rxDocument, UNO_QUERY_THROW );

        osl_incrementInterlockedCount( &m_refCount );
        {
            xBroadcaster->addDocumentEventListener( this );
        }
        osl_decrementInterlockedCount( &m_refCount );

        try
        {
            _rContext.createComponent( "com.sun.star.util.URLTransformer", m_pData->xURLTransformer );
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

            ::rtl::OUString sEventType;
            bool bScriptAssigned = aScriptDescriptor.get_ensureType( "EventType", sEventType );

            ::rtl::OUString sScript;
            bScriptAssigned = bScriptAssigned && aScriptDescriptor.get_ensureType( "Script", sScript );

            if ( !bScriptAssigned )
                // no script is assigned to this event
                return;

            bool bDispatchScriptURL =
                (   sEventType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Script" ) )
                ||  sEventType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Service" ) )
                );
            bool bNonEmptyScript = sScript.getLength() != 0;

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
