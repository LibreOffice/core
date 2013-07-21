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

#ifndef __FRAMEWORK_MACROS_XSERVICEINFO_HXX_
#define __FRAMEWORK_MACROS_XSERVICEINFO_HXX_

#include <general.h>

#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/factory.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <rtl/ustring.hxx>

namespace framework{

/*_________________________________________________________________________________________________________________

    macros for declaration and definition of XServiceInfo
    Please use follow public macros only!

    1)  DECLARE_XSERVICEINFO                                                                                => use it to declare XServiceInfo in your header
    2)  DEFINE_XSERVICEINFO_MULTISERVICE( CLASS, XINTERFACECAST, SERVICENAME, IMPLEMENTATIONNAME )          => use it to define XServiceInfo for multi service mode
    3)  DEFINE_XSERVICEINFO_ONEINSTANCESERVICE( CLASS, XINTERFACECAST, SERVICENAME, IMPLEMENTATIONNAME )    => use it to define XServiceInfo for one instance service mode
    4)  DEFINE_INIT_SERVICE( CLASS )                                                                        => use it to implement your own impl_initService() method, which is neccessary for initializeing object by using his own reference!

_________________________________________________________________________________________________________________*/

//*****************************************************************************************************************
//  private
//  implementation of XServiceInfo and helper functions
//*****************************************************************************************************************
#define PRIVATE_DEFINE_XSERVICEINFO_BASE( CLASS, XINTERFACECAST, SERVICENAME, IMPLEMENTATIONNAME )                                                  \
    /*===========================================================================================================*/                                 \
    /* XServiceInfo                                                                                              */                                 \
    /*===========================================================================================================*/                                 \
    OUString SAL_CALL CLASS::getImplementationName() throw( css::uno::RuntimeException )                                                     \
    {                                                                                                                                               \
        return impl_getStaticImplementationName();                                                                                                  \
    }                                                                                                                                               \
                                                                                                                                                    \
    /*===========================================================================================================*/                                 \
    /* XServiceInfo                                                                                              */                                 \
    /*===========================================================================================================*/                                 \
    sal_Bool SAL_CALL CLASS::supportsService( const OUString& sServiceName ) throw( css::uno::RuntimeException )                             \
    {                                                                                                                                               \
        return ::comphelper::findValue(getSupportedServiceNames(), sServiceName, sal_True).getLength() != 0;                                        \
    }                                                                                                                                               \
                                                                                                                                                    \
    /*===========================================================================================================*/                                 \
    /* XServiceInfo                                                                                              */                                 \
    /*===========================================================================================================*/                                 \
    css::uno::Sequence< OUString > SAL_CALL CLASS::getSupportedServiceNames() throw( css::uno::RuntimeException )                            \
    {                                                                                                                                               \
        return impl_getStaticSupportedServiceNames();                                                                                               \
    }                                                                                                                                               \
                                                                                                                                                    \
    /*===========================================================================================================*/                                 \
    /* Helper for XServiceInfo                                                                                   */                                 \
    /*===========================================================================================================*/                                 \
    css::uno::Sequence< OUString > CLASS::impl_getStaticSupportedServiceNames()                                                              \
    {                                                                                                                                               \
        css::uno::Sequence< OUString > seqServiceNames( 1 );                                                                                 \
        seqServiceNames.getArray() [0] = SERVICENAME ;                                                                                              \
        return seqServiceNames;                                                                                                                     \
    }                                                                                                                                               \
                                                                                                                                                    \
    /*===========================================================================================================*/                                 \
    /* Helper for XServiceInfo                                                                                   */                                 \
    /*===========================================================================================================*/                                 \
    OUString CLASS::impl_getStaticImplementationName()                                                                                       \
    {                                                                                                                                               \
        return IMPLEMENTATIONNAME ;                                                                                                                 \
    }

#define PRIVATE_DEFINE_XSERVICEINFO_OLDSTYLE( CLASS, XINTERFACECAST, SERVICENAME, IMPLEMENTATIONNAME )                                              \
    PRIVATE_DEFINE_XSERVICEINFO_BASE( CLASS, XINTERFACECAST, SERVICENAME, IMPLEMENTATIONNAME )                                                      \
    /*===========================================================================================================*/                                 \
    /* Helper for registry                                                                                       */                                 \
    /* Attention: To avoid against wrong ref counts during our own initialize procedure, we must                 */                                 \
    /*            use right EXTERNAL handling of them. That's why you should do nothing in your ctor, which could*/                                 \
    /*            work on your ref count! All other things are allowed. Do work with your own reference - please */                                 \
    /*            use "impl_initService()" method.                                                               */                                 \
    /*===========================================================================================================*/                                 \
    css::uno::Reference< css::uno::XInterface > SAL_CALL CLASS::impl_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager ) throw( css::uno::Exception )  \
    {                                                                                                                                                                                              \
        /* create new instance of service */                                                                                                                                                       \
        CLASS* pClass = new CLASS( xServiceManager );                                                                                                                                              \
        /* hold it alive by increasing his ref count!!! */                                                                                                                                         \
        css::uno::Reference< css::uno::XInterface > xService( static_cast< XINTERFACECAST* >(pClass), css::uno::UNO_QUERY );                                                                       \
        /* initialize new service instance ... he can use his own refcount ... we hold it! */                                                                                                      \
        pClass->impl_initService();                                                                                                                                                                \
        /* return new created service as reference */                                                                                                                                              \
        return xService;                                                                                                                                                                           \
    }

