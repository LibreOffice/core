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

#include <sal/config.h>
#include <config_features.h>

#include <vector>

#include <stdlib.h>

#include <sal/main.h>
#include <vcl/commandevent.hxx>
#include <vcl/ImageTree.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>

#include <osx/saldata.hxx>
#include <osx/salframe.h>
#include <osx/salframeview.h>
#include <osx/salinst.h>
#include <osx/vclnsapp.h>
#include <quartz/utils.h>

#include <premac.h>
#include <objc/objc-runtime.h>
#import "Carbon/Carbon.h"
#import "apple_remote/RemoteControl.h"
#include <postmac.h>


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

-(void)applicationDidFinishLaunching:(NSNotification*)pNotification
{
    (void)pNotification;

SAL_WNODEPRECATED_DECLARATIONS_PUSH
        // 'NSApplicationDefined' is deprecated: first deprecated in macOS 10.12
    NSEvent* pEvent = [NSEvent otherEventWithType: NSApplicationDefined
                               location: NSZeroPoint
                               modifierFlags: 0
                               timestamp: [[NSProcessInfo processInfo] systemUptime]
                               windowNumber: 0
                               context: nil
                               subtype: AquaSalInstance::AppExecuteSVMain
                               data1: 0
                               data2: 0 ];
SAL_WNODEPRECATED_DECLARATIONS_POP
    assert( pEvent );
    [NSApp postEvent: pEvent atStart: NO];

    if( [NSWindow respondsToSelector:@selector(allowsAutomaticWindowTabbing)] )
    {
        [NSWindow setAllowsAutomaticWindowTabbing:NO];
    }
}

