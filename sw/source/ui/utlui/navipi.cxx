/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#define NAVIPI_CXX

#include <string>
#include <svl/urlbmk.hxx>
#include <svl/stritem.hxx>
#include <svtools/filter.hxx>
#include <svl/urihelper.hxx>
#include <sot/formats.hxx>
#include <sot/filelist.hxx>
#include <sfx2/event.hxx>
#include <sfx2/imgmgr.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/dockwin.hxx>
#include <vcl/toolbox.hxx>
#include <swtypes.hxx>  // fuer Pathfinder
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

#include <unomid.h>

#define PAGE_CHANGE_TIMEOUT 1000 //Timeout fuer Seitenwechsel

#define JUMP_TYPE_TBL 0
#define JUMP_TYPE_FRM 1
#define JUMP_TYPE_GRF 2
#define JUMP_TYPE_REG 3
#define JUMP_TYPE_BKM 4

// Version fuer Konfiguration

#define NAVI_VERSION0   0
#define NAVI_VERSION1   1
#define NAVI_VERSION2   2 // bIsGlobalActive

#define NAVI_CONFIG_VERSION NAVI_VERSION2

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;

SFX_IMPL_CHILDWINDOW_CONTEXT( SwNavigationChild, SID_NAVIGATOR, SwView )

/*------------------------------------------------------------------------
    Bechreibung: Steuerzeichen aus dem Outline-Entry filtern
------------------------------------------------------------------------*/

void SwNavigationPI::CleanEntry( String& rEntry )
{
    USHORT i = rEntry.Len();
    if( i )
        for( sal_Unicode* pStr = rEntry.GetBufferAccess(); i; --i, ++pStr )
            if( *pStr == 10 || *pStr == 9 )
                *pStr = 0x20;
}
/*------------------------------------------------------------------------
 Beschreibung:  Ausfuehrung der Drag-Operation
                mit und ohne Childs
------------------------------------------------------------------------*/

void SwNavigationPI::MoveOutline(USHORT nSource, USHORT nTarget,
                                                    BOOL bWithChilds)
{
    SwView *pView = GetCreateView();
    SwWrtShell &rSh = pView->GetWrtShell();
    if(nTarget < nSource || nTarget == USHRT_MAX)
        nTarget ++;
    if ( rSh.IsOutlineMovable( nSource ))
    {

        short nMove = nTarget-nSource; //( nDir<0 ) ? 1 : 0 ;
        rSh.GotoOutline(nSource);
        if (bWithChilds)
            rSh.MakeOutlineSel(nSource, nSource, TRUE);
        // Die selektierten Children zaehlen bei der Bewegung vorwaerts nicht mit
        USHORT nLastOutlinePos = rSh.GetOutlinePos(MAXLEVEL);
        if(bWithChilds && nMove > 1 &&
                nLastOutlinePos < nTarget)
        {
            if(!rSh.IsCrsrPtAtEnd())
                rSh.SwapPam();
            nMove -= nLastOutlinePos - nSource;
        }
        if(!bWithChilds || nMove < 1 || nLastOutlinePos < nTarget )
            rSh.MoveOutlinePara( nMove );
        rSh.ClearMark();
        rSh.GotoOutline( nSource + nMove);
        FillBox();
    }

}


/*------------------------------------------------------------------------
 Beschreibung:  Nach Goto einen Status Rahmenselektion aufheben
------------------------------------------------------------------------*/


void lcl_UnSelectFrm(SwWrtShell *pSh)
{
    if (pSh->IsFrmSelected())
    {
        pSh->UnSelectFrm();
        pSh->LeaveSelFrmMode();
    }
}

/*------------------------------------------------------------------------
 Beschreibung:  Select der Documentanzeige
------------------------------------------------------------------------*/


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

/*------------------------------------------------------------------------
 Beschreibung:  Fuellen der ListBox fuer Outline Sicht oder Dokumente
                Der PI wird auf volle Groesse gesetzt
------------------------------------------------------------------------*/


