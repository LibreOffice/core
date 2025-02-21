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
#include <osl/diagnose.h>

#include <vcl/window.hxx>

#include <osx/salinst.h>
#include <osx/saldata.hxx>
#include <osx/salframe.h>
#include <osx/salframeview.h>
#include <osx/salmenu.h>
#include <osx/salnsmenu.h>

@implementation SalNSMenu

+(BOOL)dispatchSpecialKeyEquivalents: (NSEvent*)pEvent
{
    if( pEvent && [pEvent type] == NSEventTypeKeyDown )
    {
        // Related tdf#126638 and tdf#162010: match against -[NSEvent characters]
        // When using some non-Western European keyboard layouts, the event's
        // "characters ignoring modifiers" will be set to the original Unicode
        // character instead of the resolved key equivalent character so match
        // against the -[NSEvent characters] instead.
        NSEventModifierFlags nModMask = ([pEvent modifierFlags] & (NSEventModifierFlagShift|NSEventModifierFlagControl|NSEventModifierFlagOption|NSEventModifierFlagCommand));
        NSString *pCharacters = [pEvent characters];
        if( nModMask == NSEventModifierFlagCommand )
        {
            if( [pCharacters isEqualToString: @"v"] )
            {
                if( [NSApp sendAction: @selector(paste:) to: nil from: nil] )
                    return YES;
            }
            else if( [pCharacters isEqualToString: @"c"] )
            {
                if( [NSApp sendAction: @selector(copy:) to: nil from: nil] )
                    return YES;
            }
            else if( [pCharacters isEqualToString: @"x"] )
            {
                if( [NSApp sendAction: @selector(cut:) to: nil from: nil] )
                    return YES;
            }
            else if( [pCharacters isEqualToString: @"a"] )
            {
                if( [NSApp sendAction: @selector(selectAll:) to: nil from: nil] )
                    return YES;
            }
            else if( [pCharacters isEqualToString: @"z"] )
            {
                if( [NSApp sendAction: @selector(undo:) to: nil from: nil] )
                    return YES;
            }
        }
        else if( nModMask == (NSEventModifierFlagCommand|NSEventModifierFlagShift) )
        {
            if( [pCharacters isEqualToString: @"z"] || [pCharacters isEqualToString: @"Z"] )
            {
                if( [NSApp sendAction: @selector(redo:) to: nil from: nil] )
                    return YES;
            }
        }
    }

    return NO;
}

-(id)initWithMenu: (AquaSalMenu*)pMenu
{
    mpMenu = pMenu;
    return [super initWithTitle: [NSString string]];
}

-(void)menuNeedsUpdate: (NSMenu*)pMenu
{
    SolarMutexGuard aGuard;

    if( mpMenu )
    {
        const AquaSalFrame* pFrame = mpMenu->getFrame();
        if( pFrame && AquaSalFrame::isAlive( pFrame ) )
        {
            SalMenuEvent aMenuEvt;
            aMenuEvt.mnId   = 0;
            aMenuEvt.mpMenu = mpMenu->mpVCLMenu;
            if( aMenuEvt.mpMenu )
            {
                pFrame->CallCallback(SalEvent::MenuActivate, &aMenuEvt);
                pFrame->CallCallback(SalEvent::MenuDeactivate, &aMenuEvt);
            }
            else
                OSL_FAIL( "unconnected menu" );
        }
        else if( mpMenu->mpVCLMenu )
        {
            mpMenu->mpVCLMenu->Activate();
            mpMenu->mpVCLMenu->Deactivate();

            // Hide disabled items
            NSArray* elements = [pMenu itemArray];
            NSEnumerator* it = [elements objectEnumerator];
            id element;
            while ( ( element = [it nextObject] ) != nil )
            {
                NSMenuItem* item = static_cast< NSMenuItem* >( element );
                if( ![item isSeparatorItem] )
                    [item setHidden: ![item isEnabled]];
            }
        }
    }
}

-(void)setSalMenu: (AquaSalMenu*)pMenu
{
    mpMenu = pMenu;
}
@end

@implementation SalNSMenuItem
-(id)initWithMenuItem: (AquaSalMenuItem*)pMenuItem
{
    mpMenuItem = pMenuItem;
    id ret = [super initWithTitle: [NSString string]
                    action: @selector(menuItemTriggered:)
                    keyEquivalent: [NSString string]];
    [ret setTarget: self];
    mbReallyEnabled = [ret isEnabled];
    return ret;
}

-(BOOL)isReallyEnabled
{
    return mbReallyEnabled;
}

