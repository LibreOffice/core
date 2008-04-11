/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: service.cxx,v $
 * $Revision: 1.5 $
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

#if STLPORT_VERSION>=321
#include <cstdarg>
#endif

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

//==================================================================================================
void registerPluginService( const Reference< ::com::sun::star::lang::XMultiServiceFactory > & xMgr )
{
    if (! xMgr.is())
        return;

    Reference< ::com::sun::star::container::XSet >  xReg( xMgr, UNO_QUERY );
    if(xReg.is())
    {
        Any aAny;
        Reference< ::com::sun::star::lang::XSingleServiceFactory >  xF;
        xF = createSingleFactory( xMgr, PluginModel::getImplementationName_Static(),

                                        PluginModel_CreateInstance,
                                        PluginModel::getSupportedServiceNames_Static() );
        aAny <<= xF;
        xReg->insert( aAny );
        xF = createSingleFactory( xMgr, XPluginManager_Impl::getImplementationName_Static(),

                                        PluginManager_CreateInstance,
                                        XPluginManager_Impl::getSupportedServiceNames_Static() );
        aAny <<= xF;
        xReg->insert( aAny );
    }
}

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

                ::rtl::OUString aImplName = ::rtl::OUString::createFromAscii( "/" );
                aImplName += XPluginManager_Impl::getImplementationName_Static();
                aImplName += ::rtl::OUString::createFromAscii( "/UNO/SERVICES/com.sun.star.plugin.PluginManager" );
                xKey->createKey( aImplName );

                aImplName = ::rtl::OUString::createFromAscii( "/" );
                aImplName += PluginModel::getImplementationName_Static();
                aImplName += ::rtl::OUString::createFromAscii( "/UNO/SERVICES/com.sun.star.plugin.PluginModel" );
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

/** special registration procedure for the staroffice player */
    void SAL_CALL exService_getSmartFactory( const sal_Char* implementationName, const Reference< ::com::sun::star::lang::XMultiServiceFactory >  & rSMgr, Reference< ::com::sun::star::lang::XSingleServiceFactory > & xFactory )
    {
        ::rtl::OUString                 aImplementationName( ::rtl::OUString::createFromAscii(implementationName));

        if (aImplementationName == PluginModel::getImplementationName_Static() )

        {
            xFactory = createSingleFactory( rSMgr, aImplementationName,
                                            PluginModel_CreateInstance,
                                            PluginModel::getSupportedServiceNames_Static() );
        }
        else if (aImplementationName == XPluginManager_Impl::getImplementationName_Static() )

        {
            xFactory = createSingleFactory( rSMgr, aImplementationName,
                                            PluginManager_CreateInstance,
                                            XPluginManager_Impl::getSupportedServiceNames_Static() );
        }
    }
} /* extern "C" */
