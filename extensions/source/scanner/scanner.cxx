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
