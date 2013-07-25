/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <string>
#include <comphelper/string.hxx>
#include <svl/eitem.hxx>
#include <sfx2/htmlmode.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/imagemgr.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <initui.hxx>
#include <viewopt.hxx>
#include <docsh.hxx>
#include <gloshdl.hxx>
#include <glosdoc.hxx>
#include <gloslst.hxx>
#include <workctrl.hxx>
#include <workctrl.hrc>
#include <cmdid.h>
#include <helpid.h>
#include <wrtsh.hxx>
#include <svl/imageitm.hxx>
#include <vcl/lstbox.hxx>
#include <rtl/ustring.hxx>
#include "swabstdlg.hxx"
#include <misc.hrc>

#include <vcl/svapp.hxx>

// Size check
#define NAVI_ENTRIES 20
#if NAVI_ENTRIES != NID_COUNT
#error SwScrollNaviPopup-CTOR static array wrong size. Are new IDs added?
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;

SFX_IMPL_TOOLBOX_CONTROL( SwTbxInsertCtrl, SfxImageItem);
SFX_IMPL_TOOLBOX_CONTROL( SwTbxAutoTextCtrl, SfxBoolItem );

SwTbxInsertCtrl::SwTbxInsertCtrl(
    sal_uInt16 nSlotId,
    sal_uInt16 nId,
    ToolBox& rTbx ) :
        SfxToolBoxControl( nSlotId, nId, rTbx ),
        nLastSlotId(FN_INSERT_CTRL == nSlotId ? FN_INSERT_TABLE : SID_INSERT_DIAGRAM)
{
    rTbx.SetItemBits( nId, TIB_DROPDOWN | rTbx.GetItemBits( nId ) );
}

SwTbxInsertCtrl::~SwTbxInsertCtrl()
{
}

void SAL_CALL SwTbxInsertCtrl::update() throw (uno::RuntimeException)
{
    ToolBox& rTbx = GetToolBox();
    OUString aSlotURL( "slot:" );
    aSlotURL += OUString::valueOf( sal_Int32( nLastSlotId ));
    Image aImage = GetImage( m_xFrame, aSlotURL, hasBigImages() );

    rTbx.SetItemImage(GetId(), aImage);
    rTbx.Invalidate();

    SfxToolBoxControl::update();
}

void SwTbxInsertCtrl::StateChanged( sal_uInt16 /*nSID*/,
                                      SfxItemState eState,
                                      const SfxPoolItem* pState )
{
    sal_uInt16 nId = GetId();
    GetToolBox().EnableItem( nId, (GetItemState(pState) != SFX_ITEM_DISABLED) );

    if( eState == SFX_ITEM_AVAILABLE )
    {
        const SfxImageItem* pItem = PTR_CAST( SfxImageItem, pState );
        if(pItem)
        {
            nLastSlotId = pItem->GetValue();
            if( nLastSlotId )
                nId = nLastSlotId;

            OUString aSlotURL( "slot:" );
            aSlotURL += OUString::valueOf( sal_Int32( nId ));
            ToolBox& rBox = GetToolBox();
            Image aImage = GetImage( m_xFrame, aSlotURL, hasBigImages() );
            rBox.SetItemImage(GetId(), aImage);
            rBox.SetItemImageMirrorMode( GetId(), sal_False );
            rBox.SetItemImageAngle( GetId(), pItem->GetRotation() );
            rBox.SetItemImageMirrorMode( GetId(), pItem->IsMirrored() );
        }
    }

}

SfxPopupWindow* SwTbxInsertCtrl::CreatePopupWindow()
{
    if(GetSlotId() == FN_INSERT_CTRL)
    {
        OUString aToolBarResStr( "private:resource/toolbar/insertbar" );
        createAndPositionSubToolBar( aToolBarResStr );
    }
    else /* FN_INSERT_OBJ_CTRL */
    {
        OUString aToolBarResStr( "private:resource/toolbar/insertobjectbar" );
        createAndPositionSubToolBar( aToolBarResStr );
    }
    return NULL;
}

SfxPopupWindowType  SwTbxInsertCtrl::GetPopupWindowType() const
{
    return nLastSlotId ? SFX_POPUPWINDOW_ONTIMEOUT : SFX_POPUPWINDOW_ONCLICK;
}


void SwTbxInsertCtrl::Select( sal_Bool /*bMod1*/ )
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

