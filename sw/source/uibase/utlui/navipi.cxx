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
#include <svl/urlbmk.hxx>
#include <svl/stritem.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/layout.hxx>
#include <svl/urihelper.hxx>
#include <sot/formats.hxx>
#include <sot/filelist.hxx>
#include <sfx2/event.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/dockwin.hxx>
#include <sfx2/navigat.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/settings.hxx>
#include <swtypes.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <navicfg.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <actctrl.hxx>
#include <IMark.hxx>
#include <navipi.hxx>
#include <content.hxx>
#include <workctrl.hxx>
#include <section.hxx>
#include <edtwin.hxx>
#include <sfx2/app.hxx>
#include <cmdid.h>
#include <helpids.h>

#include <strings.hrc>
#include <globals.hrc>
#include <bitmaps.hlst>

#include <unomid.h>
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
        FillBox();
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

IMPL_LINK( SwNavigationPI, DocListBoxSelectHdl, ListBox&, rBox, void )
{
    int nEntryIdx = rBox.GetSelectedEntryPos();
    SwView *pView ;
    pView = SwModule::GetFirstView();
    while (nEntryIdx-- && pView)
    {
        pView = SwModule::GetNextView(pView);
    }
    if(!pView)
    {
        nEntryIdx == 0 ?
            m_aContentTree->ShowHiddenShell():
                m_aContentTree->ShowActualView();

    }
    else
    {
        m_aContentTree->SetConstantShell(pView->GetWrtShellPtr());
    }
}

// Filling of the list box for outline view or documents
// The PI will be set to full size

void SwNavigationPI::FillBox()
{
    if(m_pContentWrtShell)
    {
        m_aContentTree->SetHiddenShell( m_pContentWrtShell );
        m_aContentTree->Display(  false );
    }
    else
    {
        SwView *pView = GetCreateView();
        if(!pView)
        {
            m_aContentTree->SetActiveShell(nullptr);
        }
        else if( pView != m_pActContView)
        {
            SwWrtShell* pWrtShell = pView->GetWrtShellPtr();
            m_aContentTree->SetActiveShell(pWrtShell);
        }
        else
            m_aContentTree->Display( true );
        m_pActContView = pView;
    }
}

void SwNavigationPI::UsePage()
{
    SwView *pView = GetCreateView();
    SwWrtShell *pSh = pView ? &pView->GetWrtShell() : nullptr;
    GetPageEdit().SetValue(1);
    if (pSh)
    {
        const sal_uInt16 nPageCnt = pSh->GetPageCnt();
        sal_uInt16 nPhyPage, nVirPage;
        pSh->GetPageNum(nPhyPage, nVirPage);

        GetPageEdit().SetMax(nPageCnt);
        GetPageEdit().SetLast(nPageCnt);
        GetPageEdit().SetValue(nPhyPage);
    }
}

// Select handler of the toolboxes

IMPL_LINK( SwNavigationPI, ToolBoxSelectHdl, ToolBox *, pBox, void )
{
    const sal_uInt16 nCurrItemId = pBox->GetCurItemId();
    const OUString sCommand = pBox->GetItemCommand(nCurrItemId);
    SwView *pView = GetCreateView();
    if (!pView)
        return;
    SwWrtShell &rSh = pView->GetWrtShell();
    // Get MouseModifier for Outline-Move

    // Standard: sublevels are taken
    // do not take sublevels with Ctrl
    bool bOutlineWithChildren  = ( KEY_MOD1 != pBox->GetModifier());
    int nFuncId = 0;
    bool bFocusToDoc = false;
    if (sCommand == "back")
    {
        // #i75416# move the execution of the search to an asynchronously called static link
        bool* pbNext = new bool(false);
        Application::PostUserEvent(LINK(pView, SwView, MoveNavigationHdl), pbNext);
    }
    else if (sCommand == "forward")
    {
        // #i75416# move the execution of the search to an asynchronously called static link
        bool* pbNext = new bool(true);
        Application::PostUserEvent(LINK(pView, SwView, MoveNavigationHdl), pbNext);
    }
    else if (sCommand == "root")
    {
        m_aContentTree->ToggleToRoot();
    }
    else if (sCommand == "listbox")
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
    else if (sCommand == "footer")
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
    else if (sCommand == "header")
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
    else if (sCommand == "anchor")
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
    else if (sCommand == "reminder")
    {
        MakeMark();
    }
    else if (sCommand == "down" ||
             sCommand == "up" ||
             sCommand == "promote" ||
             sCommand == "demote" ||
             sCommand == "edit")
    {
        if (IsGlobalMode())
            m_aGlobalTree->ExecCommand(sCommand);
        else
            m_aContentTree->ExecCommand(sCommand, bOutlineWithChildren);
    }
    else if (sCommand == "toggle")
    {
        ToggleTree();
        m_pConfig->SetGlobalActive(IsGlobalMode());
    }
    else if (sCommand == "save")
    {
        bool bSave = rSh.IsGlblDocSaveLinks();
        rSh.SetGlblDocSaveLinks( !bSave );
        pBox->CheckItem(nCurrItemId, !bSave );
    }

    if (nFuncId)
        lcl_UnSelectFrame(&rSh);
    if (bFocusToDoc)
        pView->GetEditWin().GrabFocus();
}

