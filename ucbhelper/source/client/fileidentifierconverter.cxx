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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucbhelper.hxx"
#include <ucbhelper/fileidentifierconverter.hxx>
#include <com/sun/star/ucb/ContentProviderInfo.hpp>
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#include <com/sun/star/ucb/XFileIdentifierConverter.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/diagnose.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>

using namespace com::sun::star;

namespace ucbhelper {

//============================================================================
//
//  getLocalFileURL
//
//============================================================================

rtl::OUString
getLocalFileURL(
    uno::Reference< ucb::XContentProviderManager > const &)
    SAL_THROW((uno::RuntimeException))
{
    // If there were more file systems than just "file:///" (e.g., the obsolete
    // "vnd.sun.star.wfs:///"), this code should query all relevant UCPs for
    // their com.sun.star.ucb.XFileIdentifierConverter.getFileProviderLocality
    // and return the most local one:
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("file:///"));
}

//============================================================================
//
//  getFileURLFromSystemPath
//
//============================================================================

rtl::OUString
getFileURLFromSystemPath(
    uno::Reference< ucb::XContentProviderManager > const & rManager,
    rtl::OUString const & rBaseURL,
    rtl::OUString const & rSystemPath)
    SAL_THROW((uno::RuntimeException))
{
    OSL_ASSERT(rManager.is());

    uno::Reference< ucb::XFileIdentifierConverter >
        xConverter(rManager->queryContentProvider(rBaseURL), uno::UNO_QUERY);
    if (xConverter.is())
        return xConverter->getFileURLFromSystemPath(rBaseURL, rSystemPath);
    else
        return rtl::OUString();
}

//============================================================================
//
//  getSystemPathFromFileURL
//
//============================================================================

rtl::OUString
getSystemPathFromFileURL(
    uno::Reference< ucb::XContentProviderManager > const & rManager,
    rtl::OUString const & rURL)
    SAL_THROW((uno::RuntimeException))
{
    OSL_ASSERT(rManager.is());

    uno::Reference< ucb::XFileIdentifierConverter >
        xConverter(rManager->queryContentProvider(rURL), uno::UNO_QUERY);
    if (xConverter.is())
        return xConverter->getSystemPathFromFileURL(rURL);
    else
        return rtl::OUString();
}

}
