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
#include <sfx2/navigat.hxx>
#include <sfx2/viewfrm.hxx>
#include <tools/urlobj.hxx>
#include <vcl/toolbox.hxx>
#include <swtypes.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <navicfg.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <actctrl.hxx>
#include <navipi.hxx>
#include <workctrl.hxx>
#include <edtwin.hxx>
#include <sfx2/app.hxx>
#include <cmdid.h>
#include <helpids.h>

#include <strings.hrc>
#include <bitmaps.hlst>

#include <memory>

#include <uiobject.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;

//! soon obsolete !
std::unique_ptr<SfxChildWindowContext> SwNavigationChild::CreateImpl( vcl::Window *pParent,
        SfxBindings *pBindings, SfxChildWinInfo* /*pInfo*/ )
{
    return std::make_unique<SwNavigationChild>(pParent,
            /* cast is safe here! */static_cast< sal_uInt16 >(SwView::GetInterfaceId()),
            pBindings);
}
void    SwNavigationChild::RegisterChildWindowContext(SfxModule* pMod)
{
    auto pFact = std::make_unique<SfxChildWinContextFactory>(
       SwNavigationChild::CreateImpl,
       /* cast is safe here! */static_cast< sal_uInt16 >(SwView::GetInterfaceId()) );
    SfxChildWindowContext::RegisterChildWindowContext(pMod, SID_NAVIGATOR, std::move(pFact));
}


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

void SwNavigationPI::MoveOutline(SwOutlineNodes::size_type nSource, SwOutlineNodes::size_type nTarget,
                                                    bool bWithChildren)
{
    SwView *pView = GetCreateView();
    SwWrtShell &rSh = pView->GetWrtShell();
    if(nTarget < nSource || nTarget == SwOutlineNodes::npos)
        nTarget ++;
    if ( rSh.IsOutlineMovable( nSource ))
    {

        SwOutlineNodes::difference_type nMove = nTarget-nSource; //( nDir<0 ) ? 1 : 0 ;
        rSh.GotoOutline(nSource);
        if (bWithChildren)
            rSh.MakeOutlineSel(nSource, nSource, true);
        // While moving, the selected children does not counting.
        const SwOutlineNodes::size_type nLastOutlinePos = rSh.GetOutlinePos(MAXLEVEL);
        if(bWithChildren && nMove > 1 &&
                nLastOutlinePos < nTarget)
        {
            if(!rSh.IsCursorPtAtEnd())
                rSh.SwapPam();
            nMove -= nLastOutlinePos - nSource;
        }
        if(!bWithChildren || nMove < 1 || nLastOutlinePos < nTarget )
            rSh.MoveOutlinePara( nMove );
        rSh.ClearMark();
        rSh.GotoOutline( nSource + nMove);
    }

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
    }
}

void SwNavigationPI::UsePage()
{
    SwView *pView = GetCreateView();
    SwWrtShell *pSh = pView ? &pView->GetWrtShell() : nullptr;
    GetPageEdit().set_value(1);
    if (pSh)
    {
        const sal_uInt16 nPageCnt = pSh->GetPageCnt();
        sal_uInt16 nPhyPage, nVirPage;
        pSh->GetPageNum(nPhyPage, nVirPage);

        GetPageEdit().set_max(nPageCnt);
        GetPageEdit().set_value(nPhyPage);
    }
}

