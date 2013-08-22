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

#include <config_features.h>

#include "sal/config.h"

#include <vector>

#include "vcl/window.hxx"
#include "vcl/svapp.hxx"
#include "vcl/cmdevt.hxx"

#include "aqua/vclnsapp.h"
#include "aqua/salinst.h"
#include "aqua/saldata.hxx"
#include "aqua/salframe.h"
#include "aqua/salframeview.h"
#include "quartz/utils.h"

#include "impimagetree.hxx"

#include "premac.h"
#include <objc/objc-runtime.h>
#import "Carbon/Carbon.h"
#import "apple_remote/RemoteControl.h"
#include "postmac.h"


@implementation CocoaThreadEnabler
-(void)enableCocoaThreads:(id)param
{
    // do nothing, this is just to start an NSThread and therefore put
    // Cocoa into multithread mode
    (void)param;
}
@end

// If you wonder how this VCL_NSApplication stuff works, one thing you
// might have missed is that the NSPrincipalClass property in
// desktop/macosx/Info.plist has the value VCL_NSApplication.

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
                    // Note: gcc 4.2.1 (in the 10.6 SDK) tells us
                    // 'NSWindow' may not respond to
                    // '-windowShouldClose:' . Is that a bogus
                    // warning, or is this code bogus? No idea.
                    // Anyway, so that we can compile also against
                    // this SDK with -Werror, use objc_msgSend
                    // instead.

                    // Instead of:
                    // [pFrame->getWindow() windowShouldClose: nil];
                    // do:
                    objc_msgSend(pFrame->getWindow(), @selector(windowShouldClose:), nil);

                    return;
                }
            }

            /*
             * #i98949# - Cmd-M miniaturize window, Cmd-Option-M miniaturize all windows
             */
            if( [[pEvent charactersIgnoringModifiers] isEqualToString: @"m"] )
            {
                if ( nModMask == NSCommandKeyMask && ([pFrame->getWindow() styleMask] & NSMiniaturizableWindowMask) )
                {
                    [pFrame->getWindow() performMiniaturize: nil];
                    return;
                }

                if ( nModMask == ( NSCommandKeyMask | NSAlternateKeyMask ) )
                {
                    [NSApp miniaturizeAll: nil];
                    return;
                }
            }

            // #i90083# handle frame switching
            // FIXME: lousy workaround
            if( (nModMask & (NSControlKeyMask|NSAlternateKeyMask)) == 0 )
            {
                if( [[pEvent characters] isEqualToString: @"<"] ||
                    [[pEvent characters] isEqualToString: @"~"] )
                {
                    [self cycleFrameForward: pFrame];
                    return;
                }
                else if( [[pEvent characters] isEqualToString: @">"] ||
                         [[pEvent characters] isEqualToString: @"`"] )
                {
                    [self cycleFrameBackward: pFrame];
                    return;
                }
            }

            // get information whether the event was handled; keyDown returns nothing
            GetSalData()->maKeyEventAnswer[ pEvent ] = false;
            bool bHandled = false;

            // dispatch to view directly to avoid the key event being consumed by the menubar
            // popup windows do not get the focus, so they don't get these either
            // simplest would be dispatch this to the key window always if it is without parent
            // however e.g. in document we want the menu shortcut if e.g. the stylist has focus
            if( pFrame->mpParent && (pFrame->mnStyle & SAL_FRAME_STYLE_FLOAT) == 0 )
            {
                [[pKeyWin contentView] keyDown: pEvent];
                bHandled = GetSalData()->maKeyEventAnswer[ pEvent ];
            }

            // see whether the main menu consumes this event
            // if not, we want to dispatch it ourselves. Unless we do this "trick"
            // the main menu just beeps for an unknown or disabled key equivalent
            // and swallows the event wholesale
            NSMenu* pMainMenu = [NSApp mainMenu];
            if( ! bHandled && (pMainMenu == 0 || ! [pMainMenu performKeyEquivalent: pEvent]) )
            {
                [[pKeyWin contentView] keyDown: pEvent];
                bHandled = GetSalData()->maKeyEventAnswer[ pEvent ];
            }
            else
                bHandled = true;  // event handled already or main menu just handled it

            GetSalData()->maKeyEventAnswer.erase( pEvent );
            if( bHandled )
                return;
        }
        else if( pKeyWin )
        {
            // #i94601# a window not of vcl's making has the focus.
            // Since our menus do not invoke the usual commands
            // try to play nice with native windows like the file dialog
            // and emulate them
            // precondition: this ONLY works because CMD-V (paste), CMD-C (copy) and CMD-X (cut) are
            // NOT localized, that is the same in all locales. Should this be
            // different in any locale, this hack will fail.
            unsigned int nModMask = ([pEvent modifierFlags] & (NSShiftKeyMask|NSControlKeyMask|NSAlternateKeyMask|NSCommandKeyMask));
            if( nModMask == NSCommandKeyMask )
            {

                if( [[pEvent charactersIgnoringModifiers] isEqualToString: @"v"] )
                {
                    if( [NSApp sendAction: @selector(paste:) to: nil from: nil] )
                        return;
                }
                else if( [[pEvent charactersIgnoringModifiers] isEqualToString: @"c"] )
                {
                    if( [NSApp sendAction: @selector(copy:) to: nil from: nil] )
                        return;
                }
                else if( [[pEvent charactersIgnoringModifiers] isEqualToString: @"x"] )
                {
                    if( [NSApp sendAction: @selector(cut:) to: nil from: nil] )
                        return;
                }
            }
        }
    }
    [super sendEvent: pEvent];
}

