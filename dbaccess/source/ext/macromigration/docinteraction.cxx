/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: docinteraction.cxx,v $
 * $Revision: 1.1.2.2 $
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

#include "docinteraction.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/task/DocumentPasswordRequest.hpp>
/** === end UNO includes === **/

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
            :xHandler( _rContext.createComponent( "com.sun.star.task.InteractionHandler" ), UNO_QUERY_THROW )
        {
        }
    };

    //====================================================================
    //= InteractionHandler
    //====================================================================
    //--------------------------------------------------------------------
    InteractionHandler::InteractionHandler( const ::comphelper::ComponentContext& _rContext )
        :m_pData( new InteractionHandler_Data( _rContext ) )
    {
    }

    //--------------------------------------------------------------------
    InteractionHandler::InteractionHandler( const Reference< XInteractionHandler >& _rxHandler )
        :m_pData( new InteractionHandler_Data( _rxHandler ) )
    {
    }

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
            _io_rPassword.getLength() ? PasswordRequestMode_PASSWORD_REENTER : PasswordRequestMode_PASSWORD_ENTER,
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
