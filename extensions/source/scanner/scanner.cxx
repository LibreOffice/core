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

// ------------------
// - ScannerManager -
// ------------------

REF( XInterface ) SAL_CALL ScannerManager_CreateInstance( const REF( com::sun::star::lang::XMultiServiceFactory )& /*rxFactory*/ ) throw ( Exception )
{
    return *( new ScannerManager() );
}

// -----------------------------------------------------------------------------

ScannerManager::ScannerManager() :
    mpData( NULL )
{
    AcquireData();
}

// -----------------------------------------------------------------------------

ScannerManager::~ScannerManager()
{
    ReleaseData();
}

// -----------------------------------------------------------------------------

ANY SAL_CALL ScannerManager::queryInterface( const Type& rType ) throw( RuntimeException )
{
    const ANY aRet( cppu::queryInterface( rType,
                                          static_cast< XScannerManager2* >( this ),
                                          static_cast< AWT::XBitmap* >( this ) ) );

    return( aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ) );
}

// -----------------------------------------------------------------------------

void SAL_CALL ScannerManager::acquire() throw()
{
    OWeakObject::acquire();
}

// -----------------------------------------------------------------------------

void SAL_CALL ScannerManager::release() throw()
{
    OWeakObject::release();
}

// -----------------------------------------------------------------------------

SEQ( sal_Int8 ) SAL_CALL ScannerManager::getMaskDIB() throw()
{
    return SEQ( sal_Int8 )();
}

// -----------------------------------------------------------------------------

OUString ScannerManager::getImplementationName_Static() throw()
{
    return ::rtl::OUString( "com.sun.star.scanner.ScannerManager" );
}

// -----------------------------------------------------------------------------

SEQ( OUString ) ScannerManager::getSupportedServiceNames_Static() throw ()
{
    SEQ( OUString ) aSNS( 1 );

    aSNS.getArray()[0] = ::rtl::OUString( "com.sun.star.scanner.ScannerManager" );

    return aSNS;
}

sal_Bool SAL_CALL ScannerManager::configureScanner( ScannerContext& rContext )
    throw( ScannerException )
{
    return configureScannerAndScan( rContext, NULL );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