-(void)sendSuperEvent:(NSEvent*)pEvent
{
    [super sendEvent: pEvent];
}

-(void)cycleFrameForward: (AquaSalFrame*)pCurFrame
{
    // find current frame in list
    std::list< AquaSalFrame* >& rFrames( GetSalData()->maFrames );
    std::list< AquaSalFrame* >::iterator it = rFrames.begin();
    for( ; it != rFrames.end() && *it != pCurFrame; ++it )
        ;
    if( it != rFrames.end() )
    {
        // now find the next frame (or end)
        do
        {
            ++it;
            if( it != rFrames.end() )
            {
                if( (*it)->mpDockMenuEntry != NULL &&
                    (*it)->mbShown )
                {
                    [(*it)->getWindow() makeKeyAndOrderFront: NSApp];
                    return;
                }
            }
        } while( it != rFrames.end() );
        // cycle around, find the next up to pCurFrame
        it = rFrames.begin();
        while( *it != pCurFrame )
        {
            if( (*it)->mpDockMenuEntry != NULL &&
                (*it)->mbShown )
            {
                [(*it)->getWindow() makeKeyAndOrderFront: NSApp];
                return;
            }
            ++it;
        }
    }
}

-(void)cycleFrameBackward: (AquaSalFrame*)pCurFrame
{
    // do the same as cycleFrameForward only with a reverse iterator

    // find current frame in list
    std::list< AquaSalFrame* >& rFrames( GetSalData()->maFrames );
    std::list< AquaSalFrame* >::reverse_iterator it = rFrames.rbegin();
    for( ; it != rFrames.rend() && *it != pCurFrame; ++it )
        ;
    if( it != rFrames.rend() )
    {
        // now find the next frame (or end)
        do
        {
            ++it;
            if( it != rFrames.rend() )
            {
                if( (*it)->mpDockMenuEntry != NULL &&
                    (*it)->mbShown )
                {
                    [(*it)->getWindow() makeKeyAndOrderFront: NSApp];
                    return;
                }
            }
        } while( it != rFrames.rend() );
        // cycle around, find the next up to pCurFrame
        it = rFrames.rbegin();
        while( *it != pCurFrame )
        {
            if( (*it)->mpDockMenuEntry != NULL &&
                (*it)->mbShown )
            {
                [(*it)->getWindow() makeKeyAndOrderFront: NSApp];
                return;
            }
            ++it;
        }
    }
}

