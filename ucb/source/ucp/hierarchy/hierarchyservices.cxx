/*************************************************************************
 *
 *  $RCSfile: hierarchyservices.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-16 14:54:18 $
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

#ifndef _HIERARCHYPROVIDER_HXX
#include "hierarchyprovider.hxx"
#endif

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace hierarchy_ucp;

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
    // Hierarchy Content Provider.
    //////////////////////////////////////////////////////////////////////

    writeInfo( pRegistryKey,
               HierarchyContentProvider::getImplementationName_Static(),
               HierarchyContentProvider::getSupportedServiceNames_Static() );
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
    // Hierarchy Content Provider.
    //////////////////////////////////////////////////////////////////////

    if ( HierarchyContentProvider::getImplementationName_Static().
                compareToAscii( pImplName ) == 0 )
    {
        xFactory = HierarchyContentProvider::createServiceFactory( xSMgr );
    }

    //////////////////////////////////////////////////////////////////////

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

