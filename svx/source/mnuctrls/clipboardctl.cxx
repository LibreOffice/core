/*************************************************************************
 *
 *  $RCSfile: clipboardctl.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 13:17:35 $
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
#ifndef _PASTEDLG_HXX
#include <so3/pastedlg.hxx>
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

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

/////////////////////////////////////////////////////////////////

SFX_IMPL_TOOLBOX_CONTROL( SvxClipBoardControl, SfxVoidItem /*SfxUInt16Item*/ );


SvxClipBoardControl::SvxClipBoardControl(
        USHORT nSlotId, USHORT nId, ToolBox& rTbx ) :

    SfxToolBoxControl( nSlotId, nId, rTbx ),
    pPopup      (0),
    nItemId     (nId),
    pClipboardFmtItem( 0 )
{
    addStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ClipboardFormatItems" )));
    ToolBox& rBox = GetToolBox();
    rBox.SetItemBits( nId, TIB_DROPDOWN | rBox.GetItemBits( nId ) );
    rBox.Invalidate();
}


SvxClipBoardControl::~SvxClipBoardControl()
{
    DelPopup();
    delete pClipboardFmtItem;
}


SfxPopupWindow* SvxClipBoardControl::CreatePopupWindow()
{
    const SvxClipboardFmtItem* pFmtItem = PTR_CAST( SvxClipboardFmtItem, pClipboardFmtItem );
    if ( pFmtItem )
    {
        if (pPopup)
            pPopup->Clear();
        else
            pPopup = new PopupMenu;

        USHORT nCount = pFmtItem->Count();
        for (USHORT i = 0;  i < nCount;  ++i)
        {
            ULONG nFmtID =  pFmtItem->GetClipbrdFormatId( i );
            String aFmtStr( pFmtItem->GetClipbrdFormatName( i ) );
            if (!aFmtStr.Len())
                aFmtStr = SvPasteObjectDialog::GetSotFormatUIName( nFmtID );
            pPopup->InsertItem( (USHORT)nFmtID, aFmtStr );
        }

        ToolBox& rBox = GetToolBox();
        USHORT nId = GetId();
        rBox.SetItemDown( nId, TRUE );

        pPopup->Execute( &rBox, rBox.GetItemRect( nId ),
            (rBox.GetAlign() == WINDOWALIGN_TOP || rBox.GetAlign() == WINDOWALIGN_BOTTOM) ?
                POPUPMENU_EXECUTE_DOWN : POPUPMENU_EXECUTE_RIGHT );

        rBox.SetItemDown( nId, FALSE );

        SfxUInt32Item aItem( SID_CLIPBOARD_FORMAT_ITEMS, pPopup->GetCurItemId() );

        Any a;
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SelectedFormat" ));
        aItem.QueryValue( a );
        aArgs[0].Value = a;
        Dispatch( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ClipboardFormatItems" )),
                  aArgs );
/*
        const SfxPoolItem* pArgs[] =
        {
            &aItem, NULL
        };
        GetBindings().ExecuteSynchron( SID_CLIPBOARD_FORMAT_ITEMS, pArgs );
*/
    }

    GetToolBox().EndSelection();
    DelPopup();
    return 0;
}


SfxPopupWindowType SvxClipBoardControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONTIMEOUT;
}


void SvxClipBoardControl::StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    if ( SID_CLIPBOARD_FORMAT_ITEMS == nSID )
    {
        DELETEZ( pClipboardFmtItem );
        if ( eState >= SFX_ITEM_AVAILABLE )
        {
            pClipboardFmtItem = pState->Clone();
            GetToolBox().SetItemBits( GetId(), GetToolBox().GetItemBits( GetId() ) | TIB_DROPDOWN );
        }
        else
            GetToolBox().SetItemBits( GetId(), GetToolBox().GetItemBits( GetId() ) & ~TIB_DROPDOWN );
        GetToolBox().Invalidate( GetToolBox().GetItemRect( GetId() ) );
    }
    else
    {
        // enable the item as a whole
        GetToolBox().EnableItem( GetId(), (GetItemState(pState) != SFX_ITEM_DISABLED) );
    }
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

