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

#include <sal/config.h>

#include <string_view>

#include <i18nutil/unicode.hxx>
#include <vcl/InterimItemWindow.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <initui.hxx>
#include <docsh.hxx>
#include <gloshdl.hxx>
#include <gloslst.hxx>
#include <workctrl.hxx>
#include <strings.hrc>
#include <cmdid.h>
#include <helpids.h>
#include <wrtsh.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <vcl/event.hxx>
#include <vcl/menu.hxx>
#include <vcl/settings.hxx>
#include <rtl/ustring.hxx>
#include <swabstdlg.hxx>
#include <sfx2/zoomitem.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weldutils.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <bitmaps.hlst>
#include <toolkit/helper/vclunohelper.hxx>
#include <svx/srchdlg.hxx>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <sfx2/viewfrm.hxx>

// Size check
#define NAVI_ENTRIES 21

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;

SFX_IMPL_TOOLBOX_CONTROL( SwTbxAutoTextCtrl, SfxVoidItem );

SwTbxAutoTextCtrl::SwTbxAutoTextCtrl(
    sal_uInt16 nSlotId,
    ToolBoxItemId nId,
    ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits( nId, ToolBoxItemBits::DROPDOWN | rTbx.GetItemBits( nId ) );
}

SwTbxAutoTextCtrl::~SwTbxAutoTextCtrl()
{
}

void SwTbxAutoTextCtrl::CreatePopupWindow()
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
                sal_uInt16 nIndex = o3tl::narrowing<sal_uInt16>(100*i);
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
        ToolBoxItemId nId = GetId();
        pToolBox->SetItemDown( nId, true );

        pPopup->Execute( pToolBox, pToolBox->GetItemRect( nId ),
            (pToolBox->GetAlign() == WindowAlign::Top || pToolBox->GetAlign() == WindowAlign::Bottom) ?
                PopupMenuFlags::ExecuteDown : PopupMenuFlags::ExecuteRight );

        pToolBox->SetItemDown( nId, false );
    }
    GetToolBox().EndSelection();
}

void SwTbxAutoTextCtrl::StateChangedAtToolBoxControl( sal_uInt16,
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
    NID_TBL,
    NID_FRM,
    NID_GRF,
    NID_OLE,
    NID_PGE,
    NID_OUTL,
    NID_MARK,
    NID_DRW,
    NID_CTRL,
    NID_REG,
    NID_BKM,
    NID_SEL,
    NID_FTN,
    NID_POSTIT,
    NID_SRCH_REP,
    NID_INDEX_ENTRY,
    NID_TABLE_FORMULA,
    NID_TABLE_FORMULA_ERROR,
    NID_RECENCY,
    NID_FIELD,
    NID_FIELD_BYTYPE
};

rtl::OUStringConstExpr constexpr aNavigationImgIds[ NAVI_ENTRIES ] =
{
    RID_BMP_RIBBAR_TBL,
    RID_BMP_RIBBAR_FRM,
    RID_BMP_RIBBAR_GRF,
    RID_BMP_RIBBAR_OLE,
    RID_BMP_RIBBAR_PGE,
    RID_BMP_RIBBAR_OUTL,
    RID_BMP_RIBBAR_MARK,
    RID_BMP_RIBBAR_DRW,
    RID_BMP_RIBBAR_CTRL,
    RID_BMP_RIBBAR_REG,
    RID_BMP_RIBBAR_BKM,
    RID_BMP_RIBBAR_SEL,
    RID_BMP_RIBBAR_FTN,
    RID_BMP_RIBBAR_POSTIT,
    RID_BMP_RIBBAR_REP,
    RID_BMP_RIBBAR_ENTRY,
    RID_BMP_RIBBAR_FORMULA,
    RID_BMP_RIBBAR_ERROR,
    RID_BMP_RIBBAR_RECENCY,
    RID_BMP_RIBBAR_FIELD,
    RID_BMP_RIBBAR_FIELD_BYTYPE
};

