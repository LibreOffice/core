/*************************************************************************
 *
 *  $RCSfile: svguno.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:53 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "svgcom.hxx"
#include "svgwriter.hxx"
#include "svgprinter.hxx"

#include <cppuhelper/factory.hxx>
#include <uno/mapping.hxx>

// -------------------
// - factory methods -
// -------------------

#if SUPD>=590
static REF( NMSP_UNO::XInterface ) SAL_CALL create_SVGWriter( const REF( NMSP_LANG::XMultiServiceFactory )& rxFact )
#else
static REF( NMSP_UNO::XInterface ) create_SVGWriter( const REF( NMSP_LANG::XMultiServiceFactory )& rxFact )
#endif
{
    return REF( NMSP_UNO::XInterface )( *new SVGWriter( rxFact ) );
}

// -----------------------------------------------------------------------------

#if SUPD>=590
static REF( NMSP_UNO::XInterface ) SAL_CALL create_SVGPrinter( const REF( NMSP_LANG::XMultiServiceFactory )& rxFact )
#else
static REF( NMSP_UNO::XInterface ) create_SVGPrinter( const REF( NMSP_LANG::XMultiServiceFactory )& rxFact )
#endif
{
    return REF( NMSP_UNO::XInterface )( *new SVGPrinter( rxFact ) );
}

// ------------------------------------------
// - component_getImplementationEnvironment -
// ------------------------------------------

extern "C" void SAL_CALL component_getImplementationEnvironment( const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

// -----------------------
// - component_writeInfo -
// -----------------------

extern "C" sal_Bool SAL_CALL component_writeInfo( void* pServiceManager, void* pRegistryKey )
{
    sal_Bool bRet = sal_False;

    if( pRegistryKey )
    {
        try
        {
            REF( NMSP_REGISTRY::XRegistryKey ) xNewKey1(
                static_cast< NMSP_REGISTRY::XRegistryKey* >( pRegistryKey )->createKey(
                B2UCONST( "/com.sun.star.comp.extensions.SVGWriter/UNO/SERVICES/com.sun.star.svg.SVGWriter" ) ) );
            REF( NMSP_REGISTRY::XRegistryKey ) xNewKey2(
                static_cast< NMSP_REGISTRY::XRegistryKey* >( pRegistryKey )->createKey(
                B2UCONST( "/com.sun.star.comp.extensions.SVGPrinter/UNO/SERVICES/com.sun.star.svg.SVGPrinter" ) ) );

            bRet = sal_True;
        }
        catch( NMSP_REGISTRY::InvalidRegistryException& )
        {
            OSL_ENSHURE( sal_False, "### InvalidRegistryException!" );
        }
    }

    return bRet;
}

// ------------------------
// - component_getFactory -
// ------------------------

extern "C" void* SAL_CALL component_getFactory( const sal_Char* pImplName, void* pServiceManager, void* pRegistryKey )
{
    REF( NMSP_LANG::XSingleServiceFactory ) xFactory;
    void*                                   pRet = 0;

    if( rtl_str_compare( pImplName, "com.sun.star.comp.extensions.SVGWriter" ) == 0 )
    {
        const NMSP_RTL::OUString aServiceName( B2UCONST( "com.sun.star.svg.SVGWriter" ) );

        xFactory = REF( NMSP_LANG::XSingleServiceFactory )( NMSP_CPPU::createSingleFactory(
                        reinterpret_cast< NMSP_LANG::XMultiServiceFactory* >( pServiceManager ),
                        B2UCONST( "com.sun.star.comp.extensions.SVGWriter" ),
                        create_SVGWriter, SEQ( NMSP_RTL::OUString )( &aServiceName, 1 ) ) );
    }
    else if( rtl_str_compare( pImplName, "com.sun.star.comp.extensions.SVGPrinter" ) == 0 )
    {
        const NMSP_RTL::OUString aServiceName( B2UCONST( "com.sun.star.svg.SVGPrinter" ) );

        xFactory = REF( NMSP_LANG::XSingleServiceFactory )( NMSP_CPPU::createSingleFactory(
                        reinterpret_cast< NMSP_LANG::XMultiServiceFactory* >( pServiceManager ),
                        B2UCONST( "com.sun.star.comp.extensions.SVGPrinter" ),
                        create_SVGPrinter, SEQ( NMSP_RTL::OUString )( &aServiceName, 1 ) ) );
    }

    if( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}
