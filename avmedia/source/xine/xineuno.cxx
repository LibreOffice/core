/*************************************************************************
 *
 *  $RCSfile: xineuno.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 16:50:46 $
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

#include "xinecommon.hxx"
#include "manager.hxx"

using namespace ::com::sun::star;

// -------------------
// - factory methods -
// -------------------

static uno::Reference< uno::XInterface > SAL_CALL create_MediaPlayer( const uno::Reference< lang::XMultiServiceFactory >& rxFact )
{
    return uno::Reference< uno::XInterface >( *new ::avmedia::xine::Manager( rxFact ) );
}

// ------------------------------------------
// - component_getImplementationEnvironment -
// ------------------------------------------

extern "C" void SAL_CALL component_getImplementationEnvironment( const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

// -----------------------
// - component_writeInfo -
// -----------------------

extern "C" sal_Bool SAL_CALL component_writeInfo( void* pServiceManager, void* pRegistryKey )
{
    sal_Bool bRet = sal_False;

    if( pRegistryKey )
    {
        try
        {
            uno::Reference< registry::XRegistryKey > xNewKey1(
                static_cast< registry::XRegistryKey* >( pRegistryKey )->createKey(
                ::rtl::OUString::createFromAscii(
                    "/" AVMEDIA_XINE_MANAGER_IMPLEMENTATIONNAME "/UNO/SERVICES/"
                    AVMEDIA_XINE_MANAGER_SERVICENAME ) ) );

            bRet = sal_True;
        }
        catch( registry::InvalidRegistryException& )
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }

    return bRet;
}

// ------------------------
// - component_getFactory -
// ------------------------

extern "C" void* SAL_CALL component_getFactory( const sal_Char* pImplName, void* pServiceManager, void* pRegistryKey )
{
    uno::Reference< lang::XSingleServiceFactory > xFactory;
    void*                                   pRet = 0;

    if( rtl_str_compare( pImplName, AVMEDIA_XINE_MANAGER_IMPLEMENTATIONNAME ) == 0 )
    {
        const ::rtl::OUString aServiceName( ::rtl::OUString::createFromAscii( AVMEDIA_XINE_MANAGER_SERVICENAME ) );

        xFactory = uno::Reference< lang::XSingleServiceFactory >( ::cppu::createSingleFactory(
                        reinterpret_cast< lang::XMultiServiceFactory* >( pServiceManager ),
                        ::rtl::OUString::createFromAscii( AVMEDIA_XINE_MANAGER_IMPLEMENTATIONNAME ),
                        create_MediaPlayer, uno::Sequence< ::rtl::OUString >( &aServiceName, 1 ) ) );
    }

    if( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}