-(NSMenu*)applicationDockMenu:(NSApplication *)sender
{
    (void)sender;
    return AquaSalInstance::GetDynamicDockMenu();
}

-(BOOL)application: (NSApplication*)app openFile: (NSString*)pFile
{
    (void)app;
    std::vector<OUString> aFile;
    aFile.push_back( GetOUString( pFile ) );
    if( ! AquaSalInstance::isOnCommandLine( aFile[0] ) )
    {
        const ApplicationEvent* pAppEvent = new ApplicationEvent(ApplicationEvent::TYPE_OPEN, aFile);
        AquaSalInstance::aAppEventList.push_back( pAppEvent );
    }
    return YES;
}

-(void)application: (NSApplication*) app openFiles: (NSArray*)files
{
    (void)app;
    std::vector<OUString> aFileList;

    NSEnumerator* it = [files objectEnumerator];
    NSString* pFile = nil;

    while( (pFile = [it nextObject]) != nil )
    {
        const rtl::OUString aFile( GetOUString( pFile ) );
        if( ! AquaSalInstance::isOnCommandLine( aFile ) )
        {
            aFileList.push_back( aFile );
        }
    }

    if( !aFileList.empty() )
    {
        // we have no back channel here, we have to assume success, in which case
        // replyToOpenOrPrint does not need to be called according to documentation
        // [app replyToOpenOrPrint: NSApplicationDelegateReplySuccess];
        const ApplicationEvent* pAppEvent = new ApplicationEvent(ApplicationEvent::TYPE_OPEN, aFileList);
        AquaSalInstance::aAppEventList.push_back( pAppEvent );
    }
}

-(BOOL)application: (NSApplication*)app printFile: (NSString*)pFile
{
    (void)app;
    std::vector<OUString> aFile;
    aFile.push_back( GetOUString( pFile ) );
	const ApplicationEvent* pAppEvent = new ApplicationEvent(ApplicationEvent::TYPE_PRINT, aFile);
	AquaSalInstance::aAppEventList.push_back( pAppEvent );
    return YES;
}
-(NSApplicationPrintReply)application: (NSApplication *) app printFiles:(NSArray *)files withSettings: (NSDictionary *)printSettings showPrintPanels:(BOOL)bShowPrintPanels
{
    (void)app;
    (void)printSettings;
    (void)bShowPrintPanels;
    // currently ignores print settings an bShowPrintPanels
    std::vector<OUString> aFileList;

    NSEnumerator* it = [files objectEnumerator];
    NSString* pFile = nil;

    while( (pFile = [it nextObject]) != nil )
    {
        aFileList.push_back( GetOUString( pFile ) );
    }
	const ApplicationEvent* pAppEvent = new ApplicationEvent(ApplicationEvent::TYPE_PRINT, aFileList);
	AquaSalInstance::aAppEventList.push_back( pAppEvent );
    // we have no back channel here, we have to assume success
    // correct handling would be NSPrintingReplyLater and then send [app replyToOpenOrPrint]
    return NSPrintingSuccess;
}

-(NSApplicationTerminateReply)applicationShouldTerminate: (NSApplication *) app
{
    (void)app;
    NSApplicationTerminateReply aReply = NSTerminateNow;
    {
        YIELD_GUARD;

        SalData* pSalData = GetSalData();
        if( ! pSalData->maFrames.empty() )
        {
            // the following QueryExit will likely present a message box, activate application
            [NSApp activateIgnoringOtherApps: YES];
            aReply = pSalData->maFrames.front()->CallCallback( SALEVENT_SHUTDOWN, NULL ) ? NSTerminateCancel : NSTerminateNow;
        }

        if( aReply == NSTerminateNow )
        {
            ApplicationEvent aEv(ApplicationEvent::TYPE_PRIVATE_DOSHUTDOWN);
            GetpApp()->AppEvent( aEv );
            ImplImageTreeSingletonRef()->shutDown();
            // DeInitVCL should be called in ImplSVMain - unless someon _exits first which
            // can occur in Desktop::doShutdown for example
        }
    }

    return aReply;
}