// Select handler of the toolboxes
IMPL_LINK(SwNavigationPI, ToolBoxSelectHdl, const OString&, rCommand, void)
{
    SwView *pView = GetCreateView();
    if (!pView)
        return;
    SwWrtShell &rSh = pView->GetWrtShell();
    // Get MouseModifier for Outline-Move

    // Standard: sublevels are taken
    // do not take sublevels with Ctrl
    bool bOutlineWithChildren = true; //TODO (KEY_MOD1 != pBox->GetModifier());
    int nFuncId = 0;
    bool bFocusToDoc = false;
    if (rCommand == ".uno:ScrollToPrevious")
    {
        rSh.GetView().GetViewFrame()->GetDispatcher()->Execute(FN_SCROLL_PREV, SfxCallMode::ASYNCHRON);
    }
    else if (rCommand == ".uno:ScrollToNext")
    {
        rSh.GetView().GetViewFrame()->GetDispatcher()->Execute(FN_SCROLL_NEXT, SfxCallMode::ASYNCHRON);
    }
    else if (rCommand == "root")
    {
        m_xContentTree->ToggleToRoot();
    }
    else if (rCommand == "listbox")
    {
        if (SfxChildWindowContext::GetFloatingWindow(GetParent()))
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
        rSh.GetView().GetViewFrame()->GetDispatcher()->Execute(FN_SET_REMINDER, SfxCallMode::ASYNCHRON);
    }
    else if (rCommand == "chapterdown" ||
             rCommand == "movedown" ||
             rCommand == "chapterup" ||
             rCommand == "moveup" ||
             rCommand == "promote" ||
             rCommand == "demote" ||
             rCommand == "edit")
    {
#if 0
        if (IsGlobalMode())
            m_aGlobalTree->ExecCommand(rCommand);
        else
            m_xContentTree->ExecCommand(rCommand, bOutlineWithChildren);
#endif
    }
    else if (rCommand == "contenttoggle" || rCommand == "globaltoggle")
    {
        ToggleTree();
        m_pConfig->SetGlobalActive(IsGlobalMode());
    }
    else if (rCommand == "save")
    {
        bool bSave = rSh.IsGlblDocSaveLinks();
        rSh.SetGlblDocSaveLinks( !bSave );
//TODO        m_aGlobalTree->set_item_active(rCommand, !bSave);
    }

    if (nFuncId)
        lcl_UnSelectFrame(&rSh);
    if (bFocusToDoc)
        pView->GetEditWin().GrabFocus();
}

#if 0
// Click handler of the toolboxes
IMPL_LINK( SwNavigationPI, ToolBoxClickHdl, ToolBox *, pBox, void )
{
    const sal_uInt16 nCurrItemId = pBox->GetCurItemId();
    const OUString rCommand = pBox->GetItemCommand(nCurrItemId);
    if (rCommand == "update" || rCommand == "insert")
        m_aGlobalTree->TbxMenuHdl(nCurrItemId, pBox);
}

IMPL_LINK( SwNavigationPI, ToolBoxDropdownClickHdl, ToolBox*, pBox, void )
{
    const sal_uInt16 nCurrItemId = pBox->GetCurItemId();
    const OUString sCommand = pBox->GetItemCommand(nCurrItemId);
    if (sCommand == "dragmode")
    {
        static const char* aHIDs[] =
        {
            HID_NAVI_DRAG_HYP,
            HID_NAVI_DRAG_LINK,
            HID_NAVI_DRAG_COPY,
        };
        ScopedVclPtrInstance<PopupMenu> pMenu;
        for (sal_uInt16 i = 0; i <= static_cast<sal_uInt16>(RegionMode::EMBEDDED); ++i)
        {
            pMenu->InsertItem(i + 1, m_aContextArr[i], MenuItemBits::RADIOCHECK);
            pMenu->set_help_id(i + 1, aHIDs[i]);
        }
        pMenu->CheckItem( static_cast<int>(m_nRegionMode) + 1 );
        pMenu->SetSelectHdl(LINK(this, SwNavigationPI, MenuSelectHdl));
        pBox->SetItemDown(nCurrItemId, true);
        pMenu->Execute(pBox, pBox->GetItemRect(nCurrItemId), PopupMenuFlags::ExecuteDown);
        pBox->SetItemDown(nCurrItemId, false);
        pBox->EndSelection();
        pMenu.disposeAndClear();
        pBox->Invalidate();
    }
    else if (sCommand == "headings")
    {
        ScopedVclPtrInstance<PopupMenu> pMenu;
        for (sal_uInt16 i = 101; i <= 100 + MAXLEVEL; ++i)
        {
            pMenu->InsertItem(i, OUString::number(i - 100), MenuItemBits::RADIOCHECK);
            pMenu->set_help_id(i, HID_NAVI_OUTLINES);
        }
        pMenu->CheckItem( m_xContentTree->GetOutlineLevel() + 100 );
        pMenu->SetSelectHdl(LINK(this, SwNavigationPI, MenuSelectHdl));
        pBox->SetItemDown( nCurrItemId, true );
        pMenu->Execute(pBox, pBox->GetItemRect(nCurrItemId), PopupMenuFlags::ExecuteDown);
        pBox->SetItemDown( nCurrItemId, false );
        pMenu.disposeAndClear();
        pBox->EndSelection();
        pBox->Invalidate();
    }
}
#endif

