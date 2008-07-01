/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salnsmenu.mm,v $
 * $Revision: 1.5 $
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
                DBG_ERROR( "unconnected menu" );
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
        SalMenuEvent aMenuEvt;
        aMenuEvt.mnId   = mpMenuItem->mnId;
        aMenuEvt.mpMenu = mpMenuItem->mpVCLMenu;
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
            DBG_ERROR( "menubar item without frame !" );
    }
}
@end