-(void)systemColorsChanged: (NSNotification*) pNotification
{
    (void)pNotification;
    YIELD_GUARD;

    const SalData* pSalData = GetSalData();
	if( !pSalData->maFrames.empty() )
		pSalData->maFrames.front()->CallCallback( SALEVENT_SETTINGSCHANGED, NULL );
}

-(void)screenParametersChanged: (NSNotification*) pNotification
{
    (void)pNotification;
    YIELD_GUARD;

    SalData* pSalData = GetSalData();
    std::list< AquaSalFrame* >::iterator it;
    for( it = pSalData->maFrames.begin(); it != pSalData->maFrames.end(); ++it )
    {
        (*it)->screenParametersChanged();
    }
}

-(void)scrollbarVariantChanged: (NSNotification*) pNotification
{
    (void)pNotification;
    GetSalData()->mpFirstInstance->delayedSettingsChanged( true );
}

-(void)scrollbarSettingsChanged: (NSNotification*) pNotification
{
    (void)pNotification;
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

-(void)addDockMenuItem: (NSMenuItem*)pNewItem
{
    NSMenu* pDock = AquaSalInstance::GetDynamicDockMenu();
    [pDock insertItem: pNewItem atIndex: [pDock numberOfItems]];
}

// for Apple Remote implementation

#if !HAVE_FEATURE_MACOSX_SANDBOX
- (void)applicationWillBecomeActive:(NSNotification *)pNotification
{
    (void)pNotification;
    SalData* pSalData = GetSalData();
    if( pSalData->mpMainController && pSalData->mpMainController->remoteControl)
    {
        // [remoteControl startListening: self];
        // does crash because the right thing to do is
        // [GetSalData()->mpMainController->remoteControl startListening: self];
        // but the instance variable 'remoteControl' is declared protected
        // workaround : declare remoteControl instance variable as public in RemoteMainController.m

        [pSalData->mpMainController->remoteControl startListening: self];
#ifdef DEBUG
        NSLog(@"Apple Remote will become active - Using remote controls");
#endif
    }
    for( std::list< AquaSalFrame* >::const_iterator it = pSalData->maPresentationFrames.begin();
         it != pSalData->maPresentationFrames.end(); ++it )
    {
        [(*it)->mpWindow setLevel: NSPopUpMenuWindowLevel];
        if( [(*it)->mpWindow isVisible] )
            [(*it)->mpWindow orderFront: NSApp];
    }
}

- (void)applicationWillResignActive:(NSNotification *)pNotification
{
    (void)pNotification;
    SalData* pSalData = GetSalData();
    if( pSalData->mpMainController && pSalData->mpMainController->remoteControl)
    {
        // [remoteControl stopListening: self];
        // does crash because the right thing to do is
        // [GetSalData()->mpMainController->remoteControl stopListening: self];
        // but the instance variable 'remoteControl' is declared protected
        // workaround : declare remoteControl instance variable as public in RemoteMainController.m

        [pSalData->mpMainController->remoteControl stopListening: self];
#ifdef DEBUG
        NSLog(@"Apple Remote will resign active - Releasing remote controls");
#endif
    }
    for( std::list< AquaSalFrame* >::const_iterator it = pSalData->maPresentationFrames.begin();
         it != pSalData->maPresentationFrames.end(); ++it )
    {
        [(*it)->mpWindow setLevel: NSNormalWindowLevel];
    }
}
#endif

- (BOOL)applicationShouldHandleReopen: (NSApplication*)pApp hasVisibleWindows: (BOOL) bWinVisible
{
    (void)pApp;
    (void)bWinVisible;
    NSObject* pHdl = GetSalData()->mpDockIconClickHandler;
    if( pHdl && [pHdl respondsToSelector: @selector(dockIconClicked:)] )
    {
        [pHdl performSelector:@selector(dockIconClicked:) withObject: self];
    }
    return YES;
}

-(void)setDockIconClickHandler: (NSObject*)pHandler
{
    GetSalData()->mpDockIconClickHandler = pHandler;
}


@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