FactoryFunction SwNavigationPI::GetUITestFactory() const
{
    return SwNavigationPIUIObject::create;
}

// Action-Handler Edit:
// Switches to the page if the structure view is not turned on.
void SwNavigationPI::EditAction()
{
    SwView *pView = GetCreateView();
    if (pView)
    {
        if(m_aPageChgIdle.IsActive())
            m_aPageChgIdle.Stop();
        m_pCreateView->GetWrtShell().GotoPage(GetPageEdit().get_value(), true);
        m_pCreateView->GetEditWin().GrabFocus();
        m_pCreateView->GetViewFrame()->GetBindings().Invalidate(FN_STAT_PAGE);
    }
}

void SwNavigationPI::ZoomOut()
{
    if (!IsZoomedIn())
        return;
    SfxNavigator* pNav = dynamic_cast<SfxNavigator*>(GetParent());
    if (!pNav)
        return;
    m_bIsZoomedIn = false;
    FillBox();
#if 0
    if (IsGlobalMode())
    {
        m_xGlobalBox->show();
        m_aGlobalTree->ShowTree();
    }
    else
#endif
    {
        m_xContentBox->show();
        m_xContentTree->ShowTree();
        m_xDocListBox->show();
    }

    Size aOptimalSize(GetOptimalSize());
    Size aNewSize(pNav->GetOutputSizePixel());
    aNewSize.setHeight( m_aExpandedSize.Height() );
    pNav->SetMinOutputSizePixel(aOptimalSize);
    pNav->SetOutputSizePixel(aNewSize);

#if 0
    SvTreeListEntry* pFirst = m_xContentTree->FirstSelected();
    if (pFirst)
        m_xContentTree->Select(pFirst); // Enable toolbox
#endif
    m_pConfig->SetSmall(false);
    m_xContent2ToolBox->set_item_active("listbox", true);
}

void SwNavigationPI::ZoomIn()
{
    if (IsZoomedIn())
        return;
    SfxNavigator* pNav = dynamic_cast<SfxNavigator*>(GetParent());
    if (!pNav)
        return;

    m_aExpandedSize = GetSizePixel();

    m_xContentBox->hide();
    m_xContentTree->HideTree();
    m_xGlobalBox->hide();
#if 0
    m_aGlobalTree->HideTree();
#endif
    m_xDocListBox->hide();
    m_bIsZoomedIn = true;

    Size aOptimalSize(GetOptimalSize());
    Size aNewSize(pNav->GetOutputSizePixel());
    aNewSize.setHeight( aOptimalSize.Height() );
    pNav->SetMinOutputSizePixel(aOptimalSize);
    pNav->SetOutputSizePixel(aNewSize);

#if 0
    SvTreeListEntry* pFirst = m_xContentTree->FirstSelected();
    if (pFirst)
        m_xContentTree->Select(pFirst); // Enable toolbox
#endif

    m_pConfig->SetSmall(true);
    m_xContent2ToolBox->set_item_active("listbox", false);
}

namespace {

enum StatusIndex
{
    IDX_STR_HIDDEN = 0,
    IDX_STR_ACTIVE = 1,
    IDX_STR_INACTIVE = 2
};

}

