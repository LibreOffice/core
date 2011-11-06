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
#include "precompiled_ucbhelper.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#include <osl/diagnose.h>
#include <cppuhelper/exc_hlp.hxx>
#include <com/sun/star/ucb/CommandFailedException.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#ifndef _UCBHELPER_INTERACTIONREQUEST_HXX
#include <ucbhelper/interactionrequest.hxx>
#endif
#include <ucbhelper/cancelcommandexecution.hxx>
#include <ucbhelper/simpleioerrorrequest.hxx>

using namespace com::sun::star;

namespace ucbhelper
{

//=========================================================================
void cancelCommandExecution( const uno::Any & rException,
                             const uno::Reference<
                                        ucb::XCommandEnvironment > & xEnv )
    throw( uno::Exception )
{
    if ( xEnv.is() )
    {
        uno::Reference<
            task::XInteractionHandler > xIH = xEnv->getInteractionHandler();
        if ( xIH.is() )
        {
            rtl::Reference< ucbhelper::InteractionRequest > xRequest
                = new ucbhelper::InteractionRequest( rException );

            uno::Sequence< uno::Reference< task::XInteractionContinuation > >
                aContinuations( 1 );
            aContinuations[ 0 ]
                = new ucbhelper::InteractionAbort( xRequest.get() );

            xRequest->setContinuations( aContinuations );

            xIH->handle( xRequest.get() );

            rtl::Reference< ucbhelper::InteractionContinuation > xSelection
                = xRequest->getSelection();

            if ( xSelection.is() )
                throw ucb::CommandFailedException(
                                    rtl::OUString(),
                                    uno::Reference< uno::XInterface >(),
                                    rException );
        }
    }

    cppu::throwException( rException );

    OSL_ENSURE( sal_False, "Return from cppu::throwException call!!!" );
    throw uno::RuntimeException();
}


//=========================================================================
void cancelCommandExecution( const ucb::IOErrorCode eError,
                             const uno::Sequence< uno::Any > & rArgs,
                             const uno::Reference<
                                ucb::XCommandEnvironment > & xEnv,
                             const rtl::OUString & rMessage,
                             const uno::Reference<
                                    ucb::XCommandProcessor > & xContext )
    throw( uno::Exception )
{
    rtl::Reference< ucbhelper::SimpleIOErrorRequest > xRequest
        = new ucbhelper::SimpleIOErrorRequest(
                                    eError, rArgs, rMessage, xContext );
    if ( xEnv.is() )
    {
        uno::Reference<
            task::XInteractionHandler > xIH = xEnv->getInteractionHandler();
        if ( xIH.is() )
        {
            xIH->handle( xRequest.get() );

            rtl::Reference< ucbhelper::InteractionContinuation > xSelection
                = xRequest->getSelection();

            if ( xSelection.is() )
                throw ucb::CommandFailedException( rtl::OUString(),
                                                   xContext,
                                                   xRequest->getRequest() );
        }
    }

    cppu::throwException( xRequest->getRequest() );

    OSL_ENSURE( sal_False, "Return from cppu::throwException call!!!" );
    throw uno::RuntimeException();
}

}
