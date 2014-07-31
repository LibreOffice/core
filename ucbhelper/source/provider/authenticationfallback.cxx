/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <ucbhelper/authenticationfallback.hxx>

using namespace com::sun::star;
using namespace ucbhelper;

AuthenticationFallbackRequest::AuthenticationFallbackRequest(
                                      const OUString & rInstructions,
                                      const OUString & rURL )
{

    ucb::AuthenticationFallbackRequest aRequest;
    aRequest.instructions = rInstructions;
    aRequest.url = rURL;

    setRequest( uno::makeAny( aRequest ) );
    m_xAuthFallback = new InteractionAuthFallback( this );

    uno::Sequence<
        uno::Reference< task::XInteractionContinuation > > aContinuations( 2 );
    aContinuations[ 0 ] = new InteractionAbort( this );
    aContinuations[ 1 ] = m_xAuthFallback.get( );

    setContinuations( aContinuations );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