SwTbxAutoTextCtrl::SwTbxAutoTextCtrl(
    sal_uInt16 nSlotId,
    sal_uInt16 nId,
    ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx ),
    pPopup(0),
    pView(0)
{
    rTbx.SetItemBits( nId, TIB_DROPDOWN | rTbx.GetItemBits( nId ) );
}

SwTbxAutoTextCtrl::~SwTbxAutoTextCtrl()
{
    DelPopup();
}

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
                sal_uInt16 nGroupCount = pGlossaryList->GetGroupCount();
                for(sal_uInt16 i = 1; i <= nGroupCount; i++)
                {
                    // Acquire group name with path extension
                    String sTitle;
                    pGlossaryList->GetGroupName(i - 1, sal_False, &sTitle);
                    sal_uInt16 nBlockCount = pGlossaryList->GetBlockCount(i -1);
                    if(nBlockCount)
                    {
                        sal_uInt16 nIndex = 100 * (i);
                        // but insert without extension
                        pPopup->InsertItem( i, sTitle);
                        PopupMenu* pSub = new PopupMenu;
                        pSub->SetSelectHdl(aLnk);
                        pPopup->SetPopupMenu(i, pSub);
                        for(sal_uInt16 j = 0; j < nBlockCount; j++)
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
        sal_uInt16 nId = GetId();
        pToolBox->SetItemDown( nId, sal_True );

        pPopup->Execute( pToolBox, pToolBox->GetItemRect( nId ),
            (pToolBox->GetAlign() == WINDOWALIGN_TOP || pToolBox->GetAlign() == WINDOWALIGN_BOTTOM) ?
                POPUPMENU_EXECUTE_DOWN : POPUPMENU_EXECUTE_RIGHT );

        pToolBox->SetItemDown( nId, sal_False );
    }
    GetToolBox().EndSelection();
    DelPopup();
    return 0;

}

SfxPopupWindowType SwTbxAutoTextCtrl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONTIMEOUT;
}

void SwTbxAutoTextCtrl::StateChanged( sal_uInt16 nSID,
                                              SfxItemState eState,
                                              const SfxPoolItem* pState )
{
    GetToolBox().EnableItem( GetId(), (GetItemState(pState) != SFX_ITEM_DISABLED) );
    if(FN_INSERT_FIELD_CTRL == nSID && eState >= SFX_ITEM_DEFAULT)
    {
        GetToolBox().CheckItem( GetId(), ((SfxBoolItem*)pState)->GetValue() );
    }
}

IMPL_LINK(SwTbxAutoTextCtrl, PopupHdl, PopupMenu*, pMenu)
{
    sal_uInt16 nId = pMenu->GetCurItemId();

    if ( GetSlotId() == FN_INSERT_FIELD_CTRL)
    {
        Sequence< PropertyValue > aArgs;
        const char* pChar = 0;
        switch(nId)
        {
            case FN_INSERT_FLD_DATE:
                pChar = ".uno:InsertDateField";
            break;
            case FN_INSERT_FLD_TIME:
                pChar = ".uno:InsertTimeField";
            break;
            case FN_INSERT_FLD_PGNUMBER:
                pChar = ".uno:InsertPageNumberField";
            break;
            case FN_INSERT_FLD_PGCOUNT:
                pChar = ".uno:InsertPageCountField";
            break;
            case FN_INSERT_FLD_TOPIC:
                pChar = ".uno:InsertTopicField";
            break;
            case FN_INSERT_FLD_TITLE:
                pChar = ".uno:InsertTitleField";
            break;
            case FN_INSERT_FLD_AUTHOR:
                pChar = ".uno:InsertAuthorField";
            break;
            default:
                pChar = ".uno:InsertFieldCtrl";
        }
        Dispatch( OUString::createFromAscii( pChar ),aArgs );
    }
    else
    {
        sal_uInt16 nBlock = nId / 100;

        SwGlossaryList* pGlossaryList = ::GetGlossaryList();
        String sShortName;
        String sGroup = pGlossaryList->GetGroupName(nBlock - 1, false);
        pGlossaryList->GetBlockName(nBlock - 1, nId - (100 * nBlock) - 1, sShortName);

        SwGlossaryHdl* pGlosHdl = pView->GetGlosHdl();
        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        OSL_ENSURE(pFact, "Dialogdiet fail!");
        ::GlossarySetActGroup fnSetActGroup = pFact->SetGlossaryActGroupFunc();
        if ( fnSetActGroup )
            (*fnSetActGroup)( sGroup );
        pGlosHdl->SetCurGroup(sGroup, sal_True);
        pGlosHdl->InsertGlossary(sShortName);
    }
    return 0;
}

