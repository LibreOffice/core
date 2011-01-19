/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _VCL_VCLNSAPP_H
#define _VCL_VCLNSAPP_H

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
-(void)applicationWillBecomeActive: (NSNotification *)pNotification;
-(void)applicationWillResignActive: (NSNotification *)pNotification;
-(BOOL)applicationShouldHandleReopen: (NSApplication*)pApp hasVisibleWindows: (BOOL)bWinVisible;
-(void)setDockIconClickHandler: (NSObject*)pHandler;
-(void)cycleFrameForward: (AquaSalFrame*)pCurFrame;
-(void)cycleFrameBackward: (AquaSalFrame*)pCurFrame;
@end

#endif
