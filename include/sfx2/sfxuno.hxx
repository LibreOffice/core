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

#ifndef _SFX_SFXUNO_HXX
#define _SFX_SFXUNO_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/task/ErrorCodeIOException.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

#include <com/sun/star/uno/Type.h>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/factory.hxx>

#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>


inline sal_Bool operator==( const css::util::URL& aURL1, const css::util::URL& aURL2 )
{
    return aURL1.Complete == aURL2.Complete;
}

class SfxAllItemSet ;
class SfxItemSet    ;
class SfxSlot       ;

SFX2_DLLPUBLIC void TransformParameters(            sal_uInt16                          nSlotId     ,
                            const   css::uno::Sequence< css::beans::PropertyValue >&    seqArgs     ,
                                    SfxAllItemSet&                                      aSet        ,
                            const   SfxSlot*                                            pSlot = 0   );

SFX2_DLLPUBLIC void TransformItems(         sal_uInt16                                  nSlotId     ,
                            const   SfxItemSet&                                         aSet        ,
                                    css::uno::Sequence< css::beans::PropertyValue >&    seqArgs     ,
                            const   SfxSlot*                            pSlot = 0   );

bool GetEncryptionData_Impl( const SfxItemSet* pSet, css::uno::Sequence< css::beans::NamedValue >& aEncryptionData );

#define FrameSearchFlags            sal_Int32

//________________________________________________________________________________________________________________________
//  macros for declaration and definition of uno-services
//________________________________________________________________________________________________________________________

//************************************************************************************************************************
//  declaration of      XInterface::queryInterface()
//                      XInterface::aquire()
//                      XInterface::release()
//                      XTypeProvider::getTypes()
//                      XTypeProvider::getImplementationId()
//                      XServiceInfo::getImplementationName()
//                      XServiceInfo::supportsService()
//                      XServiceInfo::getSupportedServiceNames()
//                      static xxx::impl_getStaticSupportedServiceNames()
//                      static xxx::impl_getStaticImplementationName()
//                      static xxx::impl_createInstance()
//************************************************************************************************************************
#define SFX_DECL_XSERVICEINFO_NOFACTORY                                                                                         \
    /* XServiceInfo */                                                                                                          \
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException );                                 \
    virtual sal_Bool SAL_CALL supportsService( const OUString& sServiceName ) throw( css::uno::RuntimeException );         \
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException );        \
                                                                                                                                \
    /* Helper for XServiceInfo */                                                                                               \
    static css::uno::Sequence< OUString > impl_getStaticSupportedServiceNames();                                           \
    static OUString impl_getStaticImplementationName();                                                                    \
                                                                                                                                \
    /* Helper for registry */                                                                                                   \
    static css::uno::Reference< css::uno::XInterface > SAL_CALL impl_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager ) throw( css::uno::Exception );

//************************************************************************************************************************
//  declaration of      XInterface::queryInterface()
//                      XInterface::aquire()
//                      XInterface::release()
//                      XTypeProvider::getTypes()
//                      XTypeProvider::getImplementationId()
//                      XServiceInfo::getImplementationName()
//                      XServiceInfo::supportsService()
//                      XServiceInfo::getSupportedServiceNames()
//                      static xxx::impl_getStaticSupportedServiceNames()
//                      static xxx::impl_getStaticImplementationName()
//                      static xxx::impl_createInstance()
//                      static xxx::impl_createFactory()
//************************************************************************************************************************
#define SFX_DECL_XSERVICEINFO                                                                                             \
    SFX_DECL_XSERVICEINFO_NOFACTORY                                                                                       \
    static css::uno::Reference< css::lang::XSingleServiceFactory > impl_createFactory( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager );

