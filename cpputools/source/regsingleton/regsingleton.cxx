/*************************************************************************
 *
 *  $RCSfile: regsingleton.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dbo $ $Date: 2002-09-10 09:39:15 $
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

#include <osl/diagnose.h>
#include <osl/file.h>

#include <cppuhelper/bootstrap.hxx>

#include <com/sun/star/registry/XSimpleRegistry.hpp>

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


//==================================================================================================
extern "C" int SAL_CALL main( int argc, char const * argv [] )
{
    if (argc < 3)
    {
        fprintf(
            stderr, "\nusage: regsingleton registry_file singleton_name1=service_name1 ...\n" );
        return 1;
    }

    OUString sys_path( OUString::createFromAscii( argv[ 1 ] ) );
    OUString file_url;
    oslFileError rc = osl_getFileURLFromSystemPath( sys_path.pData, &file_url.pData );
    if (osl_File_E_None != rc)
    {
        fprintf( stderr, "\nerror: cannot make file url out of %s\n", argv[ 1 ] );
        return 1;
    }

    try
    {
        Reference< registry::XSimpleRegistry > xSimReg( ::cppu::createSimpleRegistry() );
        xSimReg->open( file_url, sal_False, sal_True );
        Reference< registry::XRegistryKey > xRoot( xSimReg->getRootKey() );
        Reference< registry::XRegistryKey > xKey( xRoot->openKey( OUSTR("SINGLETONS") ) );
        if (! xKey.is())
            xKey = xRoot->createKey( OUSTR("SINGLETONS") );

        for ( int nPos = 2; nPos < argc; ++nPos )
        {
            OUString singleton( OUString::createFromAscii( argv[ nPos ] ) );
            sal_Int32 eq = singleton.indexOf( '=' );
            if (eq < 0)
            {
                OString entry( OUStringToOString( singleton, RTL_TEXTENCODING_ASCII_US ) );
                fprintf( stderr, "skipping %s: no service name given!\n", entry.getStr() );
            }
            else
            {
                OUString service( singleton.copy( eq +1 ) );
                singleton = singleton.copy( 0, eq );

                Reference< registry::XRegistryKey > xEntry( xKey->openKey( singleton ) );
                if (! xEntry.is())
                    xEntry = xKey->createKey( singleton );
                xEntry->setStringValue( service );
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
