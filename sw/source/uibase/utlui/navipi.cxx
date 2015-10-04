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
            if(!rSh.IsCrsrPtAtEnd())
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

static void lcl_UnSelectFrm(SwWrtShell *pSh)
{
    if (pSh->IsFrmSelected())
    {
        pSh->UnSelectFrm();
        pSh->LeaveSelFrmMode();
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
            aContentTree->ShowHiddenShell():
                aContentTree->ShowActualView();

    }
    else
    {
        aContentTree->SetConstantShell(pView->GetWrtShellPtr());
    }
}

// Filling of the list box for outline view or documents
// The PI will be set to full size

void SwNavigationPI::FillBox()
{
    if(pContentWrtShell)
    {
        aContentTree->SetHiddenShell( pContentWrtShell );
        aContentTree->Display(  false );
    }
    else
    {
        SwView *pView = GetCreateView();
        if(!pView)
        {
            aContentTree->SetActiveShell(0);
        }
        else if( pView != pActContView)
        {
            SwWrtShell* pWrtShell = pView->GetWrtShellPtr();
            aContentTree->SetActiveShell(pWrtShell);
        }
        else
            aContentTree->Display( true );
        pActContView = pView;
    }
}

void SwNavigationPI::UsePage(SwWrtShell *pSh)
{
    if (!pSh)
    {
        SwView *pView = GetCreateView();
        pSh = pView ? &pView->GetWrtShell() : 0;
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
            Application::PostUserEvent( LINK(pView, SwView, MoveNavigationHdl), pbNext, true );
        }
        break;
        case FN_SHOW_ROOT:
        {
            aContentTree->ToggleToRoot();
        }
        break;
        case FN_SHOW_CONTENT_BOX:
        case FN_SELECT_CONTENT:
            if(pContextWin!=NULL && pContextWin->GetFloatingWindow()!=NULL)
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
            rSh.MoveCrsr();
            const FrmTypeFlags eType = rSh.GetFrmType(0,false);
            if (eType & FrmTypeFlags::FOOTER)
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
            rSh.MoveCrsr();
            const FrmTypeFlags eType = rSh.GetFrmType(0,false);
            if (eType & FrmTypeFlags::HEADER)
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
            rSh.MoveCrsr();
            const FrmTypeFlags eFrmType = rSh.GetFrmType(0,false);
                // Jump from the footnote to the anchor.
            if (eFrmType & FrmTypeFlags::FOOTNOTE)
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
                aGlobalTree->ExecCommand(nCurrItemId);
            else
                aContentTree->ExecCommand(nCurrItemId, bOutlineWithChildren);
        }
        break;
        case FN_GLOBAL_SWITCH:
        {
            ToggleTree();
            pConfig->SetGlobalActive(IsGlobalMode());
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
        lcl_UnSelectFrm(&rSh);
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
            aGlobalTree->TbxMenuHdl(nCurrItemId, pBox);
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
                pMenu->InsertItem( i + 1, aContextArr[i] );
                pMenu->SetHelpId(i + 1, aHIDs[i]);
            }
            pMenu->CheckItem( static_cast<int>(nRegionMode) + 1 );
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
            pMenu->CheckItem( aContentTree->GetOutlineLevel() + 100 );
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
    aT1 = pPopup->GetParent()->OutputToScreenPixel(pPopup->GetParent()->AbsoluteScreenToOutputPixel(aContentToolBox->OutputToAbsoluteScreenPixel(aT1)));
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
        if(aPageChgIdle.IsActive())
            aPageChgIdle.Stop();
        pCreateView->GetWrtShell().GotoPage((sal_uInt16)rEdit.GetValue(), true);
        pCreateView->GetEditWin().GrabFocus();
        pCreateView->GetViewFrame()->GetBindings().Invalidate(FN_STAT_PAGE);
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
        pMarkAccess->deleteMark(pMarkAccess->findMark(vNavMarkNames[nAutoMarkIdx]));

    rSh.SetBookmark(vcl::KeyCode(), OUString(), OUString(), IDocumentMarkAccess::MarkType::NAVIGATOR_REMINDER);
    SwView::SetActMark( nAutoMarkIdx );

    if(++nAutoMarkIdx == MAX_MARKS)
        nAutoMarkIdx = 0;
}

