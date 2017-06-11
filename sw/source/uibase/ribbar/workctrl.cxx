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
#include <sfx2/msgpool.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <initui.hxx>
#include <viewopt.hxx>
#include <docsh.hxx>
#include <gloshdl.hxx>
#include <glosdoc.hxx>
#include <gloslst.hxx>
#include <workctrl.hxx>
#include <strings.hrc>
#include <cmdid.h>
#include <helpid.h>
#include <wrtsh.hxx>
#include <svl/imageitm.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/settings.hxx>
#include <rtl/ustring.hxx>
#include "swabstdlg.hxx"
#include <misc.hrc>
#include <sfx2/zoomitem.hxx>
#include <vcl/svapp.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include <svx/strings.hrc>
#include "bitmaps.hlst"

// Size check
#define NAVI_ENTRIES 20
#if NAVI_ENTRIES != NID_COUNT
#error SwScrollNaviPopup-CTOR static array wrong size. Are new IDs added?
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;

SFX_IMPL_TOOLBOX_CONTROL( SwTbxAutoTextCtrl, SfxVoidItem );

SwTbxAutoTextCtrl::SwTbxAutoTextCtrl(
    sal_uInt16 nSlotId,
    sal_uInt16 nId,
    ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits( nId, ToolBoxItemBits::DROPDOWN | rTbx.GetItemBits( nId ) );
}

SwTbxAutoTextCtrl::~SwTbxAutoTextCtrl()
{
}

