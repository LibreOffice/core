/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gconfbecdef.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:47:01 $
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

#ifndef GCONFBACKEND_HXX_
#include "gconfbackend.hxx"
#endif // GCONFBACKEND_HXX_

#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif

#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_
#include <cppuhelper/implementationentry.hxx>
#endif // _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif // _RTL_USTRBUF_HXX_

#include "uno/current_context.hxx"
#include <stdio.h>
#include <orbit/orbit.h>

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
    { NULL }
} ;
//------------------------------------------------------------------------------

extern "C" void SAL_CALL component_getImplementationEnvironment(
                                            const sal_Char **aEnvTypeName,
                                            uno_Environment **aEnvironment) {
    *aEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME ;
}

//------------------------------------------------------------------------------

extern "C" sal_Bool SAL_CALL component_writeInfo(void *pServiceManager,
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

            // Register supported components
            uno::Reference<XRegistryKey> xComponentKey = xImplKey->createKey(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/DATA/SupportedComponents") )
            );

            xComponentKey->setAsciiListValue( GconfBackend::getSupportedComponents() );

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
