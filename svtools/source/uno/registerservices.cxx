/*************************************************************************
 *
 *  $RCSfile: registerservices.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mba $ $Date: 2000-11-22 12:02:22 $
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

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif

#include <cppuhelper/factory.hxx>

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#include <unoifac3.hxx>
#include <unoiface.hxx>

#include <pathoptions.hxx>

#include <toolkit/helper/macros.hxx>

#define IMPL_CREATEINSTANCE( ImplName ) \
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ImplName##_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& ) \
    { return ::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface >( ( ::cppu::OWeakObject* ) new ImplName ); }

// for CreateInstance functions implemented elsewhere
#define DECLARE_CREATEINSTANCE( ImplName ) \
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ImplName##_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );

// -------------------------------------------------------------------------------------
IMPL_CREATEINSTANCE( PathService )
IMPL_CREATEINSTANCE( SvtTextLoader )
IMPL_CREATEINSTANCE( ExtVCLXToolkit )
DECLARE_CREATEINSTANCE( SvNumberFormatsSupplierServiceObject )

extern "C"
{

void SAL_CALL component_getImplementationEnvironment( const sal_Char** ppEnvTypeName, uno_Environment** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

sal_Bool SAL_CALL component_writeInfo( void* _pServiceManager, void* _pRegistryKey )
{
    if (_pRegistryKey)
    {
        ::com::sun::star::registry::XRegistryKey * pRegistryKey = reinterpret_cast< ::com::sun::star::registry::XRegistryKey* >( _pRegistryKey );
        ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey > xNewKey;

        xNewKey = pRegistryKey->createKey( ::rtl::OUString::createFromAscii( "/stardiv.svtools.ExtVCLXToolkit/UNO/SERVICES" ) );
        xNewKey->createKey( ::rtl::OUString::createFromAscii( "com.sun.star.awt.ExtToolkit" ) );

        xNewKey = pRegistryKey->createKey( ::rtl::OUString::createFromAscii( "/stardiv.svtools.SvtTextLoader/UNO/SERVICES" ) );
        xNewKey->createKey( ::rtl::OUString::createFromAscii( "stardiv.one.frame.FrameLoader" ) );
        xNewKey->createKey( ::rtl::OUString::createFromAscii( "stardiv.one.frame.TextLoader" ) );
        xNewKey = pRegistryKey->createKey( ::rtl::OUString::createFromAscii( "/stardiv.svtools.SvtTextLoader/UNO/Loader" ) );
        ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey >  xLoaderKey = pRegistryKey->createKey( ::rtl::OUString::createFromAscii( "/stardiv.svtools.SvtTextLoader/Loader" ) );
        xNewKey = xLoaderKey->createKey( ::rtl::OUString::createFromAscii( "Pattern" ) );
        xNewKey->setAsciiValue( ::rtl::OUString::createFromAscii( ".component:Text*" ) );

        xNewKey = pRegistryKey->createKey( ::rtl::OUString::createFromAscii( "/com.sun.star.uno.util.numbers.SvNumberFormatsSupplierServiceObject/UNO/SERVICES" ) );
        xNewKey->createKey( ::rtl::OUString::createFromAscii( "com.sun.star.util.NumberFormatsSupplier" ) );

        xNewKey = pRegistryKey->createKey( ::rtl::OUString::createFromAscii( "/com.sun.star.comp.svtools.PathService/UNO/SERVICES" ) );
        xNewKey->createKey( ::rtl::OUString::createFromAscii( "com.sun.star.config.SpecialConfigManager" ) );

// !!!
/*
        ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey >  xKey;
        uno2smart(xKey, *xUnoKey);

        ::rtl::OUString aImpl( L"/" );
        aImpl += SvUnoAttributeContainer::getStaticImplementationName();
        aImpl += L"/UNO/SERVICES";
        ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey >  xNewKey = xKey->createKey( aImpl );
        ::com::sun::star::uno::Sequence <::rtl::OUString> aSequ = SvUnoAttributeContainer::getStaticSupportedServiceNames();
        const ::rtl::OUString* pArray = aSequ.getConstArray();
        for( sal_Int32 i = 0; i < aSequ.getLength(); i++ )
            xNewKey->createKey( pArray[i] );
*/

    }
    return sal_True;
}

void* SAL_CALL component_getFactory( const sal_Char* sImplementationName, void* _pServiceManager, void* _pRegistryKey )
{
    void* pRet = NULL;

    if ( _pServiceManager )
    {
        ::com::sun::star::lang::XMultiServiceFactory* pServiceManager = reinterpret_cast< ::com::sun::star::lang::XMultiServiceFactory* >( _pServiceManager );
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > xFactory;

        if ( rtl_str_compare( sImplementationName, "stardiv.svtools.SvtTextLoader") == 0 )
        {
            ::com::sun::star::uno::Sequence< ::rtl::OUString > aServiceNames(2);
            aServiceNames.getArray()[0] = ::rtl::OUString::createFromAscii( "stardiv.one.frame.FrameLoader" );
            aServiceNames.getArray()[1] = ::rtl::OUString::createFromAscii( "stardiv.one.frame.TextLoader" );
            xFactory = ::cppu::createSingleFactory( pServiceManager, ::rtl::OUString::createFromAscii( sImplementationName ), SvtTextLoader_CreateInstance, aServiceNames );
        }
        else if ( rtl_str_compare( sImplementationName, "stardiv.svtools.ExtVCLXToolkit" ) == 0 )
        {
            ::com::sun::star::uno::Sequence< ::rtl::OUString > aServiceNames(1);
            aServiceNames.getArray()[0] = ::rtl::OUString::createFromAscii( "com.sun.star.awt.ExtToolkit" );
            xFactory = ::cppu::createSingleFactory( pServiceManager, ::rtl::OUString::createFromAscii( sImplementationName ), ExtVCLXToolkit_CreateInstance, aServiceNames );
        }
        if ( rtl_str_compare( sImplementationName, "com.sun.star.uno.util.numbers.SvNumberFormatsSupplierServiceObject") == 0 )
        {
            ::com::sun::star::uno::Sequence< ::rtl::OUString > aServiceNames(1);
            aServiceNames.getArray()[0] = ::rtl::OUString::createFromAscii( "com.sun.star.util.NumberFormatsSupplier" );
            xFactory = ::cppu::createSingleFactory( pServiceManager, ::rtl::OUString::createFromAscii( sImplementationName ), SvNumberFormatsSupplierServiceObject_CreateInstance, aServiceNames );
        }
        else if ( rtl_str_compare( sImplementationName, "com.sun.star.comp.svtools.PathService" ) == 0 )
        {
            ::com::sun::star::uno::Sequence< ::rtl::OUString > aServiceNames(1);
            aServiceNames.getArray()[0] = ::rtl::OUString::createFromAscii( "com.sun.star.config.SpecialConfigManager" );
            xFactory = ::cppu::createSingleFactory( pServiceManager, ::rtl::OUString::createFromAscii( sImplementationName ), PathService_CreateInstance, aServiceNames );
        }

        if ( xFactory.is() )
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }
    return pRet;
}

}   // "C"

