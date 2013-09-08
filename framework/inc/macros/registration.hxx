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

#ifndef __FRAMEWORK_MACROS_REGISTRATION_HXX_
#define __FRAMEWORK_MACROS_REGISTRATION_HXX_

#include <macros/debug.hxx>

#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

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
    if ( CLASS::impl_getStaticImplementationName().equals( OUString::createFromAscii( pImplementationName ) ) )                  \
    {                                                                                                                                   \
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
        /* Set default return value for this operation - if it failed. */                                                               \
        void* pReturn = NULL ;                                                                                                          \
        if  (                                                                                                                           \
                ( pImplementationName   !=  NULL ) &&                                                                                   \
                ( pServiceManager       !=  NULL )                                                                                      \
            )                                                                                                                           \
        {                                                                                                                               \
            /* Define variables which are used in following macros. */                                                                  \
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >   xFactory            ;                   \
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
                xFactory->acquire();                                                                                                    \
                pReturn = xFactory.get();                                                                                               \
            }                                                                                                                           \
        }                                                                                                                               \
        /* Return with result of this operation. */                                                                                     \
        return pReturn ;                                                                                                                \
    }

//}     //  namespace framework

#endif  //  #ifndef __FRAMEWORK_MACROS_REGISTRATION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