-(void)menuItemTriggered: (id)aSender
{
    (void)aSender;
    SolarMutexGuard aGuard;

    // Commit uncommitted text before dispatching the selecting menu item. In
    // certain cases such as selecting the Insert > Comment menu item in a
    // Writer document while there is uncommitted text will call
    // AquaSalFrame::EndExtTextInput() which will dispatch a
    // SalEvent::EndExtTextInput event. Writer's handler for that event will
    // delete the uncommitted text and then insert the committed text but
    // LibreOffice will crash when deleting the uncommitted text because
    // deletion of the text also removes and deletes the newly inserted
    // comment.
    NSWindow* pKeyWin = [NSApp keyWindow];
    if( pKeyWin && [pKeyWin isKindOfClass: [SalFrameWindow class]] )
        [static_cast<SalFrameWindow*>(pKeyWin) endExtTextInput];

    // tdf#49853 Keyboard shortcuts are also handled by the menu bar, but at least some of them
    // must still end up in the view. This is necessary to handle common edit actions in docked
    // windows (e.g. in toolbar fields).
    if( pKeyWin )
    {
        // tdf#162010 match based on key equivalent instead of key event
        // The original fix for tdf#49853 only looked worked in the
        // case when both a key shortcut was pressed and the resulting
        // key event was not an input method event. If either of these
        // conditions weren't true, tdf#49853 would still occur.
        // Since we know which menu item is being triggered, check if
        // this menu item's key equivalent has been set to one of the
        // edit actions.
        // This change basically expands the fix for tdf#49853 to
        // include all cases that trigger a menu item, not just simple
        // key events.
        NSEventModifierFlags nModMask = [self keyEquivalentModifierMask];
        NSString* pCharacters = [self keyEquivalent];
        if( nModMask == NSEventModifierFlagCommand &&
          ( [pCharacters isEqualToString: @"v"] ||
            [pCharacters isEqualToString: @"c"] ||
            [pCharacters isEqualToString: @"x"] ||
            [pCharacters isEqualToString: @"a"] ||
            [pCharacters isEqualToString: @"z"] ) )
        {
            NSEvent* pKeyEvent = nil;
            NSEvent* pEvent = [NSApp currentEvent];
            if( pEvent )
            {
                switch( [pEvent type] )
                {
                    case NSEventTypeKeyDown:
                    case NSEventTypeKeyUp:
                    case NSEventTypeFlagsChanged:
                        // tdf#162843 replace the event's string parameters
                        // When using the Dvorak - QWERTY keyboard, the
                        // event's charactersIgnoringModifiers string causes
                        // pasting to fail so replace both the event's
                        // characters and charactersIgnoringModifiers strings
                        // with this menu item's key equivalent.
                        pKeyEvent = [NSEvent keyEventWithType: [pEvent type] location: [pEvent locationInWindow] modifierFlags: nModMask timestamp: [pEvent timestamp] windowNumber: [pEvent windowNumber] context: nil characters: pCharacters charactersIgnoringModifiers: pCharacters isARepeat: [pEvent isARepeat] keyCode: [pEvent keyCode]];
                        break;
                    default:
                        break;
                }
            }

            if( !pKeyEvent )
            {
                // Native key events appear to set the location to the
                // top left corner of the key window
                NSPoint aPoint = NSMakePoint(0, [pKeyWin frame].size.height);
                pKeyEvent = [NSEvent keyEventWithType: NSEventTypeKeyDown location: aPoint modifierFlags: nModMask timestamp: [[NSProcessInfo processInfo] systemUptime] windowNumber: [pKeyWin windowNumber] context: nil characters: pCharacters charactersIgnoringModifiers: pCharacters isARepeat: NO keyCode: 0];
            }

            [[pKeyWin contentView] keyDown: pKeyEvent];
            return;
        }
    }

    const AquaSalFrame* pFrame = mpMenuItem->mpParentMenu ? mpMenuItem->mpParentMenu->getFrame() : nullptr;
    if( pFrame && AquaSalFrame::isAlive( pFrame ) && ! pFrame->GetWindow()->IsInModalMode() )
    {
        SalMenuEvent aMenuEvt( mpMenuItem->mnId, mpMenuItem->mpVCLMenu );
        pFrame->CallCallback(SalEvent::MenuCommand, &aMenuEvt);
    }
    else if( mpMenuItem->mpVCLMenu )
    {
        // if an item from submenu was selected. the corresponding Window does not exist because
        // we use native popup menus, so we have to set the selected menuitem directly
        // incidentally this of course works for top level popup menus, too
        PopupMenu * pPopupMenu = dynamic_cast<PopupMenu *>(mpMenuItem->mpVCLMenu.get());
        if( pPopupMenu )
        {
            // FIXME: revise this ugly code

            // select handlers in vcl are dispatch on the original menu
            // if not consumed by the select handler of the current menu
            // however since only the starting menu ever came into Execute
            // the hierarchy is not build up. Workaround this by getting
            // the menu it should have been

            // get started from hierarchy in vcl menus
            AquaSalMenu* pParentMenu = mpMenuItem->mpParentMenu;
            Menu* pCurMenu = mpMenuItem->mpVCLMenu;
            while( pParentMenu && pParentMenu->mpVCLMenu )
            {
                pCurMenu = pParentMenu->mpVCLMenu;
                pParentMenu = pParentMenu->mpParentSalMenu;
            }

            pPopupMenu->SetSelectedEntry( mpMenuItem->mnId );
            pPopupMenu->ImplSelectWithStart( pCurMenu );
        }
        else
            OSL_FAIL( "menubar item without frame !" );
    }
}

