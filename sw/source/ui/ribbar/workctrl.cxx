/*************************************************************************
 *
 *  $RCSfile: workctrl.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 11:33:36 $
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
#ifndef _SFX_IMAGEMGR_HXX
#include <sfx2/imagemgr.hxx>
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
//CHINA001 #ifndef _GLOSSARY_HXX
//CHINA001 #include <glossary.hxx>
//CHINA001 #endif
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
#ifndef _SVT_IMAGEITM_HXX
#include <svtools/imageitm.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#include <rtl/ustring.hxx>
#include "swabstdlg.hxx" //CHINA001
#include <misc.hrc> //CHINA001

//JP 14.01.99: Size Abpruefung
#define NAVI_ENTRIES 20
#if NAVI_ENTRIES != NID_COUNT
#error SwScrollNaviPopup-CTOR static Array falsche Size. Wurden neue IDs zugefuegt ??
#endif

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::drafts::com::sun::star::frame;

SFX_IMPL_TOOLBOX_CONTROL( SwTbxInsertCtrl, SfxImageItem);
SFX_IMPL_TOOLBOX_CONTROL( SwTbxAutoTextCtrl, SfxBoolItem );

/**********************************************************************

**********************************************************************/
SwTbxInsertCtrl::SwTbxInsertCtrl(
    USHORT nSlotId,
    USHORT nId,
    ToolBox& rTbx ) :
        SfxToolBoxControl( nSlotId, nId, rTbx ),
        nLastSlotId(FN_INSERT_CTRL == nSlotId ? FN_INSERT_TABLE : SID_INSERT_DIAGRAM)
{
    rTbx.SetItemBits( nId, TIB_DROPDOWN | rTbx.GetItemBits( nId ) );
}
/**********************************************************************

**********************************************************************/
SwTbxInsertCtrl::~SwTbxInsertCtrl()
{
}

void SAL_CALL SwTbxInsertCtrl::update() throw (::com::sun::star::uno::RuntimeException)
{
    ToolBox& rTbx = GetToolBox();
    rtl::OUString aSlotURL( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));
    aSlotURL += rtl::OUString::valueOf( sal_Int32( nLastSlotId ));
    Image aImage = GetImage( m_xFrame,
                             aSlotURL,
                             hasBigImages(),
                             rTbx.GetDisplayBackground().GetColor().IsDark() );

    rTbx.SetItemImage(GetId(), aImage);
    rTbx.Invalidate();

    SfxToolBoxControl::update();
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
        const SfxImageItem* pItem = PTR_CAST( SfxImageItem, pState );
        if(pItem)
        {
            nLastSlotId = pItem->GetValue();
            if( nLastSlotId )
                nId = nLastSlotId;

            rtl::OUString aSlotURL( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));
            aSlotURL += rtl::OUString::valueOf( sal_Int32( nId ));
            Image aImage = GetImage( m_xFrame,
                                     aSlotURL,
                                     hasBigImages(),
                                     GetToolBox().GetDisplayBackground().GetColor().IsDark() );
            ToolBox& rBox = GetToolBox();
            rBox.SetItemImage(GetId(), aImage);
            rBox.SetItemImageMirrorMode( GetId(), FALSE );
            rBox.SetItemImageAngle( GetId(), pItem->GetRotation() );
            rBox.SetItemImageMirrorMode( GetId(), pItem->IsMirrored() );
        }
    }

}
/**********************************************************************

**********************************************************************/