const TranslateId aNavigationStrIds[ NAVI_ENTRIES ] =
{
    ST_TBL,
    ST_FRM,
    ST_GRF,
    ST_OLE,
    ST_PGE,
    ST_OUTL,
    ST_MARK,
    ST_DRW,
    ST_CTRL,
    ST_REG,
    ST_BKM,
    ST_SEL,
    ST_FTN,
    ST_POSTIT,
    ST_SRCH_REP,
    ST_INDEX_ENTRY,
    ST_TABLE_FORMULA,
    ST_TABLE_FORMULA_ERROR,
    ST_RECENCY,
    ST_FIELD,
    ST_FIELD_BYTYPE
};

// these are global strings
const TranslateId STR_IMGBTN_ARY[] =
{
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
    STR_IMGBTN_RECENCY_DOWN,
    STR_IMGBTN_FIELD_DOWN,
    STR_IMGBTN_FIELD_BYTYPE_DOWN,
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
    STR_IMGBTN_TBLFML_ERR_UP,
    STR_IMGBTN_RECENCY_UP,
    STR_IMGBTN_FIELD_UP,
    STR_IMGBTN_FIELD_BYTYPE_UP,
};

static OUString lcl_GetScrollToolTip(bool bNext)
{
    sal_uInt16 nResId = SwView::GetMoveType();
    OUString sToolTip = SwResId(STR_IMGBTN_ARY[(!bNext ? NID_COUNT : 0) + nResId - NID_START]);
    if (nResId == NID_FIELD_BYTYPE)
    {
        OUString sFieldType;
        SwWrtShell* pWrtSh = GetActiveWrtShell();
        if (pWrtSh)
        {
            SwField* pCurField = pWrtSh->GetCurField(true);
            if (pCurField)
                sFieldType = SwFieldType::GetTypeStr(pCurField->GetTypeId());
        }
        if (!sFieldType.isEmpty())
            sToolTip = sToolTip.replaceFirst(u"%FIELDTYPE", sFieldType);
        else
            sToolTip = SwResId(SW_STR_NONE);
    }
    return sToolTip;
}

namespace {

class SwZoomBox_Impl final : public InterimItemWindow
{
    std::unique_ptr<weld::ComboBox> m_xWidget;
    sal_uInt16       nSlotId;
    bool             bRelease;

    DECL_LINK(SelectHdl, weld::ComboBox&, void);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(ActivateHdl, weld::ComboBox&, bool);
    DECL_LINK(FocusOutHdl, weld::Widget&, void);

    void Select();

    void ReleaseFocus();

public:
    SwZoomBox_Impl(vcl::Window* pParent, sal_uInt16 nSlot);

    virtual void dispose() override
    {
        m_xWidget.reset();
        InterimItemWindow::dispose();
    }

    void save_value()
    {
        m_xWidget->save_value();
    }

    void set_entry_text(const OUString& rText)
    {
        m_xWidget->set_entry_text(rText);
    }

    virtual ~SwZoomBox_Impl() override
    {
        disposeOnce();
    }
};

}