// Click handler of the toolboxes

IMPL_LINK( SwNavigationPI, ToolBoxClickHdl, ToolBox *, pBox, void )
{
    const sal_uInt16 nCurrItemId = pBox->GetCurItemId();
    const OUString sCommand = pBox->GetItemCommand(nCurrItemId);
    if (sCommand == "update" || sCommand == "insert")
        m_aGlobalTree->TbxMenuHdl(nCurrItemId, pBox);
}

IMPL_LINK( SwNavigationPI, ToolBoxDropdownClickHdl, ToolBox*, pBox, void )
{
    const sal_uInt16 nCurrItemId = pBox->GetCurItemId();
    const OUString sCommand = pBox->GetItemCommand(nCurrItemId);
    if (sCommand == "navigation")
       CreateNavigationTool(pBox->GetItemRect(nCurrItemId), true, this);
    else if (sCommand == "dragmode")
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
            pMenu->SetHelpId(i + 1, aHIDs[i]);
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
            pMenu->SetHelpId(i, HID_NAVI_OUTLINES);
        }
        pMenu->CheckItem( m_aContentTree->GetOutlineLevel() + 100 );
        pMenu->SetSelectHdl(LINK(this, SwNavigationPI, MenuSelectHdl));
        pBox->SetItemDown( nCurrItemId, true );
        pMenu->Execute(pBox, pBox->GetItemRect(nCurrItemId), PopupMenuFlags::ExecuteDown);
        pBox->SetItemDown( nCurrItemId, false );
        pMenu.disposeAndClear();
        pBox->EndSelection();
        pBox->Invalidate();
    }
}

SwNavHelpToolBox::SwNavHelpToolBox(Window* pParent)
    : ToolBox(pParent)
{
}

VCL_BUILDER_FACTORY(SwNavHelpToolBox)

void SwNavHelpToolBox::MouseButtonDown(const MouseEvent &rEvt)
{
    sal_uInt16 nItemId = GetItemId(rEvt.GetPosPixel());
    if (rEvt.GetButtons() == MOUSE_LEFT && GetItemCommand(nItemId) == "navigation")
    {
        m_xDialog->CreateNavigationTool(GetItemRect(nItemId), false, this);
        return;
    }
    ToolBox::MouseButtonDown(rEvt);
}

void SwNavigationPI::CreateNavigationTool(const tools::Rectangle& rRect, bool bSetFocus, vcl::Window *pParent)
{
    Reference< XFrame > xFrame = GetCreateView()->GetViewFrame()->GetFrame().GetFrameInterface();
    VclPtrInstance<SwScrollNaviPopup> pPopup(FN_SCROLL_NAVIGATION, xFrame, pParent);

    tools::Rectangle aRect(rRect);
    Point aT1 = aRect.TopLeft();
    aT1 = pPopup->GetParent()->OutputToScreenPixel(pPopup->GetParent()->AbsoluteScreenToOutputPixel(m_aContentToolBox->OutputToAbsoluteScreenPixel(aT1)));
    aRect.SetPos(aT1);
    pPopup->StartPopupMode(aRect, FloatWinPopupFlags::Right|FloatWinPopupFlags::AllowTearOff);
    SetPopupWindow( pPopup );
    if(bSetFocus)
    {
        pPopup->EndPopupMode(FloatWinPopupEndFlags::TearOff);
        pPopup->GrabFocus();
    }
}

FactoryFunction SwNavigationPI::GetUITestFactory() const
{
    return SwNavigationPIUIObject::create;
}

