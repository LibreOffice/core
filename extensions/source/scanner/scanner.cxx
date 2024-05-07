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

#include "scanner.hxx"

#include <cppuhelper/supportsservice.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

Reference< XInterface > ScannerManager_CreateInstance( const Reference< css::lang::XMultiServiceFactory >& /*rxFactory*/ )
{
    return *( new ScannerManager() );
}


ScannerManager::ScannerManager() :
    mpData( nullptr )
{
    AcquireData();
}


ScannerManager::~ScannerManager()
{
    ReleaseData();
}


Sequence< sal_Int8 > SAL_CALL ScannerManager::getMaskDIB()
{
    return Sequence< sal_Int8 >();
}


OUString ScannerManager::getImplementationName()
{
    return u"com.sun.star.scanner.ScannerManager"_ustr;
}


sal_Bool ScannerManager::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}


css::uno::Sequence<OUString> ScannerManager::getSupportedServiceNames()
{
    return { u"com.sun.star.scanner.ScannerManager"_ustr };
}


sal_Bool SAL_CALL ScannerManager::configureScanner( ScannerContext& rContext )
{
    return configureScannerAndScan( rContext, nullptr );
}

void SAL_CALL ScannerManager::initialize(const css::uno::Sequence<css::uno::Any>& rArguments)
{
    ::comphelper::NamedValueCollection aProperties(rArguments);
    if (aProperties.has(u"ParentWindow"_ustr))
        aProperties.get(u"ParentWindow"_ustr) >>= mxDialogParent;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
extensions_ScannerManager_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ScannerManager());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
