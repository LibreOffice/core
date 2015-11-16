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

#include <scanner.hxx>

#include <cppuhelper/supportsservice.hxx>

// - ScannerManager -


Reference< XInterface > SAL_CALL ScannerManager_CreateInstance( const Reference< css::lang::XMultiServiceFactory >& /*rxFactory*/ ) throw ( Exception )
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

Sequence< sal_Int8 > SAL_CALL ScannerManager::getMaskDIB() throw(std::exception)
{
    return Sequence< sal_Int8 >();
}

OUString ScannerManager::getImplementationName()
    throw (css::uno::RuntimeException, std::exception)
{
    return getImplementationName_Static();
}

sal_Bool ScannerManager::supportsService(OUString const & ServiceName)
    throw (css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> ScannerManager::getSupportedServiceNames()
    throw (css::uno::RuntimeException, std::exception)
{
    return getSupportedServiceNames_Static();
}

OUString ScannerManager::getImplementationName_Static() throw()
{
    return OUString( "com.sun.star.scanner.ScannerManager" );
}



Sequence< OUString > ScannerManager::getSupportedServiceNames_Static() throw ()
{
    Sequence< OUString > aSNS { "com.sun.star.scanner.ScannerManager" };
    return aSNS;
}

sal_Bool SAL_CALL ScannerManager::configureScanner( ScannerContext& rContext )
    throw( ScannerException, std::exception )
{
    return configureScannerAndScan( rContext, nullptr );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
