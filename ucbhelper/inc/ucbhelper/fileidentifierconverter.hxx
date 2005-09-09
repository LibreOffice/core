/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fileidentifierconverter.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 16:28:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _UCBHELPER_FILEIDENTIFIERCONVERTER_HXX_
#define _UCBHELPER_FILEIDENTIFIERCONVERTER_HXX_

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef INCLUDED_UCBHELPERDLLAPI_H
#include "ucbhelper/ucbhelperdllapi.h"
#endif

namespace com { namespace sun { namespace star { namespace ucb {
    class XContentProviderManager;
} } } }
namespace rtl { class OUString; }

namespace ucb {

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