SwZoomBox_Impl::SwZoomBox_Impl(vcl::Window* pParent, sal_uInt16 nSlot)
    : InterimItemWindow(pParent, "modules/swriter/ui/zoombox.ui", "ZoomBox")
    , m_xWidget(m_xBuilder->weld_combo_box("zoom"))
    , nSlotId(nSlot)
    , bRelease(true)
{
    InitControlBase(m_xWidget.get());

    m_xWidget->set_help_id(HID_PVIEW_ZOOM_LB);
    m_xWidget->set_entry_completion(false);
    m_xWidget->connect_changed(LINK(this, SwZoomBox_Impl, SelectHdl));
    m_xWidget->connect_key_press(LINK(this, SwZoomBox_Impl, KeyInputHdl));
    m_xWidget->connect_entry_activate(LINK(this, SwZoomBox_Impl, ActivateHdl));
    m_xWidget->connect_focus_out(LINK(this, SwZoomBox_Impl, FocusOutHdl));

    const TranslateId aZoomValues[] =
    { RID_SVXSTR_ZOOM_25 , RID_SVXSTR_ZOOM_50 ,
      RID_SVXSTR_ZOOM_75 , RID_SVXSTR_ZOOM_100 ,
      RID_SVXSTR_ZOOM_150 , RID_SVXSTR_ZOOM_200 ,
      RID_SVXSTR_ZOOM_WHOLE_PAGE, RID_SVXSTR_ZOOM_PAGE_WIDTH ,
      RID_SVXSTR_ZOOM_OPTIMAL_VIEW };
    for(const TranslateId& pZoomValue : aZoomValues)
    {
        OUString sEntry = SvxResId(pZoomValue);
        m_xWidget->append_text(sEntry);
    }

    int nWidth = m_xWidget->get_pixel_size(SvxResId(RID_SVXSTR_ZOOM_200)).Width();
    m_xWidget->set_entry_width_chars(std::ceil(nWidth / m_xWidget->get_approximate_digit_width()));

    SetSizePixel(m_xWidget->get_preferred_size());
}

IMPL_LINK(SwZoomBox_Impl, SelectHdl, weld::ComboBox&, rComboBox, void)
{
    if (rComboBox.changed_by_direct_pick())  // only when picked from the list
        Select();
}

IMPL_LINK_NOARG(SwZoomBox_Impl, ActivateHdl, weld::ComboBox&, bool)
{
    Select();
    return true;
}

void SwZoomBox_Impl::Select()
{
    if( FN_PREVIEW_ZOOM == nSlotId )
    {
        bool bNonNumeric = true;

        OUString sEntry = m_xWidget->get_active_text().replaceAll("%", "");
        SvxZoomItem aZoom(SvxZoomType::PERCENT,100);
        if(sEntry == SvxResId( RID_SVXSTR_ZOOM_PAGE_WIDTH ) )
            aZoom.SetType(SvxZoomType::PAGEWIDTH);
        else if(sEntry == SvxResId( RID_SVXSTR_ZOOM_OPTIMAL_VIEW ) )
            aZoom.SetType(SvxZoomType::OPTIMAL);
        else if(sEntry == SvxResId( RID_SVXSTR_ZOOM_WHOLE_PAGE) )
            aZoom.SetType(SvxZoomType::WHOLEPAGE);
        else
        {
            bNonNumeric = false;

            sal_uInt16 nZoom = o3tl::narrowing<sal_uInt16>(sEntry.toInt32());
            if(nZoom < MINZOOM)
                nZoom = MINZOOM;
            if(nZoom > MAXZOOM)
                nZoom = MAXZOOM;
            aZoom.SetValue(nZoom);
        }

        if (bNonNumeric)
        {
            // put old value back, in case its effectively the same
            // as the picked option and no update to number comes
            // back from writer
            m_xWidget->set_entry_text(m_xWidget->get_saved_value());
        }

        SfxObjectShell* pCurrentShell = SfxObjectShell::Current();

        pCurrentShell->GetDispatcher()->ExecuteList(SID_ATTR_ZOOM,
                SfxCallMode::ASYNCHRON, { &aZoom });
    }
    ReleaseFocus();
}

IMPL_LINK(SwZoomBox_Impl, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    bool bHandled = false;

    sal_uInt16 nCode = rKEvt.GetKeyCode().GetCode();

    switch (nCode)
    {
        case KEY_TAB:
            bRelease = false;
            Select();
            break;

        case KEY_ESCAPE:
            m_xWidget->set_entry_text(m_xWidget->get_saved_value());
            ReleaseFocus();
            bHandled = true;
            break;
    }

    return bHandled || ChildKeyInput(rKEvt);
}

