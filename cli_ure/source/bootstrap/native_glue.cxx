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

#ifndef _RTL_USTRING_HXX_
#include "rtl/ustring.hxx"
#endif
#ifndef _RTL_BOOTSTRAP_HXX_
#include "rtl/bootstrap.hxx"
#endif
#ifndef _CPPUHELPER_BOOTSTRAP_HXX_
#include "cppuhelper/bootstrap.hxx"
#endif
#ifndef _UNO_MAPPING_HXX_
#include "uno/mapping.hxx"
#endif
extern "C" {
#include "mono/metadata/object.h"
}

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )

using namespace com::sun::star::uno;

static void * to_cli( Reference< XInterface > const & context )
{
    Mapping mapping( OUSTR( CPPU_CURRENT_LANGUAGE_BINDING_NAME ), OUSTR( UNO_LB_CLI ) );
    OSL_ASSERT( mapping.is() );
    if (! mapping.is() )
    {
        // FIXME do something
        return 0;
    }

    return (void *)mono_gchandle_new( (MonoObject *)mapping.mapInterface( context.get(), ::getCppuType( &context ) ), sal_False );
}

extern "C" {

void SAL_CALL native_bootstrap_set( const sal_Unicode* pName, sal_Int32 nNameLength,
                                    const sal_Unicode* pValue, sal_Int32 nValueLength )
    SAL_THROW_EXTERN_C()
{
    ::rtl::OUString const name( pName, nNameLength );
    ::rtl::OUString const value( pValue, nValueLength );

    ::rtl::Bootstrap::set( name, value );
}

void * SAL_CALL native_defaultBootstrap_InitialComponentContext()
    SAL_THROW_EXTERN_C()
{
    try {
        Reference< XComponentContext > xContext = ::cppu::defaultBootstrap_InitialComponentContext();

        return to_cli( xContext );
    } catch (const Exception &ex) {
            fprintf( stderr, "Exception bootstrapping '%s'\n",
                     (const sal_Char *) rtl::OUStringToOString( ex.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
            return NULL;
    }
}

void * SAL_CALL native_defaultBootstrap_InitialComponentContext_iniFile( const sal_Unicode *pIniFile, sal_Int32 nNameLength )
    SAL_THROW_EXTERN_C()
{
    ::rtl::OUString sFileName( pIniFile, nNameLength );

    try {
            Reference< XComponentContext > xContext = ::cppu::defaultBootstrap_InitialComponentContext( sFileName );
            if (!xContext.is())
                    fprintf( stderr, "Bootstrap context is in fact NULL\n" );

            return to_cli( xContext );
    } catch (const Exception &ex) {
            rtl::OString aIni = rtl::OUStringToOString( sFileName, RTL_TEXTENCODING_UTF8 );
            fprintf( stderr, "Exception bootstrapping from ini '%s' (%d) '%s'\n", (const sal_Char *) aIni.getStr(), nNameLength,
                     (const sal_Char *) rtl::OUStringToOString( ex.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
            return NULL;
    }
}

void * SAL_CALL native_bootstrap()
    SAL_THROW_EXTERN_C()
{
    Reference< XComponentContext > xContext = ::cppu::bootstrap();

    return to_cli( xContext );
}

} // extern "C"
