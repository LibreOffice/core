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

#include "sal/config.h"

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/interfacecontainer.h>
#include <osl/diagnose.h>
#include <FPServiceInfo.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include "SalAquaPicker.hxx"
#include <osl/file.hxx>
#include "CFStringUtilities.hxx"
#include "NSString_OOoAdditions.hxx"

#include "NSURL_OOoAdditions.hxx"

#include "SalAquaFilePicker.hxx"

#include <stdio.h>

#pragma mark DEFINES
#define kSetHideExtensionStateKey @"NSNavLastUserSetHideExtensionButtonState"

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

SalAquaPicker::SalAquaPicker()
: m_pDialog(nullptr)
, m_pControlHelper(new ControlHelper())
{
}

SalAquaPicker::~SalAquaPicker()
{
    SolarMutexGuard aGuard;

    NSAutoreleasePool *pool = [NSAutoreleasePool new];

    if (nullptr != m_pControlHelper)
        delete m_pControlHelper;

    if (nullptr != m_pDialog)
        [m_pDialog release];

    [pool release];
}

void SAL_CALL SalAquaPicker::implInitialize()
{
    SolarMutexGuard aGuard;

    if (m_pDialog != nil) {
        return;
    }

    switch (m_nDialogType)
    {
        case NAVIGATIONSERVICES_OPEN:
            m_pDialog = [NSOpenPanel openPanel];
            [(NSOpenPanel*)m_pDialog setCanChooseDirectories:NO];
            [(NSOpenPanel*)m_pDialog setCanChooseFiles:YES];
            break;

        case NAVIGATIONSERVICES_SAVE:
            m_pDialog = [NSSavePanel savePanel];
            [(NSSavePanel*)m_pDialog setCanSelectHiddenExtension:NO]; //changed for issue #102102
            /* I would have loved to use
             * [(NSSavePanel*)m_pDialog setExtensionHidden:YES];
             * here but unfortunately this
             * a) only works when the dialog is already displayed because it seems to act on the corresponding checkbox (that we don't show but that doesn't matter)
             * b) Mac OS X saves this setting on an application-based level which means that the last state is always being restored again when the app runs for the next time
             *
             * So the only reliable way seems to be using the NSUserDefaults object because that is where that value is stored and
             * to just overwrite it if it has the wrong value.
             */
            {
                NSUserDefaults *pDefaults = [NSUserDefaults standardUserDefaults];
                NSNumber *pExtn = [pDefaults objectForKey:kSetHideExtensionStateKey];
                if(pExtn == nil || [pExtn boolValue] == NO) {
                    [pDefaults setBool:YES forKey:kSetHideExtensionStateKey];
                }
            }
            break;

        case NAVIGATIONSERVICES_DIRECTORY:
            m_pDialog = [NSOpenPanel openPanel];
            [(NSOpenPanel*)m_pDialog setCanChooseDirectories:YES];
            [(NSOpenPanel*)m_pDialog setCanChooseFiles:NO];
            break;

        default:
            break;
    }

    if (m_pDialog != nil) {
        [(NSOpenPanel*)m_pDialog setCanCreateDirectories:YES];
        //Retain the dialog instance or it will go away immediately
        [m_pDialog retain];
    }
}

int SalAquaPicker::run()
{
    SolarMutexGuard aGuard;

    NSAutoreleasePool *pool = [NSAutoreleasePool new];

    if (m_pDialog == nullptr) {
        //this is the case e.g. for the folder picker at this stage
        implInitialize();
    }

    NSView *userPane = m_pControlHelper->getUserPane();
    if (userPane != nullptr) {
        [m_pDialog setAccessoryView:userPane];
    }

    int retVal = 0;

#if MACOSX_SDK_VERSION < 1060
    NSString *startDirectory;
    if (m_sDisplayDirectory.getLength() > 0) {
        NSString *temp = [NSString stringWithOUString:m_sDisplayDirectory];
        NSURL *url = [NSURL URLWithString:temp];
        startDirectory = [url path];

        OSL_TRACE("start dir: %s", [startDirectory UTF8String]);
    }
    else {
        startDirectory = NSHomeDirectory();
    }
#else
    NSURL *startDirectory;
    if (m_sDisplayDirectory.getLength() > 0) {
        NSString *temp = [NSString stringWithOUString:m_sDisplayDirectory];
        startDirectory = [NSURL URLWithString:temp];

        SAL_INFO("fpicker.aqua", "start dir: " << [startDirectory path]);
    }
    else {
        startDirectory = [NSURL fileURLWithPath:NSHomeDirectory() isDirectory:YES];
    }
#endif

    switch(m_nDialogType) {
        case NAVIGATIONSERVICES_DIRECTORY:
        case NAVIGATIONSERVICES_OPEN:
#if MACOSX_SDK_VERSION < 1060
            retVal = [(NSOpenPanel*)m_pDialog runModalForDirectory:startDirectory file:nil types:nil];
#else
            [m_pDialog setDirectoryURL:startDirectory];
            retVal = [(NSOpenPanel*)m_pDialog runModal];
#endif
            break;
        case NAVIGATIONSERVICES_SAVE:
#if MACOSX_SDK_VERSION < 1060
            retVal = [m_pDialog runModalForDirectory:startDirectory file:[NSString stringWithOUString:((SalAquaFilePicker*)this)->getSaveFileName()]];
#else
            [m_pDialog setDirectoryURL:startDirectory];
            [m_pDialog setNameFieldStringValue:[NSString stringWithOUString:static_cast<SalAquaFilePicker*>(this)->getSaveFileName()]];
            retVal = [m_pDialog runModal];
#endif
            break;
        // [m_pDialog beginSheetForDirectory:startDirectory file:[m_pDialog saveFilename] modalForWindow:[NSApp keyWindow] modalDelegate:((SalAquaFilePicker*)this)->getDelegate() didEndSelector:@selector(savePanelDidEnd:returnCode:contextInfo:) contextInfo:nil];
        default:
            break;
    }

    if (retVal == NSFileHandlingPanelOKButton) {
#if MACOSX_SDK_VERSION < 1060
        NSString* pDir = [m_pDialog directory];
        if (pDir) {
            implsetDisplayDirectory([[NSURL fileURLWithPath:pDir] OUStringForInfo:FULLPATH]);
        }
#else
        NSURL* pDir = [m_pDialog directoryURL];
        if (pDir) {
            implsetDisplayDirectory([pDir OUStringForInfo:FULLPATH]);
        }
#endif
    }

    [pool release];

    return retVal;
}

int SalAquaPicker::runandwaitforresult()
{
    SolarMutexGuard aGuard;

    int status = this->run();

    return status;
}

void SAL_CALL SalAquaPicker::implsetDisplayDirectory( const rtl::OUString& aDirectory )
    /*throw( lang::IllegalArgumentException, uno::RuntimeException )*/
{
    SolarMutexGuard aGuard;

    if (aDirectory != m_sDisplayDirectory) {
        m_sDisplayDirectory = aDirectory;
    }
}

rtl::OUString const & SAL_CALL SalAquaPicker::implgetDisplayDirectory()
    /*throw( uno::RuntimeException )*/
{
    return m_sDisplayDirectory;
}

void SAL_CALL SalAquaPicker::implsetTitle( const rtl::OUString& aTitle )
    /*throw( uno::RuntimeException )*/
{
    SolarMutexGuard aGuard;

    if (m_pDialog != nil) {
        [m_pDialog setTitle:[NSString stringWithOUString:aTitle]];
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
