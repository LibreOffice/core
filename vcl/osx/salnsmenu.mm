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

#include <vcl/window.hxx>

#include "osx/salinst.h"
#include "osx/saldata.hxx"
#include "osx/salframe.h"
#include "osx/salmenu.h"
#include "osx/salnsmenu.h"

@implementation SalNSMenu
-(id)initWithMenu: (AquaSalMenu*)pMenu
{
    mpMenu = pMenu;
    return [super initWithTitle: [NSString string]];
}

-(void)menuNeedsUpdate: (NSMenu*)pMenu
{
    (void)pMenu;
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
                pFrame->CallCallback(SALEVENT_MENUACTIVATE, &aMenuEvt);
                pFrame->CallCallback(SALEVENT_MENUDEACTIVATE, &aMenuEvt);
            }
            else
                OSL_FAIL( "unconnected menu" );
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
    return ret;
}
-(void)menuItemTriggered: (id)aSender
{
    (void)aSender;
    SolarMutexGuard aGuard;

    const AquaSalFrame* pFrame = mpMenuItem->mpParentMenu ? mpMenuItem->mpParentMenu->getFrame() : nullptr;
    if( pFrame && AquaSalFrame::isAlive( pFrame ) && ! pFrame->GetWindow()->IsInModalMode() )
    {
        SalMenuEvent aMenuEvt( mpMenuItem->mnId, mpMenuItem->mpVCLMenu );
        pFrame->CallCallback(SALEVENT_MENUCOMMAND, &aMenuEvt);
    }
    else if( mpMenuItem->mpVCLMenu )
    {
        // if an item from submenu was selected. the corresponding Window does not exist because
        // we use native popup menus, so we have to set the selected menuitem directly
        // incidentally this of course works for top level popup menus, too
        PopupMenu * pPopupMenu = dynamic_cast<PopupMenu *>(mpMenuItem->mpVCLMenu);
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
@end

@implementation OOStatusItemView
-(void)drawRect: (NSRect)aRect
{
    NSGraphicsContext* pContext = [NSGraphicsContext currentContext];
    [pContext saveGraphicsState];
    [SalData::getStatusItem() drawStatusBarBackgroundInRect: aRect withHighlight: NO];
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
                [rButtons[i].mpNSImage drawInRect: aImgRect fromRect: aFromRect operation: NSCompositeSourceOver fraction: 1.0];
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
                    AquaSalMenu::pCurrentMenuBar->mpFrame->CallCallback(SALEVENT_MENUBUTTONCOMMAND, &aMenuEvt);
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
                NSRect aImgRect = { { static_cast<CGFloat>(aSize.width),
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
