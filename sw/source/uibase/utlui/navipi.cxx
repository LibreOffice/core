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

#include <comphelper/string.hxx>
#include <svl/urlbmk.hxx>
#include <svl/stritem.hxx>
#include <vcl/graphicfilter.hxx>
#include <sot/formats.hxx>
#include <sot/filelist.hxx>
#include <sfx2/event.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <tools/urlobj.hxx>
#include <osl/diagnose.h>
#include <swtypes.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <navicfg.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <navipi.hxx>
#include <edtwin.hxx>
#include <sfx2/app.hxx>
#include <cmdid.h>
#include <helpids.h>

#include <strings.hrc>
#include <bitmaps.hlst>

#include <memory>

#include <o3tl/enumrange.hxx>

#include <workctrl.hxx>

#include <comphelper/lok.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;

// Filter the control characters out of the Outline-Entry
OUString SwNavigationPI::CleanEntry(const OUString& rEntry)
{
    if (rEntry.isEmpty())
        return rEntry;

    OUStringBuffer aEntry(rEntry);
    for (sal_Int32 i = 0; i < rEntry.getLength(); ++i)
        if(aEntry[i] == 10 || aEntry[i] == 9)
            aEntry[i] = 0x20;

    return aEntry.makeStringAndClear();
}

// Execution of the drag operation with and without the children.

void SwNavigationPI::MoveOutline(SwOutlineNodes::size_type nSource, SwOutlineNodes::size_type nTarget)
{
    SwView *pView = GetCreateView();
    if (!pView)
        return;
    SwWrtShell &rSh = pView->GetWrtShell();
    if(nTarget < nSource || nTarget == SwOutlineNodes::npos)
        nTarget ++;
    if ( !rSh.IsOutlineMovable( nSource ))
        return;

    SwOutlineNodes::difference_type nMove = nTarget-nSource; //( nDir<0 ) ? 1 : 0 ;
    rSh.GotoOutline(nSource);
    rSh.MakeOutlineSel(nSource, nSource, true);
    // While moving, the selected children does not counting.
    const SwOutlineNodes::size_type nLastOutlinePos = rSh.GetOutlinePos(MAXLEVEL);
    if(nMove > 1 && nLastOutlinePos < nTarget)
    {
        if(!rSh.IsCursorPtAtEnd())
            rSh.SwapPam();
        nMove -= nLastOutlinePos - nSource;
    }
    if( nMove < 1 || nLastOutlinePos < nTarget )
        rSh.MoveOutlinePara( nMove );
    rSh.ClearMark();
    rSh.GotoOutline( nSource + nMove);

}

// After goto cancel the status frame selection
static void lcl_UnSelectFrame(SwWrtShell *pSh)
{
    if (pSh->IsFrameSelected())
    {
        pSh->UnSelectFrame();
        pSh->LeaveSelFrameMode();
    }
}

// Select the document view
IMPL_LINK(SwNavigationPI, DocListBoxSelectHdl, weld::ComboBox&, rBox, void)
{
    int nEntryIdx = rBox.get_active();
    SwView *pView ;
    pView = SwModule::GetFirstView();
    while (nEntryIdx-- && pView)
    {
        pView = SwModule::GetNextView(pView);
    }
    if(!pView)
    {
        nEntryIdx == 0 ?
            m_xContentTree->ShowHiddenShell():
                m_xContentTree->ShowActualView();

    }
    else
    {
        m_xContentTree->SetConstantShell(pView->GetWrtShellPtr());
    }
}

void SwNavigationPI::UpdateNavigateBy()
{
    if (!m_pNavigateByComboBox)
        return;
    SfxUInt32Item aParam(FN_NAV_ELEMENT, m_pNavigateByComboBox->get_active_id().toUInt32());
    const SfxPoolItem* aArgs[2];
    aArgs[0] = &aParam;
    aArgs[1] = nullptr;
    SfxDispatcher* pDispatcher = GetCreateView()->GetFrame()->GetDispatcher();
    pDispatcher->Execute(FN_NAV_ELEMENT, SfxCallMode::SYNCHRON, aArgs);
}

IMPL_LINK(SwNavigationPI, NavigateByComboBoxSelectHdl, weld::ComboBox&, rComboBox, void)
{
    m_xContentTree->SelectContentType(rComboBox.get_active_text());
    UpdateNavigateBy();
}

void SwNavigationPI::SetContent3And4ToolBoxVisibility()
{
    if (IsGlobalMode())
        return;
    bool bIsMoveTypePage = SwView::GetMoveType() == NID_PGE;
    m_xContent3ToolBox->set_visible(!bIsMoveTypePage);
    m_xContent4ToolBox->set_visible(bIsMoveTypePage);
}

// Filling of the list box for outline view or documents
// The PI will be set to full size
void SwNavigationPI::FillBox()
{
    if(m_pContentWrtShell)
    {
        m_xContentTree->SetHiddenShell( m_pContentWrtShell );
        m_xContentTree->Display(  false );
    }
    else
    {
        SwView *pView = GetCreateView();
        if(!pView)
        {
            m_xContentTree->SetActiveShell(nullptr);
        }
        else if( pView != m_pActContView)
        {
            SwWrtShell* pWrtShell = pView->GetWrtShellPtr();
            m_xContentTree->SetActiveShell(pWrtShell);
        }
        else
            m_xContentTree->Display( true );
        m_pActContView = pView;
        if (m_pActContView)
            m_xContentTree->UpdateTracking();
    }
}