void SwTbxAutoTextCtrl::DelPopup()
{
    if(pPopup)
    {
        if (GetSlotId() != FN_INSERT_FIELD_CTRL)
        {
            for( sal_uInt16 i = 0; i < pPopup->GetItemCount(); i ++ )
            {
                PopupMenu* pSubPopup = pPopup->GetPopupMenu(pPopup->GetItemId(i));
                delete pSubPopup;
            }
        }
        delete pPopup;
        pPopup = 0;
    }
}

// Navigation-Popup
// determine the order of the toolbox items
static sal_uInt16 aNavigationInsertIds[ NAVI_ENTRIES ] =
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
static const char* aNavigationHelpIds[ NAVI_ENTRIES ] =
{
    // -- first line
    HID_NID_TBL,
    HID_NID_FRM,
    HID_NID_GRF,
    HID_NID_OLE,
    HID_NID_PGE,
    HID_NID_OUTL,
    HID_NID_MARK,
    HID_NID_DRW,
    HID_NID_CTRL,
    HID_NID_PREV,
    // -- second line
    HID_NID_REG,
    HID_NID_BKM,
    HID_NID_SEL,
    HID_NID_FTN,
    HID_NID_POSTIT,
    HID_NID_SRCH_REP,
    HID_NID_INDEX_ENTRY,
    HID_NID_TABLE_FORMULA,
    HID_NID_TABLE_FORMULA_ERROR,
    HID_NID_NEXT
};

