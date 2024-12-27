/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPHELPER_SIMPLEFILEACCESSINTERACTION_HXX
#define INCLUDED_COMPHELPER_SIMPLEFILEACCESSINTERACTION_HXX

#include <sal/config.h>
#include <ucbhelper/interceptedinteraction.hxx>
#include <comphelper/comphelperdllapi.h>

namespace com::sun::star::task { class XInteractionHandler; }

namespace comphelper {
/** An interaction handler wrapper for simple file access.

    This wrapper is to be used together with XSimpleFileAccess when
    you want to avoid the error messages displayed when accessing a file (the messages activated
    by ucphelper::cancelCommandExecution).

    This wrapper is especially useful when you need to access a Web/DAV connection
    enabling https certificate validation and optionally enabling the authentication
    dialog that may be needed in these operations.

    @param xHandler
    Used handler, always needed.
    It will be used for Certificate Validation dialog or authentication dialog.
    The authentication is used in Web/DAV access when the server requests credentials to be accessed.
*/
class COMPHELPER_DLLPUBLIC SimpleFileAccessInteraction final : public ::ucbhelper::InterceptedInteraction
{

public:
    SimpleFileAccessInteraction(const css::uno::Reference< css::task::XInteractionHandler >& xHandler);
    virtual ~SimpleFileAccessInteraction() override;

private:

    virtual ucbhelper::InterceptedInteraction::EInterceptionState intercepted(const ::ucbhelper::InterceptedInteraction::InterceptedRequest& aRequest,
        const css::uno::Reference< css::task::XInteractionRequest >& xRequest) override;

};
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