VclPtr<vcl::Window> SwNavigationPI::Create(vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
{
    if( pParent == nullptr )
        throw css::lang::IllegalArgumentException("no parent window given to SwNavigationPI::Create", nullptr, 0);
    if( !rxFrame.is() )
        throw css::lang::IllegalArgumentException("no XFrame given to SwNavigationPI::Create", nullptr, 0);
    if( pBindings == nullptr )
        throw css::lang::IllegalArgumentException("no SfxBindings given to SwNavigationPI::Create", nullptr, 0);
    return VclPtr<SwNavigationPI>::Create(pParent, rxFrame, pBindings);
}

SwNavigationPI::SwNavigationPI(vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* _pBindings)
    : PanelLayout(pParent, "NavigatorPanel", "modules/swriter/ui/navigatorpanel.ui", rxFrame, true)
    , m_aDocFullName(SID_DOCFULLNAME, *_pBindings, *this)
    , m_aPageStats(FN_STAT_PAGE, *_pBindings, *this)
    , m_pContentView(nullptr)
    , m_pContentWrtShell(nullptr)
    , m_pActContView(nullptr)
    , m_pCreateView(nullptr)
    , m_pConfig(SW_MOD()->GetNavigationConfig())
    , m_rBindings(*_pBindings)
    , m_nRegionMode(RegionMode::NONE)
    , m_bIsZoomedIn(false)
    , m_bGlobalMode(false)
    , m_xContent1ToolBox(m_xBuilder->weld_toolbar("content1"))
    , m_xContent2ToolBox(m_xBuilder->weld_toolbar("content2"))
    , m_xContent3ToolBox(m_xBuilder->weld_toolbar("content3"))
    , m_xContent1Dispatch(new ToolbarUnoDispatcher(*m_xContent1ToolBox, rxFrame))
    , m_xGlobalToolBox(m_xBuilder->weld_toolbar("global"))
    , m_xEdit(m_xBuilder->weld_spin_button("spinbutton"))
    , m_xContentBox(m_xBuilder->weld_widget("contentbox"))
    , m_xContentTree(new SwContentTree(m_xBuilder->weld_tree_view("contenttree"), this))
    , m_xGlobalBox(m_xBuilder->weld_widget("globalbox"))
//TODO    , m_xGlobalTree(new SwGlobalTree(m_xBuilder->weld_tree_view("global"), this))
    , m_xDocListBox(m_xBuilder->weld_combo_box("documents"))
{
    GetCreateView();

    m_xContent1ToolBox->set_help_id(HID_NAVIGATOR_TOOLBOX);
    m_xContent2ToolBox->set_help_id(HID_NAVIGATOR_TOOLBOX);
    m_xContent3ToolBox->set_help_id(HID_NAVIGATOR_TOOLBOX);
    m_xGlobalToolBox->set_help_id(HID_NAVIGATOR_GLOBAL_TOOLBOX);
    m_xDocListBox->set_help_id(HID_NAVIGATOR_LISTBOX);
    m_xDocListBox->set_size_request(42, -1); // set a nominal width so it takes width of surroundings

    // Insert the numeric field in the toolbox.
//TODO    m_xEdit = VclPtr<NumEditAction>::Create(m_xContentToolBox.get());
    m_xEdit->set_accessible_name(m_xEdit->get_tooltip_text());
    m_xEdit->connect_value_changed(LINK(this, SwNavigationPI, PageEditModifyHdl));

#if 0
    // Double separators are not allowed, so you have to
    // determine the suitable size differently.
    tools::Rectangle aFirstRect = m_xContentToolBox->GetItemRect(m_xContentToolBox->GetItemId("anchor"));
    tools::Rectangle aSecondRect = m_xContentToolBox->GetItemRect(m_xContentToolBox->GetItemId("header"));
    Size aItemWinSize( aFirstRect.Left() - aSecondRect.Left(),
                       aFirstRect.Bottom() - aFirstRect.Top() );
    Size aOptimalSize(m_xEdit->GetSizePixel());
    aItemWinSize.setWidth( std::max(aItemWinSize.Width(), aOptimalSize.Width()) );
    aItemWinSize.setHeight( std::max(aItemWinSize.Height(), aOptimalSize.Height()) );
    m_xEdit->SetSizePixel(aItemWinSize);
    m_xContentToolBox->InsertSeparator(4);
    m_xContentToolBox->InsertWindow( FN_PAGENUMBER, m_xEdit, ToolBoxItemBits::NONE, 4);
    m_xContentToolBox->InsertSeparator(4);
    m_xContentToolBox->set_help_id(FN_PAGENUMBER, "modules/swriter/ui/navigatorpanel/numericfield");
    m_xContentToolBox->ShowItem(FN_PAGENUMBER);
#endif
    if (!IsGlobalDoc())
    {
        m_xContent3ToolBox->set_item_sensitive("contenttoggle", false);
    }

    const char* REGIONNAME_ARY[] =
    {
        STR_HYPERLINK,
        STR_LINK_REGION,
        STR_COPY_REGION
    };

    const char* REGIONMODE_ARY[] =
    {
        STR_HIDDEN,
        STR_ACTIVE,
        STR_INACTIVE
    };

    static_assert(SAL_N_ELEMENTS(REGIONNAME_ARY) == SAL_N_ELEMENTS(REGIONMODE_ARY), "### unexpected size!");
    static_assert(SAL_N_ELEMENTS(REGIONNAME_ARY) == static_cast<sal_uInt16>(RegionMode::EMBEDDED) + 1, "### unexpected size!");

    for (sal_uInt16 i = 0; i <= static_cast<sal_uInt16>(RegionMode::EMBEDDED); ++i)
    {
        m_aContextArr[i] = SwResId(REGIONNAME_ARY[i]);
        m_aStatusArr[i] = SwResId(REGIONMODE_ARY[i]);
    }

    m_aStatusArr[3] = SwResId(STR_ACTIVE_VIEW);

#if 0
    m_xContentTree->SetStyle(m_xContentTree->GetStyle()|WB_HASBUTTONS|WB_HASBUTTONSATROOT|
                             WB_CLIPCHILDREN|WB_HSCROLL|WB_TABSTOP);
#endif
//TODO    m_xContentTree->SetForceMakeVisible(true);
    m_xContentTree->set_selection_mode(SelectionMode::Single);
#if 0
    m_xContentTree->SetDragDropMode(DragDropMode::CTRL_MOVE |
                                    DragDropMode::CTRL_COPY |
                                    DragDropMode::ENABLE_TOP);
    m_xContentTree->EnableAsyncDrag(true);
#endif
    m_xContentTree->ShowTree();
    m_xContent2ToolBox->set_item_active("listbox", true);
    m_xContent2ToolBox->set_item_visible("listbox", SfxChildWindowContext::GetFloatingWindow(GetParent()) != nullptr);

#if 0
//  TreeListBox for global document
    m_aGlobalTree->SetSelectionMode( SelectionMode::Multiple );
    m_aGlobalTree->SetStyle(m_aGlobalTree->GetStyle()|WB_HASBUTTONS|WB_HASBUTTONSATROOT|
                            WB_CLIPCHILDREN|WB_HSCROLL|WB_TABSTOP);
#endif

//  Handler
    Link<const OString&, void> aLk = LINK(this, SwNavigationPI, ToolBoxSelectHdl);
    m_xContent1ToolBox->connect_clicked(aLk);
    m_xContent2ToolBox->connect_clicked(aLk);
    m_xContent3ToolBox->connect_clicked(aLk);
    m_xGlobalToolBox->connect_clicked(aLk);
    m_xDocListBox->connect_changed(LINK(this, SwNavigationPI, DocListBoxSelectHdl));
#if 0
    m_xContentToolBox->SetClickHdl( LINK(this, SwNavigationPI, ToolBoxClickHdl) );
    m_xContentToolBox->SetDropdownClickHdl( LINK(this, SwNavigationPI, ToolBoxDropdownClickHdl) );
    m_xGlobalToolBox->SetClickHdl( LINK(this, SwNavigationPI, ToolBoxClickHdl) );
    m_xGlobalToolBox->SetDropdownClickHdl( LINK(this, SwNavigationPI, ToolBoxDropdownClickHdl) );
#endif
    m_xGlobalToolBox->set_item_active("globaltoggle", true);

    StartListening(*SfxGetpApp());

#if 0
    if(IsGlobalDoc())
    {
        SwView *pActView = GetCreateView();
        m_xGlobalToolBox->set_item_active("save",
                    pActView->GetWrtShellPtr()->IsGlblDocSaveLinks());
        if (m_pConfig->IsGlobalActive())
            ToggleTree();
        m_aGlobalTree->GrabFocus();
    }
    else
#endif
        m_xContentTree->grab_focus();
    UsePage();
    m_aPageChgIdle.SetInvokeHandler(LINK(this, SwNavigationPI, ChangePageHdl));
    m_aPageChgIdle.SetPriority(TaskPriority::LOWEST);

    m_xContentTree->set_accessible_name(SwResId(STR_ACCESS_TL_CONTENT));
//TODO    m_aGlobalTree->SetAccessibleName(SwResId(STR_ACCESS_TL_GLOBAL));
    m_xDocListBox->set_accessible_name(m_aStatusArr[3]);

    m_aExpandedSize = GetOptimalSize();
}

SwNavigationPI::~SwNavigationPI()
{
    disposeOnce();
}

void SwNavigationPI::dispose()
{
    if (IsGlobalDoc() && !IsGlobalMode())
    {
        SwView *pView = GetCreateView();
        SwWrtShell &rSh = pView->GetWrtShell();
        if (!rSh.IsAllProtect())
            pView->GetDocShell()->SetReadOnlyUI(false);
    }

    EndListening(*SfxGetpApp());

#if 0
    m_xContentToolBox->GetItemWindow(FN_PAGENUMBER)->disposeOnce();
    m_xContentToolBox->Clear();
#endif

    if (m_pxObjectShell)
    {
        if (m_pxObjectShell->Is())
            (*m_pxObjectShell)->DoClose();
        m_pxObjectShell.reset();
    }

    m_xDocListBox.reset();
//TODO    m_xGlobalTree.reset();
    m_xGlobalBox.reset();
    m_xContentTree.reset();
    m_xContentBox.reset();
    m_xGlobalToolBox.reset();
    m_xEdit.reset();
    m_xContent1Dispatch.reset();
    m_xContent1ToolBox.reset();
    m_xContent2ToolBox.reset();
    m_xContent3ToolBox.reset();

    m_aPageChgIdle.Stop();

    m_aDocFullName.dispose();
    m_aPageStats.dispose();

    PanelLayout::dispose();
}

void SwNavigationPI::NotifyItemUpdate(sal_uInt16 nSID, SfxItemState /*eState*/,
                                      const SfxPoolItem* /*pState*/)
{
    if (nSID == SID_DOCFULLNAME)
    {
        SwView *pActView = GetCreateView();
        if(pActView)
        {
            SwWrtShell* pWrtShell = pActView->GetWrtShellPtr();
            m_xContentTree->SetActiveShell(pWrtShell);
            bool bGlobal = IsGlobalDoc();
            m_xContent3ToolBox->set_item_sensitive("contenttoggle", bGlobal);
            if ((!bGlobal && IsGlobalMode()) || (!IsGlobalMode() && m_pConfig->IsGlobalActive()))
            {
                ToggleTree();
            }
            if (bGlobal)
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
    else if (nSID == FN_STAT_PAGE)
    {
        SwView *pActView = GetCreateView();
        if(pActView)
        {
            SwWrtShell &rSh = pActView->GetWrtShell();
            GetPageEdit().set_max(rSh.GetPageCnt());
        }
    }
}

void SwNavigationPI::StateChanged(StateChangedType nStateChange)
{
    PanelLayout::StateChanged(nStateChange);
    if (nStateChange == StateChangedType::InitShow)
    {
        // if the parent isn't a float, then the navigator is displayed in
        // the sidebar or is otherwise docked. While the navigator could change
        // its size, the sidebar can not, and the navigator would just waste
        // space. Therefore hide this button.
        m_xContent2ToolBox->set_item_visible("listbox", SfxChildWindowContext::GetFloatingWindow(GetParent()) != nullptr);
        // show content if docked
        if (SfxChildWindowContext::GetFloatingWindow(GetParent()) == nullptr && IsZoomedIn())
            ZoomOut();
    }
    else if (nStateChange == StateChangedType::ControlFocus)
    {
        if (m_xContentTree)
        {
            // update documents listbox
            UpdateListBox();
        }
    }
}

// Get the numeric field from the toolbox.
weld::SpinButton& SwNavigationPI::GetPageEdit()
{
//TODO    return *static_cast<NumEditAction*>(m_xContentToolBox->GetItemWindow(FN_PAGENUMBER));
    return *m_xEdit;
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
        }
    }
    else
    {
        if (const SfxEventHint* pHint = dynamic_cast<const SfxEventHint*>(&rHint))
        {
            SfxEventHintId eEventId = pHint->GetEventId();
            if (eEventId == SfxEventHintId::OpenDoc)
            {
                SwView *pActView = GetCreateView();
                if(pActView)
                {
                    SwWrtShell* pWrtShell = pActView->GetWrtShellPtr();
                    m_xContentTree->SetActiveShell(pWrtShell);
#if 0
                    if(m_aGlobalTree->IsVisible())
                    {
                        if(m_aGlobalTree->Update( false ))
                            m_aGlobalTree->Display();
                        else
                        // If no update is needed, then paint at least,
                        // because of the red entries for the broken links.
                            m_aGlobalTree->Invalidate();
                    }
#endif
                }
            }
        }
    }
}

IMPL_LINK( SwNavigationPI, MenuSelectHdl, Menu *, pMenu, bool )
{
    sal_uInt16 nMenuId = pMenu->GetCurItemId();
    if(nMenuId != USHRT_MAX)
    {
        if(nMenuId < 100)
            SetRegionDropMode( static_cast<RegionMode>(--nMenuId));
        else
            m_xContentTree->SetOutlineLevel( static_cast< sal_uInt8 >(nMenuId - 100) );
    }
    return false;
}

void SwNavigationPI::UpdateListBox()
{
    if (isDisposed())
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
                sEntry += m_aStatusArr[IDX_STR_ACTIVE];
            }
            else
                sEntry += m_aStatusArr[IDX_STR_INACTIVE];
            sEntry += ")";
            m_xDocListBox->append_text(sEntry);

            if (pConstView && pView == pConstView)
                nConstPos = nCount;

            nCount++;
        }
        pView = SwModule::GetNextView(pView);
    }
    m_xDocListBox->append_text(m_aStatusArr[3]); // "Active Window"
    nCount++;

    if(m_xContentTree->GetHiddenWrtShell())
    {
        OUString sEntry = m_xContentTree->GetHiddenWrtShell()->GetView().
                                        GetDocShell()->GetTitle() +
            " (" +
            m_aStatusArr[IDX_STR_HIDDEN] +
            ")";
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
    if( pFrameItem )
    {
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
            m_pxObjectShell.reset( new SfxObjectShellLock(pFrame->GetObjectShell()) );
            FillBox();
//TODO            m_xContentTree->Update();
        }
    }
}