-(void)sendEvent:(NSEvent*)pEvent
{
    NSEventType eType = [pEvent type];
SAL_WNODEPRECATED_DECLARATIONS_PUSH
        // 'NSAlternateKeyMask' is deprecated: first deprecated in macOS 10.12
        // 'NSApplicationDefined' is deprecated: first deprecated in macOS 10.12
        // 'NSClosableWindowMask' is deprecated: first deprecated in macOS 10.12
        // 'NSCommandKeyMask' is deprecated: first deprecated in macOS 10.12
        // 'NSControlKeyMask' is deprecated: first deprecated in macOS 10.12
        // 'NSKeyDown' is deprecated: first deprecated in macOS 10.12
        // 'NSMiniaturizableWindowMask' is deprecated: first deprecated in macOS 10.12
        // 'NSShiftKeyMask' is deprecated: first deprecated in macOS 10.12
    if( eType == NSApplicationDefined )
    {
        AquaSalInstance::handleAppDefinedEvent( pEvent );
    }
    else if( eType == NSKeyDown && ([pEvent modifierFlags] & NSCommandKeyMask) != 0 )
    {
        NSWindow* pKeyWin = [NSApp keyWindow];
        if( pKeyWin && [pKeyWin isKindOfClass: [SalFrameWindow class]] )
        {
            AquaSalFrame* pFrame = [static_cast<SalFrameWindow*>(pKeyWin) getSalFrame];
            unsigned int nModMask = ([pEvent modifierFlags] & (NSShiftKeyMask|NSControlKeyMask|NSAlternateKeyMask|NSCommandKeyMask));
            /*
             * #i98949# - Cmd-M miniaturize window, Cmd-Option-M miniaturize all windows
             */
            if( [[pEvent charactersIgnoringModifiers] isEqualToString: @"m"] )
            {
                if ( nModMask == NSCommandKeyMask && ([pFrame->getNSWindow() styleMask] & NSMiniaturizableWindowMask) )
                {
                    [pFrame->getNSWindow() performMiniaturize: nil];
                    return;
                }

                if ( nModMask == ( NSCommandKeyMask | NSAlternateKeyMask ) )
                {
                    [NSApp miniaturizeAll: nil];
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
            if( pFrame->mpParent && !(pFrame->mnStyle & SalFrameStyleFlags::FLOAT) )
            {
                [[pKeyWin contentView] keyDown: pEvent];
                bHandled = GetSalData()->maKeyEventAnswer[ pEvent ];
            }

            // see whether the main menu consumes this event
            // if not, we want to dispatch it ourselves. Unless we do this "trick"
            // the main menu just beeps for an unknown or disabled key equivalent
            // and swallows the event wholesale
            NSMenu* pMainMenu = [NSApp mainMenu];
            if( ! bHandled &&
                (pMainMenu == nullptr || ! [NSMenu menuBarVisible] || ! [pMainMenu performKeyEquivalent: pEvent]) )
            {
                [[pKeyWin contentView] keyDown: pEvent];
                bHandled = GetSalData()->maKeyEventAnswer[ pEvent ];
            }
            else
            {
                bHandled = true;  // event handled already or main menu just handled it
            }
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
                else if( [[pEvent charactersIgnoringModifiers] isEqualToString: @"a"] )
                {
                    if( [NSApp sendAction: @selector(selectAll:) to: nil from: nil] )
                        return;
                }
                else if( [[pEvent charactersIgnoringModifiers] isEqualToString: @"z"] )
                {
                    if( [NSApp sendAction: @selector(undo:) to: nil from: nil] )
                        return;
                }
            }
            else if( nModMask == (NSCommandKeyMask|NSShiftKeyMask) )
            {
                if( [[pEvent charactersIgnoringModifiers] isEqualToString: @"Z"] )
                {
                    if( [NSApp sendAction: @selector(redo:) to: nil from: nil] )
                        return;
                }
            }
        }
    }
SAL_WNODEPRECATED_DECLARATIONS_POP
    [super sendEvent: pEvent];
}

-(void)sendSuperEvent:(NSEvent*)pEvent
{
    [super sendEvent: pEvent];
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
        const ApplicationEvent* pAppEvent = new ApplicationEvent(ApplicationEvent::Type::Open, aFile);
        AquaSalInstance::aAppEventList.push_back( pAppEvent );
        AquaSalInstance *pInst = GetSalData()->mpInstance;
        if( pInst )
            pInst->TriggerUserEventProcessing();
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
        const OUString aFile( GetOUString( pFile ) );
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
        const ApplicationEvent* pAppEvent = new ApplicationEvent(ApplicationEvent::Type::Open, aFileList);
        AquaSalInstance::aAppEventList.push_back( pAppEvent );
        AquaSalInstance *pInst = GetSalData()->mpInstance;
        if( pInst )
            pInst->TriggerUserEventProcessing();
    }
}

-(BOOL)application: (NSApplication*)app printFile: (NSString*)pFile
{
    (void)app;
    std::vector<OUString> aFile;
    aFile.push_back( GetOUString( pFile ) );
    const ApplicationEvent* pAppEvent = new ApplicationEvent(ApplicationEvent::Type::Print, aFile);
    AquaSalInstance::aAppEventList.push_back( pAppEvent );
    AquaSalInstance *pInst = GetSalData()->mpInstance;
    if( pInst )
        pInst->TriggerUserEventProcessing();
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
    const ApplicationEvent* pAppEvent = new ApplicationEvent(ApplicationEvent::Type::Print, aFileList);
    AquaSalInstance::aAppEventList.push_back( pAppEvent );
    AquaSalInstance *pInst = GetSalData()->mpInstance;
    if( pInst )
        pInst->TriggerUserEventProcessing();
    // we have no back channel here, we have to assume success
    // correct handling would be NSPrintingReplyLater and then send [app replyToOpenOrPrint]
    return NSPrintingSuccess;
}

-(void)applicationWillTerminate: (NSNotification *) aNotification
{
    (void)aNotification;
    sal_detail_deinitialize();
    _Exit(0);
}