IMPL_LINK_NOARG(SwZoomBox_Impl, FocusOutHdl, weld::Widget&, void)
{
    if (!m_xWidget->has_focus()) // a combobox can be comprised of different subwidget so double-check if none of those has focus
        m_xWidget->set_entry_text(m_xWidget->get_saved_value());
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
    ToolBoxItemId nId,
    ToolBox& rTbx) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
{
}

SwPreviewZoomControl::~SwPreviewZoomControl()
{
}

void SwPreviewZoomControl::StateChangedAtToolBoxControl( sal_uInt16 /*nSID*/,
                                         SfxItemState eState,
                                         const SfxPoolItem* pState )
{
    ToolBoxItemId nId = GetId();
    GetToolBox().EnableItem( nId, (GetItemState(pState) != SfxItemState::DISABLED) );
    SwZoomBox_Impl* pBox = static_cast<SwZoomBox_Impl*>(GetToolBox().GetItemWindow( GetId() ));
    if(SfxItemState::DEFAULT <= eState)
    {
        OUString sZoom(unicode::formatPercent(static_cast<const SfxUInt16Item*>(pState)->GetValue(),
            Application::GetSettings().GetUILanguageTag()));
        pBox->set_entry_text(sZoom);
        pBox->save_value();
    }
}

VclPtr<InterimItemWindow> SwPreviewZoomControl::CreateItemWindow( vcl::Window *pParent )
{
    VclPtrInstance<SwZoomBox_Impl> pRet( pParent, GetSlotId() );
    return pRet.get();
}

namespace {

class SwJumpToSpecificBox_Impl final : public InterimItemWindow
{
    std::unique_ptr<weld::Entry> m_xWidget;

    sal_uInt16 nSlotId;

    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(SelectHdl, weld::Entry&, bool);
public:
    SwJumpToSpecificBox_Impl(vcl::Window* pParent, sal_uInt16 nSlot);
    virtual void dispose() override
    {
        m_xWidget.reset();
        InterimItemWindow::dispose();
    }
    virtual ~SwJumpToSpecificBox_Impl() override
    {
        disposeOnce();
    }
};

}

IMPL_LINK(SwJumpToSpecificBox_Impl, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    return ChildKeyInput(rKEvt);
}

SwJumpToSpecificBox_Impl::SwJumpToSpecificBox_Impl(vcl::Window* pParent, sal_uInt16 nSlot)
    : InterimItemWindow(pParent, "modules/swriter/ui/jumpposbox.ui", "JumpPosBox")
    , m_xWidget(m_xBuilder->weld_entry("jumppos"))
    , nSlotId(nSlot)
{
    InitControlBase(m_xWidget.get());

    m_xWidget->connect_key_press(LINK(this, SwJumpToSpecificBox_Impl, KeyInputHdl));
    m_xWidget->connect_activate(LINK(this, SwJumpToSpecificBox_Impl, SelectHdl));

    SetSizePixel(m_xWidget->get_preferred_size());
}

IMPL_LINK_NOARG(SwJumpToSpecificBox_Impl, SelectHdl, weld::Entry&, bool)
{
    OUString sEntry(m_xWidget->get_text());
    SfxUInt16Item aPageNum(nSlotId);
    aPageNum.SetValue(o3tl::narrowing<sal_uInt16>(sEntry.toInt32()));
    SfxObjectShell* pCurrentShell = SfxObjectShell::Current();
    pCurrentShell->GetDispatcher()->ExecuteList(nSlotId, SfxCallMode::ASYNCHRON,
            { &aPageNum });
    return true;
}

SFX_IMPL_TOOLBOX_CONTROL( SwJumpToSpecificPageControl, SfxUInt16Item);

SwJumpToSpecificPageControl::SwJumpToSpecificPageControl(
    sal_uInt16 nSlotId,
    ToolBoxItemId nId,
    ToolBox& rTbx) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
{}

SwJumpToSpecificPageControl::~SwJumpToSpecificPageControl()
{}

