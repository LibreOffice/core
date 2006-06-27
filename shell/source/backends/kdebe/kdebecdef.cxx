/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: kdebecdef.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2006-06-27 11:51:58 $
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

#ifndef KDEBACKEND_HXX_
#include "kdebackend.hxx"
#endif

#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif

#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_
#include <cppuhelper/implementationentry.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#include <kapplication.h>

#include "uno/current_context.hxx"

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backend = css::configuration::backend ;

//==============================================================================

static uno::Reference<uno::XInterface> SAL_CALL createKDEBackend(const uno::Reference<uno::XComponentContext>& xContext)
{
    try {
        uno::Reference< uno::XCurrentContext > xCurrentContext(uno::getCurrentContext());

        if (xCurrentContext.is())
        {
            uno::Any aValue = xCurrentContext->getValueByName(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "system.desktop-environment" ) ) );

            rtl::OUString aDesktopEnvironment;
            if ( (aValue >>= aDesktopEnvironment) && (aDesktopEnvironment.equalsAscii("KDE")) && (KApplication::kApplication() != NULL) )
                return * KDEBackend::createInstance(xContext);
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
        createKDEBackend,
        KDEBackend::getBackendName,
        KDEBackend::getBackendServiceNames,
        cppu::createSingleComponentFactory,
        NULL,
        0
    },
    { NULL, NULL, NULL, NULL, NULL, 0 }
} ;
//------------------------------------------------------------------------------

extern "C" void SAL_CALL component_getImplementationEnvironment(
                                            const sal_Char **aEnvTypeName,
                                            uno_Environment **) {
    *aEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME ;
}

//------------------------------------------------------------------------------

extern "C" sal_Bool SAL_CALL component_writeInfo(void *,
                                                 void *pRegistryKey) {

    using namespace ::com::sun::star::registry;
    if (pRegistryKey)
    {
        try
        {
            uno::Reference< XRegistryKey > xImplKey = static_cast< XRegistryKey* >( pRegistryKey )->createKey(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) + KDEBackend::getBackendName()
            );

        // Register associated service names
            uno::Reference< XRegistryKey > xServicesKey = xImplKey->createKey(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES") )
            );

            uno::Sequence<rtl::OUString> sServiceNames = KDEBackend::getBackendServiceNames();
            for (sal_Int32 i = 0 ; i < sServiceNames.getLength() ; ++ i)
                xServicesKey->createKey(sServiceNames[i]);

            // Register supported components
            uno::Reference<XRegistryKey> xComponentKey = xImplKey->createKey(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/DATA/SupportedComponents") )
            );

            xComponentKey->setAsciiListValue( KDEBackend::getSupportedComponents() );

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