// Select handler of the toolboxes
IMPL_LINK(SwNavigationPI, ToolBoxSelectHdl, const OUString&, rCommand, void)
{
    SwView *pView = GetCreateView();
    if (!pView)
        return;
    SwWrtShell &rSh = pView->GetWrtShell();
    // Get MouseModifier for Outline-Move

    int nFuncId = 0;
    bool bFocusToDoc = false;
    if (rCommand == "root")
    {
        m_xContentTree->ToggleToRoot();
    }
    else if (rCommand == "listbox")
    {
        if (ParentIsFloatingWindow(m_xNavigatorDlg))
        {
            if (IsZoomedIn())
            {
                ZoomOut();
            }
            else
            {
                ZoomIn();
            }
        }
        return;
    }
    // Functions that will trigger a direct action.
    else if (rCommand == "footer")
    {
        rSh.MoveCursor();
        const FrameTypeFlags eType = rSh.GetFrameType(nullptr,false);
        if (eType & FrameTypeFlags::FOOTER)
        {
            if (rSh.EndPg())
                nFuncId = FN_END_OF_PAGE;
        }
        else if (rSh.GotoFooterText())
            nFuncId = FN_TO_FOOTER;
        bFocusToDoc = true;
    }
    else if (rCommand == "header")
    {
        rSh.MoveCursor();
        const FrameTypeFlags eType = rSh.GetFrameType(nullptr,false);
        if (eType & FrameTypeFlags::HEADER)
        {
            if (rSh.SttPg())
                nFuncId = FN_START_OF_PAGE;
        }
        else if (rSh.GotoHeaderText())
            nFuncId = FN_TO_HEADER;
        bFocusToDoc = true;
    }
    else if (rCommand == "anchor")
    {
        rSh.MoveCursor();
        const FrameTypeFlags eFrameType = rSh.GetFrameType(nullptr,false);
            // Jump from the footnote to the anchor.
        if (eFrameType & FrameTypeFlags::FOOTNOTE)
        {
            if (rSh.GotoFootnoteAnchor())
                nFuncId = FN_FOOTNOTE_TO_ANCHOR;
        }
            // Otherwise, jump to the first footnote text;
            // go to the next footnote if this is not possible;
            // if this is also not possible got to the footnote before.
        else
        {
            if (rSh.GotoFootnoteText())
                nFuncId = FN_FOOTNOTE_TO_ANCHOR;
            else if (rSh.GotoNextFootnoteAnchor())
                nFuncId = FN_NEXT_FOOTNOTE;
            else if (rSh.GotoPrevFootnoteAnchor())
                nFuncId = FN_PREV_FOOTNOTE;
        }
        bFocusToDoc = true;
    }
    else if (rCommand == "reminder")
    {
        rSh.GetView().GetViewFrame().GetDispatcher()->Execute(FN_SET_REMINDER, SfxCallMode::ASYNCHRON);
    }
    else if (rCommand == "chapterdown" ||
             rCommand == "movedown" ||
             rCommand == "chapterup" ||
             rCommand == "moveup" ||
             rCommand == "promote" ||
             rCommand == "demote" ||
             rCommand == "edit")
    {
        if (IsGlobalMode())
            m_xGlobalTree->ExecCommand(rCommand);
        else
        {
            // Standard: sublevels are taken
            // do not take sublevels with Ctrl
            bool bOutlineWithChildren = (KEY_MOD1 != m_xContent6ToolBox->get_modifier_state());
            m_xContentTree->ExecCommand(rCommand, bOutlineWithChildren);
        }
    }
    else if (rCommand == "contenttoggle" || rCommand == "globaltoggle")
    {
        ToggleTree();
        bool bGlobalMode = IsGlobalMode();
        m_pConfig->SetGlobalActive(bGlobalMode);
        m_xGlobalToolBox->set_item_active("globaltoggle", bGlobalMode);
    }
    else if (rCommand == "save")
    {
        bool bSave = rSh.IsGlblDocSaveLinks();
        rSh.SetGlblDocSaveLinks( !bSave );
        m_xGlobalToolBox->set_item_active(rCommand, !bSave);
    }
    else if (rCommand == "headings")
        m_xContent5ToolBox->set_menu_item_active("headings", !m_xContent5ToolBox->get_menu_item_active("headings"));
    else if (rCommand == "update")
        m_xGlobalToolBox->set_menu_item_active("update", !m_xGlobalToolBox->get_menu_item_active("update"));
    else if (rCommand == "insert")
        m_xGlobalToolBox->set_menu_item_active("insert", !m_xGlobalToolBox->get_menu_item_active("insert"));

    if (nFuncId)
        lcl_UnSelectFrame(&rSh);
    if (bFocusToDoc)
        pView->GetEditWin().GrabFocus();
}

// Click handler of the toolboxes
IMPL_LINK(SwNavigationPI, ToolBoxClickHdl, const OUString&, rCommand, void)
{
    if (!m_xGlobalToolBox->get_menu_item_active(rCommand))
        return;

    if (rCommand == "update")
        m_xGlobalTree->TbxMenuHdl(rCommand, *m_xUpdateMenu);
    else if (rCommand == "insert")
        m_xGlobalTree->TbxMenuHdl(rCommand, *m_xInsertMenu);
}

