/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbmm.hxx"

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
