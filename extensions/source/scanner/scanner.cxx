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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"
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
                                          static_cast< XScannerManager* >( this ),
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
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.scanner.ScannerManager" ) );
}

// -----------------------------------------------------------------------------

SEQ( OUString ) ScannerManager::getSupportedServiceNames_Static() throw ()
{
    SEQ( OUString ) aSNS( 1 );

    aSNS.getArray()[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.scanner.ScannerManager" ) );

    return aSNS;
}
