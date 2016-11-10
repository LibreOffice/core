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
#include <vcl/graphicfilter.hxx>
#include <svl/urihelper.hxx>
#include <sot/formats.hxx>
#include <sot/filelist.hxx>
#include <sfx2/event.hxx>
#include <sfx2/imgmgr.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/dockwin.hxx>
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
#include <helpid.h>
#include <ribbar.hrc>
#include <navipi.hrc>
#include <utlui.hrc>

#include "access.hrc"
#include "globals.hrc"

#include <unomid.h>
#include <memory>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;

SFX_IMPL_CHILDWINDOW_CONTEXT( SwNavigationChild, SID_NAVIGATOR, SwView )

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

void SwNavigationPI::MoveOutline(sal_uInt16 nSource, sal_uInt16 nTarget,
                                                    bool bWithChildren)
{
    SwView *pView = GetCreateView();
    SwWrtShell &rSh = pView->GetWrtShell();
    if(nTarget < nSource || nTarget == USHRT_MAX)
        nTarget ++;
    if ( rSh.IsOutlineMovable( nSource ))
    {

        short nMove = nTarget-nSource; //( nDir<0 ) ? 1 : 0 ;
        rSh.GotoOutline(nSource);
        if (bWithChildren)
            rSh.MakeOutlineSel(nSource, nSource, true);
        // While moving, the selected children does not counting.
        const sal_uInt16 nLastOutlinePos = rSh.GetOutlinePos(MAXLEVEL);
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

IMPL_LINK_TYPED( SwNavigationPI, DocListBoxSelectHdl, ListBox&, rBox, void )
{
    int nEntryIdx = rBox.GetSelectEntryPos();
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

void SwNavigationPI::UsePage(SwWrtShell *pSh)
{
    if (!pSh)
    {
        SwView *pView = GetCreateView();
        pSh = pView ? &pView->GetWrtShell() : nullptr;
        GetPageEdit().SetValue(1);
    }
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

IMPL_LINK_TYPED( SwNavigationPI, ToolBoxSelectHdl, ToolBox *, pBox, void )
{
    const sal_uInt16 nCurrItemId = pBox->GetCurItemId();
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
    switch (nCurrItemId)
    {
        case FN_UP:
        case FN_DOWN:
        {
            // #i75416# move the execution of the search to an asynchronously called static link
            bool* pbNext = new bool( FN_DOWN == nCurrItemId );
            Application::PostUserEvent( LINK(pView, SwView, MoveNavigationHdl), pbNext );
        }
        break;
        case FN_SHOW_ROOT:
        {
            m_aContentTree->ToggleToRoot();
        }
        break;
        case FN_SHOW_CONTENT_BOX:
        case FN_SELECT_CONTENT:
            if(m_pContextWin!=nullptr && m_pContextWin->GetFloatingWindow()!=nullptr)
            {
                if(_IsZoomedIn() )
                {
                    _ZoomOut();
                }
                else
                {
                    _ZoomIn();
                }
            }
            return;
        // Functions that will trigger a direct action.

        case FN_SELECT_FOOTER:
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
        break;
        case FN_SELECT_HEADER:
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
        break;
        case FN_SELECT_FOOTNOTE:
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
        break;

        case FN_SELECT_SET_AUTO_BOOKMARK:
            MakeMark();
        break;
        case FN_ITEM_DOWN:
        case FN_ITEM_UP:
        case FN_ITEM_LEFT:
        case FN_ITEM_RIGHT:
        case FN_GLOBAL_EDIT:
        {
            if(IsGlobalMode())
                m_aGlobalTree->ExecCommand(nCurrItemId);
            else
                m_aContentTree->ExecCommand(nCurrItemId, bOutlineWithChildren);
        }
        break;
        case FN_GLOBAL_SWITCH:
        {
            ToggleTree();
            m_pConfig->SetGlobalActive(IsGlobalMode());
        }
        break;
        case FN_GLOBAL_SAVE_CONTENT:
        {
            bool bSave = rSh.IsGlblDocSaveLinks();
            rSh.SetGlblDocSaveLinks( !bSave );
            pBox->CheckItem(FN_GLOBAL_SAVE_CONTENT, !bSave );
        }
        break;
    }
    if (nFuncId)
    {
        lcl_UnSelectFrame(&rSh);
    }
    if(bFocusToDoc)
        pView->GetEditWin().GrabFocus();
}

// Click handler of the toolboxes

IMPL_LINK_TYPED( SwNavigationPI, ToolBoxClickHdl, ToolBox *, pBox, void )
{
    const sal_uInt16 nCurrItemId = pBox->GetCurItemId();
    switch (nCurrItemId)
    {
        case FN_GLOBAL_UPDATE:
        case FN_GLOBAL_OPEN:
        {
            m_aGlobalTree->TbxMenuHdl(nCurrItemId, pBox);
        }
        break;
    }
}

IMPL_LINK_TYPED( SwNavigationPI, ToolBoxDropdownClickHdl, ToolBox*, pBox, void )
{
    const sal_uInt16 nCurrItemId = pBox->GetCurItemId();
    switch (nCurrItemId)
    {
        case FN_CREATE_NAVIGATION:
        {
            CreateNavigationTool(pBox->GetItemRect(FN_CREATE_NAVIGATION), true, this);
        }
        break;

        case FN_DROP_REGION:
        {
            static const char* aHIDs[] =
            {
                HID_NAVI_DRAG_HYP,
                HID_NAVI_DRAG_LINK,
                HID_NAVI_DRAG_COPY,
            };
            std::unique_ptr<PopupMenu> pMenu(new PopupMenu);
            for (sal_uInt16 i = 0; i <= static_cast<sal_uInt16>(RegionMode::EMBEDDED); i++)
            {
                pMenu->InsertItem( i + 1, m_aContextArr[i] );
                pMenu->SetHelpId(i + 1, aHIDs[i]);
            }
            pMenu->CheckItem( static_cast<int>(m_nRegionMode) + 1 );
            pMenu->SetSelectHdl(LINK(this, SwNavigationPI, MenuSelectHdl));
            pBox->SetItemDown( nCurrItemId, true );
            pMenu->Execute( pBox,
                    pBox->GetItemRect(FN_DROP_REGION),
                    PopupMenuFlags::ExecuteDown );
            pBox->SetItemDown( nCurrItemId, false );
            pBox->EndSelection();
            pMenu.reset();
            pBox->Invalidate();
        }
        break;
        case FN_OUTLINE_LEVEL:
        {
            std::unique_ptr<PopupMenu> pMenu(new PopupMenu);
            for (sal_uInt16 i = 101; i <= 100 + MAXLEVEL; i++)
            {
                pMenu->InsertItem( i, OUString::number(i - 100) );
                pMenu->SetHelpId( i, HID_NAVI_OUTLINES );
            }
            pMenu->CheckItem( m_aContentTree->GetOutlineLevel() + 100 );
            pMenu->SetSelectHdl(LINK(this, SwNavigationPI, MenuSelectHdl));
            pBox->SetItemDown( nCurrItemId, true );
            pMenu->Execute( pBox,
                    pBox->GetItemRect(FN_OUTLINE_LEVEL),
                    PopupMenuFlags::ExecuteDown );
            pBox->SetItemDown( nCurrItemId, false );
            pMenu.reset();
            pBox->EndSelection();
            pBox->Invalidate();
        }
        break;
    }
}

SwNavHelpToolBox::SwNavHelpToolBox(SwNavigationPI* pParent, const ResId &rResId) :
            SwHelpToolBox(pParent, rResId)
{}

void SwNavHelpToolBox::MouseButtonDown(const MouseEvent &rEvt)
{
    if(rEvt.GetButtons() == MOUSE_LEFT &&
            FN_CREATE_NAVIGATION == GetItemId(rEvt.GetPosPixel()))
    {
        static_cast<SwNavigationPI*>(GetParent())->CreateNavigationTool(GetItemRect(FN_CREATE_NAVIGATION), false, this);
    }
    else
        SwHelpToolBox::MouseButtonDown(rEvt);
}

void SwNavigationPI::CreateNavigationTool(const Rectangle& rRect, bool bSetFocus, vcl::Window *pParent)
{
    Reference< XFrame > xFrame = GetCreateView()->GetViewFrame()->GetFrame().GetFrameInterface();
    VclPtrInstance<SwScrollNaviPopup> pPopup(FN_SCROLL_NAVIGATION, xFrame, pParent);

    Rectangle aRect(rRect);
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

void  SwNavHelpToolBox::RequestHelp( const HelpEvent& rHEvt )
{
    const sal_uInt16 nItemId = GetItemId(ScreenToOutputPixel(rHEvt.GetMousePosPixel()));
    if( FN_UP == nItemId || FN_DOWN == nItemId )
    {
        SetItemText(nItemId, SwScrollNaviPopup::GetQuickHelpText((FN_DOWN == nItemId)));
    }
    SwHelpToolBox::RequestHelp(rHEvt);
}

// Action-Handler Edit:
// Switches to the page if the structure view is not turned on.

IMPL_LINK_TYPED( SwNavigationPI, EditAction, NumEditAction&, rEdit, void )
{
    SwView *pView = GetCreateView();
    if (pView)
    {
        if(m_aPageChgIdle.IsActive())
            m_aPageChgIdle.Stop();
        m_pCreateView->GetWrtShell().GotoPage((sal_uInt16)rEdit.GetValue(), true);
        m_pCreateView->GetEditWin().GrabFocus();
        m_pCreateView->GetViewFrame()->GetBindings().Invalidate(FN_STAT_PAGE);
    }
}

// If the page can be set here, the maximum is set.

IMPL_LINK_TYPED( SwNavigationPI, EditGetFocus, Control&, rControl, void )
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
    ::std::vector< OUString > vNavMarkNames;
    for(IDocumentMarkAccess::const_iterator_t ppMark = pMarkAccess->getAllMarksBegin();
        ppMark != pMarkAccess->getAllMarksEnd();
        ++ppMark)
        if( IDocumentMarkAccess::GetType(**ppMark) == IDocumentMarkAccess::MarkType::NAVIGATOR_REMINDER )
            vNavMarkNames.push_back(ppMark->get()->GetName());
    ::std::sort(vNavMarkNames.begin(), vNavMarkNames.end());

    // we are maxed out and delete one
    // nAutoMarkIdx rotates through the available MarkNames
    // this assumes that IDocumentMarkAccess generates Names in ascending order
    if(vNavMarkNames.size() == MAX_MARKS)
        pMarkAccess->deleteMark(pMarkAccess->findMark(vNavMarkNames[m_nAutoMarkIdx]));

    rSh.SetBookmark(vcl::KeyCode(), OUString(), OUString(), IDocumentMarkAccess::MarkType::NAVIGATOR_REMINDER);
    SwView::SetActMark( m_nAutoMarkIdx );

    if(++m_nAutoMarkIdx == MAX_MARKS)
        m_nAutoMarkIdx = 0;
}

void SwNavigationPI::GotoPage()
{
    if (m_pContextWin && m_pContextWin->GetFloatingWindow() && m_pContextWin->GetFloatingWindow()->IsRollUp())
        _ZoomIn();
    if(IsGlobalMode())
        ToggleTree();
    UsePage(nullptr);
    GetPageEdit().GrabFocus();
}

void SwNavigationPI::_ZoomOut()
{
    if (_IsZoomedIn())
    {
        FloatingWindow* pFloat = m_pContextWin!=nullptr ? m_pContextWin->GetFloatingWindow() : nullptr;
        m_bIsZoomedIn = false;
        Size aSz(GetOutputSizePixel());
        aSz.Height() = m_nZoomOut;
        Size aMinOutSizePixel = static_cast<SfxDockingWindow*>(GetParent())->GetMinOutputSizePixel();
        static_cast<SfxDockingWindow*>(GetParent())->SetMinOutputSizePixel(Size(
                            aMinOutSizePixel.Width(),m_nZoomOutInit));
        if (pFloat != nullptr)
            pFloat->SetOutputSizePixel(aSz);
        FillBox();
        if(IsGlobalMode())
        {
            m_aGlobalTree->ShowTree();
        }
        else
        {
            m_aContentTree->ShowTree();
            m_aDocListBox->Show();
        }
        SvTreeListEntry* pFirst = m_aContentTree->FirstSelected();
        if(pFirst)
            m_aContentTree->Select(pFirst); // Enable toolbox
        m_pConfig->SetSmall( false );
        m_aContentToolBox->CheckItem(FN_SHOW_CONTENT_BOX);
    }
}

void SwNavigationPI::_ZoomIn()
{
    if (m_pContextWin != nullptr)
    {
        FloatingWindow* pFloat = m_pContextWin->GetFloatingWindow();
        if (pFloat &&
            (!_IsZoomedIn() || ( m_pContextWin->GetFloatingWindow()->IsRollUp())))
        {
            m_aContentTree->HideTree();
            m_aDocListBox->Hide();
            m_aGlobalTree->HideTree();
            m_bIsZoomedIn = true;
            Size aSz(GetOutputSizePixel());
            if( aSz.Height() > m_nZoomIn )
                m_nZoomOut = ( short ) aSz.Height();

            aSz.Height() = m_nZoomIn;
            Size aMinOutSizePixel = static_cast<SfxDockingWindow*>(GetParent())->GetMinOutputSizePixel();
            static_cast<SfxDockingWindow*>(GetParent())->SetMinOutputSizePixel(Size(
                    aMinOutSizePixel.Width(), aSz.Height()));
            pFloat->SetOutputSizePixel(aSz);
            SvTreeListEntry* pFirst = m_aContentTree->FirstSelected();
            if(pFirst)
                m_aContentTree->Select(pFirst); // Enable toolbox
            m_pConfig->SetSmall( true );
            m_aContentToolBox->CheckItem(FN_SHOW_CONTENT_BOX, false);
        }
    }
}

void SwNavigationPI::Resize()
{
    vcl::Window* pParent = GetParent();
    if( !_IsZoomedIn() )
    {
        Size aNewSize (pParent->GetOutputSizePixel());

        SfxDockingWindow* pDockingParent = dynamic_cast<SfxDockingWindow*>(pParent);
        if (pDockingParent != nullptr)
        {
            FloatingWindow* pFloat =  pDockingParent->GetFloatingWindow();
            //change the minimum width depending on the dock status
            Size aMinOutSizePixel = pDockingParent->GetMinOutputSizePixel();
            if( pFloat)
            {
                aNewSize = pFloat->GetOutputSizePixel();
                aMinOutSizePixel.Width() = m_nWishWidth;
                aMinOutSizePixel.Height() = _IsZoomedIn() ? m_nZoomIn : m_nZoomOutInit;
            }
            else
            {
                aMinOutSizePixel.Width() = 0;
                aMinOutSizePixel.Height() = 0;
            }
            pDockingParent->SetMinOutputSizePixel(aMinOutSizePixel);
        }

        const Point aPos = m_aContentTree->GetPosPixel();
        Point aLBPos = m_aDocListBox->GetPosPixel();
        long nDist = aPos.X();
        aNewSize.Height() -= (aPos.Y() + aPos.X() + m_nDocLBIniHeight + nDist);
        aNewSize.Width() -= 2 * nDist;
        aLBPos.Y() = aPos.Y() + aNewSize.Height() + nDist;
        m_aDocListBox->Show(!m_aGlobalTree->IsVisible() && aLBPos.Y() > aPos.Y() );

        Size aDocLBSz = m_aDocListBox->GetSizePixel();
        aDocLBSz.Width() = aNewSize.Width();
        if(aNewSize.Height() < 0)
            aDocLBSz.Height() = 0;
        else
            aDocLBSz.Height() = m_nDocLBIniHeight;
        m_aContentTree->SetSizePixel(aNewSize);
        // GlobalTree starts on to the top and goes all the way down.
        aNewSize.Height() += (nDist + m_nDocLBIniHeight + aPos.Y() - m_aGlobalTree->GetPosPixel().Y());
        m_aGlobalTree->SetSizePixel(aNewSize);
        m_aDocListBox->setPosSizePixel( aLBPos.X(), aLBPos.Y(),
            aDocLBSz.Width(), aDocLBSz.Height(),
            PosSizeFlags::X|PosSizeFlags::Y|PosSizeFlags::Width);
    }
}

SwNavigationPI::SwNavigationPI( SfxBindings* _pBindings,
                                SfxChildWindowContext* pCw,
                                vcl::Window* pParent) :

    Window( pParent, SW_RES(DLG_NAVIGATION_PI)),
    SfxControllerItem( SID_DOCFULLNAME, *_pBindings ),

    m_aContentToolBox(VclPtr<SwNavHelpToolBox>::Create(this, SW_RES(TB_CONTENT))),
    m_aGlobalToolBox(VclPtr<SwHelpToolBox>::Create(this, SW_RES(TB_GLOBAL))),
    m_aContentImageList(SW_RES(IL_CONTENT)),
    m_aContentTree(VclPtr<SwContentTree>::Create(this, SW_RES(TL_CONTENT))),
    m_aGlobalTree(VclPtr<SwGlobalTree>::Create(this, SW_RES(TL_GLOBAL))),
    m_aDocListBox(VclPtr<ListBox>::Create(this, SW_RES(LB_DOCS))),

    m_pxObjectShell(nullptr),
    m_pContentView(nullptr),
    m_pContentWrtShell(nullptr),
    m_pActContView(nullptr),
    m_pCreateView(nullptr),
    m_pPopupWindow(nullptr),
    m_pFloatingWindow(nullptr),

    m_pContextWin(pCw),

    m_pConfig(SW_MOD()->GetNavigationConfig()),
    m_rBindings(*_pBindings),

    m_nWishWidth(0),
    m_nAutoMarkIdx(1),
    m_nRegionMode(RegionMode::NONE),

    m_bIsZoomedIn(false),
    m_bPageCtrlsVisible(false),
    m_bGlobalMode(false)
{
    GetCreateView();
    InitImageList();

    m_aContentToolBox->SetHelpId(HID_NAVIGATOR_TOOLBOX );
    m_aGlobalToolBox->SetHelpId(HID_NAVIGATOR_GLOBAL_TOOLBOX);
    m_aDocListBox->SetHelpId(HID_NAVIGATOR_LISTBOX );
    m_aDocListBox->SetDropDownLineCount(9);

    m_nDocLBIniHeight = m_aDocListBox->GetSizePixel().Height();
    m_nZoomOutInit = m_nZoomOut = Resource::ReadShortRes();

    // Insert the numeric field in the toolbox.
    VclPtr<NumEditAction> pEdit = VclPtr<NumEditAction>::Create(
                    m_aContentToolBox.get(), SW_RES(NF_PAGE ));
    pEdit->SetActionHdl(LINK(this, SwNavigationPI, EditAction));
    pEdit->SetGetFocusHdl(LINK(this, SwNavigationPI, EditGetFocus));
    pEdit->SetAccessibleName(pEdit->GetQuickHelpText());
    pEdit->SetUpHdl(LINK(this, SwNavigationPI, PageEditModifyHdl));
    pEdit->SetDownHdl(LINK(this, SwNavigationPI, PageEditModifyHdl));

    m_bPageCtrlsVisible = true;

    // Double separators are not allowed, so you have to
    // determine the suitable size differently.
    Rectangle aFirstRect = m_aContentToolBox->GetItemRect(FN_SELECT_FOOTNOTE);
    Rectangle aSecondRect = m_aContentToolBox->GetItemRect(FN_SELECT_HEADER);
    Size aItemWinSize( aFirstRect.Left() - aSecondRect.Left(),
                       aFirstRect.Bottom() - aFirstRect.Top() );
    Size aOptimalSize(pEdit->get_preferred_size());
    aItemWinSize.Width() = std::max(aItemWinSize.Width(), aOptimalSize.Width());
    pEdit->SetSizePixel(aItemWinSize);
    m_aContentToolBox->InsertSeparator(4);
    m_aContentToolBox->InsertWindow( FN_PAGENUMBER, pEdit, ToolBoxItemBits::NONE, 4);
    m_aContentToolBox->InsertSeparator(4);
    m_aContentToolBox->SetHelpId(FN_PAGENUMBER, HID_NAVI_TBX16);
    m_aContentToolBox->ShowItem( FN_PAGENUMBER );

    for( sal_uInt16 i = 0; i <= static_cast<sal_uInt16>(RegionMode::EMBEDDED); i++  )
    {
        m_aContextArr[i] = SW_RESSTR(STR_HYPERLINK + i);
        m_aStatusArr[i] = SW_RESSTR(STR_STATUS_FIRST + i);
    }
    m_aStatusArr[3] = SW_RESSTR(STR_ACTIVE_VIEW);
    FreeResource();

    const Size& rOutSize =  GetOutputSizePixel();

    m_nZoomIn = (short)rOutSize.Height();

    // Make sure the toolbox has a size that fits all its contents
    Size aContentToolboxSize( m_aContentToolBox->CalcWindowSizePixel() );
    m_aContentToolBox->SetOutputSizePixel( aContentToolboxSize );

    // position listbox below toolbar and add some space
    long nListboxYPos = m_aContentToolBox->GetPosPixel().Y() + aContentToolboxSize.Height() + 4;

    // The left and right margins around the toolboxes should be equal.
    m_nWishWidth = aContentToolboxSize.Width();
    m_nWishWidth += 2 * m_aContentToolBox->GetPosPixel().X();

    DockingWindow* pDockingParent = dynamic_cast<DockingWindow*>(pParent);
    if (pDockingParent != nullptr)
    {
        FloatingWindow* pFloat =  pDockingParent->GetFloatingWindow();
        Size aMinSize(pFloat ? m_nWishWidth : 0, pFloat ? m_nZoomOutInit : 0);
        pDockingParent->SetMinOutputSizePixel(aMinSize);
        SetOutputSizePixel( Size( m_nWishWidth, m_nZoomOutInit));

        SfxDockingWindow* pSfxDockingParent = dynamic_cast<SfxDockingWindow*>(pParent);
        if (pSfxDockingParent != nullptr)
        {
            Size aTmpParentSize(pSfxDockingParent->GetSizePixel());
            if (aTmpParentSize.Width() < aMinSize.Width()
                || aTmpParentSize.Height() < aMinSize.Height())
            {
                if (pSfxDockingParent->GetFloatingWindow()
                    && ! pSfxDockingParent->GetFloatingWindow()->IsRollUp())
                {
                    pSfxDockingParent->SetOutputSizePixel(aMinSize);
                }
            }
        }
    }

    m_aContentTree->setPosSizePixel( 0, nListboxYPos, 0, 0, PosSizeFlags::Y );
    m_aContentTree->SetStyle( m_aContentTree->GetStyle()|WB_HASBUTTONS|WB_HASBUTTONSATROOT|
                            WB_CLIPCHILDREN|WB_HSCROLL|WB_FORCE_MAKEVISIBLE );
    m_aContentTree->SetSpaceBetweenEntries(3);
    m_aContentTree->SetSelectionMode( SINGLE_SELECTION );
    m_aContentTree->SetDragDropMode( DragDropMode::CTRL_MOVE |
                                  DragDropMode::CTRL_COPY |
                                  DragDropMode::ENABLE_TOP );
    m_aContentTree->EnableAsyncDrag(true);
    m_aContentTree->ShowTree();
    m_aContentToolBox->CheckItem(FN_SHOW_CONTENT_BOX);

//  TreeListBox for global document
    m_aGlobalTree->setPosSizePixel( 0, nListboxYPos, 0, 0, PosSizeFlags::Y );
    m_aGlobalTree->SetSelectionMode( MULTIPLE_SELECTION );
    m_aGlobalTree->SetStyle( m_aGlobalTree->GetStyle()|WB_HASBUTTONS|WB_HASBUTTONSATROOT|
                                WB_CLIPCHILDREN|WB_HSCROLL );
    Size aGlblSize(m_aGlobalToolBox->CalcWindowSizePixel());
    m_aGlobalToolBox->SetSizePixel(aGlblSize);

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
    m_aGlobalToolBox->CheckItem(FN_GLOBAL_SWITCH);

    vcl::Font aFont(GetFont());
    aFont.SetWeight(WEIGHT_NORMAL);
    GetPageEdit().SetFont(aFont);
    aFont = m_aContentTree->GetFont();
    aFont.SetWeight(WEIGHT_NORMAL);
    m_aContentTree->SetFont(aFont);
    m_aGlobalTree->SetFont(aFont);

    StartListening(*SfxGetpApp());
    if ( m_pCreateView )
        StartListening(*m_pCreateView);
    SfxImageManager* pImgMan = SfxImageManager::GetImageManager(*SW_MOD());
    pImgMan->RegisterToolBox(m_aContentToolBox.get(), SfxToolboxFlags::CHANGEOUTSTYLE);
    pImgMan->RegisterToolBox(m_aGlobalToolBox.get(), SfxToolboxFlags::CHANGEOUTSTYLE);

    m_aContentToolBox->SetItemBits( FN_CREATE_NAVIGATION, m_aContentToolBox->GetItemBits( FN_CREATE_NAVIGATION ) | ToolBoxItemBits::DROPDOWNONLY );
    m_aContentToolBox->SetItemBits( FN_DROP_REGION, m_aContentToolBox->GetItemBits( FN_DROP_REGION ) | ToolBoxItemBits::DROPDOWNONLY );
    m_aContentToolBox->SetItemBits( FN_OUTLINE_LEVEL, m_aContentToolBox->GetItemBits( FN_OUTLINE_LEVEL ) | ToolBoxItemBits::DROPDOWNONLY );

    if(IsGlobalDoc())
    {
        SwView *pActView = GetCreateView();
        m_aGlobalToolBox->CheckItem(FN_GLOBAL_SAVE_CONTENT,
                    pActView->GetWrtShellPtr()->IsGlblDocSaveLinks());
        if(m_pConfig->IsGlobalActive())
            ToggleTree();
        m_aGlobalTree->GrabFocus();
    }
    else
        m_aContentTree->GrabFocus();
    UsePage(nullptr);
    m_aPageChgIdle.SetIdleHdl(LINK(this, SwNavigationPI, ChangePageHdl));
    m_aPageChgIdle.SetPriority(SchedulerPriority::LOWEST);

    m_aContentTree->SetAccessibleName(SW_RESSTR(STR_ACCESS_TL_CONTENT));
    m_aGlobalTree->SetAccessibleName(SW_RESSTR(STR_ACCESS_TL_GLOBAL));
    m_aDocListBox->SetAccessibleName(m_aStatusArr[3]);

    if (m_pContextWin == nullptr)
    {
        // When the context window is missing then the navigator is
        // displayed in the sidebar.  While the navigator could change
        // its size, the sidebar can not, and the navigator would just
        // waste space.  Therefore hide this button.
        m_aContentToolBox->RemoveItem(m_aContentToolBox->GetItemPos(FN_SHOW_CONTENT_BOX));
    }
}

SwNavigationPI::~SwNavigationPI()
{
    disposeOnce();
}

void SwNavigationPI::dispose()
{
    if(IsGlobalDoc() && !IsGlobalMode())
    {
        SwView *pView = GetCreateView();
        SwWrtShell &rSh = pView->GetWrtShell();
        if( !rSh.IsAllProtect() )
            pView->GetDocShell()->SetReadOnlyUI(false);
    }

    EndListening(*SfxGetpApp());

    SfxImageManager* pImgMan = SfxImageManager::GetImageManager(*SW_MOD());
    pImgMan->ReleaseToolBox(m_aContentToolBox.get());
    pImgMan->ReleaseToolBox(m_aGlobalToolBox.get());
    m_aContentToolBox->GetItemWindow(FN_PAGENUMBER)->disposeOnce();
    m_aContentToolBox->Clear();
    if(m_pxObjectShell)
    {
        if(m_pxObjectShell->Is())
            (*m_pxObjectShell)->DoClose();
        delete m_pxObjectShell;
    }

    if ( IsBound() )
        m_rBindings.Release(*this);

    m_pFloatingWindow.disposeAndClear();
    m_pPopupWindow.disposeAndClear();
    m_aDocListBox.disposeAndClear();
    m_aGlobalTree.disposeAndClear();
    m_aContentTree.disposeAndClear();
    m_aGlobalToolBox.disposeAndClear();
    m_aContentToolBox.disposeAndClear();

    m_aPageChgIdle.Stop();

    ::SfxControllerItem::dispose();

    vcl::Window::dispose();
}

void SwNavigationPI::SetPopupWindow( SfxPopupWindow* pWindow )
{
    m_pPopupWindow = pWindow;
    m_pPopupWindow->SetPopupModeEndHdl( LINK( this, SwNavigationPI, PopupModeEndHdl ));
    m_pPopupWindow->SetDeleteLink_Impl( LINK( this, SwNavigationPI, ClosePopupWindow ));
}

IMPL_LINK_NOARG_TYPED(SwNavigationPI, PopupModeEndHdl, FloatingWindow*, void)
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

IMPL_LINK_TYPED( SwNavigationPI, ClosePopupWindow, SfxPopupWindow *, pWindow, void )
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
            m_aContentToolBox->EnableItem(FN_GLOBAL_SWITCH, bGlobal);
            if( (!bGlobal && IsGlobalMode()) ||
                    (!IsGlobalMode() && m_pConfig->IsGlobalActive()) )
            {
                ToggleTree();
            }
            if(bGlobal)
            {
                m_aGlobalToolBox->CheckItem(FN_GLOBAL_SAVE_CONTENT, pWrtShell->IsGlblDocSaveLinks());
            }
        }
        else
        {
            m_aContentTree->SetActiveShell(nullptr);
        }
        UpdateListBox();
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
        if(dynamic_cast<const SfxSimpleHint*>(&rHint) && static_cast<const SfxSimpleHint&>(rHint).GetId() == SFX_HINT_DYING)
        {
            m_pCreateView = nullptr;
        }
    }
    else
    {
        if(dynamic_cast<const SfxEventHint*>(&rHint))
        {
            if( m_pxObjectShell &&
                        static_cast<const SfxEventHint&>( rHint).GetEventId() == SFX_EVENT_CLOSEAPP)
            {
                DELETEZ(m_pxObjectShell);
            }
            else if(static_cast<const SfxEventHint&>( rHint).GetEventId() == SFX_EVENT_OPENDOC)
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

IMPL_LINK_TYPED( SwNavigationPI, MenuSelectHdl, Menu *, pMenu, bool )
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
                sEntry += m_aStatusArr[STR_ACTIVE - STR_STATUS_FIRST];
            }
            else
                sEntry += m_aStatusArr[STR_INACTIVE - STR_STATUS_FIRST];
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
        sEntry += m_aStatusArr[STR_HIDDEN - STR_STATUS_FIRST];
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

IMPL_LINK_TYPED(SwNavigationPI, DoneLink, SfxPoolItem *, pItem, void)
{
    const SfxViewFrameItem* pFrameItem = dynamic_cast<SfxViewFrameItem*>( pItem  );
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
            m_pxObjectShell = new SfxObjectShellLock(pFrame->GetObjectShell());
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
        INetBookmark aBkmk( aEmptyOUStr, aEmptyOUStr );
        if (rData.GetINetBookmark(nFormat, aBkmk))
            sFileName = aBkmk.GetURL();
    }
    if( !sFileName.isEmpty() )
    {
        sFileName = INetURLObject( sFileName ).GetMainURL( INetURLObject::NO_DECODE );
    }
    return sFileName;
}

sal_Int8 SwNavigationPI::AcceptDrop( const AcceptDropEvent& /*rEvt*/ )
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

    if ((m_sContentFileName.isEmpty() || m_sContentFileName != sFileName))
    {
        nRet = rEvt.mnAction;
        sFileName = comphelper::string::stripEnd(sFileName, 0);
        m_sContentFileName = sFileName;
        if(m_pxObjectShell)
        {
            m_aContentTree->SetHiddenShell( nullptr );
            (*m_pxObjectShell)->DoClose();
            DELETEZ( m_pxObjectShell);
        }
        SfxStringItem aFileItem(SID_FILE_NAME, sFileName );
        SfxStringItem aOptionsItem( SID_OPTIONS, OUString("HRC") );
        SfxLinkItem aLink( SID_DONELINK,
                            LINK( this, SwNavigationPI, DoneLink ) );
        GetActiveView()->GetViewFrame()->GetDispatcher()->Execute(
                    SID_OPENDOC, SfxCallMode::ASYNCHRON,
                    &aFileItem, &aOptionsItem, &aLink, 0L );
    }
    return nRet;
}

void SwNavigationPI::SetRegionDropMode(RegionMode nNewMode)
{
    m_nRegionMode = nNewMode;
    m_pConfig->SetRegionMode( m_nRegionMode );

    sal_uInt16 nDropId = FN_DROP_REGION;
    if(m_nRegionMode == RegionMode::LINK)
        nDropId = FN_DROP_REGION_LINK;
    else if(m_nRegionMode == RegionMode::EMBEDDED)
        nDropId = FN_DROP_REGION_COPY;

    ImageList& rImgLst = m_aContentImageList;

    m_aContentToolBox->SetItemImage( FN_DROP_REGION, rImgLst.GetImage(nDropId));
}

bool    SwNavigationPI::ToggleTree()
{
    bool bRet = true;
    bool bGlobalDoc = IsGlobalDoc();
    if(!IsGlobalMode() && bGlobalDoc)
    {
        SetUpdateMode(false);
        if(_IsZoomedIn())
            _ZoomOut();
        m_aGlobalTree->ShowTree();
        m_aGlobalToolBox->Show();
        m_aContentTree->HideTree();
        m_aContentToolBox->Hide();
        m_aDocListBox->Hide();
        SetGlobalMode(true);
        SetUpdateMode(true);
    }
    else
    {
        m_aGlobalTree->HideTree();
        m_aGlobalToolBox->Hide();
        if(!_IsZoomedIn())
        {
            m_aContentTree->ShowTree();
            m_aContentToolBox->Show();
            m_aDocListBox->Show();
        }
        bRet = false;
        SetGlobalMode(false);
    }
    return bRet;
}

bool    SwNavigationPI::IsGlobalDoc() const
{
    bool bRet = false;
    SwView *pView = GetCreateView();
    if(pView)
    {
        SwWrtShell &rSh = pView->GetWrtShell();
        bRet = rSh.IsGlobalDoc();
    }
    return bRet;
}

IMPL_LINK_NOARG_TYPED(SwNavigationPI, ChangePageHdl, Idle *, void)
{
    if (!IsDisposed())
    {
        EditAction(GetPageEdit());
        GetPageEdit().GrabFocus();
    }
}

IMPL_LINK_NOARG_TYPED(SwNavigationPI, PageEditModifyHdl, SpinField&, void)
{
    if(m_aPageChgIdle.IsActive())
        m_aPageChgIdle.Stop();
    m_aPageChgIdle.Start();
}

SwView*  SwNavigationPI::GetCreateView() const
{
    if(!m_pCreateView)
    {
        SwView* pView = SwModule::GetFirstView();
        while(pView)
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
                        SfxBindings* _pBindings,
                        SfxChildWinInfo* )
    : SfxChildWindowContext( nId )
{
    VclPtr<SwNavigationPI> pNavi = VclPtr<SwNavigationPI>::Create( _pBindings, this, pParent );
    SetWindow( pNavi );
    _pBindings->Invalidate(SID_NAVIGATOR);

    SwNavigationConfig* pNaviConfig = SW_MOD()->GetNavigationConfig();

    const ContentTypeId nRootType = pNaviConfig->GetRootType();
    if( nRootType != ContentTypeId::UNKNOWN )
    {
        pNavi->m_aContentTree->SetRootType(nRootType);
        pNavi->m_aContentToolBox->CheckItem(FN_SHOW_ROOT);
    }
    pNavi->m_aContentTree->SetOutlineLevel( static_cast< sal_uInt8 >( pNaviConfig->GetOutlineLevel() ) );
    pNavi->SetRegionDropMode( pNaviConfig->GetRegionMode() );

    if(GetFloatingWindow() && pNaviConfig->IsSmall())
    {
        pNavi->_ZoomIn();
    }
}

void SwNavigationPI::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );
    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        InitImageList();
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
        Color aBgColor = rStyleSettings.GetFaceColor();
        Wallpaper aBack( aBgColor );
        SetBackground( aBack );
    }
}

void SwNavigationPI::InitImageList()
{
    ImageList& rImgLst = m_aContentImageList;
    for( sal_uInt16 k = 0; k < m_aContentToolBox->GetItemCount(); k++)
            m_aContentToolBox->SetItemImage(m_aContentToolBox->GetItemId(k),
                    rImgLst.GetImage(m_aContentToolBox->GetItemId(k)));

    for( sal_uInt16 k = 0; k < m_aGlobalToolBox->GetItemCount(); k++)
            m_aGlobalToolBox->SetItemImage(m_aGlobalToolBox->GetItemId(k),
                    rImgLst.GetImage(m_aGlobalToolBox->GetItemId(k)));

    sal_uInt16 nDropId = FN_DROP_REGION;
    if(m_nRegionMode == RegionMode::LINK)
        nDropId = FN_DROP_REGION_LINK;
    else if(m_nRegionMode == RegionMode::EMBEDDED)
        nDropId = FN_DROP_REGION_COPY;
    m_aContentToolBox->SetItemImage( FN_DROP_REGION,
                                    rImgLst.GetImage(nDropId));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