IMPL_LINK(SwNavigationPI, GlobalMenuSelectHdl, const OUString&, rIdent, void)
{
    m_xGlobalTree->ExecuteContextMenuAction(rIdent);
}

IMPL_LINK(SwNavigationPI, ToolBox5DropdownClickHdl, const OUString&, rCommand, void)
{
    if (!m_xContent5ToolBox->get_menu_item_active(rCommand))
        return;

    if (rCommand == "headings")
        m_xHeadingsMenu->set_active(OUString::number(m_xContentTree->GetOutlineLevel()), true);
}

void SwNavigationPI::ZoomOut()
{
    if (!IsZoomedIn())
        return;
    SfxNavigator* pNav = m_xNavigatorDlg.get();
    if (!pNav)
        return;
    m_bIsZoomedIn = false;
    FillBox();
    if (IsGlobalMode())
    {
        m_xGlobalBox->show();
        m_xGlobalTree->ShowTree();
    }
    else
    {
        m_xContentBox->show();
        m_xContentTree->ShowTree();
        m_xDocListBox->show();
    }

    pNav->InvalidateChildSizeCache();
    Size aOptimalSize(pNav->GetOptimalSize());
    Size aNewSize(pNav->GetOutputSizePixel());
    aNewSize.setHeight( m_aExpandedSize.Height() );
    pNav->SetMinOutputSizePixel(aOptimalSize);
    pNav->SetOutputSizePixel(aNewSize);

    m_xContentTree->Select(); // Enable toolbox
    m_pConfig->SetSmall(false);
    m_xContent6ToolBox->set_item_active("listbox", true);
}

void SwNavigationPI::ZoomIn()
{
    if (IsZoomedIn())
        return;
    SfxNavigator* pNav = m_xNavigatorDlg.get();
    if (!pNav)
        return;

    m_aExpandedSize = m_xNavigatorDlg->GetSizePixel();

    m_xContentBox->hide();
    m_xContentTree->HideTree();
    m_xGlobalBox->hide();
    m_xGlobalTree->HideTree();
    m_xDocListBox->hide();
    m_bIsZoomedIn = true;

    pNav->InvalidateChildSizeCache();
    Size aOptimalSize(pNav->GetOptimalSize());
    Size aNewSize(pNav->GetOutputSizePixel());
    aNewSize.setHeight( aOptimalSize.Height() );
    pNav->SetMinOutputSizePixel(aOptimalSize);
    pNav->SetOutputSizePixel(aNewSize);

    m_xContentTree->Select(); // Enable toolbox

    m_pConfig->SetSmall(true);
    m_xContent6ToolBox->set_item_active("listbox", false);
}

