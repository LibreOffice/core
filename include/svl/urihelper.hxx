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

#ifndef INCLUDED_SVL_URIHELPER_HXX
#define INCLUDED_SVL_URIHELPER_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <rtl/textenc.h>
#include <svl/svldllapi.h>
#include <tools/link.hxx>
#include <tools/urlobj.hxx>

namespace com { namespace sun { namespace star {
    namespace uno { class XComponentContext; }
    namespace uri { class XUriReference; }
} } }

class CharClass;

namespace URIHelper {

/**
   @ATT
   Calling this function with defaulted arguments rMaybeFileHdl = Link() and
   bCheckFileExists = true often leads to results that are not intended:
   Whenever the given rTheBaseURIRef is a file URL, the given rTheRelURIRef is
   relative, and rTheRelURIRef could also be smart-parsed as a non-file URL
   (e.g., the relative URL "foo/bar" can be smart-parsed as "http://foo/bar"),
   then SmartRel2Abs called with rMaybeFileHdl = Link() and bCheckFileExists =
   true returns the non-file URL interpretation.  To avoid this, either pass
   some non-null rMaybeFileHdl if you want to check generated file URLs for
   existence (see URIHelper::GetMaybeFileHdl), or use bCheckFileExists = false
   if you want to generate file URLs without checking for their existence.
*/
SVL_DLLPUBLIC OUString SmartRel2Abs(INetURLObject const & rTheBaseURIRef,
                                    OUString const & rTheRelURIRef,
                                    Link<OUString *, bool> const & rMaybeFileHdl = Link<OUString *, bool>(),
                                    bool bCheckFileExists = true,
                                    bool bIgnoreFragment = false,
                                    INetURLObject::EncodeMechanism eEncodeMechanism = INetURLObject::WAS_ENCODED,
                                    INetURLObject::DecodeMechanism eDecodeMechanism = INetURLObject::DECODE_TO_IURI,
                                    rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8,
                                    bool bRelativeNonURIs = false,
                                    INetURLObject::FSysStyle eStyle = INetURLObject::FSYS_DETECT);

SVL_DLLPUBLIC void SetMaybeFileHdl(Link<OUString *, bool> const & rTheMaybeFileHdl);

SVL_DLLPUBLIC Link<OUString *, bool> GetMaybeFileHdl();

/**
   Converts a URI reference to a relative one, ignoring certain differences (for
   example, treating file URLs for case-ignoring file systems
   case-insensitively).

   @param context a component context; must not be null

   @param baseUriReference a base URI reference

   @param uriReference a URI reference

   @return a URI reference representing the given uriReference relative to the
   given baseUriReference; if the given baseUriReference is not an absolute,
   hierarchical URI reference, or the given uriReference is not a valid URI
   reference, null is returned

   @exception std::bad_alloc if an out-of-memory condition occurs

   @exception css::uno::RuntimeException if any error occurs
 */
SVL_DLLPUBLIC css::uno::Reference< css::uri::XUriReference >
normalizedMakeRelative(
    css::uno::Reference< css::uno::XComponentContext > const & context,
    OUString const & baseUriReference,
    OUString const & uriReference);

/**
   A variant of normalizedMakeRelative with a simplified interface.

   Internally calls normalizedMakeRelative with the default component context.

   @param baseUriReference a base URI reference, passed to
   normalizedMakeRelative

   @param uriReference a URI reference, passed to normalizedMakeRelative

   @return if the XUriReference returned by normalizedMakeRelative is empty,
   uriReference is returned unmodified; otherwise, the result of calling
   XUriReference::getUriReference on the XUriReference returned by
   normalizedMakeRelative is returned

   @exception std::bad_alloc if an out-of-memory condition occurs

   @exception css::uno::RuntimeException if any error occurs

   @deprecated
   No code should rely on the default component context.
*/
SVL_DLLPUBLIC OUString simpleNormalizedMakeRelative( OUString const & baseUriReference,
                                                     OUString const & uriReference);

SVL_DLLPUBLIC OUString FindFirstURLInText(OUString const & rText,
                                          sal_Int32 & rBegin,
                                          sal_Int32 & rEnd,
                                          CharClass const & rCharClass,
                                          INetURLObject::EncodeMechanism eMechanism = INetURLObject::WAS_ENCODED,
                                          rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8,
                                          INetURLObject::FSysStyle eStyle = INetURLObject::FSYS_DETECT);

/** Remove any password component from both absolute and relative URLs.

    @ATT  The current implementation will not remove a password from a
    relative URL that has an authority component (e.g., the password is not
    removed from the relative ftp URL <//user:password@domain/path>).  But
    since our functions to translate between absolute and relative URLs never
    produce relative URLs with authority components, this is no real problem.

    @ATT  For relative URLs (or anything not recognized as an absolute URI),
    the current implementation will return the input unmodified, not applying
    any translations implied by the encode/decode parameters.

    @param rURI  An absolute or relative URI reference.

    @param eEncodeMechanism  See the general discussion for INetURLObject set-
    methods.

    @param eDecodeMechanism  See the general discussion for INetURLObject get-
    methods.

    @param eCharset  See the general discussion for INetURLObject get- and
    set-methods.

    @return  The input URI with any password component removed.
 */
SVL_DLLPUBLIC OUString removePassword(OUString const & rURI,
                                      INetURLObject::EncodeMechanism eEncodeMechanism = INetURLObject::WAS_ENCODED,
                                      INetURLObject::DecodeMechanism eDecodeMechanism = INetURLObject::DECODE_TO_IURI,
                                      rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);

/** Resolve a URL's host component domain name in IDNA syntax to plain DNS
    syntax.

    For details, see RFC 5890 "Internationalized Domain Names for Applications
    (IDNA): Definitions and Document Framework."

    @param: url  An arbitrary string, should be a URI.

    @return  If the input matches the syntax of a hierarchical URL, and it has
    a host component that matches the IDNA2008 domain name syntax, and that
    domain name contains any U-labels, return a version of the input URL with
    the host component resolved to plain DNS syntax.  Otherwise, return the
    input unchanged.
*/
SVL_DLLPUBLIC OUString resolveIdnaHost(OUString const & url);

}

#endif // INCLUDED_SVL_URIHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
