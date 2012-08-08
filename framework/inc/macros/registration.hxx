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

#ifndef __FRAMEWORK_MACROS_REGISTRATION_HXX_
#define __FRAMEWORK_MACROS_REGISTRATION_HXX_

#include <macros/debug.hxx>

#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

//namespace framework{

/*_________________________________________________________________________________________________________________

    macros for registration of services
    Please use follow public macros only!

    IFFACTORY( CLASS )                          => use it as parameter for COMPONENT_GETFACTORY( IFFACTORIES )
    COMPONENTGETFACTORY( LIB, IFFACTORIES )     => use it to define exported function LIB_component_getFactory()

_________________________________________________________________________________________________________________*/

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
//  define method to instanciate new services
//*****************************************************************************************************************
#define COMPONENTGETFACTORY( LIB, IFFACTORIES )                                                                                         \
    extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL LIB##_component_getFactory( const sal_Char* pImplementationName,                      \
                                                            void*       pServiceManager     ,                                           \
                                                            void*     /*pRegistryKey*/      )                                           \
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
