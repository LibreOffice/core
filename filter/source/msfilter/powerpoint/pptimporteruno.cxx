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
#include "precompiled_filter.hxx"

#include "pptimporter.hxx"

#include <cppuhelper/factory.hxx>
#include <uno/mapping.hxx>

// -------------------
// - factory methods -
// -------------------

static REF( NMSP_UNO::XInterface ) SAL_CALL create_PptImporter( const REF( NMSP_LANG::XMultiServiceFactory )& rxFact )
{
    return REF( NMSP_UNO::XInterface )( *new PptImporter( rxFact ) );
}

// ------------------------------------------
// - component_getImplementationEnvironment -
// ------------------------------------------

extern "C" void SAL_CALL component_getImplementationEnvironment( const sal_Char ** ppEnvTypeName, uno_Environment ** /* ppEnv */ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

// -----------------------
// - component_writeInfo -
// -----------------------

extern "C" sal_Bool SAL_CALL component_writeInfo( void* /* pServiceManager */, void* pRegistryKey )
{
    sal_Bool bRet = sal_False;

    if( pRegistryKey )
    {
        try
        {
            NMSP_UNO::Reference< NMSP_REGISTRY::XRegistryKey > xNewKey(
                reinterpret_cast< NMSP_REGISTRY::XRegistryKey * >( pRegistryKey )->createKey(
                    PptImporter_getImplementationName() ) ); 
            xNewKey = xNewKey->createKey( B2UCONST( "/UNO/SERVICES" ) );
            const SEQ( NMSP_RTL::OUString )& rSNL = PptImporter_getSupportedServiceNames();
            const NMSP_RTL::OUString * pArray = rSNL.getConstArray();
            for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );
            bRet = sal_True;
        }
        catch( NMSP_REGISTRY::InvalidRegistryException& )
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }

    return bRet;
}

// ------------------------
// - component_getFactory -
// ------------------------

extern "C" void* SAL_CALL component_getFactory( const sal_Char* pImplName, void* pServiceManager, void* /* pRegistryKey */ )
{
    REF( NMSP_LANG::XSingleServiceFactory ) xFactory;
    void*									pRet = 0;
    
    if( rtl_str_compare( pImplName, "com.sun.star.presentation.PptImporter" ) == 0 )
    {
        const NMSP_RTL::OUString aServiceName( B2UCONST( "com.sun.star.presentation.PptImporter" ) );

        xFactory = REF( NMSP_LANG::XSingleServiceFactory )( NMSP_CPPU::createSingleFactory(
            reinterpret_cast< NMSP_LANG::XMultiServiceFactory* >( pServiceManager ),
                B2UCONST( "com.sun.star.presentation.PptImporter" ),
                        create_PptImporter, SEQ( NMSP_RTL::OUString )( &aServiceName, 1 ) ) );
    }
    if( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }
    
    return pRet;
}
