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

#ifndef _UNOTOOLS_UCBHELPER_HXX
#define _UNOTOOLS_UCBHELPER_HXX

#include "sal/config.h"

#include "com/sun/star/uno/Sequence.hxx"
#include "sal/types.h"
#include "unotools/unotoolsdllapi.h"

namespace com { namespace sun { namespace star { namespace uno {
    class Any;
} } } }
namespace rtl { class OUString; }
namespace ucbhelper { class Content; }

namespace utl { namespace UCBContentHelper {

UNOTOOLS_DLLPUBLIC bool IsDocument(rtl::OUString const & url);

UNOTOOLS_DLLPUBLIC bool IsFolder(rtl::OUString const & url);

/// @param title must not be null
/// @return true iff title has been set (i.e., if obtaining the "Title" property
///     of the given content yields a non-void value without raising a
///     non-RuntimeException; RuntimeExceptions are passed through)
UNOTOOLS_DLLPUBLIC bool GetTitle(
    rtl::OUString const & url, rtl::OUString * title);

UNOTOOLS_DLLPUBLIC bool Kill(rtl::OUString const & url);

UNOTOOLS_DLLPUBLIC com::sun::star::uno::Any GetProperty(
    rtl::OUString const & url, rtl::OUString const & property);

UNOTOOLS_DLLPUBLIC bool MakeFolder(
    rtl::OUString const & url, bool exclusive = false);

UNOTOOLS_DLLPUBLIC bool MakeFolder(
    ucbhelper::Content & parent, rtl::OUString const & title,
    ucbhelper::Content & result, bool exclusive = false);

/// @return the value of the "Size" property of the given content, or zero if
///     obtaining the property yields a void value or raises a
///     non-RuntimeException (RuntimeExceptions are passed through)
UNOTOOLS_DLLPUBLIC sal_Int64 GetSize(rtl::OUString const & url);

UNOTOOLS_DLLPUBLIC bool IsYounger(
    rtl::OUString const & younger, rtl::OUString const & older);

UNOTOOLS_DLLPUBLIC bool Exists(rtl::OUString const & url);

UNOTOOLS_DLLPUBLIC bool IsSubPath(
    rtl::OUString const & parent, rtl::OUString const & child);

UNOTOOLS_DLLPUBLIC bool EqualURLs(
    rtl::OUString const & url1, rtl::OUString const & url2);

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
