/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fntctl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:52:23 $
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
#ifndef _SVX_FNTCTL_HXX
#define _SVX_FNTCTL_HXX

// include ---------------------------------------------------------------

#ifndef _SFXMNUITEM_HXX //autogen
#include <sfx2/mnuitem.hxx>
#endif
#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

#ifdef ITEMID_FONT

class FontNameMenu;
class SfxBindings;

// class SvxFontMenuControl ----------------------------------------------

class SVX_DLLPUBLIC SvxFontMenuControl : public SfxMenuControl, public SfxListener
{
private:
    FontNameMenu*   pMenu;
    Menu&           rParent;

//#if 0 // _SOLAR__PRIVATE
    SVX_DLLPRIVATE void         FillMenu();
    DECL_LINK( MenuSelect, FontNameMenu * );
    SVX_DLLPRIVATE virtual void StateChanged( USHORT nSID, SfxItemState eState,
                                  const SfxPoolItem* pState );
    SVX_DLLPRIVATE virtual void     SFX_NOTIFY( SfxBroadcaster& rBC,
                                const TypeId& rBCType,
                                const SfxHint& rHint,
                                const TypeId& rHintType );
//#endif

public:
    SvxFontMenuControl( USHORT nId, Menu&, SfxBindings& );
    ~SvxFontMenuControl();

    virtual PopupMenu*  GetPopup() const;

    SFX_DECL_MENU_CONTROL();
};

#endif

#endif

