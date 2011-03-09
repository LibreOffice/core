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

#include "plugin/impl.hxx"

#include "osl/mutex.hxx"
#include "unotools/pathoptions.hxx"
#include "vcl/configsettings.hxx"

#include "com/sun/star/container/XEnumerationAccess.hpp"
#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/container/XEnumeration.hpp"
#include "com/sun/star/container/XElementAccess.hpp"
#include "com/sun/star/container/XIndexAccess.hpp"
#include "com/sun/star/loader/XImplementationLoader.hpp"
#include "com/sun/star/loader/CannotActivateFactoryException.hpp"

PluginManager* PluginManager::pManager = NULL;

PluginManager& PluginManager::get()
{
    if( ! pManager )
        pManager = new PluginManager();
    return *pManager;
}

void PluginManager::setServiceFactory( const Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory )
{
    PluginManager& rManager = get();
    if( ! rManager.m_xSMgr.is() )
        rManager.m_xSMgr = xFactory;
}

PluginManager::PluginManager()
{
}

const Sequence< ::rtl::OUString >& PluginManager::getAdditionalSearchPaths()
{
    static Sequence< ::rtl::OUString > aPaths;

    if( ! aPaths.getLength() )
    {
        SvtPathOptions aOptions;
        String aPluginPath( aOptions.GetPluginPath() );
        if( aPluginPath.Len() )
        {
            sal_uInt16 nPaths = aPluginPath.GetTokenCount( ';' );
            aPaths.realloc( nPaths );
            for( sal_uInt16 i = 0; i < nPaths; i++ )
                aPaths.getArray()[i] = aPluginPath.GetToken( i, ';' );
        }
    }

    return aPaths;
}

//==================================================================================================
Reference< XInterface > SAL_CALL PluginManager_CreateInstance( const Reference< ::com::sun::star::lang::XMultiServiceFactory >  & rSMgr ) throw( Exception )
{
    Reference< XInterface >  xService = *new XPluginManager_Impl( rSMgr );
    return xService;
}

// ::com::sun::star::lang::XServiceInfo
::rtl::OUString XPluginManager_Impl::getImplementationName() throw(  )

{
    return getImplementationName_Static();

}

// ::com::sun::star::lang::XServiceInfo
sal_Bool XPluginManager_Impl::supportsService(const ::rtl::OUString& ServiceName) throw(  )
{
    Sequence< ::rtl::OUString > aSNL = getSupportedServiceNames();
    const ::rtl::OUString * pArray = aSNL.getConstArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;
    return sal_False;
}

// ::com::sun::star::lang::XServiceInfo
Sequence< ::rtl::OUString > XPluginManager_Impl::getSupportedServiceNames(void) throw(  )
{
    return getSupportedServiceNames_Static();
}

// XPluginManager_Impl
Sequence< ::rtl::OUString > XPluginManager_Impl::getSupportedServiceNames_Static(void) throw(  )
{
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS.getArray()[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.plugin.PluginManager"));
    return aSNS;
}

XPluginManager_Impl::XPluginManager_Impl( const Reference< ::com::sun::star::lang::XMultiServiceFactory >  & rSMgr )
: m_xSMgr( rSMgr )
{
    PluginManager::setServiceFactory( rSMgr );
}

XPluginManager_Impl::~XPluginManager_Impl()
{
}

XPlugin_Impl* XPluginManager_Impl::getXPluginFromNPP( NPP instance )
{
    ::std::list<XPlugin_Impl*>::iterator iter;
    for( iter = PluginManager::get().getPlugins().begin();
         iter != PluginManager::get().getPlugins().end(); ++iter )
    {
        if( (*iter)->getNPPInstance() == instance )
            return *iter;
    }

    return NULL;
}

XPlugin_Impl* XPluginManager_Impl::getPluginImplementation( const Reference< ::com::sun::star::plugin::XPlugin >& plugin )
{
    ::std::list<XPlugin_Impl*>::iterator iter;
    for( iter = PluginManager::get().getPlugins().begin();
         iter != PluginManager::get().getPlugins().end(); ++iter )
    {
        if( plugin == Reference< ::com::sun::star::plugin::XPlugin >((*iter)) )
            return *iter;
    }

    return NULL;
}

Sequence<com::sun::star::plugin::PluginDescription> XPluginManager_Impl::getPluginDescriptions() throw()
{
    Sequence<com::sun::star::plugin::PluginDescription> aRet;

    vcl::SettingsConfigItem* pCfg = vcl::SettingsConfigItem::get();
    rtl::OUString aVal( pCfg->getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "BrowserPlugins" ) ),
                                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Disabled" ) ) ) );
    if( ! aVal.toBoolean() )
    {
        aRet = impl_getPluginDescriptions();
    }
    return aRet;
}

Reference< ::com::sun::star::plugin::XPlugin > XPluginManager_Impl::createPlugin( const Reference< ::com::sun::star::plugin::XPluginContext >& acontext, sal_Int16 mode, const Sequence< ::rtl::OUString >& argn, const Sequence< ::rtl::OUString >& argv, const ::com::sun::star::plugin::PluginDescription& plugintype)
    throw( RuntimeException,::com::sun::star::plugin::PluginException )
{
    XPlugin_Impl* pImpl = new XPlugin_Impl( m_xSMgr );
    pImpl->setPluginContext( acontext );

    PluginManager::get().getPlugins().push_back( pImpl );

    pImpl->initInstance( plugintype,
                         argn,
                         argv,
                         mode );

    return pImpl;
}

Reference< ::com::sun::star::plugin::XPlugin >  XPluginManager_Impl::createPluginFromURL( const Reference< ::com::sun::star::plugin::XPluginContext > & acontext, sal_Int16 mode, const Sequence< ::rtl::OUString >& argn, const Sequence< ::rtl::OUString >& argv, const Reference< ::com::sun::star::awt::XToolkit > & toolkit, const Reference< ::com::sun::star::awt::XWindowPeer > & parent, const ::rtl::OUString& url ) throw()
{
    XPlugin_Impl* pImpl = new XPlugin_Impl( m_xSMgr );
    Reference< ::com::sun::star::plugin::XPlugin >  xRef = pImpl;

    pImpl->setPluginContext( acontext );

    PluginManager::get().getPlugins().push_back( pImpl );


    pImpl->initInstance( url,
                         argn,
                         argv,
                         mode );

    pImpl->createPeer( toolkit, parent );

    pImpl->provideNewStream( pImpl->getDescription().Mimetype,
                             Reference< com::sun::star::io::XActiveDataSource >(),
                             url,
                             0, 0, (sal_Bool)(url.compareToAscii( "file:", 5  ) == 0) );

    if( ! pImpl->getPluginComm() )
    {
        pImpl->dispose();
        xRef = NULL;
    }

    return xRef;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