-(void)setReallyEnabled: (BOOL)bEnabled
{
    mbReallyEnabled = bEnabled;
    [self setEnabled: mbReallyEnabled];
}

-(BOOL)validateMenuItem: (NSMenuItem *)pMenuItem
{
    // Related: tdf#126638 disable all menu items when displaying modal windows
    // For some unknown reason, key shortcuts are dispatched to the LibreOffice
    // menu items instead of the modal window so disable all LibreOffice menu
    // items while a native modal dialog such as the native Open, Save, or
    // Print dialog is displayed.
    if (!pMenuItem || [NSApp modalWindow])
        return NO;

    // Related: tdf#161623 the menubar is always visible when in native
    // full screen mode so disable all menu items when also in LibreOffice
    // full screen mode to mimic the effect of a hidden menubar.
    SolarMutexGuard aGuard;
    const AquaSalFrame* pFrame = mpMenuItem->mpParentMenu ? mpMenuItem->mpParentMenu->getFrame() : nullptr;
    if (pFrame && AquaSalFrame::isAlive( pFrame ) && pFrame->mbInternalFullScreen)
    {
        NSMenu *pMainMenu = [NSApp mainMenu];
        NSMenu *pParentMenu = [pMenuItem menu];
        while (pParentMenu && pParentMenu != pMainMenu)
            pParentMenu = [pParentMenu supermenu];
        if (pParentMenu && pParentMenu == pMainMenu)
            return NO;
    }

    // Related: tdf#126638 return the last enabled state set by the LibreOffice code
    // Apparently whatever is returned will be passed to
    // -[NSMenuItem setEnabled:] which can cause the enabled state
    // to be different than the enabled state that the LibreOffice
    // code expoects. This results in menu items failing to be
    // reenabled after being temporarily disabled such as when a
    // native modal dialog is closed. So, return the last enabled
    // state set by the LibreOffice code.
    if ([pMenuItem isKindOfClass: [SalNSMenuItem class]])
        return [static_cast<SalNSMenuItem*>(pMenuItem) isReallyEnabled];
    else
        return [pMenuItem isEnabled];
}
@end

@implementation OOStatusItemView
-(void)drawRect: (NSRect)aRect
{
    NSGraphicsContext* pContext = [NSGraphicsContext currentContext];
    [pContext saveGraphicsState];
SAL_WNODEPRECATED_DECLARATIONS_PUSH
        // "'drawStatusBarBackgroundInRect:withHighlight:' is deprecated: first deprecated in macOS
        // 10.14 - Use the standard button instead which handles highlight drawing, making this
        // method obsolete"
    [SalData::getStatusItem() drawStatusBarBackgroundInRect: aRect withHighlight: NO];
SAL_WNODEPRECATED_DECLARATIONS_POP
    if( AquaSalMenu::pCurrentMenuBar )
    {
        const std::vector< AquaSalMenu::MenuBarButtonEntry >& rButtons( AquaSalMenu::pCurrentMenuBar->getButtons() );
        NSRect aFrame = [self frame];
        NSRect aImgRect = { { 2, 0 }, { 0, 0 } };
        for( size_t i = 0; i < rButtons.size(); ++i )
        {
            const Size aPixSize = rButtons[i].maButton.maImage.GetSizePixel();
            const NSRect aFromRect = { NSZeroPoint, NSMakeSize( aPixSize.Width(), aPixSize.Height()) };
            aImgRect.origin.y = floor((aFrame.size.height - aFromRect.size.height)/2);
            aImgRect.size = aFromRect.size;
            if( rButtons[i].mpNSImage )
                [rButtons[i].mpNSImage drawInRect: aImgRect fromRect: aFromRect operation: NSCompositingOperationSourceOver fraction: 1.0];
            aImgRect.origin.x += aFromRect.size.width + 2;
        }
    }
    [pContext restoreGraphicsState];
}