void SwNavigationPI::FillBox()
{
    if(pContentWrtShell)
    {
        aContentTree.SetHiddenShell( pContentWrtShell );
        aContentTree.Display(  FALSE );
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
            aContentTree.Display( TRUE );
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
        const USHORT nPageCnt = pSh->GetPageCnt();
        USHORT nPhyPage, nVirPage;
        pSh->GetPageNum(nPhyPage, nVirPage);

        GetPageEdit().SetMax(nPageCnt);
        GetPageEdit().SetLast(nPageCnt);
        GetPageEdit().SetValue(nPhyPage);
    }
}

/*------------------------------------------------------------------------
 Beschreibung:  SelectHandler der Toolboxen
------------------------------------------------------------------------*/


IMPL_LINK( SwNavigationPI, ToolBoxSelectHdl, ToolBox *, pBox )
{
    const USHORT nCurrItemId = pBox->GetCurItemId();
    SwView *pView = GetCreateView();
    if (!pView)
        return 1;
    SwWrtShell &rSh = pView->GetWrtShell();
    //MouseModifier fuer Outline-Move besorgen

    //Standard: Unterebenen werden mitgenommen
    // mit Ctrl Unterebenen nicht mitnehmen
    BOOL bOutlineWithChilds  = ( KEY_MOD1 != pBox->GetModifier());
    int nFuncId = 0;
    BOOL bFocusToDoc = FALSE;
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
        if(pContextWin->GetFloatingWindow())
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
        return TRUE;
        // Funktionen, die eine direkte Aktion ausloesen

        case FN_SELECT_FOOTER:
        {
            rSh.MoveCrsr();
            const USHORT eType = rSh.GetFrmType(0,FALSE);
            if (eType & FRMTYPE_FOOTER)
            {
                if (rSh.EndPg())
                    nFuncId = FN_END_OF_PAGE;
            }
            else if (rSh.GotoFooterTxt())
                nFuncId = FN_TO_FOOTER;
            bFocusToDoc = TRUE;
        }
        break;
        case FN_SELECT_HEADER:
        {
            rSh.MoveCrsr();
            const USHORT eType = rSh.GetFrmType(0,FALSE);
            if (eType & FRMTYPE_HEADER)
            {
                if (rSh.SttPg())
                    nFuncId = FN_START_OF_PAGE;
            }
            else if (rSh.GotoHeaderTxt())
                nFuncId = FN_TO_HEADER;
            bFocusToDoc = TRUE;
        }
        break;
        case FN_SELECT_FOOTNOTE:
        {
            rSh.MoveCrsr();
            const USHORT eFrmType = rSh.GetFrmType(0,FALSE);
                // aus Fussnote zum Anker springen
            if (eFrmType & FRMTYPE_FOOTNOTE)
            {
                if (rSh.GotoFtnAnchor())
                    nFuncId = FN_FOOTNOTE_TO_ANCHOR;
            }
                // andernfalls zuerst zum Fussnotentext springen; geht
                // dies nicht, zur naechten Fussnote; geht auch dies
                // nicht, zur vorhergehenden Fussnote
            else
            {
                if (rSh.GotoFtnTxt())
                    nFuncId = FN_FOOTNOTE_TO_ANCHOR;
                else if (rSh.GotoNextFtnAnchor())
                    nFuncId = FN_NEXT_FOOTNOTE;
                else if (rSh.GotoPrevFtnAnchor())
                    nFuncId = FN_PREV_FOOTNOTE;
            }
            bFocusToDoc = TRUE;
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
                aContentTree.ExecCommand(nCurrItemId, bOutlineWithChilds);
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
            BOOL bSave = rSh.IsGlblDocSaveLinks();
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
    return TRUE;
}
/*------------------------------------------------------------------------
 Beschreibung:  ClickHandler der Toolboxen
------------------------------------------------------------------------*/


IMPL_LINK( SwNavigationPI, ToolBoxClickHdl, ToolBox *, pBox )
{
    const USHORT nCurrItemId = pBox->GetCurItemId();
    switch (nCurrItemId)
    {
        case FN_GLOBAL_UPDATE:
        case FN_GLOBAL_OPEN:
        {
            aGlobalTree.TbxMenuHdl(nCurrItemId, pBox);
        }
        break;
    }

    return TRUE;
}

IMPL_LINK( SwNavigationPI, ToolBoxDropdownClickHdl, ToolBox*, pBox )
{
    const USHORT nCurrItemId = pBox->GetCurItemId();
    switch (nCurrItemId)
    {
        case FN_CREATE_NAVIGATION:
        {
            CreateNavigationTool(pBox->GetItemRect(FN_CREATE_NAVIGATION), TRUE);
        }
        break;

        case FN_DROP_REGION:
        {
            PopupMenu *pMenu = new PopupMenu;
            for (USHORT i = 0; i <= REGION_MODE_EMBEDDED; i++)
            {
                pMenu->InsertItem( i + 1, aContextArr[i] );
                pMenu->SetHelpId(i + 1, HID_NAVI_DRAG_HYP + i);

            }
            pMenu->CheckItem( nRegionMode + 1 );
            pMenu->SetSelectHdl(LINK(this, SwNavigationPI, MenuSelectHdl));
            pBox->SetItemDown( nCurrItemId, TRUE );
            pMenu->Execute( pBox,
                    pBox->GetItemRect(FN_DROP_REGION),
                    POPUPMENU_EXECUTE_DOWN );
            pBox->SetItemDown( nCurrItemId, FALSE );
            pBox->EndSelection();
            delete pMenu;
            pBox->Invalidate();
        }
        break;
        case FN_OUTLINE_LEVEL:
        {
            PopupMenu *pMenu = new PopupMenu;
            for (USHORT i = 101; i <= 100 + MAXLEVEL; i++)
            {
                pMenu->InsertItem( i, String::CreateFromInt32(i - 100) );
                pMenu->SetHelpId( i, HID_NAVI_OUTLINES );
            }
            pMenu->CheckItem( aContentTree.GetOutlineLevel() + 100 );
            pMenu->SetSelectHdl(LINK(this, SwNavigationPI, MenuSelectHdl));
            pBox->SetItemDown( nCurrItemId, TRUE );
            pMenu->Execute( pBox,
                    pBox->GetItemRect(FN_OUTLINE_LEVEL),
                    POPUPMENU_EXECUTE_DOWN );
            pBox->SetItemDown( nCurrItemId, FALSE );
            delete pMenu;
            pBox->EndSelection();
            pBox->Invalidate();
        }
        break;
    }
    return TRUE;
}


SwNavHelpToolBox::SwNavHelpToolBox(SwNavigationPI* pParent, const ResId &rResId) :
            SwHelpToolBox(pParent, rResId)
{}

void SwNavHelpToolBox::MouseButtonDown(const MouseEvent &rEvt)
{
    if(rEvt.GetButtons() == MOUSE_LEFT &&
            FN_CREATE_NAVIGATION == GetItemId(rEvt.GetPosPixel()))
    {
        ((SwNavigationPI*)GetParent())->CreateNavigationTool(GetItemRect(FN_CREATE_NAVIGATION), FALSE);
    }
    else
        SwHelpToolBox::MouseButtonDown(rEvt);
}

void SwNavigationPI::CreateNavigationTool(const Rectangle& rRect, BOOL bSetFocus)
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
    USHORT nItemId = GetItemId(ScreenToOutputPixel(rHEvt.GetMousePosPixel()));
    if( FN_UP == nItemId || FN_DOWN == nItemId )
    {
        SetItemText(nItemId, SwScrollNaviPopup::GetQuickHelpText((FN_DOWN == nItemId)));
    }
    SwHelpToolBox::RequestHelp(rHEvt);
}

