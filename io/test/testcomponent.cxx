/*************************************************************************
 *
 *  $RCSfile: testcomponent.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:24:18 $
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

//------------------------------------------------------
// testcomponent - Loads a service and its testcomponent from dlls performs a test.
// Expands the dll-names depending on the actual environment.
// Example : testcomponent stardiv.uno.io.Pipe stm
//
// Therefor the testcode must exist in teststm and the testservice must be named test.stardiv.uno.io.Pipe
//

#include <stdio.h>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <com/sun/star/test/XSimpleTest.hpp>

#include <cppuhelper/servicefactory.hxx>

#include <vos/dynload.hxx>
#include <vos/diagnose.hxx>

#include <assert.h>


using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::test;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

// Needed to switch on solaris threads
#ifdef SOLARIS
extern "C" void ChangeGlobalInit();
#endif

int main (int argc, char **argv)
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
    Reference< XMultiServiceFactory > xSMgr = createRegistryServiceFactory(
        OUString( RTL_CONSTASCII_USTRINGPARAM( "applicat.rdb" ) ) );

    Reference < XImplementationRegistration > xReg;
    Reference < XSimpleRegistry > xSimpleReg;

    try
    {
        // Create registration service
        Reference < XInterface > x = xSMgr->createInstance(
            OUString::createFromAscii( "com.sun.star.registry.ImplementationRegistration" ) );
        xReg = Reference<  XImplementationRegistration > ( x , UNO_QUERY );
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
#ifdef SAL_W32
            OUString aDllName = OStringToOUString( argv[n] , RTL_TEXTENCODING_ASCII_US );
#else
            OUString aDllName = L"lib";
            aDllName += OStringToOUString( argv[n] , RTL_TEXTENCODING_ASCII_US );
            aDllName += L".so";
#endif
            xReg->registerImplementation(
                OUString::createFromAscii( "com.sun.star.loader.SharedLibrary" ),
                aDllName,
                xSimpleReg );
        }
    }
    catch( Exception &e ) {
        printf( "Couldn't reach dll %s\n" , szBuf );
        printf( "%s\n" , OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US ).getStr() );

        exit(1);
    }


    try
    {
        // Load dll for the test component
        sTestName = "test";
        sTestName += argv[2];

#ifdef SAL_W32
        OUString aDllName = OStringToOUString( sTestName , RTL_TEXTENCODING_ASCII_US );
#else
        OUString aDllName = L"lib";
        aDllName += OStringToOUString( sTestName , RTL_TEXTENCODING_ASCII_US );
        aDllName += L".so";
#endif

        xReg->registerImplementation(
            OUString::createFromAscii( "com.sun.star.loader.SharedLibrary" ) ,
            aDllName,
            xSimpleReg );
    }
    catch( Exception & e )
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
        catch( Exception & e ) {
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
