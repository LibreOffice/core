/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fntszctl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:53:01 $
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
#ifndef _SVX_FNTSZCTL_HXX
#define _SVX_FNTSZCTL_HXX

// include ---------------------------------------------------------------

#ifndef _SFXMNUITEM_HXX //autogen
#include <sfx2/mnuitem.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

#ifdef ITEMID_FONTHEIGHT

class SfxBindings;
class FontSizeMenu;

// class SvxFontSizeMenuControl ------------------------------------------

class SVX_DLLPUBLIC SvxFontSizeMenuControl : public SfxMenuControl
{
private:
    FontSizeMenu*   pMenu;
    Menu&           rParent;
    SfxStatusForwarder  aFontNameForwarder;

//#if 0 // _SOLAR__PRIVATE
    DECL_LINK( MenuSelect, FontSizeMenu * );
//#endif

protected:
    virtual void    StateChanged( USHORT nSID, SfxItemState eState,
                                  const SfxPoolItem* pState );

public:
    SvxFontSizeMenuControl( USHORT nId, Menu&, SfxBindings& );
    ~SvxFontSizeMenuControl();

    virtual PopupMenu*  GetPopup() const;

    SFX_DECL_MENU_CONTROL();
};

#endif

#endif