std::unique_ptr<PanelLayout> SwNavigationPI::Create(weld::Widget* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
{
    if( pParent == nullptr )
        throw css::lang::IllegalArgumentException("no parent window given to SwNavigationPI::Create", nullptr, 0);
    if( !rxFrame.is() )
        throw css::lang::IllegalArgumentException("no XFrame given to SwNavigationPI::Create", nullptr, 0);
    if( pBindings == nullptr )
        throw css::lang::IllegalArgumentException("no SfxBindings given to SwNavigationPI::Create", nullptr, 0);
    return std::make_unique<SwNavigationPI>(pParent, rxFrame, pBindings, nullptr);
}

SwNavigationPI::SwNavigationPI(weld::Widget* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* _pBindings, SfxNavigator* pNavigatorDlg)
    : PanelLayout(pParent, "NavigatorPanel", "modules/swriter/ui/navigatorpanel.ui")
    , m_aDocFullName(SID_DOCFULLNAME, *_pBindings, *this)
    , m_aPageStats(FN_STAT_PAGE, *_pBindings, *this)
    , m_aNavElement(FN_NAV_ELEMENT, *_pBindings, *this)
    , m_xContent1ToolBox(m_xBuilder->weld_toolbar("content1"))
    , m_xContent2ToolBox(m_xBuilder->weld_toolbar("content2"))
    , m_xContent3ToolBox(m_xBuilder->weld_toolbar("content3"))
    , m_xContent4ToolBox(m_xBuilder->weld_toolbar("content4"))
    , m_xContent5ToolBox(m_xBuilder->weld_toolbar("content5"))
    , m_xContent6ToolBox(m_xBuilder->weld_toolbar("content6"))
    , m_xContent2Dispatch(new ToolbarUnoDispatcher(*m_xContent2ToolBox, *m_xBuilder, rxFrame))
    , m_xContent3Dispatch(new ToolbarUnoDispatcher(*m_xContent3ToolBox, *m_xBuilder, rxFrame))
    , m_xHeadingsMenu(m_xBuilder->weld_menu("headingsmenu"))
    , m_xUpdateMenu(m_xBuilder->weld_menu("updatemenu"))
    , m_xInsertMenu(m_xBuilder->weld_menu("insertmenu"))
    , m_xGlobalToolBox(m_xBuilder->weld_toolbar("global"))
    , m_xGotoPageSpinButton(m_xBuilder->weld_spin_button("gotopage"))
    , m_xContentBox(m_xBuilder->weld_widget("contentbox"))
    , m_xContentTree(new SwContentTree(m_xBuilder->weld_tree_view("contenttree"), this))
    , m_xGlobalBox(m_xBuilder->weld_widget("globalbox"))
    , m_xGlobalTree(new SwGlobalTree(m_xBuilder->weld_tree_view("globaltree"), this))
    , m_xDocListBox(m_xBuilder->weld_combo_box("documents"))
    , m_xNavigatorDlg(pNavigatorDlg)
    , m_pContentView(nullptr)
    , m_pContentWrtShell(nullptr)
    , m_pActContView(nullptr)
    , m_pCreateView(nullptr)
    , m_pConfig(SW_MOD()->GetNavigationConfig())
    , m_rBindings(*_pBindings)
    , m_bIsZoomedIn(false)
    , m_bGlobalMode(false)
{
    m_xContainer->connect_container_focus_changed(LINK(this, SwNavigationPI, SetFocusChildHdl));

    Reference<XToolbarController> xController =
            m_xContent2Dispatch->GetControllerForCommand(".uno:NavElement");
    NavElementToolBoxControl* pToolBoxControl =
            dynamic_cast<NavElementToolBoxControl*>(xController.get());

    // In case of LOK, the xController may not a NavElementToolBoxControl
    if (comphelper::LibreOfficeKit::isActive() && !pToolBoxControl)
    {
        m_pNavigateByComboBox = nullptr;
    }
    else
    {
        assert(pToolBoxControl);
        m_pNavigateByComboBox = pToolBoxControl->GetComboBox();
        SetContent3And4ToolBoxVisibility();
    }

    // Restore content tree settings before calling UpdateInitShow. UpdateInitShow calls Fillbox,
    // which calls Display and UpdateTracking. Incorrect outline levels could be displayed and
    // unexpected content tracking could occur if these content tree settings are not done before.
    m_xContentTree->SetOutlineLevel(static_cast<sal_uInt8>(m_pConfig->GetOutlineLevel()));
    m_xContentTree->SetOutlineTracking(static_cast<sal_uInt8>(m_pConfig->GetOutlineTracking()));
    for (ContentTypeId eCntTypeId : o3tl::enumrange<ContentTypeId>())
    {
        if (eCntTypeId != ContentTypeId::OUTLINE)
            m_xContentTree->SetContentTypeTracking(
                        eCntTypeId, m_pConfig->IsContentTypeTrack(eCntTypeId));
    }

    if (const ContentTypeId nRootType = m_pConfig->GetRootType();
            nRootType != ContentTypeId::UNKNOWN)
    {
        m_xContentTree->SetRootType(nRootType);
        m_xContent5ToolBox->set_item_active("root", true);
        if (nRootType == ContentTypeId::OUTLINE || nRootType == ContentTypeId::DRAWOBJECT)
            m_xContentTree->set_selection_mode(SelectionMode::Multiple);
        else
            m_xContentTree->set_selection_mode(SelectionMode::Single);
    }
    else
        m_xContentTree->set_selection_mode(SelectionMode::Single);

    UpdateInitShow();

    GetCreateView();

    m_xContent1ToolBox->set_help_id(HID_NAVIGATOR_TOOLBOX);
    m_xContent2ToolBox->set_help_id(HID_NAVIGATOR_TOOLBOX);
    m_xContent3ToolBox->set_help_id(HID_NAVIGATOR_TOOLBOX);
    m_xContent4ToolBox->set_help_id(HID_NAVIGATOR_TOOLBOX);
    m_xContent5ToolBox->set_help_id(HID_NAVIGATOR_TOOLBOX);
    m_xContent6ToolBox->set_help_id(HID_NAVIGATOR_TOOLBOX);
    m_xGlobalToolBox->set_help_id(HID_NAVIGATOR_GLOBAL_TOOLBOX);
    m_xDocListBox->set_help_id(HID_NAVIGATOR_LISTBOX);
    m_xDocListBox->set_size_request(42, -1); // set a nominal width so it takes width of surroundings

    bool bFloatingNavigator = ParentIsFloatingWindow(m_xNavigatorDlg);

    m_xContentTree->ShowTree();
    m_xContent6ToolBox->set_item_active("listbox", true);
    m_xContent6ToolBox->set_item_sensitive("listbox", bFloatingNavigator);

//  TreeListBox for global document
    m_xGlobalTree->set_selection_mode(SelectionMode::Multiple);

//  Handler
    Link<const OUString&, void> aLk = LINK(this, SwNavigationPI, ToolBoxSelectHdl);
    m_xContent1ToolBox->connect_clicked(aLk);
    m_xContent5ToolBox->connect_clicked(aLk);
    m_xContent6ToolBox->connect_clicked(aLk);
    m_xGlobalToolBox->connect_clicked(aLk);
    m_xDocListBox->connect_changed(LINK(this, SwNavigationPI, DocListBoxSelectHdl));
    m_xContent5ToolBox->set_item_menu("headings", m_xHeadingsMenu.get());
    m_xHeadingsMenu->connect_activate(LINK(this, SwNavigationPI, HeadingsMenuSelectHdl));
    m_xContent5ToolBox->connect_menu_toggled(LINK(this, SwNavigationPI, ToolBox5DropdownClickHdl));
    m_xGlobalToolBox->set_item_menu("update", m_xUpdateMenu.get());
    m_xUpdateMenu->connect_activate(LINK(this, SwNavigationPI, GlobalMenuSelectHdl));
    m_xGlobalToolBox->set_item_menu("insert", m_xInsertMenu.get());
    m_xInsertMenu->connect_activate(LINK(this, SwNavigationPI, GlobalMenuSelectHdl));
    m_xGlobalToolBox->connect_menu_toggled(LINK(this, SwNavigationPI, ToolBoxClickHdl));
    m_xGlobalToolBox->set_item_active("globaltoggle", true);
    if (m_pNavigateByComboBox)
        m_pNavigateByComboBox->connect_changed(
            LINK(this, SwNavigationPI, NavigateByComboBoxSelectHdl));
    m_xGotoPageSpinButton->connect_value_changed(
        LINK(this, SwNavigationPI, GotoPageSpinButtonValueChangedHdl));

    StartListening(*SfxGetpApp());

    if(IsGlobalDoc())
    {
        SwView *pActView = GetCreateView();
        if (pActView && pActView->GetWrtShellPtr())
            m_xGlobalToolBox->set_item_active("save",
                        pActView->GetWrtShellPtr()->IsGlblDocSaveLinks());
        if (m_pConfig->IsGlobalActive())
            ToggleTree();
        else
            m_xContent1ToolBox->set_visible(true);
        if (bFloatingNavigator)
            m_xGlobalTree->grab_focus();
    }
    else if (bFloatingNavigator)
        m_xContentTree->grab_focus();

    m_xContentTree->set_accessible_name(SwResId(STR_ACCESS_TL_CONTENT));
    m_xGlobalTree->set_accessible_name(SwResId(STR_ACCESS_TL_GLOBAL));
    m_xDocListBox->set_accessible_name(SwResId(STR_ACTIVE_VIEW));

    m_aExpandedSize = m_xContainer->get_preferred_size();

    if(comphelper::LibreOfficeKit::isActive())
    {
        m_xBuilder->weld_container("gridcontent16")->hide();
        m_xDocListBox->hide();
        m_xGlobalBox->hide();
        m_xGlobalToolBox->hide();
        m_xGlobalTree->HideTree();

        //Open Headings by default
        SwView *pView = GetCreateView();
        if (pView && pView->m_nNaviExpandedStatus < 0)
        {
            pView->m_nNaviExpandedStatus = 1;
            m_xContentTree->ExpandAllHeadings();
        }
    }
}

weld::Window* SwNavigationPI::GetFrameWeld() const
{
    if (m_xNavigatorDlg)
        return m_xNavigatorDlg->GetFrameWeld();
    return PanelLayout::GetFrameWeld();
}

IMPL_LINK_NOARG(SwNavigationPI, GotoPageSpinButtonValueChangedHdl, weld::SpinButton&, void)
{
    auto nPage = m_xGotoPageSpinButton->get_value();
    SwView *pView = GetCreateView();
    SwWrtShell &rSh = pView->GetWrtShell();
    auto nPageCount = rSh.GetPageCount();
    if (nPage > nPageCount)
    {
        nPage = nPageCount;
        m_xGotoPageSpinButton->set_text(OUString::number(nPage));
    }
    rSh.LockView(true);
    rSh.GotoPage(nPage, false);
    // adjust the visible area so that the top of the page is at the top of the view
    const Point aPt(pView->GetVisArea().Left(), rSh.GetPagePos(nPage).Y());
    pView->SetVisArea(aPt);
    rSh.LockView(false);
}

SwNavigationPI::~SwNavigationPI()
{
    if (IsGlobalDoc() && !IsGlobalMode())
    {
        SwView *pView = GetCreateView();
        SwWrtShell &rSh = pView->GetWrtShell();
        if (!rSh.IsAllProtect())
            pView->GetDocShell()->SetReadOnlyUI(false);
    }

    EndListening(*SfxGetpApp());

    if (m_oObjectShell)
    {
        if (m_oObjectShell->Is())
            (*m_oObjectShell)->DoClose();
        m_oObjectShell.reset();
    }

    m_xDocListBox.reset();
    m_xGlobalTree.reset();
    m_xGlobalBox.reset();
    m_xContentTree.reset();
    m_xContentBox.reset();
    m_xGlobalToolBox.reset();
    m_xGotoPageSpinButton.reset();
    m_xHeadingsMenu.reset();
    m_xUpdateMenu.reset();
    m_xInsertMenu.reset();
    m_xContent2Dispatch.reset();
    m_xContent3Dispatch.reset();
    m_xContent1ToolBox.reset();
    m_xContent2ToolBox.reset();
    m_xContent3ToolBox.reset();
    m_xContent4ToolBox.reset();
    m_xContent5ToolBox.reset();
    m_xContent6ToolBox.reset();

    m_aDocFullName.dispose();
    m_aPageStats.dispose();
    m_aNavElement.dispose();
}

void SwNavigationPI::NotifyItemUpdate(sal_uInt16 nSID, SfxItemState /*eState*/,
                                      const SfxPoolItem* /*pState*/)
{
    switch (nSID)
    {
        case SID_DOCFULLNAME:
        {
            SwView *pActView = GetCreateView();
            if(pActView)
            {
                SwWrtShell* pWrtShell = pActView->GetWrtShellPtr();
                m_xContentTree->SetActiveShell(pWrtShell);
                if (IsGlobalDoc())
                {
                    m_xGlobalToolBox->set_item_active("save", pWrtShell->IsGlblDocSaveLinks());
                }
            }
            else
            {
                m_xContentTree->SetActiveShell(nullptr);
            }
            UpdateListBox();
        }
        break;
        case FN_NAV_ELEMENT:
            SetContent3And4ToolBoxVisibility();
            [[fallthrough]];
        case FN_STAT_PAGE:
        {
            if (SwView::GetMoveType() == NID_PGE)
            {
                SwView *pView = GetCreateView();
                if (pView)
                {
                    SwWrtShell& rSh = pView->GetWrtShell();
                    // GetPageNum - return current page number:
                    // true: in which cursor is located.
                    // false: which is visible at the upper margin.
                    sal_uInt16 nPhyNum, nVirtNum;
                    rSh.GetPageNum(nPhyNum, nVirtNum, false);
                    m_xGotoPageSpinButton->set_text(OUString::number(nPhyNum));
                }
            }
        }
    }
}

void SwNavigationPI::UpdateInitShow()
{
    // if the parent isn't a float, then the navigator is displayed in
    // the sidebar or is otherwise docked. While the navigator could change
    // its size, the sidebar can not, and the navigator would just waste
    // space. Therefore disable this button.
    bool bParentIsFloatingWindow(ParentIsFloatingWindow(m_xNavigatorDlg));
    m_xContent6ToolBox->set_item_sensitive("listbox", bParentIsFloatingWindow);
    // show content if docked
    if (!bParentIsFloatingWindow && IsZoomedIn())
        ZoomOut();
    if (!IsZoomedIn())
        FillBox();
}

IMPL_LINK_NOARG(SwNavigationPI, SetFocusChildHdl, weld::Container&, void)
{
    // update documents listbox
    UpdateListBox();
}

// Notification on modified DocInfo
void SwNavigationPI::Notify( SfxBroadcaster& rBrdc, const SfxHint& rHint )
{
    if(&rBrdc == m_pCreateView)
    {
        if (rHint.GetId() == SfxHintId::Dying)
        {
            EndListening(*m_pCreateView);
            m_pCreateView = nullptr;
            m_xContentTree->SetActiveShell(nullptr);
        }
    }
    else
    {
        if (rHint.GetId() == SfxHintId::ThisIsAnSfxEventHint)
        {
            SfxEventHintId eEventId = static_cast<const SfxEventHint&>(rHint).GetEventId();
            if (eEventId == SfxEventHintId::OpenDoc)
            {
                SwView *pActView = GetCreateView();
                if(pActView)
                {
                    SwWrtShell* pWrtShell = pActView->GetWrtShellPtr();
                    m_xContentTree->SetActiveShell(pWrtShell);
                    if (m_xGlobalTree->get_visible())
                    {
                        bool bUpdateAll = m_xGlobalTree->Update(false);
                        // If no update is needed, then update the font colors
                        // at the entries of broken links.
                        m_xGlobalTree->Display(!bUpdateAll);
                    }
                }
            }
        }
    }
}

IMPL_LINK( SwNavigationPI, HeadingsMenuSelectHdl, const OUString&, rMenuId, void )
{
    if (!rMenuId.isEmpty())
        m_xContentTree->SetOutlineLevel(rMenuId.toUInt32());
}

void SwNavigationPI::UpdateListBox()
{
    if (!m_xDocListBox) // disposed
        return;

    m_xDocListBox->freeze();
    m_xDocListBox->clear();
    SwView *pActView = GetCreateView();
    bool bDisable = pActView == nullptr;
    SwView *pView = SwModule::GetFirstView();
    sal_Int32 nCount = 0;
    sal_Int32 nAct = 0;
    sal_Int32 nConstPos = 0;
    const SwView* pConstView = m_xContentTree->IsConstantView() &&
                                m_xContentTree->GetActiveWrtShell() ?
                                    &m_xContentTree->GetActiveWrtShell()->GetView():
                                        nullptr;
    while (pView)
    {
        SfxObjectShell* pDoc = pView->GetDocShell();
        // #i53333# don't show help pages here
        if ( !pDoc->IsHelpDocument() )
        {
            OUString sEntry = pDoc->GetTitle() + " (";
            if (pView == pActView)
            {
                nAct = nCount;
                sEntry += SwResId(STR_ACTIVE);
            }
            else
                sEntry += SwResId(STR_INACTIVE);
            sEntry += ")";
            m_xDocListBox->append_text(sEntry);

            if (pConstView && pView == pConstView)
                nConstPos = nCount;

            nCount++;
        }
        pView = SwModule::GetNextView(pView);
    }
    m_xDocListBox->append_text(SwResId(STR_ACTIVE_VIEW)); // "Active Window"
    nCount++;

    if (SwWrtShell* pHiddenWrtShell = m_xContentTree->GetHiddenWrtShell())
    {
        OUString sEntry = pHiddenWrtShell->GetView().GetDocShell()->GetTitle() +
                " (" + SwResId(STR_HIDDEN) + ")";
        m_xDocListBox->append_text(sEntry);
        bDisable = false;
    }

    m_xDocListBox->thaw();

    if(m_xContentTree->IsActiveView())
    {
        //Either the name of the current Document or "Active Document".
        m_xDocListBox->set_active(pActView ? nAct : --nCount);
    }
    else if(m_xContentTree->IsHiddenView())
    {
        m_xDocListBox->set_active(nCount);
    }
    else
        m_xDocListBox->set_active(nConstPos);

    m_xDocListBox->set_sensitive(!bDisable);
}

IMPL_LINK(SwNavigationPI, DoneLink, SfxPoolItem const *, pItem, void)
{
    const SfxViewFrameItem* pFrameItem = dynamic_cast<SfxViewFrameItem const *>( pItem  );
    if( !pFrameItem )
        return;

    SfxViewFrame* pFrame =  pFrameItem->GetFrame();
    if(pFrame)
    {
        m_xContentTree->clear();
        m_pContentView = dynamic_cast<SwView*>( pFrame->GetViewShell() );
        OSL_ENSURE(m_pContentView, "no SwView");
        if(m_pContentView)
            m_pContentWrtShell = m_pContentView->GetWrtShellPtr();
        else
            m_pContentWrtShell = nullptr;
        m_oObjectShell.emplace( pFrame->GetObjectShell() );
        FillBox();
    }
}

OUString SwNavigationPI::CreateDropFileName( const TransferableDataHelper& rData )
{
    OUString sFileName;
    SotClipboardFormatId nFormat;
    if( rData.HasFormat( nFormat = SotClipboardFormatId::FILE_LIST ))
    {
        FileList aFileList;
        rData.GetFileList( nFormat, aFileList );
        sFileName = aFileList.GetFile( 0 );
    }
    else if( rData.HasFormat( nFormat = SotClipboardFormatId::STRING ) ||
              rData.HasFormat( nFormat = SotClipboardFormatId::SIMPLE_FILE ) ||
             rData.HasFormat( nFormat = SotClipboardFormatId::FILENAME ))
    {
        (void)rData.GetString(nFormat, sFileName);
    }
    else if( rData.HasFormat( nFormat = SotClipboardFormatId::SOLK ) ||
                rData.HasFormat( nFormat = SotClipboardFormatId::NETSCAPE_BOOKMARK )||
                rData.HasFormat( nFormat = SotClipboardFormatId::FILECONTENT ) ||
                rData.HasFormat( nFormat = SotClipboardFormatId::FILEGRPDESCRIPTOR ) ||
                rData.HasFormat( nFormat = SotClipboardFormatId::UNIFORMRESOURCELOCATOR ))
    {
        INetBookmark aBkmk { OUString(), OUString() };
        if (rData.GetINetBookmark(nFormat, aBkmk))
            sFileName = aBkmk.GetURL();
    }
    if( !sFileName.isEmpty() )
    {
        sFileName = INetURLObject( sFileName ).GetMainURL( INetURLObject::DecodeMechanism::NONE );
    }
    return sFileName;
}

sal_Int8 SwNavigationPI::AcceptDrop()
{
    return ( !m_xContentTree->IsInDrag() &&
        ( m_xContentTree->IsDropFormatSupported( SotClipboardFormatId::SIMPLE_FILE ) ||
          m_xContentTree->IsDropFormatSupported( SotClipboardFormatId::STRING ) ||
          m_xContentTree->IsDropFormatSupported( SotClipboardFormatId::SOLK ) ||
           m_xContentTree->IsDropFormatSupported( SotClipboardFormatId::NETSCAPE_BOOKMARK )||
           m_xContentTree->IsDropFormatSupported( SotClipboardFormatId::FILECONTENT ) ||
           m_xContentTree->IsDropFormatSupported( SotClipboardFormatId::FILEGRPDESCRIPTOR ) ||
           m_xContentTree->IsDropFormatSupported( SotClipboardFormatId::UNIFORMRESOURCELOCATOR ) ||
           m_xContentTree->IsDropFormatSupported( SotClipboardFormatId::FILENAME )))
        ? DND_ACTION_COPY
        : DND_ACTION_NONE;
}

sal_Int8 SwNavigationPI::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    TransferableDataHelper aData( rEvt.maDropEvent.Transferable );
    sal_Int8 nRet = DND_ACTION_NONE;
    if (m_xContentTree->IsInDrag())
        return nRet;

    OUString sFileName = SwNavigationPI::CreateDropFileName(aData);
    if (sFileName.isEmpty())
        return nRet;

    INetURLObject aTemp(sFileName);
    GraphicDescriptor aDesc(aTemp);
    if (aDesc.Detect())   // accept no graphics
        return nRet;

    if (-1 != sFileName.indexOf('#'))
        return nRet;

    if (m_sContentFileName.isEmpty() || m_sContentFileName != sFileName)
    {
        nRet = rEvt.mnAction;
        sFileName = comphelper::string::stripEnd(sFileName, 0);
        m_sContentFileName = sFileName;
        if(m_oObjectShell)
        {
            m_xContentTree->SetHiddenShell( nullptr );
            (*m_oObjectShell)->DoClose();
            m_oObjectShell.reset();
        }
        SfxStringItem aFileItem(SID_FILE_NAME, sFileName );
        SfxStringItem aOptionsItem( SID_OPTIONS, "HRC" );
        SfxLinkItem aLink( SID_DONELINK,
                            LINK( this, SwNavigationPI, DoneLink ) );
        if (SwView* pView = GetActiveView())
            pView->GetViewFrame().GetDispatcher()->ExecuteList(
                        SID_OPENDOC, SfxCallMode::ASYNCHRON,
                        { &aFileItem, &aOptionsItem, &aLink });
    }
    return nRet;
}

