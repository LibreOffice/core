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
#include "FPServiceInfo.hxx"
#include <osl/mutex.hxx>
#include <sal/log.hxx>
#include <vcl/svapp.hxx>
#include "SalAquaFolderPicker.hxx"

#include <iostream>

#include "resourceprovider.hxx"

#include <osl/file.hxx>
#include "CFStringUtilities.hxx"
#include "NSString_OOoAdditions.hxx"
#include "NSURL_OOoAdditions.hxx"

#pragma mark DEFINES

using namespace ::com::sun::star;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

namespace
{
    // controlling event notifications
    uno::Sequence<rtl::OUString> FolderPicker_getSupportedServiceNames()
    {
        uno::Sequence<rtl::OUString> aRet(2);
        aRet[0] = "com.sun.star.ui.dialogs.SystemFolderPicker";
        aRet[1] = "com.sun.star.ui.dialogs.AquaFolderPicker";
        return aRet;
    }
}

SalAquaFolderPicker::SalAquaFolderPicker( const uno::Reference<lang::XMultiServiceFactory>& xServiceMgr ) :
    m_xServiceMgr( xServiceMgr )
{
    m_nDialogType = NAVIGATIONSERVICES_DIRECTORY;
}

// XExecutableDialog

void SAL_CALL SalAquaFolderPicker::setTitle( const rtl::OUString& aTitle )
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

void SAL_CALL SalAquaFolderPicker::setDisplayDirectory( const rtl::OUString& aDirectory )
{
    SolarMutexGuard aGuard;

    implsetDisplayDirectory(aDirectory);
}

rtl::OUString SAL_CALL SalAquaFolderPicker::getDisplayDirectory()
{
    SolarMutexGuard aGuard;

    OUString aDirectory = implgetDisplayDirectory();

    return aDirectory;
}

rtl::OUString SAL_CALL SalAquaFolderPicker::getDirectory()
{
    SolarMutexGuard aGuard;

    NSArray *files = nil;
    if (m_nDialogType == NAVIGATIONSERVICES_DIRECTORY) {
        files = [static_cast<NSOpenPanel*>(m_pDialog) URLs];
    }

    long nFiles = [files count];
    SAL_INFO("fpicker.aqua", "# of items: " << nFiles);

    if (nFiles < 1) {
        throw uno::RuntimeException("no directory selected", static_cast< cppu::OWeakObject * >( this ));
    }

    rtl::OUString aDirectory;

    NSURL *url = [files objectAtIndex:0];

    aDirectory = [url OUStringForInfo:FULLPATH];

    implsetDisplayDirectory(aDirectory);

    return aDirectory;
}

void SAL_CALL SalAquaFolderPicker::setDescription( const rtl::OUString& rDescription )
{
    [m_pDialog setMessage:[NSString stringWithOUString:rDescription]];
}

// XServiceInfo

rtl::OUString SAL_CALL SalAquaFolderPicker::getImplementationName()
{
    return FOLDER_PICKER_IMPL_NAME;
}

sal_Bool SAL_CALL SalAquaFolderPicker::supportsService( const rtl::OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

uno::Sequence<rtl::OUString> SAL_CALL SalAquaFolderPicker::getSupportedServiceNames()
{
    return FolderPicker_getSupportedServiceNames();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
