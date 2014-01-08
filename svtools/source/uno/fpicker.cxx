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

#include "com/sun/star/lang/XMultiComponentFactory.hpp"

#include "svtools/miscopt.hxx"
#include "svl/pickerhistoryaccess.hxx"

#include "vcl/svapp.hxx"

using css::uno::Reference;
using css::uno::Sequence;

/*
 * FilePicker implementation.
 */
static OUString FilePicker_getSystemPickerServiceName()
{
#ifdef UNX
    OUString aDesktopEnvironment (Application::GetDesktopEnvironment());
    if (aDesktopEnvironment.equalsIgnoreAsciiCase("tde"))
        return OUString ("com.sun.star.ui.dialogs.TDEFilePicker");
    else if (aDesktopEnvironment.equalsIgnoreAsciiCase("kde"))
        return OUString ("com.sun.star.ui.dialogs.KDEFilePicker");
    else if (aDesktopEnvironment.equalsIgnoreAsciiCase("kde4"))
        return OUString ("com.sun.star.ui.dialogs.KDE4FilePicker");
    else if (aDesktopEnvironment.equalsIgnoreAsciiCase("macosx"))
        return OUString ("com.sun.star.ui.dialogs.AquaFilePicker");
    else
        return OUString ("com.sun.star.ui.dialogs.SystemFilePicker");
#endif
#ifdef WNT
    return OUString ("com.sun.star.ui.dialogs.Win32FilePicker");
#endif
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_svt_FilePicker_get_implementation(
        css::uno::XComponentContext *context, uno_Sequence * arguments)
{
    assert(arguments != 0 && arguments->nElements == 0); (void) arguments;
    Reference< css::uno::XInterface > xResult;
    Reference< css::lang::XMultiComponentFactory > xFactory (context->getServiceManager());
    if (xFactory.is() && SvtMiscOptions().UseSystemFileDialog())
    {
        xResult = Reference< css::uno::XInterface >( Application::createFilePicker( context ) );

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
                OUString( "com.sun.star.ui.dialogs.OfficeFilePicker"),
                context);
    }
    if (xResult.is())
    {
        // Add to FilePicker history.
        svt::addFilePicker (xResult);
    }
    xResult->acquire();
    return xResult.get();
}

/*
 * FolderPicker implementation.
 */
static OUString FolderPicker_getSystemPickerServiceName()
{
    OUString aDesktopEnvironment (Application::GetDesktopEnvironment());
#ifdef UNX
    if (aDesktopEnvironment.equalsIgnoreAsciiCase("tde"))
        return OUString("com.sun.star.ui.dialogs.TDEFolderPicker");
    else if (aDesktopEnvironment.equalsIgnoreAsciiCase("kde"))
        return OUString("com.sun.star.ui.dialogs.KDEFolderPicker");
    else if (aDesktopEnvironment.equalsIgnoreAsciiCase("macosx"))
        return OUString("com.sun.star.ui.dialogs.AquaFolderPicker");
#endif
    return OUString("com.sun.star.ui.dialogs.SystemFolderPicker");
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_svt_FolderPicker_get_implementation(
        css::uno::XComponentContext *context, uno_Sequence * arguments)
{
    assert(arguments != 0 && arguments->nElements == 0); (void) arguments;
    Reference< css::uno::XInterface > xResult;
    Reference< css::lang::XMultiComponentFactory > xFactory (context->getServiceManager());
    if (xFactory.is() && SvtMiscOptions().UseSystemFileDialog())
    {
        xResult = Reference< css::uno::XInterface >( Application::createFolderPicker( context ) );
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
                OUString( "com.sun.star.ui.dialogs.OfficeFolderPicker"),
                context);
    }
    if (xResult.is())
    {
        // Add to FolderPicker history.
        svt::addFolderPicker (xResult);
    }
    xResult->acquire();
    return xResult.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
