/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salnsmenu.h,v $
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

#ifndef _VCL_SALNSMENU_H
#define _VCL_SALNSMENU_H

class AquaSalMenu;
class AquaSalMenuItem;

@interface OOStatusItemView : NSView
{
}
-(void)drawRect: (NSRect)aRect;
-(void)layout;
-(void)mouseUp: (NSEvent *)pEvent;
@end

@interface SalNSMenu : NSMenu
{
    /* Caution: SalNSMenu instances occasionally are binary copied
       in AquaSalMenu::ShowNativePopupMenu. If any members are added,
       please take this into account !
    */
    AquaSalMenu*        mpMenu;
}
-(id)initWithMenu: (AquaSalMenu*)pMenu;
-(void)menuNeedsUpdate: (NSMenu*)pMenu;
-(void)setSalMenu: (AquaSalMenu*)pMenu;
@end

@interface SalNSMenuItem : NSMenuItem
{
    /* Caution: SalNSMenuItem instances occasionally are binary copied
       in AquaSalMenu::ShowNativePopupMenu. If any members are added,
       please take this into account !
    */
    AquaSalMenuItem*    mpMenuItem;
}
-(id)initWithMenuItem: (AquaSalMenuItem*)pMenuItem;
-(void)menuItemTriggered: (id)aSender;
@end


#endif
