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
#include <sfx2/mnumgr.hxx>
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
#include <sfx2/zoomitem.hxx>
#include <vcl/svapp.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>

// Size check
#define NAVI_ENTRIES 20
#if NAVI_ENTRIES != NID_COUNT
#error SwScrollNaviPopup-CTOR static array wrong size. Are new IDs added?
#endif

#define ZOOM_ENTRIES 9

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;

SFX_IMPL_TOOLBOX_CONTROL( SwTbxAutoTextCtrl, SfxVoidItem );
SFX_IMPL_TOOLBOX_CONTROL( SwTbxFieldCtrl, SfxBoolItem );

SwTbxAutoTextCtrl::SwTbxAutoTextCtrl(
    sal_uInt16 nSlotId,
    sal_uInt16 nId,
    ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx ),
    pPopup(0)
{
    rTbx.SetItemBits( nId, ToolBoxItemBits::DROPDOWN | rTbx.GetItemBits( nId ) );
}

SwTbxAutoTextCtrl::~SwTbxAutoTextCtrl()
{
    DelPopup();
}

VclPtr<SfxPopupWindow> SwTbxAutoTextCtrl::CreatePopupWindow()
{
    SwView* pView = ::GetActiveView();
    if(pView && !pView->GetDocShell()->IsReadOnly() &&
       !pView->GetWrtShell().HasReadonlySel() )
    {
        Link<Menu*,bool> aLnk = LINK(this, SwTbxAutoTextCtrl, PopupHdl);

        pPopup = new PopupMenu;
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

        ToolBox* pToolBox = &GetToolBox();
        sal_uInt16 nId = GetId();
        pToolBox->SetItemDown( nId, true );

        pPopup->Execute( pToolBox, pToolBox->GetItemRect( nId ),
            (pToolBox->GetAlign() == WindowAlign::Top || pToolBox->GetAlign() == WindowAlign::Bottom) ?
                PopupMenuFlags::ExecuteDown : PopupMenuFlags::ExecuteRight );

        pToolBox->SetItemDown( nId, false );
    }
    GetToolBox().EndSelection();
    DelPopup();
    return 0;

}

void SwTbxAutoTextCtrl::StateChanged( sal_uInt16,
                                              SfxItemState,
                                              const SfxPoolItem* pState )
{
    GetToolBox().EnableItem( GetId(), (GetItemState(pState) != SfxItemState::DISABLED) );
}

IMPL_STATIC_LINK_TYPED(SwTbxAutoTextCtrl, PopupHdl, Menu*, pMenu, bool)
{
    sal_uInt16 nId = pMenu->GetCurItemId();

    sal_uInt16 nBlock = nId / 100;

    SwGlossaryList* pGlossaryList = ::GetGlossaryList();
    OUString sGroup = pGlossaryList->GetGroupName(nBlock - 1, false);
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
    SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits( nId, ToolBoxItemBits::DROPDOWNONLY | rTbx.GetItemBits( nId ) );
}

SwTbxFieldCtrl::~SwTbxFieldCtrl()
{
}

VclPtr<SfxPopupWindow> SwTbxFieldCtrl::CreatePopupWindow()
{
    SwView* pView = ::GetActiveView();
    if(pView && !pView->GetDocShell()->IsReadOnly() &&
       !pView->GetWrtShell().HasReadonlySel() )
    {
        PopupMenu* pPopup = new PopupMenu(SW_RES(RID_INSERT_FIELD_CTRL));

        if (::GetHtmlMode(pView->GetDocShell()) & HTMLMODE_ON)
        {
            pPopup->RemoveItem(pPopup->GetItemPos(FN_INSERT_FLD_PGCOUNT));
            pPopup->RemoveItem(pPopup->GetItemPos(FN_INSERT_FLD_TOPIC));
        }

        ToolBox*      pToolBox = &GetToolBox();
        sal_uInt16    nId      = GetId();
        SfxDispatcher *rDispat = pView->GetViewFrame()->GetDispatcher();

        // set the icons in the Popup-Menu, delete the pPopup
        SfxPopupMenuManager aPop( pPopup, rDispat->GetFrame()->GetBindings() );

        pToolBox->SetItemDown( nId, true );

        pPopup->Execute( pToolBox, pToolBox->GetItemRect( nId ),
                (pToolBox->GetAlign() == WindowAlign::Top || pToolBox->GetAlign() == WindowAlign::Bottom) ?
                 PopupMenuFlags::ExecuteDown : PopupMenuFlags::ExecuteRight );

        pToolBox->SetItemDown( nId, false );
    }

    GetToolBox().EndSelection();

    return 0;
}

