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

#include <rtl/ustring.hxx>

#include <com/sun/star/lang/XMultiComponentFactory.hpp>

#include <svl/pickerhistoryaccess.hxx>
#include <officecfg/Office/Common.hxx>

#include <vcl/svapp.hxx>

#include "fpicker.hxx"

using css::uno::Reference;
using css::uno::Sequence;

/*
 * FilePicker implementation.
 */
static OUString FilePicker_getSystemPickerServiceName()
{
#ifdef UNX
    OUString aDesktopEnvironment (Application::GetDesktopEnvironment());
    if (aDesktopEnvironment.equalsIgnoreAsciiCase("macosx"))
        return u"com.sun.star.ui.dialogs.AquaFilePicker"_ustr;
    else
        return u"com.sun.star.ui.dialogs.SystemFilePicker"_ustr;
#endif
#ifdef _WIN32
    return "com.sun.star.ui.dialogs.Win32FilePicker";
#endif
}

// Ensure that we use not the system file dialogs as headless mode relies on
// Application::EnableHeadlessMode() which only works for VCL dialogs
static bool UseSystemFileDialog()
{
    return !Application::IsHeadlessModeEnabled() && officecfg::Office::Common::Misc::UseSystemFileDialog::get();
}

Reference< css::uno::XInterface > FilePicker_CreateInstance (
    Reference< css::uno::XComponentContext > const & context)
{
    Reference< css::uno::XInterface > xResult;

    if (!context.is())
        return xResult;

    Reference< css::lang::XMultiComponentFactory > xFactory (context->getServiceManager());
    if (xFactory.is() && UseSystemFileDialog())
    {
        xResult.set( Application::createFilePicker( context ) );

        if (!xResult.is())
        {
            try
            {
                xResult = xFactory->createInstanceWithContext (
                        FilePicker_getSystemPickerServiceName(),
                        context);
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
                u"com.sun.star.ui.dialogs.OfficeFilePicker"_ustr,
                context);
    }
    if (xResult.is())
    {
        // Add to FilePicker history.
        svt::addFilePicker (xResult);
    }
    return xResult;
}

OUString FilePicker_getImplementationName()
{
    return u"com.sun.star.comp.svt.FilePicker"_ustr;
}

Sequence< OUString > FilePicker_getSupportedServiceNames()
{
    Sequence< OUString > aServiceNames { u"com.sun.star.ui.dialogs.FilePicker"_ustr };
    return aServiceNames;
}

/*
 * FolderPicker implementation.
 */
static OUString FolderPicker_getSystemPickerServiceName()
{
#ifdef UNX
    OUString aDesktopEnvironment (Application::GetDesktopEnvironment());
    if (aDesktopEnvironment.equalsIgnoreAsciiCase("macosx"))
        return u"com.sun.star.ui.dialogs.AquaFolderPicker"_ustr;
#endif
    return u"com.sun.star.ui.dialogs.SystemFolderPicker"_ustr;
}

Reference< css::uno::XInterface > FolderPicker_CreateInstance (
    Reference< css::uno::XComponentContext > const & context)
{
    Reference< css::uno::XInterface > xResult;

    if (!context.is())
        return xResult;

    Reference< css::lang::XMultiComponentFactory > xFactory (context->getServiceManager());
    if (xFactory.is() && UseSystemFileDialog())
    {
        xResult.set( Application::createFolderPicker( context ) );
        if (!xResult.is())
        {
            try
            {
                xResult = xFactory->createInstanceWithContext (
                                FolderPicker_getSystemPickerServiceName(),
                                context);
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
                 u"com.sun.star.ui.dialogs.OfficeFolderPicker"_ustr,
                context);
    }
    if (xResult.is())
    {
        // Add to FolderPicker history.
        svt::addFolderPicker (xResult);
    }
    return xResult;
}

OUString FolderPicker_getImplementationName()
{
    return u"com.sun.star.comp.svt.FolderPicker"_ustr;
}

Sequence< OUString > FolderPicker_getSupportedServiceNames()
{
    Sequence< OUString > aServiceNames { u"com.sun.star.ui.dialogs.FolderPicker"_ustr };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
