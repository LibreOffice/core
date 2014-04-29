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



//______________________________________________________________________________________________________________
//  includes of other projects
//______________________________________________________________________________________________________________

#include <cppuhelper/factory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/container/XSet.hpp>

#include <stdio.h>

//______________________________________________________________________________________________________________
//  includes of my own project
//______________________________________________________________________________________________________________

//=============================================================================
//  Add new include line to use new services.
//=============================================================================
#include "framecontrol.hxx"
#include "progressbar.hxx"
#include "progressmonitor.hxx"
#include "statusindicator.hxx"
//=============================================================================

//______________________________________________________________________________________________________________
//  defines
//______________________________________________________________________________________________________________

// If you will debug macros of this file ... you must define follow constant!
// This switch on another macro AS_DBG_OUT(...), which will print text to "stdout".

//#define AS_DBG_SWITCH

//______________________________________________________________________________________________________________
//  namespaces
//______________________________________________________________________________________________________________

using namespace ::rtl                                           ;
using namespace ::cppu                                          ;
using namespace ::unocontrols                                   ;
using namespace ::com::sun::star::uno                           ;
using namespace ::com::sun::star::container                     ;
using namespace ::com::sun::star::lang                          ;
using namespace ::com::sun::star::registry                      ;

//______________________________________________________________________________________________________________
//  macros
//______________________________________________________________________________________________________________

//******************************************************************************************************************************
// See AS_DBG_SWITCH below !!!
#ifdef AS_DBG_SWITCH
    #define AS_DBG_OUT(OUTPUT)  printf( OUTPUT );
#else
    #define AS_DBG_OUT(OUTPUT)
#endif

//******************************************************************************************************************************
#define CREATEINSTANCE(CLASS)                                                                                                               \
                                                                                                                                            \
    static Reference< XInterface > SAL_CALL CLASS##_createInstance ( const Reference< XMultiServiceFactory >& rServiceManager ) throw ( Exception ) \
    {                                                                                                                                       \
        AS_DBG_OUT ( "\tCREATEINSTANCE():\tOK\n" )                                                                                          \
        return Reference< XInterface >( *(OWeakObject*)(new CLASS( rServiceManager )) );                                                    \
    }

//******************************************************************************************************************************
#define CREATEFACTORY_ONEINSTANCE(CLASS)                                                                                \
                                                                                                                        \
    AS_DBG_OUT ( "\tCREATEFACTORY_ONEINSTANCE():\t[start]\n" )                                                          \
    /* Create right factory ... */                                                                                      \
    xFactory = Reference< XSingleServiceFactory >                                                                       \
                    (                                                                                                   \
                        cppu::createOneInstanceFactory  (   xServiceManager                                     ,       \
                                                            CLASS::impl_getStaticImplementationName     ()  ,       \
                                                            CLASS##_createInstance                              ,       \
                                                            CLASS::impl_getStaticSupportedServiceNames  ()  )       \
                    ) ;                                                                                                 \
    AS_DBG_OUT ( "\tCREATEFACTORY_ONEINSTANCE():\t[end]\n" )

//******************************************************************************************************************************
#define CREATEFACTORY_SINGLE(CLASS)                                                                                     \
                                                                                                                        \
    AS_DBG_OUT ( "\tCREATEFACTORY_SINGLE():\t[start]\n" )                                                               \
    /* Create right factory ... */                                                                                      \
    xFactory = Reference< XSingleServiceFactory >                                                                       \
                    (                                                                                                   \
                        cppu::createSingleFactory   (   xServiceManager                                     ,           \
                                                        CLASS::impl_getStaticImplementationName     ()  ,           \
                                                        CLASS##_createInstance                              ,           \
                                                        CLASS::impl_getStaticSupportedServiceNames  ()  )           \
                    ) ;                                                                                                 \
    AS_DBG_OUT ( "\tCREATEFACTORY_SINGLE():\t[end]\n" )

