/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