/*------------------------------------------------------------------------
 Beschreibung:  Action-Handler Edit; wechselt auf die Seite, wenn
                nicht Gliederungssicht angeschaltet ist.
------------------------------------------------------------------------*/


IMPL_LINK( SwNavigationPI, EditAction, NumEditAction *, pEdit )
{
    SwView *pView = GetCreateView();
    if (pView)
    {
        if(aPageChgTimer.IsActive())
            aPageChgTimer.Stop();
        pCreateView->GetWrtShell().GotoPage((USHORT)pEdit->GetValue(), TRUE);
        pCreateView->GetEditWin().GrabFocus();
        pCreateView->GetViewFrame()->GetBindings().Invalidate(FN_STAT_PAGE);
    }
    return 0;
}

/*------------------------------------------------------------------------
 Beschreibung:  Falls die Seite eingestellt werden kann, wird hier
                das Maximum gesetzt.
------------------------------------------------------------------------*/


IMPL_LINK( SwNavigationPI, EditGetFocus, NumEditAction *, pEdit )
{
    SwView *pView = GetCreateView();
    if (!pView)
        return 0;
    SwWrtShell &rSh = pView->GetWrtShell();

    const USHORT nPageCnt = rSh.GetPageCnt();
    pEdit->SetMax(nPageCnt);
    pEdit->SetLast(nPageCnt);
    return 0;
}

