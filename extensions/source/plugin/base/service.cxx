/*************************************************************************
 *
 *  $RCSfile: service.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:51 $
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

#if STLPORT_VERSION>=321
#include <cstdarg>
#endif

#include <plugin/impl.hxx>
#include <tools/debug.hxx>
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XSET_HPP_
#include <com/sun/star/container/XSet.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif

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
        uno_Environment** ppEnv )
    {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    sal_Bool SAL_CALL component_writeInfo( void* pServiceManager, void* pXUnoKey )
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
        void* pXUnoKey
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
