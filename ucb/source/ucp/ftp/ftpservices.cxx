/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ftpservices.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:25:58 $
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

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
#ifndef _FTP_FTPCONTENTPROVIDER_HXX_
#include "ftpcontentprovider.hxx"
#endif

using namespace com::sun::star;
using namespace ftp;

//=========================================================================
static sal_Bool writeInfo( void * pRegistryKey,
                           const rtl::OUString & rImplementationName,
                           uno::Sequence< rtl::OUString > const & rServiceNames )
{
    rtl::OUString aKeyName( rtl::OUString::createFromAscii( "/" ) );
    aKeyName += rImplementationName;
    aKeyName += rtl::OUString::createFromAscii( "/UNO/SERVICES" );

    uno::Reference< registry::XRegistryKey > xKey;
    try
    {
        xKey = static_cast< registry::XRegistryKey * >(
            pRegistryKey )->createKey( aKeyName );
    }
    catch ( registry::InvalidRegistryException const & )
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
        catch ( registry::InvalidRegistryException const & )
        {
            bSuccess = sal_False;
            break;
        }
    }
    return bSuccess;
}

//=========================================================================
extern "C" void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//=========================================================================
extern "C" sal_Bool SAL_CALL component_writeInfo(
    void * /*pServiceManager*/, void * pRegistryKey )
{
    return pRegistryKey &&

        //////////////////////////////////////////////////////////////////////
        // FTP Content Provider.
        //////////////////////////////////////////////////////////////////////

        writeInfo( pRegistryKey,
                   FTPContentProvider::getImplementationName_Static(),
                   FTPContentProvider::getSupportedServiceNames_Static() );
}

//=========================================================================
extern "C" void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void * pRet = 0;

    uno::Reference< lang::XMultiServiceFactory > xSMgr(
        reinterpret_cast< lang::XMultiServiceFactory * >(
            pServiceManager ) );
    uno::Reference< lang::XSingleServiceFactory > xFactory;

    //////////////////////////////////////////////////////////////////////
    // FTP Content Provider.
    //////////////////////////////////////////////////////////////////////

    if ( FTPContentProvider::getImplementationName_Static().
         compareToAscii( pImplName ) == 0 )
    {
        xFactory = FTPContentProvider::createServiceFactory( xSMgr );
    }

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

