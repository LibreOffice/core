/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* $RCSfile: code,v $
*
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
************************************************************************/

#include "precompiled_configmgr.hxx"
#include "sal/config.h"

#include "com/sun/star/registry/XRegistryKey.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "cppuhelper/implementationentry.hxx"
#include "osl/diagnose.h"
#include "uno/lbnames.h"
#include "rtl/textenc.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "configurationprovider.hxx"
#include "defaultprovider.hxx"

namespace {

namespace css = com::sun::star;

css::uno::Reference< css::uno::XInterface > SAL_CALL dummy(
    css::uno::Reference< css::uno::XComponentContext > const &)
    SAL_THROW((css::uno::Exception))
{
    OSL_ASSERT(false);
    return css::uno::Reference< css::uno::XInterface >();
}

static cppu::ImplementationEntry const services[] = {
    { &dummy, &configmgr::configuration_provider::getImplementationName,
      &configmgr::configuration_provider::getSupportedServiceNames,
      &configmgr::configuration_provider::createFactory, 0, 0 },
     { &dummy, &configmgr::default_provider::getImplementationName,
       &configmgr::default_provider::getSupportedServiceNames,
       &configmgr::default_provider::createFactory, 0, 0 },
    { 0, 0, 0, 0, 0, 0 }
};

}

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    char const * pImplName, void * pServiceManager, void * pRegistryKey)
{
    return cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey, services);
}

extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL
component_getImplementationEnvironment(
    char const ** ppEnvTypeName, uno_Environment **)
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey)
{
    if (!component_writeInfoHelper(pServiceManager, pRegistryKey, services)) {
        return false;
    }
    try {
        css::uno::Reference< css::registry::XRegistryKey >(
            (css::uno::Reference< css::registry::XRegistryKey >(
                static_cast< css::registry::XRegistryKey * >(pRegistryKey))->
             createKey(
                 rtl::OUString(
                     RTL_CONSTASCII_USTRINGPARAM(
                         "/com.sun.star.comp.configuration.DefaultProvider/UNO/"
                         "SINGLETONS/"
                         "com.sun.star.configuration.theDefaultProvider")))),
            css::uno::UNO_SET_THROW)->
            setStringValue(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "com.sun.star.configuration.DefaultProvider")));
    } catch (css::uno::Exception & e) {
        (void) e;
        OSL_TRACE(
            "configmgr component_writeInfo exception: %s",
            rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr());
        return false;
    }
    return true;
}

/*
com.sun.star.configuration.AdministrationProvider
com.sun.star.configuration.ConfigurationRegistry
com.sun.star.configuration.backend.Backend
com.sun.star.configuration.backend.Backend
com.sun.star.configuration.backend.Backend
com.sun.star.configuration.backend.BackendAdapter
com.sun.star.configuration.backend.CopyImporter
com.sun.star.configuration.backend.DefaultBackend
com.sun.star.configuration.backend.LayerUpdateMerger
com.sun.star.configuration.backend.LocalHierarchyBrowser
com.sun.star.configuration.backend.LocalHierarchyBrowser
com.sun.star.configuration.backend.LocalMultiStratum
com.sun.star.configuration.backend.LocalSchemaSupplier
com.sun.star.configuration.backend.LocalSingleBackend
com.sun.star.configuration.backend.LocalSingleStratum
com.sun.star.configuration.backend.LocalSingleStratum
com.sun.star.configuration.backend.LocalSingleStratum
com.sun.star.configuration.backend.LocalSingleStratum
com.sun.star.configuration.backend.MergeImporter
com.sun.star.configuration.backend.OnlineBackend
com.sun.star.configuration.backend.xml.LayerParser
com.sun.star.configuration.backend.xml.LayerWriter
com.sun.star.configuration.backend.xml.SchemaParser
com.sun.star.configuration.bootstrap.BootstrapContext
*/
