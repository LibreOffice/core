/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: desktopbecdef.cxx,v $
 * $Revision: 1.4 $
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

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XSCHEMASUPPLIER_HPP_
#include <com/sun/star/configuration/backend/XSingleLayerStratum.hpp>
#endif
#include <com/sun/star/registry/XRegistryKey.hpp>

#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_
#include <cppuhelper/implementationentry.hxx>
#endif
#include <com/sun/star/uno/XComponentContext.hpp>

#include "uno/current_context.hxx"

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backend = css::configuration::backend ;

//==============================================================================

static uno::Reference<uno::XInterface> SAL_CALL createDesktopBackend(const uno::Reference<uno::XComponentContext>& xContext)
{
    try {
        uno::Reference< uno::XCurrentContext > xCurrentContext(uno::getCurrentContext());

        if (xCurrentContext.is())
        {
            uno::Any aValue = xCurrentContext->getValueByName(
                rtl::OUString::createFromAscii( "system.desktop-environment" ) );

            rtl::OUString aDesktopEnvironment;
            if ( aValue >>= aDesktopEnvironment )
            {
                rtl::OUString aDesktopService;
                if ( aDesktopEnvironment.equalsIgnoreAsciiCaseAscii( "gnome" ) )
                    aDesktopService = rtl::OUString::createFromAscii( "com.sun.star.configuration.backend.GconfBackend" );
                else if ( aDesktopEnvironment.equalsIgnoreAsciiCaseAscii( "kde" ) )
                    aDesktopService = rtl::OUString::createFromAscii( "com.sun.star.configuration.backend.KDEBackend" );
                else
                    return uno::Reference<uno::XInterface>();

                uno::Reference< lang::XMultiComponentFactory > xServiceManager = xContext->getServiceManager();
                if( xServiceManager.is() )
                {
                    return uno::Reference< backend::XSingleLayerStratum >::query(
                            xServiceManager->createInstanceWithContext( aDesktopService, xContext) );
                }
            }
        }
    } catch (uno::RuntimeException e) {
    }

    return uno::Reference<uno::XInterface>();
}

static rtl::OUString SAL_CALL getBackendName(void) {
    return rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.configuration.backend.DesktopBackend") );
}

static uno::Sequence<rtl::OUString> SAL_CALL getBackendServiceNames(void)
{
    uno::Sequence<rtl::OUString> aServices(2) ;
    aServices[0] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.backend.DesktopBackend")) ;
    aServices[1] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.backend.PlatformBackend")) ;

    return aServices ;
}

static uno::Sequence<rtl::OUString> SAL_CALL getSupportedComponents(void)
{
    const sal_Int32 nComponents = 7;

    uno::Sequence<rtl::OUString> aSupportedComponentsList(nComponents) ;

    aSupportedComponentsList[0] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("org.openoffice.VCL")) ;
    aSupportedComponentsList[1] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Inet")) ;
    aSupportedComponentsList[2] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Office.Common")) ;
    aSupportedComponentsList[3] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Office.Paths")) ;

    aSupportedComponentsList[4] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("org.openoffice.UserProfile")) ;
    aSupportedComponentsList[5] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Office.Recovery")) ;
    aSupportedComponentsList[6] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Setup")) ;

    return aSupportedComponentsList ;
}
//==============================================================================

static const cppu::ImplementationEntry kImplementations_entries[] =
{
    {
        createDesktopBackend,
        getBackendName,
        getBackendServiceNames,
        cppu::createSingleComponentFactory,
        NULL,
        0
    },
    { NULL, NULL, NULL, NULL, NULL, 0 }
} ;
//------------------------------------------------------------------------------

extern "C" void SAL_CALL component_getImplementationEnvironment(
                                            const sal_Char **aEnvTypeName,
                                            uno_Environment ** /* aEnvironment */)
{
    *aEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME ;
}

//------------------------------------------------------------------------------

extern "C" sal_Bool SAL_CALL component_writeInfo(void * /* pServiceManager */,
                                                 void *pRegistryKey)
{
    using namespace ::com::sun::star::registry;
    if (pRegistryKey)
    {
        try
        {
            uno::Reference< XRegistryKey > xImplKey = static_cast< XRegistryKey* >( pRegistryKey )->createKey(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) + getBackendName()
            );

            // Register associated service names
            uno::Reference< XRegistryKey > xServicesKey = xImplKey->createKey(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES") )
            );

            uno::Sequence<rtl::OUString> sServiceNames = getBackendServiceNames();
            for (sal_Int32 i = 0 ; i < sServiceNames.getLength() ; ++ i)
                xServicesKey->createKey(sServiceNames[i]);

            // Register supported components
            uno::Reference<XRegistryKey> xComponentKey = xImplKey->createKey(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/DATA/SupportedComponents") )
            );

            xComponentKey->setAsciiListValue( getSupportedComponents() );

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
                                      void *aRegistryKey)
{
    return cppu::component_getFactoryHelper(
        aImplementationName,
        aServiceManager,
        aRegistryKey,
        kImplementations_entries) ;
}
//------------------------------------------------------------------------------
