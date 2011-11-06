/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_fpicker.hxx"
#include "sal/types.h"
#include "rtl/ustring.hxx"

#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_
#include "cppuhelper/implementationentry.hxx"
#endif
#include "com/sun/star/lang/XMultiComponentFactory.hpp"
#include "svtools/miscopt.hxx"
#include "svl/pickerhistoryaccess.hxx"

#ifndef _SV_APP_HXX
#include "vcl/svapp.hxx"
#endif

namespace css = com::sun::star;

using css::uno::Reference;
using css::uno::Sequence;
using rtl::OUString;

/*
 * FilePicker implementation.
 */
static OUString FilePicker_getSystemPickerServiceName()
{
    OUString aDesktopEnvironment (Application::GetDesktopEnvironment());
    if (aDesktopEnvironment.equalsIgnoreAsciiCaseAscii ("gnome"))
        return OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.ui.dialogs.GtkFilePicker"));
    else if (aDesktopEnvironment.equalsIgnoreAsciiCaseAscii ("kde"))
        return OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.ui.dialogs.KDEFilePicker"));
    else if (aDesktopEnvironment.equalsIgnoreAsciiCaseAscii ("kde4"))
        return OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.ui.dialogs.KDE4FilePicker"));
    else if (aDesktopEnvironment.equalsIgnoreAsciiCaseAscii ("macosx"))
        return OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.ui.dialogs.AquaFilePicker"));
    else
        return OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.ui.dialogs.SystemFilePicker"));
}

static Reference< css::uno::XInterface > FilePicker_createInstance (
    Reference< css::uno::XComponentContext > const & rxContext)
{
    Reference< css::uno::XInterface > xResult;
    if (rxContext.is())
    {
        Reference< css::lang::XMultiComponentFactory > xFactory (rxContext->getServiceManager());
        if (xFactory.is())
        {
            if (SvtMiscOptions().UseSystemFileDialog())
            {
                try
                {
                    xResult = xFactory->createInstanceWithContext (
                        FilePicker_getSystemPickerServiceName(),
                        rxContext);
                }
                catch (css::uno::Exception const &)
                {
                    // Handled below (see @ fallback).
                }
            }
            if (!xResult.is())
            {
                // Always fall back to OfficeFilePicker.
                xResult = xFactory->createInstanceWithContext (
                    OUString (RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.dialogs.OfficeFilePicker")),
                    rxContext);
            }
            if (xResult.is())
            {
                // Add to FilePicker history.
                svt::addFilePicker (xResult);
            }
        }
    }
    return xResult;
}

static OUString FilePicker_getImplementationName()
{
    return OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.comp.fpicker.FilePicker"));
}

static Sequence< OUString > FilePicker_getSupportedServiceNames()
{
    Sequence< OUString > aServiceNames(1);
    aServiceNames.getArray()[0] =
        OUString (RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.dialogs.FilePicker"));
    return aServiceNames;
}

/*
 * FolderPicker implementation.
 */
static OUString FolderPicker_getSystemPickerServiceName()
{
    OUString aDesktopEnvironment (Application::GetDesktopEnvironment());
    if (aDesktopEnvironment.equalsIgnoreAsciiCaseAscii ("gnome"))
        return OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.ui.dialogs.GtkFolderPicker"));
    else if (aDesktopEnvironment.equalsIgnoreAsciiCaseAscii ("kde"))
        return OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.ui.dialogs.KDEFolderPicker"));
    else if (aDesktopEnvironment.equalsIgnoreAsciiCaseAscii ("macosx"))
        return OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.ui.dialogs.AquaFolderPicker"));
    else
        return OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.ui.dialogs.SystemFolderPicker"));
}

static Reference< css::uno::XInterface > FolderPicker_createInstance (
    Reference< css::uno::XComponentContext > const & rxContext)
{
    Reference< css::uno::XInterface > xResult;
    if (rxContext.is())
    {
        Reference< css::lang::XMultiComponentFactory > xFactory (rxContext->getServiceManager());
        if (xFactory.is())
        {
            if (SvtMiscOptions().UseSystemFileDialog())
            {
                try
                {
                    xResult = xFactory->createInstanceWithContext (
                        FolderPicker_getSystemPickerServiceName(),
                        rxContext);
                }
                catch (css::uno::Exception const &)
                {
                    // Handled below (see @ fallback).
                }
            }
            if (!xResult.is())
            {
                // Always fall back to OfficeFolderPicker.
                xResult = xFactory->createInstanceWithContext (
                    OUString (RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.dialogs.OfficeFolderPicker")),
                    rxContext);
            }
            if (xResult.is())
            {
                // Add to FolderPicker history.
                svt::addFolderPicker (xResult);
            }
        }
    }
    return xResult;
}

static OUString FolderPicker_getImplementationName()
{
    return OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.comp.fpicker.FolderPicker"));
}

static Sequence< OUString > FolderPicker_getSupportedServiceNames()
{
    Sequence< OUString > aServiceNames(1);
    aServiceNames.getArray()[0] =
        OUString (RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.dialogs.FolderPicker"));
    return aServiceNames;
}

/*
 * Implementation entries.
 */
static cppu::ImplementationEntry g_entries[] =
{
    {
        FilePicker_createInstance,
        FilePicker_getImplementationName,
        FilePicker_getSupportedServiceNames,
        cppu::createSingleComponentFactory, 0, 0
    },
    {
        FolderPicker_createInstance,
        FolderPicker_getImplementationName,
        FolderPicker_getSupportedServiceNames,
        cppu::createSingleComponentFactory, 0, 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

/*
 * Public (exported) interface.
 */
extern "C"
{
SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment (
    const sal_Char ** ppEnvTypeName, uno_Environment ** /* ppEnv */)
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory (
    const sal_Char * pImplementationName, void * pServiceManager, void * pRegistryKey)
{
    return cppu::component_getFactoryHelper (
        pImplementationName, pServiceManager, pRegistryKey, g_entries);
}

} // extern "C"