BOOL SwNavigationPI::Close()
{
    SfxViewFrame* pVFrame = pCreateView->GetViewFrame();
    pVFrame->GetBindings().Invalidate(SID_NAVIGATOR);
    pVFrame->GetDispatcher()->Execute(SID_NAVIGATOR);
    return TRUE;
}

/*------------------------------------------------------------------------
 Beschreibung:  Setzen einer automatischen Marke
------------------------------------------------------------------------*/


void SwNavigationPI::MakeMark()
{
    SwView *pView = GetCreateView();
    if (!pView) return;
    SwWrtShell &rSh = pView->GetWrtShell();
    IDocumentMarkAccess* const pMarkAccess = rSh.getIDocumentMarkAccess();

    // collect and sort navigator reminder names
    ::std::vector< ::rtl::OUString > vNavMarkNames;
    for(IDocumentMarkAccess::const_iterator_t ppMark = pMarkAccess->getMarksBegin();
        ppMark != pMarkAccess->getMarksEnd();
        ppMark++)
        if( IDocumentMarkAccess::GetType(**ppMark) == IDocumentMarkAccess::NAVIGATOR_REMINDER )
            vNavMarkNames.push_back(ppMark->get()->GetName());
    ::std::sort(vNavMarkNames.begin(), vNavMarkNames.end());

    // we are maxed out and delete one
    // nAutoMarkIdx rotates through the available MarkNames
    // this assumes that IDocumentMarkAccess generates Names in ascending order
    if(vNavMarkNames.size() == MAX_MARKS)
        pMarkAccess->deleteMark(pMarkAccess->findMark(vNavMarkNames[nAutoMarkIdx]));

    rSh.SetBookmark(KeyCode(), ::rtl::OUString(), ::rtl::OUString(), IDocumentMarkAccess::NAVIGATOR_REMINDER);
    SwView::SetActMark( nAutoMarkIdx );

    if(++nAutoMarkIdx == MAX_MARKS)
        nAutoMarkIdx = 0;
}

void SwNavigationPI::GotoPage()
{
    if ( pContextWin->GetFloatingWindow() && pContextWin->GetFloatingWindow()->IsRollUp())
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
        FloatingWindow* pFloat = pContextWin->GetFloatingWindow();
        bIsZoomedIn = FALSE;
        Size aSz(GetOutputSizePixel());
        aSz.Height() = nZoomOut;
        Size aMinOutSizePixel = ((SfxDockingWindow*)GetParent())->GetMinOutputSizePixel();
        ((SfxDockingWindow*)GetParent())->SetMinOutputSizePixel(Size(
                            aMinOutSizePixel.Width(),nZoomOutInit));
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
        SvLBoxEntry* pFirst = aContentTree.FirstSelected();
        if(pFirst)
            aContentTree.Select(pFirst, TRUE); // toolbox enablen
        pConfig->SetSmall( FALSE );
        aContentToolBox.CheckItem(FN_SHOW_CONTENT_BOX);
    }
}

