/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: registerservices.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:29:45 $
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
#include "precompiled_svtools.hxx"

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

#ifndef _RTL_USTRING_HXX_
#include "rtl/ustring.hxx"
#endif

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif

namespace css = com::sun::star;
using css::uno::Reference;
using css::uno::Sequence;
using rtl::OUString;

// -------------------------------------------------------------------------------------

#define DECLARE_CREATEINSTANCE( ImplName ) \
    Reference< css::uno::XInterface > SAL_CALL ImplName##_CreateInstance( const Reference< css::lang::XMultiServiceFactory >& );

DECLARE_CREATEINSTANCE( SvNumberFormatterServiceObj )
DECLARE_CREATEINSTANCE( SvNumberFormatsSupplierServiceObject )

// -------------------------------------------------------------------------------------

extern "C"
{

SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment (
    const sal_Char ** ppEnvTypeName, uno_Environment ** /* ppEnv */)
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo (
    void * /* _pServiceManager */, void * _pRegistryKey)
{
    if (_pRegistryKey)
    {
        Reference< css::registry::XRegistryKey > xRegistryKey (
            reinterpret_cast< css::registry::XRegistryKey* >(_pRegistryKey));
        Reference< css::registry::XRegistryKey > xNewKey;

        xNewKey = xRegistryKey->createKey (
            OUString::createFromAscii(
                "/com.sun.star.uno.util.numbers.SvNumberFormatsSupplierServiceObject/UNO/SERVICES" ) );
        xNewKey->createKey (
            OUString::createFromAscii( "com.sun.star.util.NumberFormatsSupplier" ) );

        xNewKey = xRegistryKey->createKey (
            OUString::createFromAscii(
                "/com.sun.star.uno.util.numbers.SvNumberFormatterServiceObject/UNO/SERVICES" ) );
        xNewKey->createKey (
            OUString::createFromAscii( "com.sun.star.util.NumberFormatter" ) );

        return sal_True;
    }
    return sal_False;
}

SAL_DLLPUBLIC_EXPORT void* SAL_CALL component_getFactory (
    const sal_Char * pImplementationName, void * _pServiceManager, void * /* _pRegistryKey*/)
{
    void * pResult = 0;
    if ( _pServiceManager )
    {
        Reference< css::lang::XSingleServiceFactory > xFactory;
        if (rtl_str_compare(
                pImplementationName,
                "com.sun.star.uno.util.numbers.SvNumberFormatsSupplierServiceObject") == 0)
        {
            Sequence< OUString > aServiceNames(1);
            aServiceNames.getArray()[0] =
                OUString::createFromAscii( "com.sun.star.util.NumberFormatsSupplier" );

            xFactory = ::cppu::createSingleFactory(
                reinterpret_cast< css::lang::XMultiServiceFactory* >(_pServiceManager),
                OUString::createFromAscii( pImplementationName ),
                SvNumberFormatsSupplierServiceObject_CreateInstance,
                aServiceNames);
        }
        else if (rtl_str_compare(
                     pImplementationName,
                     "com.sun.star.uno.util.numbers.SvNumberFormatterServiceObject") == 0)
        {
            Sequence< OUString > aServiceNames(1);
            aServiceNames.getArray()[0] =
                OUString::createFromAscii( "com.sun.star.util.NumberFormatter" );

            xFactory = ::cppu::createSingleFactory(
                reinterpret_cast< css::lang::XMultiServiceFactory* >(_pServiceManager),
                OUString::createFromAscii( pImplementationName ),
                SvNumberFormatterServiceObj_CreateInstance,
                aServiceNames);
        }
        if ( xFactory.is() )
        {
            xFactory->acquire();
            pResult = xFactory.get();
        }
    }
    return pResult;
}

}   // "C"

