/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <ucbhelper/fileidentifierconverter.hxx>
#include <com/sun/star/ucb/ContentProviderInfo.hpp>
#include <com/sun/star/ucb/XFileIdentifierConverter.hpp>
#include <com/sun/star/ucb/XUniversalContentBroker.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/diagnose.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>

using namespace com::sun::star;

namespace ucbhelper {


//

//


OUString
getLocalFileURL() SAL_THROW((uno::RuntimeException))
{
    
    
    
    
    return OUString("file:
}


//

//


OUString
getFileURLFromSystemPath(
    uno::Reference< ucb::XUniversalContentBroker > const & rUcb,
    OUString const & rBaseURL,
    OUString const & rSystemPath)
    SAL_THROW((uno::RuntimeException))
{
    OSL_ASSERT(rUcb.is());

    uno::Reference< ucb::XFileIdentifierConverter >
        xConverter(rUcb->queryContentProvider(rBaseURL), uno::UNO_QUERY);
    if (xConverter.is())
        return xConverter->getFileURLFromSystemPath(rBaseURL, rSystemPath);
    else
        return OUString();
}


//

//


OUString
getSystemPathFromFileURL(
    uno::Reference< ucb::XUniversalContentBroker > const & rUcb,
    OUString const & rURL)
    SAL_THROW((uno::RuntimeException))
{
    OSL_ASSERT(rUcb.is());

    uno::Reference< ucb::XFileIdentifierConverter >
        xConverter(rUcb->queryContentProvider(rURL), uno::UNO_QUERY);
    if (xConverter.is())
        return xConverter->getSystemPathFromFileURL(rURL);
    else
        return OUString();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
