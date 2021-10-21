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

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/supportsservice.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <osl/mutex.hxx>
#include <sal/log.hxx>
#include <vcl/svapp.hxx>
#include "SalAquaFolderPicker.hxx"

#include <iostream>

#include "resourceprovider.hxx"

#include <osl/file.hxx>
#include "NSString_OOoAdditions.hxx"
#include "NSURL_OOoAdditions.hxx"

#pragma mark DEFINES

using namespace ::com::sun::star;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

SalAquaFolderPicker::SalAquaFolderPicker()
{
    m_nDialogType = NAVIGATIONSERVICES_DIRECTORY;
}

// XExecutableDialog

void SAL_CALL SalAquaFolderPicker::setTitle( const OUString& aTitle )
{
    SolarMutexGuard aGuard;

    implsetTitle(aTitle);
}

sal_Int16 SAL_CALL SalAquaFolderPicker::execute()
{
    SolarMutexGuard aGuard;

    sal_Int16 retVal = 0;

    int nResult = runandwaitforresult();

    switch( nResult )
    {
    case NSModalResponseOK:
        retVal = ExecutableDialogResults::OK;
        break;

    case NSModalResponseCancel:
        retVal = ExecutableDialogResults::CANCEL;
        break;

    default:
        throw uno::RuntimeException("The dialog returned with an unknown result!", static_cast< cppu::OWeakObject * >( this ));
        break;
    }

    return retVal;
}

// XFolderPicker

void SAL_CALL SalAquaFolderPicker::setDisplayDirectory( const OUString& aDirectory )
{
    SolarMutexGuard aGuard;

    implsetDisplayDirectory(aDirectory);
}

OUString SAL_CALL SalAquaFolderPicker::getDisplayDirectory()
{
    SolarMutexGuard aGuard;

    OUString aDirectory = implgetDisplayDirectory();

    return aDirectory;
}

OUString SAL_CALL SalAquaFolderPicker::getDirectory()
{
    SolarMutexGuard aGuard;

    NSArray *files = nil;
    if (m_nDialogType == NAVIGATIONSERVICES_DIRECTORY) {
        files = [static_cast<NSOpenPanel*>(m_pDialog) URLs];
    }

    NSUInteger nFiles = [files count];
    SAL_INFO("fpicker.aqua", "# of items: " << nFiles);

    if (nFiles < 1) {
        throw uno::RuntimeException("no directory selected", static_cast< cppu::OWeakObject * >( this ));
    }

    OUString aDirectory;

    NSURL *url = [files objectAtIndex:0];

    aDirectory = [url OUString];

    implsetDisplayDirectory(aDirectory);

    return aDirectory;
}

void SAL_CALL SalAquaFolderPicker::setDescription( const OUString& rDescription )
{
    [m_pDialog setMessage:[NSString stringWithOUString:rDescription]];
}

// XServiceInfo

OUString SAL_CALL SalAquaFolderPicker::getImplementationName()
{
    return "com.sun.star.ui.dialogs.SalAquaFolderPicker";
}

sal_Bool SAL_CALL SalAquaFolderPicker::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

uno::Sequence<OUString> SAL_CALL SalAquaFolderPicker::getSupportedServiceNames()
{
    return { "com.sun.star.ui.dialogs.SystemFolderPicker", "com.sun.star.ui.dialogs.AquaFolderPicker" };
}

// XCancellable

void SAL_CALL SalAquaFolderPicker::cancel()
{
    SolarMutexGuard aGuard;

    [m_pDialog cancel:nil];
}

// XEventListener

void SAL_CALL SalAquaFolderPicker::disposing( const lang::EventObject& )
{
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
fpicker_SalAquaFolderPicker_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new SalAquaFolderPicker());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
