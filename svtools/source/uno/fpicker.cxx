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

#include <sal/types.h>
#include <rtl/ustring.hxx>

#include <com/sun/star/lang/XMultiComponentFactory.hpp>

#include <svtools/miscopt.hxx>
#include <svl/pickerhistoryaccess.hxx>

#include <vcl/svapp.hxx>

#include <cassert>

#include "fpicker.hxx"

using css::uno::Reference;
using css::uno::Sequence;

/*
 * FilePicker implementation.
 */
Reference< css::uno::XInterface > FilePicker_CreateInstance (
    Reference< css::uno::XComponentContext > const & context)
{
    Reference< css::uno::XInterface > xResult;

    if (!context.is())
        return xResult;

    Reference< css::lang::XMultiComponentFactory > xFactory (context->getServiceManager());
    if (xFactory.is() && SvtMiscOptions().UseSystemFileDialog())
    {
        // calls the SalInstance's createFilePicker function
        xResult.set( Application::createFilePicker( context ) );

        // Windows and MacOS have extra SystemFilePicker services in /fpicker/
        if (!xResult.is())
        {
            try
            {
                xResult = xFactory->createInstanceWithContext (
                    OUStringLiteral("com.sun.star.ui.dialogs.SystemFilePicker"), context);
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
            OUStringLiteral("com.sun.star.ui.dialogs.OfficeFilePicker"), context);
    }

    assert(xResult.is());
    if (xResult.is())
    {
        // Add to FilePicker history.
        svt::addFilePicker (xResult);
    }
    return xResult;
}

OUString FilePicker_getImplementationName()
{
    return OUStringLiteral("com.sun.star.comp.svt.FilePicker");
}

Sequence< OUString > FilePicker_getSupportedServiceNames()
{
    Sequence< OUString > aServiceNames { "com.sun.star.ui.dialogs.FilePicker" };
    return aServiceNames;
}

/*
 * FolderPicker implementation.
 */
Reference< css::uno::XInterface > FolderPicker_CreateInstance (
    Reference< css::uno::XComponentContext > const & context)
{
    Reference< css::uno::XInterface > xResult;

    if (!context.is())
        return xResult;

    Reference< css::lang::XMultiComponentFactory > xFactory (context->getServiceManager());
    if (xFactory.is() && SvtMiscOptions().UseSystemFileDialog())
    {
        // calls the SalInstance's createFolderPicker function
        xResult.set( Application::createFolderPicker( context ) );

        // Windows and MacOS have extra SystemFolderPicker services in /fpicker/
        if (!xResult.is())
        {
            try
            {
                xResult = xFactory->createInstanceWithContext (
                    OUStringLiteral("com.sun.star.ui.dialogs.SystemFolderPicker"), context);
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
            OUStringLiteral("com.sun.star.ui.dialogs.OfficeFolderPicker"), context);
    }

    assert(xResult.is());
    if (xResult.is())
    {
        // Add to FolderPicker history.
        svt::addFolderPicker (xResult);
    }
    return xResult;
}

OUString FolderPicker_getImplementationName()
{
    return OUStringLiteral("com.sun.star.comp.svt.FolderPicker");
}

Sequence< OUString > FolderPicker_getSupportedServiceNames()
{
    Sequence< OUString > aServiceNames { "com.sun.star.ui.dialogs.FolderPicker" };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
