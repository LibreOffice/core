/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scanner.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 15:03:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
}

// -----------------------------------------------------------------------------

ScannerManager::~ScannerManager()
{
    DestroyData();
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

OUString ScannerManager::getImplementationName() throw ()
{
    return getImplementationName_Static();
}

// -----------------------------------------------------------------------------

OUString ScannerManager::getImplementationName_Static() throw()
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.scanner.ScannerManager" ) );
}

// -----------------------------------------------------------------------------

SEQ( OUString ) ScannerManager::getSupportedServiceNames() throw ()
{
    return getSupportedServiceNames_Static();
}

// -----------------------------------------------------------------------------

SEQ( OUString ) ScannerManager::getSupportedServiceNames_Static() throw ()
{
    SEQ( OUString ) aSNS( 1 );

    aSNS.getArray()[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.scanner.ScannerManager" ) );

    return aSNS;
}

// -----------------------------------------------------------------------------

BOOL ScannerManager::supportsService( const OUString& ServiceName ) throw ()
{
    SEQ( OUString ) aSNL( getSupportedServiceNames() );
    const OUString* pArray = aSNL.getConstArray();

    for( INT32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return TRUE;

    return FALSE;
}
