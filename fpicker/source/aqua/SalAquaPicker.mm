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
#include <cppuhelper/interfacecontainer.h>
#include <osl/diagnose.h>
#include <com/sun/star/uno/Any.hxx>
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
#define CLASS_NAME "SalAquaPicker"
#define kSetHideExtensionStateKey @"NSNavLastUserSetHideExtensionButtonState"

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

// constructor
SalAquaPicker::SalAquaPicker()
: m_pDialog(NULL)
, m_pControlHelper(new ControlHelper())
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);
    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

SalAquaPicker::~SalAquaPicker()
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    SolarMutexGuard aGuard;

    NSAutoreleasePool *pool = [NSAutoreleasePool new];

    if (NULL != m_pControlHelper)
        delete m_pControlHelper;

    if (NULL != m_pDialog)
        [m_pDialog release];

    [pool release];

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void SAL_CALL SalAquaPicker::implInitialize()
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    SolarMutexGuard aGuard;

    if (m_pDialog != nil) {
        return;
    }

    switch (m_nDialogType)
    {
        case NAVIGATIONSERVICES_OPEN:
            OSL_TRACE("NAVIGATIONSERVICES_OPEN");
            m_pDialog = [NSOpenPanel openPanel];
            [(NSOpenPanel*)m_pDialog setCanChooseDirectories:NO];
            [(NSOpenPanel*)m_pDialog setCanChooseFiles:YES];
            break;

        case NAVIGATIONSERVICES_SAVE:
            OSL_TRACE("NAVIGATIONSERVICES_SAVE");
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
                    OSL_TRACE("Hiding extension");
                    [pDefaults setBool:YES forKey:kSetHideExtensionStateKey];
                }
            }
            break;

        case NAVIGATIONSERVICES_DIRECTORY:
            OSL_TRACE("NAVIGATIONSERVICES_DIRECTORY");
            m_pDialog = [NSOpenPanel openPanel];
            [(NSOpenPanel*)m_pDialog setCanChooseDirectories:YES];
            [(NSOpenPanel*)m_pDialog setCanChooseFiles:NO];
            break;

        default:
            OSL_TRACE("m_nDialogType is UNKNOWN: %d", m_nDialogType);
            break;
    }

    if (m_pDialog == nil) {
        OSL_TRACE("An error occurred while creating the dialog!");
    }
    else {
        [(NSOpenPanel*)m_pDialog setCanCreateDirectories:YES];
        //Retain the dialog instance or it will go away immediately
        [m_pDialog retain];
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

int SalAquaPicker::run()
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    SolarMutexGuard aGuard;

    NSAutoreleasePool *pool = [NSAutoreleasePool new];

    if (m_pDialog == NULL) {
        //this is the case e.g. for the folder picker at this stage
        implInitialize();
    }

    NSView *userPane = m_pControlHelper->getUserPane();
    if (userPane != NULL) {
        [m_pDialog setAccessoryView:userPane];
    }

    int retVal = 0;

    NSString *startDirectory;
    if (m_sDisplayDirectory.getLength() > 0) {
        NSString *temp = [NSString stringWithOUString:m_sDisplayDirectory];
        NSURL *url = [NSURL URLWithString:temp];
        startDirectory = [url path];

        OSL_TRACE("start dir: %s", [startDirectory UTF8String]);
        // NSLog(@"%@", startDirectory);
    }
    else {
        startDirectory = NSHomeDirectory();
    }

#if defined(LIBO_WERROR) && defined(__clang__) && MACOSX_SDK_VERSION >= 1070
#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
#endif
    switch(m_nDialogType) {
        case NAVIGATIONSERVICES_DIRECTORY:
        case NAVIGATIONSERVICES_OPEN:
            retVal = [(NSOpenPanel*)m_pDialog runModalForDirectory:startDirectory file:nil types:nil];
            break;
        case NAVIGATIONSERVICES_SAVE:
            retVal = [m_pDialog runModalForDirectory:startDirectory file:[NSString stringWithOUString:((SalAquaFilePicker*)this)->getSaveFileName()]/*[m_pDialog saveFilename]*/];
            break;
        // [m_pDialog beginSheetForDirectory:startDirectory file:[m_pDialog saveFilename] modalForWindow:[NSApp keyWindow] modalDelegate:((SalAquaFilePicker*)this)->getDelegate() didEndSelector:@selector(savePanelDidEnd:returnCode:contextInfo:) contextInfo:nil];
        default:
            break;
    }

    if (retVal == NSFileHandlingPanelOKButton) {
        NSString* pDir = [m_pDialog directory];
        if (pDir) {
            implsetDisplayDirectory([[NSURL fileURLWithPath:pDir] OUStringForInfo:FULLPATH]);
        }
    }
#if defined(LIBO_WERROR) && defined(__clang__) && MACOSX_SDK_VERSION >= 1070
#pragma GCC diagnostic pop
#endif
    DBG_PRINT_EXIT(CLASS_NAME, __func__, retVal);

    [pool release];

    return retVal;
}

int SalAquaPicker::runandwaitforresult()
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    SolarMutexGuard aGuard;

    int status = this->run();

    DBG_PRINT_EXIT(CLASS_NAME, __func__, status);
    return status;
}

void SAL_CALL SalAquaPicker::implsetDisplayDirectory( const rtl::OUString& aDirectory )
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "directory", aDirectory);

    SolarMutexGuard aGuard;

    if (aDirectory != m_sDisplayDirectory) {
        m_sDisplayDirectory = aDirectory;
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

rtl::OUString SAL_CALL SalAquaPicker::implgetDisplayDirectory() throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);
    DBG_PRINT_EXIT(CLASS_NAME, __func__, m_sDisplayDirectory);

    return m_sDisplayDirectory;
}

void SAL_CALL SalAquaPicker::implsetTitle( const rtl::OUString& aTitle ) throw( uno::RuntimeException )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "title", aTitle);

    SolarMutexGuard aGuard;

    if (m_pDialog != nil) {
        [m_pDialog setTitle:[NSString stringWithOUString:aTitle]];
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