// toggle between showing the global tree or the content tree
void SwNavigationPI::ToggleTree()
{
    if (comphelper::LibreOfficeKit::isActive())
    {
        m_xGlobalTree->HideTree();
        return;
    }

    bool bGlobalDoc = IsGlobalDoc();
    if (!IsGlobalMode() && bGlobalDoc)
    {
        // toggle to global mode
        if (IsZoomedIn())
            ZoomOut();
        m_xGlobalBox->show();
        m_xGlobalTree->ShowTree();
        m_xGlobalToolBox->show();
        m_xContentBox->hide();
        m_xContentTree->HideTree();
        m_xContent1ToolBox->hide();
        m_xContent2ToolBox->hide();
        m_xContent3ToolBox->hide();
        m_xContent4ToolBox->hide();
        m_xContent5ToolBox->hide();
        m_xContent6ToolBox->hide();
        m_xDocListBox->hide();
        SetGlobalMode(true);
    }
    else
    {
        m_xGlobalBox->hide();
        m_xGlobalTree->HideTree();
        m_xGlobalToolBox->hide();
        SetGlobalMode(false);
        if (!IsZoomedIn())
        {
            m_xContentBox->show();
            m_xContentTree->ShowTree();
            m_xContent1ToolBox->show();
            m_xContent2ToolBox->show();
            SetContent3And4ToolBoxVisibility();
            m_xContent5ToolBox->show();
            m_xContent6ToolBox->show();
            m_xDocListBox->show();
        }
    }
}

