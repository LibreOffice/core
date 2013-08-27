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

#ifndef _UNOTOOLS_UCBHELPER_HXX
#define _UNOTOOLS_UCBHELPER_HXX

#include "sal/config.h"

#include "com/sun/star/uno/Sequence.hxx"
#include "sal/types.h"
#include "unotools/unotoolsdllapi.h"

namespace com { namespace sun { namespace star {
    namespace uno {
        class Any;
        class XComponentContext;
    }
    namespace ucb{
        class XCommandEnvironment;
    }
} } }
namespace ucbhelper { class Content; }

namespace utl { namespace UCBContentHelper {

UNOTOOLS_DLLPUBLIC bool IsDocument(OUString const & url);

UNOTOOLS_DLLPUBLIC bool IsFolder(OUString const & url);

/// @param title must not be null
/// @return true iff title has been set (i.e., if obtaining the "Title" property
///     of the given content yields a non-void value without raising a
///     non-RuntimeException; RuntimeExceptions are passed through)
UNOTOOLS_DLLPUBLIC bool GetTitle(
    OUString const & url, OUString * title);

UNOTOOLS_DLLPUBLIC bool Kill(OUString const & url);

UNOTOOLS_DLLPUBLIC com::sun::star::uno::Any GetProperty(
    OUString const & url, OUString const & property);

UNOTOOLS_DLLPUBLIC bool MakeFolder(
    ucbhelper::Content & parent, OUString const & title,
    ucbhelper::Content & result, bool exclusive = false);

/// like mkdir -p
UNOTOOLS_DLLPUBLIC bool ensureFolder(
    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > xCtx,
    com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment > xEnv,
    const OUString& rFolder, ucbhelper::Content & result);

/// @return the value of the "Size" property of the given content, or zero if
///     obtaining the property yields a void value or raises a
///     non-RuntimeException (RuntimeExceptions are passed through)
UNOTOOLS_DLLPUBLIC sal_Int64 GetSize(OUString const & url);

UNOTOOLS_DLLPUBLIC bool IsYounger(
    OUString const & younger, OUString const & older);

UNOTOOLS_DLLPUBLIC bool Exists(OUString const & url);

UNOTOOLS_DLLPUBLIC bool IsSubPath(
    OUString const & parent, OUString const & child);

UNOTOOLS_DLLPUBLIC bool EqualURLs(
    OUString const & url1, OUString const & url2);

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
