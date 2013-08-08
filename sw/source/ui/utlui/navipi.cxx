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
#include <swtypes.hxx>  // for pathfinder
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

#include <unomid.h>

#define PAGE_CHANGE_TIMEOUT 1000

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;

SFX_IMPL_CHILDWINDOW_CONTEXT( SwNavigationChild, SID_NAVIGATOR, SwView )

// Filter the control characters out of the Outline-Entry

void SwNavigationPI::CleanEntry( String& rEntry )
{
    sal_uInt16 i = rEntry.Len();
    if( i )
        for( sal_Unicode* pStr = rEntry.GetBufferAccess(); i; --i, ++pStr )
            if( *pStr == 10 || *pStr == 9 )
                *pStr = 0x20;
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
            rSh.MakeOutlineSel(nSource, nSource, sal_True);
        // While moving, the selected children does not counting.
        sal_uInt16 nLastOutlinePos = rSh.GetOutlinePos(MAXLEVEL);
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

IMPL_LINK( SwNavigationPI, DocListBoxSelectHdl, ListBox *, pBox )
{
    int nEntryIdx = pBox->GetSelectEntryPos();
    SwView *pView ;
    pView = SwModule::GetFirstView();
    while (nEntryIdx-- && pView)
    {
        pView = SwModule::GetNextView(pView);
    }
    if(!pView)
    {
        nEntryIdx == 0 ?
            aContentTree.ShowHiddenShell():
                aContentTree.ShowActualView();


    }
    else
    {
        aContentTree.SetConstantShell(pView->GetWrtShellPtr());
    }
    return 0;
}

// Filling of the list box for outline view or documents
// The PI will be set to full size

void SwNavigationPI::FillBox()
{
    if(pContentWrtShell)
    {
        aContentTree.SetHiddenShell( pContentWrtShell );
        aContentTree.Display(  sal_False );
    }
    else
    {
        SwView *pView = GetCreateView();
        if(!pView)
        {
            aContentTree.SetActiveShell(0);
        }
        else if( pView != pActContView)
        {
            SwWrtShell* pWrtShell = pView->GetWrtShellPtr();
            aContentTree.SetActiveShell(pWrtShell);
        }
        else
            aContentTree.Display( sal_True );
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

IMPL_LINK( SwNavigationPI, ToolBoxSelectHdl, ToolBox *, pBox )
{
    const sal_uInt16 nCurrItemId = pBox->GetCurItemId();
    SwView *pView = GetCreateView();
    if (!pView)
        return 1;
    SwWrtShell &rSh = pView->GetWrtShell();
    // Get MouseModifier for Outline-Move

    // Standard: sublevels are taken
    // do not take sublevels with Ctrl
    sal_Bool bOutlineWithChildren  = ( KEY_MOD1 != pBox->GetModifier());
    int nFuncId = 0;
    bool bFocusToDoc = false;
    switch (nCurrItemId)
    {
        case FN_UP:
        case FN_DOWN:
        {
            // #i75416# move the execution of the search to an asynchronously called static link
            bool* pbNext = new bool( FN_DOWN == nCurrItemId );
            Application::PostUserEvent( STATIC_LINK(pView, SwView, MoveNavigationHdl), pbNext );
        }
        break;
        case FN_SHOW_ROOT:
        {
            aContentTree.ToggleToRoot();
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
            return sal_True;
        // Functions that will trigger a direct action.

        case FN_SELECT_FOOTER:
        {
            rSh.MoveCrsr();
            const sal_uInt16 eType = rSh.GetFrmType(0,sal_False);
            if (eType & FRMTYPE_FOOTER)
            {
                if (rSh.EndPg())
                    nFuncId = FN_END_OF_PAGE;
            }
            else if (rSh.GotoFooterTxt())
                nFuncId = FN_TO_FOOTER;
            bFocusToDoc = true;
        }
        break;
        case FN_SELECT_HEADER:
        {
            rSh.MoveCrsr();
            const sal_uInt16 eType = rSh.GetFrmType(0,sal_False);
            if (eType & FRMTYPE_HEADER)
            {
                if (rSh.SttPg())
                    nFuncId = FN_START_OF_PAGE;
            }
            else if (rSh.GotoHeaderTxt())
                nFuncId = FN_TO_HEADER;
            bFocusToDoc = true;
        }
        break;
        case FN_SELECT_FOOTNOTE:
        {
            rSh.MoveCrsr();
            const sal_uInt16 eFrmType = rSh.GetFrmType(0,sal_False);
                // Jump from the footnote to the anchor.
            if (eFrmType & FRMTYPE_FOOTNOTE)
            {
                if (rSh.GotoFtnAnchor())
                    nFuncId = FN_FOOTNOTE_TO_ANCHOR;
            }
                // Otherwise, jump to the first footnote text;
                // go to the next footnote if this is not possible;
                // if this is also not possible got to the footnote before.
            else
            {
                if (rSh.GotoFtnTxt())
                    nFuncId = FN_FOOTNOTE_TO_ANCHOR;
                else if (rSh.GotoNextFtnAnchor())
                    nFuncId = FN_NEXT_FOOTNOTE;
                else if (rSh.GotoPrevFtnAnchor())
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
                aGlobalTree.ExecCommand(nCurrItemId);
            else
                aContentTree.ExecCommand(nCurrItemId, bOutlineWithChildren);
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
            sal_Bool bSave = rSh.IsGlblDocSaveLinks();
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
    return sal_True;
}

// Click handler of the toolboxes

IMPL_LINK( SwNavigationPI, ToolBoxClickHdl, ToolBox *, pBox )
{
    const sal_uInt16 nCurrItemId = pBox->GetCurItemId();
    switch (nCurrItemId)
    {
        case FN_GLOBAL_UPDATE:
        case FN_GLOBAL_OPEN:
        {
            aGlobalTree.TbxMenuHdl(nCurrItemId, pBox);
        }
        break;
    }

    return sal_True;
}

IMPL_LINK( SwNavigationPI, ToolBoxDropdownClickHdl, ToolBox*, pBox )
{
    const sal_uInt16 nCurrItemId = pBox->GetCurItemId();
    switch (nCurrItemId)
    {
        case FN_CREATE_NAVIGATION:
        {
            CreateNavigationTool(pBox->GetItemRect(FN_CREATE_NAVIGATION), true);
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
            PopupMenu *pMenu = new PopupMenu;
            for (sal_uInt16 i = 0; i <= REGION_MODE_EMBEDDED; i++)
            {
                pMenu->InsertItem( i + 1, aContextArr[i] );
                pMenu->SetHelpId(i + 1, aHIDs[i]);
            }
            pMenu->CheckItem( nRegionMode + 1 );
            pMenu->SetSelectHdl(LINK(this, SwNavigationPI, MenuSelectHdl));
            pBox->SetItemDown( nCurrItemId, sal_True );
            pMenu->Execute( pBox,
                    pBox->GetItemRect(FN_DROP_REGION),
                    POPUPMENU_EXECUTE_DOWN );
            pBox->SetItemDown( nCurrItemId, sal_False );
            pBox->EndSelection();
            delete pMenu;
            pBox->Invalidate();
        }
        break;
        case FN_OUTLINE_LEVEL:
        {
            PopupMenu *pMenu = new PopupMenu;
            for (sal_uInt16 i = 101; i <= 100 + MAXLEVEL; i++)
            {
                pMenu->InsertItem( i, OUString::number(i - 100) );
                pMenu->SetHelpId( i, HID_NAVI_OUTLINES );
            }
            pMenu->CheckItem( aContentTree.GetOutlineLevel() + 100 );
            pMenu->SetSelectHdl(LINK(this, SwNavigationPI, MenuSelectHdl));
            pBox->SetItemDown( nCurrItemId, sal_True );
            pMenu->Execute( pBox,
                    pBox->GetItemRect(FN_OUTLINE_LEVEL),
                    POPUPMENU_EXECUTE_DOWN );
            pBox->SetItemDown( nCurrItemId, sal_False );
            delete pMenu;
            pBox->EndSelection();
            pBox->Invalidate();
        }
        break;
    }
    return sal_True;
}

SwNavHelpToolBox::SwNavHelpToolBox(SwNavigationPI* pParent, const ResId &rResId) :
            SwHelpToolBox(pParent, rResId)
{}

void SwNavHelpToolBox::MouseButtonDown(const MouseEvent &rEvt)
{
    if(rEvt.GetButtons() == MOUSE_LEFT &&
            FN_CREATE_NAVIGATION == GetItemId(rEvt.GetPosPixel()))
    {
        ((SwNavigationPI*)GetParent())->CreateNavigationTool(GetItemRect(FN_CREATE_NAVIGATION), false);
    }
    else
        SwHelpToolBox::MouseButtonDown(rEvt);
}

void SwNavigationPI::CreateNavigationTool(const Rectangle& rRect, bool bSetFocus)
{
    Reference< XFrame > xFrame = GetCreateView()->GetViewFrame()->GetFrame().GetFrameInterface();
    SwScrollNaviPopup* pPopup = new
        SwScrollNaviPopup(FN_SCROLL_NAVIGATION,
                          xFrame );

    Rectangle aRect(rRect);
    Point aT1 = aRect.TopLeft();
    aT1 = pPopup->GetParent()->OutputToScreenPixel(pPopup->GetParent()->AbsoluteScreenToOutputPixel(aContentToolBox.OutputToAbsoluteScreenPixel(aT1)));
    aRect.SetPos(aT1);
    pPopup->StartPopupMode(aRect, FLOATWIN_POPUPMODE_RIGHT|FLOATWIN_POPUPMODE_ALLOWTEAROFF);
    SetPopupWindow( pPopup );
    if(bSetFocus)
    {
        pPopup->EndPopupMode(FLOATWIN_POPUPMODEEND_TEAROFF);
        pPopup->GrabFocus();
    }
}

void  SwNavHelpToolBox::RequestHelp( const HelpEvent& rHEvt )
{
    sal_uInt16 nItemId = GetItemId(ScreenToOutputPixel(rHEvt.GetMousePosPixel()));
    if( FN_UP == nItemId || FN_DOWN == nItemId )
    {
        SetItemText(nItemId, SwScrollNaviPopup::GetQuickHelpText((FN_DOWN == nItemId)));
    }
    SwHelpToolBox::RequestHelp(rHEvt);
}

// Action-Handler Edit:
// Switches to the page if the structure view is not turned on.

IMPL_LINK( SwNavigationPI, EditAction, NumEditAction *, pEdit )
{
    SwView *pView = GetCreateView();
    if (pView)
    {
        if(aPageChgTimer.IsActive())
            aPageChgTimer.Stop();
        pCreateView->GetWrtShell().GotoPage((sal_uInt16)pEdit->GetValue(), sal_True);
        pCreateView->GetEditWin().GrabFocus();
        pCreateView->GetViewFrame()->GetBindings().Invalidate(FN_STAT_PAGE);
    }
    return 0;
}

// If the page can be set here, the maximum is set.

IMPL_LINK( SwNavigationPI, EditGetFocus, NumEditAction *, pEdit )
{
    SwView *pView = GetCreateView();
    if (!pView)
        return 0;
    SwWrtShell &rSh = pView->GetWrtShell();

    const sal_uInt16 nPageCnt = rSh.GetPageCnt();
    pEdit->SetMax(nPageCnt);
    pEdit->SetLast(nPageCnt);
    return 0;
}

sal_Bool SwNavigationPI::Close()
{
    SfxViewFrame* pVFrame = pCreateView->GetViewFrame();
    pVFrame->GetBindings().Invalidate(SID_NAVIGATOR);
    pVFrame->GetDispatcher()->Execute(SID_NAVIGATOR);
    return sal_True;
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
    for(IDocumentMarkAccess::const_iterator_t ppMark = pMarkAccess->getMarksBegin();
        ppMark != pMarkAccess->getMarksEnd();
        ++ppMark)
        if( IDocumentMarkAccess::GetType(**ppMark) == IDocumentMarkAccess::NAVIGATOR_REMINDER )
            vNavMarkNames.push_back(ppMark->get()->GetName());
    ::std::sort(vNavMarkNames.begin(), vNavMarkNames.end());

    // we are maxed out and delete one
    // nAutoMarkIdx rotates through the available MarkNames
    // this assumes that IDocumentMarkAccess generates Names in ascending order
    if(vNavMarkNames.size() == MAX_MARKS)
        pMarkAccess->deleteMark(pMarkAccess->findMark(vNavMarkNames[nAutoMarkIdx]));

    rSh.SetBookmark(KeyCode(), OUString(), OUString(), IDocumentMarkAccess::NAVIGATOR_REMINDER);
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
        bIsZoomedIn = sal_False;
        Size aSz(GetOutputSizePixel());
        aSz.Height() = nZoomOut;
        Size aMinOutSizePixel = ((SfxDockingWindow*)GetParent())->GetMinOutputSizePixel();
        ((SfxDockingWindow*)GetParent())->SetMinOutputSizePixel(Size(
                            aMinOutSizePixel.Width(),nZoomOutInit));
        if (pFloat != NULL)
            pFloat->SetOutputSizePixel(aSz);
        FillBox();
        if(IsGlobalMode())
        {
            aGlobalTree.ShowTree();
        }
        else
        {
            aContentTree.ShowTree();
            aDocListBox.Show();
        }
        SvTreeListEntry* pFirst = aContentTree.FirstSelected();
        if(pFirst)
            aContentTree.Select(pFirst, sal_True); // Enable toolbox
        pConfig->SetSmall( sal_False );
        aContentToolBox.CheckItem(FN_SHOW_CONTENT_BOX);
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
            aContentTree.HideTree();
            aDocListBox.Hide();
            aGlobalTree.HideTree();
            bIsZoomedIn = sal_True;
            Size aSz(GetOutputSizePixel());
            if( aSz.Height() > nZoomIn )
                nZoomOut = ( short ) aSz.Height();

            aSz.Height() = nZoomIn;
            Size aMinOutSizePixel = ((SfxDockingWindow*)GetParent())->GetMinOutputSizePixel();
            ((SfxDockingWindow*)GetParent())->SetMinOutputSizePixel(Size(
                    aMinOutSizePixel.Width(), aSz.Height()));
            pFloat->SetOutputSizePixel(aSz);
            SvTreeListEntry* pFirst = aContentTree.FirstSelected();
            if(pFirst)
                aContentTree.Select(pFirst, sal_True); // Enable toolbox
            pConfig->SetSmall( sal_True );
            aContentToolBox.CheckItem(FN_SHOW_CONTENT_BOX, sal_False);
        }
    }
}

void SwNavigationPI::Resize()
{
    Window* pParent = GetParent();
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

        const Point aPos = aContentTree.GetPosPixel();
        Point aLBPos = aDocListBox.GetPosPixel();
        long nDist = aPos.X();
        aNewSize.Height() -= (aPos.Y() + aPos.X() + nDocLBIniHeight + nDist);
        aNewSize.Width() -= 2 * nDist;
        aLBPos.Y() = aPos.Y() + aNewSize.Height() + nDist;
        aDocListBox.Show(!aGlobalTree.IsVisible() && aLBPos.Y() > aPos.Y() );

        Size aDocLBSz = aDocListBox.GetSizePixel();
        aDocLBSz.Width() = aNewSize.Width();
        if(aNewSize.Height() < 0)
            aDocLBSz.Height() = 0;
        else
            aDocLBSz.Height() = nDocLBIniHeight;
        aContentTree.SetSizePixel(aNewSize);
        // GlobalTree starts on to the top and goes all the way down.
        aNewSize.Height() += (nDist + nDocLBIniHeight + aPos.Y() - aGlobalTree.GetPosPixel().Y());
        aGlobalTree.SetSizePixel(aNewSize);
        aDocListBox.setPosSizePixel( aLBPos.X(), aLBPos.Y(),
            aDocLBSz.Width(), aDocLBSz.Height(),
            WINDOW_POSSIZE_X|WINDOW_POSSIZE_Y|WINDOW_POSSIZE_WIDTH);
    }
}

SwNavigationPI::SwNavigationPI( SfxBindings* _pBindings,
                                SfxChildWindowContext* pCw,
                                Window* pParent) :

    Window( pParent, SW_RES(DLG_NAVIGATION_PI)),
    SfxControllerItem( SID_DOCFULLNAME, *_pBindings ),

    aContentToolBox(this, SW_RES(TB_CONTENT)),
    aGlobalToolBox(this, SW_RES(TB_GLOBAL)),
    aContentImageList(SW_RES(IL_CONTENT)),
    aContentTree(this, SW_RES(TL_CONTENT)),
    aGlobalTree(this, SW_RES(TL_GLOBAL)),
    aDocListBox(this, SW_RES(LB_DOCS)),

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
    nRegionMode(REGION_MODE_NONE),

    bSmallMode(sal_False),
    bIsZoomedIn(sal_False),
    bPageCtrlsVisible(sal_False),
    bGlobalMode(sal_False)
{
    GetCreateView();
    InitImageList();

    aContentToolBox.SetHelpId(HID_NAVIGATOR_TOOLBOX );
    aGlobalToolBox.SetHelpId(HID_NAVIGATOR_GLOBAL_TOOLBOX);
    aDocListBox.SetHelpId(HID_NAVIGATOR_LISTBOX );

    nDocLBIniHeight = aDocListBox.GetSizePixel().Height();
    nZoomOutInit = nZoomOut = Resource::ReadShortRes();

    // Insert the numeric field in the toolbox.
    NumEditAction* pEdit = new NumEditAction(
                    &aContentToolBox, SW_RES(NF_PAGE ));
    pEdit->SetActionHdl(LINK(this, SwNavigationPI, EditAction));
    pEdit->SetGetFocusHdl(LINK(this, SwNavigationPI, EditGetFocus));
    pEdit->SetAccessibleName(pEdit->GetQuickHelpText());
    pEdit->SetUpHdl(LINK(this, SwNavigationPI, PageEditModifyHdl));
    pEdit->SetDownHdl(LINK(this, SwNavigationPI, PageEditModifyHdl));

    bPageCtrlsVisible = sal_True;

    // Double separators are not allowed, so you have to
    // determine the suitable size differently.
    Rectangle aFirstRect = aContentToolBox.GetItemRect(FN_SELECT_FOOTNOTE);
    Rectangle aSecondRect = aContentToolBox.GetItemRect(FN_SELECT_HEADER);
    sal_uInt16 nWidth = sal_uInt16(aFirstRect.Left() - aSecondRect.Left());

    Size aItemWinSize( nWidth , aFirstRect.Bottom() - aFirstRect.Top() );
    pEdit->SetSizePixel(aItemWinSize);
    aContentToolBox.InsertSeparator(4);
    aContentToolBox.InsertWindow( FN_PAGENUMBER, pEdit, 0, 4);
    aContentToolBox.InsertSeparator(4);
    aContentToolBox.SetHelpId(FN_PAGENUMBER, HID_NAVI_TBX16);
    aContentToolBox.ShowItem( FN_PAGENUMBER );

    for( sal_uInt16 i = 0; i <= REGION_MODE_EMBEDDED; i++  )
    {
        aContextArr[i] = SW_RESSTR(ST_HYPERLINK + i);
        aStatusArr[i] = SW_RESSTR(ST_STATUS_FIRST + i);
    }
    aStatusArr[3] = SW_RESSTR(ST_ACTIVE_VIEW);
    FreeResource();


    const Size& rOutSize =  GetOutputSizePixel();

    nZoomIn = (short)rOutSize.Height();

    // Make sure the toolbox has a size that fits all its contents
    Size aContentToolboxSize( aContentToolBox.CalcWindowSizePixel() );
    aContentToolBox.SetOutputSizePixel( aContentToolboxSize );

    // position listbox below toolbar and add some space
    long nListboxYPos = aContentToolBox.GetPosPixel().Y() + aContentToolboxSize.Height() + 4;

    // The left and right margins around the toolboxes should be equal.
    nWishWidth = aContentToolboxSize.Width();
    nWishWidth += 2 * aContentToolBox.GetPosPixel().X();

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

    aContentTree.setPosSizePixel( 0, nListboxYPos, 0, 0, WINDOW_POSSIZE_Y );
    aContentTree.SetStyle( aContentTree.GetStyle()|WB_HASBUTTONS|WB_HASBUTTONSATROOT|
                            WB_CLIPCHILDREN|WB_HSCROLL|WB_FORCE_MAKEVISIBLE );
    aContentTree.SetSpaceBetweenEntries(3);
    aContentTree.SetSelectionMode( SINGLE_SELECTION );
    aContentTree.SetDragDropMode(   SV_DRAGDROP_CTRL_MOVE |
                                    SV_DRAGDROP_CTRL_COPY |
                                    SV_DRAGDROP_ENABLE_TOP );
    aContentTree.EnableAsyncDrag(sal_True);
    aContentTree.ShowTree();
    aContentToolBox.CheckItem(FN_SHOW_CONTENT_BOX, sal_True);

//  TreeListBox for global document
    aGlobalTree.setPosSizePixel( 0, nListboxYPos, 0, 0, WINDOW_POSSIZE_Y );
    aGlobalTree.SetSelectionMode( MULTIPLE_SELECTION );
    aGlobalTree.SetStyle( aGlobalTree.GetStyle()|WB_HASBUTTONS|WB_HASBUTTONSATROOT|
                                WB_CLIPCHILDREN|WB_HSCROLL );
    Size aGlblSize(aGlobalToolBox.CalcWindowSizePixel());
    aGlobalToolBox.SetSizePixel(aGlblSize);

//  Handler

    Link aLk = LINK(this, SwNavigationPI, ToolBoxSelectHdl);
    aContentToolBox.SetSelectHdl( aLk );
    aGlobalToolBox.SetSelectHdl( aLk );
    aDocListBox.SetSelectHdl(LINK(this, SwNavigationPI,
                                                    DocListBoxSelectHdl));
    aContentToolBox.SetClickHdl( LINK(this, SwNavigationPI, ToolBoxClickHdl) );
    aContentToolBox.SetDropdownClickHdl( LINK(this, SwNavigationPI, ToolBoxDropdownClickHdl) );
    aGlobalToolBox.SetClickHdl( LINK(this, SwNavigationPI, ToolBoxClickHdl) );
    aGlobalToolBox.SetDropdownClickHdl( LINK(this, SwNavigationPI, ToolBoxDropdownClickHdl) );
    aGlobalToolBox.CheckItem(FN_GLOBAL_SWITCH, sal_True);

    Font aFont(GetFont());
    aFont.SetWeight(WEIGHT_NORMAL);
    GetPageEdit().SetFont(aFont);
    aFont = aContentTree.GetFont();
    aFont.SetWeight(WEIGHT_NORMAL);
    aContentTree.SetFont(aFont);
    aGlobalTree.SetFont(aFont);

    StartListening(*SFX_APP());
    if ( pCreateView )
        StartListening(*pCreateView);
    SfxImageManager* pImgMan = SfxImageManager::GetImageManager( SW_MOD() );
    pImgMan->RegisterToolBox(&aContentToolBox, SFX_TOOLBOX_CHANGEOUTSTYLE);
    pImgMan->RegisterToolBox(&aGlobalToolBox, SFX_TOOLBOX_CHANGEOUTSTYLE);

    aContentToolBox.SetItemBits( FN_CREATE_NAVIGATION, aContentToolBox.GetItemBits( FN_CREATE_NAVIGATION ) | TIB_DROPDOWNONLY );
    aContentToolBox.SetItemBits( FN_DROP_REGION, aContentToolBox.GetItemBits( FN_DROP_REGION ) | TIB_DROPDOWNONLY );
    aContentToolBox.SetItemBits( FN_OUTLINE_LEVEL, aContentToolBox.GetItemBits( FN_OUTLINE_LEVEL ) | TIB_DROPDOWNONLY );

    if(IsGlobalDoc())
    {
        SwView *pActView = GetCreateView();
        aGlobalToolBox.CheckItem(FN_GLOBAL_SAVE_CONTENT,
                    pActView->GetWrtShellPtr()->IsGlblDocSaveLinks());
        if(pConfig->IsGlobalActive())
            ToggleTree();
        aGlobalTree.GrabFocus();
    }
    else
        aContentTree.GrabFocus();
    UsePage(0);
    aPageChgTimer.SetTimeoutHdl(LINK(this, SwNavigationPI, ChangePageHdl));
    aPageChgTimer.SetTimeout(PAGE_CHANGE_TIMEOUT);

    aContentTree.SetAccessibleName(SW_RESSTR(STR_ACCESS_TL_CONTENT));
    aGlobalTree.SetAccessibleName(SW_RESSTR(STR_ACCESS_TL_GLOBAL));
    aDocListBox.SetAccessibleName(aStatusArr[3]);

    if (pContextWin == NULL)
    {
        // When the context window is missing then the navigator is
        // displayed in the sidebar.  While the navigator could change
        // its size, the sidebar can not, and the navigator would just
        // waste space.  Therefore hide this button.
        aContentToolBox.RemoveItem(aContentToolBox.GetItemPos(FN_SHOW_CONTENT_BOX));
    }
}

SwNavigationPI::~SwNavigationPI()
{
    if(IsGlobalDoc() && !IsGlobalMode())
    {
        SwView *pView = GetCreateView();
        SwWrtShell &rSh = pView->GetWrtShell();
        if( !rSh.IsAllProtect() )
            pView->GetDocShell()->SetReadOnlyUI(sal_False);
    }

    EndListening(*SFX_APP());

    SfxImageManager* pImgMan = SfxImageManager::GetImageManager( SW_MOD() );
    pImgMan->ReleaseToolBox(&aContentToolBox);
    pImgMan->ReleaseToolBox(&aGlobalToolBox);
    delete aContentToolBox.GetItemWindow(FN_PAGENUMBER);
    aContentToolBox.Clear();
    if(pxObjectShell)
    {
        if(pxObjectShell->Is())
            (*pxObjectShell)->DoClose();
        delete pxObjectShell;
    }
    delete pPopupWindow;
    delete pFloatingWindow;

    if ( IsBound() )
        rBindings.Release(*this);
}

void SwNavigationPI::SetPopupWindow( SfxPopupWindow* pWindow )
{
    pPopupWindow = pWindow;
    pPopupWindow->SetPopupModeEndHdl( LINK( this, SwNavigationPI, PopupModeEndHdl ));
    pPopupWindow->SetDeleteLink_Impl( LINK( this, SwNavigationPI, ClosePopupWindow ));
}

IMPL_LINK_NOARG(SwNavigationPI, PopupModeEndHdl)
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

IMPL_LINK( SwNavigationPI, ClosePopupWindow, SfxPopupWindow *, pWindow )
{
    if ( pWindow == pFloatingWindow )
        pFloatingWindow = 0;
    else
        pPopupWindow = 0;

    return 1;
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
            aContentTree.SetActiveShell(pWrtShell);
            sal_Bool bGlobal = IsGlobalDoc();
            aContentToolBox.EnableItem(FN_GLOBAL_SWITCH, bGlobal);
            if( (!bGlobal && IsGlobalMode()) ||
                    (!IsGlobalMode() && pConfig->IsGlobalActive()) )
            {
                ToggleTree();
            }
            if(bGlobal)
            {
                aGlobalToolBox.CheckItem(FN_GLOBAL_SAVE_CONTENT, pWrtShell->IsGlblDocSaveLinks());
            }
        }
        else
        {
            aContentTree.SetActiveShell(0);
        }
        UpdateListBox();
    }
}

// Get the numeric field from the toolbox.

NumEditAction& SwNavigationPI::GetPageEdit()
{
    return *(NumEditAction*)aContentToolBox.GetItemWindow(FN_PAGENUMBER);
}

SfxChildAlignment SwNavigationPI::CheckAlignment
    (
        SfxChildAlignment eActAlign,
        SfxChildAlignment eAlign
    )
{
SfxChildAlignment eRetAlign;

    if(_IsZoomedIn())
        eRetAlign = SFX_ALIGN_NOALIGNMENT;
    else
        switch (eAlign)
        {
            case SFX_ALIGN_BOTTOM:
            case SFX_ALIGN_LOWESTBOTTOM:
            case SFX_ALIGN_HIGHESTBOTTOM:
                eRetAlign = eActAlign;
                break;

            case SFX_ALIGN_TOP:
            case SFX_ALIGN_HIGHESTTOP:
            case SFX_ALIGN_LOWESTTOP:
            case SFX_ALIGN_LEFT:
            case SFX_ALIGN_RIGHT:
            case SFX_ALIGN_FIRSTLEFT:
            case SFX_ALIGN_LASTLEFT:
            case SFX_ALIGN_FIRSTRIGHT:
            case SFX_ALIGN_LASTRIGHT:
                eRetAlign = eAlign;
                break;

            default:
                eRetAlign = eAlign;
                break;
        }
    return eRetAlign;

}

// Notification on modified DocInfo

void SwNavigationPI::Notify( SfxBroadcaster& rBrdc, const SfxHint& rHint )
{
    if(&rBrdc == pCreateView)
    {
        if(rHint.ISA(SfxSimpleHint) && ((SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING)
        {
            pCreateView = 0;
        }
    }
    else
    {
        if(rHint.ISA(SfxEventHint))
        {
            if( pxObjectShell &&
                        ((SfxEventHint&) rHint).GetEventId() == SFX_EVENT_CLOSEAPP)
            {
                DELETEZ(pxObjectShell);
            }
            else if(((SfxEventHint&) rHint).GetEventId() == SFX_EVENT_OPENDOC)
            {

                SwView *pActView = GetCreateView();
                if(pActView)
                {
                    SwWrtShell* pWrtShell = pActView->GetWrtShellPtr();
                    aContentTree.SetActiveShell(pWrtShell);
                    if(aGlobalTree.IsVisible())
                    {
                        if(aGlobalTree.Update( sal_False ))
                            aGlobalTree.Display();
                        else
                        // If no update is needed, then paint at least,
                        // because of the red entries for the broken links.
                            aGlobalTree.Invalidate();
                    }
                }
            }
        }
    }
}

IMPL_LINK( SwNavigationPI, MenuSelectHdl, Menu *, pMenu )
{
    sal_uInt16 nMenuId = pMenu->GetCurItemId();
    if(nMenuId != USHRT_MAX)
    {
        if(nMenuId < 100)
            SetRegionDropMode( --nMenuId);
        else
            aContentTree.SetOutlineLevel( static_cast< sal_uInt8 >(nMenuId - 100) );
    }
    return 0;
}

void SwNavigationPI::UpdateListBox()
{
    aDocListBox.SetUpdateMode(sal_False);
    aDocListBox.Clear();
    SwView *pActView = GetCreateView();
    bool bDisable = pActView == 0;
    SwView *pView = SwModule::GetFirstView();
    sal_uInt16 nCount = 0;
    sal_uInt16 nAct = 0;
    sal_uInt16 nConstPos = 0;
    const SwView* pConstView = aContentTree.IsConstantView() &&
                                aContentTree.GetActiveWrtShell() ?
                                    &aContentTree.GetActiveWrtShell()->GetView():
                                        0;
    while (pView)
    {
        SfxObjectShell* pDoc = pView->GetDocShell();
        // #i53333# don't show help pages here
        if ( !pDoc->IsHelpDocument() )
        {
            String sEntry = pDoc->GetTitle();
            sEntry += OUString(" (");
            if (pView == pActView)
            {
                nAct = nCount;
                sEntry += aStatusArr[ST_ACTIVE - ST_STATUS_FIRST];
            }
            else
                sEntry += aStatusArr[ST_INACTIVE - ST_STATUS_FIRST];
            sEntry += ')';
            aDocListBox.InsertEntry(sEntry);


            if (pConstView && pView == pConstView)
                nConstPos = nCount;

            nCount++;
        }
        pView = SwModule::GetNextView(pView);
    }
    aDocListBox.InsertEntry(aStatusArr[3]); // "Active Window"
    nCount++;

    if(aContentTree.GetHiddenWrtShell())
    {
        String sEntry = aContentTree.GetHiddenWrtShell()->GetView().
                                        GetDocShell()->GetTitle();
        sEntry += OUString(" (");
        sEntry += aStatusArr[ST_HIDDEN - ST_STATUS_FIRST];
        sEntry += ')';
        aDocListBox.InsertEntry(sEntry);
        bDisable = false;
    }
    if(aContentTree.IsActiveView())
    {
        //Either the name of the current Document or "Active Document".
        sal_uInt16 nTmp = pActView ? nAct : --nCount;
        aDocListBox.SelectEntryPos( nTmp );
    }
    else if(aContentTree.IsHiddenView())
    {
        aDocListBox.SelectEntryPos(nCount);
    }
    else
        aDocListBox.SelectEntryPos(nConstPos);

    aDocListBox.Enable( !bDisable );
    aDocListBox.SetUpdateMode(sal_True);
}

IMPL_LINK(SwNavigationPI, DoneLink, SfxPoolItem *, pItem)
{
    const SfxViewFrameItem* pFrameItem = PTR_CAST(SfxViewFrameItem, pItem );
    if( pFrameItem )
    {
        SfxViewFrame* pFrame =  pFrameItem->GetFrame();
        if(pFrame)
        {
            aContentTree.Clear();
            pContentView = PTR_CAST(SwView, pFrame->GetViewShell());
            OSL_ENSURE(pContentView, "no SwView");
            if(pContentView)
                pContentWrtShell = pContentView->GetWrtShellPtr();
            else
                pContentWrtShell = 0;
            pxObjectShell = new SfxObjectShellLock(pFrame->GetObjectShell());
            FillBox();
            aContentTree.Update();
        }
    }
    return 0;
}

String SwNavigationPI::CreateDropFileName( TransferableDataHelper& rData )
{
    OUString sFileName;
    sal_uLong nFmt;
    if( rData.HasFormat( nFmt = FORMAT_FILE_LIST ))
    {
        FileList aFileList;
        rData.GetFileList( nFmt, aFileList );
        sFileName = aFileList.GetFile( 0 );
    }
    else if( rData.HasFormat( nFmt = FORMAT_STRING ) ||
              rData.HasFormat( nFmt = FORMAT_FILE ) ||
             rData.HasFormat( nFmt = SOT_FORMATSTR_ID_FILENAME ))
        rData.GetString( nFmt, sFileName );
    else if( rData.HasFormat( nFmt = SOT_FORMATSTR_ID_SOLK ) ||
                rData.HasFormat( nFmt = SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK )||
                rData.HasFormat( nFmt = SOT_FORMATSTR_ID_FILECONTENT ) ||
                rData.HasFormat( nFmt = SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR ) ||
                rData.HasFormat( nFmt = SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR ))
    {
        INetBookmark aBkmk( aEmptyStr, aEmptyStr );
        rData.GetINetBookmark( nFmt, aBkmk );
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
    return ( !aContentTree.IsInDrag() &&
        ( aContentTree.IsDropFormatSupported( FORMAT_FILE ) ||
          aContentTree.IsDropFormatSupported( FORMAT_STRING ) ||
          aContentTree.IsDropFormatSupported( SOT_FORMATSTR_ID_SOLK ) ||
           aContentTree.IsDropFormatSupported( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK )||
           aContentTree.IsDropFormatSupported( SOT_FORMATSTR_ID_FILECONTENT ) ||
           aContentTree.IsDropFormatSupported( SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR ) ||
           aContentTree.IsDropFormatSupported( SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR ) ||
           aContentTree.IsDropFormatSupported( SOT_FORMATSTR_ID_FILENAME )))
        ? DND_ACTION_COPY
        : DND_ACTION_NONE;
}

sal_Int8 SwNavigationPI::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    TransferableDataHelper aData( rEvt.maDropEvent.Transferable );
    sal_Int8 nRet = DND_ACTION_NONE;
    String sFileName;
    if( !aContentTree.IsInDrag() &&
        0 != (sFileName = SwNavigationPI::CreateDropFileName( aData )).Len() )
    {
        INetURLObject aTemp( sFileName );
        GraphicDescriptor aDesc( aTemp );
        if( !aDesc.Detect() )   // accept no graphics
        {
            if( STRING_NOTFOUND == sFileName.Search('#')
                && (!sContentFileName.Len() || sContentFileName != sFileName ))
            {
                nRet = rEvt.mnAction;
                sFileName = comphelper::string::stripEnd(sFileName, 0);
                sContentFileName = sFileName;
                if(pxObjectShell)
                {
                    aContentTree.SetHiddenShell( 0 );
                    (*pxObjectShell)->DoClose();
                    DELETEZ( pxObjectShell);
                }
                SfxStringItem aFileItem(SID_FILE_NAME, sFileName );
                SfxStringItem aOptionsItem( SID_OPTIONS, OUString("HRC") );
                SfxLinkItem aLink( SID_DONELINK,
                                    LINK( this, SwNavigationPI, DoneLink ) );
                GetActiveView()->GetViewFrame()->GetDispatcher()->Execute(
                            SID_OPENDOC, SFX_CALLMODE_ASYNCHRON,
                            &aFileItem, &aOptionsItem, &aLink, 0L );
            }
        }
    }
    return nRet;
}

void SwNavigationPI::SetRegionDropMode(sal_uInt16 nNewMode)
{
    nRegionMode = nNewMode;
    pConfig->SetRegionMode( nRegionMode );

    sal_uInt16 nDropId = FN_DROP_REGION;
    if(nRegionMode == REGION_MODE_LINK)
        nDropId = FN_DROP_REGION_LINK;
    else if(nRegionMode == REGION_MODE_EMBEDDED)
        nDropId = FN_DROP_REGION_COPY;

    ImageList& rImgLst = aContentImageList;

    aContentToolBox.SetItemImage( FN_DROP_REGION, rImgLst.GetImage(nDropId));
}

sal_Bool    SwNavigationPI::ToggleTree()
{
    sal_Bool bRet = sal_True;
    sal_Bool bGlobalDoc = IsGlobalDoc();
    if(!IsGlobalMode() && bGlobalDoc)
    {
        SetUpdateMode(sal_False);
        if(_IsZoomedIn())
            _ZoomOut();
        aGlobalTree.ShowTree();
        aGlobalToolBox.Show();
        aContentTree.HideTree();
        aContentToolBox.Hide();
        aDocListBox.Hide();
        SetGlobalMode(sal_True);
        SetUpdateMode(sal_True);
    }
    else
    {
        aGlobalTree.HideTree();
        aGlobalToolBox.Hide();
        if(!_IsZoomedIn())
        {
            aContentTree.ShowTree();
            aContentToolBox.Show();
            aDocListBox.Show();
        }
        bRet = sal_False;
        SetGlobalMode(sal_False);
    }
    return bRet;
}

sal_Bool    SwNavigationPI::IsGlobalDoc() const
{
    sal_Bool bRet = sal_False;
    SwView *pView = GetCreateView();
    if(pView)
    {
        SwWrtShell &rSh = pView->GetWrtShell();
        bRet = rSh.IsGlobalDoc();
    }
    return bRet;
}

IMPL_LINK_NOARG(SwNavigationPI, ChangePageHdl)
{
    EditAction(&GetPageEdit());
    GetPageEdit().GrabFocus();
    return 0;
}

IMPL_LINK_NOARG(SwNavigationPI, PageEditModifyHdl)
{
    if(aPageChgTimer.IsActive())
        aPageChgTimer.Stop();
    aPageChgTimer.Start();
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
                ((SwNavigationPI*)this)->pCreateView = pView;
                ((SwNavigationPI*)this)->StartListening(*pCreateView);
                break;
            }
            pView = SwModule::GetNextView(pView);
        }
    }
    return pCreateView;
}

SwNavigationChild::SwNavigationChild( Window* pParent,
                        sal_uInt16 nId,
                        SfxBindings* _pBindings,
                        SfxChildWinInfo* )
    : SfxChildWindowContext( nId )
{
    SwNavigationPI* pNavi  = new SwNavigationPI( _pBindings, this, pParent );
    SetWindow( pNavi );
    _pBindings->Invalidate(SID_NAVIGATOR);

    SwNavigationConfig* pNaviConfig = SW_MOD()->GetNavigationConfig();

    sal_uInt16 nRootType = static_cast< sal_uInt16 >( pNaviConfig->GetRootType() );
    if( nRootType < CONTENT_TYPE_MAX )
    {
        pNavi->aContentTree.SetRootType(nRootType);
        pNavi->aContentToolBox.CheckItem(FN_SHOW_ROOT, sal_True);
    }
    pNavi->aContentTree.SetOutlineLevel( static_cast< sal_uInt8 >( pNaviConfig->GetOutlineLevel() ) );
    pNavi->SetRegionDropMode( static_cast< sal_uInt16 >( pNaviConfig->GetRegionMode() ) );

    if(GetFloatingWindow() && pNaviConfig->IsSmall())
    {
        pNavi->_ZoomIn();
    }
}

void SwNavigationPI::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
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
    sal_uInt16 k;

    ImageList& rImgLst = aContentImageList;
    for( k = 0; k < aContentToolBox.GetItemCount(); k++)
            aContentToolBox.SetItemImage(aContentToolBox.GetItemId(k),
                    rImgLst.GetImage(aContentToolBox.GetItemId(k)));

    for( k = 0; k < aGlobalToolBox.GetItemCount(); k++)
            aGlobalToolBox.SetItemImage(aGlobalToolBox.GetItemId(k),
                    rImgLst.GetImage(aGlobalToolBox.GetItemId(k)));

    sal_uInt16 nDropId = FN_DROP_REGION;
    if(nRegionMode == REGION_MODE_LINK)
        nDropId = FN_DROP_REGION_LINK;
    else if(nRegionMode == REGION_MODE_EMBEDDED)
        nDropId = FN_DROP_REGION_COPY;
    aContentToolBox.SetItemImage( FN_DROP_REGION,
                                    rImgLst.GetImage(nDropId));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
