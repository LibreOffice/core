/*************************************************************************
 *
 *  $RCSfile: workctrl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2000-10-23 07:57:09 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include <string> // HACK: prevent conflict between STLPORT and Workshop headers

#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif


#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _INITUI_HXX
#include <initui.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _GLOSHDL_HXX
#include <gloshdl.hxx>
#endif
#ifndef _GLOSSARY_HXX
#include <glossary.hxx>
#endif
#ifndef _GLOSDOC_HXX
#include <glosdoc.hxx>
#endif
#ifndef _GLOSLST_HXX
#include <gloslst.hxx>
#endif
#ifndef _TBXMGR_HXX
#include <tbxmgr.hxx>
#endif
#ifndef _WORKCTRL_HXX
#include <workctrl.hxx>
#endif
#ifndef _WORKCTRL_HRC
#include <workctrl.hrc>
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _WRTSH_HXX //autogen
#include <wrtsh.hxx>
#endif


//JP 14.01.99: Size Abpruefung
#define NAVI_ENTRIES 20
#if NAVI_ENTRIES != NID_COUNT
#error SwScrollNaviPopup-CTOR static Array falsche Size. Wurden neue IDs zugefuegt ??
#endif


SFX_IMPL_TOOLBOX_CONTROL( SwTbxInsertCtrl, SfxUInt16Item);
SFX_IMPL_TOOLBOX_CONTROL( SwTbxAutoTextCtrl, SfxBoolItem );

/**********************************************************************

**********************************************************************/



SwTbxInsertCtrl::SwTbxInsertCtrl( USHORT nId,
                                    ToolBox& rTbx,
                                    SfxBindings& rBind ) :
        SfxToolBoxControl( nId, rTbx, rBind ),
        nLastSlotId(FN_INSERT_CTRL == nId ? FN_INSERT_TABLE : SID_INSERT_DIAGRAM)
{
    Image aImage = SFX_IMAGEMANAGER()->GetImage( nLastSlotId, SW_MOD() );
    rTbx.SetItemImage(GetId(), aImage);
}
/**********************************************************************

**********************************************************************/


SwTbxInsertCtrl::~SwTbxInsertCtrl()
{
}

/**********************************************************************

**********************************************************************/


void SwTbxInsertCtrl::StateChanged( USHORT nSID,
                                      SfxItemState eState,
                                      const SfxPoolItem* pState )
{
    USHORT nId = GetId();
    GetToolBox().EnableItem( nId, (GetItemState(pState) != SFX_ITEM_DISABLED) );

    if( eState == SFX_ITEM_AVAILABLE )
    {
        const SfxUInt16Item* pItem = PTR_CAST( SfxUInt16Item, pState );
        if(pItem)
        {
            nLastSlotId = pItem->GetValue();
            if( nLastSlotId )
                nId = nLastSlotId;
            Image aImage = SFX_IMAGEMANAGER()->GetImage( nId, SW_MOD() );
            GetToolBox().SetItemImage(GetId(), aImage);
        }
    }

}
/**********************************************************************

**********************************************************************/


SfxPopupWindow* SwTbxInsertCtrl::CreatePopupWindow()
{
    USHORT nWinResId, nTbxResId;
    if(GetId() == FN_INSERT_CTRL)
    {
        nWinResId = RID_INSERT_CTRL;
        nTbxResId = TBX_INSERT;
    }
    else /* FN_INSERT_OBJ_CTRL */
    {
        nWinResId = RID_INSERT_OBJ_CTRL;
        nTbxResId = TBX_OBJ_INSERT;
    }

    WindowAlign eAlign = WINDOWALIGN_TOP;
    if(GetToolBox().IsHorizontal())
        eAlign = WINDOWALIGN_LEFT;
    SwPopupWindowTbxMgr *pWin = new SwPopupWindowTbxMgr( GetId(),
                                    eAlign,
                                    SW_RES(nWinResId),
                                    SW_RES(nTbxResId),
                                    GetBindings());

    pWin->StartPopupMode(&GetToolBox(), TRUE);
    pWin->StartSelection();
    pWin->Show();
    return pWin;

}

/**********************************************************************

**********************************************************************/


SfxPopupWindowType  SwTbxInsertCtrl::GetPopupWindowType() const
{
    return nLastSlotId ? SFX_POPUPWINDOW_ONTIMEOUT : SFX_POPUPWINDOW_ONCLICK;
}


/**********************************************************************

**********************************************************************/


void SwTbxInsertCtrl::Select( BOOL bMod1 )
{
    if(nLastSlotId)
        SfxViewFrame::Current()->GetDispatcher()->Execute(nLastSlotId);
}

/**********************************************************************

**********************************************************************/


