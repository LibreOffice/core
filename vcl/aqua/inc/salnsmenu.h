/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salnsmenu.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 16:14:15 $
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

#ifndef _VCL_SALNSMENU_H
#define _VCL_SALNSMENU_H

class AquaSalMenuItem : public SalMenuItem
{
public:
    AquaSalMenuItem( const SalItemParams* );
    virtual ~AquaSalMenuItem();

    USHORT              mnId;                 // Item ID
    Menu*               mpVCLMenu;            // VCL Menu into which this MenuItem is inserted
    AquaSalMenu*        mpParentMenu;         // The menu in which this menu item is inserted
    AquaSalMenu*        mpSubMenu;            // Sub menu of this item (if defined)
    NSMenuItem*         mpMenuItem;           // The NSMenuItem
};

@interface SalNSMenu : NSMenu
{
    AquaSalMenu*        mpMenu;
}
-(id)initWithMenu: (AquaSalMenu*)pMenu;
-(void)menuNeedsUpdate: (NSMenu*)pMenu;
-(void)setSalMenu: (AquaSalMenu*)pMenu;
@end

@interface SalNSMenuItem : NSMenuItem
{
    AquaSalMenuItem*    mpMenuItem;
}
-(id)initWithMenuItem: (AquaSalMenuItem*)pMenuItem;
-(void)menuItemTriggered: (id)aSender;
@end


#endif
