/*************************************************************************
 *
 *  $RCSfile: testcomponent.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:56 $
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
#include <smart/com/sun/star/registry/XImplementationRegistration.hxx>
#include <smart/com/sun/star/lang/XComponent.hxx>
//#include <com/sun/star/registry/ stardiv/uno/repos/simplreg.hxx>

#include <vos/dynload.hxx>
#include <vos/diagnose.hxx>
#include <usr/services.hxx>
#include <vcl/svapp.hxx>
#include <usr/ustring.hxx>
#include <tools/string.hxx>
#include <vos/conditn.hxx>

#include <assert.h>

#include <smart/com/sun/star/test/XSimpleTest.hxx>

#ifdef _USE_NAMESPACE
using namespace rtl;
using namespace vos;
using namespace usr;
#endif


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
    assert( xSMgr.is() );
    registerUsrServices( xSMgr );
    setProcessServiceManager( xSMgr );

    XImplementationRegistrationRef xReg;
    XSimpleRegistryRef xSimpleReg;

    TRY {
        // Create registration service
        XInterfaceRef x = xSMgr->createInstance(
            UString::createFromAscii( "com.sun.star.registry.ImplementationRegistration" ) );
        x->queryInterface( XImplementationRegistration::getSmartUik() , xReg );

/*      x = xSMgr->createInstance( L"stardiv.uno.repos.SimpleRegistry" );
        assert( x.is() );
        x->queryInterface( XSimpleRegistry::getSmartUik() , xSimpleReg );
        assert( xSimpleReg.is() );
        xSimpleReg->open( L"testcomp.rdb" , FALSE , TRUE );
*/  }
    CATCH( Exception ,e ) {
        printf( "%s\n" , OWStringToOString( e.getName() , CHARSET_SYSTEM ).getStr() );
        exit(1);
    }
    END_CATCH;

    sal_Char szBuf[1024];
    OString sTestName;

    TRY {
        // Load dll for the tested component
        for( int n = 2 ; n <argc ; n ++ ) {
            ORealDynamicLoader::computeModuleName( argv[n] , szBuf, 1024 );
            UString aDllName( OStringToOWString( szBuf, CHARSET_SYSTEM ) );

            xReg->registerImplementation(
                UString::createFromAscii( "com.sun.star.loader.SharedLibrary" ),
                aDllName,
                xSimpleReg );
        }
    }
    CATCH( Exception, e ) {
        printf( "Couldn't reach dll %s\n" , szBuf );
        printf( "%s\n" , OWStringToOString( e.getName() , CHARSET_SYSTEM ).getStr() );

        exit(1);
    }
    END_CATCH;


    TRY {
        // Load dll for the test component
        sTestName = "test";
        sTestName += argv[2];

        ORealDynamicLoader::computeModuleName( sTestName.getStr() , szBuf, 1024 );
        UString aDllName = OStringToOWString( szBuf, CHARSET_SYSTEM );
        xReg->registerImplementation(
            UString::createFromAscii( "com.sun.star.loader.SharedLibrary" ) ,
            aDllName,
            xSimpleReg );
    }
    CATCH( Exception , e ) {
        printf( "Couldn't reach dll %s\n" , szBuf );
        printf( "%s\n" , OWStringToOString( e.getName() , CHARSET_SYSTEM ).getStr() );
        exit(1);
    }
    END_CATCH;


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
        TRY {
            nNewHandle = xTest->test( OStringToOWString( argv[1] , CHARSET_SYSTEM ) , x , nHandle );
        }
        CATCH ( Exception , e ) {
            printf( "testcomponent : uncaught exception %s\n" ,
                        OWStringToOString( e.getName(), CHARSET_SYSTEM ).getStr() );
            exit(1);
        }
        AND_CATCH_ALL() {
            printf( "testcomponent : uncaught unknown exception\n"  );
            exit(1);
        }
        END_CATCH;


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