void SwTbxFieldCtrl::StateChanged( sal_uInt16,
                                              SfxItemState eState,
                                              const SfxPoolItem* pState )
{
    GetToolBox().EnableItem( GetId(), (GetItemState(pState) != SfxItemState::DISABLED) );
    if (eState >= SfxItemState::DEFAULT)
    {
        GetToolBox().CheckItem( GetId(), static_cast<const SfxBoolItem*>(pState)->GetValue() );
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

SwScrollNaviPopup::SwScrollNaviPopup(sal_uInt16 nId, const Reference< XFrame >& rFrame, vcl::Window *pParent)
    : SfxPopupWindow(nId, pParent, "FloatingNavigation",
        "modules/swriter/ui/floatingnavigation.ui", rFrame),
    aIList(SW_RES(IL_VALUES))
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
        OUString sText;
        ToolBoxItemBits  nTbxBits = ToolBoxItemBits::NONE;
        if((NID_PREV != nNaviId) && (NID_NEXT != nNaviId))
        {
            // -2, there's no string for Next/Prev
            sal_uInt16 nResStr = ST_TBL - 2 + nNaviId - NID_START;
            sText = SW_RESSTR(nResStr);
            nTbxBits = ToolBoxItemBits::CHECKABLE;
        }
        else
        {
            if (nNaviId == NID_PREV)
                sText = SW_RESSTR(STR_IMGBTN_PGE_UP);
            else if (nNaviId == NID_NEXT)
                sText = SW_RESSTR(STR_IMGBTN_PGE_DOWN);
        }
        m_pToolBox->InsertItem(nNaviId, sText, nTbxBits);
        m_pToolBox->SetHelpId( nNaviId, aNavigationHelpIds[i] );
    }
    ApplyImageList();
    m_pToolBox->InsertBreak(NID_COUNT/2);

    // these are global strings
    for( i = 0; i < 2 * NID_COUNT; i++)
    {
        sQuickHelp[i] = SW_RESSTR(STR_IMGBTN_START + i);
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

void SwScrollNaviPopup::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
            ApplyImageList();

    Window::DataChanged( rDCEvt );
}

void SwScrollNaviPopup::ApplyImageList()
{
    ImageList& rImgLst = aIList;
    for(sal_uInt16 i = 0; i < NID_COUNT; i++)
    {
        sal_uInt16 nNaviId = aNavigationInsertIds[i];
        m_pToolBox->SetItemImage(nNaviId, rImgLst.GetImage(nNaviId));
    }
}

IMPL_LINK_TYPED(SwScrollNaviPopup, SelectHdl, ToolBox*, pSet, void)
{
    sal_uInt16 nSet = pSet->GetCurItemId();
    if( nSet != NID_PREV && nSet != NID_NEXT )
    {
        SwView::SetMoveType(nSet);
        m_pToolBox->SetItemText(NID_NEXT, sQuickHelp[nSet - NID_START]);
        m_pToolBox->SetItemText(NID_PREV, sQuickHelp[nSet - NID_START + NID_COUNT]);
        m_pInfoField->SetText(m_pToolBox->GetItemText(nSet));
        // check the current button only
        for(sal_uInt16 i = 0; i < NID_COUNT; i++)
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
        vcl::Window* pParent,
        sal_uInt16 nSlot,
        const Reference< XDispatchProvider >& rDispatchProvider );
    virtual ~SwZoomBox_Impl();

protected:
    virtual void    Select() override;
    virtual bool    Notify( NotifyEvent& rNEvt ) override;

    void ReleaseFocus();

};

SwZoomBox_Impl::SwZoomBox_Impl(
    vcl::Window* pParent,
    sal_uInt16 nSlot,
    const Reference< XDispatchProvider >& rDispatchProvider ):
    ComboBox( pParent, SW_RES(RID_PVIEW_ZOOM_LB)),
    nSlotId(nSlot),
    bRelease(true),
    m_xDispatchProvider( rDispatchProvider )
{
    EnableAutocomplete( false );
    sal_uInt16 aZoomValues[] =
    { RID_SVXSTR_ZOOM_25 , RID_SVXSTR_ZOOM_50 ,
      RID_SVXSTR_ZOOM_75 , RID_SVXSTR_ZOOM_100 ,
      RID_SVXSTR_ZOOM_150 , RID_SVXSTR_ZOOM_200 ,
      RID_SVXSTR_ZOOM_WHOLE_PAGE, RID_SVXSTR_ZOOM_PAGE_WIDTH ,
      RID_SVXSTR_ZOOM_OPTIMAL_VIEW };
    for(sal_uInt16 i = 0; i < ZOOM_ENTRIES ; i++)
    {
        OUString sEntry = SVX_RESSTR( aZoomValues[i] );
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
        SvxZoomItem aZoom(SvxZoomType::PERCENT,100);
        if(sEntry == SVX_RESSTR( RID_SVXSTR_ZOOM_PAGE_WIDTH ) )
            aZoom.SetType(SvxZoomType::PAGEWIDTH);
        else if(sEntry == SVX_RESSTR( RID_SVXSTR_ZOOM_OPTIMAL_VIEW ) )
            aZoom.SetType(SvxZoomType::OPTIMAL);
        else if(sEntry == SVX_RESSTR( RID_SVXSTR_ZOOM_WHOLE_PAGE) )
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

            pCurrentShell->GetDispatcher()->Execute(SID_ATTR_ZOOM, SfxCallMode::ASYNCHRON, &aZoom, 0L);
        }
        ReleaseFocus();
    }
}