VclPtr<InterimItemWindow> SwJumpToSpecificPageControl::CreateItemWindow( vcl::Window *pParent )
{
    VclPtrInstance<SwJumpToSpecificBox_Impl> pRet( pParent, GetSlotId() );
    return pRet.get();
}

namespace {

class NavElementBox_Base;
class NavElementBox_Impl;

class NavElementToolBoxControl : public svt::ToolboxController,
                                 public lang::XServiceInfo
{
    public:
        explicit NavElementToolBoxControl(
            const css::uno::Reference< css::uno::XComponentContext >& rServiceManager );

        // XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
        virtual void SAL_CALL acquire() noexcept override;
        virtual void SAL_CALL release() noexcept override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

        // XComponent
        virtual void SAL_CALL dispose() override;

        // XStatusListener
        virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) override;

        // XToolbarController
        virtual void SAL_CALL execute( sal_Int16 KeyModifier ) override;
        virtual void SAL_CALL click() override;
        virtual void SAL_CALL doubleClick() override;
        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createPopupWindow() override;
        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createItemWindow( const css::uno::Reference< css::awt::XWindow >& Parent ) override;

        void dispatchCommand( const css::uno::Sequence< css::beans::PropertyValue >& rArgs );
        using svt::ToolboxController::dispatchCommand;

    private:
        VclPtr<NavElementBox_Impl> m_xVclBox;
        std::unique_ptr<NavElementBox_Base> m_xWeldBox;
        NavElementBox_Base* m_pBox;
};

class NavElementBox_Base
{
public:
    NavElementBox_Base(std::unique_ptr<weld::ComboBox> xWidget,
                       const uno::Reference<frame::XFrame>& _xFrame,
                       NavElementToolBoxControl& rCtrl);

    virtual ~NavElementBox_Base()
    {
    }

    void set_sensitive(bool bSensitive)
    {
        m_xWidget->set_sensitive(bSensitive);
    }

    void                UpdateBox();

protected:
    std::unique_ptr<weld::ComboBox>            m_xWidget;
    NavElementToolBoxControl*                  m_pCtrl;
    bool                                       m_bRelease;
    uno::Reference< frame::XFrame >            m_xFrame;

    virtual bool DoKeyInput(const KeyEvent& rKEvt);

    DECL_LINK(SelectHdl, weld::ComboBox&, void);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);

    void                ReleaseFocus_Impl();
};


class NavElementBox_Impl final : public InterimItemWindow
                               , public NavElementBox_Base
{
public:
    NavElementBox_Impl(vcl::Window* pParent,
                       const uno::Reference<frame::XFrame>& _xFrame,
                       NavElementToolBoxControl& rCtrl);

    virtual void dispose() override
    {
        m_xWidget.reset();
        InterimItemWindow::dispose();
    }

    virtual void GetFocus() override
    {
        if (m_xWidget)
            m_xWidget->grab_focus();
        InterimItemWindow::GetFocus();
    }

    virtual bool DoKeyInput(const KeyEvent& rKEvt) override;

    virtual ~NavElementBox_Impl() override
    {
        disposeOnce();
    }
};

}

NavElementBox_Base::NavElementBox_Base(
    std::unique_ptr<weld::ComboBox> xWidget,
    const uno::Reference< frame::XFrame >&            _xFrame,
    NavElementToolBoxControl&                         _rCtrl )
    : m_xWidget(std::move(xWidget))
    , m_pCtrl(&_rCtrl)
    , m_bRelease(true)
    , m_xFrame(_xFrame)
{
    m_xWidget->set_size_request(150, -1);

    m_xWidget->make_sorted();
    m_xWidget->freeze();
    for (sal_uInt16 i = 0; i < NID_COUNT; i++)
         m_xWidget->append(OUString::number(aNavigationInsertIds[i]), SwResId(aNavigationStrIds[i]), OUString(aNavigationImgIds[i]));
    m_xWidget->thaw();

    m_xWidget->connect_changed(LINK(this, NavElementBox_Base, SelectHdl));
    m_xWidget->connect_key_press(LINK(this, NavElementBox_Base, KeyInputHdl));
}