//******************************************************************************************************************************
#define IF_NAME_CREATECOMPONENTFACTORY_ONEINSTANCE(CLASS)                                                               \
                                                                                                                        \
    if ( CLASS::impl_getStaticImplementationName().equals( OUString::createFromAscii( pImplementationName ) ) )     \
    {                                                                                                                   \
        AS_DBG_OUT ( "\tIF_NAME_CREATECOMPONENTFACTORY_ONEINSTANCE():\timplementationname found\n" )                    \
        CREATEFACTORY_ONEINSTANCE ( CLASS )                                                                         \
    }

//******************************************************************************************************************************
#define IF_NAME_CREATECOMPONENTFACTORY_SINGLE(CLASS)                                                                    \
                                                                                                                        \
    if ( CLASS::impl_getStaticImplementationName().equals( OUString::createFromAscii( pImplementationName ) ) )     \
    {                                                                                                                   \
        AS_DBG_OUT ( "\tIF_NAME_CREATECOMPONENTFACTORY_SINGLE():\timplementationname found\n" )                         \
        CREATEFACTORY_SINGLE ( CLASS )                                                                              \
    }

//______________________________________________________________________________________________________________
//  declare functions to create a new instance of service
//______________________________________________________________________________________________________________

//=============================================================================
//  Add new macro line to use new services.
//
//  !!! ATTENTION !!!
//      Write no ";" at end of line! (see macro)
//=============================================================================
CREATEINSTANCE  ( FrameControl      )
CREATEINSTANCE  ( ProgressBar       )
CREATEINSTANCE  ( ProgressMonitor   )
CREATEINSTANCE  ( StatusIndicator   )
//=============================================================================

//______________________________________________________________________________________________________________
//  return environment
//______________________________________________________________________________________________________________

extern "C" void SAL_CALL component_getImplementationEnvironment(    const   sal_Char**          ppEnvironmentTypeName   ,
                                                                            uno_Environment**   /*ppEnvironment*/           )
{
    *ppEnvironmentTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME ;
}

//______________________________________________________________________________________________________________
//  create right component factory
//______________________________________________________________________________________________________________

extern "C" void* SAL_CALL component_getFactory( const   sal_Char*   pImplementationName ,
                                                        void*       pServiceManager     ,
                                                        void*       /*pRegistryKey*/        )
{
    AS_DBG_OUT( "component_getFactory():\t[start]\n" )

    // Set default return value for this operation - if it failed.
    void* pReturn = NULL ;

    if  (
            ( pImplementationName   !=  NULL ) &&
            ( pServiceManager       !=  NULL )
        )
    {
        AS_DBG_OUT( "component_getFactory():\t\t... enter scope - pointer are valid\n" )

        // Define variables which are used in following macros.
        Reference< XSingleServiceFactory >  xFactory                                                                        ;
        Reference< XMultiServiceFactory >   xServiceManager( reinterpret_cast< XMultiServiceFactory* >( pServiceManager ) ) ;

        //=============================================================================
        //  Add new macro line to handle new service.
        //
        //  !!! ATTENTION !!!
        //      Write no ";" at end of line and dont forget "else" ! (see macro)
        //=============================================================================
        IF_NAME_CREATECOMPONENTFACTORY_SINGLE( FrameControl     )
        else
        IF_NAME_CREATECOMPONENTFACTORY_SINGLE( ProgressBar      )
        else
        IF_NAME_CREATECOMPONENTFACTORY_SINGLE( ProgressMonitor  )
        else
        IF_NAME_CREATECOMPONENTFACTORY_SINGLE( StatusIndicator  )
        //=============================================================================

        // Factory is valid - service was found.
        if ( xFactory.is() )
        {
            AS_DBG_OUT( "component_getFactory():\t\t\t... xFactory valid - service was found\n" )

            xFactory->acquire();
            pReturn = xFactory.get();
        }

        AS_DBG_OUT( "component_getFactory():\t\t... leave scope\n" )
    }

    AS_DBG_OUT ( "component_getFactory():\t[end]\n" )

    // Return with result of this operation.
    return pReturn ;
}