bool SwZoomBox_Impl::Notify( NotifyEvent& rNEvt )
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
    VclPtrInstance<SwZoomBox_Impl> pRet( pParent, GetSlotId(), Reference< XDispatchProvider >( m_xFrame->getController(), UNO_QUERY ));
    return pRet.get();
}

class SwJumpToSpecificBox_Impl : public NumericField
{
    sal_uInt16          nSlotId;
    uno::Reference< frame::XDispatchProvider > m_xDispatchProvider;

public:
    SwJumpToSpecificBox_Impl(
        vcl::Window* pParent,
        sal_uInt16 nSlot,
        const Reference< XDispatchProvider >& rDispatchProvider );
    virtual ~SwJumpToSpecificBox_Impl();

protected:
    virtual void    Select();
    virtual bool    Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
};

SwJumpToSpecificBox_Impl::SwJumpToSpecificBox_Impl(
    vcl::Window* pParent,
    sal_uInt16 nSlot,
    const Reference< XDispatchProvider >& rDispatchProvider ):
    NumericField( pParent, SW_RES(RID_JUMP_TO_SPEC_PAGE)),
    nSlotId(nSlot),
    m_xDispatchProvider( rDispatchProvider )
{}

SwJumpToSpecificBox_Impl::~SwJumpToSpecificBox_Impl()
{}

void SwJumpToSpecificBox_Impl::Select()
{
    OUString sEntry(GetText());
    SfxUInt16Item aPageNum(nSlotId);
    aPageNum.SetValue((sal_uInt16)sEntry.toInt32());
    SfxObjectShell* pCurrentShell = SfxObjectShell::Current();
    pCurrentShell->GetDispatcher()->Execute(nSlotId, SfxCallMode::ASYNCHRON, &aPageNum, 0L);
}

bool SwJumpToSpecificBox_Impl::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
        Select();
    return NumericField::Notify( rNEvt );
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
    VclPtrInstance<SwJumpToSpecificBox_Impl> pRet( pParent, GetSlotId(), Reference< XDispatchProvider >( m_xFrame->getController(), UNO_QUERY ));
    return pRet.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