NavElementBox_Impl::NavElementBox_Impl(
    vcl::Window*                                      _pParent,
    const uno::Reference< frame::XFrame >&            _xFrame,
    NavElementToolBoxControl&                         _rCtrl )
    : InterimItemWindow(_pParent, "modules/swriter/ui/combobox.ui", "ComboBox")
    , NavElementBox_Base(m_xBuilder->weld_combo_box("combobox"), _xFrame, _rCtrl)
{
    SetSizePixel(m_xContainer->get_preferred_size());
}

void NavElementBox_Base::ReleaseFocus_Impl()
{
    if ( !m_bRelease )
    {
        m_bRelease = true;
        return;
    }

    if ( m_xFrame.is() && m_xFrame->getContainerWindow().is() )
        m_xFrame->getContainerWindow()->setFocus();
}

IMPL_LINK(NavElementBox_Base, SelectHdl, weld::ComboBox&, rComboBox, void)
{
    if (!rComboBox.changed_by_direct_pick())  // only when picked from the list
        return;

    SvxSearchDialogWrapper::SetSearchLabel( SearchLabel::Empty );

    sal_uInt16 nMoveType = rComboBox.get_active_id().toUInt32();
    SwView::SetMoveType( nMoveType );

    css::uno::Sequence< css::beans::PropertyValue > aArgs;
    m_pCtrl->dispatchCommand( aArgs );
}

void NavElementBox_Base::UpdateBox()
{
    sal_uInt16 nMoveType = SwView::GetMoveType();
    for ( size_t i = 0; i < SAL_N_ELEMENTS( aNavigationInsertIds ); ++i )
    {
        if ( nMoveType == aNavigationInsertIds[i] )
        {
            TranslateId id = aNavigationStrIds[i];
            OUString sText = SwResId( id );
            m_xWidget->set_active_text(sText);
            break;
        }
    }
}

IMPL_LINK(NavElementBox_Base, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    return DoKeyInput(rKEvt);
}

bool NavElementBox_Base::DoKeyInput(const KeyEvent& rKEvt)
{
    bool bHandled = false;

    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();
    sal_uInt16 nCode = aKeyCode.GetCode();

    switch ( nCode )
    {
        case KEY_TAB:
        {
            m_bRelease = false;
            SelectHdl(*m_xWidget);
            break;
        }
        case KEY_RETURN:
        {
            m_bRelease = false;
            SelectHdl(*m_xWidget);
            break;
        }
    }

    return bHandled;
}

bool NavElementBox_Impl::DoKeyInput(const KeyEvent& rKEvt)
{
    if (KEY_ESCAPE == rKEvt.GetKeyCode().GetCode())
    {
        ReleaseFocus_Impl();
        return true;
    }
    return NavElementBox_Base::DoKeyInput(rKEvt) || ChildKeyInput(rKEvt);
}


NavElementToolBoxControl::NavElementToolBoxControl( const uno::Reference< uno::XComponentContext >& rxContext )
 : svt::ToolboxController( rxContext,
                           uno::Reference< frame::XFrame >(),
                           ".uno:NavElement" ),
   m_pBox( nullptr )
{
}

// XInterface
css::uno::Any SAL_CALL NavElementToolBoxControl::queryInterface( const css::uno::Type& aType )
{
    uno::Any a = ToolboxController::queryInterface( aType );
    if ( a.hasValue() )
        return a;

    return ::cppu::queryInterface( aType, static_cast< lang::XServiceInfo* >( this ) );
}

void SAL_CALL NavElementToolBoxControl::acquire() noexcept
{
    ToolboxController::acquire();
}

void SAL_CALL NavElementToolBoxControl::release() noexcept
{
    ToolboxController::release();
}

