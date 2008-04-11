/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vclnsapp.mm,v $
 * $Revision: 1.4 $
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

 // MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include "vclnsapp.h"
#include "salinst.h"
#include "saldata.hxx"
#include "salframe.h"
#include "salframeview.h"

#include "vcl/window.hxx"
#include "vcl/svapp.hxx"

#include "rtl/ustrbuf.hxx"

 
@implementation CocoaThreadEnabler
-(void)enableCocoaThreads:(id)param
{
    // do nothing, this is just to start an NSThread and therefore put
    // Cocoa into multithread mode
}
@end

@implementation VCL_NSApplication
-(void)sendEvent:(NSEvent*)pEvent
{
    NSEventType eType = [pEvent type];
    if( eType == NSApplicationDefined )
        GetSalData()->mpFirstInstance->handleAppDefinedEvent( pEvent );
    else if( eType == NSKeyDown && ([pEvent modifierFlags] & NSCommandKeyMask) != 0 )
    {
        NSWindow* pKeyWin = [NSApp keyWindow];
        if( pKeyWin && [pKeyWin isKindOfClass: [SalFrameWindow class]] )
        {
            AquaSalFrame* pFrame = [(SalFrameWindow*)pKeyWin getSalFrame];
            // dispatch to view directly to avoid the key event being consumed by the menubar
            // popup windows do not get the focus, so they don't get these either
            // simplest would be dispatch this to the key window always if it is without parent
            // however e.g. in document we want the menu shortcut if e.g. the stylist has focus
            if( pFrame->mpParent && (pFrame->mnStyle & SAL_FRAME_STYLE_FLOAT) == 0 )
            {
                [[pKeyWin contentView] keyDown: pEvent];
                return;
            }
        }

    }
    else if( eType == NSScrollWheel )
    {
        // FIMXE: this is necessary on MacOS 10.4 only
        // how do I find out the OS version ?
        
        NSWindow* pWin = [pEvent window];
        // on Tiger wheel events do not reach non key windows
        // which probably should be considered a bug
        if( [pWin isKindOfClass: [SalFrameWindow class]] && [pWin canBecomeKeyWindow] == NO )
        {
            [[pWin contentView] scrollWheel: pEvent];
            return;
        }
    }
    [super sendEvent: pEvent];
}

-(NSMenu*)applicationDockMenu:(NSApplication *)sender
{
    return AquaSalInstance::GetDynamicDockMenu();
}

-(MacOSBOOL)application: (NSApplication*)app openFile: (NSString*)pFile
{
    const rtl::OUString aFile( GetOUString( pFile ) );
    if( ! AquaSalInstance::isOnCommandLine( aFile ) )
    {
        const ApplicationEvent* pAppEvent = new ApplicationEvent( String(), ApplicationAddress(),
                                                    APPEVENT_OPEN_STRING, aFile );
        AquaSalInstance::aAppEventList.push_back( pAppEvent );
    }
    return YES;
}

-(void)application: (NSApplication*) app openFiles: (NSArray*)files
{
    rtl::OUStringBuffer aFileList( 256 );
    
    NSEnumerator* it = [files objectEnumerator];
    NSString* pFile = nil;
    
    while( (pFile = [it nextObject]) != nil )
    {
        const rtl::OUString aFile( GetOUString( pFile ) );
        if( ! AquaSalInstance::isOnCommandLine( aFile ) )
        {
            if( aFileList.getLength() > 0 )
                aFileList.append( sal_Unicode( APPEVENT_PARAM_DELIMITER ) );
            aFileList.append( aFile );
        }
    }
    
    if( aFileList.getLength() )
    {
        // we have no back channel here, we have to assume success, in which case
        // replyToOpenOrPrint does not need to be called according to documentation
        // [app replyToOpenOrPrint: NSApplicationDelegateReplySuccess];
        const ApplicationEvent* pAppEvent = new ApplicationEvent( String(), ApplicationAddress(),
                                                    APPEVENT_OPEN_STRING, aFileList.makeStringAndClear() );
        AquaSalInstance::aAppEventList.push_back( pAppEvent );
    }
}

-(MacOSBOOL)application: (NSApplication*)app printFile: (NSString*)pFile
{
    const rtl::OUString aFile( GetOUString( pFile ) );
	const ApplicationEvent* pAppEvent = new ApplicationEvent( String(), ApplicationAddress(),
                                                APPEVENT_PRINT_STRING, aFile );
	AquaSalInstance::aAppEventList.push_back( pAppEvent );
    return YES;
}
-(NSApplicationPrintReply)application: (NSApplication *) app printFiles:(NSArray *)files withSettings: (NSDictionary *)printSettings showPrintPanels:(MacOSBOOL)bShowPrintPanels
{
    // currently ignores print settings an bShowPrintPanels
    rtl::OUStringBuffer aFileList( 256 );
    
    NSEnumerator* it = [files objectEnumerator];
    NSString* pFile = nil;
    
    while( (pFile = [it nextObject]) != nil )
    {
        if( aFileList.getLength() > 0 )
            aFileList.append( sal_Unicode( APPEVENT_PARAM_DELIMITER ) );
        aFileList.append( GetOUString( pFile ) );
    }
	const ApplicationEvent* pAppEvent = new ApplicationEvent( String(), ApplicationAddress(),
                                                APPEVENT_PRINT_STRING, aFileList.makeStringAndClear() );
	AquaSalInstance::aAppEventList.push_back( pAppEvent );
    // we have no back channel here, we have to assume success
    // correct handling would be NSPrintingReplyLater and then send [app replyToOpenOrPrint]
    return NSPrintingSuccess;
}

-(NSApplicationTerminateReply)applicationShouldTerminate: (NSApplication *) app
{
    const SalData* pSalData = GetSalData();
    if( ! pSalData->maFrames.empty() )
        return pSalData->maFrames.front()->CallCallback( SALEVENT_SHUTDOWN, NULL ) ? NSTerminateCancel : NSTerminateNow;
    return NSTerminateNow;
}

-(void)systemColorsChanged: (NSNotification*) pNotification
{
    const SalData* pSalData = GetSalData();
	if( !pSalData->maFrames.empty() )
		pSalData->maFrames.front()->CallCallback( SALEVENT_SETTINGSCHANGED, NULL );
}

-(void)screenParametersChanged: (NSNotification*) pNotification
{
    SalData* pSalData = GetSalData();
    std::list< AquaSalFrame* >::iterator it;
    for( it = pSalData->maFrames.begin(); it != pSalData->maFrames.end(); ++it )
    {
        (*it)->screenParametersChanged();
    }
}

-(void)scrollbarVariantChanged: (NSNotification*) pNotification
{
    GetSalData()->mpFirstInstance->delayedSettingsChanged( true );
}

-(void)scrollbarSettingsChanged: (NSNotification*) pNotification
{
    GetSalData()->mpFirstInstance->delayedSettingsChanged( false );
}

-(void)addFallbackMenuItem: (NSMenuItem*)pNewItem
{
    AquaSalMenu::addFallbackMenuItem( pNewItem );
}

-(void)removeFallbackMenuItem: (NSMenuItem*)pItem
{
    AquaSalMenu::removeFallbackMenuItem( pItem );
}

@end

