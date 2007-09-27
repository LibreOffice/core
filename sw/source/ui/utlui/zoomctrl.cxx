/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: zoomctrl.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:50:22 $
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
#include "precompiled_sw.hxx"



#include "hintids.hxx"

#ifndef _STATUS_HXX //autogen
#include <vcl/status.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SVX_ZOOMITEM_HXX //autogen
#include <svx/zoomitem.hxx>
#endif

#include "swtypes.hxx"
#include "zoomctrl.hxx"


SFX_IMPL_STATUSBAR_CONTROL( SwZoomControl, SvxZoomItem );


SwZoomControl::SwZoomControl( USHORT _nSlotId,
                              USHORT _nId,
                              StatusBar& rStb ) :
    SvxZoomStatusBarControl( _nSlotId, _nId, rStb )
{
}

// -----------------------------------------------------------------------

SwZoomControl::~SwZoomControl()
{
}

// -----------------------------------------------------------------------

void SwZoomControl::StateChanged( USHORT nSID, SfxItemState eState,
                                  const SfxPoolItem* pState )
{
    if(SFX_ITEM_AVAILABLE == eState && pState->ISA( SfxStringItem ))
    {
        sPreviewZoom = ((const SfxStringItem*)pState)->GetValue();
        GetStatusBar().SetItemText( GetId(), sPreviewZoom );
    }
    else
    {
        sPreviewZoom = aEmptyStr;
        SvxZoomStatusBarControl::StateChanged(nSID, eState, pState);
    }
}

// -----------------------------------------------------------------------

void SwZoomControl::Paint( const UserDrawEvent& rUsrEvt )
{
    if(!sPreviewZoom.Len())
        SvxZoomStatusBarControl::Paint(rUsrEvt);
    else
        GetStatusBar().SetItemText( GetId(), sPreviewZoom );
}

// -----------------------------------------------------------------------

void SwZoomControl::Command( const CommandEvent& rCEvt )
{
    if(!sPreviewZoom.Len())
        SvxZoomStatusBarControl::Command(rCEvt);
}