SwTbxAutoTextCtrl::SwTbxAutoTextCtrl( USHORT nId,
                                    ToolBox& rTbx,
                                    SfxBindings& rBind ) :
    SfxToolBoxControl( nId, rTbx, rBind ),
    pView(0),
    pPopup(0)
{
}
/**********************************************************************

**********************************************************************/


SwTbxAutoTextCtrl::~SwTbxAutoTextCtrl()
{
    DelPopup();
}

/**********************************************************************

**********************************************************************/


SfxPopupWindow* SwTbxAutoTextCtrl::CreatePopupWindow()
{
    pView = ::GetActiveView();
    if(pView && !pView->GetDocShell()->IsReadOnly() &&
       !pView->GetWrtShell().HasReadonlySel() )
    {
        ToolBox& rBox = GetToolBox();
        Point aPt(rBox.OutputToScreenPixel(rBox.GetPointerPosPixel()));
        if(pView)
        {
            Link aLnk = LINK(this, SwTbxAutoTextCtrl, PopupHdl);

            if (GetId() == FN_INSERT_FIELD_CTRL)
            {
                pPopup = new PopupMenu(SW_RES(RID_INSERT_FIELD_CTRL));
                pPopup->SetSelectHdl(aLnk);

                if (::GetHtmlMode(pView->GetDocShell()) & HTMLMODE_ON)
                {
                    pPopup->RemoveItem(pPopup->GetItemPos(FN_INSERT_FLD_PGCOUNT));
                    pPopup->RemoveItem(pPopup->GetItemPos(FN_INSERT_FLD_TOPIC));
                }
            }
            else
            {
                pPopup = new PopupMenu;
                SwGlossaryList* pGlossaryList = ::GetGlossaryList();
                USHORT nGroupCount = pGlossaryList->GetGroupCount();
                for(USHORT i = 1; i <= nGroupCount; i++)
                {
                    // Gruppenname mit Pfad-Extension besorgen
                    String sTitle;
                    String sGroupName = pGlossaryList->GetGroupName(i - 1, FALSE, &sTitle);
                    USHORT nBlockCount = pGlossaryList->GetBlockCount(i -1);
                    if(nBlockCount)
                    {
                        USHORT nIndex = 100 * (i);
                        // aber ohne extension einfuegen
                        pPopup->InsertItem( i, sTitle);//sGroupName.GetToken(0, GLOS_DELIM));
                        PopupMenu* pSub = new PopupMenu;
                        pSub->SetSelectHdl(aLnk);
                        pPopup->SetPopupMenu(i, pSub);
                        for(USHORT j = 0; j < nBlockCount; j++)
                        {
                            String sEntry;
                            String sLongName(pGlossaryList->GetBlockName(i - 1, j, sEntry));
                            sEntry.AppendAscii(" - ");
                            sEntry += sLongName;
                            pSub->InsertItem(++nIndex, sEntry);
                        }
                    }
                }
            }
        }
        ToolBox* pToolBox = &GetToolBox();
        USHORT nId = GetId();
        pToolBox->SetItemDown( nId, TRUE );
        pPopup->Execute( pToolBox, pToolBox->ScreenToOutputPixel( aPt ) );
        pToolBox->SetItemDown( nId, FALSE );
    }
    GetToolBox().EndSelection();
    DelPopup();
    return 0;

}


/**********************************************************************

**********************************************************************/


SfxPopupWindowType SwTbxAutoTextCtrl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONTIMEOUT;
}

/**********************************************************************

**********************************************************************/


void SwTbxAutoTextCtrl::StateChanged( USHORT nSID,
                                              SfxItemState eState,
                                              const SfxPoolItem* pState )
{
    GetToolBox().EnableItem( GetId(), (GetItemState(pState) != SFX_ITEM_DISABLED) );
    if(FN_INSERT_FIELD_CTRL == nSID && eState >= SFX_ITEM_DEFAULT)
    {
        GetToolBox().CheckItem( GetId(), ((SfxBoolItem*)pState)->GetValue() );
    }
}

/**********************************************************************

**********************************************************************/


IMPL_LINK(SwTbxAutoTextCtrl, PopupHdl, PopupMenu*, pMenu)
{
    int nId = pMenu->GetCurItemId();

    if (GetId() == FN_INSERT_FIELD_CTRL)
    {
        SfxViewFrame::Current()->GetDispatcher()->Execute(nId);
    }
    else
    {
        USHORT nBlock = nId/100;

        SwGlossaryList* pGlossaryList = ::GetGlossaryList();
        String sShortName;
        String sGroup = pGlossaryList->GetGroupName(nBlock - 1, FALSE);
        String sLongName(pGlossaryList->GetBlockName(nBlock - 1, nId - (100 * nBlock) - 1, sShortName));

        SwGlossaryHdl* pGlosHdl = pView->GetGlosHdl();
        SwGlossaryDlg::SetActGroup(sGroup);
        pGlosHdl->SetCurGroup(sGroup, TRUE);
        pGlosHdl->InsertGlossary(sShortName);
    }
    return 0;
}

