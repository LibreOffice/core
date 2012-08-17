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

#include <ZipPackageFolderEnumeration.hxx>
#include <ContentInfo.hxx>

using namespace com::sun::star;
using rtl::OUString;

ZipPackageFolderEnumeration::ZipPackageFolderEnumeration ( ContentHash &rInput)
: rContents (rInput)
, aIterator (rContents.begin())
{
}

ZipPackageFolderEnumeration::~ZipPackageFolderEnumeration( void )
{
}

sal_Bool SAL_CALL ZipPackageFolderEnumeration::hasMoreElements(  )
        throw(uno::RuntimeException)
{
    return (aIterator != rContents.end() );
}
uno::Any SAL_CALL ZipPackageFolderEnumeration::nextElement(  )
        throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any aAny;
    if (aIterator == rContents.end() )
        throw container::NoSuchElementException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    aAny <<= (*aIterator).second->xTunnel;
    aIterator++;
    return aAny;
}

OUString ZipPackageFolderEnumeration::getImplementationName()
    throw (uno::RuntimeException)
{
    return OUString ("ZipPackageFolderEnumeration");
}

uno::Sequence< OUString > ZipPackageFolderEnumeration::getSupportedServiceNames()
    throw (uno::RuntimeException)
{
    uno::Sequence< OUString > aNames(1);
    aNames[0] = "com.sun.star.packages.PackageFolderEnumeration";
    return aNames;
}
sal_Bool SAL_CALL ZipPackageFolderEnumeration::supportsService( OUString const & rServiceName )
    throw (uno::RuntimeException)
{
    return rServiceName == getSupportedServiceNames()[0];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
