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

#ifndef INCLUDED_UNOTOOLS_UCBHELPER_HXX
#define INCLUDED_UNOTOOLS_UCBHELPER_HXX

#include <sal/config.h>

#include <unotools/unotoolsdllapi.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.hxx>

namespace com::sun::star {
    namespace uno {
        class Any;
        class XComponentContext;
    }
    namespace ucb{
        class XCommandEnvironment;
    }
}
namespace ucbhelper { class Content; }

namespace utl::UCBContentHelper {

UNOTOOLS_DLLPUBLIC bool IsDocument(OUString const & url);

UNOTOOLS_DLLPUBLIC bool IsFolder(OUString const & url);

/// @param title must not be null
/// @return true iff title has been set (i.e., if obtaining the "Title" property
///     of the given content yields a non-void value without raising a
///     non-RuntimeException; RuntimeExceptions are passed through)
UNOTOOLS_DLLPUBLIC bool GetTitle(
    OUString const & url, OUString * title);

UNOTOOLS_DLLPUBLIC bool Kill(OUString const & url);

UNOTOOLS_DLLPUBLIC css::uno::Any GetProperty(
    OUString const & url, OUString const & property);

UNOTOOLS_DLLPUBLIC bool MakeFolder(
    ucbhelper::Content & parent, OUString const & title,
    ucbhelper::Content & result);

/// like mkdir -p
UNOTOOLS_DLLPUBLIC bool ensureFolder(
    const css::uno::Reference< css::uno::XComponentContext >& xCtx,
    const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv,
    const OUString& rFolder, ucbhelper::Content & result) noexcept;

UNOTOOLS_DLLPUBLIC bool IsYounger(
    OUString const & younger, OUString const & older);

UNOTOOLS_DLLPUBLIC bool Exists(OUString const & url);

UNOTOOLS_DLLPUBLIC bool IsSubPath(
    OUString const & parent, OUString const & child);

UNOTOOLS_DLLPUBLIC bool EqualURLs(
    OUString const & url1, OUString const & url2);

/**
* Returns a default XCommandEnvironment to be used
* when creating a ucbhelper::Content.
*
* Due to the way the WebDAV UCP provider works, an interaction handler
* is always needed:
* 1) to activate the credential dialog or to provide the cached credentials
* whenever the server requests them;
*
* 2) in case of ssl connection (https) to activate the dialog to show the
* certificate if said certificate looks wrong or dubious.
*
* This helper provides the XCommandEnvironment with an interaction
* handler that intercepts:
* 1) css::ucb::AuthenticationRequest()
* 2) css::ucb::CertificateValidationRequest()
* 3) css::ucb::InteractiveIOException()
* 4) css::ucb::UnsupportedDataSinkException()
*
* Exception 1) and 2) will be passed to the UI handler, e.g. shown to
* the user for interaction.
*
* Exception 3) and 4) will be have a default 'Abort' result.
* See comphelper::StillReadWriteInteraction for details.
* comphelper::StillReadWriteInteraction was introduced in
* commit bbe51f039dffca2506ea542feb78571b6358b981.
*/
UNOTOOLS_DLLPUBLIC
    css::uno::Reference< css::ucb::XCommandEnvironment > getDefaultCommandEnvironment();

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