void SwNavigationPI::_ZoomIn()
{
    FloatingWindow* pFloat = pContextWin->GetFloatingWindow();
    if (pFloat &&
        (!_IsZoomedIn() || ( pContextWin->GetFloatingWindow()->IsRollUp())))
    {
        aContentTree.HideTree();
        aDocListBox.Hide();
        aGlobalTree.HideTree();
        bIsZoomedIn = TRUE;
        Size aSz(GetOutputSizePixel());
        if( aSz.Height() > nZoomIn )
            nZoomOut = ( short ) aSz.Height();

        aSz.Height() = nZoomIn;
        Size aMinOutSizePixel = ((SfxDockingWindow*)GetParent())->GetMinOutputSizePixel();
        ((SfxDockingWindow*)GetParent())->SetMinOutputSizePixel(Size(
                            aMinOutSizePixel.Width(), aSz.Height()));
        pFloat->SetOutputSizePixel(aSz);
        SvLBoxEntry* pFirst = aContentTree.FirstSelected();
        if(pFirst)
            aContentTree.Select(pFirst, TRUE); // toolbox enablen
        pConfig->SetSmall( TRUE );
        aContentToolBox.CheckItem(FN_SHOW_CONTENT_BOX, FALSE);
    }
}

void SwNavigationPI::Resize()
{
    Window* pParent = GetParent();
    FloatingWindow* pFloat =  ((DockingWindow*)pParent)->GetFloatingWindow();
    Size aNewSize;
    if( !_IsZoomedIn() )
    {
        //change the minimum width depending on the dock status
        Size aMinOutSizePixel = ((SfxDockingWindow*)pParent)->GetMinOutputSizePixel();
        if( pFloat)
        {
            aNewSize = pFloat->GetOutputSizePixel();
            aMinOutSizePixel.Width() = nWishWidth;
            aMinOutSizePixel.Height() = _IsZoomedIn() ? nZoomIn : nZoomOutInit;
        }
        else
        {
            aNewSize = pParent->GetOutputSizePixel();
            aMinOutSizePixel.Width() = 0;
            aMinOutSizePixel.Height() = 0;
        }
        ((SfxDockingWindow*)GetParent())->SetMinOutputSizePixel(aMinOutSizePixel);

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
        // GlobalTree faengt weiter oben an und reicht bis ganz unten
        aNewSize.Height() += (nDist + nDocLBIniHeight + aPos.Y() - aGlobalTree.GetPosPixel().Y());
        aGlobalTree.SetSizePixel(aNewSize);
        aDocListBox.SetPosSizePixel( aLBPos.X(), aLBPos.Y(),
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

    bSmallMode(FALSE),
    bIsZoomedIn(FALSE),
    bPageCtrlsVisible(FALSE),
    bGlobalMode(FALSE)
{
    GetCreateView();
    InitImageList();

    aContentToolBox.SetHelpId(HID_NAVIGATOR_TOOLBOX );
    aGlobalToolBox.SetHelpId(HID_NAVIGATOR_GLOBAL_TOOLBOX);
    aDocListBox.SetHelpId(HID_NAVIGATOR_LISTBOX );

    nDocLBIniHeight = aDocListBox.GetSizePixel().Height();
    nZoomOutInit = nZoomOut = Resource::ReadShortRes();

    //NumericField in die Toolbox einfuegen
    NumEditAction* pEdit = new NumEditAction(
                    &aContentToolBox, SW_RES(NF_PAGE ));
    pEdit->SetActionHdl(LINK(this, SwNavigationPI, EditAction));
    pEdit->SetGetFocusHdl(LINK(this, SwNavigationPI, EditGetFocus));
    pEdit->SetModifyHdl(LINK(this, SwNavigationPI, PageEditModifyHdl));

    bPageCtrlsVisible = TRUE;

    //doppelte Separatoren sind nicht erlaubt, also muss
    //die passende Groesse anders ermittelt werden
    Rectangle aFirstRect = aContentToolBox.GetItemRect(FN_SELECT_FOOTNOTE);
    Rectangle aSecondRect = aContentToolBox.GetItemRect(FN_SELECT_HEADER);
    USHORT nWidth = USHORT(aFirstRect.Left() - aSecondRect.Left());

    Size aItemWinSize( nWidth , aFirstRect.Bottom() - aFirstRect.Top() );
    pEdit->SetSizePixel(aItemWinSize);
    aContentToolBox.InsertSeparator(4);
    aContentToolBox.InsertWindow( FN_PAGENUMBER, pEdit, 0, 4);
    aContentToolBox.InsertSeparator(4);
    aContentToolBox.SetHelpId(FN_PAGENUMBER, HID_NAVI_TBX16);
    aContentToolBox.ShowItem( FN_PAGENUMBER );

    for( USHORT i = 0; i <= REGION_MODE_EMBEDDED; i++  )
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

    //Der linke und rechte Rand um die Toolboxen soll gleich sein
    nWishWidth = aContentToolboxSize.Width();
    nWishWidth += 2 * aContentToolBox.GetPosPixel().X();

    FloatingWindow* pFloat =  ((DockingWindow*)pParent)->GetFloatingWindow();
    Size aMinSize(pFloat ? nWishWidth : 0, pFloat ? nZoomOutInit : 0);
    ((SfxDockingWindow*)pParent)->SetMinOutputSizePixel(aMinSize);
    SetOutputSizePixel( Size( nWishWidth, nZoomOutInit));
    Size aTmpParentSize(((SfxDockingWindow*)pParent)->GetSizePixel());
    if(
        (
           aTmpParentSize.Width() < aMinSize.Width() ||
           aTmpParentSize.Height() < aMinSize.Height()
        )
        &&
        ((SfxDockingWindow*)pParent)->GetFloatingWindow() &&
        !((SfxDockingWindow*)pParent)->GetFloatingWindow()->IsRollUp()
      )
        ((SfxDockingWindow*)pParent)->SetOutputSizePixel(aMinSize);

    aContentTree.SetPosSizePixel( 0, nListboxYPos, 0, 0, WINDOW_POSSIZE_Y );
    aContentTree.SetWindowBits( WB_HASBUTTONS|WB_HASBUTTONSATROOT|
                            WB_CLIPCHILDREN|WB_HSCROLL|WB_FORCE_MAKEVISIBLE );
    aContentTree.SetSpaceBetweenEntries(3);
    aContentTree.SetSelectionMode( SINGLE_SELECTION );
    aContentTree.SetDragDropMode(   SV_DRAGDROP_CTRL_MOVE |
                                    SV_DRAGDROP_CTRL_COPY |
                                    SV_DRAGDROP_ENABLE_TOP );
    aContentTree.EnableAsyncDrag(TRUE);
    aContentTree.ShowTree();
    aContentToolBox.CheckItem(FN_SHOW_CONTENT_BOX, TRUE);

//  TreeListBox fuer Globaldokument
    aGlobalTree.SetPosSizePixel( 0, nListboxYPos, 0, 0, WINDOW_POSSIZE_Y );
    aGlobalTree.SetSelectionMode( MULTIPLE_SELECTION );
    aGlobalTree.SetWindowBits( WB_HASBUTTONS|WB_HASBUTTONSATROOT|
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
    aGlobalToolBox.CheckItem(FN_GLOBAL_SWITCH, TRUE);

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
}

SwNavigationPI::~SwNavigationPI()
{
    if(IsGlobalDoc() && !IsGlobalMode())
    {
        SwView *pView = GetCreateView();
        SwWrtShell &rSh = pView->GetWrtShell();
        if( !rSh.IsAllProtect() )
            pView->GetDocShell()->SetReadOnlyUI(FALSE);
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

IMPL_LINK( SwNavigationPI, PopupModeEndHdl, void *, EMPTYARG )
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

void SwNavigationPI::StateChanged( USHORT nSID, SfxItemState /*eState*/,
                                            const SfxPoolItem* /*pState*/ )
{
    if(nSID == SID_DOCFULLNAME)
    {
        SwView *pActView = GetCreateView();
        if(pActView)
        {
            SwWrtShell* pWrtShell = pActView->GetWrtShellPtr();
            aContentTree.SetActiveShell(pWrtShell);
            BOOL bGlobal = IsGlobalDoc();
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

/*------------------------------------------------------------------------
    Bechreibung: NumericField aus der Toolbox holen
------------------------------------------------------------------------*/

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

/*--------------------------------------------------------------------
    Beschreibung:   Benachrichtigung bei geaenderter DocInfo
 --------------------------------------------------------------------*/

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
                        if(aGlobalTree.Update( FALSE ))
                            aGlobalTree.Display();
                        else
                        // wenn kein Update notwendig, dann zumindest painten
                        // wg. der roten Eintraege fuer broken links
                            aGlobalTree.Invalidate();
                    }
                }
            }
        }
    }
}

IMPL_LINK( SwNavigationPI, MenuSelectHdl, Menu *, pMenu )
{
    USHORT nMenuId = pMenu->GetCurItemId();
    if(nMenuId != USHRT_MAX)
    {
        if(nMenuId < 100)
            SetRegionDropMode( --nMenuId);
        else
            aContentTree.SetOutlineLevel( static_cast< BYTE >(nMenuId - 100) );
    }
    return 0;
}

void SwNavigationPI::UpdateListBox()
{
    aDocListBox.SetUpdateMode(FALSE);
    aDocListBox.Clear();
    SwView *pActView = GetCreateView();
    BOOL bDisable = pActView == 0;
    SwView *pView = SwModule::GetFirstView();
    USHORT nCount = 0;
    USHORT nAct = 0;
    USHORT nConstPos = 0;
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
            sEntry += C2S(" (");
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
    aDocListBox.InsertEntry(aStatusArr[3]); //"Aktives Fenster"
    nCount++;

    if(aContentTree.GetHiddenWrtShell())
    {
        String sEntry = aContentTree.GetHiddenWrtShell()->GetView().
                                        GetDocShell()->GetTitle();
        sEntry += C2S(" (");
        sEntry += aStatusArr[ST_HIDDEN - ST_STATUS_FIRST];
        sEntry += ')';
        aDocListBox.InsertEntry(sEntry);
        bDisable = FALSE;
    }
    if(aContentTree.IsActiveView())
    {
        //entweder den Namen des akt. Docs oder "Aktives Dokument"
        USHORT nTmp = pActView ? nAct : --nCount;
        aDocListBox.SelectEntryPos( nTmp );
    }
    else if(aContentTree.IsHiddenView())
    {
        aDocListBox.SelectEntryPos(nCount);
    }
    else
        aDocListBox.SelectEntryPos(nConstPos);

    aDocListBox.Enable( !bDisable );
    aDocListBox.SetUpdateMode(TRUE);
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
    String sFileName;
    ULONG nFmt;
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
    if( sFileName.Len() )
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
        if( !aDesc.Detect() )   // keine Grafiken annehmen
        {
            if( STRING_NOTFOUND == sFileName.Search('#')
                && (!sContentFileName.Len() || sContentFileName != sFileName ))
            {
                nRet = rEvt.mnAction;
                sFileName.EraseTrailingChars( char(0) );
                sContentFileName = sFileName;
                if(pxObjectShell)
                {
                    aContentTree.SetHiddenShell( 0 );
                    (*pxObjectShell)->DoClose();
                    DELETEZ( pxObjectShell);
                }
                SfxStringItem aFileItem(SID_FILE_NAME, sFileName );
                String sOptions = C2S("HRC");
                SfxStringItem aOptionsItem( SID_OPTIONS, sOptions );
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

void SwNavigationPI::SetRegionDropMode(USHORT nNewMode)
{
    nRegionMode = nNewMode;
    pConfig->SetRegionMode( nRegionMode );

    USHORT nDropId = FN_DROP_REGION;
    if(nRegionMode == REGION_MODE_LINK)
        nDropId = FN_DROP_REGION_LINK;
    else if(nRegionMode == REGION_MODE_EMBEDDED)
        nDropId = FN_DROP_REGION_COPY;

    ImageList& rImgLst = aContentImageList;

    aContentToolBox.SetItemImage( FN_DROP_REGION, rImgLst.GetImage(nDropId));
}


BOOL    SwNavigationPI::ToggleTree()
{
    BOOL bRet = TRUE;
    BOOL bGlobalDoc = IsGlobalDoc();
    if(!IsGlobalMode() && bGlobalDoc)
    {
        SetUpdateMode(FALSE);
        if(_IsZoomedIn())
            _ZoomOut();
        aGlobalTree.ShowTree();
        aGlobalToolBox.Show();
        aContentTree.HideTree();
        aContentToolBox.Hide();
        aDocListBox.Hide();
        SetGlobalMode(TRUE);
        SetUpdateMode(TRUE);
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
        bRet = FALSE;
        SetGlobalMode(FALSE);
    }
    return bRet;
}

BOOL    SwNavigationPI::IsGlobalDoc() const
{
    BOOL bRet = FALSE;
    SwView *pView = GetCreateView();
    if(pView)
    {
        SwWrtShell &rSh = pView->GetWrtShell();
        bRet = rSh.IsGlobalDoc();
    }
    return bRet;
}

IMPL_LINK( SwNavigationPI, ChangePageHdl, Timer*, EMPTYARG )
{
    EditAction(&GetPageEdit());
    GetPageEdit().GrabFocus();
    return 0;
}

IMPL_LINK( SwNavigationPI, PageEditModifyHdl, Edit*, EMPTYARG )
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
                        USHORT nId,
                        SfxBindings* _pBindings,
                        SfxChildWinInfo* pInfo )
    : SfxChildWindowContext( nId )
{
    SwNavigationPI* pNavi  = new SwNavigationPI( _pBindings, this, pParent );
    SetWindow( pNavi );
    _pBindings->Invalidate(SID_NAVIGATOR);
    String sExtra = pInfo->aExtraString;

    SwNavigationConfig* pNaviConfig = SW_MOD()->GetNavigationConfig();

    USHORT nRootType = static_cast< USHORT >( pNaviConfig->GetRootType() );
    if( nRootType < CONTENT_TYPE_MAX )
    {
        pNavi->aContentTree.SetRootType(nRootType);
        pNavi->aContentToolBox.CheckItem(FN_SHOW_ROOT, TRUE);
    }
    pNavi->aContentTree.SetOutlineLevel( static_cast< BYTE >( pNaviConfig->GetOutlineLevel() ) );
    pNavi->SetRegionDropMode( static_cast< USHORT >( pNaviConfig->GetRegionMode() ) );

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
    USHORT k;

    ImageList& rImgLst = aContentImageList;
    for( k = 0; k < aContentToolBox.GetItemCount(); k++)
            aContentToolBox.SetItemImage(aContentToolBox.GetItemId(k),
                    rImgLst.GetImage(aContentToolBox.GetItemId(k)));

    for( k = 0; k < aGlobalToolBox.GetItemCount(); k++)
            aGlobalToolBox.SetItemImage(aGlobalToolBox.GetItemId(k),
                    rImgLst.GetImage(aGlobalToolBox.GetItemId(k)));

    USHORT nDropId = FN_DROP_REGION;
    if(nRegionMode == REGION_MODE_LINK)
        nDropId = FN_DROP_REGION_LINK;
    else if(nRegionMode == REGION_MODE_EMBEDDED)
        nDropId = FN_DROP_REGION_COPY;
    aContentToolBox.SetItemImage( FN_DROP_REGION,
                                    rImgLst.GetImage(nDropId));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
