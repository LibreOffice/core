/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scnserv.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 15:04:18 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#include <cppuhelper/factory.hxx>
#include <uno/mapping.hxx>
#include "scanner.hxx"

#include <com/sun/star/registry/XRegistryKey.hpp>

using namespace com::sun::star::registry;

// ------------------------------------------
// - component_getImplementationEnvironment -
// ------------------------------------------

extern "C" void SAL_CALL component_getImplementationEnvironment( const sal_Char** ppEnvTypeName, uno_Environment** /*ppEnv*/ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

// -----------------------
// - component_writeInfo -
// -----------------------

extern "C" sal_Bool SAL_CALL component_writeInfo( void* /*pServiceManager*/, void* pRegistryKey )
{
    sal_Bool bRet = sal_False;

    if( pRegistryKey )
    {
        try
        {
            ::rtl::OUString aImplName( '/' );

            aImplName += ScannerManager::getImplementationName_Static();
            aImplName += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES/" ) );
            aImplName += ScannerManager::getImplementationName_Static();

            REF( XRegistryKey ) xNewKey1( static_cast< XRegistryKey* >( pRegistryKey )->createKey( aImplName ) );

            bRet = sal_True;
        }
        catch( InvalidRegistryException& )
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }

    return bRet;
}

// ------------------------
// - component_getFactory -
// ------------------------

extern "C" void* SAL_CALL component_getFactory( const sal_Char* pImplName, void* pServiceManager, void* /*pRegistryKey*/ )
{
    REF( ::com::sun::star::lang::XSingleServiceFactory ) xFactory;
    void*                                                pRet = 0;

    if( ::rtl::OUString::createFromAscii( pImplName ) == ScannerManager::getImplementationName_Static() )
    {
        xFactory = REF( ::com::sun::star::lang::XSingleServiceFactory )( ::cppu::createSingleFactory(
                        static_cast< ::com::sun::star::lang::XMultiServiceFactory* >( pServiceManager ),
                        ScannerManager::getImplementationName_Static(),
                        ScannerManager_CreateInstance,
                        ScannerManager::getSupportedServiceNames_Static() ) );
    }

    if( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}