SwScrollNaviPopup::SwScrollNaviPopup( sal_uInt16 nId, const Reference< XFrame >& rFrame )
    : SfxPopupWindow(nId, rFrame, SW_RES(RID_SCROLL_NAVIGATION_WIN) ),
    aToolBox(this, 0),
    aSeparator(this, SW_RES(FL_SEP)),
    aInfoField(this, SW_RES(FI_INFO)),
    aIList(SW_RES(IL_VALUES))
{
    sal_uInt16 i;

    aToolBox.SetHelpId(HID_NAVI_VS);
    aToolBox.SetLineCount( 2 );
    aToolBox.SetOutStyle(TOOLBOX_STYLE_FLAT);
    for( i = 0; i < NID_COUNT; i++)
    {
        sal_uInt16 nNaviId = aNavigationInsertIds[i];
        String sText;
        ToolBoxItemBits  nTbxBits = 0;
        if((NID_PREV != nNaviId) && (NID_NEXT != nNaviId))
        {
            // -2, there's no string for Next/Prev
            sal_uInt16 nResStr = ST_TBL - 2 + nNaviId - NID_START;
            sText = String(SW_RES(nResStr));
            nTbxBits = TIB_CHECKABLE;
        }
        aToolBox.InsertItem(nNaviId, sText, nTbxBits);
        aToolBox.SetHelpId( nNaviId, aNavigationHelpIds[i] );
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
    sal_uInt16 nItemId = SwView::GetMoveType();
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

SwScrollNaviPopup::~SwScrollNaviPopup()
{
}

void SwScrollNaviPopup::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
            ApplyImageList();

    Window::DataChanged( rDCEvt );
}

void SwScrollNaviPopup::ApplyImageList()
{
    ImageList& rImgLst = aIList;
    for(sal_uInt16 i = 0; i < NID_COUNT; i++)
    {
        sal_uInt16 nNaviId = aNavigationInsertIds[i];
        aToolBox.SetItemImage(nNaviId, rImgLst.GetImage(nNaviId));
    }
}

SfxPopupWindow* SwScrollNaviPopup::Clone() const
{
    return new SwScrollNaviPopup( GetId(), GetFrame() );
}

IMPL_LINK(SwScrollNaviPopup, SelectHdl, ToolBox*, pSet)
{
    sal_uInt16 nSet = pSet->GetCurItemId();
    if( nSet != NID_PREV && nSet != NID_NEXT )
    {
        SwView::SetMoveType(nSet);
        aToolBox.SetItemText(NID_NEXT, sQuickHelp[nSet - NID_START]);
        aToolBox.SetItemText(NID_PREV, sQuickHelp[nSet - NID_START + NID_COUNT]);
        aInfoField.SetText(aToolBox.GetItemText(nSet));
        // check the current button only
        for(sal_uInt16 i = 0; i < NID_COUNT; i++)
        {
            sal_uInt16 nItemId = aToolBox.GetItemId( i );
            aToolBox.CheckItem( nItemId, nItemId == nSet );
        }
    }
    else
    {
        SfxBoolItem aNext(FN_SCROLL_NEXT_PREV, NID_NEXT == nSet);
        Any a;
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = OUString( "ScrollNextPrev" );
        aNext.QueryValue( a );
        aArgs[0].Value = a;
        SfxToolBoxControl::Dispatch( Reference< XDispatchProvider >( GetFrame()->getController(), UNO_QUERY ),
                                     OUString( ".uno:ScrollNextPrev" ),
                                     aArgs );
    }
    return 0;
}

void SwScrollNaviToolBox::MouseButtonUp( const MouseEvent& rMEvt )
{
    ToolBox::MouseButtonUp(rMEvt);
    if ( ((SwScrollNaviPopup*)GetParent())->IsInPopupMode() )
        ((SwScrollNaviPopup*)GetParent())->EndPopupMode( FLOATWIN_POPUPMODEEND_CLOSEALL );
}

void  SwScrollNaviToolBox::RequestHelp( const HelpEvent& rHEvt )
{
    SetItemText(NID_NEXT, SwScrollNaviPopup::GetQuickHelpText(sal_True));
    SetItemText(NID_PREV, SwScrollNaviPopup::GetQuickHelpText(sal_False));
    ToolBox::RequestHelp( rHEvt );

}

String  SwScrollNaviPopup::GetQuickHelpText(sal_Bool bNext)
{
    sal_uInt16 nResId = STR_IMGBTN_START;
    nResId += SwView::GetMoveType() - NID_START;
    if(!bNext)
        nResId += NID_COUNT;
    return String(SW_RES(nResId));
}

void SwNaviImageButton::Click()
{
    pPopup = new
        SwScrollNaviPopup( FN_SCROLL_NAVIGATION,
                           m_xFrame );
    Point aPos = OutputToScreenPixel(Point(0,0));
    Rectangle aRect(aPos, GetSizePixel());
    SetPopupWindow( pPopup );
    pPopup->StartPopupMode(aRect, FLOATWIN_POPUPMODE_LEFT|FLOATWIN_POPUPMODE_ALLOWTEAROFF);
}

void SwNaviImageButton::SetPopupWindow( SfxPopupWindow* pWindow )
{
    pPopupWindow = pWindow;
    pPopupWindow->SetPopupModeEndHdl( LINK( this, SwNaviImageButton, PopupModeEndHdl ));
    pPopupWindow->SetDeleteLink_Impl( LINK( this, SwNaviImageButton, ClosePopupWindow ));
}

IMPL_LINK_NOARG(SwNaviImageButton, PopupModeEndHdl)
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

IMPL_LINK( SwNaviImageButton, ClosePopupWindow, SfxPopupWindow *, pWindow )
{
    if ( pWindow == pFloatingWindow )
        pFloatingWindow = 0;
    else
        pPopupWindow = 0;

    return 1;
}

void SwHlpImageButton::RequestHelp( const HelpEvent& rHEvt )
{

    SetQuickHelpText(SwScrollNaviPopup::GetQuickHelpText(!bUp));

    ImageButton::RequestHelp(rHEvt);
}

SwNaviImageButton::SwNaviImageButton(
    Window* pParent,
    const Reference< XFrame >& rFrame ) :
    ImageButton(pParent, SW_RES(BTN_NAVI)),
        pPopup(0),
        aImage(SW_RES(IMG_BTN)),
        sQuickText(SW_RES(ST_QUICK)),
        pPopupWindow(0),
        pFloatingWindow(0),
        m_xFrame( rFrame )
{
    FreeResource();
    SetStyle(GetStyle()|WB_NOPOINTERFOCUS);
    SetQuickHelpText(sQuickText);
    SetModeImage( aImage );
}

void SwNaviImageButton::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
            SetModeImage( aImage );

    Window::DataChanged( rDCEvt );
}