void SwNavHelpToolBox::RequestHelp(const HelpEvent& rHEvt)
{
    const sal_uInt16 nItemId = GetItemId(ScreenToOutputPixel(rHEvt.GetMousePosPixel()));
    const OUString sCommand(GetItemCommand(nItemId));
    if (sCommand == "back")
        SetQuickHelpText(nItemId, SwScrollNaviPopup::GetToolTip(false));
    else if (sCommand == "forward")
        SetQuickHelpText(nItemId, SwScrollNaviPopup::GetToolTip(true));
    ToolBox::RequestHelp(rHEvt);
}

void SwNavHelpToolBox::dispose()
{
    m_xDialog.clear();
    ToolBox::dispose();
}

SwNavHelpToolBox::~SwNavHelpToolBox()
{
    disposeOnce();
}

// Action-Handler Edit:
// Switches to the page if the structure view is not turned on.

IMPL_LINK( SwNavigationPI, EditAction, NumEditAction&, rEdit, void )
{
    SwView *pView = GetCreateView();
    if (pView)
    {
        if(m_aPageChgIdle.IsActive())
            m_aPageChgIdle.Stop();
        m_pCreateView->GetWrtShell().GotoPage(static_cast<sal_uInt16>(rEdit.GetValue()), true);
        m_pCreateView->GetEditWin().GrabFocus();
        m_pCreateView->GetViewFrame()->GetBindings().Invalidate(FN_STAT_PAGE);
    }
}

// If the page can be set here, the maximum is set.

IMPL_LINK( SwNavigationPI, EditGetFocus, Control&, rControl, void )
{
    NumEditAction* pEdit = static_cast<NumEditAction*>(&rControl);
    SwView *pView = GetCreateView();
    if (!pView)
        return;
    SwWrtShell &rSh = pView->GetWrtShell();

    const sal_uInt16 nPageCnt = rSh.GetPageCnt();
    pEdit->SetMax(nPageCnt);
    pEdit->SetLast(nPageCnt);
}

// Setting of an automatic mark
void SwNavigationPI::MakeMark()
{
    SwView *pView = GetCreateView();
    if (!pView) return;
    SwWrtShell &rSh = pView->GetWrtShell();
    IDocumentMarkAccess* const pMarkAccess = rSh.getIDocumentMarkAccess();

    // collect and sort navigator reminder names
    std::vector< OUString > vNavMarkNames;
    for(IDocumentMarkAccess::const_iterator_t ppMark = pMarkAccess->getAllMarksBegin();
        ppMark != pMarkAccess->getAllMarksEnd();
        ++ppMark)
        if( IDocumentMarkAccess::GetType(**ppMark) == IDocumentMarkAccess::MarkType::NAVIGATOR_REMINDER )
            vNavMarkNames.push_back(ppMark->get()->GetName());
    std::sort(vNavMarkNames.begin(), vNavMarkNames.end());

    // we are maxed out and delete one
    // nAutoMarkIdx rotates through the available MarkNames
    // this assumes that IDocumentMarkAccess generates Names in ascending order
    if(vNavMarkNames.size() == MAX_MARKS)
        pMarkAccess->deleteMark(pMarkAccess->findMark(vNavMarkNames[m_nAutoMarkIdx]));

    rSh.SetBookmark(vcl::KeyCode(), OUString(), IDocumentMarkAccess::MarkType::NAVIGATOR_REMINDER);
    SwView::SetActMark( m_nAutoMarkIdx );

    if(++m_nAutoMarkIdx == MAX_MARKS)
        m_nAutoMarkIdx = 0;
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
    if (IsGlobalMode())
    {
        m_aGlobalBox->Show();
        m_aGlobalTree->ShowTree();
    }
    else
    {
        m_aContentBox->Show();
        m_aContentTree->ShowTree();
        m_aDocListBox->Show();
    }

    Size aOptimalSize(GetOptimalSize());
    Size aNewSize(pNav->GetOutputSizePixel());
    aNewSize.setHeight( m_aExpandedSize.Height() );
    pNav->SetMinOutputSizePixel(aOptimalSize);
    pNav->SetOutputSizePixel(aNewSize);

    SvTreeListEntry* pFirst = m_aContentTree->FirstSelected();
    if (pFirst)
        m_aContentTree->Select(pFirst); // Enable toolbox
    m_pConfig->SetSmall(false);
    m_aContentToolBox->CheckItem(m_aContentToolBox->GetItemId("listbox"));
}

