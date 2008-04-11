/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: wininetbecdef.cxx,v $
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
#include "precompiled_shell.hxx"

#include <wininetbackend.hxx>
#include <com/sun/star/registry/XRegistryKey.hpp>

#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_
#include <cppuhelper/implementationentry.hxx>
#endif
#include <com/sun/star/uno/XComponentContext.hpp>

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backend = css::configuration::backend ;

//------------------------------------------------------------------------------

static uno::Reference<uno::XInterface> SAL_CALL createWinInetBackend(
    const uno::Reference<uno::XComponentContext>& xContext){

    return * WinInetBackend::createInstance(xContext);
}

//------------------------------------------------------------------------------

static const cppu::ImplementationEntry kImplementations_entries[] =
{
    {
        createWinInetBackend,
        WinInetBackend::getBackendName,
        WinInetBackend::getBackendServiceNames,
        cppu::createSingleComponentFactory,
        NULL,
        0
    },
    { NULL }
} ;

//------------------------------------------------------------------------------

extern "C" void SAL_CALL component_getImplementationEnvironment(
    const sal_Char **aEnvTypeName, uno_Environment ** /*aEnvironment*/) {

    *aEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME ;
}

//------------------------------------------------------------------------------

extern "C" sal_Bool SAL_CALL component_writeInfo(void * /*pServiceManager*/, void *pRegistryKey) {

    using namespace ::com::sun::star::registry;
    if (pRegistryKey)
    {
        try
        {
            uno::Reference< XRegistryKey > xImplKey = static_cast< XRegistryKey* >( pRegistryKey )->createKey(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) + WinInetBackend::getBackendName()
            );

        // Register associated service names
            uno::Reference< XRegistryKey > xServicesKey = xImplKey->createKey(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES") )
            );

            uno::Sequence<rtl::OUString> sServiceNames = WinInetBackend::getBackendServiceNames();
            for (sal_Int32 i = 0 ; i < sServiceNames.getLength() ; ++ i)
                xServicesKey->createKey(sServiceNames[i]);

            // Register supported components
            uno::Reference<XRegistryKey> xComponentKey = xImplKey->createKey(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/DATA/SupportedComponents") )
            );

            xComponentKey->setAsciiListValue( WinInetBackend::getSupportedComponents() );

            return sal_True;
        }
        catch( InvalidRegistryException& )
        {
            OSL_ENSURE(sal_False, "InvalidRegistryException caught");
        }
    }
    return sal_False;
}

//------------------------------------------------------------------------------

extern "C" void *component_getFactory( const sal_Char *aImplementationName,
    void *aServiceManager,
    void *aRegistryKey) {

    return cppu::component_getFactoryHelper(
        aImplementationName,
        aServiceManager,
        aRegistryKey,
        kImplementations_entries) ;
}

//------------------------------------------------------------------------------