class SwZoomBox_Impl : public ComboBox
{
    sal_uInt16          nSlotId;
    bool            bRelease;
    uno::Reference< frame::XDispatchProvider > m_xDispatchProvider;

public:
    SwZoomBox_Impl(
        Window* pParent,
        sal_uInt16 nSlot,
        const Reference< XDispatchProvider >& rDispatchProvider );
    ~SwZoomBox_Impl();

protected:
    virtual void    Select();
    virtual long    Notify( NotifyEvent& rNEvt );

    void ReleaseFocus();

};

SwZoomBox_Impl::SwZoomBox_Impl(
    Window* pParent,
    sal_uInt16 nSlot,
    const Reference< XDispatchProvider >& rDispatchProvider ):
    ComboBox( pParent, SW_RES(RID_PVIEW_ZOOM_LB)),
    nSlotId(nSlot),
    bRelease(true),
    m_xDispatchProvider( rDispatchProvider )
{
    EnableAutocomplete( sal_False );
    sal_uInt16 aZoomValues[] =
    {   25, 50, 75, 100, 150, 200 };
    for(sal_uInt16 i = 0; i < sizeof(aZoomValues)/sizeof(sal_uInt16); i++)
    {
        String sEntry = OUString::number(aZoomValues[i]);
        sEntry += '%';
        InsertEntry(sEntry);
    }
}

SwZoomBox_Impl::~SwZoomBox_Impl()
{}

void    SwZoomBox_Impl::Select()
{
    if ( !IsTravelSelect() )
    {
        String sEntry(comphelper::string::remove(GetText(), '%'));
        sal_uInt16 nZoom = (sal_uInt16)sEntry.ToInt32();
        if(nZoom < MINZOOM)
            nZoom = MINZOOM;
        if(nZoom > MAXZOOM)
            nZoom = MAXZOOM;

        SfxUInt16Item aItem( nSlotId, nZoom );
        if ( FN_PREVIEW_ZOOM == nSlotId )
        {
            Any a;
            Sequence< PropertyValue > aArgs( 1 );
            aArgs[0].Name = OUString( "PreviewZoom" );
            aItem.QueryValue( a );
            aArgs[0].Value = a;
            SfxToolBoxControl::Dispatch(
                m_xDispatchProvider,
                OUString( ".uno:PreviewZoom" ),
                aArgs );
        }

        ReleaseFocus();
    }
}

long SwZoomBox_Impl::Notify( NotifyEvent& rNEvt )
{
    long nHandled = 0;

    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        sal_uInt16 nCode = rNEvt.GetKeyEvent()->GetKeyCode().GetCode();

        switch ( nCode )
        {
            case KEY_RETURN:
            case KEY_TAB:
            {
                if ( KEY_TAB == nCode )
                    bRelease = false;
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

void SwZoomBox_Impl::ReleaseFocus()
{
    if ( !bRelease )
    {
        bRelease = true;
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

SFX_IMPL_TOOLBOX_CONTROL( SwPreviewZoomControl, SfxUInt16Item);

SwPreviewZoomControl::SwPreviewZoomControl(
    sal_uInt16 nSlotId,
    sal_uInt16 nId,
    ToolBox& rTbx) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
{
}

SwPreviewZoomControl::~SwPreviewZoomControl()
{
}

void SwPreviewZoomControl::StateChanged( sal_uInt16 /*nSID*/,
                                         SfxItemState eState,
                                         const SfxPoolItem* pState )
{
    sal_uInt16 nId = GetId();
    GetToolBox().EnableItem( nId, (GetItemState(pState) != SFX_ITEM_DISABLED) );
    SwZoomBox_Impl* pBox = (SwZoomBox_Impl*)GetToolBox().GetItemWindow( GetId() );
    if(SFX_ITEM_AVAILABLE <= eState)
    {
        String sZoom(OUString::number(((const SfxUInt16Item*)pState)->GetValue()));
        sZoom += '%';
        pBox->SetText(sZoom);
        pBox->SaveValue();
    }
}

Window* SwPreviewZoomControl::CreateItemWindow( Window *pParent )
{
    SwZoomBox_Impl* pRet = new SwZoomBox_Impl( pParent, GetSlotId(), Reference< XDispatchProvider >( m_xFrame->getController(), UNO_QUERY ));
    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