-(NSApplicationTerminateReply)applicationShouldTerminate: (NSApplication *) app
{
    (void)app;
    NSApplicationTerminateReply aReply = NSTerminateNow;
    {
        SolarMutexGuard aGuard;

        AquaSalInstance *pInst = GetSalData()->mpInstance;
        SalFrame *pAnyFrame = pInst->anyFrame();
        if( pAnyFrame )
        {
            // the following QueryExit will likely present a message box, activate application
            [NSApp activateIgnoringOtherApps: YES];
            aReply = pAnyFrame->CallCallback( SalEvent::Shutdown, nullptr ) ? NSTerminateCancel : NSTerminateNow;
        }

        if( aReply == NSTerminateNow )
        {
            ApplicationEvent aEv(ApplicationEvent::Type::PrivateDoShutdown);
            GetpApp()->AppEvent( aEv );
            ImageTree::get().shutdown();
            // DeInitVCL should be called in ImplSVMain - unless someone exits first which
            // can occur in Desktop::doShutdown for example
        }
    }

    return aReply;
}

-(void)systemColorsChanged: (NSNotification*) pNotification
{
    (void)pNotification;
    SolarMutexGuard aGuard;

    AquaSalInstance *pInst = GetSalData()->mpInstance;
    SalFrame *pAnyFrame = pInst->anyFrame();
    if(  pAnyFrame )
        pAnyFrame->CallCallback( SalEvent::SettingsChanged, nullptr );
}

-(void)screenParametersChanged: (NSNotification*) pNotification
{
    (void)pNotification;
    SolarMutexGuard aGuard;

    for( auto pSalFrame : GetSalData()->mpInstance->getFrames() )
    {
        AquaSalFrame *pFrame = static_cast<AquaSalFrame*>( pSalFrame );
        pFrame->screenParametersChanged();
    }
}

-(void)scrollbarVariantChanged: (NSNotification*) pNotification
{
    (void)pNotification;
    GetSalData()->mpInstance->delayedSettingsChanged( true );
}

-(void)scrollbarSettingsChanged: (NSNotification*) pNotification
{
    (void)pNotification;
    GetSalData()->mpInstance->delayedSettingsChanged( false );
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
    AppleRemoteMainController* pAppleRemoteCtrl = pSalData->mpAppleRemoteMainController;
    if( pAppleRemoteCtrl && pAppleRemoteCtrl->remoteControl)
    {
        // [remoteControl startListening: self];
        // does crash because the right thing to do is
        // [pAppleRemoteCtrl->remoteControl startListening: self];
        // but the instance variable 'remoteControl' is declared protected
        // workaround : declare remoteControl instance variable as public in RemoteMainController.m

        [pAppleRemoteCtrl->remoteControl startListening: self];
#ifdef DEBUG
        NSLog(@"Apple Remote will become active - Using remote controls");
#endif
    }
    for( std::list< AquaSalFrame* >::const_iterator it = pSalData->maPresentationFrames.begin();
         it != pSalData->maPresentationFrames.end(); ++it )
    {
        NSWindow* pNSWindow = (*it)->getNSWindow();
        [pNSWindow setLevel: NSPopUpMenuWindowLevel];
        if( [pNSWindow isVisible] )
            [pNSWindow orderFront: NSApp];
    }
}

- (void)applicationWillResignActive:(NSNotification *)pNotification
{
    (void)pNotification;
    SalData* pSalData = GetSalData();
    AppleRemoteMainController* pAppleRemoteCtrl = pSalData->mpAppleRemoteMainController;
    if( pAppleRemoteCtrl && pAppleRemoteCtrl->remoteControl)
    {
        // [remoteControl stopListening: self];
        // does crash because the right thing to do is
        // [pAppleRemoteCtrl->remoteControl stopListening: self];
        // but the instance variable 'remoteControl' is declared protected
        // workaround : declare remoteControl instance variable as public in RemoteMainController.m

        [pAppleRemoteCtrl->remoteControl stopListening: self];
#ifdef DEBUG
        NSLog(@"Apple Remote will resign active - Releasing remote controls");
#endif
    }
    for( std::list< AquaSalFrame* >::const_iterator it = pSalData->maPresentationFrames.begin();
         it != pSalData->maPresentationFrames.end(); ++it )
    {
        [(*it)->getNSWindow() setLevel: NSNormalWindowLevel];
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
