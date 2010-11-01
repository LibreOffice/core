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

#ifndef _UCBHELPER_FILEIDENTIFIERCONVERTER_HXX_
#define _UCBHELPER_FILEIDENTIFIERCONVERTER_HXX_

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <sal/types.h>
#include "ucbhelper/ucbhelperdllapi.h"

namespace com { namespace sun { namespace star { namespace ucb {
    class XContentProviderManager;
} } } }
namespace rtl { class OUString; }

namespace ucbhelper {

//============================================================================
/** Get a 'root' URL for the most 'local' file content provider.

    @descr
    The result can be used as the rBaseURL parameter of
    ucb::getFileURLFromSystemPath().

    @param rManager
    A content provider manager.  Must not be null.

    @returns
    either a 'root' URL for the most 'local' file content provider, or an
    empty string, if no such URL can meaningfully be constructed.
 */
UCBHELPER_DLLPUBLIC rtl::OUString getLocalFileURL(
    com::sun::star::uno::Reference<
            com::sun::star::ucb::XContentProviderManager > const &
        rManager)
    SAL_THROW((com::sun::star::uno::RuntimeException));

//============================================================================
/** Using a specific content provider manager, convert a file path in system
    dependent notation to a (file) URL.

    @param rManager
    A content provider manager.  Must not be null.

    @param rBaseURL
    See the corresponding parameter of
    com::sun::star::ucb::XFileIdentifierConverter::getFileURLFromSystemPath().

    @param rURL
    See the corresponding parameter of
    com::sun::star::ucb::XFileIdentifierConverter::getFileURLFromSystemPath().

    @returns
    a URL, if the content provider registered at the content provider manager
    that is responsible for the base URL returns a URL when calling
    com::sun::star::ucb::XFileIdentiferConverter::getFileURLFromSystemPath()
    on it.  Otherwise, an empty string is returned.

    @see
    com::sun::star::ucb::XFileIdentiferConverter::getFileURLFromSystemPath().
 */
UCBHELPER_DLLPUBLIC rtl::OUString
getFileURLFromSystemPath(
    com::sun::star::uno::Reference<
            com::sun::star::ucb::XContentProviderManager > const &
        rManager,
    rtl::OUString const & rBaseURL,
    rtl::OUString const & rSystemPath)
    SAL_THROW((com::sun::star::uno::RuntimeException));

//============================================================================
/** Using a specific content provider manager, convert a (file) URL to a
    file path in system dependent notation.

    @param rManager
    A content provider manager.  Must not be null.

    @param rURL
    See the corresponding parameter of
    com::sun::star::ucb::XFileIdentiferConverter::getSystemPathFromFileURL().

    @returns
    a system path, if the content provider registered at the content provider
    manager that is responsible for the base URL returns a system path when
    calling
    com::sun::star::ucb::XFileIdentiferConverter::getSystemPathFromFileURL()
    on it.  Otherwise, an empty string is returned.

    @see
    com::sun::star::ucb::XFileIdentiferConverter::getSystemPathFromFileURL().
 */
UCBHELPER_DLLPUBLIC rtl::OUString
getSystemPathFromFileURL(
    com::sun::star::uno::Reference<
            com::sun::star::ucb::XContentProviderManager > const &
        rManager,
    rtl::OUString const & rURL)
    SAL_THROW((com::sun::star::uno::RuntimeException));

}

#endif // _UCBHELPER_FILEIDENTIFIERCONVERTER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
