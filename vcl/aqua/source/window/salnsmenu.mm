/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salnsmenu.mm,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 16:19:25 $
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
    id ret = [super initWithTitle: [NSString string] action: nil keyEquivalent: [NSString string]];
    [ret setAction:@selector(menuItemTriggered:)];
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
}
@end

