/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vclnsapp.mm,v $
 * $Revision: 1.9 $
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
            // handle Cmd-W
            // FIXME: the correct solution would be to handle this in framework
            // in the menu code
            // however that is currently being revised, so let's use a preliminary solution here
            // this hack is based on assumption
            // a) Cmd-W is the same in all languages in OOo's menu conig
            // b) Cmd-W is the same in all languages in on MacOS
            // for now this seems to be true
            unsigned int nModMask = ([pEvent modifierFlags] & (NSShiftKeyMask|NSControlKeyMask|NSAlternateKeyMask|NSCommandKeyMask));
            if( (pFrame->mnStyleMask & NSClosableWindowMask) != 0 )
            {
                if( nModMask == NSCommandKeyMask
                    && [[pEvent charactersIgnoringModifiers] isEqualToString: @"w"] )
                {
                    [pFrame->getWindow() windowShouldClose: nil];
                    return;
                }
            }
            
            /* #i89611#
               Cmd-Option-Space is for some reason not consumed by the menubar,
               but also not by the input method (like e.g. Cmd-Space) and stays
               without function.

               However MOD1 + MOD2 combinations are not used throughout OOo code
               since they tend to clash with system shortcuts on all platforms so
               we can skip this case here.
            */
            if( nModMask != (NSCommandKeyMask | NSAlternateKeyMask) )
            {
                // dispatch to view directly to avoid the key event being consumed by the menubar
                // popup windows do not get the focus, so they don't get these either
                // simplest would be dispatch this to the key window always if it is without parent
                // however e.g. in document we want the menu shortcut if e.g. the stylist has focus
                if( pFrame->mpParent && (pFrame->mnStyle & SAL_FRAME_STYLE_FLOAT) == 0 ) 
                {
                    [[pKeyWin contentView] keyDown: pEvent];
                    return;
                }
                
                // see whether the main menu consumes this event
                // if not, we want to dispatch it ourselves. Unless we do this "trick"
                // the main menu just beeps for an unknown or disabled key equivalent
                // and swallows the event wholesale
                NSMenu* pMainMenu = [NSApp mainMenu];
                if( pMainMenu == 0 || ! [pMainMenu performKeyEquivalent: pEvent] )
                    [[pKeyWin contentView] keyDown: pEvent];
                
                // at this point either the menu has executed the accelerator
                // or we have dispatched the event
                // so no need to dispatch further
                return;
            }
        }
    }
    else if( eType == NSScrollWheel && ( GetSalData()->mnSystemVersion < VER_LEOPARD /* fixed in Leopard and above */ ) )
    {

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

-(void)sendSuperEvent:(NSEvent*)pEvent
{
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
    SalData* pSalData = GetSalData();
    #if 1 // currently do some really bad hack
    if( ! pSalData->maFrames.empty() )
    {
        /* #i92766# something really weird is going on with the retain count of
           our windows; sometimes we get a duplicate free before exit on one of our
           NSWindows. The reason is unclear; to avoid this currently we retain them once more
           
           FIXME: this is a really bad hack, relying on the system to catch the leaked
           resources. Find out what really goes on here and fix it !
        */
        std::vector< NSWindow* > aHackRetainedWindows;
        for( std::list< AquaSalFrame* >::iterator it = pSalData->maFrames.begin();
             it != pSalData->maFrames.end(); ++it )
        {
            #if OSL_DEBUG_LEVEL > 1
            Window* pWin = (*it)->GetWindow();
            String aTitle = pWin->GetText();
            Window* pClient = pWin->ImplGetClientWindow();
            fprintf( stderr, "retaining %p (old count %d) windowtype=%s clienttyp=%s title=%s\n",
                (*it)->mpWindow, [(*it)->mpWindow retainCount],
                typeid(*pWin).name(), pClient ? typeid(*pClient).name() : "<nil>",
                rtl::OUStringToOString( aTitle, RTL_TEXTENCODING_UTF8 ).getStr()
                );
            #endif
            [(*it)->mpWindow retain];
            aHackRetainedWindows.push_back( (*it)->mpWindow ); 
        }
        if( pSalData->maFrames.front()->CallCallback( SALEVENT_SHUTDOWN, NULL ) )
        {
            for( std::vector< NSWindow* >::iterator it = aHackRetainedWindows.begin();
                 it != aHackRetainedWindows.end(); ++it )
            {
                // clean up the retaing count again from the shutdown workaround
                #if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "releasing %p\n", (*it) );
                #endif
                [(*it) release];
            }
            return NSTerminateCancel;
        }
        #if OSL_DEBUG_LEVEL > 1
        for( std::list< AquaSalFrame* >::iterator it = pSalData->maFrames.begin();
             it != pSalData->maFrames.end(); ++it )
        {
            Window* pWin = (*it)->GetWindow();
            String aTitle = pWin->GetText();
            Window* pClient = pWin->ImplGetClientWindow();
            fprintf( stderr, "frame still alive: NSWindow %p windowtype=%s clienttyp=%s title=%s\n",
                (*it)->mpWindow, typeid(*pWin).name(), pClient ? typeid(*pClient).name() : "<nil>",
                rtl::OUStringToOString( aTitle, RTL_TEXTENCODING_UTF8 ).getStr()
                );
        }
        #endif
    }
    #else // the clean version follows
    return pSalData->maFrames.front()->CallCallback( SALEVENT_SHUTDOWN, NULL ) ? NSTerminateCancel : NSTerminateNow;
    #endif
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

- (void)getSystemVersionMajor:(unsigned *)major
                        minor:(unsigned *)minor
                       bugFix:(unsigned *)bugFix
{
    OSErr err;
    SInt32 systemVersion = VER_TIGER; // Initialize with minimal requirement
    if ((err = Gestalt(gestaltSystemVersion, &systemVersion)) == noErr) 
    {
        GetSalData()->mnSystemVersion = systemVersion;
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "System Version %x\n", (unsigned int)systemVersion);
        fprintf( stderr, "Stored System Version %x\n", (unsigned int)GetSalData()->mnSystemVersion);
#endif
    }
    else
        NSLog(@"Unable to obtain system version: %ld", (long)err);

    return;
}

-(void)addDockMenuItem: (NSMenuItem*)pNewItem
{
    NSMenu* pDock = AquaSalInstance::GetDynamicDockMenu();
    [pDock insertItem: pNewItem atIndex: [pDock numberOfItems]];
}

@end