void SwNavigationPI::GotoPage()
{
    if (pContextWin && pContextWin->GetFloatingWindow() && pContextWin->GetFloatingWindow()->IsRollUp())
        _ZoomIn();
    if(IsGlobalMode())
        ToggleTree();
    UsePage(0);
    GetPageEdit().GrabFocus();
}

void SwNavigationPI::_ZoomOut()
{
    if (_IsZoomedIn())
    {
        FloatingWindow* pFloat = pContextWin!=NULL ? pContextWin->GetFloatingWindow() : NULL;
        bIsZoomedIn = false;
        Size aSz(GetOutputSizePixel());
        aSz.Height() = nZoomOut;
        Size aMinOutSizePixel = static_cast<SfxDockingWindow*>(GetParent())->GetMinOutputSizePixel();
        static_cast<SfxDockingWindow*>(GetParent())->SetMinOutputSizePixel(Size(
                            aMinOutSizePixel.Width(),nZoomOutInit));
        if (pFloat != NULL)
            pFloat->SetOutputSizePixel(aSz);
        FillBox();
        if(IsGlobalMode())
        {
            aGlobalTree->ShowTree();
        }
        else
        {
            aContentTree->ShowTree();
            aDocListBox->Show();
        }
        SvTreeListEntry* pFirst = aContentTree->FirstSelected();
        if(pFirst)
            aContentTree->Select(pFirst); // Enable toolbox
        pConfig->SetSmall( false );
        aContentToolBox->CheckItem(FN_SHOW_CONTENT_BOX);
    }
}

