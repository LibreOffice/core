/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */






//

//

#include <stdio.h>
#include <smart/com/sun/star/registry/XImplementationRegistration.hxx>
#include <smart/com/sun/star/lang/XComponent.hxx>

#include <osl/diagnose.h>
#include <usr/services.hxx>
#include <vcl/svapp.hxx>
#include <usr/ustring.hxx>
#include <osl/conditn.hxx>

#include <smart/com/sun/star/test/XSimpleTest.hxx>

using namespace usr;

using ::rtl::OWStringToOString;



#ifdef SOLARIS
extern "C" void ChangeGlobalInit();
#endif

int SAL_CALL main (int argc, char **argv)
{

    if( argc < 3) {
        printf( "usage : testcomponent service dll [additional dlls]\n" );
        exit( 0 );
    }
#ifdef SOLARIS
    
    ChangeGlobalInit();
#endif

    

    XMultiServiceFactoryRef xSMgr = createRegistryServiceManager();
    OSL_ASSERT( xSMgr.is() );
    registerUsrServices( xSMgr );
    setProcessServiceManager( xSMgr );

    XImplementationRegistrationRef xReg;
    XSimpleRegistryRef xSimpleReg;

    try {
        
        XInterfaceRef x = xSMgr->createInstance(
            UString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.registry.ImplementationRegistration" )) );
        x->queryInterface( XImplementationRegistration::getSmartUik() , xReg );

    }
    catch( Exception& e ) {
        printf( "%s\n" , OWStringToOString( e.getName() , CHARSET_SYSTEM ).getStr() );
        exit(1);
    }

    sal_Char szBuf[1024];
    OString sTestName;

    try {
        
        for( int n = 2 ; n <argc ; n ++ ) {
            ORealDynamicLoader::computeModuleName( argv[n] , szBuf, 1024 );
            UString aDllName( OStringToOWString( szBuf, CHARSET_SYSTEM ) );

            xReg->registerImplementation(
                UString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.loader.SharedLibrary" )),
                aDllName,
                xSimpleReg );
        }
    }
    catch( Exception& e ) {
        printf( "Couldn't reach dll %s\n" , szBuf );
        printf( "%s\n" , OWStringToOString( e.getName() , CHARSET_SYSTEM ).getStr() );

        exit(1);
    }


    try {
        
        sTestName = "test";
        sTestName += argv[2];

        ORealDynamicLoader::computeModuleName( sTestName.getStr() , szBuf, 1024 );
        UString aDllName = OStringToOWString( szBuf, CHARSET_SYSTEM );
        xReg->registerImplementation(
            UString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.loader.SharedLibrary" )) ,
            aDllName,
            xSimpleReg );
    }
    catch( Exception& e ) {
        printf( "Couldn't reach dll %s\n" , szBuf );
        printf( "%s\n" , OWStringToOString( e.getName() , CHARSET_SYSTEM ).getStr() );
        exit(1);
    }


    
    sTestName = "test.";
    sTestName += argv[1];

    XInterfaceRef xIntTest = xSMgr->createInstance( OStringToOWString( sTestName , CHARSET_SYSTEM ) );
    XSimpleTestRef xTest( xIntTest , USR_QUERY );

    if( ! xTest.is() ) {
        printf( "Couldn't instantiate test service \n" );
        exit( 1 );
    }


    INT32 nHandle = 0;
    INT32 nNewHandle;
    INT32 nErrorCount = 0;
    INT32 nWarningCount = 0;

    
    while( nHandle != -1 ) {

        
        XInterfaceRef x = xSMgr->createInstance( OStringToOWString( argv[1] , CHARSET_SYSTEM ) );
        if( ! x.is() ) {
            printf( "Couldn't instantiate service !\n" );
            exit( 1 );
        }

        
        try {
            nNewHandle = xTest->test( OStringToOWString( argv[1] , CHARSET_SYSTEM ) , x , nHandle );
        }
        catch ( Exception& e ) {
            printf( "testcomponent : uncaught exception %s\n" ,
                        OWStringToOString( e.getName(), CHARSET_SYSTEM ).getStr() );
            exit(1);
        }
        catch(...) {
            printf( "testcomponent : uncaught unknown exception\n"  );
            exit(1);
        }


        
        Sequence<UString> seqErrors = xTest->getErrors();
        Sequence<UString> seqWarnings = xTest->getWarnings();
        if( seqWarnings.getLen() > nWarningCount ) {
            printf( "Warnings during test %d!\n" , nHandle );
            for( ; nWarningCount < seqWarnings.getLen() ; nWarningCount ++ ) {
                printf( "Warning\n%s\n---------\n" ,
                    OWStringToOString( seqWarnings.getArray()[nWarningCount], CHARSET_SYSTEM ).getStr() );
            }
        }


        if( seqErrors.getLen() > nErrorCount ) {
            printf( "Errors during test %d!\n" , nHandle );
            for( ; nErrorCount < seqErrors.getLen() ; nErrorCount ++ ) {
                printf( "%s\n" ,
                            OWStringToOString(
                                    seqErrors.getArray()[nErrorCount], CHARSET_SYSTEM ).getStr() );
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

    XComponentRef rComp( xSMgr , USR_QUERY );
    rComp->dispose();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
