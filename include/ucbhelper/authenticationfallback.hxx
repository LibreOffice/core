/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_UCBHELPER_AUTHENTICATIONFALLBACK_HXX
#define INCLUDED_UCBHELPER_AUTHENTICATIONFALLBACK_HXX

#include <rtl/ref.hxx>
#include <ucbhelper/interactionrequest.hxx>
#include <ucbhelper/ucbhelperdllapi.h>
#include <com/sun/star/ucb/AuthenticationFallbackRequest.hpp>


namespace ucbhelper {

/**
  * This class implements a simple authentication interaction request used
  * when programmatically authentication cannot succeed.
  *
  * Read-only values : instructions, url
  * Read-write values: code
  */
class UCBHELPER_DLLPUBLIC AuthenticationFallbackRequest : public ucbhelper::InteractionRequest
{
private:
    rtl::Reference< ucbhelper::InteractionAuthFallback > m_xAuthFallback;

public:
    /**
      * Constructor.
      *
      * @param rInstructions instructions to be followed by the user
      * @param rURL contains a URL for which authentication is requested.
      */
    AuthenticationFallbackRequest( const OUString & rInstructions,
                                 const OUString & rURL );

    const rtl::Reference< ucbhelper::InteractionAuthFallback >&
        getAuthFallbackInter( ) const { return m_xAuthFallback; }

};

}

#endif /* ! INCLUDED_UCBHELPER_AUTHENTICATIONFALLBACK_HXX*/

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