/**********************************************************************

**********************************************************************/


void SwTbxAutoTextCtrl::DelPopup()
{
    if(pPopup)
    {
        if (GetId() != FN_INSERT_FIELD_CTRL)
        {
            for( USHORT i = 0; i < pPopup->GetItemCount(); i ++ )
            {
                PopupMenu* pSubPopup = pPopup->GetPopupMenu(pPopup->GetItemId(i));
                delete pSubPopup;
            }
        }
        delete pPopup;
        pPopup = 0;
    }
}

/*-----------------19.02.97 10.52-------------------
    Navigations-Popup
--------------------------------------------------*/

SwScrollNaviPopup::SwScrollNaviPopup( USHORT nId,
                                const ResId &rId,
                                SfxBindings & rBnd)
    : SfxPopupWindow(nId, rId, rBnd),
    aVSet(this, WB_NAMEFIELD),
    aIList(ResId(IL_VALUES)),
    rResId(rId),
    nFwdId(FN_START_OF_NEXT_PAGE),
    nBackId(FN_START_OF_PREV_PAGE)
{
    aVSet.SetHelpId(HID_NAVI_VS);
    // die Konvertierungstabelle legt die Reihenfolge fest
    static USHORT __READONLY_DATA aInsert[ NAVI_ENTRIES ] =
    {
        // -- erste Zeile
        NID_TBL,
        NID_FRM,
        NID_GRF,
        NID_OLE,
        NID_PGE,
        NID_OUTL,
        NID_MARK,
        NID_DRW,
        NID_CTRL,
        // -- zweite Zeile
        NID_REG,
        NID_BKM,
        NID_SEL,
        NID_FTN,
        NID_POSTIT,
        NID_SRCH_REP,
        NID_INDEX_ENTRY,
        NID_TABLE_FORMULA,
        NID_TABLE_FORMULA_ERROR
    };

    aVSet.SetLineCount( 2 );
    aVSet.SetColCount(NID_COUNT / 2 - 1);
    for(USHORT i = 0; i < NID_COUNT - 2; i++)
    {
        USHORT nId = aInsert[i];
        aVSet.InsertItem(nId, aIList.GetImage(nId));
        // -2, weil es fuer Next/Prev keinen String gibt
        USHORT nResStr = ST_TBL - 2 + nId - NID_START;
        aVSet.SetItemText(nId, ResId(nResStr));
    }
    // erst hier!
    FreeResource();

    // diese Strings sind global
    for( i = 0; i < 2 * NID_COUNT; i++)
    {
        sQuickHelp[i] = String(SW_RES(STR_IMGBTN_START + i));
    }

    Size aImgSize = aIList.GetImageSize();
    aImgSize.Width() += 5;
    aImgSize.Height() += 5;
    Size aSz = aVSet.CalcWindowSizePixel(aImgSize, aVSet.GetItemCount() / 2, 2);
    aVSet.SetPosSizePixel( Point(), aSz );
    SetOutputSizePixel(aSz);
    aVSet.SetSelectHdl(LINK(this, SwScrollNaviPopup, SelectHdl));
    aVSet.StartSelection();
    aVSet.Show();
    SetBackground(Wallpaper(Color(COL_WHITE)));
}
/*-----------------19.02.97 12.45-------------------

--------------------------------------------------*/

SwScrollNaviPopup::~SwScrollNaviPopup()
{
}
/*-----------------19.02.97 13.58-------------------

--------------------------------------------------*/

SfxPopupWindow* SwScrollNaviPopup::Clone() const
{
    return new SwScrollNaviPopup(GetId(), rResId, ( SfxBindings & ) GetBindings());
}

/*-----------------19.02.97 14.10-------------------

--------------------------------------------------*/

IMPL_LINK(SwScrollNaviPopup, SelectHdl, ValueSet*, pSet)
{
    USHORT nSet = pSet->GetSelectItemId();
    if( nSet != NID_PREV && nSet != NID_NEXT )
    {
        SwView::SetMoveType(nSet);
        aVSet.SetItemText(NID_NEXT, sQuickHelp[nSet - NID_START]);
        aVSet.SetItemText(NID_PREV, sQuickHelp[nSet - NID_START + NID_COUNT]);
    }
    else
    {
        SfxBoolItem aNext(FN_SCROLL_NEXT_PREV, NID_NEXT == nSet);
        SfxViewFrame::Current()->GetDispatcher()->
                Execute(FN_SCROLL_NEXT_PREV,SFX_CALLMODE_SYNCHRON ,&aNext, 0L);
    }
    return 0;
}
/*-----------------23.02.97 18.21-------------------

--------------------------------------------------*/

