/*************************************************************************
 *
 *  $RCSfile: regsingleton.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-10-28 16:22:25 $
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
