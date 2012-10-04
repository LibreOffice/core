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


#include "docinteraction.hxx"

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/task/DocumentPasswordRequest.hpp>

#include <comphelper/componentcontext.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/interaction.hxx>
#include <rtl/ref.hxx>
#include <tools/diagnose_ex.h>

//........................................................................
namespace dbmm
{
//........................................................................

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
    using ::com::sun::star::task::XInteractionHandler;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::task::DocumentPasswordRequest;
    using ::com::sun::star::task::InteractionClassification_QUERY;
    using ::com::sun::star::task::PasswordRequestMode_PASSWORD_ENTER;
    using ::com::sun::star::task::PasswordRequestMode_PASSWORD_REENTER;
    /** === end UNO using === **/

    //====================================================================
    //= InteractionHandler_Data
    //====================================================================
    struct InteractionHandler_Data
    {
        Reference< XInteractionHandler >    xHandler;

        InteractionHandler_Data( const Reference< XInteractionHandler >& _rxHandler )
            :xHandler( _rxHandler )
        {
        }

        InteractionHandler_Data( const ::comphelper::ComponentContext& _rContext )
            :xHandler( ::com::sun::star::task::InteractionHandler::createWithParent(_rContext.getUNOContext(), 0), UNO_QUERY )
        {
        }
    };

    //====================================================================
    //= InteractionHandler
    //====================================================================
    //--------------------------------------------------------------------
    InteractionHandler::InteractionHandler( const ::comphelper::ComponentContext& _rContext, const Reference< XModel >& _rxDocument )
        :m_pData( new InteractionHandler_Data( _rContext ) )
    {
        // check whether the doumentc has an own interaction handler set
        ::comphelper::NamedValueCollection aDocArgs( _rxDocument->getArgs() );
        m_pData->xHandler = aDocArgs.getOrDefault( "InteractionHandler", m_pData->xHandler );
    }

    //--------------------------------------------------------------------
    InteractionHandler::~InteractionHandler()
    {
    }

    //--------------------------------------------------------------------
    bool InteractionHandler::requestDocumentPassword( const ::rtl::OUString& _rDocumentName, ::rtl::OUString& _io_rPassword )
    {
        // create request
        DocumentPasswordRequest aRequest(
            ::rtl::OUString(), NULL,
            InteractionClassification_QUERY,
            _io_rPassword.isEmpty() ? PasswordRequestMode_PASSWORD_ENTER : PasswordRequestMode_PASSWORD_REENTER,
            _rDocumentName
        );

        ::rtl::Reference< ::comphelper::OInteractionRequest > pRequest( new ::comphelper::OInteractionRequest( makeAny( aRequest ) ) );
        ::rtl::Reference< ::comphelper::OInteractionPassword > pPassword( new ::comphelper::OInteractionPassword( _io_rPassword ) );
        ::rtl::Reference< ::comphelper::OInteractionAbort > pAbort( new ::comphelper::OInteractionAbort );
        pRequest->addContinuation( pPassword.get() );
        pRequest->addContinuation( pAbort.get() );

        // handle
        m_pData->xHandler->handle( pRequest.get() );

        // finish up
        if ( pAbort->wasSelected() )
            return false;

        _io_rPassword = pPassword->getPassword();
        return true;
    }

    //--------------------------------------------------------------------
    void InteractionHandler::reportError( const Any& _rError )
    {
        ::rtl::Reference< ::comphelper::OInteractionRequest > pRequest( new ::comphelper::OInteractionRequest( _rError ) );
        ::rtl::Reference< ::comphelper::OInteractionApprove > pApprove( new ::comphelper::OInteractionApprove );
        pRequest->addContinuation( pApprove.get() );

        m_pData->xHandler->handle( pRequest.get() );
    }

//........................................................................
} // namespace dbmm
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