void SwScrollNaviValueSet::MouseButtonUp( const MouseEvent& rMEvt )
{
    ValueSet::MouseButtonUp(rMEvt);
    if ( ((SwScrollNaviPopup*)GetParent())->IsInPopupMode() )
        ((SwScrollNaviPopup*)GetParent())->EndPopupMode( FLOATWIN_POPUPMODEEND_CLOSEALL );
}

/*-----------------20.06.97 13:28-------------------

--------------------------------------------------*/
void  SwScrollNaviValueSet::RequestHelp( const HelpEvent& rHEvt )
{
    USHORT nMoveType = SwView::GetMoveType();
    SetItemText(NID_NEXT, SwScrollNaviPopup::GetQuickHelpText(TRUE));
    SetItemText(NID_PREV, SwScrollNaviPopup::GetQuickHelpText(FALSE));
    ValueSet::RequestHelp( rHEvt );

}
/*-----------------21.02.97 11:25-------------------

--------------------------------------------------*/

void  SwScrollNaviPopup::PopupModeEnd()
{
    if(aVSet.GetItemCount() < NID_COUNT)
    {
        aVSet.InsertItem(NID_NEXT, aIList.GetImage(NID_NEXT), NID_COUNT);
        aVSet.InsertItem(NID_PREV, aIList.GetImage(NID_PREV), NID_COUNT/2 - 1);
        USHORT nItemBits = aVSet.GetItemBits(NID_NEXT);
        nItemBits |= VIB_NODOUBLECLICK;
        aVSet.SetItemBits(NID_NEXT, nItemBits);
        aVSet.SetItemBits(NID_PREV, nItemBits);
        Size aImgSize = aIList.GetImageSize();
        aImgSize.Width() += 5;
        aImgSize.Height() += 5;
        aVSet.SetColCount(NID_COUNT/2);
        Size aSz = aVSet.CalcWindowSizePixel(aImgSize, NID_COUNT / 2, 2);
        aVSet.SetPosSizePixel( Point(), aSz );
        SetOutputSizePixel(aSz);
    }
    SfxPopupWindow::PopupModeEnd();
}

/*-----------------20.06.97 13:41-------------------

--------------------------------------------------*/
String  SwScrollNaviPopup::GetQuickHelpText(BOOL bNext)
{
    USHORT nResId = STR_IMGBTN_START;
    nResId += SwView::GetMoveType() - NID_START;
    if(!bNext)
        nResId += NID_COUNT;
    return String(SW_RES(nResId));
}


/*-----------------20.02.97 10:17-------------------

--------------------------------------------------*/

void SwNaviImageButton::MouseButtonDown( const MouseEvent& rMEvt )
{
    if(MOUSE_LEFT == rMEvt.GetButtons())
    {
        SfxBindings& rBind = SfxViewFrame::Current()->GetBindings();
        rBind.ENTERREGISTRATIONS();
        pPopup = new
            SwScrollNaviPopup(FN_SCROLL_NAVIGATION,
                            SW_RES(RID_SCROLL_NAVIGATION_WIN),
                            rBind);
        rBind.LEAVEREGISTRATIONS();
        Point aPos = OutputToScreenPixel(Point(0,0));
        Rectangle aRect(aPos, GetSizePixel());
        pPopup->StartPopupMode(aRect, FLOATWIN_POPUPMODE_LEFT|FLOATWIN_POPUPMODE_ALLOWTEAROFF);
    }
    else
        ImageButton::MouseButtonDown(rMEvt);
}
/*-----------------20.02.97 10:17-------------------

--------------------------------------------------*/

void SwNaviImageButton::MouseButtonUp( const MouseEvent& rMEvt )
{
}

/*-----------------21.02.97 09:41-------------------

--------------------------------------------------*/

void SwHlpImageButton::RequestHelp( const HelpEvent& rHEvt )
{

    SetQuickHelpText(SwScrollNaviPopup::GetQuickHelpText(!bUp));

    ImageButton::RequestHelp(rHEvt);
}

/*-----------------25.02.97 12:38-------------------

--------------------------------------------------*/

SwNaviImageButton::SwNaviImageButton(Window* pParent) :
            ImageButton(pParent, SW_RES(BTN_NAVI)), pPopup(0),
            aImage(ResId(IMG_BTN)),
            sQuickText(ResId(ST_QUICK))
{
    FreeResource();
    SetStyle(GetStyle()|WB_NOPOINTERFOCUS);
    SetQuickHelpText(sQuickText);
    SetImage(aImage);
}


