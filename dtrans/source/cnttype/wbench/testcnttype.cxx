/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <cppuhelper/servicefactory.hxx>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/datatransfer/XMimeContentType.hpp>
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <osl/diagnose.h>

#include <stdio.h>

#include <vector>

// my defines

#define TEST_CLIPBOARD
#define RDB_SYSPATH  "d:\\projects\\src621\\dtrans\\wntmsci7\\bin\\applicat.rdb"

//  namesapces

using namespace ::std;
using namespace ::cppu;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;

void ShutdownServiceMgr( Reference< XMultiServiceFactory >& SrvMgr )
{
    // Cast factory to XComponent
    Reference< XComponent > xComponent( SrvMgr, UNO_QUERY );

    if ( !xComponent.is() )
        OSL_FAIL("Error shuting down");

    // Dispose and clear factory
    xComponent->dispose();
    SrvMgr.clear();
}

sal_Bool readCntTypesFromFileIntoVector( char* fname, vector< string >& vecData )
{
    FILE* fstream;

    fstream = fopen( fname, "r+" );
    if ( !fstream )
        return sal_False;

    // set pointer to file start
    fseek( fstream, 0L, SEEK_SET );

    char line[1024];
    while ( fscanf( fstream, "%[^\n]s", line ) != EOF )
    {
        vecData.push_back( line );
        fgetc( fstream );
    }

    fclose( fstream );

    return sal_True;
}

sal_Bool processCntTypesAndWriteResultIntoFile( char* fname, vector< string >& vecData, Reference< XMimeContentTypeFactory > cnttypeFactory )
{
    FILE* fstream;

    fstream = fopen( fname, "w" );
    if ( !fstream )
        return sal_False;

    // set pointer to file start
    fseek( fstream, 0L, SEEK_SET );

    vector< string >::iterator iter_end = vecData.end( );
    for ( vector< string >::iterator iter = vecData.begin( ); iter != iter_end; ++iter )
    {
        try
        {
            fprintf( fstream, "Read: %s\n", iter->c_str( ) );

            Reference< XMimeContentType > xMCntTyp = cnttypeFactory->createMimeContentType( OUString::createFromAscii( iter->c_str( ) ) );

            fwprintf( fstream, OUString("Type: %s\n"),  xMCntTyp->getMediaType( ).getStr( ) );
            fwprintf( fstream, OUString("Subtype: %s\n"), xMCntTyp->getMediaSubtype( ).getStr( ) );

            Sequence< OUString > seqParam = xMCntTyp->getParameters( );
            sal_Int32 nParams = seqParam.getLength( );

            for ( sal_Int32 i = 0; i < nParams; i++ )
            {
                fwprintf( fstream, OUString("PName: %s\n"), seqParam[i].getStr( ) );
                fwprintf( fstream, OUString("PValue: %s\n"), xMCntTyp->getParameterValue( seqParam[i] ).getStr( ) );
            }
        }
        catch( IllegalArgumentException& ex )
        {
            fwprintf( fstream, OUString("Read incorrect content type!\n\n") );
        }
        catch( NoSuchElementException& )
        {
            fwprintf( fstream, OUString("Value of parameter not available\n") );
        }
        catch( ... )
        {
            fwprintf( fstream, OUString("Unknown error!\n\n") );
        }

        fwprintf( fstream, OUString("\n#############################################\n\n") );
    }

    fclose( fstream );

    return sal_True;
}

//  main

int SAL_CALL main( int nArgc, char* argv[] )
{
    if ( nArgc != 3 )
        printf( "Start with: testcnttype input-file output-file\n" );

    // get the global service-manager

    Reference< XMultiServiceFactory > g_xFactory( createRegistryServiceFactory( RDB_SYSPATH ) );

    // Print a message if an error occurred.
    if ( !g_xFactory.is( ) )
    {
        OSL_FAIL("Can't create RegistryServiceFactory");
        return(-1);
    }

    vector< string > vecCntTypes;

    // open input-file and read the data
    if ( !readCntTypesFromFileIntoVector( argv[1], vecCntTypes ) )
    {
        printf( "Can't open input file" );
        ShutdownServiceMgr( g_xFactory );
    }

    Reference< XMimeContentTypeFactory >
        xMCntTypeFactory( g_xFactory->createInstance("com.sun.star.datatransfer.MimeContentTypeFactory"), UNO_QUERY );

    if ( !xMCntTypeFactory.is( ) )
    {
        OSL_FAIL( "Error creating MimeContentTypeFactory Service" );
        return(-1);
    }

    if ( !processCntTypesAndWriteResultIntoFile( argv[2], vecCntTypes, xMCntTypeFactory ) )
    {
        printf( "Can't open output file" );
        ShutdownServiceMgr( g_xFactory );
    }

    // shutdown the service manager

    ShutdownServiceMgr( g_xFactory );

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