OUString SwNavigationPI::CreateDropFileName( TransferableDataHelper& rData )
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
#if 0
    return ( !SwContentTree::IsInDrag() &&
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
#else
    return DND_ACTION_NONE;
#endif
}

sal_Int8 SwNavigationPI::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    TransferableDataHelper aData( rEvt.maDropEvent.Transferable );
    sal_Int8 nRet = DND_ACTION_NONE;
    if(SwContentTree::IsInDrag())
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
        if(m_pxObjectShell)
        {
            m_xContentTree->SetHiddenShell( nullptr );
            (*m_pxObjectShell)->DoClose();
            m_pxObjectShell.reset();
        }
        SfxStringItem aFileItem(SID_FILE_NAME, sFileName );
        SfxStringItem aOptionsItem( SID_OPTIONS, "HRC" );
        SfxLinkItem aLink( SID_DONELINK,
                            LINK( this, SwNavigationPI, DoneLink ) );
        GetActiveView()->GetViewFrame()->GetDispatcher()->ExecuteList(
                    SID_OPENDOC, SfxCallMode::ASYNCHRON,
                    { &aFileItem, &aOptionsItem, &aLink });
    }
    return nRet;
}

void SwNavigationPI::SetRegionDropMode(RegionMode nNewMode)
{
    m_nRegionMode = nNewMode;
    m_pConfig->SetRegionMode( m_nRegionMode );

    OUString sImageId;
    switch (nNewMode)
    {
        case RegionMode::NONE:
            sImageId = RID_BMP_DROP_REGION;
            break;
        case RegionMode::LINK:
            sImageId = RID_BMP_DROP_LINK;
            break;
        case RegionMode::EMBEDDED:
            sImageId = RID_BMP_DROP_COPY;
            break;
    }
    m_xContent3ToolBox->set_item_icon_name("dragmode", sImageId);
}

