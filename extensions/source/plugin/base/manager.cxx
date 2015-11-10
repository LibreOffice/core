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


#ifdef AIX
#define _LINUX_SOURCE_COMPAT
#include <sys/timer.h>
#undef _LINUX_SOURCE_COMPAT
#endif

#ifdef WNT
#include <prewin.h>
#include <postwin.h>
#endif

#include <cstdarg>

#include <comphelper/fileurl.hxx>
#include <comphelper/string.hxx>
#include <comphelper/processfactory.hxx>

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

#include <cppuhelper/supportsservice.hxx>

PluginManager* PluginManager::pManager = nullptr;

PluginManager& PluginManager::get()
{
    if( ! pManager )
        pManager = new PluginManager();
    return *pManager;
}

void PluginManager::setServiceFactory( const Reference< css::lang::XMultiServiceFactory >& xFactory )
{
    PluginManager& rManager = get();
    if( ! rManager.m_xSMgr.is() )
        rManager.m_xSMgr = xFactory;
}

PluginManager::PluginManager()
{
}

const Sequence< OUString >& PluginManager::getAdditionalSearchPaths()
{
    static Sequence< OUString > aPaths;

    if( ! aPaths.getLength() )
    {
        SvtPathOptions aOptions;
        OUString aPluginPath( aOptions.GetPluginPath() );
        if( !aPluginPath.isEmpty() )
        {
            sal_Int32 nPaths = comphelper::string::getTokenCount(aPluginPath, ';');
            aPaths.realloc( nPaths );
            for( sal_Int32 i = 0; i < nPaths; i++ )
                aPaths.getArray()[i] = aPluginPath.getToken(i, ';');
        }
    }

    return aPaths;
}


Reference< XInterface > SAL_CALL PluginManager_CreateInstance( const Reference< css::lang::XMultiServiceFactory >  & rSMgr ) throw( Exception )
{
    Reference< XInterface >  xService = *new XPluginManager_Impl( comphelper::getComponentContext(rSMgr) );
    return xService;
}

// css::lang::XServiceInfo
OUString XPluginManager_Impl::getImplementationName() throw(  )

{
    return getImplementationName_Static();

}

// css::lang::XServiceInfo
sal_Bool XPluginManager_Impl::supportsService(const OUString& ServiceName) throw(  )
{
    return cppu::supportsService(this, ServiceName);
}

// css::lang::XServiceInfo
Sequence< OUString > XPluginManager_Impl::getSupportedServiceNames() throw(  )
{
    return getSupportedServiceNames_Static();
}

// XPluginManager_Impl
Sequence< OUString > XPluginManager_Impl::getSupportedServiceNames_Static() throw(  )
{
    Sequence< OUString > aSNS( 1 );
    aSNS[0] = "com.sun.star.plugin.PluginManager";
    return aSNS;
}

XPluginManager_Impl::XPluginManager_Impl( const Reference< css::uno::XComponentContext >  & rxContext )
: m_xContext( rxContext )
{
    PluginManager::setServiceFactory( Reference< css::lang::XMultiServiceFactory>(rxContext->getServiceManager(), UNO_QUERY_THROW) );
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
        if( &(*iter)->getNPPInstance() == instance )
            return *iter;
    }

    return nullptr;
}

XPlugin_Impl* XPluginManager_Impl::getPluginImplementation( const Reference< css::plugin::XPlugin >& plugin )
{
    ::std::list<XPlugin_Impl*>::iterator iter;
    for( iter = PluginManager::get().getPlugins().begin();
         iter != PluginManager::get().getPlugins().end(); ++iter )
    {
        if( plugin == Reference< css::plugin::XPlugin >((*iter)) )
            return *iter;
    }

    return nullptr;
}

Sequence<css::plugin::PluginDescription> XPluginManager_Impl::getPluginDescriptions()
    throw (RuntimeException, std::exception)
{
    Sequence<css::plugin::PluginDescription> aRet;

    vcl::SettingsConfigItem* pCfg = vcl::SettingsConfigItem::get();
    OUString aVal( pCfg->getValue( "BrowserPlugins",
                                        "Disabled" ) );
    if( ! aVal.toBoolean() )
    {
        aRet = impl_getPluginDescriptions();
    }
    return aRet;
}

Reference< css::plugin::XPlugin > XPluginManager_Impl::createPlugin( const Reference< css::plugin::XPluginContext >& acontext, sal_Int16 mode, const Sequence< OUString >& argn, const Sequence< OUString >& argv, const css::plugin::PluginDescription& plugintype)
    throw( RuntimeException,css::plugin::PluginException, std::exception )
{
    XPlugin_Impl* pImpl = new XPlugin_Impl( Reference< css::lang::XMultiServiceFactory>(m_xContext->getServiceManager(), UNO_QUERY_THROW) );
    pImpl->setPluginContext( acontext );

    PluginManager::get().getPlugins().push_back( pImpl );

    pImpl->initInstance( plugintype,
                         argn,
                         argv,
                         mode );

    return pImpl;
}

Reference< css::plugin::XPlugin >  XPluginManager_Impl::createPluginFromURL( const Reference< css::plugin::XPluginContext > & acontext, sal_Int16 mode, const Sequence< OUString >& argn, const Sequence< OUString >& argv, const Reference< css::awt::XToolkit > & toolkit, const Reference< css::awt::XWindowPeer > & parent, const OUString& url ) throw (RuntimeException, std::exception)
{
    XPlugin_Impl* pImpl = new XPlugin_Impl( Reference< css::lang::XMultiServiceFactory>(m_xContext->getServiceManager(), UNO_QUERY_THROW) );
    Reference< css::plugin::XPlugin >  xRef = pImpl;

    pImpl->setPluginContext( acontext );

    PluginManager::get().getPlugins().push_back( pImpl );


    pImpl->initInstance( url,
                         argn,
                         argv,
                         mode );

    pImpl->createPeer( toolkit, parent );

    pImpl->provideNewStream( pImpl->getDescription().Mimetype,
                             Reference< css::io::XActiveDataSource >(),
                             url,
                             0, 0, comphelper::isFileUrl(url) );

    if( ! pImpl->getPluginComm() )
    {
        pImpl->dispose();
        xRef = nullptr;
    }

    return xRef;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
