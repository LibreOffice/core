/*************************************************************************
 *
 *  $RCSfile: facreg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2001-03-29 14:19:00 $
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

#include <string.h>

#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#include <cppuhelper/factory.hxx>
#include <uno/lbnames.h>

#ifndef _SD_UNOGRAPHICEXPORTER_HXX_
#include "UnoGraphicExporter.hxx"
#endif

using namespace rtl;
using namespace com::sun::star;
using namespace sd;

//
#ifdef __cplusplus
extern "C"
{
#endif

void SAL_CALL component_getImplementationEnvironment( const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

void SAL_CALL writeInfo( registry::XRegistryKey * pRegistryKey, const OUString& rImplementationName, const uno::Sequence< OUString >& rServices )
{
    uno::Reference< registry::XRegistryKey > xNewKey(
        pRegistryKey->createKey(
            OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) + rImplementationName + OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") ) ) );

    for( sal_Int32 i = 0; i < rServices.getLength(); i++ )
        xNewKey->createKey( rServices.getConstArray()[i]);
}

sal_Bool SAL_CALL component_writeInfo( void * pServiceManager, void * pRegistryKey )
{
    if( pRegistryKey )
    {
        try
        {
            registry::XRegistryKey *pKey = reinterpret_cast< registry::XRegistryKey * >( pRegistryKey );

            writeInfo( pKey, GraphicExporter_getImplementationName(), GraphicExporter_getSupportedServiceNames() );
        }
        catch (registry::InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_True;
}

void * SAL_CALL component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;
    if( pServiceManager )
    {
        uno::Reference< lang::XMultiServiceFactory > xMSF( reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ) );

        uno::Reference< lang::XSingleServiceFactory > xFactory;

        const sal_Int32 nImplNameLen = strlen( pImplName );
        if( GraphicExporter_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                GraphicExporter_getImplementationName(),
                GraphicExporter_createInstance,
                GraphicExporter_getSupportedServiceNames() );
        }
        if( xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }
    return pRet;
}

}
