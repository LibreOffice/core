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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#ifdef AIX
#define _LINUX_SOURCE_COMPAT
#include <sys/timer.h>
#undef _LINUX_SOURCE_COMPAT
#endif

#include <cstdarg>

#include <plugin/impl.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

#include <uno/dispatcher.h> // declaration of generic uno interface
#include <uno/mapping.hxx> // mapping stuff

#include <cppuhelper/factory.hxx>

using namespace cppu;

extern "C" {
    void SAL_CALL component_getImplementationEnvironment(
        const sal_Char** ppEnvTypeName,
        uno_Environment** /*ppEnv*/ )
    {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    sal_Bool SAL_CALL component_writeInfo( void* /*pServiceManager*/, void* pXUnoKey )
    {
        if( pXUnoKey )
        {
            try
            {
                Reference< ::com::sun::star::registry::XRegistryKey > xKey( reinterpret_cast< ::com::sun::star::registry::XRegistryKey* >( pXUnoKey ) );

                ::rtl::OUString aImplName (RTL_CONSTASCII_USTRINGPARAM("/"));
                aImplName += XPluginManager_Impl::getImplementationName_Static();
                aImplName += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES/com.sun.star.plugin.PluginManager"));
                xKey->createKey( aImplName );

                aImplName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
                aImplName += PluginModel::getImplementationName_Static();
                aImplName += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES/com.sun.star.plugin.PluginModel"));
                xKey->createKey( aImplName );

                return sal_True;
            }
            catch( ::com::sun::star::registry::InvalidRegistryException& )
            {
            }
        }
        return sal_False;
    }

    void* SAL_CALL component_getFactory(
        const sal_Char* pImplementationName,
        void* pXUnoSMgr,
        void* /*pXUnoKey*/
        )
    {
        void* pRet = 0;

        ::rtl::OUString aImplName( ::rtl::OUString::createFromAscii( pImplementationName ) );

        if( pXUnoSMgr )
        {
            Reference< ::com::sun::star::lang::XMultiServiceFactory > xMgr(
                reinterpret_cast< ::com::sun::star::lang::XMultiServiceFactory* >( pXUnoSMgr )
                );
            Reference< ::com::sun::star::lang::XSingleServiceFactory > xFactory;
            if( aImplName.equals( XPluginManager_Impl::getImplementationName_Static() ) )
            {
                xFactory = ::cppu::createSingleFactory(
                    xMgr, aImplName, PluginManager_CreateInstance,
                    XPluginManager_Impl::getSupportedServiceNames_Static() );
            }
            else if( aImplName.equals( PluginModel::getImplementationName_Static() ) )
            {
                xFactory = ::cppu::createSingleFactory(
                    xMgr, aImplName, PluginModel_CreateInstance,
                    PluginModel::getSupportedServiceNames_Static() );
            }
            if( xFactory.is() )
            {
                xFactory->acquire();
                pRet = xFactory.get();
            }
        }
        return pRet;
    }
} /* extern "C" */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