bool SwNavigationPI::IsGlobalDoc() const
{
    bool bRet = false;
    SwView *pView = GetCreateView();
    if (pView)
    {
        SwWrtShell &rSh = pView->GetWrtShell();
        bRet = rSh.IsGlobalDoc();
    }
    return bRet;
}

void SwNavigationPI::SelectNavigateByContentType(const OUString& rContentTypeName)
{
    if (!m_pNavigateByComboBox)
        return;
    if (auto nPos = m_pNavigateByComboBox->find_text(rContentTypeName); nPos != -1)
    {
        m_pNavigateByComboBox->set_active(nPos);
        UpdateNavigateBy();
    }
}

SwView*  SwNavigationPI::GetCreateView() const
{
    if (!m_pCreateView)
    {
        SwView* pView = SwModule::GetFirstView();
        while (pView)
        {
            if(&pView->GetViewFrame().GetBindings() == &m_rBindings)
            {
                const_cast<SwNavigationPI*>(this)->m_pCreateView = pView;
                const_cast<SwNavigationPI*>(this)->StartListening(*m_pCreateView);
                break;
            }
            pView = SwModule::GetNextView(pView);
        }
    }
    return m_pCreateView;
}

class SwNavigatorWin : public SfxNavigator
{
private:
    std::unique_ptr<SwNavigationPI> m_xNavi;
public:
    SwNavigatorWin(SfxBindings* _pBindings, SfxChildWindow* _pMgr,
                   vcl::Window* pParent, SfxChildWinInfo* pInfo);
    virtual void StateChanged(StateChangedType nStateChange) override;
    virtual void dispose() override
    {
        m_xNavi.reset();
        SfxNavigator::dispose();
    }
    virtual ~SwNavigatorWin() override
    {
        disposeOnce();
    }
};

