/*************************************************************************
 *
 *  $RCSfile: clipboardctl.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tl $ $Date: 2001-03-30 14:23:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFXTBXCTRL_HXX
#include <sfx2/tbxctrl.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef _SOT_EXCHANGE_HXX
#include <sot/exchange.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif

#ifndef _SVX_CLIPBOARDCTL_HXX_
#include <clipboardctl.hxx>
#endif
#ifndef _SVX_CLIPFMTITEM_HXX
#include <clipfmtitem.hxx>
#endif

#include <svxids.hrc>

/////////////////////////////////////////////////////////////////

SFX_IMPL_TOOLBOX_CONTROL( SvxClipBoardControl, SfxBoolItem /*SfxUInt16Item*/ );


SvxClipBoardControl::SvxClipBoardControl(
        USHORT nId, ToolBox& rTbx, SfxBindings& rBind ) :

    SfxToolBoxControl( nId, rTbx, rBind ),
    pPopup      (0),
    nItemId     (nId)
{
    ToolBox& rBox = GetToolBox();
    rBox.SetItemBits( nId, TIB_DROPDOWN | rBox.GetItemBits( nId ) );
    rBox.Invalidate();
}


SvxClipBoardControl::~SvxClipBoardControl()
{
    DelPopup();
}


SfxPopupWindow* SvxClipBoardControl::CreatePopupWindow()
{
    SfxPoolItem* pState = 0;
    SfxDispatcher &rDispatch = *GetBindings().GetDispatcher();
    SfxItemState eState = rDispatch.QueryState( SID_CLIPBOARD_FORMAT_ITEMS,
                                                pState );
    if (eState >= SFX_ITEM_AVAILABLE  &&  pState)
    {
        if (pPopup)
            pPopup->Clear();
        else
            pPopup = new PopupMenu;

        const SvxClipboardFmtItem &rFmtItem = *(SvxClipboardFmtItem *) pState;
        USHORT nCount = rFmtItem.Count();
        for (USHORT i = 0;  i < nCount;  ++i)
        {
            ULONG nFmtID =  rFmtItem.GetClipbrdFormatId( i );
            String aFmtStr( rFmtItem.GetClipbrdFormatName( i ) );
            if (!aFmtStr.Len())
                aFmtStr = SotExchange::GetFormatName( nFmtID );
            pPopup->InsertItem( (USHORT)nFmtID, aFmtStr );
        }

        ToolBox& rBox = GetToolBox();
        USHORT nId = GetId();
        rBox.SetItemDown( nId, TRUE );

        //Point aPt( rBox.OutputToScreenPixel( rBox.GetPointerPosPixel() ) );
        Point aPt( rBox.OutputToScreenPixel(
                        rBox.GetItemRect( nItemId ).BottomLeft() ) );
        pPopup->Execute( &rBox, rBox.ScreenToOutputPixel( aPt ) );
        rBox.SetItemDown( nId, FALSE );

        SfxUInt32Item aItem( SID_CLIPBOARD_FORMAT_ITEMS, pPopup->GetCurItemId() );
        rDispatch.Execute( SID_CLIPBOARD_FORMAT_ITEMS,
                           SFX_CALLMODE_SYNCHRON, &aItem, 0L );
    }

    GetToolBox().EndSelection();
    DelPopup();
    return 0;
}


SfxPopupWindowType SvxClipBoardControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONTIMEOUT;
}


void SvxClipBoardControl::StateChanged(
        USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    GetToolBox().EnableItem( GetId(), (GetItemState(pState) != SFX_ITEM_DISABLED) );
}


void SvxClipBoardControl::DelPopup()
{
    if(pPopup)
    {
        delete pPopup;
        pPopup = 0;
    }
}


/////////////////////////////////////////////////////////////////

