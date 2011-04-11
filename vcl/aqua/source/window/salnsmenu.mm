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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include "salinst.h"
#include "saldata.hxx"
#include "salframe.h"
#include "salmenu.h"
#include "salnsmenu.h"

#include "vcl/window.hxx"
 
@implementation SalNSMenu
-(id)initWithMenu: (AquaSalMenu*)pMenu
{
    mpMenu = pMenu;
    return [super initWithTitle: [NSString string]];
}

-(void)menuNeedsUpdate: (NSMenu*)pMenu
{
    YIELD_GUARD;
    
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
    YIELD_GUARD;
    
    const AquaSalFrame* pFrame = mpMenuItem->mpParentMenu ? mpMenuItem->mpParentMenu->getFrame() : NULL;
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
            NSRect aFromRect = { { 0, 0 },
                                 { rButtons[i].maButton.maImage.GetSizePixel().Width(),
                                   rButtons[i].maButton.maImage.GetSizePixel().Height() } };
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
            NSRect aFromRect = { { 0, 0 },
                                 { rButtons[i].maButton.maImage.GetSizePixel().Width(),
                                   rButtons[i].maButton.maImage.GetSizePixel().Height() } };
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
                NSRect aImgRect = { { aSize.width, floor((aSize.height-rButtons[i].maButton.maImage.GetSizePixel().Height())/2) },
                                     { rButtons[i].maButton.maImage.GetSizePixel().Width(),
                                       rButtons[i].maButton.maImage.GetSizePixel().Height() } };
                if( rButtons[i].mpToolTipString )
                    [self addToolTipRect: aImgRect owner: rButtons[i].mpToolTipString userData: NULL];
                aSize.width += 2 + aImgRect.size.width;
            }
        }
    }
    [self setFrameSize: aSize];
}
@end


