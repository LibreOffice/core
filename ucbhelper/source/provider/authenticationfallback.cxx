/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <ucbhelper/authenticationfallback.hxx>
#include <com/sun/star/ucb/AuthenticationFallbackRequest.hpp>

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

    setContinuations({ new InteractionAbort(this), m_xAuthFallback.get() });
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
