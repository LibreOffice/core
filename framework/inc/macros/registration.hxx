/*************************************************************************
 *
 *  $RCSfile: registration.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:29:23 $
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

#ifndef __FRAMEWORK_MACROS_REGISTRATION_HXX_
#define __FRAMEWORK_MACROS_REGISTRATION_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_REGISTRY_INVALIDREGISTRYEXCEPTION_HPP_
#include <com/sun/star/registry/InvalidRegistryException.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

//namespace framework{

/*_________________________________________________________________________________________________________________

    macros for registration of services
    Please use follow public macros only!

    1)  COMPONENTINFO( CLASS )                      => use it as parameter for COMPONENT_WRITEINFO( INFOS )
    2)  IFFACTORY( CLASS )                          => use it as parameter for COMPONENT_GETFACTORY( IFFACTORIES )
    3)  COMPONENTGETIMPLEMENTATIONENVIRONMENT       => use it to define exported function component_getImplementationEnvironment()
    4)  COMPONENTWRITEINFO( INFOS )                 => use it to define exported function component_writeInfo()
    5)  COMPONENTGETFACTORY( IFFACTORIES )          => use it to define exported function component_getFactory()

_________________________________________________________________________________________________________________*/

//*****************************************************************************************************************
//  public
//  use it as parameter for COMPONENT_WRITEINFO( INFOS )
//*****************************************************************************************************************
#define COMPONENTINFO( CLASS )                                                                                                          \
    try                                                                                                                                 \
    {                                                                                                                                   \
        /* Set default result of follow operations !!! */                                                                               \
        bReturn = sal_False;                                                                                                            \
        /* Do the follow only, if given key is valid ! */                                                                               \
        if ( xKey.is() == sal_True )                                                                                                    \
        {                                                                                                                               \
            LOG_REGISTRATION_WRITEINFO( "\t\t\txKey is valid ...\n" )                                                                   \
            /* Build new keyname    */                                                                                                  \
            sKeyName     =  DECLARE_ASCII( "/" );                                                                                       \
            sKeyName    +=  CLASS::impl_getStaticImplementationName();                                                                  \
            sKeyName    +=  DECLARE_ASCII( "/UNO/SERVICES" );                                                                           \
            LOG_REGISTRATION_WRITEINFO( "\t\t\tcreate key \"" )                                                                         \
            LOG_REGISTRATION_WRITEINFO( U2B( sKeyName ) )                                                                               \
            LOG_REGISTRATION_WRITEINFO( "\" ...\n" )                                                                                    \
            /* Create new key with new name. */                                                                                         \
             xNewKey = xKey->createKey( sKeyName );                                                                                     \
            /* If this new key valid ... */                                                                                             \
            if ( xNewKey.is() == sal_True )                                                                                             \
            {                                                                                                                           \
                LOG_REGISTRATION_WRITEINFO( "\t\t\t\ttsuccessful ...\n" )                                                               \
                /* Get information about supported services. */                                                                         \
                seqServiceNames =   CLASS::impl_getStaticSupportedServiceNames()    ;                                                   \
                pArray          =   seqServiceNames.getArray()                      ;                                                   \
                nLength         =   seqServiceNames.getLength()                     ;                                                   \
                nCounter        =   0                                               ;                                                   \
                /* Then set this information on this key. */                                                                            \
                for ( nCounter = 0; nCounter < nLength; ++nCounter )                                                                    \
                {                                                                                                                       \
                    LOG_REGISTRATION_WRITEINFO( "\t\t\t\twrite key \"" )                                                                \
                    LOG_REGISTRATION_WRITEINFO( U2B( pArray[nCounter] ) )                                                               \
                    LOG_REGISTRATION_WRITEINFO( "\" to registry ...\n" )                                                                \
                    xNewKey->createKey( pArray[nCounter] );                                                                             \
                }                                                                                                                       \
                /* Result of this operations = OK. */                                                                                   \
                bReturn = sal_True ;                                                                                                    \
            }                                                                                                                           \
        }                                                                                                                               \
    }                                                                                                                                   \
    catch( ::com::sun::star::registry::InvalidRegistryException& )                                                                      \
    {                                                                                                                                   \
        LOG_REGISTRATION_WRITEINFO( "\n\nERROR:\nInvalidRegistryException detected\n\n" )                                               \
        bReturn = sal_False ;                                                                                                           \
    }

//*****************************************************************************************************************
//  public
//  use it as parameter for COMPONENT_GETFACTORY( IFFACTORIES )
//*****************************************************************************************************************
#define IFFACTORY( CLASS )                                                                                                              \
    /* If searched name found ... */                                                                                                    \
    /* You can't add some statements before follow line ... Here can be an ELSE-statement! */                                           \
    if ( CLASS::impl_getStaticImplementationName().equals( ::rtl::OUString::createFromAscii( pImplementationName ) ) )                  \
    {                                                                                                                                   \
        LOG_REGISTRATION_GETFACTORY( "\t\tImplementationname found - try to create factory! ...\n" )                                    \
        /* ... then create right factory for this service.                                  */                                          \
        /* xFactory and xServiceManager are local variables of method which use this macro. */                                          \
        xFactory = CLASS::impl_createFactory( xServiceManager );                                                                        \
    }

