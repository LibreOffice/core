/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: objmnctl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:07:47 $
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
#ifndef _SFX_OBJMNCTL_HXX
#define _SFX_OBJMNCTL_HXX


#ifndef _SFX_HXX
#include "mnuitem.hxx"
#endif

class SfxBindings;
class PopupMenu;
class Menu;

class SfxObjectVerbsControl: public SfxMenuControl
{
    PopupMenu*          pMenu;
    Menu&               rParent;

private:
    void                FillMenu();
    DECL_LINK( MenuSelect, Menu * );
    virtual             void StateChanged( USHORT, SfxItemState, const SfxPoolItem* pState );

public:
                        SfxObjectVerbsControl(USHORT, Menu&, SfxBindings&);
                        ~SfxObjectVerbsControl();
    virtual PopupMenu*  GetPopup() const;
    SFX_DECL_MENU_CONTROL();
};

#endif
