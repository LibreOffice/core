/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <sfx2/app.hxx>
#include <sfx2/tbxctrl.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <tools/gen.hxx>
#include <svl/intitem.hxx>
#include <sot/exchange.hxx>
#include <svl/eitem.hxx>
#include <vcl/toolbox.hxx>
#include <svx/clipboardctl.hxx>
#include <svx/clipfmtitem.hxx>

#include <svtools/insdlg.hxx>
#include <svx/svxids.hrc>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

/////////////////////////////////////////////////////////////////

SFX_IMPL_TOOLBOX_CONTROL( SvxClipBoardControl, SfxVoidItem /*SfxUInt16Item*/ );


SvxClipBoardControl::SvxClipBoardControl(
        sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :

    SfxToolBoxControl( nSlotId, nId, rTbx ),
    pClipboardFmtItem( 0 ),
     pPopup     (0),
    nItemId     (nId),
    bDisabled( sal_False )
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
    const SvxClipboardFmtItem* pFmtItem = dynamic_cast< const SvxClipboardFmtItem* >( pClipboardFmtItem );
    if ( pFmtItem )
    {
        if (pPopup)
            pPopup->Clear();
        else
            pPopup = new PopupMenu;

        sal_uInt16 nCount = pFmtItem->Count();
        for (sal_uInt16 i = 0;  i < nCount;  ++i)
        {
            sal_uIntPtr nFmtID =  pFmtItem->GetClipbrdFormatId( i );
            String aFmtStr( pFmtItem->GetClipbrdFormatName( i ) );
            if (!aFmtStr.Len())
              aFmtStr = SvPasteObjectHelper::GetSotFormatUIName( nFmtID );
            pPopup->InsertItem( (sal_uInt16)nFmtID, aFmtStr );
        }

        ToolBox& rBox = GetToolBox();
        sal_uInt16 nId = GetId();
        rBox.SetItemDown( nId, sal_True );

        pPopup->Execute( &rBox, rBox.GetItemRect( nId ),
            (rBox.GetAlign() == WINDOWALIGN_TOP || rBox.GetAlign() == WINDOWALIGN_BOTTOM) ?
                POPUPMENU_EXECUTE_DOWN : POPUPMENU_EXECUTE_RIGHT );

        rBox.SetItemDown( nId, sal_False );

        SfxUInt32Item aItem( SID_CLIPBOARD_FORMAT_ITEMS, pPopup->GetCurItemId() );

        Any a;
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SelectedFormat" ));
        aItem.QueryValue( a );
        aArgs[0].Value = a;
        Dispatch( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ClipboardFormatItems" )),
                  aArgs );
    }

    GetToolBox().EndSelection();
    DelPopup();
    return 0;
}


SfxPopupWindowType SvxClipBoardControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONTIMEOUT;
}


void SvxClipBoardControl::StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    if ( SID_CLIPBOARD_FORMAT_ITEMS == nSID )
    {
        DELETEZ( pClipboardFmtItem );
        if ( eState >= SFX_ITEM_AVAILABLE )
        {
            pClipboardFmtItem = pState->Clone();
            GetToolBox().SetItemBits( GetId(), GetToolBox().GetItemBits( GetId() ) | TIB_DROPDOWN );
        }
        else if ( !bDisabled )
            GetToolBox().SetItemBits( GetId(), GetToolBox().GetItemBits( GetId() ) & ~TIB_DROPDOWN );
        GetToolBox().Invalidate( GetToolBox().GetItemRect( GetId() ) );
    }
    else
    {
        // enable the item as a whole
        bDisabled = (GetItemState(pState) == SFX_ITEM_DISABLED);
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