//*****************************************************************************************************************
//  public
//  define helper to get information about service environment
//*****************************************************************************************************************
#define COMPONENTGETIMPLEMENTATIONENVIRONMENT                                                                                           \
    extern "C" void SAL_CALL component_getImplementationEnvironment( const  sal_Char**          ppEnvironmentTypeName   ,               \
                                                                             uno_Environment**  ppEnvironment           )               \
    {                                                                                                                                   \
        *ppEnvironmentTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME ;                                                                   \
    }

//*****************************************************************************************************************
//  public
//  define registration of service
//*****************************************************************************************************************
#define COMPONENTWRITEINFO( INFOS )                                                                                                     \
    extern "C" sal_Bool SAL_CALL component_writeInfo(   void*   pServiceManager ,                                                       \
                                                        void*   pRegistryKey    )                                                       \
    {                                                                                                                                   \
        LOG_REGISTRATION_WRITEINFO( "\t[start]\n" )                                                                                     \
        /* Set default return value for this operation - if it failed. */                                                               \
        sal_Bool bReturn = sal_False ;                                                                                                  \
        if ( pRegistryKey != NULL )                                                                                                     \
        {                                                                                                                               \
            LOG_REGISTRATION_WRITEINFO( "\t\tpRegistryKey is valid ...\n" )                                                             \
            /* Define variables for following helper macros! */                                                                         \
            /* bReturn will set automaticly.                 */                                                                         \
            ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey >    xKey            ;                           \
            ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey >    xNewKey         ;                           \
            ::com::sun::star::uno::Sequence<  ::rtl::OUString >                             seqServiceNames ;                           \
            const ::rtl::OUString*                                                          pArray          ;                           \
            sal_Int32                                                                       nLength         ;                           \
            sal_Int32                                                                       nCounter        ;                           \
            ::rtl::OUString                                                                 sKeyName        ;                           \
            xKey = reinterpret_cast< ::com::sun::star::registry::XRegistryKey* >( pRegistryKey );                                       \
            /* This parameter will expand to */                                                                                         \
            /*   "COMPONENT_INFO(a)          */                                                                                         \
            /*    ...                        */                                                                                         \
            /*    COMPONENT_INFO(z)"         */                                                                                         \
            INFOS                                                                                                                       \
        }                                                                                                                               \
        LOG_REGISTRATION_WRITEINFO( "\t[end]\n" )                                                                                       \
        /* Return with result of this operation. */                                                                                     \
        return bReturn ;                                                                                                                \
    }

//*****************************************************************************************************************
//  public
//  define method to instanciate new services
//*****************************************************************************************************************
#define COMPONENTGETFACTORY( IFFACTORIES )                                                                                              \
    extern "C" void* SAL_CALL component_getFactory( const   sal_Char*   pImplementationName ,                                           \
                                                            void*       pServiceManager     ,                                           \
                                                            void*       pRegistryKey        )                                           \
    {                                                                                                                                   \
        LOG_REGISTRATION_GETFACTORY( "\t[start]\n" )                                                                                    \
        /* Set default return value for this operation - if it failed. */                                                               \
        void* pReturn = NULL ;                                                                                                          \
        if  (                                                                                                                           \
                ( pImplementationName   !=  NULL ) &&                                                                                   \
                ( pServiceManager       !=  NULL )                                                                                      \
            )                                                                                                                           \
        {                                                                                                                               \
            LOG_REGISTRATION_GETFACTORY( "\t\tpImplementationName and pServiceManager are valid ...\n" )                                \
            /* Define variables which are used in following macros. */                                                                  \
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory >   xFactory            ;                   \
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    xServiceManager     ;                   \
            xServiceManager = reinterpret_cast< ::com::sun::star::lang::XMultiServiceFactory* >( pServiceManager )  ;                   \
            /* These parameter will expand to      */                                                                                   \
            /* "IF_NAME_CREATECOMPONENTFACTORY(a)  */                                                                                   \
            /*  else                               */                                                                                   \
            /*  ...                                */                                                                                   \
            /*  else                               */                                                                                   \
            /*  IF_NAME_CREATECOMPONENTFACTORY(z)" */                                                                                   \
            IFFACTORIES                                                                                                                 \
            /* Factory is valid, if service was found. */                                                                               \
            if ( xFactory.is() == sal_True )                                                                                            \
            {                                                                                                                           \
                LOG_REGISTRATION_GETFACTORY( "\t\t\txFactory valid - service was found ...\n" )                                         \
                xFactory->acquire();                                                                                                    \
                pReturn = xFactory.get();                                                                                               \
            }                                                                                                                           \
        }                                                                                                                               \
        LOG_REGISTRATION_GETFACTORY( "\t[end]\n" )                                                                                      \
        /* Return with result of this operation. */                                                                                     \
        return pReturn ;                                                                                                                \
    }

//}     //  namespace framework

#endif  //  #ifndef __FRAMEWORK_MACROS_REGISTRATION_HXX_
