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

#ifndef __COMPHELPER_REGISTRATION_HXX_
#define __COMPHELPER_REGISTRATION_HXX_

//_______________________________________________
// includes

#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/registry/InvalidRegistryException.hpp>
#include <rtl/ustrbuf.hxx>
#include <cppuhelper/factory.hxx>

//_______________________________________________
// namespace

namespace comphelper{

//_______________________________________________
// declaration

//_______________________________________________

/** TODO    doc
 */
#define _COMPHELPER_COMPONENT_GETIMPLEMENTATIONENVIRONMENT                                                          \
    extern "C" void SAL_CALL component_getImplementationEnvironment(const sal_Char**        ppEnvironmentTypeName,  \
                                                                          uno_Environment** /* ppEnvironment */ )   \
    {                                                                                                               \
        *ppEnvironmentTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;                                                \
    }

//_______________________________________________

/** @short  creates a new key inside component registry.

    @descr  using: a) _COMPHELPER_COMPONENTINFO( MyClass,
                                                 MyClass::st_getImplName(),
                                                 MyClass::st_getServNames())

                   b) _COMPHELPER_COMPONENTINFO( MyClass,
                                                 ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "css.MyClass" )),
                                                 lServiceNames)

    @param  CLASS
            must be the class name of the implementation
            of an uno service, which should be registered here.

    @param  IMPLEMENTATIONNAME
            can be an uno implementation name from type [::rtl::OUString]
            directly or any possible method call, which returns such
            name.

    @param  SERVICENAME
            a list of supported uno service names from type
            [::com::sun::star::uno::Sequence< ::rtl::OUString >]
            or any possible method call, which returns such
            list.
 */
#define _COMPHELPER_COMPONENTINFO(CLASS, IMPLEMENTATIONNAME, SERVICENAMES)                                          \
    /*define new scope to prevent multiple using of the same variables ... */                                       \
    {                                                                                                               \
        /* build new key name */                                                                                    \
        ::rtl::OUStringBuffer sKeyBuf(256);                                                                         \
        sKeyBuf.appendAscii("/"               );                                                                    \
        sKeyBuf.append     (IMPLEMENTATIONNAME);                                                                    \
        sKeyBuf.appendAscii("/UNO/SERVICES"   );                                                                    \
        ::rtl::OUString sKey = sKeyBuf.makeStringAndClear();                                                        \
                                                                                                                    \
        /* try to register this service ... thrown exception will be catched by COMPONENT_WRITEINFO! */             \
        ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey > xKey = xRoot->createKey(sKey);                           \
        if (!xKey.is())                                                                                             \
            throw ::com::sun::star::registry::InvalidRegistryException(sKey, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >());     \
                                                                                                                    \
        /* dont optimize it! it must work for simple types and function calls! */                                   \
        const ::com::sun::star::uno::Sequence< ::rtl::OUString > lServiceNames = SERVICENAMES;                                   \
        const ::rtl::OUString*                      pServiceNames = lServiceNames.getConstArray();                  \
        sal_Int32                                   nCount        = lServiceNames.getLength();                      \
                                                                                                                    \
        for (sal_Int32 i=0; i<nCount; ++i)                                                                          \
            xKey->createKey(pServiceNames[i]);                                                                      \
    }

//_______________________________________________

/** @short  implments extern C function component_writeInfo

    @descr  using: _COMPHELPER_COMPONENT_WRITEINFO
                        (
                            _COMPHELPER_COMPONENTINFO(...)
                            ..
                            _COMPHELPER_COMPONENTINFO(...)
                        )

    @param  INFOLIST
            list of macros of type COMPONENTINFO without(!) any
            seperator signs between two elements.
 */
#define _COMPHELPER_COMPONENT_WRITEINFO(INFOLIST)                                                                                   \
    extern "C" sal_Bool SAL_CALL component_writeInfo(void* pServiceManager,                                                         \
                                                     void* pRegistryKey   )                                                         \
    {                                                                                                                               \
        if (!pServiceManager || !pRegistryKey)                                                                                      \
            return sal_False;                                                                                                       \
                                                                                                                                    \
        ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey > xRoot = reinterpret_cast< ::com::sun::star::registry::XRegistryKey* >(pRegistryKey);  \
                                                                                                                                    \
        /*if one of following registration will fail ... an exception is thrown! */                                                 \
        try                                                                                                                         \
        {                                                                                                                           \
            /* This parameter will expand to: */                                                                                    \
            /*   _COMPHELPER_COMPONENTINFO(1) */                                                                                    \
            /*   ...                          */                                                                                    \
            /*   _COMPHELPER_COMPONENTINFO(n) */                                                                                    \
            INFOLIST                                                                                                                \
        }                                                                                                                           \
        catch(const ::com::sun::star::registry::InvalidRegistryException&)                                                                       \
        {                                                                                                                           \
            return sal_False;                                                                                                       \
        }                                                                                                                           \
        return sal_True;                                                                                                            \
    }

//_______________________________________________

/** TODO    doc
 */
#define _COMPHELPER_MULTIINSTANCEFACTORY(IMPLEMENTATIONNAME, SERVICENAMES, FACTORYMETHOD)   \
    if (IMPLEMENTATIONNAME == sImplName)                                                    \
        xFactory = ::cppu::createSingleFactory(xSMGR             ,                          \
                                               IMPLEMENTATIONNAME,                          \
                                               FACTORYMETHOD     ,                          \
                                               SERVICENAMES      );

//_______________________________________________

/** TODO    doc
 */
#define _COMPHELPER_ONEINSTANCEFACTORY(IMPLEMENTATIONNAME, SERVICENAMES, FACTORYMETHOD)     \
    if (IMPLEMENTATIONNAME == sImplName)                                                    \
        xFactory = ::cppu::createOneInstanceFactory(xSMGR             ,                     \
                                                    IMPLEMENTATIONNAME,                     \
                                                    FACTORYMETHOD     ,                     \
                                                    SERVICENAMES      );

//_______________________________________________

/** TODO    doc
 */
#define _COMPHELPER_COMPONENT_GETFACTORY(STATIC_INIT,FACTORYLIST)                                                                                               \
    extern "C" void* SAL_CALL component_getFactory(const sal_Char* pImplementationName,                                                             \
                                                         void*     pServiceManager    ,                                                             \
                                                         void*     /* pRegistryKey */ )                                                             \
    {                                                                                                                                               \
        if (                                                                                                                                        \
            (!pImplementationName) ||                                                                                                               \
            (!pServiceManager    )                                                                                                                  \
           )                                                                                                                                        \
        return NULL;                                                                                                                                \
                                                                                                                                                    \
        STATIC_INIT                                                                                                                                 \
                                                                                                                                                    \
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  xSMGR     = reinterpret_cast< ::com::sun::star::lang::XMultiServiceFactory* >(pServiceManager);  \
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > xFactory  ;                                                                         \
        rtl::OUString                                           sImplName = ::rtl::OUString::createFromAscii(pImplementationName);                  \
                                                                                                                                                    \
        /* This parameter will expand to: */                                                                                                        \
        /*  _COMPHELPER_xxxFACTORY(1)     */                                                                                                        \
        /*  else                          */                                                                                                        \
        /*  ...                           */                                                                                                        \
        /*  else                          */                                                                                                        \
        /*  _COMPHELPER_xxxFACTORY(n)     */                                                                                                        \
        FACTORYLIST                                                                                                                                 \
                                                                                                                                                    \
        /* And if one of these checks was successfully => xFactory was set! */                                                                      \
        if (xFactory.is())                                                                                                                          \
        {                                                                                                                                           \
            xFactory->acquire();                                                                                                                    \
            return xFactory.get();                                                                                                                  \
        }                                                                                                                                           \
                                                                                                                                                    \
        return NULL;                                                                                                                                \
    }

} // namespace comphelper

#endif  //  #ifndef __COMPHELPER_REGISTRATION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
