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
getLocalFileURL() SAL_THROW((uno::RuntimeException))
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
