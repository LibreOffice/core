/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_shell.hxx"


//-----------------------------------------------------------
//  interface includes
//-----------------------------------------------------------
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/system/XSystemShellExecute.hpp>
#include <cppuhelper/servicefactory.hxx>


#include <osl/file.hxx>

//--------------------------------------------------------------
//  other includes
//--------------------------------------------------------------
#include <cppuhelper/servicefactory.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <osl/diagnose.h>

#include <stdio.h>

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

//--------------------------------------------------------------
//  namesapces
//--------------------------------------------------------------

using namespace ::rtl                   ;
using namespace ::cppu                  ;
using namespace ::com::sun::star::uno   ;
using namespace ::com::sun::star::lang  ;
using namespace std                     ;
using namespace com::sun::star::system;

//--------------------------------------------------------------
//  defines
//--------------------------------------------------------------

#define RDB_SYSPATH "D:\\Projects\\gsl\\shell\\wntmsci7\\bin\\applicat.rdb"

//--------------------------------------------------------------
//  global variables
//--------------------------------------------------------------

Reference< XMultiServiceFactory >   g_xFactory;


//--------------------------------------------------------------
//  main
//--------------------------------------------------------------


// int SAL_CALL main(int nArgc, char* Argv[], char* Env[]   )
int SAL_CALL main(int nArgc, char* Argv[], char*    )
{
    //-------------------------------------------------
    // get the global service-manager
    //-------------------------------------------------

    if ( nArgc < 4 )
        return 0;

    // Get global factory for uno services.
    OUString rdbName = OUString( RTL_CONSTASCII_USTRINGPARAM( RDB_SYSPATH ) );
    Reference< XMultiServiceFactory > g_xFactory( createRegistryServiceFactory( rdbName ) );

    // Print a message if an error occured.
    if ( g_xFactory.is() == sal_False )
    {
        OSL_ENSURE(sal_False, "Can't create RegistryServiceFactory");
        return(-1);
    }

    printf("Creating RegistryServiceFactory successful\n");

    //-------------------------------------------------
    // try to get an Interface to a XFilePicker Service
    //-------------------------------------------------

    Reference< XSystemShellExecute > xSysShExec(
        g_xFactory->createInstance( OUString::createFromAscii( "com.sun.star.system.SystemShellExecute" ) ), UNO_QUERY );

    if ( !xSysShExec.is() )
    {
        OSL_ENSURE( sal_False, "Error creating SystemShellExecute Service" );
        return(-1);
    }

    //"c:\\winnt\\notepad.exe"
    OUString cmd = OUString::createFromAscii( Argv[1] );
    OUString param = OUString::createFromAscii( Argv[2] ); //c:\\winnt\\iis5.log

    try
    {
        xSysShExec->execute( cmd, param, atoi( Argv[3] ) );
    }
    catch( SystemShellExecuteException&  )
    {
        OSL_ENSURE( sal_False, "Error executing system command" );
    }
    catch( IllegalArgumentException& )
    {
        OSL_ENSURE( sal_False, "Invalid parameter" );
    }

    //--------------------------------------------------
    // shutdown
    //--------------------------------------------------

    // Cast factory to XComponent
    Reference< XComponent > xComponent( g_xFactory, UNO_QUERY );

    // Print a message if an error occured.
    if ( xComponent.is() == sal_False )
    {
        OSL_ENSURE(sal_False, "Error shuting down");
    }

    // Dispose and clear factory
    xComponent->dispose();
    g_xFactory.clear();
    g_xFactory = Reference< XMultiServiceFactory >();

    printf("Test successful\n");

    return 0;
}