#define PRIVATE_DEFINE_XSERVICEINFO_NEWSTYLE( CLASS, XINTERFACECAST, SERVICENAME, IMPLEMENTATIONNAME )                                              \
    PRIVATE_DEFINE_XSERVICEINFO_BASE( CLASS, XINTERFACECAST, SERVICENAME, IMPLEMENTATIONNAME )                                                      \
    /*===========================================================================================================*/                                 \
    /* Helper for registry                                                                                       */                                 \
    /* Attention: To avoid against wrong ref counts during our own initialize procedure, we must                 */                                 \
    /*            use right EXTERNAL handling of them. That's why you should do nothing in your ctor, which could*/                                 \
    /*            work on your ref count! All other things are allowed. Do work with your own reference - please */                                 \
    /*            use "impl_initService()" method.                                                               */                                 \
    /*===========================================================================================================*/                                 \
    css::uno::Reference< css::uno::XInterface > SAL_CALL CLASS::impl_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager )\
        throw( css::uno::Exception )                                                                                                                                \
    {                                                                                                                                                               \
        /* retrieve component context from the given service manager */                                                                                             \
        css::uno::Reference< css::uno::XComponentContext > xComponentContext(                                                                                       \
            comphelper::getComponentContext( xServiceManager ) );                                                                                                   \
        /* create new instance of service */                                                                                                                        \
        CLASS* pClass = new CLASS( xComponentContext );                                                                                                             \
        /* hold it alive by increasing his ref count!!! */                                                                                                          \
        css::uno::Reference< css::uno::XInterface > xService( static_cast< XINTERFACECAST* >(pClass), css::uno::UNO_QUERY );                                        \
        /* initialize new service instance ... he can use his own refcount ... we hold it! */                                                                       \
        pClass->impl_initService();                                                                                                                                 \
        /* return new created service as reference */                                                                                                               \
        return xService;                                                                                                                                            \
    }

//*****************************************************************************************************************
//  private
//  definition of helper function createFactory() for multiple services
//*****************************************************************************************************************
#define PRIVATE_DEFINE_SINGLEFACTORY( CLASS )                                                                                                                           \
    css::uno::Reference< css::lang::XSingleServiceFactory > CLASS::impl_createFactory( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager )  \
    {                                                                                                                                                                   \
        css::uno::Reference< css::lang::XSingleServiceFactory > xReturn ( cppu::createSingleFactory (   xServiceManager                             ,                   \
                                                                                                        CLASS::impl_getStaticImplementationName()   ,                   \
                                                                                                        CLASS::impl_createInstance                  ,                   \
                                                                                                        CLASS::impl_getStaticSupportedServiceNames()                    \
                                                                                                    )                                                                   \
                                                                        );                                                                                              \
        return xReturn;                                                                                                                                                 \
    }

//*****************************************************************************************************************
//  private
//  definition of helper function createFactory() for one instance services
//*****************************************************************************************************************
#define PRIVATE_DEFINE_ONEINSTANCEFACTORY( CLASS )                                                                                                                      \
    css::uno::Reference< css::lang::XSingleServiceFactory > CLASS::impl_createFactory( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager )  \
    {                                                                                                                                                                   \
        css::uno::Reference< css::lang::XSingleServiceFactory > xReturn ( cppu::createOneInstanceFactory    (   xServiceManager                             ,           \
                                                                                                                CLASS::impl_getStaticImplementationName()   ,           \
                                                                                                                CLASS::impl_createInstance                  ,           \
                                                                                                                CLASS::impl_getStaticSupportedServiceNames()            \
                                                                                                            )                                                           \
                                                                        );                                                                                              \
        return xReturn;                                                                                                                                                 \
    }

