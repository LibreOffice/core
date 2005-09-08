/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: zoomctrl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:53:29 $
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
#ifndef _SVX_ZOOMCTRL_HXX
#define _SVX_ZOOMCTRL_HXX

// include ---------------------------------------------------------------

#ifndef _SFXSTBITEM_HXX //autogen
#include <sfx2/stbitem.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

// class SvxZoomToolBoxControl -------------------------------------------

class SVX_DLLPUBLIC SvxZoomStatusBarControl : public SfxStatusBarControl
{
private:
    USHORT  nZoom;
    USHORT  nValueSet;

public:
    virtual void    StateChanged( USHORT nSID, SfxItemState eState,
                                  const SfxPoolItem* pState );
    virtual void    Paint( const UserDrawEvent& rEvt );
    virtual void    Command( const CommandEvent& rCEvt );

    SFX_DECL_STATUSBAR_CONTROL();

    SvxZoomStatusBarControl( USHORT nSlotId, USHORT nId, StatusBar& rStb );

    static  ULONG   GetDefItemWidth(const StatusBar& rStb);

};


#endif