SwNavigatorWin::SwNavigatorWin(SfxBindings* _pBindings, SfxChildWindow* _pMgr,
                               vcl::Window* pParent, SfxChildWinInfo* pInfo)
    : SfxNavigator(_pBindings, _pMgr, pParent, pInfo)
    , m_xNavi(std::make_unique<SwNavigationPI>(m_xContainer.get(), _pBindings->GetActiveFrame(), _pBindings, this))
{
    _pBindings->Invalidate(SID_NAVIGATOR);

    SwNavigationConfig* pNaviConfig = SW_MOD()->GetNavigationConfig();

    SetMinOutputSizePixel(GetOptimalSize());
    if (pNaviConfig->IsSmall())
        m_xNavi->ZoomIn();
}

void SwNavigatorWin::StateChanged(StateChangedType nStateChange)
{
    SfxNavigator::StateChanged(nStateChange);
    if (nStateChange == StateChangedType::InitShow)
        m_xNavi->UpdateInitShow();
}

SFX_IMPL_DOCKINGWINDOW(SwNavigatorWrapper, SID_NAVIGATOR);

SwNavigatorWrapper::SwNavigatorWrapper(vcl::Window *_pParent, sal_uInt16 nId,
                                       SfxBindings* pBindings, SfxChildWinInfo* pInfo)
    : SfxNavigatorWrapper(_pParent, nId)
{
    SetWindow(VclPtr<SwNavigatorWin>::Create(pBindings, this, _pParent, pInfo));
    Initialize();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
