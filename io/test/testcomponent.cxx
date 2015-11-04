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



// testcomponent - Loads a service and its testcomponent from dlls performs a test.
// Expands the dll-names depending on the actual environment.
// Example : testcomponent com.sun.star.io.Pipe stm
//
// Therefore the testcode must exist in teststm and the testservice must be named test.com.sun.star.io.Pipe

#include <stdio.h>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <com/sun/star/test/XSimpleTest.hpp>

#include <cppuhelper/servicefactory.hxx>

using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::test;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

// Needed to switch on solaris threads

int main (int argc, char **argv)
{

    if( argc < 3) {
        printf( "usage : testcomponent service dll [additional dlls]\n" );
        exit( 0 );
    }

    // create service manager
    Reference< XMultiServiceFactory > xSMgr = createRegistryServiceFactory(
        OUString( "applicat.rdb"  ) );

    Reference < XImplementationRegistration > xReg;
    Reference < XSimpleRegistry > xSimpleReg;

    try
    {
        // Create registration service
        Reference < XInterface > x = xSMgr->createInstance(
            "com.sun.star.registry.ImplementationRegistration" );
        xReg.set( x , UNO_QUERY );
    }
    catch( Exception & ) {
        printf( "Couldn't create ImplementationRegistration service\n" );
        exit(1);
    }

    sal_Char szBuf[1024];
    OString sTestName;

    try
    {
        // Load dll for the tested component
        for( int n = 2 ; n <argc ; n ++ ) {
            OUString aDllName = OStringToOUString( argv[n] , RTL_TEXTENCODING_ASCII_US );
            xReg->registerImplementation(
                OUString("com.sun.star.loader.SharedLibrary"),
                aDllName,
                xSimpleReg );
        }
    }
    catch( const Exception &e ) {
        printf( "%s\n" , OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US ).getStr() );

        exit(1);
    }


    try
    {
        // Load dll for the test component
        sTestName = "test";
        sTestName += argv[2];

#if defined(SAL_W32)
        OUString aDllName = OStringToOUString( sTestName , RTL_TEXTENCODING_ASCII_US );
#else
        OUString aDllName("lib");
        aDllName += OStringToOUString( sTestName , RTL_TEXTENCODING_ASCII_US );
        aDllName += ".so";
#endif

        xReg->registerImplementation(
            OUString("com.sun.star.loader.SharedLibrary") ,
            aDllName,
            xSimpleReg );
    }
    catch( Exception & )
    {
        printf( "Couldn't reach dll %s\n" , szBuf );
        exit(1);
    }


    // Instantiate test service
    sTestName = "test.";
    sTestName += argv[1];

    Reference < XInterface > xIntTest =
        xSMgr->createInstance( OStringToOUString( sTestName , RTL_TEXTENCODING_ASCII_US ) );
    Reference< XSimpleTest > xTest( xIntTest , UNO_QUERY );

    if( ! xTest.is() ) {
        printf( "Couldn't instantiate test service \n" );
        exit( 1 );
    }


    sal_Int32 nHandle = 0;
    sal_Int32 nNewHandle;
    sal_Int32 nErrorCount = 0;
    sal_Int32 nWarningCount = 0;

    // loop until all test are performed
    while( nHandle != -1 )
    {
        // Instantiate serivce
        Reference< XInterface > x =
            xSMgr->createInstance( OStringToOUString( argv[1] , RTL_TEXTENCODING_ASCII_US ) );
        if( ! x.is() )
        {
            printf( "Couldn't instantiate service !\n" );
            exit( 1 );
        }

        // do the test
        try
        {
            nNewHandle = xTest->test(
                OStringToOUString( argv[1] , RTL_TEXTENCODING_ASCII_US ) , x , nHandle );
        }
        catch( const Exception & e ) {
            OString o  = OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US );
            printf( "testcomponent : uncaught exception %s\n" , o.getStr() );
            exit(1);
        }
        catch( ... )
        {
            printf( "testcomponent : uncaught unknown exception\n"  );
            exit(1);
        }


        // print errors and warning
        Sequence<OUString> seqErrors = xTest->getErrors();
        Sequence<OUString> seqWarnings = xTest->getWarnings();
        if( seqWarnings.getLength() > nWarningCount )
        {
            printf( "Warnings during test %d!\n" , nHandle );
            for( ; nWarningCount < seqWarnings.getLength() ; nWarningCount ++ )
            {
                OString o = OUStringToOString(
                    seqWarnings.getArray()[nWarningCount], RTL_TEXTENCODING_ASCII_US );
                printf( "Warning\n%s\n---------\n" , o.getStr() );
            }
        }


        if( seqErrors.getLength() > nErrorCount ) {
            printf( "Errors during test %d!\n" , nHandle );
            for( ; nErrorCount < seqErrors.getLength() ; nErrorCount ++ )
            {
                OString o = OUStringToOString(
                    seqErrors.getArray()[nErrorCount], RTL_TEXTENCODING_ASCII_US );
                printf( "%s\n" , o.getStr() );
            }
        }

        nHandle = nNewHandle;
    }

    if( xTest->testPassed() ) {
        printf( "Test passed !\n" );
    }
    else {
        printf( "Test failed !\n" );
    }

    Reference <XComponent >  rComp( xSMgr , UNO_QUERY );
    rComp->dispose();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
