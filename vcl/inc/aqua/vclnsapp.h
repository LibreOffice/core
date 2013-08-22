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

#ifndef _VCL_VCLNSAPP_H
#define _VCL_VCLNSAPP_H

#include <config_features.h>

#include "premac.h"
#include "Cocoa/Cocoa.h"
#include "postmac.h"

class AquaSalFrame;

@interface CocoaThreadEnabler : NSObject
{
}
-(void)enableCocoaThreads:(id)param;
@end

// our very own application
@interface VCL_NSApplication : NSApplication
{
}
-(void)sendEvent:(NSEvent*)pEvent;
-(void)sendSuperEvent:(NSEvent*)pEvent;
-(NSMenu*)applicationDockMenu:(NSApplication *)sender;
-(BOOL)application: (NSApplication*) app openFile: (NSString*)file;
-(void)application: (NSApplication*) app openFiles: (NSArray*)files;
-(BOOL)application: (NSApplication*) app printFile: (NSString*)file;
-(NSApplicationPrintReply)application: (NSApplication *) app printFiles:(NSArray *)files withSettings: (NSDictionary *)printSettings showPrintPanels:(BOOL)bShowPrintPanels;
-(NSApplicationTerminateReply)applicationShouldTerminate: (NSApplication *) app;
-(void)systemColorsChanged: (NSNotification*) pNotification;
-(void)screenParametersChanged: (NSNotification*) pNotification;
-(void)scrollbarVariantChanged: (NSNotification*) pNotification;
-(void)scrollbarSettingsChanged: (NSNotification*) pNotification;
-(void)addFallbackMenuItem: (NSMenuItem*)pNewItem;
-(void)removeFallbackMenuItem: (NSMenuItem*)pOldItem;
-(void)addDockMenuItem: (NSMenuItem*)pNewItem;
#if !HAVE_FEATURE_MACOSX_SANDBOX
-(void)applicationWillBecomeActive: (NSNotification *)pNotification;
-(void)applicationWillResignActive: (NSNotification *)pNotification;
#endif
-(BOOL)applicationShouldHandleReopen: (NSApplication*)pApp hasVisibleWindows: (BOOL)bWinVisible;
-(void)setDockIconClickHandler: (NSObject*)pHandler;
-(void)cycleFrameForward: (AquaSalFrame*)pCurFrame;
-(void)cycleFrameBackward: (AquaSalFrame*)pCurFrame;
@end

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
