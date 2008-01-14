/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclnsapp.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 16:14:54 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _VCL_VCLNSAPP_H
#define _VCL_VCLNSAPP_H

#include "premac.h"
#include "Cocoa/Cocoa.h"
#include "postmac.h"

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
-(NSMenu*)applicationDockMenu:(NSApplication *)sender;
-(MacOSBOOL)application: (NSApplication*) app openFile: (NSString*)file;
-(void)application: (NSApplication*) app openFiles: (NSArray*)files;
-(MacOSBOOL)application: (NSApplication*) app printFile: (NSString*)file;
-(NSApplicationPrintReply)application: (NSApplication *) app printFiles:(NSArray *)files withSettings: (NSDictionary *)printSettings showPrintPanels:(MacOSBOOL)bShowPrintPanels;
-(NSApplicationTerminateReply)applicationShouldTerminate: (NSApplication *) app;
-(void)systemColorsChanged: (NSNotification*) pNotification;
-(void)screenParametersChanged: (NSNotification*) pNotification;
@end


#endif
