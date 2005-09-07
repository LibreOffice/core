/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: localebecdef.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:48:44 $
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

#include <localebackend.hxx>

#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif

#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_
#include <cppuhelper/implementationentry.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backend = css::configuration::backend ;

//------------------------------------------------------------------------------

static uno::Reference<uno::XInterface> SAL_CALL createLocaleBackend(
    const uno::Reference<uno::XComponentContext>& xContext){

    return * LocaleBackend::createInstance(xContext);
}

//------------------------------------------------------------------------------

static const cppu::ImplementationEntry kImplementations_entries[] =
{
    {
        createLocaleBackend,
        LocaleBackend::getBackendName,
        LocaleBackend::getBackendServiceNames,
        cppu::createSingleComponentFactory,
        NULL,
        0
    },
    { NULL }
} ;

//------------------------------------------------------------------------------

extern "C" void SAL_CALL component_getImplementationEnvironment(
    const sal_Char **aEnvTypeName, uno_Environment **aEnvironment) {

    *aEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME ;
}

//------------------------------------------------------------------------------

extern "C" sal_Bool SAL_CALL component_writeInfo(void *pServiceManager, void *pRegistryKey) {

    using namespace ::com::sun::star::registry;
    if (pRegistryKey)
    {
        try
        {
            uno::Reference< XRegistryKey > xImplKey = static_cast< XRegistryKey* >( pRegistryKey )->createKey(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) + LocaleBackend::getBackendName()
            );

        // Register associated service names
            uno::Reference< XRegistryKey > xServicesKey = xImplKey->createKey(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES") )
            );

            uno::Sequence<rtl::OUString> sServiceNames = LocaleBackend::getBackendServiceNames();
            for (sal_Int32 i = 0 ; i < sServiceNames.getLength() ; ++ i)
                xServicesKey->createKey(sServiceNames[i]);

            // Register supported components
            uno::Reference<XRegistryKey> xComponentKey = xImplKey->createKey(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/DATA/SupportedComponents") )
            );

            xComponentKey->setAsciiListValue( LocaleBackend::getSupportedComponents() );

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

