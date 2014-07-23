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
#include <i18nutil/unicode.hxx>
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
#include <vcl/settings.hxx>
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
SFX_IMPL_TOOLBOX_CONTROL( SwTbxAutoTextCtrl, SfxVoidItem );
SFX_IMPL_TOOLBOX_CONTROL( SwTbxFieldCtrl, SfxBoolItem );

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

void SAL_CALL SwTbxInsertCtrl::update() throw (uno::RuntimeException, std::exception)
{
    ToolBox& rTbx = GetToolBox();
    OUString aSlotURL = "slot:" + OUString::number( nLastSlotId);
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

            OUString aSlotURL = "slot:" + OUString::number( nId);
            ToolBox& rBox = GetToolBox();
            Image aImage = GetImage( m_xFrame, aSlotURL, hasBigImages() );
            rBox.SetItemImage(GetId(), aImage);
            rBox.SetItemImageMirrorMode( GetId(), false );
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

void SwTbxInsertCtrl::Select( sal_uInt16 /*nSelectModifier*/ )
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

            pPopup = new PopupMenu;
            SwGlossaryList* pGlossaryList = ::GetGlossaryList();
            sal_uInt16 nGroupCount = pGlossaryList->GetGroupCount();
            for(sal_uInt16 i = 1; i <= nGroupCount; i++)
            {
                OUString sTitle = pGlossaryList->GetGroupTitle(i - 1);
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
                        OUString sLongName(pGlossaryList->GetBlockLongName(i - 1, j));
                        OUString sShortName(pGlossaryList->GetBlockShortName(i - 1, j));

                        OUString sEntry = sShortName + " - " + sLongName;
                        pSub->InsertItem(++nIndex, sEntry);
                    }
                }
            }
        }
        ToolBox* pToolBox = &GetToolBox();
        sal_uInt16 nId = GetId();
        pToolBox->SetItemDown( nId, true );

        pPopup->Execute( pToolBox, pToolBox->GetItemRect( nId ),
            (pToolBox->GetAlign() == WINDOWALIGN_TOP || pToolBox->GetAlign() == WINDOWALIGN_BOTTOM) ?
                POPUPMENU_EXECUTE_DOWN : POPUPMENU_EXECUTE_RIGHT );

        pToolBox->SetItemDown( nId, false );
    }
    GetToolBox().EndSelection();
    DelPopup();
    return 0;

}

SfxPopupWindowType SwTbxAutoTextCtrl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONTIMEOUT;
}

void SwTbxAutoTextCtrl::StateChanged( sal_uInt16,
                                              SfxItemState,
                                              const SfxPoolItem* pState )
{
    GetToolBox().EnableItem( GetId(), (GetItemState(pState) != SFX_ITEM_DISABLED) );
}

IMPL_LINK(SwTbxAutoTextCtrl, PopupHdl, PopupMenu*, pMenu)
{
    sal_uInt16 nId = pMenu->GetCurItemId();

    sal_uInt16 nBlock = nId / 100;

    SwGlossaryList* pGlossaryList = ::GetGlossaryList();
    OUString sGroup = pGlossaryList->GetGroupName(nBlock - 1, false);
    OUString sShortName =
        pGlossaryList->GetBlockShortName(nBlock - 1, nId - (100 * nBlock) - 1);

    SwGlossaryHdl* pGlosHdl = pView->GetGlosHdl();
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialogdiet fail!");
    ::GlossarySetActGroup fnSetActGroup = pFact->SetGlossaryActGroupFunc();
    if ( fnSetActGroup )
        (*fnSetActGroup)( sGroup );
    pGlosHdl->SetCurGroup(sGroup, true);
    pGlosHdl->InsertGlossary(sShortName);

    return 0;
}

void SwTbxAutoTextCtrl::DelPopup()
{
    if(pPopup)
    {
        for( sal_uInt16 i = 0; i < pPopup->GetItemCount(); i ++ )
        {
            PopupMenu* pSubPopup = pPopup->GetPopupMenu(pPopup->GetItemId(i));
            delete pSubPopup;
        }
        delete pPopup;
        pPopup = 0;
    }
}

SwTbxFieldCtrl::SwTbxFieldCtrl(
    sal_uInt16 nSlotId,
    sal_uInt16 nId,
    ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx ),
    pPopup(0),
    pView(0)
{
    rTbx.SetItemBits( nId, TIB_DROPDOWN | rTbx.GetItemBits( nId ) );
}

SwTbxFieldCtrl::~SwTbxFieldCtrl()
{
    DelPopup();
}

SfxPopupWindow* SwTbxFieldCtrl::CreatePopupWindow()
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
            Link aLnk = LINK(this, SwTbxFieldCtrl, PopupHdl);

            pPopup = new PopupMenu(SW_RES(RID_INSERT_FIELD_CTRL));
            pPopup->SetSelectHdl(aLnk);

            if (::GetHtmlMode(pView->GetDocShell()) & HTMLMODE_ON)
            {
                pPopup->RemoveItem(pPopup->GetItemPos(FN_INSERT_FLD_PGCOUNT));
                pPopup->RemoveItem(pPopup->GetItemPos(FN_INSERT_FLD_TOPIC));
            }
        }
        ToolBox* pToolBox = &GetToolBox();
        sal_uInt16 nId = GetId();
        pToolBox->SetItemDown( nId, true );

        pPopup->Execute( pToolBox, pToolBox->GetItemRect( nId ),
            (pToolBox->GetAlign() == WINDOWALIGN_TOP || pToolBox->GetAlign() == WINDOWALIGN_BOTTOM) ?
                POPUPMENU_EXECUTE_DOWN : POPUPMENU_EXECUTE_RIGHT );

        pToolBox->SetItemDown( nId, false );
    }
    GetToolBox().EndSelection();
    DelPopup();
    return 0;

}