//************************************************************************************************************************
//  implementation of   XServiceInfo::getImplementationName()
//                      XServiceInfo::supportsService()
//                      XServiceInfo::getSupportedServiceNames()
//                      static xxx::impl_getStaticSupportedServiceNames()
//                      static xxx::impl_getStaticImplementationName()
//                      static xxx::impl_createInstance()
//************************************************************************************************************************
#define SFX_IMPL_XSERVICEINFO( IMPLCLASS, IMPLSERVICENAME, IMPLNAME )                                                                               \
                                                                                                                                                    \
    /* XServiceInfo */                                                                                                                              \
    OUString SAL_CALL IMPLCLASS::getImplementationName() throw( css::uno::RuntimeException )                                                   \
    {                                                                                                                                               \
        return impl_getStaticImplementationName();                                                                                                  \
    }                                                                                                                                               \
                                                                                                                                                    \
    /* XServiceInfo */                                                                                                                              \
    sal_Bool SAL_CALL IMPLCLASS::supportsService( const OUString& sServiceName ) throw( css::uno::RuntimeException )                           \
    {                                                                                                                                               \
        css::uno::Sequence< OUString > seqServiceNames = getSupportedServiceNames();                                                           \
        const OUString*         pArray          = seqServiceNames.getConstArray();                                                             \
        for ( sal_Int32 nCounter=0; nCounter<seqServiceNames.getLength(); nCounter++ )                                                              \
        {                                                                                                                                           \
            if ( pArray[nCounter] == sServiceName )                                                                                                 \
            {                                                                                                                                       \
                return sal_True ;                                                                                                                   \
            }                                                                                                                                       \
        }                                                                                                                                           \
        return sal_False ;                                                                                                                          \
    }                                                                                                                                               \
                                                                                                                                                    \
    /* XServiceInfo */                                                                                                                              \
    css::uno::Sequence< OUString > SAL_CALL IMPLCLASS::getSupportedServiceNames() throw( css::uno::RuntimeException )                          \
    {                                                                                                                                               \
        return impl_getStaticSupportedServiceNames();                                                                                               \
    }                                                                                                                                               \
                                                                                                                                                    \
    /* Helper for XServiceInfo */                                                                                                                   \
    css::uno::Sequence< OUString > IMPLCLASS::impl_getStaticSupportedServiceNames()                                                            \
    {                                                                                                                                               \
        css::uno::Sequence< OUString > seqServiceNames( 1 );                                                                                   \
        seqServiceNames.getArray() [0] = OUString::createFromAscii( IMPLSERVICENAME );                                                         \
        return seqServiceNames ;                                                                                                                    \
    }                                                                                                                                               \
                                                                                                                                                    \
    /* Helper for XServiceInfo */                                                                                                                   \
    OUString IMPLCLASS::impl_getStaticImplementationName()                                                                                     \
    {                                                                                                                                               \
        return OUString::createFromAscii( IMPLNAME );                                                                                          \
    }                                                                                                                                               \
                                                                                                                                                    \
    /* Helper for registry */                                                                                                                       \
    css::uno::Reference< css::uno::XInterface > SAL_CALL IMPLCLASS::impl_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager ) throw( css::uno::Exception )       \
    {                                                                                                                                               \
        return css::uno::Reference< css::uno::XInterface >( *new IMPLCLASS( xServiceManager ) );                                                    \
    }

