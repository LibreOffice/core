/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
-(void)applicationDidFinishLaunching:(NSNotification*)aNotification;
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