//*****************************************************************************************************************
//  public
//  declaration of XServiceInfo and helper functions
//*****************************************************************************************************************

#define DECLARE_XSERVICEINFO_NOFACTORY                                                                                                                                                                                                  \
    /* interface XServiceInfo */                                                                                                                                                                                                        \
    virtual OUString                                        SAL_CALL getImplementationName              (                                                                               ) throw( css::uno::RuntimeException );   \
    virtual sal_Bool                                               SAL_CALL supportsService                    ( const OUString&                                        sServiceName    ) throw( css::uno::RuntimeException );   \
    virtual css::uno::Sequence< OUString >                  SAL_CALL getSupportedServiceNames           (                                                                               ) throw( css::uno::RuntimeException );   \
    /* Helper for XServiceInfo */                                                                                                                                                                                                       \
    static css::uno::Sequence< OUString >                   SAL_CALL impl_getStaticSupportedServiceNames(                                                                               );                                       \
    static OUString                                         SAL_CALL impl_getStaticImplementationName   (                                                                               );                                       \
    /* Helper for initialization of service by using own reference! */                                                                                                                                                                  \
    virtual void                                                   SAL_CALL impl_initService                   (                                                                               );                                       \

#define DECLARE_XSERVICEINFO                                                                                                                                                                                                            \
    DECLARE_XSERVICEINFO_NOFACTORY \
    /* Helper for registry */                                                                                                                                                                                                           \
    static css::uno::Reference< css::uno::XInterface >             SAL_CALL impl_createInstance                ( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager ) throw( css::uno::Exception );          \
    static css::uno::Reference< css::lang::XSingleServiceFactory > SAL_CALL impl_createFactory                 ( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager );                                       \

//*****************************************************************************************************************
//  public
//  implementation of XServiceInfo
//*****************************************************************************************************************
#define DEFINE_XSERVICEINFO_MULTISERVICE( CLASS, XINTERFACECAST, SERVICENAME, IMPLEMENTATIONNAME )              \
    PRIVATE_DEFINE_XSERVICEINFO_OLDSTYLE( CLASS, XINTERFACECAST, SERVICENAME, IMPLEMENTATIONNAME )              \
    PRIVATE_DEFINE_SINGLEFACTORY( CLASS )

#define DEFINE_XSERVICEINFO_ONEINSTANCESERVICE( CLASS, XINTERFACECAST, SERVICENAME, IMPLEMENTATIONNAME )        \
    PRIVATE_DEFINE_XSERVICEINFO_OLDSTYLE( CLASS, XINTERFACECAST, SERVICENAME, IMPLEMENTATIONNAME )              \
    PRIVATE_DEFINE_ONEINSTANCEFACTORY( CLASS )

#define DEFINE_XSERVICEINFO_MULTISERVICE_2( CLASS, XINTERFACECAST, SERVICENAME, IMPLEMENTATIONNAME )            \
    PRIVATE_DEFINE_XSERVICEINFO_NEWSTYLE( CLASS, XINTERFACECAST, SERVICENAME, IMPLEMENTATIONNAME )              \
    PRIVATE_DEFINE_SINGLEFACTORY( CLASS )

#define DEFINE_XSERVICEINFO_ONEINSTANCESERVICE_2( CLASS, XINTERFACECAST, SERVICENAME, IMPLEMENTATIONNAME )        \
    PRIVATE_DEFINE_XSERVICEINFO_NEWSTYLE( CLASS, XINTERFACECAST, SERVICENAME, IMPLEMENTATIONNAME )              \
    PRIVATE_DEFINE_ONEINSTANCEFACTORY( CLASS )

//*****************************************************************************************************************
//  public
//  implementation of service initialize!
//  example of using:   DEFINE_INIT_SERVICE( MyClassName,
//                          {
//                              ...
//                              Reference< XInterface > xThis( this, UNO_QUERY );
//                              myMember* pMember = new myMember( xThis );
//                              ...
//                          }
//                      )
//*****************************************************************************************************************
#define DEFINE_INIT_SERVICE( CLASS, FUNCTIONBODY )                                                              \
    void SAL_CALL CLASS::impl_initService()                                                                     \
    {                                                                                                           \
        FUNCTIONBODY                                                                                            \
    }

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_MACROS_XSERVICEINFO_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
