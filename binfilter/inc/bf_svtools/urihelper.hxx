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

#ifndef SVTOOLS_URIHELPER_HXX
#define SVTOOLS_URIHELPER_HXX

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uri/XUriReference.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

#include <rtl/textenc.h>
#include <rtl/ustring.hxx>

#include <unotools/charclass.hxx>
#include <tools/link.hxx>
#include <tools/solar.h>
#include <tools/urlobj.hxx>
#include <tools/string.hxx>


//============================================================================
namespace binfilter
{

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
   existence (see GetMaybeFileHdl), or use bCheckFileExists = false
   if you want to generate file URLs without checking for their existence.
*/
 UniString
SmartRel2Abs(INetURLObject const & rTheBaseURIRef,
             UniString const & rTheRelURIRef,
             Link const & rMaybeFileHdl = Link(),
             bool bCheckFileExists = true,
             bool bIgnoreFragment = false,
             INetURLObject::EncodeMechanism eEncodeMechanism
                 = INetURLObject::WAS_ENCODED,
             INetURLObject::DecodeMechanism eDecodeMechanism
                 = INetURLObject::DECODE_TO_IURI,
             rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8,
             bool bRelativeNonURIs = false,
             INetURLObject::FSysStyle eStyle = INetURLObject::FSYS_DETECT);

//============================================================================
 void SetMaybeFileHdl(Link const & rTheMaybeFileHdl);

//============================================================================
 Link GetMaybeFileHdl();

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

   @exception com::sun::star::uno::RuntimeException if any error occurs
 */
 com::sun::star::uno::Reference< com::sun::star::uri::XUriReference >
normalizedMakeRelative(
    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
    const & context,
    rtl::OUString const & baseUriReference, rtl::OUString const & uriReference);

/**
   A variant of normalizedMakeRelative with a simplified interface.

   Internally calls normalizedMakeRelative with the default component context.

   @param baseUriReference a base URI reference, passed to
   normalizedMakeRelative

   @param uriReference a URI reference, passed to normalizedMakeRelative

   @return if the XUriReference returnd by normalizedMakeRelative is empty,
   uriReference is returned unmodified; otherwise, the result of calling
   XUriReference::getUriReference on the XUriReference returnd by
   normalizedMakeRelative is returned

   @exception std::bad_alloc if an out-of-memory condition occurs

   @exception com::sun::star::uno::RuntimeException if any error occurs

   @deprecated
   No code should rely on the default component context.
*/
 rtl::OUString simpleNormalizedMakeRelative(
    rtl::OUString const & baseUriReference, rtl::OUString const & uriReference);

//============================================================================
 UniString
FindFirstURLInText(UniString const & rText,
                   xub_StrLen & rBegin,
                   xub_StrLen & rEnd,
                   CharClass const & rCharClass,
                   INetURLObject::EncodeMechanism eMechanism
                       = INetURLObject::WAS_ENCODED,
                   rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8,
                   INetURLObject::FSysStyle eStyle
                       = INetURLObject::FSYS_DETECT);

//============================================================================
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
 UniString
removePassword(UniString const & rURI,
               INetURLObject::EncodeMechanism eEncodeMechanism
                   = INetURLObject::WAS_ENCODED,
               INetURLObject::DecodeMechanism eDecodeMechanism
                   = INetURLObject::DECODE_TO_IURI,
               rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);
}

#endif // SVTOOLS_URIHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