-(void)mouseUp: (NSEvent *)pEvent
{
    /* check if button goes up inside one of our status buttons */
    if( AquaSalMenu::pCurrentMenuBar )
    {
        const std::vector< AquaSalMenu::MenuBarButtonEntry >& rButtons( AquaSalMenu::pCurrentMenuBar->getButtons() );
        NSRect aFrame = [self frame];
        NSRect aImgRect = { { 2, 0 }, { 0, 0 } };
        NSPoint aMousePt = [pEvent locationInWindow];
        for( size_t i = 0; i < rButtons.size(); ++i )
        {
            const Size aPixSize = rButtons[i].maButton.maImage.GetSizePixel();
            const NSRect aFromRect = { NSZeroPoint, NSMakeSize( aPixSize.Width(), aPixSize.Height()) };
            aImgRect.origin.y = (aFrame.size.height - aFromRect.size.height)/2;
            aImgRect.size = aFromRect.size;
            if( aMousePt.x >= aImgRect.origin.x && aMousePt.x <= (aImgRect.origin.x+aImgRect.size.width) &&
                aMousePt.y >= aImgRect.origin.y && aMousePt.y <= (aImgRect.origin.y+aImgRect.size.height) )
            {
                if( AquaSalMenu::pCurrentMenuBar->mpFrame && AquaSalFrame::isAlive( AquaSalMenu::pCurrentMenuBar->mpFrame ) )
                {
                    SalMenuEvent aMenuEvt( rButtons[i].maButton.mnId, AquaSalMenu::pCurrentMenuBar->mpVCLMenu );
                    AquaSalMenu::pCurrentMenuBar->mpFrame->CallCallback(SalEvent::MenuButtonCommand, &aMenuEvt);
                }
                return;
            }

            aImgRect.origin.x += aFromRect.size.width + 2;
        }
    }
}

-(void)layout
{
    NSStatusBar* pStatBar = [NSStatusBar systemStatusBar];
    NSSize aSize = { 0, [pStatBar thickness] };
    [self removeAllToolTips];
    if( AquaSalMenu::pCurrentMenuBar )
    {
        const std::vector< AquaSalMenu::MenuBarButtonEntry >& rButtons( AquaSalMenu::pCurrentMenuBar->getButtons() );
        if( ! rButtons.empty() )
        {
            aSize.width = 2;
            for( size_t i = 0; i < rButtons.size(); ++i )
            {
                NSRect aImgRect = { { aSize.width,
                                      static_cast<CGFloat>(floor((aSize.height-rButtons[i].maButton.maImage.GetSizePixel().Height())/2)) },
                                    { static_cast<CGFloat>(rButtons[i].maButton.maImage.GetSizePixel().Width()),
                                      static_cast<CGFloat>(rButtons[i].maButton.maImage.GetSizePixel().Height()) } };
                if( rButtons[i].mpToolTipString )
                    [self addToolTipRect: aImgRect owner: rButtons[i].mpToolTipString userData: nullptr];
                aSize.width += 2 + aImgRect.size.width;
            }
        }
    }
    [self setFrameSize: aSize];
}
@end

@implementation SalNSMainMenu

- (id)initWithTitle:(NSString*)pTitle
{
    mpLastPerformKeyEquivalentEvent = nil;
    return [super initWithTitle:pTitle];
}

- (void)dealloc
{
    if (mpLastPerformKeyEquivalentEvent)
        [mpLastPerformKeyEquivalentEvent release];

    [super dealloc];
}

- (BOOL)performKeyEquivalent:(NSEvent*)pEvent
{
    // Related: tdf#162843 prevent dispatch of the same event more than once
    // When pressing Command-V with a Dvorak - QWERTY keyboard,
    // that single event passes through this selector twice which
    // causes content to be pasted twice in any text fields in the
    // Find and Replace dialog.
    if (pEvent == mpLastPerformKeyEquivalentEvent)
        return false;

    if (mpLastPerformKeyEquivalentEvent)
        [mpLastPerformKeyEquivalentEvent release];
    mpLastPerformKeyEquivalentEvent = pEvent;
    if (mpLastPerformKeyEquivalentEvent)
        [mpLastPerformKeyEquivalentEvent retain];

    bool bRet = [super performKeyEquivalent: pEvent];

    // tdf#126638 dispatch key shortcut events to modal windows
    // Some modal windows, such as the native Open and Save dialogs,
    // return NO from -[NSWindow performKeyEquivalent:]. Fortunately,
    // the main menu's -[NSMenu performKeyEquivalent:] is then called
    // so we can catch and redirect any modal window's key shortcut
    // events without triggering the modal window's "disallowed
    // action" beep.
    if( !bRet && [NSApp modalWindow] )
        bRet = [SalNSMenu dispatchSpecialKeyEquivalents: pEvent];

    return bRet;
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