void SwNavigationPI::ZoomIn()
{
    if (IsZoomedIn())
        return;
    SfxNavigator* pNav = dynamic_cast<SfxNavigator*>(GetParent());
    if (!pNav)
        return;

    m_aExpandedSize = GetSizePixel();

    m_aContentBox->Hide();
    m_aContentTree->HideTree();
    m_aGlobalBox->Hide();
    m_aGlobalTree->HideTree();
    m_aDocListBox->Hide();
    m_bIsZoomedIn = true;

    Size aOptimalSize(GetOptimalSize());
    Size aNewSize(pNav->GetOutputSizePixel());
    aNewSize.setHeight( aOptimalSize.Height() );
    pNav->SetMinOutputSizePixel(aOptimalSize);
    pNav->SetOutputSizePixel(aNewSize);

    SvTreeListEntry* pFirst = m_aContentTree->FirstSelected();
    if (pFirst)
        m_aContentTree->Select(pFirst); // Enable toolbox
    m_pConfig->SetSmall(true);
    m_aContentToolBox->CheckItem(m_aContentToolBox->GetItemId("listbox"), false);
}

enum StatusIndex
{
    IDX_STR_HIDDEN = 0,
    IDX_STR_ACTIVE = 1,
    IDX_STR_INACTIVE = 2
};