void SwNavigationPI::_ZoomIn()
{
    if (pContextWin != NULL)
    {
        FloatingWindow* pFloat = pContextWin->GetFloatingWindow();
        if (pFloat &&
            (!_IsZoomedIn() || ( pContextWin->GetFloatingWindow()->IsRollUp())))
        {
            aContentTree->HideTree();
            aDocListBox->Hide();
            aGlobalTree->HideTree();
            bIsZoomedIn = true;
            Size aSz(GetOutputSizePixel());
            if( aSz.Height() > nZoomIn )
                nZoomOut = ( short ) aSz.Height();

            aSz.Height() = nZoomIn;
            Size aMinOutSizePixel = static_cast<SfxDockingWindow*>(GetParent())->GetMinOutputSizePixel();
            static_cast<SfxDockingWindow*>(GetParent())->SetMinOutputSizePixel(Size(
                    aMinOutSizePixel.Width(), aSz.Height()));
            pFloat->SetOutputSizePixel(aSz);
            SvTreeListEntry* pFirst = aContentTree->FirstSelected();
            if(pFirst)
                aContentTree->Select(pFirst); // Enable toolbox
            pConfig->SetSmall( true );
            aContentToolBox->CheckItem(FN_SHOW_CONTENT_BOX, false);
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
        if (pDockingParent != NULL)
        {
            FloatingWindow* pFloat =  pDockingParent->GetFloatingWindow();
            //change the minimum width depending on the dock status
            Size aMinOutSizePixel = pDockingParent->GetMinOutputSizePixel();
            if( pFloat)
            {
                aNewSize = pFloat->GetOutputSizePixel();
                aMinOutSizePixel.Width() = nWishWidth;
                aMinOutSizePixel.Height() = _IsZoomedIn() ? nZoomIn : nZoomOutInit;
            }
            else
            {
                aMinOutSizePixel.Width() = 0;
                aMinOutSizePixel.Height() = 0;
            }
            pDockingParent->SetMinOutputSizePixel(aMinOutSizePixel);
        }

        const Point aPos = aContentTree->GetPosPixel();
        Point aLBPos = aDocListBox->GetPosPixel();
        long nDist = aPos.X();
        aNewSize.Height() -= (aPos.Y() + aPos.X() + nDocLBIniHeight + nDist);
        aNewSize.Width() -= 2 * nDist;
        aLBPos.Y() = aPos.Y() + aNewSize.Height() + nDist;
        aDocListBox->Show(!aGlobalTree->IsVisible() && aLBPos.Y() > aPos.Y() );

        Size aDocLBSz = aDocListBox->GetSizePixel();
        aDocLBSz.Width() = aNewSize.Width();
        if(aNewSize.Height() < 0)
            aDocLBSz.Height() = 0;
        else
            aDocLBSz.Height() = nDocLBIniHeight;
        aContentTree->SetSizePixel(aNewSize);
        // GlobalTree starts on to the top and goes all the way down.
        aNewSize.Height() += (nDist + nDocLBIniHeight + aPos.Y() - aGlobalTree->GetPosPixel().Y());
        aGlobalTree->SetSizePixel(aNewSize);
        aDocListBox->setPosSizePixel( aLBPos.X(), aLBPos.Y(),
            aDocLBSz.Width(), aDocLBSz.Height(),
            PosSizeFlags::X|PosSizeFlags::Y|PosSizeFlags::Width);
    }
}

SwNavigationPI::SwNavigationPI( SfxBindings* _pBindings,
                                SfxChildWindowContext* pCw,
                                vcl::Window* pParent) :

    Window( pParent, SW_RES(DLG_NAVIGATION_PI)),
    SfxControllerItem( SID_DOCFULLNAME, *_pBindings ),

    aContentToolBox(VclPtr<SwNavHelpToolBox>::Create(this, SW_RES(TB_CONTENT))),
    aGlobalToolBox(VclPtr<SwHelpToolBox>::Create(this, SW_RES(TB_GLOBAL))),
    aContentImageList(SW_RES(IL_CONTENT)),
    aContentTree(VclPtr<SwContentTree>::Create(this, SW_RES(TL_CONTENT))),
    aGlobalTree(VclPtr<SwGlobalTree>::Create(this, SW_RES(TL_GLOBAL))),
    aDocListBox(VclPtr<ListBox>::Create(this, SW_RES(LB_DOCS))),

    pxObjectShell(0),
    pContentView(0),
    pContentWrtShell(0),
    pActContView(0),
    pCreateView(0),
    pPopupWindow(0),
    pFloatingWindow(0),

    pContextWin(pCw),

    pConfig(SW_MOD()->GetNavigationConfig()),
    rBindings(*_pBindings),

    nWishWidth(0),
    nAutoMarkIdx(1),
    nRegionMode(RegionMode::NONE),

    bSmallMode(false),
    bIsZoomedIn(false),
    bPageCtrlsVisible(false),
    bGlobalMode(false)
{
    GetCreateView();
    InitImageList();

    aContentToolBox->SetHelpId(HID_NAVIGATOR_TOOLBOX );
    aGlobalToolBox->SetHelpId(HID_NAVIGATOR_GLOBAL_TOOLBOX);
    aDocListBox->SetHelpId(HID_NAVIGATOR_LISTBOX );
    aDocListBox->SetDropDownLineCount(9);

    nDocLBIniHeight = aDocListBox->GetSizePixel().Height();
    nZoomOutInit = nZoomOut = Resource::ReadShortRes();

    // Insert the numeric field in the toolbox.
    VclPtr<NumEditAction> pEdit = VclPtr<NumEditAction>::Create(
                    aContentToolBox.get(), SW_RES(NF_PAGE ));
    pEdit->SetActionHdl(LINK(this, SwNavigationPI, EditAction));
    pEdit->SetGetFocusHdl(LINK(this, SwNavigationPI, EditGetFocus));
    pEdit->SetAccessibleName(pEdit->GetQuickHelpText());
    pEdit->SetUpHdl(LINK(this, SwNavigationPI, PageEditModifyHdl));
    pEdit->SetDownHdl(LINK(this, SwNavigationPI, PageEditModifyHdl));

    bPageCtrlsVisible = true;

    // Double separators are not allowed, so you have to
    // determine the suitable size differently.
    Rectangle aFirstRect = aContentToolBox->GetItemRect(FN_SELECT_FOOTNOTE);
    Rectangle aSecondRect = aContentToolBox->GetItemRect(FN_SELECT_HEADER);
    Size aItemWinSize( aFirstRect.Left() - aSecondRect.Left(),
                       aFirstRect.Bottom() - aFirstRect.Top() );
    Size aOptimalSize(pEdit->get_preferred_size());
    aItemWinSize.Width() = std::max(aItemWinSize.Width(), aOptimalSize.Width());
    pEdit->SetSizePixel(aItemWinSize);
    aContentToolBox->InsertSeparator(4);
    aContentToolBox->InsertWindow( FN_PAGENUMBER, pEdit, ToolBoxItemBits::NONE, 4);
    aContentToolBox->InsertSeparator(4);
    aContentToolBox->SetHelpId(FN_PAGENUMBER, HID_NAVI_TBX16);
    aContentToolBox->ShowItem( FN_PAGENUMBER );

    for( sal_uInt16 i = 0; i <= static_cast<sal_uInt16>(RegionMode::EMBEDDED); i++  )
    {
        aContextArr[i] = SW_RESSTR(STR_HYPERLINK + i);
        aStatusArr[i] = SW_RESSTR(STR_STATUS_FIRST + i);
    }
    aStatusArr[3] = SW_RESSTR(STR_ACTIVE_VIEW);
    FreeResource();

    const Size& rOutSize =  GetOutputSizePixel();

    nZoomIn = (short)rOutSize.Height();

    // Make sure the toolbox has a size that fits all its contents
    Size aContentToolboxSize( aContentToolBox->CalcWindowSizePixel() );
    aContentToolBox->SetOutputSizePixel( aContentToolboxSize );

    // position listbox below toolbar and add some space
    long nListboxYPos = aContentToolBox->GetPosPixel().Y() + aContentToolboxSize.Height() + 4;

    // The left and right margins around the toolboxes should be equal.
    nWishWidth = aContentToolboxSize.Width();
    nWishWidth += 2 * aContentToolBox->GetPosPixel().X();

    DockingWindow* pDockingParent = dynamic_cast<DockingWindow*>(pParent);
    if (pDockingParent != NULL)
    {
        FloatingWindow* pFloat =  pDockingParent->GetFloatingWindow();
        Size aMinSize(pFloat ? nWishWidth : 0, pFloat ? nZoomOutInit : 0);
        pDockingParent->SetMinOutputSizePixel(aMinSize);
        SetOutputSizePixel( Size( nWishWidth, nZoomOutInit));

        SfxDockingWindow* pSfxDockingParent = dynamic_cast<SfxDockingWindow*>(pParent);
        if (pSfxDockingParent != NULL)
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

    aContentTree->setPosSizePixel( 0, nListboxYPos, 0, 0, PosSizeFlags::Y );
    aContentTree->SetStyle( aContentTree->GetStyle()|WB_HASBUTTONS|WB_HASBUTTONSATROOT|
                            WB_CLIPCHILDREN|WB_HSCROLL|WB_FORCE_MAKEVISIBLE );
    aContentTree->SetSpaceBetweenEntries(3);
    aContentTree->SetSelectionMode( SINGLE_SELECTION );
    aContentTree->SetDragDropMode( DragDropMode::CTRL_MOVE |
                                  DragDropMode::CTRL_COPY |
                                  DragDropMode::ENABLE_TOP );
    aContentTree->EnableAsyncDrag(true);
    aContentTree->ShowTree();
    aContentToolBox->CheckItem(FN_SHOW_CONTENT_BOX);

//  TreeListBox for global document
    aGlobalTree->setPosSizePixel( 0, nListboxYPos, 0, 0, PosSizeFlags::Y );
    aGlobalTree->SetSelectionMode( MULTIPLE_SELECTION );
    aGlobalTree->SetStyle( aGlobalTree->GetStyle()|WB_HASBUTTONS|WB_HASBUTTONSATROOT|
                                WB_CLIPCHILDREN|WB_HSCROLL );
    Size aGlblSize(aGlobalToolBox->CalcWindowSizePixel());
    aGlobalToolBox->SetSizePixel(aGlblSize);

//  Handler

    Link<ToolBox *, void> aLk = LINK(this, SwNavigationPI, ToolBoxSelectHdl);
    aContentToolBox->SetSelectHdl( aLk );
    aGlobalToolBox->SetSelectHdl( aLk );
    aDocListBox->SetSelectHdl(LINK(this, SwNavigationPI,
                                                    DocListBoxSelectHdl));
    aContentToolBox->SetClickHdl( LINK(this, SwNavigationPI, ToolBoxClickHdl) );
    aContentToolBox->SetDropdownClickHdl( LINK(this, SwNavigationPI, ToolBoxDropdownClickHdl) );
    aGlobalToolBox->SetClickHdl( LINK(this, SwNavigationPI, ToolBoxClickHdl) );
    aGlobalToolBox->SetDropdownClickHdl( LINK(this, SwNavigationPI, ToolBoxDropdownClickHdl) );
    aGlobalToolBox->CheckItem(FN_GLOBAL_SWITCH);

    vcl::Font aFont(GetFont());
    aFont.SetWeight(WEIGHT_NORMAL);
    GetPageEdit().SetFont(aFont);
    aFont = aContentTree->GetFont();
    aFont.SetWeight(WEIGHT_NORMAL);
    aContentTree->SetFont(aFont);
    aGlobalTree->SetFont(aFont);

    StartListening(*SfxGetpApp());
    if ( pCreateView )
        StartListening(*pCreateView);
    SfxImageManager* pImgMan = SfxImageManager::GetImageManager(*SW_MOD());
    pImgMan->RegisterToolBox(aContentToolBox.get(), SfxToolboxFlags::CHANGEOUTSTYLE);
    pImgMan->RegisterToolBox(aGlobalToolBox.get(), SfxToolboxFlags::CHANGEOUTSTYLE);

    aContentToolBox->SetItemBits( FN_CREATE_NAVIGATION, aContentToolBox->GetItemBits( FN_CREATE_NAVIGATION ) | ToolBoxItemBits::DROPDOWNONLY );
    aContentToolBox->SetItemBits( FN_DROP_REGION, aContentToolBox->GetItemBits( FN_DROP_REGION ) | ToolBoxItemBits::DROPDOWNONLY );
    aContentToolBox->SetItemBits( FN_OUTLINE_LEVEL, aContentToolBox->GetItemBits( FN_OUTLINE_LEVEL ) | ToolBoxItemBits::DROPDOWNONLY );

    if(IsGlobalDoc())
    {
        SwView *pActView = GetCreateView();
        aGlobalToolBox->CheckItem(FN_GLOBAL_SAVE_CONTENT,
                    pActView->GetWrtShellPtr()->IsGlblDocSaveLinks());
        if(pConfig->IsGlobalActive())
            ToggleTree();
        aGlobalTree->GrabFocus();
    }
    else
        aContentTree->GrabFocus();
    UsePage(0);
    aPageChgIdle.SetIdleHdl(LINK(this, SwNavigationPI, ChangePageHdl));
    aPageChgIdle.SetPriority(SchedulerPriority::LOWEST);

    aContentTree->SetAccessibleName(SW_RESSTR(STR_ACCESS_TL_CONTENT));
    aGlobalTree->SetAccessibleName(SW_RESSTR(STR_ACCESS_TL_GLOBAL));
    aDocListBox->SetAccessibleName(aStatusArr[3]);

    if (pContextWin == NULL)
    {
        // When the context window is missing then the navigator is
        // displayed in the sidebar.  While the navigator could change
        // its size, the sidebar can not, and the navigator would just
        // waste space.  Therefore hide this button.
        aContentToolBox->RemoveItem(aContentToolBox->GetItemPos(FN_SHOW_CONTENT_BOX));
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
    pImgMan->ReleaseToolBox(aContentToolBox.get());
    pImgMan->ReleaseToolBox(aGlobalToolBox.get());
    aContentToolBox->GetItemWindow(FN_PAGENUMBER)->disposeOnce();
    aContentToolBox->Clear();
    if(pxObjectShell)
    {
        if(pxObjectShell->Is())
            (*pxObjectShell)->DoClose();
        delete pxObjectShell;
    }

    if ( IsBound() )
        rBindings.Release(*this);

    pFloatingWindow.disposeAndClear();
    pPopupWindow.disposeAndClear();
    aDocListBox.disposeAndClear();
    aGlobalTree.disposeAndClear();
    aContentTree.disposeAndClear();
    aGlobalToolBox.disposeAndClear();
    aContentToolBox.disposeAndClear();

    aPageChgIdle.Stop();

    ::SfxControllerItem::dispose();

    vcl::Window::dispose();
}

void SwNavigationPI::SetPopupWindow( SfxPopupWindow* pWindow )
{
    pPopupWindow = pWindow;
    pPopupWindow->SetPopupModeEndHdl( LINK( this, SwNavigationPI, PopupModeEndHdl ));
    pPopupWindow->SetDeleteLink_Impl( LINK( this, SwNavigationPI, ClosePopupWindow ));
}

IMPL_LINK_NOARG_TYPED(SwNavigationPI, PopupModeEndHdl, FloatingWindow*, void)
{
    if ( pPopupWindow->IsVisible() )
    {
        // Replace floating window with popup window and destroy
        // floating window instance.
        pFloatingWindow.disposeAndClear();
        pFloatingWindow = pPopupWindow;
        pPopupWindow    = 0;
    }
    else
    {
        // Popup window has been closed by the user. No replacement, instance
        // will destroy itself.
        pPopupWindow = 0;
    }
}

IMPL_LINK_TYPED( SwNavigationPI, ClosePopupWindow, SfxPopupWindow *, pWindow, void )
{
    if ( pWindow == pFloatingWindow )
        pFloatingWindow = 0;
    else
        pPopupWindow = 0;
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
            aContentTree->SetActiveShell(pWrtShell);
            bool bGlobal = IsGlobalDoc();
            aContentToolBox->EnableItem(FN_GLOBAL_SWITCH, bGlobal);
            if( (!bGlobal && IsGlobalMode()) ||
                    (!IsGlobalMode() && pConfig->IsGlobalActive()) )
            {
                ToggleTree();
            }
            if(bGlobal)
            {
                aGlobalToolBox->CheckItem(FN_GLOBAL_SAVE_CONTENT, pWrtShell->IsGlblDocSaveLinks());
            }
        }
        else
        {
            aContentTree->SetActiveShell(0);
        }
        UpdateListBox();
    }
}

// Get the numeric field from the toolbox.

NumEditAction& SwNavigationPI::GetPageEdit()
{
    return *static_cast<NumEditAction*>(aContentToolBox->GetItemWindow(FN_PAGENUMBER));
}

// Notification on modified DocInfo
void SwNavigationPI::Notify( SfxBroadcaster& rBrdc, const SfxHint& rHint )
{
    if(&rBrdc == pCreateView)
    {
        if(dynamic_cast<const SfxSimpleHint*>(&rHint) && static_cast<const SfxSimpleHint&>(rHint).GetId() == SFX_HINT_DYING)
        {
            pCreateView = 0;
        }
    }
    else
    {
        if(dynamic_cast<const SfxEventHint*>(&rHint))
        {
            if( pxObjectShell &&
                        static_cast<const SfxEventHint&>( rHint).GetEventId() == SFX_EVENT_CLOSEAPP)
            {
                DELETEZ(pxObjectShell);
            }
            else if(static_cast<const SfxEventHint&>( rHint).GetEventId() == SFX_EVENT_OPENDOC)
            {

                SwView *pActView = GetCreateView();
                if(pActView)
                {
                    SwWrtShell* pWrtShell = pActView->GetWrtShellPtr();
                    aContentTree->SetActiveShell(pWrtShell);
                    if(aGlobalTree->IsVisible())
                    {
                        if(aGlobalTree->Update( false ))
                            aGlobalTree->Display();
                        else
                        // If no update is needed, then paint at least,
                        // because of the red entries for the broken links.
                            aGlobalTree->Invalidate();
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
            aContentTree->SetOutlineLevel( static_cast< sal_uInt8 >(nMenuId - 100) );
    }
    return false;
}

void SwNavigationPI::UpdateListBox()
{
    aDocListBox->SetUpdateMode(false);
    aDocListBox->Clear();
    SwView *pActView = GetCreateView();
    bool bDisable = pActView == 0;
    SwView *pView = SwModule::GetFirstView();
    sal_Int32 nCount = 0;
    sal_Int32 nAct = 0;
    sal_Int32 nConstPos = 0;
    const SwView* pConstView = aContentTree->IsConstantView() &&
                                aContentTree->GetActiveWrtShell() ?
                                    &aContentTree->GetActiveWrtShell()->GetView():
                                        0;
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
                sEntry += aStatusArr[STR_ACTIVE - STR_STATUS_FIRST];
            }
            else
                sEntry += aStatusArr[STR_INACTIVE - STR_STATUS_FIRST];
            sEntry += ")";
            aDocListBox->InsertEntry(sEntry);

            if (pConstView && pView == pConstView)
                nConstPos = nCount;

            nCount++;
        }
        pView = SwModule::GetNextView(pView);
    }
    aDocListBox->InsertEntry(aStatusArr[3]); // "Active Window"
    nCount++;

    if(aContentTree->GetHiddenWrtShell())
    {
        OUString sEntry = aContentTree->GetHiddenWrtShell()->GetView().
                                        GetDocShell()->GetTitle();
        sEntry += " (";
        sEntry += aStatusArr[STR_HIDDEN - STR_STATUS_FIRST];
        sEntry += ")";
        aDocListBox->InsertEntry(sEntry);
        bDisable = false;
    }
    if(aContentTree->IsActiveView())
    {
        //Either the name of the current Document or "Active Document".
        aDocListBox->SelectEntryPos( pActView ? nAct : --nCount );
    }
    else if(aContentTree->IsHiddenView())
    {
        aDocListBox->SelectEntryPos(nCount);
    }
    else
        aDocListBox->SelectEntryPos(nConstPos);

    aDocListBox->Enable( !bDisable );
    aDocListBox->SetUpdateMode(true);
}

IMPL_LINK_TYPED(SwNavigationPI, DoneLink, SfxPoolItem *, pItem, void)
{
    const SfxViewFrameItem* pFrameItem = dynamic_cast<SfxViewFrameItem*>( pItem  );
    if( pFrameItem )
    {
        SfxViewFrame* pFrame =  pFrameItem->GetFrame();
        if(pFrame)
        {
            aContentTree->Clear();
            pContentView = dynamic_cast<SwView*>( pFrame->GetViewShell() );
            OSL_ENSURE(pContentView, "no SwView");
            if(pContentView)
                pContentWrtShell = pContentView->GetWrtShellPtr();
            else
                pContentWrtShell = 0;
            pxObjectShell = new SfxObjectShellLock(pFrame->GetObjectShell());
            FillBox();
            aContentTree->Update();
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
        ( aContentTree->IsDropFormatSupported( SotClipboardFormatId::SIMPLE_FILE ) ||
          aContentTree->IsDropFormatSupported( SotClipboardFormatId::STRING ) ||
          aContentTree->IsDropFormatSupported( SotClipboardFormatId::SOLK ) ||
           aContentTree->IsDropFormatSupported( SotClipboardFormatId::NETSCAPE_BOOKMARK )||
           aContentTree->IsDropFormatSupported( SotClipboardFormatId::FILECONTENT ) ||
           aContentTree->IsDropFormatSupported( SotClipboardFormatId::FILEGRPDESCRIPTOR ) ||
           aContentTree->IsDropFormatSupported( SotClipboardFormatId::UNIFORMRESOURCELOCATOR ) ||
           aContentTree->IsDropFormatSupported( SotClipboardFormatId::FILENAME )))
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

    if ((sContentFileName.isEmpty() || sContentFileName != sFileName))
    {
        nRet = rEvt.mnAction;
        sFileName = comphelper::string::stripEnd(sFileName, 0);
        sContentFileName = sFileName;
        if(pxObjectShell)
        {
            aContentTree->SetHiddenShell( 0 );
            (*pxObjectShell)->DoClose();
            DELETEZ( pxObjectShell);
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
    nRegionMode = nNewMode;
    pConfig->SetRegionMode( nRegionMode );

    sal_uInt16 nDropId = FN_DROP_REGION;
    if(nRegionMode == RegionMode::LINK)
        nDropId = FN_DROP_REGION_LINK;
    else if(nRegionMode == RegionMode::EMBEDDED)
        nDropId = FN_DROP_REGION_COPY;

    ImageList& rImgLst = aContentImageList;

    aContentToolBox->SetItemImage( FN_DROP_REGION, rImgLst.GetImage(nDropId));
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
        aGlobalTree->ShowTree();
        aGlobalToolBox->Show();
        aContentTree->HideTree();
        aContentToolBox->Hide();
        aDocListBox->Hide();
        SetGlobalMode(true);
        SetUpdateMode(true);
    }
    else
    {
        aGlobalTree->HideTree();
        aGlobalToolBox->Hide();
        if(!_IsZoomedIn())
        {
            aContentTree->ShowTree();
            aContentToolBox->Show();
            aDocListBox->Show();
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

IMPL_LINK_NOARG(SwNavigationPI, PageEditModifyHdl)
{
    if(aPageChgIdle.IsActive())
        aPageChgIdle.Stop();
    aPageChgIdle.Start();
    return 0;
}

SwView*  SwNavigationPI::GetCreateView() const
{
    if(!pCreateView)
    {
        SwView* pView = SwModule::GetFirstView();
        while(pView)
        {
            if(&pView->GetViewFrame()->GetBindings() == &rBindings)
            {
                const_cast<SwNavigationPI*>(this)->pCreateView = pView;
                const_cast<SwNavigationPI*>(this)->StartListening(*pCreateView);
                break;
            }
            pView = SwModule::GetNextView(pView);
        }
    }
    return pCreateView;
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
        pNavi->aContentTree->SetRootType(nRootType);
        pNavi->aContentToolBox->CheckItem(FN_SHOW_ROOT);
    }
    pNavi->aContentTree->SetOutlineLevel( static_cast< sal_uInt8 >( pNaviConfig->GetOutlineLevel() ) );
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
    ImageList& rImgLst = aContentImageList;
    for( sal_uInt16 k = 0; k < aContentToolBox->GetItemCount(); k++)
            aContentToolBox->SetItemImage(aContentToolBox->GetItemId(k),
                    rImgLst.GetImage(aContentToolBox->GetItemId(k)));

    for( sal_uInt16 k = 0; k < aGlobalToolBox->GetItemCount(); k++)
            aGlobalToolBox->SetItemImage(aGlobalToolBox->GetItemId(k),
                    rImgLst.GetImage(aGlobalToolBox->GetItemId(k)));

    sal_uInt16 nDropId = FN_DROP_REGION;
    if(nRegionMode == RegionMode::LINK)
        nDropId = FN_DROP_REGION_LINK;
    else if(nRegionMode == RegionMode::EMBEDDED)
        nDropId = FN_DROP_REGION_COPY;
    aContentToolBox->SetItemImage( FN_DROP_REGION,
                                    rImgLst.GetImage(nDropId));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