SfxPopupWindowType SwTbxFieldCtrl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONTIMEOUT;
}

void SwTbxFieldCtrl::StateChanged( sal_uInt16,
                                              SfxItemState eState,
                                              const SfxPoolItem* pState )
{
    GetToolBox().EnableItem( GetId(), (GetItemState(pState) != SFX_ITEM_DISABLED) );
    if (eState >= SFX_ITEM_DEFAULT)
    {
        GetToolBox().CheckItem( GetId(), ((SfxBoolItem*)pState)->GetValue() );
    }
}

IMPL_LINK(SwTbxFieldCtrl, PopupHdl, PopupMenu*, pMenu)
{
    sal_uInt16 nId = pMenu->GetCurItemId();

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

    return 0;
}

void SwTbxFieldCtrl::DelPopup()
{
    if(pPopup)
    {
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

SwScrollNaviPopup::SwScrollNaviPopup(sal_uInt16 nId, const Reference< XFrame >& rFrame, Window *pParent)
    : SfxPopupWindow(nId, rFrame, pParent, SW_RES(RID_SCROLL_NAVIGATION_WIN)),
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
        OUString sText;
        ToolBoxItemBits  nTbxBits = 0;
        if((NID_PREV != nNaviId) && (NID_NEXT != nNaviId))
        {
            // -2, there's no string for Next/Prev
            sal_uInt16 nResStr = ST_TBL - 2 + nNaviId - NID_START;
            sText = SW_RESSTR(nResStr);
            nTbxBits = TIB_CHECKABLE;
        }
        else
        {
            if (nNaviId == NID_PREV)
                sText = SW_RESSTR(STR_IMGBTN_PGE_UP);
            else if (nNaviId == NID_NEXT)
                sText = SW_RESSTR(STR_IMGBTN_PGE_DOWN);
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
        sQuickHelp[i] = SW_RESSTR(STR_IMGBTN_START + i);
    }

    Size aImgSize = aIList.GetImageSize();
    aImgSize.Width() += 5;
    aImgSize.Height() += 5;
    Size aSz = aToolBox.CalcWindowSizePixel(2);
    aToolBox.SetPosSizePixel( Point(), aSz );
    sal_uInt16 nItemId = SwView::GetMoveType();
    aInfoField.SetText(aToolBox.GetItemText(nItemId));
    aToolBox.CheckItem( nItemId, true );
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
    return new SwScrollNaviPopup( GetId(), GetFrame(), GetParent() );
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
        Sequence< PropertyValue > aArgs;
        OUString cmd(".uno:ScrollToPrevious");
        if (NID_NEXT == nSet)
            cmd = OUString(".uno:ScrollToNext");
        SfxToolBoxControl::Dispatch( Reference< XDispatchProvider >( GetFrame()->getController(), UNO_QUERY ),
                                     cmd, aArgs );
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
    SetItemText(NID_NEXT, SwScrollNaviPopup::GetQuickHelpText(true));
    SetItemText(NID_PREV, SwScrollNaviPopup::GetQuickHelpText(false));
    ToolBox::RequestHelp( rHEvt );

}

OUString SwScrollNaviPopup::GetQuickHelpText(bool bNext)
{
    sal_uInt16 nResId = STR_IMGBTN_START;
    nResId += SwView::GetMoveType() - NID_START;
    if(!bNext)
        nResId += NID_COUNT;
    return SW_RESSTR(nResId);
}

void SwHlpImageButton::RequestHelp( const HelpEvent& rHEvt )
{

    SetQuickHelpText(SwScrollNaviPopup::GetQuickHelpText(!bUp));

    ImageButton::RequestHelp(rHEvt);
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
    virtual ~SwZoomBox_Impl();

protected:
    virtual void    Select() SAL_OVERRIDE;
    virtual bool    Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

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
    EnableAutocomplete( false );
    sal_uInt16 aZoomValues[] =
    {   25, 50, 75, 100, 150, 200 };
    for(sal_uInt16 i = 0; i < sizeof(aZoomValues)/sizeof(sal_uInt16); i++)
    {
        OUString sEntry = unicode::formatPercent(aZoomValues[i],
            Application::GetSettings().GetUILanguageTag());
        InsertEntry(sEntry);
    }
}

SwZoomBox_Impl::~SwZoomBox_Impl()
{}

void    SwZoomBox_Impl::Select()
{
    if ( !IsTravelSelect() )
    {
        OUString sEntry(comphelper::string::remove(GetText(), '%'));
        sal_uInt16 nZoom = (sal_uInt16)sEntry.toInt32();
        if(nZoom < MINZOOM)
            nZoom = MINZOOM;
        if(nZoom > MAXZOOM)
            nZoom = MAXZOOM;

        SfxUInt16Item aItem( nSlotId, nZoom );
        if ( FN_PREVIEW_ZOOM == nSlotId )
        {
            Any a;
            Sequence< PropertyValue > aArgs( 1 );
            aArgs[0].Name = "PreviewZoom";
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

bool SwZoomBox_Impl::Notify( NotifyEvent& rNEvt )
{
    bool bHandled = false;

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
                    bHandled = true;
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

    return bHandled || ComboBox::Notify( rNEvt );
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
        OUString sZoom(unicode::formatPercent(((const SfxUInt16Item*)pState)->GetValue(),
            Application::GetSettings().GetUILanguageTag()));
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
