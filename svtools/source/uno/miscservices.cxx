/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: miscservices.cxx,v $
 * $Revision: 1.6 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"
#include "sal/types.h"
#include "rtl/ustring.hxx"
#include <cppuhelper/factory.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::lang;

using rtl::OUString;

// -------------------------------------------------------------------------------------

// for CreateInstance functions implemented elsewhere
#define DECLARE_CREATEINSTANCE( ImplName ) \
    Reference< XInterface > SAL_CALL ImplName##_CreateInstance( const Reference< XMultiServiceFactory >& );

// for CreateInstance functions implemented elsewhere, while the function is within a namespace
#define DECLARE_CREATEINSTANCE_NAMESPACE( nmspe, ImplName ) \
    namespace nmspe {   \
        Reference< XInterface > SAL_CALL ImplName##_CreateInstance( const Reference< XMultiServiceFactory >& ); \
    }

// -------------------------------------------------------------------------------------

DECLARE_CREATEINSTANCE_NAMESPACE( svt, OAddressBookSourceDialogUno )
DECLARE_CREATEINSTANCE( SvFilterOptionsDialog )
DECLARE_CREATEINSTANCE( PathService )

// -------------------------------------------------------------------------------------

extern "C"
{

SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment (
    const sal_Char ** ppEnvTypeName, uno_Environment ** /* ppEnv */)
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo (
    void * /* _pServiceManager */, void * _pRegistryKey )
{
    if (_pRegistryKey)
    {
        Reference< XRegistryKey > xRegistryKey (
            reinterpret_cast< XRegistryKey* >( _pRegistryKey ));
        Reference< XRegistryKey > xNewKey;

        xNewKey = xRegistryKey->createKey (
            OUString::createFromAscii( "/com.sun.star.comp.svtools.OAddressBookSourceDialogUno/UNO/SERVICES" ) );
        xNewKey->createKey(
            OUString::createFromAscii( "com.sun.star.ui.AddressBookSourceDialog" ) );

        xNewKey = xRegistryKey->createKey (
            OUString::createFromAscii( "/com.sun.star.svtools.SvFilterOptionsDialog/UNO/SERVICES" ) );
        xNewKey->createKey (
            OUString::createFromAscii( "com.sun.star.ui.dialogs.FilterOptionsDialog" ) );

        return sal_True;
    }
    return sal_False;
}

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory (
    const sal_Char * pImplementationName, void * _pServiceManager, void * /* _pRegistryKey */)
{
    void * pResult = 0;
    if ( _pServiceManager )
    {
        Reference< XSingleServiceFactory > xFactory;
        if (rtl_str_compare (
                pImplementationName, "com.sun.star.comp.svtools.OAddressBookSourceDialogUno") == 0)
        {
            Sequence< OUString > aServiceNames(1);
            aServiceNames.getArray()[0] =
                OUString::createFromAscii( "com.sun.star.ui.AddressBookSourceDialog" );

            xFactory = ::cppu::createSingleFactory (
                reinterpret_cast< XMultiServiceFactory* >( _pServiceManager ),
                OUString::createFromAscii( pImplementationName ),
                svt::OAddressBookSourceDialogUno_CreateInstance,
                aServiceNames);
        }
        else if (rtl_str_compare (
                     pImplementationName, "com.sun.star.svtools.SvFilterOptionsDialog") == 0)
        {
            Sequence< OUString > aServiceNames(1);
            aServiceNames.getArray()[0] =
                OUString::createFromAscii( "com.sun.star.ui.dialogs.FilterOptionsDialog" );

            xFactory = ::cppu::createSingleFactory (
                reinterpret_cast< XMultiServiceFactory* >( _pServiceManager ),
                OUString::createFromAscii( pImplementationName ),
                SvFilterOptionsDialog_CreateInstance,
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

