/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: registercontrols.cxx,v $
 * $Revision: 1.8 $
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
// Ths switch on another macro AS_DBG_OUT(...), which will print text to "stdout".

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
#define COMPONENT_INFO(CLASS)                                                                                           \
                                                                                                                        \
    AS_DBG_OUT ( "\tCOMPONENT_INFO():\t[start]\n" )                                                                     \
    try                                                                                                                 \
    {                                                                                                                   \
        /* Set default result of follow operations !!! */                                                               \
        bReturn = sal_False ;                                                                                           \
                                                                                                                        \
        /* Do the follow only, if given key is valid ! */                                                               \
        if ( xKey.is () )                                                                                               \
        {                                                                                                               \
            AS_DBG_OUT ( "\tCOMPONENT_INFO():\t\txkey is valid ...\n" )                                                 \
            /* Build new keyname */                                                                                     \
            sKeyName     =  OUString::createFromAscii( "/" )            ;                                               \
            sKeyName    +=  CLASS::impl_getStaticImplementationName()   ;                                               \
            sKeyName    +=  OUString::createFromAscii( "/UNO/SERVICES" );                                               \
                                                                                                                        \
            /* Create new key with new name. */                                                                         \
             xNewKey = xKey->createKey( sKeyName );                                                                     \
                                                                                                                        \
            /* If this new key valid ... */                                                                             \
            if ( xNewKey.is () )                                                                                        \
            {                                                                                                           \
                AS_DBG_OUT ( "\tCOMPONENT_INFO():\t\txNewkey is valid ...\n" )                                          \
                /* Get information about supported services. */                                                         \
                seqServiceNames =   CLASS::impl_getStaticSupportedServiceNames()    ;                                   \
                pArray          =   seqServiceNames.getArray()                      ;                                   \
                nLength         =   seqServiceNames.getLength()                     ;                                   \
                nCounter        =   0                                               ;                                   \
                                                                                                                        \
                AS_DBG_OUT ( "\tCOMPONENT_INFO():\t\tloop ..." )                                                        \
                /* Then set this information on this key. */                                                            \
                for ( nCounter = 0; nCounter < nLength; ++nCounter )                                                    \
                {                                                                                                       \
                    xNewKey->createKey( pArray [nCounter] );                                                            \
                }                                                                                                       \
                AS_DBG_OUT ( " OK\n" )                                                                                  \
                                                                                                                        \
                /* Result of this operations = OK. */                                                                   \
                bReturn = sal_True ;                                                                                    \
            }                                                                                                           \
            AS_DBG_OUT ( "\tCOMPONENT_INFO():\t\t... leave xNewKey\n" )                                                 \
        }                                                                                                               \
        AS_DBG_OUT ( "\tCOMPONENT_INFO():\t\t... leave xKey\n" )                                                        \
    }                                                                                                                   \
    catch( InvalidRegistryException& )                                                                                  \
    {                                                                                                                   \
        AS_DBG_OUT ( "\tCOMPONENT_INFO():\t\tInvalidRegistryException detected!!!\n" )                                  \
        bReturn = sal_False ;                                                                                           \
    }                                                                                                                   \
    AS_DBG_OUT ( "\tCOMPONENT_INFO():\t[end]\n" )

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
//  write component info to registry
//______________________________________________________________________________________________________________

extern "C" sal_Bool SAL_CALL component_writeInfo(   void*   /*pServiceManager*/ ,
                                                    void*   pRegistryKey    )
{
    AS_DBG_OUT ( "component_writeInfo():\t[start]\n" )

    // Set default return value for this operation - if it failed.
    sal_Bool bReturn = sal_False ;

    if ( pRegistryKey != NULL )
    {
        AS_DBG_OUT ( "component_writeInfo():\t\tpRegistryKey is valid ... enter scope\n" )

        // Define variables for following macros!
        // bReturn is set automaticly.
        Reference< XRegistryKey >       xKey( reinterpret_cast< XRegistryKey* >( pRegistryKey ) )   ;
        Reference< XRegistryKey >       xNewKey                                                     ;
        Sequence< OUString >            seqServiceNames                                             ;
        const OUString*                 pArray                                                      ;
        sal_Int32                       nLength                                                     ;
        sal_Int32                       nCounter                                                    ;
        OUString                        sKeyName                                                    ;

        //=============================================================================
        //  Add new macro line to register new services.
        //
        //  !!! ATTENTION !!!
        //      Write no ";" at end of line! (see macro)
        //=============================================================================
        COMPONENT_INFO  ( FrameControl      )
        COMPONENT_INFO  ( ProgressBar       )
        COMPONENT_INFO  ( ProgressMonitor   )
        COMPONENT_INFO  ( StatusIndicator   )
        //=============================================================================

        AS_DBG_OUT ( "component_writeInfo():\t\t... leave pRegistryKey scope\n" )
    }

    AS_DBG_OUT ( "component_writeInfo():\t[end]\n" )

    // Return with result of this operation.
    return bReturn ;
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
