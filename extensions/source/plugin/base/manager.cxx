/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: manager.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 14:50:27 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#if STLPORT_VERSION>=321
#include <cstdarg>
#endif

#include <plugin/impl.hxx>

#ifndef _OSL_MUTEX_HXX
#include <osl/mutex.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XELEMENTACCESS_HPP_
#include <com/sun/star/container/XElementAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LOADER_XIMPLEMENTATIONLOADER_HPP_
#include <com/sun/star/loader/XImplementationLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_LOADER_CANNOTACTIVATEFACTORYEXCEPTION_HPP_
#include <com/sun/star/loader/CannotActivateFactoryException.hpp>
#endif

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

PluginManager::~PluginManager()
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
            USHORT nPaths = aPluginPath.GetTokenCount( ';' );
            aPaths.realloc( nPaths );
            for( USHORT i = 0; i < nPaths; i++ )
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
    aSNS.getArray()[0] = ::rtl::OUString::createFromAscii( "com.sun.star.plugin.PluginManager" );
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

XPlugin_Impl* XPluginManager_Impl::getFirstXPlugin()
{
    if( PluginManager::get().getPlugins().begin() == PluginManager::get().getPlugins().end() )
        return NULL;

    return *PluginManager::get().getPlugins().begin();
}

Reference< ::com::sun::star::plugin::XPlugin > XPluginManager_Impl::createPlugin( const Reference< ::com::sun::star::plugin::XPluginContext >& acontext, INT16 mode, const Sequence< ::rtl::OUString >& argn, const Sequence< ::rtl::OUString >& argv, const ::com::sun::star::plugin::PluginDescription& plugintype)
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