// XServiceInfo
sal_Bool SAL_CALL NavElementToolBoxControl::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

OUString SAL_CALL NavElementToolBoxControl::getImplementationName()
{
    return "lo.writer.NavElementToolBoxController";
}

uno::Sequence< OUString > SAL_CALL NavElementToolBoxControl::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

// XComponent
void SAL_CALL NavElementToolBoxControl::dispose()
{
    svt::ToolboxController::dispose();

    SolarMutexGuard aSolarMutexGuard;
    m_xVclBox.disposeAndClear();
    m_xWeldBox.reset();
    m_pBox = nullptr;
}

// XStatusListener
void SAL_CALL NavElementToolBoxControl::statusChanged( const frame::FeatureStateEvent& rEvent )
{
    if (!m_pBox)
        return;

    SolarMutexGuard aSolarMutexGuard;
    if ( rEvent.FeatureURL.Path != "NavElement" )
        return;

    if ( rEvent.IsEnabled )
    {
        m_pBox->set_sensitive(true);
        m_pBox->UpdateBox();
    }
    else
        m_pBox->set_sensitive(true);

    SwView* pView = GetActiveView();
    if (pView && pView->GetViewFrame())
    {
        pView->GetViewFrame()->GetBindings().Invalidate(FN_SCROLL_NEXT);
        pView->GetViewFrame()->GetBindings().Invalidate(FN_SCROLL_PREV);
    }
}

// XToolbarController
void SAL_CALL NavElementToolBoxControl::execute( sal_Int16 /*KeyModifier*/ )
{
}

void SAL_CALL NavElementToolBoxControl::click()
{
}

void SAL_CALL NavElementToolBoxControl::doubleClick()
{
}

uno::Reference< awt::XWindow > SAL_CALL NavElementToolBoxControl::createPopupWindow()
{
    return uno::Reference< awt::XWindow >();
}

uno::Reference< awt::XWindow > SAL_CALL NavElementToolBoxControl::createItemWindow(
    const uno::Reference< awt::XWindow >& xParent )
{
    uno::Reference< awt::XWindow > xItemWindow;

    if (m_pBuilder)
    {
        SolarMutexGuard aSolarMutexGuard;

        std::unique_ptr<weld::ComboBox> xWidget(m_pBuilder->weld_combo_box("NavElementWidget"));

        xItemWindow = css::uno::Reference<css::awt::XWindow>(new weld::TransportAsXWindow(xWidget.get()));

        m_xWeldBox.reset(new NavElementBox_Base(std::move(xWidget), m_xFrame, *this));
        m_pBox = m_xWeldBox.get();
    }
    else
    {
        VclPtr<vcl::Window> pParent = VCLUnoHelper::GetWindow( xParent );
        if ( pParent )
        {
            SolarMutexGuard aSolarMutexGuard;
            m_xVclBox = VclPtr<NavElementBox_Impl>::Create( pParent, m_xFrame, *this );
            m_pBox = m_xVclBox.get();
            xItemWindow = VCLUnoHelper::GetInterface(m_xVclBox);
        }
    }

    return xItemWindow;
}

void NavElementToolBoxControl::dispatchCommand(
    const uno::Sequence< beans::PropertyValue >& rArgs )
{
    uno::Reference< frame::XDispatchProvider > xDispatchProvider( m_xFrame, uno::UNO_QUERY );
    if ( xDispatchProvider.is() )
    {
        util::URL                               aURL;
        uno::Reference< frame::XDispatch >      xDispatch;
        uno::Reference< util::XURLTransformer > xURLTransformer = getURLTransformer();

        aURL.Complete = ".uno:NavElement";
        xURLTransformer->parseStrict( aURL );
        xDispatch = xDispatchProvider->queryDispatch( aURL, OUString(), 0 );
        if ( xDispatch.is() )
            xDispatch->dispatch( aURL, rArgs );
    }
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
lo_writer_NavElementToolBoxController_get_implementation(
    css::uno::XComponentContext *rxContext,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire( new NavElementToolBoxControl( rxContext ) );
}

namespace {

class PrevNextScrollToolboxController : public svt::ToolboxController,
                                      public css::lang::XServiceInfo
{
public:
    enum Type { PREVIOUS, NEXT };

    PrevNextScrollToolboxController( const css::uno::Reference< css::uno::XComponentContext >& rxContext, Type eType );

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XComponent
    virtual void SAL_CALL dispose() override;

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) override;

private:
    Type                     meType;
};

}