void SwNavigationPI::ToggleTree()
{
    bool bGlobalDoc = IsGlobalDoc();
    if (!IsGlobalMode() && bGlobalDoc)
    {
        SetUpdateMode(false);
        if (IsZoomedIn())
            ZoomOut();
        m_xGlobalBox->show();
//TODO        m_aGlobalTree->ShowTree();
        m_xGlobalToolBox->show();
        m_xContentBox->hide();
        m_xContentTree->HideTree();
        m_xContent1ToolBox->hide();
        m_xContent2ToolBox->hide();
        m_xContent3ToolBox->hide();
        m_xDocListBox->hide();
        SetGlobalMode(true);
        SetUpdateMode(true);
    }
    else
    {
        m_xGlobalBox->hide();
//TODO        m_aGlobalTree->HideTree();
        m_xGlobalToolBox->hide();
        if (!IsZoomedIn())
        {
            m_xContentBox->show();
            m_xContentTree->ShowTree();
            m_xContent1ToolBox->show();
            m_xContent2ToolBox->show();
            m_xContent3ToolBox->show();
            m_xDocListBox->show();
        }
        SetGlobalMode(false);
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

IMPL_LINK_NOARG(SwNavigationPI, ChangePageHdl, Timer *, void)
{
    if (!IsDisposed())
    {
        EditAction();
        GetPageEdit().grab_focus();
    }
}

IMPL_LINK_NOARG(SwNavigationPI, PageEditModifyHdl, weld::SpinButton&, void)
{
    if (m_aPageChgIdle.IsActive())
        m_aPageChgIdle.Stop();
    m_aPageChgIdle.Start();
}

SwView*  SwNavigationPI::GetCreateView() const
{
    if (!m_pCreateView)
    {
        SwView* pView = SwModule::GetFirstView();
        while (pView)
        {
            if(&pView->GetViewFrame()->GetBindings() == &m_rBindings)
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

SwNavigationChild::SwNavigationChild( vcl::Window* pParent,
                        sal_uInt16 nId,
                        SfxBindings* _pBindings )
    : SfxChildWindowContext( nId )
{
    Reference< XFrame > xFrame = _pBindings->GetActiveFrame();
    VclPtr< SwNavigationPI > pNavi = VclPtr< SwNavigationPI >::Create( pParent, xFrame, _pBindings );
    _pBindings->Invalidate(SID_NAVIGATOR);

    SwNavigationConfig* pNaviConfig = SW_MOD()->GetNavigationConfig();

    const ContentTypeId nRootType = pNaviConfig->GetRootType();
    if( nRootType != ContentTypeId::UNKNOWN )
    {
        pNavi->m_xContentTree->SetRootType(nRootType);
        pNavi->m_xContent2ToolBox->set_item_active("root", true);
        if (nRootType == ContentTypeId::OUTLINE)
        {
            pNavi->m_xContentTree->set_selection_mode(SelectionMode::Multiple);
#if 0
            pNavi->m_xContentTree->SetDragDropMode(DragDropMode::CTRL_MOVE |
                                                   DragDropMode::CTRL_COPY |
                                                   DragDropMode::ENABLE_TOP);
#endif
        }
    }
    pNavi->m_xContentTree->SetOutlineLevel( static_cast< sal_uInt8 >( pNaviConfig->GetOutlineLevel() ) );
    pNavi->SetRegionDropMode( pNaviConfig->GetRegionMode() );

    if (SfxNavigator* pNav = dynamic_cast<SfxNavigator*>(pParent))
    {
        pNav->SetMinOutputSizePixel(pNavi->GetOptimalSize());
        if (pNaviConfig->IsSmall())
            pNavi->ZoomIn();
    }

    SetWindow(pNavi);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
