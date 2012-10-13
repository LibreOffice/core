/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "sal/types.h"
#include "rtl/ustring.hxx"

#include "cppuhelper/implementationentry.hxx"
#include "com/sun/star/lang/XMultiComponentFactory.hpp"

#include "svtools/miscopt.hxx"
#include "svl/pickerhistoryaccess.hxx"

#include "vcl/svapp.hxx"

#ifdef WNT
#define GradientStyle_RECT BLA_GradientStyle_RECT
#include <windows.h>
#undef GradientStyle_RECT
#include <odma_lib.hxx>
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
#ifdef UNX
    OUString aDesktopEnvironment (Application::GetDesktopEnvironment());
    if (aDesktopEnvironment.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("tde")))
        return OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.ui.dialogs.TDEFilePicker"));
    else if (aDesktopEnvironment.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("kde")))
        return OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.ui.dialogs.KDEFilePicker"));
    else if (aDesktopEnvironment.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("kde4")))
        return OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.ui.dialogs.KDE4FilePicker"));
    else if (aDesktopEnvironment.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("macosx")))
        return OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.ui.dialogs.AquaFilePicker"));
    else
        return OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.ui.dialogs.SystemFilePicker"));
#endif
#ifdef WNT
    if (SvtMiscOptions().TryODMADialog() && ::odma::DMSsAvailable()) {
        return OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.ui.dialogs.ODMAFilePicker"));
    }
    return OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.ui.dialogs.Win32FilePicker"));
#endif
}

static Reference< css::uno::XInterface > FilePicker_createInstance (
    Reference< css::uno::XComponentContext > const & rxContext)
{
    Reference< css::uno::XInterface > xResult;

    if (!rxContext.is())
        return xResult;

    Reference< css::lang::XMultiComponentFactory > xFactory (rxContext->getServiceManager());
    if (xFactory.is() && SvtMiscOptions().UseSystemFileDialog())
    {
        xResult = Reference< css::uno::XInterface >( Application::createFilePicker( rxContext ) );

        if (!xResult.is())
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
    }


    if (!xResult.is() && xFactory.is())
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
#ifdef UNX
    if (aDesktopEnvironment.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("tde")))
        return OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.ui.dialogs.TDEFolderPicker"));
    else if (aDesktopEnvironment.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("kde")))
        return OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.ui.dialogs.KDEFolderPicker"));
    else if (aDesktopEnvironment.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("macosx")))
        return OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.ui.dialogs.AquaFolderPicker"));
#endif
#ifdef WNT
    if (SvtMiscOptions().TryODMADialog() && ::odma::DMSsAvailable()) {
        return OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.ui.dialogs.ODMAFolderPicker"));
    }
#endif
    return OUString (RTL_CONSTASCII_USTRINGPARAM ("com.sun.star.ui.dialogs.SystemFolderPicker"));
}

static Reference< css::uno::XInterface > FolderPicker_createInstance (
    Reference< css::uno::XComponentContext > const & rxContext)
{
    Reference< css::uno::XInterface > xResult;

    if (!rxContext.is())
        return xResult;

    Reference< css::lang::XMultiComponentFactory > xFactory (rxContext->getServiceManager());
    if (xFactory.is() && SvtMiscOptions().UseSystemFileDialog())
    {
        xResult = Reference< css::uno::XInterface >( Application::createFolderPicker( rxContext ) );
        if (!xResult.is())
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
    }
    if (!xResult.is() && xFactory.is() )
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
SAL_DLLPUBLIC_EXPORT void * SAL_CALL fpicker_component_getFactory (
    const sal_Char * pImplementationName, void * pServiceManager, void * pRegistryKey)
{
    return cppu::component_getFactoryHelper (
        pImplementationName, pServiceManager, pRegistryKey, g_entries);
}

} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