PrevNextScrollToolboxController::PrevNextScrollToolboxController( const css::uno::Reference< css::uno::XComponentContext > & rxContext, Type eType )
    : svt::ToolboxController( rxContext,
            css::uno::Reference< css::frame::XFrame >(),
            (eType == PREVIOUS) ? OUString( ".uno:ScrollToPrevious" ): OUString( ".uno:ScrollToNext" ) ),
      meType( eType )
{
    addStatusListener(".uno:NavElement");
}

// XInterface
css::uno::Any SAL_CALL PrevNextScrollToolboxController::queryInterface( const css::uno::Type& aType )
{
    css::uno::Any a = ToolboxController::queryInterface( aType );
    if ( a.hasValue() )
        return a;

    return ::cppu::queryInterface( aType, static_cast< css::lang::XServiceInfo* >( this ) );
}

void SAL_CALL PrevNextScrollToolboxController::acquire() noexcept
{
    ToolboxController::acquire();
}

void SAL_CALL PrevNextScrollToolboxController::release() noexcept
{
    ToolboxController::release();
}

// XServiceInfo
OUString SAL_CALL PrevNextScrollToolboxController::getImplementationName()
{
    return meType == PrevNextScrollToolboxController::PREVIOUS?
        OUString( "lo.writer.PreviousScrollToolboxController" ) :
        OUString( "lo.writer.NextScrollToolboxController" );
}

sal_Bool SAL_CALL PrevNextScrollToolboxController::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > SAL_CALL PrevNextScrollToolboxController::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

// XComponent
void SAL_CALL PrevNextScrollToolboxController::dispose()
{
    SolarMutexGuard aSolarMutexGuard;

    svt::ToolboxController::dispose();
}

// XStatusListener
void SAL_CALL PrevNextScrollToolboxController::statusChanged( const css::frame::FeatureStateEvent& rEvent )
{
    if (rEvent.FeatureURL.Path == "NavElement")
    {
        if (m_pToolbar)
            m_pToolbar->set_item_tooltip_text(m_aCommandURL.toUtf8(), lcl_GetScrollToolTip(meType != PrevNextScrollToolboxController::PREVIOUS));
        else
        {
            ToolBox* pToolBox = nullptr;
            ToolBoxItemId nId;
            if (getToolboxId(nId, &pToolBox))
                pToolBox->SetQuickHelpText(nId, lcl_GetScrollToolTip(meType != PrevNextScrollToolboxController::PREVIOUS));
        }
    }
    else if (rEvent.FeatureURL.Path == "ScrollToPrevious" || rEvent.FeatureURL.Path == "ScrollToNext")
    {
        if (m_pToolbar)
            m_pToolbar->set_item_sensitive(m_aCommandURL.toUtf8(), rEvent.IsEnabled);
        else
        {
            ToolBox* pToolBox = nullptr;
            ToolBoxItemId nId;
            if (getToolboxId(nId, &pToolBox))
                pToolBox->EnableItem(nId, rEvent.IsEnabled);
        }
    }
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
lo_writer_PreviousScrollToolboxController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire( new PrevNextScrollToolboxController( context, PrevNextScrollToolboxController::PREVIOUS ) );
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
lo_writer_NextScrollToolboxController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire( new PrevNextScrollToolboxController( context, PrevNextScrollToolboxController::NEXT ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
