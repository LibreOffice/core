/*************************************************************************
 *
 *  $RCSfile: ucbserv.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-16 14:52:48 $
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

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif

#ifndef _UCB_HXX
#include "ucb.hxx"
#endif
#ifndef _UCBCFG_HXX
#include "ucbcfg.hxx"
#endif
#ifndef _UCBSTORE_HXX
#include "ucbstore.hxx"
#endif
#ifndef _UCBPROPS_HXX
#include "ucbprops.hxx"
#endif
#ifndef _PROVPROX_HXX
#include "provprox.hxx"
#endif
#ifndef _UCB_UCBDISTRIBUTOR_HXX_
#include "ucbdistributor.hxx"
#endif

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;

//=========================================================================
static sal_Bool writeInfo( void * pRegistryKey,
                           const OUString & rImplementationName,
                              Sequence< OUString > const & rServiceNames )
{
    OUString aKeyName( OUString::createFromAscii( "/" ) );
    aKeyName += rImplementationName;
    aKeyName += OUString::createFromAscii( "/UNO/SERVICES" );

    Reference< XRegistryKey > xKey;
    try
    {
        xKey = static_cast< XRegistryKey * >(
                                    pRegistryKey )->createKey( aKeyName );
    }
    catch ( InvalidRegistryException const & )
    {
    }

    if ( !xKey.is() )
        return sal_False;

    sal_Bool bSuccess = sal_True;

    for ( sal_Int32 n = 0; n < rServiceNames.getLength(); ++n )
    {
        try
        {
            xKey->createKey( rServiceNames[ n ] );
        }
        catch ( InvalidRegistryException const & )
        {
            bSuccess = sal_False;
            break;
        }
    }
    return bSuccess;
}

//=========================================================================
extern "C" void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//=========================================================================
extern "C" sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    return pRegistryKey &&

    //////////////////////////////////////////////////////////////////////
    // Universal Content Broker.
    //////////////////////////////////////////////////////////////////////

    writeInfo( pRegistryKey,
               UniversalContentBroker::getImplementationName_Static(),
               UniversalContentBroker::getSupportedServiceNames_Static() ) &&

    //////////////////////////////////////////////////////////////////////
    // UCB Configuration.
    //////////////////////////////////////////////////////////////////////

    writeInfo( pRegistryKey,
               UcbConfigurationManager::getImplementationName_Static(),
               UcbConfigurationManager::getSupportedServiceNames_Static() ) &&

    //////////////////////////////////////////////////////////////////////
    // UCB Store.
    //////////////////////////////////////////////////////////////////////

    writeInfo( pRegistryKey,
               UcbStore::getImplementationName_Static(),
               UcbStore::getSupportedServiceNames_Static() ) &&

    //////////////////////////////////////////////////////////////////////
    // UCB PropertiesManager.
    //////////////////////////////////////////////////////////////////////

    writeInfo( pRegistryKey,
               UcbPropertiesManager::getImplementationName_Static(),
               UcbPropertiesManager::getSupportedServiceNames_Static() ) &&

    //////////////////////////////////////////////////////////////////////
    // UCP Proxy Factory.
    //////////////////////////////////////////////////////////////////////

    writeInfo( pRegistryKey,
       UcbContentProviderProxyFactory::getImplementationName_Static(),
       UcbContentProviderProxyFactory::getSupportedServiceNames_Static() ) &&

    //////////////////////////////////////////////////////////////////////
    // UCB Distributor.
    //////////////////////////////////////////////////////////////////////

    writeInfo( pRegistryKey,
               chaos_ucb::UcbDistributor::getImplementationName_Static(),
               chaos_ucb::UcbDistributor::getSupportedServiceNames_Static() );
}

//=========================================================================
extern "C" void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;

    Reference< XMultiServiceFactory > xSMgr(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ) );
    Reference< XSingleServiceFactory > xFactory;

    //////////////////////////////////////////////////////////////////////
    // Universal Content Broker.
    //////////////////////////////////////////////////////////////////////

    if ( UniversalContentBroker::getImplementationName_Static().
                compareToAscii( pImplName ) == 0 )
    {
        xFactory = UniversalContentBroker::createServiceFactory( xSMgr );
    }

    //////////////////////////////////////////////////////////////////////
    // UCB Configuration.
    //////////////////////////////////////////////////////////////////////

    else if ( UcbConfigurationManager::getImplementationName_Static().
                compareToAscii( pImplName ) == 0 )
    {
        xFactory = UcbConfigurationManager::createServiceFactory( xSMgr );
    }

    //////////////////////////////////////////////////////////////////////
    // UCB Store.
    //////////////////////////////////////////////////////////////////////

    else if ( UcbStore::getImplementationName_Static().
                compareToAscii( pImplName ) == 0 )
    {
        xFactory = UcbStore::createServiceFactory( xSMgr );
    }

    //////////////////////////////////////////////////////////////////////
    // UCB PropertiesManager.
    //////////////////////////////////////////////////////////////////////

    else if ( UcbPropertiesManager::getImplementationName_Static().
                compareToAscii( pImplName ) == 0 )
    {
        xFactory = UcbPropertiesManager::createServiceFactory( xSMgr );
    }

    //////////////////////////////////////////////////////////////////////
    // UCP Proxy Factory.
    //////////////////////////////////////////////////////////////////////

    else if ( UcbContentProviderProxyFactory::getImplementationName_Static().
                compareToAscii( pImplName ) == 0 )
    {
        xFactory
            = UcbContentProviderProxyFactory::createServiceFactory( xSMgr );
    }

    //////////////////////////////////////////////////////////////////////
    // UCB Distributor.
    //////////////////////////////////////////////////////////////////////

    else if ( chaos_ucb::UcbDistributor::getImplementationName_Static().
                compareToAscii( pImplName ) == 0 )
    {
        xFactory
            = chaos_ucb::UcbDistributor::createServiceFactory( xSMgr );
    }

    //////////////////////////////////////////////////////////////////////

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

