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

#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
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
    extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL component_getFactory(const sal_Char* pImplementationName,                                        \
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
