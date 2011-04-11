/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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
#include "precompiled_ucbhelper.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#include <osl/diagnose.h>
#include <cppuhelper/exc_hlp.hxx>
#include <com/sun/star/ucb/CommandFailedException.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <ucbhelper/interactionrequest.hxx>
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

    OSL_FAIL( "Return from cppu::throwException call!!!" );
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

    OSL_FAIL( "Return from cppu::throwException call!!!" );
    throw uno::RuntimeException();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