SwNavigationPI::SwNavigationPI(SfxBindings* _pBindings,
                               vcl::Window* pParent)
    : PanelLayout(pParent, "NavigatorPanel", "modules/swriter/ui/navigatorpanel.ui", nullptr)
    , SfxControllerItem(SID_DOCFULLNAME, *_pBindings)
    , m_pContentView(nullptr)
    , m_pContentWrtShell(nullptr)
    , m_pActContView(nullptr)
    , m_pCreateView(nullptr)
    , m_pPopupWindow(nullptr)
    , m_pFloatingWindow(nullptr)
    , m_pConfig(SW_MOD()->GetNavigationConfig())
    , m_rBindings(*_pBindings)
    , m_nAutoMarkIdx(1)
    , m_nRegionMode(RegionMode::NONE)
    , m_bIsZoomedIn(false)
    , m_bGlobalMode(false)
{
    get(m_aContentToolBox, "content");
    m_aContentToolBox->SetLineCount(2);
    m_aContentToolBox->InsertBreak(8);
    m_aContentToolBox->SetDialog(this);
    get(m_aGlobalToolBox, "global");
    get(m_aDocListBox, "documents");

    get(m_aContentBox, "contentbox");
    m_aContentTree = VclPtr<SwContentTree>::Create(m_aContentBox, this);
    m_aContentTree->set_expand(true);
    get(m_aGlobalBox, "globalbox");
    m_aGlobalTree = VclPtr<SwGlobalTree>::Create(m_aGlobalBox, this);
    m_aGlobalTree->set_expand(true);

    GetCreateView();

    m_aContentToolBox->SetHelpId(HID_NAVIGATOR_TOOLBOX);
    m_aGlobalToolBox->SetHelpId(HID_NAVIGATOR_GLOBAL_TOOLBOX);
    m_aDocListBox->SetHelpId(HID_NAVIGATOR_LISTBOX);
    m_aDocListBox->SetDropDownLineCount(9);

    // Insert the numeric field in the toolbox.
    m_xEdit = VclPtr<NumEditAction>::Create(
                    m_aContentToolBox.get(), WB_BORDER|WB_TABSTOP|WB_LEFT|WB_REPEAT|WB_SPIN);
    m_xEdit->SetMin(1);
    m_xEdit->SetFirst(1);
    m_xEdit->SetActionHdl(LINK(this, SwNavigationPI, EditAction));
    m_xEdit->SetGetFocusHdl(LINK(this, SwNavigationPI, EditGetFocus));
    m_xEdit->SetAccessibleName(m_xEdit->GetQuickHelpText());
    m_xEdit->SetUpHdl(LINK(this, SwNavigationPI, PageEditModifyHdl));
    m_xEdit->SetDownHdl(LINK(this, SwNavigationPI, PageEditModifyHdl));

    // Double separators are not allowed, so you have to
    // determine the suitable size differently.
    tools::Rectangle aFirstRect = m_aContentToolBox->GetItemRect(m_aContentToolBox->GetItemId("anchor"));
    tools::Rectangle aSecondRect = m_aContentToolBox->GetItemRect(m_aContentToolBox->GetItemId("header"));
    Size aItemWinSize( aFirstRect.Left() - aSecondRect.Left(),
                       aFirstRect.Bottom() - aFirstRect.Top() );
    Size aOptimalSize(m_xEdit->CalcMinimumSizeForText(m_xEdit->CreateFieldText(9999)));
    aItemWinSize.setWidth( std::max(aItemWinSize.Width(), aOptimalSize.Width()) );
    m_xEdit->SetSizePixel(aItemWinSize);
    m_aContentToolBox->InsertSeparator(4);
    m_aContentToolBox->InsertWindow( FN_PAGENUMBER, m_xEdit, ToolBoxItemBits::NONE, 4);
    m_aContentToolBox->InsertSeparator(4);
    m_aContentToolBox->SetHelpId(FN_PAGENUMBER, "modules/swriter/ui/navigatorpanel/numericfield");
    m_aContentToolBox->ShowItem(FN_PAGENUMBER);
    if (!IsGlobalDoc())
    {
        m_aContentToolBox->HideItem(m_aContentToolBox->GetItemId("toggle"));
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

    m_aContentTree->SetStyle(m_aContentTree->GetStyle()|WB_HASBUTTONS|WB_HASBUTTONSATROOT|
                             WB_CLIPCHILDREN|WB_HSCROLL|WB_TABSTOP);
    m_aContentTree->SetForceMakeVisible(true);
    m_aContentTree->SetSpaceBetweenEntries(3);
    m_aContentTree->SetSelectionMode(SelectionMode::Single);
    m_aContentTree->SetDragDropMode(DragDropMode::CTRL_MOVE |
                                    DragDropMode::CTRL_COPY |
                                    DragDropMode::ENABLE_TOP);
    m_aContentTree->EnableAsyncDrag(true);
    m_aContentTree->ShowTree();
    m_aContentToolBox->CheckItem(m_aContentToolBox->GetItemId("listbox"));

//  TreeListBox for global document
    m_aGlobalTree->SetSelectionMode( SelectionMode::Multiple );
    m_aGlobalTree->SetStyle(m_aGlobalTree->GetStyle()|WB_HASBUTTONS|WB_HASBUTTONSATROOT|
                            WB_CLIPCHILDREN|WB_HSCROLL|WB_TABSTOP);

//  Handler
    Link<ToolBox *, void> aLk = LINK(this, SwNavigationPI, ToolBoxSelectHdl);
    m_aContentToolBox->SetSelectHdl( aLk );
    m_aGlobalToolBox->SetSelectHdl( aLk );
    m_aDocListBox->SetSelectHdl(LINK(this, SwNavigationPI,
                                                    DocListBoxSelectHdl));
    m_aContentToolBox->SetClickHdl( LINK(this, SwNavigationPI, ToolBoxClickHdl) );
    m_aContentToolBox->SetDropdownClickHdl( LINK(this, SwNavigationPI, ToolBoxDropdownClickHdl) );
    m_aGlobalToolBox->SetClickHdl( LINK(this, SwNavigationPI, ToolBoxClickHdl) );
    m_aGlobalToolBox->SetDropdownClickHdl( LINK(this, SwNavigationPI, ToolBoxDropdownClickHdl) );
    m_aGlobalToolBox->CheckItem(m_aGlobalToolBox->GetItemId("toggle"));

    vcl::Font aFont(GetFont());
    aFont.SetWeight(WEIGHT_NORMAL);
    GetPageEdit().SetFont(aFont);
    aFont = m_aContentTree->GetFont();
    aFont.SetWeight(WEIGHT_NORMAL);
    m_aContentTree->SetFont(aFont);
    m_aGlobalTree->SetFont(aFont);

    StartListening(*SfxGetpApp());

    sal_uInt16 nNavId = m_aContentToolBox->GetItemId("navigation");
    m_aContentToolBox->SetItemBits(nNavId, m_aContentToolBox->GetItemBits(nNavId) | ToolBoxItemBits::DROPDOWNONLY );
    sal_uInt16 nDropId = m_aContentToolBox->GetItemId("dragmode");
    m_aContentToolBox->SetItemBits(nDropId, m_aContentToolBox->GetItemBits(nDropId) | ToolBoxItemBits::DROPDOWNONLY );
    sal_uInt16 nOutlineId = m_aContentToolBox->GetItemId("headings");
    m_aContentToolBox->SetItemBits(nOutlineId, m_aContentToolBox->GetItemBits(nOutlineId) | ToolBoxItemBits::DROPDOWNONLY );

    if(IsGlobalDoc())
    {
        SwView *pActView = GetCreateView();
        m_aGlobalToolBox->CheckItem(m_aGlobalToolBox->GetItemId("save"),
                    pActView->GetWrtShellPtr()->IsGlblDocSaveLinks());
        if (m_pConfig->IsGlobalActive())
            ToggleTree();
        m_aGlobalTree->GrabFocus();
    }
    else
        m_aContentTree->GrabFocus();
    UsePage();
    m_aPageChgIdle.SetInvokeHandler(LINK(this, SwNavigationPI, ChangePageHdl));
    m_aPageChgIdle.SetPriority(TaskPriority::LOWEST);

    m_aContentTree->SetAccessibleName(SwResId(STR_ACCESS_TL_CONTENT));
    m_aGlobalTree->SetAccessibleName(SwResId(STR_ACCESS_TL_GLOBAL));
    m_aDocListBox->SetAccessibleName(m_aStatusArr[3]);

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

    m_aContentToolBox->GetItemWindow(FN_PAGENUMBER)->disposeOnce();
    m_aContentToolBox->Clear();
    if (m_pxObjectShell)
    {
        if (m_pxObjectShell->Is())
            (*m_pxObjectShell)->DoClose();
        m_pxObjectShell.reset();
    }

    if (IsBound())
        m_rBindings.Release(*this);

    m_pFloatingWindow.disposeAndClear();
    m_pPopupWindow.disposeAndClear();
    m_aDocListBox.clear();
    m_aGlobalTree.disposeAndClear();
    m_aGlobalBox.clear();
    m_aContentTree.disposeAndClear();
    m_aContentBox.clear();
    m_aGlobalToolBox.disposeAndClear();
    m_xEdit.disposeAndClear();
    m_aContentToolBox.clear();

    m_aPageChgIdle.Stop();

    ::SfxControllerItem::dispose();

    PanelLayout::dispose();
}

void SwNavigationPI::SetPopupWindow( SfxPopupWindow* pWindow )
{
    m_pPopupWindow.disposeAndClear();
    m_pPopupWindow = pWindow;
    m_pPopupWindow->SetPopupModeEndHdl( LINK( this, SwNavigationPI, PopupModeEndHdl ));
    m_pPopupWindow->SetDeleteLink_Impl( LINK( this, SwNavigationPI, ClosePopupWindow ));
}

IMPL_LINK_NOARG(SwNavigationPI, PopupModeEndHdl, FloatingWindow*, void)
{
    if ( m_pPopupWindow->IsVisible() )
    {
        // Replace floating window with popup window and destroy
        // floating window instance.
        m_pFloatingWindow.disposeAndClear();
        m_pFloatingWindow = m_pPopupWindow;
        m_pPopupWindow    = nullptr;
    }
    else
    {
        // Popup window has been closed by the user. No replacement, instance
        // will destroy itself.
        m_pPopupWindow = nullptr;
    }
}

IMPL_LINK( SwNavigationPI, ClosePopupWindow, SfxPopupWindow *, pWindow, void )
{
    if ( pWindow == m_pFloatingWindow )
        m_pFloatingWindow = nullptr;
    else
        m_pPopupWindow = nullptr;
}

void SwNavigationPI::StateChanged( sal_uInt16 nSID, SfxItemState /*eState*/,
                                            const SfxPoolItem* /*pState*/ )
{
    if(nSID == SID_DOCFULLNAME)
    {
        SwView *pActView = GetCreateView();
        if(pActView)
        {
            SwWrtShell* pWrtShell = pActView->GetWrtShellPtr();
            m_aContentTree->SetActiveShell(pWrtShell);
            bool bGlobal = IsGlobalDoc();
            m_aContentToolBox->ShowItem(m_aContentToolBox->GetItemId("toggle"), bGlobal);
            if ((!bGlobal && IsGlobalMode()) || (!IsGlobalMode() && m_pConfig->IsGlobalActive()))
            {
                ToggleTree();
            }
            if (bGlobal)
            {
                m_aGlobalToolBox->CheckItem(m_aGlobalToolBox->GetItemId("save"), pWrtShell->IsGlblDocSaveLinks());
            }
        }
        else
        {
            m_aContentTree->SetActiveShell(nullptr);
        }
        UpdateListBox();
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
        m_aContentToolBox->ShowItem(m_aContentToolBox->GetItemId("listbox"), SfxChildWindowContext::GetFloatingWindow(GetParent()) != nullptr);
    }
    else if (nStateChange == StateChangedType::ControlFocus)
    {
        if (m_aContentTree)
        {
            // update documents listbox
            UpdateListBox();
        }
    }
}

// Get the numeric field from the toolbox.

NumEditAction& SwNavigationPI::GetPageEdit()
{
    return *static_cast<NumEditAction*>(m_aContentToolBox->GetItemWindow(FN_PAGENUMBER));
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
            if (pHint->GetEventId() == SfxEventHintId::OpenDoc)
            {
                SwView *pActView = GetCreateView();
                if(pActView)
                {
                    SwWrtShell* pWrtShell = pActView->GetWrtShellPtr();
                    m_aContentTree->SetActiveShell(pWrtShell);
                    if(m_aGlobalTree->IsVisible())
                    {
                        if(m_aGlobalTree->Update( false ))
                            m_aGlobalTree->Display();
                        else
                        // If no update is needed, then paint at least,
                        // because of the red entries for the broken links.
                            m_aGlobalTree->Invalidate();
                    }
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
            m_aContentTree->SetOutlineLevel( static_cast< sal_uInt8 >(nMenuId - 100) );
    }
    return false;
}

void SwNavigationPI::UpdateListBox()
{
    if (isDisposed())
        return;

    m_aDocListBox->SetUpdateMode(false);
    m_aDocListBox->Clear();
    SwView *pActView = GetCreateView();
    bool bDisable = pActView == nullptr;
    SwView *pView = SwModule::GetFirstView();
    sal_Int32 nCount = 0;
    sal_Int32 nAct = 0;
    sal_Int32 nConstPos = 0;
    const SwView* pConstView = m_aContentTree->IsConstantView() &&
                                m_aContentTree->GetActiveWrtShell() ?
                                    &m_aContentTree->GetActiveWrtShell()->GetView():
                                        nullptr;
    while (pView)
    {
        SfxObjectShell* pDoc = pView->GetDocShell();
        // #i53333# don't show help pages here
        if ( !pDoc->IsHelpDocument() )
        {
            OUString sEntry = pDoc->GetTitle();
            sEntry += " (";
            if (pView == pActView)
            {
                nAct = nCount;
                sEntry += m_aStatusArr[IDX_STR_ACTIVE];
            }
            else
                sEntry += m_aStatusArr[IDX_STR_INACTIVE];
            sEntry += ")";
            m_aDocListBox->InsertEntry(sEntry);

            if (pConstView && pView == pConstView)
                nConstPos = nCount;

            nCount++;
        }
        pView = SwModule::GetNextView(pView);
    }
    m_aDocListBox->InsertEntry(m_aStatusArr[3]); // "Active Window"
    nCount++;

    if(m_aContentTree->GetHiddenWrtShell())
    {
        OUString sEntry = m_aContentTree->GetHiddenWrtShell()->GetView().
                                        GetDocShell()->GetTitle();
        sEntry += " (";
        sEntry += m_aStatusArr[IDX_STR_HIDDEN];
        sEntry += ")";
        m_aDocListBox->InsertEntry(sEntry);
        bDisable = false;
    }
    if(m_aContentTree->IsActiveView())
    {
        //Either the name of the current Document or "Active Document".
        m_aDocListBox->SelectEntryPos( pActView ? nAct : --nCount );
    }
    else if(m_aContentTree->IsHiddenView())
    {
        m_aDocListBox->SelectEntryPos(nCount);
    }
    else
        m_aDocListBox->SelectEntryPos(nConstPos);

    m_aDocListBox->Enable( !bDisable );
    m_aDocListBox->SetUpdateMode(true);
}

IMPL_LINK(SwNavigationPI, DoneLink, SfxPoolItem const *, pItem, void)
{
    const SfxViewFrameItem* pFrameItem = dynamic_cast<SfxViewFrameItem const *>( pItem  );
    if( pFrameItem )
    {
        SfxViewFrame* pFrame =  pFrameItem->GetFrame();
        if(pFrame)
        {
            m_aContentTree->Clear();
            m_pContentView = dynamic_cast<SwView*>( pFrame->GetViewShell() );
            OSL_ENSURE(m_pContentView, "no SwView");
            if(m_pContentView)
                m_pContentWrtShell = m_pContentView->GetWrtShellPtr();
            else
                m_pContentWrtShell = nullptr;
            m_pxObjectShell.reset( new SfxObjectShellLock(pFrame->GetObjectShell()) );
            FillBox();
            m_aContentTree->Update();
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
        INetBookmark aBkmk = INetBookmark(OUString(), OUString());
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
    return ( !SwContentTree::IsInDrag() &&
        ( m_aContentTree->IsDropFormatSupported( SotClipboardFormatId::SIMPLE_FILE ) ||
          m_aContentTree->IsDropFormatSupported( SotClipboardFormatId::STRING ) ||
          m_aContentTree->IsDropFormatSupported( SotClipboardFormatId::SOLK ) ||
           m_aContentTree->IsDropFormatSupported( SotClipboardFormatId::NETSCAPE_BOOKMARK )||
           m_aContentTree->IsDropFormatSupported( SotClipboardFormatId::FILECONTENT ) ||
           m_aContentTree->IsDropFormatSupported( SotClipboardFormatId::FILEGRPDESCRIPTOR ) ||
           m_aContentTree->IsDropFormatSupported( SotClipboardFormatId::UNIFORMRESOURCELOCATOR ) ||
           m_aContentTree->IsDropFormatSupported( SotClipboardFormatId::FILENAME )))
        ? DND_ACTION_COPY
        : DND_ACTION_NONE;
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
            m_aContentTree->SetHiddenShell( nullptr );
            (*m_pxObjectShell)->DoClose();
            m_pxObjectShell.reset();
        }
        SfxStringItem aFileItem(SID_FILE_NAME, sFileName );
        SfxStringItem aOptionsItem( SID_OPTIONS, OUString("HRC") );
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
    m_aContentToolBox->SetItemImage(m_aContentToolBox->GetItemId("dragmode"), Image(StockImage::Yes, sImageId));
}

void SwNavigationPI::ToggleTree()
{
    bool bGlobalDoc = IsGlobalDoc();
    if (!IsGlobalMode() && bGlobalDoc)
    {
        SetUpdateMode(false);
        if (IsZoomedIn())
            ZoomOut();
        m_aGlobalBox->Show();
        m_aGlobalTree->ShowTree();
        m_aGlobalToolBox->Show();
        m_aContentBox->Hide();
        m_aContentTree->HideTree();
        m_aContentToolBox->Hide();
        m_aDocListBox->Hide();
        SetGlobalMode(true);
        SetUpdateMode(true);
    }
    else
    {
        m_aGlobalBox->Hide();
        m_aGlobalTree->HideTree();
        m_aGlobalToolBox->Hide();
        if (!IsZoomedIn())
        {
            m_aContentBox->Show();
            m_aContentTree->ShowTree();
            m_aContentToolBox->Show();
            m_aDocListBox->Show();
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
        EditAction(GetPageEdit());
        GetPageEdit().GrabFocus();
    }
}

IMPL_LINK_NOARG(SwNavigationPI, PageEditModifyHdl, SpinField&, void)
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
    VclPtr<SwNavigationPI> pNavi = VclPtr<SwNavigationPI>::Create(_pBindings, pParent);
    _pBindings->Invalidate(SID_NAVIGATOR);

    SwNavigationConfig* pNaviConfig = SW_MOD()->GetNavigationConfig();

    const ContentTypeId nRootType = pNaviConfig->GetRootType();
    if( nRootType != ContentTypeId::UNKNOWN )
    {
        pNavi->m_aContentTree->SetRootType(nRootType);
        pNavi->m_aContentToolBox->CheckItem(pNavi->m_aContentToolBox->GetItemId("root"));
        if (nRootType == ContentTypeId::OUTLINE)
        {
            pNavi->m_aContentTree->SetSelectionMode(SelectionMode::Multiple);
            pNavi->m_aContentTree->SetDragDropMode(DragDropMode::CTRL_MOVE |
                                                   DragDropMode::CTRL_COPY |
                                                   DragDropMode::ENABLE_TOP);
        }
    }
    pNavi->m_aContentTree->SetOutlineLevel( static_cast< sal_uInt8 >( pNaviConfig->GetOutlineLevel() ) );
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