VclPtr<SfxPopupWindow> SwTbxAutoTextCtrl::CreatePopupWindow()
{
    SwView* pView = ::GetActiveView();
    if(pView && !pView->GetDocShell()->IsReadOnly() &&
       !pView->GetWrtShell().HasReadonlySel() )
    {
        Link<Menu*,bool> aLnk = LINK(this, SwTbxAutoTextCtrl, PopupHdl);

        ScopedVclPtrInstance<PopupMenu> pPopup;
        SwGlossaryList* pGlossaryList = ::GetGlossaryList();
        const size_t nGroupCount = pGlossaryList->GetGroupCount();
        for(size_t i = 1; i <= nGroupCount; ++i)
        {
            OUString sTitle = pGlossaryList->GetGroupTitle(i - 1);
            const sal_uInt16 nBlockCount = pGlossaryList->GetBlockCount(i -1);
            if(nBlockCount)
            {
                sal_uInt16 nIndex = static_cast<sal_uInt16>(100*i);
                // but insert without extension
                pPopup->InsertItem( i, sTitle);
                VclPtrInstance<PopupMenu> pSub;
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

        ToolBox* pToolBox = &GetToolBox();
        sal_uInt16 nId = GetId();
        pToolBox->SetItemDown( nId, true );

        pPopup->Execute( pToolBox, pToolBox->GetItemRect( nId ),
            (pToolBox->GetAlign() == WindowAlign::Top || pToolBox->GetAlign() == WindowAlign::Bottom) ?
                PopupMenuFlags::ExecuteDown : PopupMenuFlags::ExecuteRight );

        pToolBox->SetItemDown( nId, false );
    }
    GetToolBox().EndSelection();
    return nullptr;
}

void SwTbxAutoTextCtrl::StateChanged( sal_uInt16,
                                              SfxItemState,
                                              const SfxPoolItem* pState )
{
    GetToolBox().EnableItem( GetId(), (GetItemState(pState) != SfxItemState::DISABLED) );
}

IMPL_STATIC_LINK(SwTbxAutoTextCtrl, PopupHdl, Menu*, pMenu, bool)
{
    sal_uInt16 nId = pMenu->GetCurItemId();

    sal_uInt16 nBlock = nId / 100;

    SwGlossaryList* pGlossaryList = ::GetGlossaryList();
    OUString sGroup = pGlossaryList->GetGroupName(nBlock - 1);
    OUString sShortName =
        pGlossaryList->GetBlockShortName(nBlock - 1, nId - (100 * nBlock) - 1);

    SwGlossaryHdl* pGlosHdl = ::GetActiveView()->GetGlosHdl();
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialog creation failed!");
    ::GlossarySetActGroup fnSetActGroup = pFact->SetGlossaryActGroupFunc();
    if ( fnSetActGroup )
        (*fnSetActGroup)( sGroup );
    pGlosHdl->SetCurGroup(sGroup, true);
    pGlosHdl->InsertGlossary(sShortName);

    return false;
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

static OUStringLiteral aNavigationImgIds[ NAVI_ENTRIES ] =
{
    // -- first line
    RID_BMP_RIBBAR_TBL,
    RID_BMP_RIBBAR_FRM,
    RID_BMP_RIBBAR_GRF,
    RID_BMP_RIBBAR_OLE,
    RID_BMP_RIBBAR_PGE,
    RID_BMP_RIBBAR_OUTL,
    RID_BMP_RIBBAR_MARK,
    RID_BMP_RIBBAR_DRW,
    RID_BMP_RIBBAR_CTRL,
    RID_BMP_RIBBAR_PREV,
    // -- second line
    RID_BMP_RIBBAR_REG,
    RID_BMP_RIBBAR_BKM,
    RID_BMP_RIBBAR_SEL,
    RID_BMP_RIBBAR_FTN,
    RID_BMP_RIBBAR_POSTIT,
    RID_BMP_RIBBAR_REP,
    RID_BMP_RIBBAR_ENTRY,
    RID_BMP_RIBBAR_FORMULA,
    RID_BMP_RIBBAR_ERROR,
    RID_BMP_RIBBAR_NEXT
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

static const char* aNavigationStrIds[ NAVI_ENTRIES ] =
{
    // -- first line
    ST_TBL,
    ST_FRM,
    ST_GRF,
    ST_OLE,
    ST_PGE,
    ST_OUTL,
    ST_MARK,
    ST_DRW,
    ST_CTRL,
    STR_IMGBTN_PGE_UP,
    // -- second line
    ST_REG,
    ST_BKM,
    ST_SEL,
    ST_FTN,
    ST_POSTIT,
    ST_SRCH_REP,
    ST_INDEX_ENTRY,
    ST_TABLE_FORMULA,
    ST_TABLE_FORMULA_ERROR,
    STR_IMGBTN_PGE_DOWN
};

// these are global strings
static const char* STR_IMGBTN_ARY[] =
{
    nullptr,
    nullptr,
    STR_IMGBTN_TBL_DOWN,
    STR_IMGBTN_FRM_DOWN,
    STR_IMGBTN_PGE_DOWN,
    STR_IMGBTN_DRW_DOWN,
    STR_IMGBTN_CTRL_DOWN,
    STR_IMGBTN_REG_DOWN,
    STR_IMGBTN_BKM_DOWN,
    STR_IMGBTN_GRF_DOWN,
    STR_IMGBTN_OLE_DOWN,
    STR_IMGBTN_OUTL_DOWN,
    STR_IMGBTN_SEL_DOWN,
    STR_IMGBTN_FTN_DOWN,
    STR_IMGBTN_MARK_DOWN,
    STR_IMGBTN_POSTIT_DOWN,
    STR_IMGBTN_SRCH_REP_DOWN,
    STR_IMGBTN_INDEX_ENTRY_DOWN,
    STR_IMGBTN_TBLFML_DOWN,
    STR_IMGBTN_TBLFML_ERR_DOWN,
    nullptr,
    nullptr,
    STR_IMGBTN_TBL_UP,
    STR_IMGBTN_FRM_UP,
    STR_IMGBTN_PGE_UP,
    STR_IMGBTN_DRW_UP,
    STR_IMGBTN_CTRL_UP,
    STR_IMGBTN_REG_UP,
    STR_IMGBTN_BKM_UP,
    STR_IMGBTN_GRF_UP,
    STR_IMGBTN_OLE_UP,
    STR_IMGBTN_OUTL_UP,
    STR_IMGBTN_SEL_UP,
    STR_IMGBTN_FTN_UP,
    STR_IMGBTN_MARK_UP,
    STR_IMGBTN_POSTIT_UP,
    STR_IMGBTN_SRCH_REP_UP,
    STR_IMGBTN_INDEX_ENTRY_UP,
    STR_IMGBTN_TBLFML_UP,
    STR_IMGBTN_TBLFML_ERR_UP
};

SwScrollNaviPopup::SwScrollNaviPopup(sal_uInt16 nId, const Reference< XFrame >& rFrame, vcl::Window *pParent)
    : SfxPopupWindow(nId, pParent, "FloatingNavigation",
        "modules/swriter/ui/floatingnavigation.ui", rFrame)
{
    m_pToolBox = VclPtr<SwScrollNaviToolBox>::Create(get<vcl::Window>("box"), this, 0);
    get(m_pInfoField, "label");

    sal_uInt16 i;

    m_pToolBox->SetHelpId(HID_NAVI_VS);
    m_pToolBox->SetLineCount( 2 );
    m_pToolBox->SetOutStyle(TOOLBOX_STYLE_FLAT);
    for( i = 0; i < NID_COUNT; i++)
    {
        sal_uInt16 nNaviId = aNavigationInsertIds[i];
        ToolBoxItemBits nTbxBits = ToolBoxItemBits::NONE;
        if ((NID_PREV != nNaviId) && (NID_NEXT != nNaviId))
            nTbxBits = ToolBoxItemBits::CHECKABLE;
        m_pToolBox->InsertItem(nNaviId, Image(BitmapEx(aNavigationImgIds[i])),
                              SwResId(aNavigationStrIds[i]), nTbxBits);
        m_pToolBox->SetHelpId(nNaviId, aNavigationHelpIds[i]);
    }

    m_pToolBox->InsertBreak(NID_COUNT/2);

    for (i = 0; i < SAL_N_ELEMENTS(STR_IMGBTN_ARY); ++i)
    {
        const char* id = STR_IMGBTN_ARY[i];
        if (!id)
            continue;
        sQuickHelp[i] = SwResId(id);
    }

    sal_uInt16 nItemId = SwView::GetMoveType();
    m_pInfoField->SetText(m_pToolBox->GetItemText(nItemId));
    m_pToolBox->CheckItem( nItemId );

    m_pToolBox->SetSelectHdl(LINK(this, SwScrollNaviPopup, SelectHdl));
    m_pToolBox->StartSelection();
    m_pToolBox->Show();
}

SwScrollNaviPopup::~SwScrollNaviPopup()
{
    disposeOnce();
}

void SwScrollNaviPopup::dispose()
{
    m_pToolBox.disposeAndClear();
    m_pInfoField.clear();
    SfxPopupWindow::dispose();
}

IMPL_LINK(SwScrollNaviPopup, SelectHdl, ToolBox*, pSet, void)
{
    sal_uInt16 nSet = pSet->GetCurItemId();
    if( nSet != NID_PREV && nSet != NID_NEXT )
    {
        SwView::SetMoveType(nSet);
        m_pToolBox->SetItemText(NID_NEXT, sQuickHelp[nSet - NID_START]);
        m_pToolBox->SetItemText(NID_PREV, sQuickHelp[nSet - NID_START + NID_COUNT]);
        m_pInfoField->SetText(m_pToolBox->GetItemText(nSet));
        // check the current button only
        for(ToolBox::ImplToolItems::size_type i = 0; i < NID_COUNT; i++)
        {
            sal_uInt16 nItemId = m_pToolBox->GetItemId( i );
            m_pToolBox->CheckItem( nItemId, nItemId == nSet );
        }
    }
    else
    {
        Sequence< PropertyValue > aArgs;
        OUString cmd(".uno:ScrollToPrevious");
        if (NID_NEXT == nSet)
            cmd = ".uno:ScrollToNext";
        SfxToolBoxControl::Dispatch( Reference< XDispatchProvider >( GetFrame()->getController(), UNO_QUERY ),
                                     cmd, aArgs );
    }
}

SwScrollNaviToolBox::~SwScrollNaviToolBox()
{
    disposeOnce();
}

void SwScrollNaviToolBox::dispose()
{
    m_pNaviPopup.disposeAndClear();
    ToolBox::dispose();
}

void SwScrollNaviToolBox::MouseButtonUp( const MouseEvent& rMEvt )
{
    ToolBox::MouseButtonUp(rMEvt);
    if (m_pNaviPopup->IsInPopupMode())
        m_pNaviPopup->EndPopupMode(FloatWinPopupEndFlags::CloseAll);
}

void  SwScrollNaviToolBox::RequestHelp( const HelpEvent& rHEvt )
{
    SetItemText(NID_NEXT, SwScrollNaviPopup::GetToolTip(true));
    SetItemText(NID_PREV, SwScrollNaviPopup::GetToolTip(false));
    ToolBox::RequestHelp( rHEvt );
}

OUString SwScrollNaviPopup::GetToolTip(bool bNext)
{
    sal_uInt16 nResId = SwView::GetMoveType();
    if (!bNext)
        nResId += NID_COUNT;
    const char* id = STR_IMGBTN_ARY[nResId];
    return id ? SwResId(id): OUString();
}

class SwZoomBox_Impl : public ComboBox
{
    sal_uInt16          nSlotId;
    bool            bRelease;

public:
    SwZoomBox_Impl(
        vcl::Window* pParent,
        sal_uInt16 nSlot );

protected:
    virtual void    Select() override;
    virtual bool    EventNotify( NotifyEvent& rNEvt ) override;

    void ReleaseFocus();

};

SwZoomBox_Impl::SwZoomBox_Impl(vcl::Window* pParent, sal_uInt16 nSlot)
    : ComboBox(pParent, WB_HIDE | WB_BORDER | WB_DROPDOWN | WB_AUTOHSCROLL)
    , nSlotId(nSlot)
    , bRelease(true)
{
    SetHelpId(HID_PVIEW_ZOOM_LB);
    SetSizePixel(LogicToPixel(Size(30, 86), MapUnit::MapAppFont));
    EnableAutocomplete( false );
    const char* aZoomValues[] =
    { RID_SVXSTR_ZOOM_25 , RID_SVXSTR_ZOOM_50 ,
      RID_SVXSTR_ZOOM_75 , RID_SVXSTR_ZOOM_100 ,
      RID_SVXSTR_ZOOM_150 , RID_SVXSTR_ZOOM_200 ,
      RID_SVXSTR_ZOOM_WHOLE_PAGE, RID_SVXSTR_ZOOM_PAGE_WIDTH ,
      RID_SVXSTR_ZOOM_OPTIMAL_VIEW };
    for(const char* pZoomValue : aZoomValues)
    {
        OUString sEntry = SvxResId(pZoomValue);
        InsertEntry(sEntry);
    }
}

void    SwZoomBox_Impl::Select()
{
    if ( !IsTravelSelect() )
    {
        OUString sEntry = GetText().replaceAll("%", "");
        SvxZoomItem aZoom(SvxZoomType::PERCENT,100);
        if(sEntry == SvxResId( RID_SVXSTR_ZOOM_PAGE_WIDTH ) )
            aZoom.SetType(SvxZoomType::PAGEWIDTH);
        else if(sEntry == SvxResId( RID_SVXSTR_ZOOM_OPTIMAL_VIEW ) )
            aZoom.SetType(SvxZoomType::OPTIMAL);
        else if(sEntry == SvxResId( RID_SVXSTR_ZOOM_WHOLE_PAGE) )
            aZoom.SetType(SvxZoomType::WHOLEPAGE);
        else
        {
            sal_uInt16 nZoom = (sal_uInt16)sEntry.toInt32();
            if(nZoom < MINZOOM)
                nZoom = MINZOOM;
            if(nZoom > MAXZOOM)
                nZoom = MAXZOOM;
            aZoom.SetValue(nZoom);
        }
        if( FN_PREVIEW_ZOOM == nSlotId )
        {
            SfxObjectShell* pCurrentShell = SfxObjectShell::Current();

            pCurrentShell->GetDispatcher()->ExecuteList(SID_ATTR_ZOOM,
                    SfxCallMode::ASYNCHRON, { &aZoom });
        }
        ReleaseFocus();
    }
}

bool SwZoomBox_Impl::EventNotify( NotifyEvent& rNEvt )
{
    bool bHandled = false;

    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
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
    else if ( MouseNotifyEvent::LOSEFOCUS == rNEvt.GetType() )
    {
        vcl::Window* pFocusWin = Application::GetFocusWindow();
        if ( !HasFocus() && GetSubEdit() != pFocusWin )
            SetText( GetSavedValue() );
    }

    return bHandled || ComboBox::EventNotify(rNEvt);
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
        vcl::Window* pShellWnd = pCurSh->GetWindow();

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
    GetToolBox().EnableItem( nId, (GetItemState(pState) != SfxItemState::DISABLED) );
    SwZoomBox_Impl* pBox = static_cast<SwZoomBox_Impl*>(GetToolBox().GetItemWindow( GetId() ));
    if(SfxItemState::DEFAULT <= eState)
    {
        OUString sZoom(unicode::formatPercent(static_cast<const SfxUInt16Item*>(pState)->GetValue(),
            Application::GetSettings().GetUILanguageTag()));
        pBox->SetText(sZoom);
        pBox->SaveValue();
    }
}

VclPtr<vcl::Window> SwPreviewZoomControl::CreateItemWindow( vcl::Window *pParent )
{
    VclPtrInstance<SwZoomBox_Impl> pRet( pParent, GetSlotId() );
    return pRet.get();
}

class SwJumpToSpecificBox_Impl : public NumericField
{
    sal_uInt16      nSlotId;

public:
    SwJumpToSpecificBox_Impl(vcl::Window* pParent, sal_uInt16 nSlot);

protected:
    void            Select();
    virtual bool    EventNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
};

SwJumpToSpecificBox_Impl::SwJumpToSpecificBox_Impl(vcl::Window* pParent, sal_uInt16 nSlot)
    : NumericField(pParent, WB_HIDE | WB_BORDER)
    , nSlotId(nSlot)
{
    SetSizePixel(LogicToPixel(Size(16, 12), MapUnit::MapAppFont));
}

void SwJumpToSpecificBox_Impl::Select()
{
    OUString sEntry(GetText());
    SfxUInt16Item aPageNum(nSlotId);
    aPageNum.SetValue((sal_uInt16)sEntry.toInt32());
    SfxObjectShell* pCurrentShell = SfxObjectShell::Current();
    pCurrentShell->GetDispatcher()->ExecuteList(nSlotId, SfxCallMode::ASYNCHRON,
            { &aPageNum });
}

bool SwJumpToSpecificBox_Impl::EventNotify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
        Select();
    return NumericField::EventNotify(rNEvt);
}

SFX_IMPL_TOOLBOX_CONTROL( SwJumpToSpecificPageControl, SfxUInt16Item);

SwJumpToSpecificPageControl::SwJumpToSpecificPageControl(
    sal_uInt16 nSlotId,
    sal_uInt16 nId,
    ToolBox& rTbx) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
{}

SwJumpToSpecificPageControl::~SwJumpToSpecificPageControl()
{}

VclPtr<vcl::Window> SwJumpToSpecificPageControl::CreateItemWindow( vcl::Window *pParent )
{
    VclPtrInstance<SwJumpToSpecificBox_Impl> pRet( pParent, GetSlotId() );
    return pRet.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