//************************************************************************************************************************
//  implementation of   XServiceInfo::getImplementationName()
//                      XServiceInfo::supportsService()
//                      XServiceInfo::getSupportedServiceNames()
//                      static xxx::impl_getStaticSupportedServiceNames()
//                      static xxx::impl_getStaticImplementationName()
//                      static xxx::impl_createInstance()
//************************************************************************************************************************
#define SFX_IMPL_XSERVICEINFO_CTX( IMPLCLASS, IMPLSERVICENAME, IMPLNAME )                                                                           \
                                                                                                                                                    \
    /* XServiceInfo */                                                                                                                              \
    OUString SAL_CALL IMPLCLASS::getImplementationName() throw( css::uno::RuntimeException )                                                   \
    {                                                                                                                                               \
        return impl_getStaticImplementationName();                                                                                                  \
    }                                                                                                                                               \
                                                                                                                                                    \
    /* XServiceInfo */                                                                                                                              \
    sal_Bool SAL_CALL IMPLCLASS::supportsService( const OUString& sServiceName ) throw( css::uno::RuntimeException )                           \
    {                                                                                                                                               \
        css::uno::Sequence< OUString > seqServiceNames = getSupportedServiceNames();                                                           \
        const OUString*         pArray          = seqServiceNames.getConstArray();                                                             \
        for ( sal_Int32 nCounter=0; nCounter<seqServiceNames.getLength(); nCounter++ )                                                              \
        {                                                                                                                                           \
            if ( pArray[nCounter] == sServiceName )                                                                                                 \
            {                                                                                                                                       \
                return sal_True ;                                                                                                                   \
            }                                                                                                                                       \
        }                                                                                                                                           \
        return sal_False ;                                                                                                                          \
    }                                                                                                                                               \
                                                                                                                                                    \
    /* XServiceInfo */                                                                                                                              \
    css::uno::Sequence< OUString > SAL_CALL IMPLCLASS::getSupportedServiceNames() throw( css::uno::RuntimeException )                          \
    {                                                                                                                                               \
        return impl_getStaticSupportedServiceNames();                                                                                               \
    }                                                                                                                                               \
                                                                                                                                                    \
    /* Helper for XServiceInfo */                                                                                                                   \
    css::uno::Sequence< OUString > IMPLCLASS::impl_getStaticSupportedServiceNames()                                                            \
    {                                                                                                                                               \
        css::uno::Sequence< OUString > seqServiceNames( 1 );                                                                                   \
        seqServiceNames.getArray() [0] = OUString::createFromAscii( IMPLSERVICENAME );                                                         \
        return seqServiceNames ;                                                                                                                    \
    }                                                                                                                                               \
                                                                                                                                                    \
    /* Helper for XServiceInfo */                                                                                                                   \
    OUString IMPLCLASS::impl_getStaticImplementationName()                                                                                     \
    {                                                                                                                                               \
        return OUString::createFromAscii( IMPLNAME );                                                                                          \
    }                                                                                                                                               \
                                                                                                                                                    \
    /* Helper for registry */                                                                                                                       \
    css::uno::Reference< css::uno::XInterface > SAL_CALL IMPLCLASS::impl_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager ) throw( css::uno::Exception )       \
    {                                                                                                                                               \
        return css::uno::Reference< css::uno::XInterface >( *new IMPLCLASS( comphelper::getComponentContext(xServiceManager) ) );                   \
    }

//************************************************************************************************************************
//  definition of createFactory() for MultiServices
//************************************************************************************************************************
#define SFX_IMPL_SINGLEFACTORY( IMPLCLASS )                                                                                                         \
                                                                                                                                                    \
    css::uno::Reference< css::lang::XSingleServiceFactory > IMPLCLASS::impl_createFactory( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager )        \
    {                                                                                                                                               \
        css::uno::Reference< css::lang::XSingleServiceFactory > xReturn    (                                                                        \
                                                    cppu::createSingleFactory(  xServiceManager                                     ,               \
                                                                                IMPLCLASS::impl_getStaticImplementationName()       ,               \
                                                                                IMPLCLASS::impl_createInstance                      ,               \
                                                                                IMPLCLASS::impl_getStaticSupportedServiceNames()    )               \
                                                            );                                                                                      \
        return xReturn ;                                                                                                                            \
    }

//************************************************************************************************************************
//  definition of createFactory() for OneInstance-Services
//************************************************************************************************************************
#define SFX_IMPL_ONEINSTANCEFACTORY( IMPLCLASS )                                                                                                    \
                                                                                                                                                    \
    css::uno::Reference< css::lang::XSingleServiceFactory > IMPLCLASS::impl_createFactory( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager )        \
    {                                                                                                                                               \
        css::uno::Reference< css::lang::XSingleServiceFactory > xReturn(                                                                            \
                                                    cppu::createOneInstanceFactory( xServiceManager                                     ,           \
                                                                                    IMPLCLASS::impl_getStaticImplementationName()       ,           \
                                                                                    IMPLCLASS::impl_createInstance                      ,           \
                                                                                    IMPLCLASS::impl_getStaticSupportedServiceNames()    )           \
                                                        );                                                                                          \
        return xReturn ;                                                                                                                            \
    }

//************************************************************************************************************************
//  definition for "extern c sfx_component_getFactory()"
//************************************************************************************************************************
#define CREATEFACTORY(CLASS)                                                                                            \
                                                                                                                        \
    /* Create right factory ... */                                                                                      \
    xFactory = CLASS::impl_createFactory( xServiceManager );

//************************************************************************************************************************
//  definition for "extern c sfx_component_getFactory()"
//************************************************************************************************************************
#define IF_NAME_CREATECOMPONENTFACTORY(CLASS)                                                                           \
                                                                                                                        \
    if ( CLASS::impl_getStaticImplementationName().equals( OUString::createFromAscii( pImplementationName ) ) )    \
    {                                                                                                                   \
        CREATEFACTORY ( CLASS )                                                                                         \
    }

#endif // _SFX_SFXUNO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
