/*************************************************************************
 *
 *  $RCSfile: manager.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pl $ $Date: 2000-11-28 10:02:58 $
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
            int nPaths = aPluginPath.GetTokenCount( ';' );
            aPaths.realloc( nPaths );
            for( int i = 0; i < nPaths; i++ )
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

Reference< ::com::sun::star::plugin::XPlugin >  XPluginManager_Impl::createPluginFromURL( const Reference< ::com::sun::star::plugin::XPluginContext > & acontext, sal_Int16 mode, const Sequence< ::rtl::OUString >& argn, const Sequence< ::rtl::OUString >& argv, const Reference< ::com::sun::star::awt::XToolkit > & toolkit, const Reference< ::com::sun::star::awt::XWindowPeer > & parent, const ::rtl::OUString& url )
{
    XPlugin_Impl* pImpl = new XPlugin_Impl( m_xSMgr );
    Reference< ::com::sun::star::plugin::XPlugin >  xRef = pImpl;   // festhalten...

    pImpl->setPluginContext( acontext );

    PluginManager::get().getPlugins().push_back( pImpl );

    int nDescr = -1;
    Sequence< ::com::sun::star::plugin::PluginDescription > aDescrs = getPluginDescriptions();
    const ::com::sun::star::plugin::PluginDescription* pDescrs = aDescrs.getConstArray();

    int nPos = url.lastIndexOf( (sal_Unicode)'.' );
    if( nPos != -1 )
    {
        ::rtl::OUString aExt = url.copy( nPos ).toLowerCase();
        for( int i = 0; i < aDescrs.getLength(); i++ )
        {
            if( pDescrs[ i ].Extension.equalsIgnoreCase( aExt ) != STRING_NOTFOUND )
            {
                nDescr = i;
                break;
            }
        }
    }

    pImpl->initInstance( (nDescr != -1) ? pDescrs[ nDescr ] : ::com::sun::star::plugin::PluginDescription(),
                         argn,
                         argv,
                         mode );

    pImpl->createPeer( toolkit, parent );

    Reference< ::com::sun::star::beans::XPropertySet >  xProperty( pImpl->getModel(), UNO_QUERY );
    if( xProperty.is() )
    {
        Any aAny;
        aAny <<= url;
        xProperty->setPropertyValue( ::rtl::OUString::createFromAscii( "URL" ), aAny );
    }

    if( ! pImpl->getPluginComm() )
    {
        pImpl->dispose();
        xRef = NULL;
    }

    return xRef;
}