SfxPopupWindow* SwTbxInsertCtrl::CreatePopupWindow()
{
//  USHORT nWinResId, nTbxResId;
    if(GetSlotId() == FN_INSERT_CTRL)
    {
        OUString aToolBarResStr( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/insertbar" ));
        createAndPositionSubToolBar( aToolBarResStr );
//      nWinResId = RID_INSERT_CTRL;
//      nTbxResId = TBX_INSERT;
    }
    else /* FN_INSERT_OBJ_CTRL */
    {
        OUString aToolBarResStr( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/insertobjectbar" ));
        createAndPositionSubToolBar( aToolBarResStr );
//      nWinResId = RID_INSERT_OBJ_CTRL;
//      nTbxResId = TBX_OBJ_INSERT;
    }
/*
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
*/
    return NULL;
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
    if( nLastSlotId )
    {
        SfxViewShell*   pCurSh( SfxViewShell::Current() );
        SfxDispatcher*  pDispatch( 0 );

        if ( pCurSh )
        {
            SfxViewFrame*   pViewFrame = pCurSh->GetViewFrame();
            if ( pViewFrame )
                pDispatch = pViewFrame->GetDispatcher();
        }

        if ( pDispatch )
            pDispatch->Execute(nLastSlotId);
    }
}

/**********************************************************************

**********************************************************************/


SwTbxAutoTextCtrl::SwTbxAutoTextCtrl(
    USHORT nSlotId,
    USHORT nId,
    ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx ),
    pView(0),
    pPopup(0)
{
    rTbx.SetItemBits( nId, TIB_DROPDOWN | rTbx.GetItemBits( nId ) );
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

        Rectangle aItemRect( rBox.GetItemRect( GetId() ) );
        Point aPt(rBox.OutputToScreenPixel(aItemRect.TopLeft()));
        aPt.X() += aItemRect.GetWidth()/2;
        aPt.Y() += aItemRect.GetHeight()/2;
        if(pView)
        {
            Link aLnk = LINK(this, SwTbxAutoTextCtrl, PopupHdl);

            if (GetSlotId() == FN_INSERT_FIELD_CTRL)
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

        pPopup->Execute( pToolBox, pToolBox->GetItemRect( nId ),
            (pToolBox->GetAlign() == WINDOWALIGN_TOP || pToolBox->GetAlign() == WINDOWALIGN_BOTTOM) ?
                POPUPMENU_EXECUTE_DOWN : POPUPMENU_EXECUTE_RIGHT );

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

    if ( GetSlotId() == FN_INSERT_FIELD_CTRL)
    {
        Sequence< PropertyValue > aArgs;
        Dispatch( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:InsertFieldCtrl" )),
                  aArgs );
//        GetBindings().Execute( nId );
    }
    else
    {
        USHORT nBlock = nId/100;

        SwGlossaryList* pGlossaryList = ::GetGlossaryList();
        String sShortName;
        String sGroup = pGlossaryList->GetGroupName(nBlock - 1, FALSE);
        String sLongName(pGlossaryList->GetBlockName(nBlock - 1, nId - (100 * nBlock) - 1, sShortName));

        SwGlossaryHdl* pGlosHdl = pView->GetGlosHdl();
        //CHINA001 SwGlossaryDlg::SetActGroup(sGroup);
        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001
        ::GlossarySetActGroup fnSetActGroup = pFact->SetGlossaryActGroupFunc( DLG_RENAME_GLOS );
        if ( fnSetActGroup )
            (*fnSetActGroup)( sGroup );
        //CHINA001 end
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
        if (GetSlotId() != FN_INSERT_FIELD_CTRL)
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
// determine the order of the toolbox items
static USHORT __READONLY_DATA aNavigationInsertIds[ NAVI_ENTRIES ] =
{
    // -- first line
    NID_TBL,
    NID_FRM,
    NID_GRF,
    NID_OLE,
    NID_PGE,
    NID_OUTL,
    NID_MARK,
    NID_DRW,
    NID_CTRL,
    NID_PREV,
    // -- second line
    NID_REG,
    NID_BKM,
    NID_SEL,
    NID_FTN,
    NID_POSTIT,
    NID_SRCH_REP,
    NID_INDEX_ENTRY,
    NID_TABLE_FORMULA,
    NID_TABLE_FORMULA_ERROR,
    NID_NEXT
};

SwScrollNaviPopup::SwScrollNaviPopup( USHORT nId, const Reference< XFrame >& rFrame )
    : SfxPopupWindow(nId, rFrame, SW_RES(RID_SCROLL_NAVIGATION_WIN) ),
    aToolBox(this, 0),
    aSeparator(this, ResId(FL_SEP)),
    aInfoField(this, ResId(FI_INFO)),
    aIList(ResId(IL_VALUES)),
    aIListH(ResId(ILH_VALUES)),
    nFwdId(FN_START_OF_NEXT_PAGE),
    nBackId(FN_START_OF_PREV_PAGE)
{
    USHORT i;

    aToolBox.SetHelpId(HID_NAVI_VS);
    aToolBox.SetLineCount( 2 );
    aToolBox.SetOutStyle(TOOLBOX_STYLE_FLAT);
    for( i = 0; i < NID_COUNT; i++)
    {
        USHORT nId = aNavigationInsertIds[i];
        String sText;
        ToolBoxItemBits  nTbxBits = 0;
        if((NID_PREV != nId) && (NID_NEXT != nId))
        {
            // -2, there's no string for Next/Prev
            USHORT nResStr = ST_TBL - 2 + nId - NID_START;
            sText = String(ResId(nResStr));
            nTbxBits = TIB_CHECKABLE;
        }
        aToolBox.InsertItem(nId, sText, nTbxBits);
    }
    ApplyImageList();
    aToolBox.InsertBreak(NID_COUNT/2);
    // don't call it before!
    FreeResource();

    // these are global strings
    for( i = 0; i < 2 * NID_COUNT; i++)
    {
        sQuickHelp[i] = String(SW_RES(STR_IMGBTN_START + i));
    }

    Size aImgSize = aIList.GetImageSize();
    aImgSize.Width() += 5;
    aImgSize.Height() += 5;
    Size aSz = aToolBox.CalcWindowSizePixel(2);
    aToolBox.SetPosSizePixel( Point(), aSz );
    USHORT nItemId = SwView::GetMoveType();
    aInfoField.SetText(aToolBox.GetItemText(nItemId));
    aToolBox.CheckItem( nItemId, sal_True );
    Size aFTSize(aInfoField.GetSizePixel());
    Size aSepSize(aSeparator.GetSizePixel());
    aSepSize.Width() = aSz.Width();

    aSz.Height() += aFTSize.Height() + aSepSize.Height();
    aInfoField.SetPosSizePixel(
        Point(0, aSz.Height() - aFTSize.Height()), Size(aSz.Width(), aFTSize.Height()));

    aSeparator.SetSizePixel(aSepSize);
    aSeparator.SetPosPixel(Point(0, aSz.Height() - aFTSize.Height() - aSepSize.Height()));

    SetOutputSizePixel(aSz);
    aToolBox.SetSelectHdl(LINK(this, SwScrollNaviPopup, SelectHdl));
    aToolBox.StartSelection();
    aToolBox.Show();
}
/*-----------------19.02.97 12.45-------------------

--------------------------------------------------*/

SwScrollNaviPopup::~SwScrollNaviPopup()
{
}
/* -----------------------------08.05.2002 14:00------------------------------

 ---------------------------------------------------------------------------*/
void SwScrollNaviPopup::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
            ApplyImageList();

    Window::DataChanged( rDCEvt );
}
/* -----------------------------08.05.2002 14:02------------------------------

 ---------------------------------------------------------------------------*/
void SwScrollNaviPopup::ApplyImageList()
{
    ImageList& rImgLst = aToolBox.GetDisplayBackground().GetColor().IsDark() ?
        aIListH : aIList;
    for(USHORT i = 0; i < NID_COUNT; i++)
    {
        USHORT nId = aNavigationInsertIds[i];
        aToolBox.SetItemImage(nId, rImgLst.GetImage(nId));
    }
}
/*-----------------19.02.97 13.58-------------------

--------------------------------------------------*/

SfxPopupWindow* SwScrollNaviPopup::Clone() const
{
    return new SwScrollNaviPopup( GetId(), GetFrame() );
}

/*-----------------19.02.97 14.10-------------------

--------------------------------------------------*/

IMPL_LINK(SwScrollNaviPopup, SelectHdl, ToolBox*, pSet)
{
    USHORT nSet = pSet->GetCurItemId();
    if( nSet != NID_PREV && nSet != NID_NEXT )
    {
        SwView::SetMoveType(nSet);
        aToolBox.SetItemText(NID_NEXT, sQuickHelp[nSet - NID_START]);
        aToolBox.SetItemText(NID_PREV, sQuickHelp[nSet - NID_START + NID_COUNT]);
        aInfoField.SetText(aToolBox.GetItemText(nSet));
        //check the current button only
        for(USHORT i = 0; i < NID_COUNT; i++)
        {
            USHORT nItemId = aToolBox.GetItemId( i );
            aToolBox.CheckItem( nItemId, nItemId == nSet );
        }
    }
    else
    {
/*
        const SfxPoolItem* aItems[2];
*/
        SfxBoolItem aNext(FN_SCROLL_NEXT_PREV, NID_NEXT == nSet);
/*
        aItems[0] = &aNext;
        aItems[1] = NULL;
        GetBindings().ExecuteSynchron(FN_SCROLL_NEXT_PREV,aItems, 0L);
*/
        Any a;
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ScrollNextPrev" ));
        aNext.QueryValue( a );
        aArgs[0].Value = a;
        SfxToolBoxControl::Dispatch( Reference< XDispatchProvider >( GetFrame()->getController(), UNO_QUERY ),
                                     rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ScrollNextPrev" )),
                                     aArgs );
    }
    return 0;
}
/*-----------------23.02.97 18.21-------------------

--------------------------------------------------*/

void SwScrollNaviToolBox::MouseButtonUp( const MouseEvent& rMEvt )
{
    ToolBox::MouseButtonUp(rMEvt);
    if ( ((SwScrollNaviPopup*)GetParent())->IsInPopupMode() )
        ((SwScrollNaviPopup*)GetParent())->EndPopupMode( FLOATWIN_POPUPMODEEND_CLOSEALL );
}

/*-----------------20.06.97 13:28-------------------

--------------------------------------------------*/
void  SwScrollNaviToolBox::RequestHelp( const HelpEvent& rHEvt )
{
    USHORT nMoveType = SwView::GetMoveType();
    SetItemText(NID_NEXT, SwScrollNaviPopup::GetQuickHelpText(TRUE));
    SetItemText(NID_PREV, SwScrollNaviPopup::GetQuickHelpText(FALSE));
    ToolBox::RequestHelp( rHEvt );

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
/* -----------------------------05.09.2002 13:53------------------------------

 ---------------------------------------------------------------------------*/
void SwNaviImageButton::Click()
{
//    SfxBindings& rBind = SfxViewFrame::Current()->GetBindings();
//    rBind.ENTERREGISTRATIONS();
    pPopup = new
        SwScrollNaviPopup( FN_SCROLL_NAVIGATION,
                           m_xFrame );
//    rBind.LEAVEREGISTRATIONS();
    Point aPos = OutputToScreenPixel(Point(0,0));
    Rectangle aRect(aPos, GetSizePixel());
    SetPopupWindow( pPopup );
    pPopup->StartPopupMode(aRect, FLOATWIN_POPUPMODE_LEFT|FLOATWIN_POPUPMODE_ALLOWTEAROFF);
}

//--------------------------------------------------------------------

void SwNaviImageButton::SetPopupWindow( SfxPopupWindow* pWindow )
{
    pPopupWindow = pWindow;
    pPopupWindow->SetPopupModeEndHdl( LINK( this, SwNaviImageButton, PopupModeEndHdl ));
    pPopupWindow->SetDeleteLink_Impl( LINK( this, SwNaviImageButton, ClosePopupWindow ));
}

//--------------------------------------------------------------------

IMPL_LINK( SwNaviImageButton, PopupModeEndHdl, void *, EMPTYARG )
{
    if ( pPopupWindow->IsVisible() )
    {
        // Replace floating window with popup window and destroy
        // floating window instance.
        delete pFloatingWindow;
        pFloatingWindow = pPopupWindow;
        pPopupWindow    = 0;
    }
    else
    {
        // Popup window has been closed by the user. No replacement, instance
        // will destroy itself.
        pPopupWindow = 0;
    }

    return 1;
}

//--------------------------------------------------------------------
IMPL_LINK( SwNaviImageButton, ClosePopupWindow, SfxPopupWindow *, pWindow )
{
    if ( pWindow == pFloatingWindow )
        pFloatingWindow = 0;
    else
        pPopupWindow = 0;

    return 1;
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

SwNaviImageButton::SwNaviImageButton(
    Window* pParent,
    const Reference< XFrame >& rFrame ) :
    ImageButton(pParent, SW_RES(BTN_NAVI)),
        pPopup(0),
        aImage(ResId(IMG_BTN)),
        aImageH(ResId(IMG_BTN_H)),
        sQuickText(ResId(ST_QUICK)),
        m_xFrame( rFrame ),
        pPopupWindow(0),
        pFloatingWindow(0)
{
    FreeResource();
    SetStyle(GetStyle()|WB_NOPOINTERFOCUS);
    SetQuickHelpText(sQuickText);
    SetModeImage( GetDisplayBackground().GetColor().IsDark() ? aImageH : aImage);
}
/* -----------------------------2002/07/05 9:41-------------------------------

 ---------------------------------------------------------------------------*/
void SwNaviImageButton::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
            SetModeImage( GetDisplayBackground().GetColor().IsDark() ? aImageH : aImage);

    Window::DataChanged( rDCEvt );
}
/* -----------------26.11.2002 09:28-----------------
 *
 * --------------------------------------------------*/
class SwZoomBox_Impl : public ComboBox
{
    USHORT          nSlotId;
    BOOL            bRelease;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > m_xDispatchProvider;

public:
    SwZoomBox_Impl(
        Window* pParent,
        USHORT nSlot,
        const Reference< XDispatchProvider >& rDispatchProvider );
    ~SwZoomBox_Impl();

protected:
    virtual void    Select();
    virtual long    Notify( NotifyEvent& rNEvt );

    void ReleaseFocus();

};
/* -----------------26.11.2002 09:29-----------------
 *
 * --------------------------------------------------*/
SwZoomBox_Impl::SwZoomBox_Impl(
    Window* pParent,
    USHORT nSlot,
    const Reference< XDispatchProvider >& rDispatchProvider ):
    ComboBox( pParent, SW_RES(RID_PVIEW_ZOOM_LB)),
    nSlotId(nSlot),
    bRelease(TRUE),
    m_xDispatchProvider( rDispatchProvider )
{
    EnableAutocomplete( FALSE );
    USHORT aZoomValues[] =
    {   25, 50, 75, 100, 150, 200 };
    for(USHORT i = 0; i < sizeof(aZoomValues)/sizeof(USHORT); i++)
    {
        String sEntry = String::CreateFromInt32(aZoomValues[i]);
        sEntry += '%';
        InsertEntry(sEntry);
    }
}
/* -----------------26.11.2002 09:29-----------------
 *
 * --------------------------------------------------*/
SwZoomBox_Impl::~SwZoomBox_Impl()
{}
/* -----------------26.11.2002 09:34-----------------
 *
 * --------------------------------------------------*/
void    SwZoomBox_Impl::Select()
{
    if ( !IsTravelSelect() )
    {
        String sEntry(GetText());
        sEntry.EraseAllChars( '%' );
        USHORT nZoom = (USHORT)sEntry.ToInt32();
        if(nZoom < MINZOOM)
            nZoom = MINZOOM;
        if(nZoom > MAXZOOM)
            nZoom = MAXZOOM;

        SfxUInt16Item aItem( nSlotId, nZoom );
/*
        rBindings.GetDispatcher()->Execute(
            nSlotId, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );
*/
        if ( FN_PREVIEW_ZOOM == nSlotId )
        {
            Any a;
            Sequence< PropertyValue > aArgs( 1 );
            aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PreviewZoom" ));
            aItem.QueryValue( a );
            aArgs[0].Value = a;
            SfxToolBoxControl::Dispatch(
                m_xDispatchProvider,
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:PreviewZoom" )),
                aArgs );
        }

        ReleaseFocus();
    }
}
/* -----------------02.12.2002 07:49-----------------
 *
 * --------------------------------------------------*/
long SwZoomBox_Impl::Notify( NotifyEvent& rNEvt )
{
    long nHandled = 0;

    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        USHORT nCode = rNEvt.GetKeyEvent()->GetKeyCode().GetCode();

        switch ( nCode )
        {
            case KEY_RETURN:
            case KEY_TAB:
            {
                if ( KEY_TAB == nCode )
                    bRelease = FALSE;
                else
                    nHandled = 1;
                Select();
                break;
            }

            case KEY_ESCAPE:
                SetText( GetSavedValue() );
                ReleaseFocus();
                break;
        }
    }
    else if ( EVENT_LOSEFOCUS == rNEvt.GetType() )
    {
        Window* pFocusWin = Application::GetFocusWindow();
        if ( !HasFocus() && GetSubEdit() != pFocusWin )
            SetText( GetSavedValue() );
    }

    return nHandled ? nHandled : ComboBox::Notify( rNEvt );
}
/* -----------------02.12.2002 07:51-----------------
 *
 * --------------------------------------------------*/
void SwZoomBox_Impl::ReleaseFocus()
{
    if ( !bRelease )
    {
        bRelease = TRUE;
        return;
    }
    SfxViewShell* pCurSh = SfxViewShell::Current();

    if ( pCurSh )
    {
        Window* pShellWnd = pCurSh->GetWindow();

        if ( pShellWnd )
            pShellWnd->GrabFocus();
    }
}

/* -----------------26.11.2002 09:29-----------------
 *
 * --------------------------------------------------*/
SFX_IMPL_TOOLBOX_CONTROL( SwPreviewZoomControl, SfxUInt16Item);

SwPreviewZoomControl::SwPreviewZoomControl(
    USHORT nSlotId,
    USHORT nId,
    ToolBox& rTbx) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
{
}
/* -----------------26.11.2002 09:29-----------------
 *
 * --------------------------------------------------*/
SwPreviewZoomControl::~SwPreviewZoomControl()
{
}
/* -----------------26.11.2002 09:29-----------------
 *
 * --------------------------------------------------*/
void SwPreviewZoomControl::StateChanged( USHORT nSID,
                                         SfxItemState eState,
                                         const SfxPoolItem* pState )
{
    USHORT nId = GetId();
    GetToolBox().EnableItem( nId, (GetItemState(pState) != SFX_ITEM_DISABLED) );
    SwZoomBox_Impl* pBox = (SwZoomBox_Impl*)GetToolBox().GetItemWindow( GetId() );
    if(SFX_ITEM_AVAILABLE <= eState)
    {
        String sZoom(String::CreateFromInt32(((const SfxUInt16Item*)pState)->GetValue()));
        sZoom += '%';
        pBox->SetText(sZoom);
        pBox->SaveValue();
    }
}
/* -----------------26.11.2002 09:29-----------------
 *
 * --------------------------------------------------*/
Window* SwPreviewZoomControl::CreateItemWindow( Window *pParent )
{
    SwZoomBox_Impl* pRet = new SwZoomBox_Impl( pParent, GetSlotId(), Reference< XDispatchProvider >( m_xFrame->getController(), UNO_QUERY ));
    return pRet;
}


