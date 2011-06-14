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

#include <stdio.h>

#include "sal/main.h"
#include <osl/diagnose.h>
#include <osl/file.h>

#include <cppuhelper/bootstrap.hxx>

#include <com/sun/star/registry/XSimpleRegistry.hpp>

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

static void print_options() SAL_THROW( () )
{
    printf(
        "\nusage: regsingleton [-r|-ra] registry_file singleton_name[=service_name] ...\n\n"
        "Inserts a singleton entry into rdb.\n"
        "Option -r revokes given entries, -ra revokes all entries.\n" );
}

//==================================================================================================
SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    if (argc < 3)
    {
        print_options();
        return 1;
    }

    bool insert_entry = true;
    bool remove_all = false;
    int nPos = 1;
    if ('-' == argv[ nPos ][ 0 ] && 'r' == argv[ nPos ][ 1 ])
    {
        if ('a' == argv[ nPos ][ 2 ] && '\0' == argv[ nPos ][ 3 ])
        {
            remove_all = true;
        }
        else if ('\0' != argv[ nPos ][ 2 ])
        {
            print_options();
            return 1;
        }
        insert_entry = false;
        ++nPos;
    }

    OUString sys_path( OUString::createFromAscii( argv[ nPos ] ) );
    OUString file_url;
    oslFileError rc = osl_getFileURLFromSystemPath( sys_path.pData, &file_url.pData );
    if (osl_File_E_None != rc)
    {
        fprintf( stderr, "\nerror: cannot make file url out of %s\n", argv[ nPos ] );
        return 1;
    }
    ++nPos;

    try
    {
        Reference< registry::XSimpleRegistry > xSimReg( ::cppu::createSimpleRegistry() );
        xSimReg->open( file_url, sal_False, sal_True );
        Reference< registry::XRegistryKey > xRoot( xSimReg->getRootKey() );

        if (remove_all)
        {
            try
            {
                xRoot->deleteKey( OUSTR("SINGLETONS") );
            }
            catch (registry::InvalidRegistryException & exc)
            {
                OString cstr_msg(
                    OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
                fprintf(
                    stderr, "\nwarning: removing all singletons failed: %s\n",
                    cstr_msg.getStr() );
            }
        }
        else
        {
            Reference< registry::XRegistryKey > xKey( xRoot->openKey( OUSTR("SINGLETONS") ) );
            if (! xKey.is())
                xKey = xRoot->createKey( OUSTR("SINGLETONS") );

            for ( ; nPos < argc; ++nPos )
            {
                OUString singleton( OUString::createFromAscii( argv[ nPos ] ) );
                OUString service;
                sal_Int32 eq = singleton.indexOf( '=' );
                if (eq >= 0)
                {
                    service = singleton.copy( eq +1 );
                    singleton = singleton.copy( 0, eq );
                }

                if (insert_entry)
                {
                    if (service.getLength())
                    {
                        Reference< registry::XRegistryKey > xEntry( xKey->openKey( singleton ) );
                        if (! xEntry.is())
                            xEntry = xKey->createKey( singleton );
                        xEntry->setStringValue( service );
                    }
                    else
                    {
                        OString entry( OUStringToOString( singleton, RTL_TEXTENCODING_ASCII_US ) );
                        fprintf(
                            stderr, "\nwarning: no service name given for singleton %s!\n",
                            entry.getStr() );
                    }
                }
                else
                {
                    try
                    {
                        xKey->deleteKey( singleton );
                    }
                    catch (registry::InvalidRegistryException & exc)
                    {
                        OString cstr_singleton(
                            OUStringToOString( singleton, RTL_TEXTENCODING_ASCII_US ) );
                        OString cstr_msg(
                            OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
                        fprintf(
                            stderr, "\nwarning: singleton %s is not registered: %s\n",
                            cstr_singleton.getStr(), cstr_msg.getStr() );
                    }
                }
            }
        }

        return 0;
    }
    catch (Exception & rExc)
    {
        OString msg( OUStringToOString( rExc.Message, RTL_TEXTENCODING_ASCII_US ) );
        fprintf( stderr, "\nerror: %s\n", msg.getStr() );
        return 1;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
