/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "svgcom.hxx"
#include "svgwriter.hxx"
#include "svgprinter.hxx"

#include <cppuhelper/factory.hxx>
#include <uno/mapping.hxx>

// -------------------
// - factory methods -
// -------------------

static REF( NMSP_UNO::XInterface ) SAL_CALL create_SVGWriter( const REF( NMSP_LANG::XMultiServiceFactory )& rxFact )
{
    return REF( NMSP_UNO::XInterface )( *new SVGWriter( rxFact ) );
}

// -----------------------------------------------------------------------------

static REF( NMSP_UNO::XInterface ) SAL_CALL create_SVGPrinter( const REF( NMSP_LANG::XMultiServiceFactory )& rxFact )
{
    return REF( NMSP_UNO::XInterface )( *new SVGPrinter( rxFact ) );
}

// ------------------------------------------
// - component_getImplementationEnvironment -
// ------------------------------------------

extern "C" void SAL_CALL component_getImplementationEnvironment( const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

// ------------------------
// - component_getFactory -
// ------------------------

extern "C" void* SAL_CALL component_getFactory( const sal_Char* pImplName, void* pServiceManager, void* /*pRegistryKey*/ )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
