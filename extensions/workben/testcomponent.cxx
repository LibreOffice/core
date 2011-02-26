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

//------------------------------------------------------
// testcomponent - Loads a service and its testcomponent from dlls performs a test.
// Expands the dll-names depending on the actual environment.
// Example : testcomponent stardiv.uno.io.Pipe stm
//
// Therefor the testcode must exist in teststm and the testservice must be named test.stardiv.uno.io.Pipe
//

#include <stdio.h>
#include <smart/com/sun/star/registry/XImplementationRegistration.hxx>
#include <smart/com/sun/star/lang/XComponent.hxx>

#include <osl/diagnose.h>
#include <usr/services.hxx>
#include <vcl/svapp.hxx>
#include <usr/ustring.hxx>
#include <tools/string.hxx>
#include <osl/conditn.hxx>

#include <smart/com/sun/star/test/XSimpleTest.hxx>

using namespace usr;

using ::rtl::OString;
using ::rtl::OWStringToOString;
using ::rtl::OStringToOWString;


// Needed to switch on solaris threads
#ifdef SOLARIS
extern "C" void ChangeGlobalInit();
#endif

int __LOADONCALLAPI main (int argc, char **argv)
{

    if( argc < 3) {
        printf( "usage : testcomponent service dll [additional dlls]\n" );
        exit( 0 );
    }
#ifdef SOLARIS
    // switch on threads in solaris
    ChangeGlobalInit();
#endif

    // create service manager
//  XMultiServiceFactoryRef xSMgr = getProcessServiceManager();
    XMultiServiceFactoryRef xSMgr = createRegistryServiceManager();
    OSL_ASSERT( xSMgr.is() );
    registerUsrServices( xSMgr );
    setProcessServiceManager( xSMgr );

    XImplementationRegistrationRef xReg;
    XSimpleRegistryRef xSimpleReg;

    try {
        // Create registration service
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
        // Load dll for the tested component
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
        // Load dll for the test component
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


    // Instantiate test service
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

    // loop until all test are performed
    while( nHandle != -1 ) {

        // Instantiate serivce
        XInterfaceRef x = xSMgr->createInstance( OStringToOWString( argv[1] , CHARSET_SYSTEM ) );
        if( ! x.is() ) {
            printf( "Couldn't instantiate service !\n" );
            exit( 1 );
        }

        // do the test
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


        // print errors and warning
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
