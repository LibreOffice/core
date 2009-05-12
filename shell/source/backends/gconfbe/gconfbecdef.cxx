/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: gconfbecdef.cxx,v $
 * $Revision: 1.11 $
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
#include "gconfbackend.hxx"
#include <com/sun/star/registry/XRegistryKey.hpp>

#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_
#include <cppuhelper/implementationentry.hxx>
#endif // _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_
#include <com/sun/star/uno/XComponentContext.hpp>
#include <rtl/ustrbuf.hxx>

#include "uno/current_context.hxx"
#include <stdio.h>
#include "orbit.h"

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backend = css::configuration::backend ;

//==============================================================================

static uno::Reference<uno::XInterface> SAL_CALL createGconfBackend(const uno::Reference<uno::XComponentContext>& xContext)
{
    try {
        uno::Reference< uno::XCurrentContext > xCurrentContext(uno::getCurrentContext());

        if (xCurrentContext.is())
        {
            uno::Any aValue = xCurrentContext->getValueByName(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "system.desktop-environment" ) ) );

            rtl::OUString aDesktopEnvironment;
            if ( (aValue >>= aDesktopEnvironment) && (aDesktopEnvironment.equalsAscii("GNOME")) )
            {
                // ORBit-2 versions < 2.8 cause a deadlock with the gtk+ VCL plugin
                if ( (orbit_major_version >= 2) && (orbit_minor_version >= 8) )
                {
                    return * GconfBackend::createInstance(xContext);
                }
            }
        }

        return uno::Reference<uno::XInterface>();

    } catch (uno::RuntimeException e) {
        return uno::Reference<uno::XInterface>();
    }

}

//==============================================================================

static const cppu::ImplementationEntry kImplementations_entries[] =
{
    {
        createGconfBackend,
        GconfBackend::getBackendName,
        GconfBackend::getBackendServiceNames,
        cppu::createSingleComponentFactory,
        NULL,
        0
    },
    { NULL, NULL, NULL, NULL, NULL, 0 }
} ;
//------------------------------------------------------------------------------

extern "C" void SAL_CALL component_getImplementationEnvironment(
                                            const sal_Char **aEnvTypeName,
                                            uno_Environment **/*aEnvironment*/) {
    *aEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME ;
}

//------------------------------------------------------------------------------

extern "C" sal_Bool SAL_CALL component_writeInfo(void */*pServiceManager*/,
                                                 void *pRegistryKey) {

    using namespace ::com::sun::star::registry;
    if (pRegistryKey)
    {
        try
        {
            uno::Reference< XRegistryKey > xImplKey = static_cast< XRegistryKey* >( pRegistryKey )->createKey(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) + GconfBackend::getBackendName()
            );

        // Register associated service names
            uno::Reference< XRegistryKey > xServicesKey = xImplKey->createKey(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES") )
            );

            uno::Sequence<rtl::OUString> sServiceNames = GconfBackend::getBackendServiceNames();
            for (sal_Int32 i = 0 ; i < sServiceNames.getLength() ; ++ i)
                xServicesKey->createKey(sServiceNames[i]);

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

extern "C" void *component_getFactory(const sal_Char *aImplementationName,
                                      void *aServiceManager,
                                      void *aRegistryKey) {

    return cppu::component_getFactoryHelper(
        aImplementationName,
        aServiceManager,
        aRegistryKey,
        kImplementations_entries) ;
}
//------------------------------------------------------------------------------
