/*************************************************************************
 *
 *  $RCSfile: navipi.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2000-10-12 08:29:05 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif


#pragma hdrstop

#define NAVIPI_CXX

#include <string> // HACK: prevent conflict between STLPORT and Workshop headers

#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _SV_DRAG_HXX //autogen
#include <vcl/drag.hxx>
#endif
#ifndef _SV_SYSTEM_HXX
#include <vcl/system.hxx>
#endif
#ifndef _URLBMK_HXX //autogen
#include <svtools/urlbmk.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _FILTER_HXX //autogen
#include <svtools/filter.hxx>
#endif
#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif
#ifndef _SFXEVENT_HXX //autogen
#include <sfx2/event.hxx>
#endif
#ifndef _SFXIMGMGR_HXX //autogen
#include <sfx2/imgmgr.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXDOCKWIN_HXX //autogen
#include <sfx2/dockwin.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>  // fuer Pathfinder
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _NAVICFG_HXX
#include <navicfg.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _ACTCTRL_HXX
#include <actctrl.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>
#endif
#ifndef _NAVIPI_HXX
#include <navipi.hxx>
#endif
#ifndef _CONTENT_HXX
#include <content.hxx>
#endif
#ifndef _WORKCTRL_HXX
#include <workctrl.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _RIBBAR_HRC
#include <ribbar.hrc>
#endif
#ifndef _NAVIPI_HRC
#include <navipi.hrc>
#endif
#ifndef _UTLUI_HRC
#include <utlui.hrc>
#endif

#define C2S(cChar) UniString::CreateFromAscii(cChar)

static const sal_Unicode cPrefix = '_';

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
    SwView *pView = ::GetActiveView();
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
 Beschreibung:  Erzeugen des automatischen Namens des unbenannten Merkers
------------------------------------------------------------------------*/

inline String &MakeAutoName(String &rToChange, USHORT i)
{
    rToChange = cPrefix;
    rToChange += String::CreateFromInt32( i );
    return rToChange;
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
 Beschreibung:  Select der Typenlistbox
------------------------------------------------------------------------*/
/*

IMPL_LINK( SwNavigationPI, TypeListBoxSelectHdl, ListBox *, pBox )
{
    USHORT nEntryIdx = pBox->GetSelectEntryPos();
    USHORT nCount =  pBox->GetEntryCount();
    if(nEntryIdx == nCount - 1)
    {
        if(!bPageCtrlsVisible)
        {
            bPageCtrlsVisible = TRUE;
            aContentToolBox.ShowItem( FN_PAGENUMBER );
            aContentToolBox.HideItem( FN_UP );
            aContentToolBox.HideItem( FN_DOWN );
        }
    }
    else
    {
        if(bPageCtrlsVisible)
        {
            aContentToolBox.HideItem( FN_PAGENUMBER );
            aContentToolBox.ShowItem( FN_UP );
            aContentToolBox.ShowItem( FN_DOWN );
            bPageCtrlsVisible = FALSE;
        }
    }

    pConfig->SetSelectedPos( nEntryIdx );
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
        SwView *pView = ::GetActiveView();
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
        SwView *pView = ::GetActiveView();
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
    const USHORT nId = pBox->GetCurItemId();
    SwView *pView = ::GetActiveView();
    if (!pView)
        return 1;
    SwWrtShell &rSh = pView->GetWrtShell();
    //MouseModifier fuer Outline-Move besorgen

    //Standard: Unterebenen werden mitgenommen
    // mit Ctrl Unterebenen nicht mitnehmen
    BOOL bOutlineWithChilds  = ( KEY_MOD1 != pBox->GetModifier());
    int nFuncId = 0;
    switch (nId)
    {
        case FN_UP:
        case FN_DOWN:
            pView->MoveNavigation(FN_DOWN == nId);
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
        //break;
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
                aGlobalTree.ExecCommand(nId);
            else
                aContentTree.ExecCommand(nId, bOutlineWithChilds);
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
        case FN_CREATE_NAVIGATION:
        {
        }
        break;
    }

    if (nFuncId)
    {
        lcl_UnSelectFrm(&rSh);
    }
    pView->GetEditWin().GrabFocus();
    return TRUE;
}
/*------------------------------------------------------------------------
 Beschreibung:  ClickHandler der Toolboxen
------------------------------------------------------------------------*/


IMPL_LINK( SwNavigationPI, ToolBoxClickHdl, ToolBox *, pBox )
{
    const USHORT nId = pBox->GetCurItemId();
    switch (nId)
    {
        case FN_DROP_REGION:
        {
            PopupMenu *pMenu = new PopupMenu;
            for (USHORT i = 0; i <= REGION_MODE_EMBEDDED; i++)
            {
                pMenu->InsertItem( i + 1, aContextArr[i] );
                pMenu->SetHelpId(i + 1, HID_NAVI_DRAG_HYP + i);
                    /*HID_NAVI_DRAG_HYP
                    HID_NAVI_DRAG_LINK
                    HID_NAVI_DRAG_COPY  */
            }
            pMenu->CheckItem( nRegionMode + 1 );
            pMenu->SetSelectHdl(LINK(this, SwNavigationPI, MenuSelectHdl));
            pMenu->Execute( pBox,
                    pBox->GetItemRect(FN_DROP_REGION).BottomLeft());
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
            pMenu->Execute( pBox,
                    pBox->GetItemRect(FN_OUTLINE_LEVEL).BottomLeft());
            delete pMenu;
            pBox->EndSelection();
            pBox->Invalidate();
        }
        break;
        case FN_GLOBAL_UPDATE:
        case FN_GLOBAL_OPEN:
        {
            aGlobalTree.TbxMenuHdl(nId, pBox);
        }
        break;
    }

    return TRUE;
}
/* -----------------------------19.09.00 15:16--------------------------------

 ---------------------------------------------------------------------------*/
SwNavHelpToolBox::SwNavHelpToolBox(SwNavigationPI* pParent, const ResId &rResId) :
            SwHelpToolBox(pParent, rResId)
{}
/*-----------------19.06.97 09:09-------------------

--------------------------------------------------*/
void __EXPORT SwNavHelpToolBox::MouseButtonDown(const MouseEvent &rEvt)
{
    if(rEvt.GetButtons() == MOUSE_LEFT &&
            FN_CREATE_NAVIGATION == GetItemId(rEvt.GetPosPixel()))
    {
        SfxBindings& rBind = ((SwNavigationPI*)GetParent())->GetCreateView()->GetViewFrame()->GetBindings();
        rBind.ENTERREGISTRATIONS();
        SwScrollNaviPopup* pPopup = new
            SwScrollNaviPopup(FN_SCROLL_NAVIGATION,
                            SW_RES(RID_SCROLL_NAVIGATION_WIN),
                            rBind);
        rBind.LEAVEREGISTRATIONS();

        Rectangle aRect = GetItemRect(FN_CREATE_NAVIGATION);
        aRect.SetPos(OutputToScreenPixel(aRect.TopLeft()));
        pPopup->StartPopupMode(aRect, FLOATWIN_POPUPMODE_RIGHT|FLOATWIN_POPUPMODE_ALLOWTEAROFF);
    }
    else
        SwHelpToolBox::MouseButtonDown(rEvt);
}

/*-----------------19.06.97 10:12-------------------

--------------------------------------------------*/
void  __EXPORT  SwNavHelpToolBox::RequestHelp( const HelpEvent& rHEvt )
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
    SwView *pView = ::GetActiveView();
    if (pView)
    {
        if(aPageChgTimer.IsActive())
            aPageChgTimer.Stop();
        pCreateView->GetWrtShell().GotoPage((USHORT)pEdit->GetValue());
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
    SwView *pView = ::GetActiveView();
    if (!pView)
        return 0;
    SwWrtShell &rSh = pView->GetWrtShell();

    const USHORT nPageCnt = rSh.GetPageCnt();
    pEdit->SetMax(nPageCnt);
    pEdit->SetLast(nPageCnt);
    return 0;
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

BOOL __EXPORT SwNavigationPI::Close()
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
    SwView *pView = ::GetActiveView();
    if (!pView)
        return;
    SwWrtShell &rSh = pView->GetWrtShell();

    const USHORT nBookCnt = rSh.GetBookmarkCnt();
    USHORT nMarkCount = 0;
    USHORT nFirstFound = MAX_MARKS;
    for (USHORT nCount = 0; nCount < nBookCnt; ++nCount)
    {
        SwBookmark& rBkmk = rSh.GetBookmark( nCount );
        if( rBkmk.IsMark() )
        {
            String aBookmark( rBkmk.GetName() );
            aBookmark.Erase(0, 1);
            nFirstFound = Min(nFirstFound, (USHORT)aBookmark.ToInt32());
            ++nMarkCount;
        }
    }
        // maximale Anzahl Bookmarks vergeben
    if (nAutoMarkIdx == MAX_MARKS)
        nAutoMarkIdx = 1;
        // erster freier neu vergeben
    else if (nFirstFound != MAX_MARKS)
        nAutoMarkIdx = Max(USHORT(1), USHORT(nFirstFound - 1));
    else
        ++nAutoMarkIdx;

    String aMark;
    MakeAutoName(aMark,nAutoMarkIdx);
    if (nMarkCount >= MAX_MARKS)
        rSh.DelBookmark( aMark );
    rSh.SetBookmark(KeyCode(), aMark, aEmptyStr, MARK);
    SwView::SetActMark( nAutoMarkIdx );
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

void SwNavigationPI::GotoPage()
{
    if ( pContextWin->GetFloatingWindow() && pContextWin->GetFloatingWindow()->IsRollUp())
        _ZoomIn();
    if(IsGlobalMode())
        ToggleTree();
    UsePage(0);
    GetPageEdit().GrabFocus();
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

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
            aGlobalTree.Show();
        }
        else
        {
            aContentTree.Show();
            aDocListBox.Show();
        }
        SvLBoxEntry* pFirst = aContentTree.FirstSelected();
        if(pFirst)
            aContentTree.Select(pFirst, TRUE); // toolbox enablen
        pConfig->SetSmall( FALSE );
        aContentToolBox.CheckItem(FN_SHOW_CONTENT_BOX);
    }
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

void SwNavigationPI::_ZoomIn()
{
    FloatingWindow* pFloat = pContextWin->GetFloatingWindow();
    if (pFloat &&
        (!_IsZoomedIn() || ( pContextWin->GetFloatingWindow()->IsRollUp())))
    {
        aContentTree.Hide();
        aDocListBox.Hide();
        aGlobalTree.Hide();
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
/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

void __EXPORT SwNavigationPI::Resize()
{
    Window* pParent = GetParent();
    FloatingWindow* pFloat =  ((DockingWindow*)pParent)->GetFloatingWindow();
    Size aNewSize;
    if( !_IsZoomedIn() )
    {
        if( pFloat)
        {
            aNewSize = pFloat->GetOutputSizePixel();
        }
        else
        {
            aNewSize = pParent->GetOutputSizePixel();
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
        // GlobalTree faengt weiter oben an und reicht bis ganz unten
        aNewSize.Height() += (nDist + nDocLBIniHeight + aPos.Y() - aGlobalTree.GetPosPixel().Y());
        aGlobalTree.SetSizePixel(aNewSize);
        aDocListBox.SetPosSizePixel( aLBPos.X(), aLBPos.Y(),
                                     aDocLBSz.Width(), aDocLBSz.Height(),
                                      WINDOW_POSSIZE_X|WINDOW_POSSIZE_Y|WINDOW_POSSIZE_WIDTH);

    }
}


/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

SwNavigationPI::SwNavigationPI( SfxBindings* pBindings,
                                SfxChildWindowContext* pCw,
                                Window* pParent) :

    Window( pParent, SW_RES(DLG_NAVIGATION_PI)),
    SfxControllerItem( SID_DOCFULLNAME, *pBindings ),
    aContentTree(this, SW_RES(TL_CONTENT)),
    aGlobalTree(this, SW_RES(TL_GLOBAL)),
    aContentToolBox(this, SW_RES(TB_CONTENT)),
    aGlobalToolBox(this, SW_RES(TB_GLOBAL)),
    aContentImageList(SW_RES(IL_CONTENT)),
    aDocListBox(this, SW_RES(LB_DOCS)),
    nActMark(0),
    nAutoMarkIdx(0),
    rBindings(*pBindings),
    bSmallMode(FALSE),
    bIsZoomedIn(FALSE),
    bPageCtrlsVisible(FALSE),
    bGlobalMode(FALSE),
    nRegionMode(REGION_MODE_NONE),
    pxObjectShell(0),
    pContentView(0),
    pContentWrtShell(0),
    pActContView(0),
    pContextWin(pCw),
    nWishWidth(0),
    pConfig(SW_MOD()->GetNavigationConfig()),
    pCreateView(::GetActiveView())
{

    for(USHORT k = 0; k < aContentToolBox.GetItemCount(); k++)
            aContentToolBox.SetItemImage(aContentToolBox.GetItemId(k),
                    aContentImageList.GetImage(aContentToolBox.GetItemId(k)));

    for( k = 0; k < aGlobalToolBox.GetItemCount(); k++)
            aGlobalToolBox.SetItemImage(aGlobalToolBox.GetItemId(k),
                    aContentImageList.GetImage(aGlobalToolBox.GetItemId(k)));

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

//  Rectangle aFirstRect = aContentToolBox.GetItemRect(FN_SHOW_ROOT);
//  USHORT nWidth = 2 * (USHORT)aFirstRect.Left();
    //doppelte Separatoren sind nicht erlaubt, also muss
    //die passende Groesse anders ermittelt werden
    Rectangle aFirstRect = aContentToolBox.GetItemRect(FN_DROP_REGION);
    Rectangle aSecondRect = aContentToolBox.GetItemRect(FN_ITEM_DOWN);
    USHORT nWidth = USHORT(aSecondRect.Left() - aFirstRect.Left());

    Size aItemWinSize( nWidth, aFirstRect.Bottom() - aFirstRect.Top() );
    pEdit->SetSizePixel(aItemWinSize);
    aContentToolBox.InsertWindow( FN_PAGENUMBER, pEdit, 0, 5);
    //aContentToolBox.InsertSeparator(4);
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
    Point aBoxPos = aContentTree.GetPosPixel();

    //Der linke und rechte Rand um die Toolboxen soll gleich sein
    nWishWidth = aContentToolBox.CalcWindowSizePixel().Width();
    nWishWidth += 2 * aContentToolBox.GetPosPixel().X();

//  Navi um Border vergroessern
//  FloatingWindow* pFloat =  ((DockingWindow*)pParent)->GetFloatingWindow();
//  if(pFloat)
    ((SfxDockingWindow*)pParent)->SetMinOutputSizePixel(Size(nWishWidth, nZoomOutInit));
    SetOutputSizePixel( Size( nWishWidth, nZoomOutInit));

    aContentTree.SetWindowBits( WB_HASBUTTONS|WB_HASBUTTONSATROOT|
                            WB_CLIPCHILDREN|WB_HSCROLL|WB_FORCE_MAKEVISIBLE );

    aContentTree.SetSpaceBetweenEntries(0);
    aContentTree.SetSelectionMode( SINGLE_SELECTION );
    aContentTree.SetDragDropMode(   SV_DRAGDROP_CTRL_MOVE |
                                    SV_DRAGDROP_CTRL_COPY |
                                    SV_DRAGDROP_ENABLE_TOP );
    aContentTree.EnableAsyncDrag(TRUE);
    aContentTree.Show();
    aContentToolBox.CheckItem(FN_SHOW_CONTENT_BOX, TRUE);

//  TreeListBox fuer Globaldokument
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
    aGlobalToolBox.SetClickHdl( LINK(this, SwNavigationPI, ToolBoxClickHdl) );
    aGlobalToolBox.CheckItem(FN_GLOBAL_SWITCH, TRUE);

#ifndef MAC
    Font aFont(aContentTree.GetFont());
    aFont.SetWeight(WEIGHT_NORMAL);
    aContentTree.SetFont(aFont);
    aGlobalTree.SetFont(aFont);
    GetPageEdit().SetFont(aFont);

#endif
#if defined( WIN ) || defined( WNT )
    Font aSwiss = System::GetStandardFont( STDFONT_SWISS );
    aContentTree.SetFont(aSwiss);
    aGlobalTree.SetFont(aSwiss);
#endif
    StartListening(*SFX_APP());
    SfxImageManager* pImgMan = SFX_APP()->GetImageManager();
    pImgMan->RegisterToolBox(&aContentToolBox, SFX_TOOLBOX_CHANGEOUTSTYLE);
    pImgMan->RegisterToolBox(&aGlobalToolBox, SFX_TOOLBOX_CHANGEOUTSTYLE);
    if(IsGlobalDoc())
    {
        SwView *pActView = ::GetActiveView();
        aGlobalToolBox.CheckItem(FN_GLOBAL_SAVE_CONTENT,
                    pActView->GetWrtShellPtr()->IsGlblDocSaveLinks());
        if(pConfig->IsGlobalActive())
            ToggleTree();
        else
            Application::PostUserEvent( LINK( this, SwNavigationPI, ReadOnlyHdl ) );
    }
    UsePage(0);
    aPageChgTimer.SetTimeoutHdl(LINK(this, SwNavigationPI, ChangePageHdl));
    aPageChgTimer.SetTimeout(PAGE_CHANGE_TIMEOUT);
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

__EXPORT SwNavigationPI::~SwNavigationPI()
{
    if(IsGlobalDoc() && !IsGlobalMode())
    {
        SwView *pView = ::GetActiveView();
        SwWrtShell &rSh = pView->GetWrtShell();
        if( !rSh.IsAllProtect() )
            pView->GetDocShell()->SetReadOnlyUI(FALSE);
    }

    EndListening(*SFX_APP());
    SfxImageManager* pImgMan = SFX_APP()->GetImageManager();
    pImgMan->ReleaseToolBox(&aContentToolBox);
    pImgMan->ReleaseToolBox(&aGlobalToolBox);
    delete aContentToolBox.GetItemWindow(FN_PAGENUMBER);
    delete aContentToolBox.GetItemWindow(FN_CONTENT_LB);
    if(pxObjectShell)
    {
        if(pxObjectShell->Is())
            (*pxObjectShell)->DoClose();
        delete pxObjectShell;
    }

}
/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

void __EXPORT SwNavigationPI::StateChanged( USHORT nSID, SfxItemState eState,
                                            const SfxPoolItem* pState )
{
    if(nSID == SID_DOCFULLNAME)
    {
        SwView *pActView = ::GetActiveView();
        if(pActView)
        {
            SwWrtShell* pWrtShell = pActView->GetWrtShellPtr();
            aContentTree.SetActiveShell(pWrtShell);
            BOOL bGlobal = IsGlobalDoc();
            aContentToolBox.EnableItem(FN_GLOBAL_SWITCH, bGlobal);
            if(!bGlobal && IsGlobalMode() ||
                    !IsGlobalMode() && pConfig->IsGlobalActive())
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

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/
/*

ListBox& SwNavigationPI::GetTypeSelBox()
{
    DBG_ASSERT(aContentToolBox.GetItemWindow(FN_CONTENT_LB),
                                "ItemWindow nicht vorhanden")
    return *(ListBox*)aContentToolBox.GetItemWindow(FN_CONTENT_LB);

}*/
/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

SfxChildAlignment __EXPORT SwNavigationPI::CheckAlignment
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

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

BOOL SwNavigationPI::IsInDrag() const
{
    return aContentTree.IsInternalDrag() || aContentTree.IsInDrag();
}

/*--------------------------------------------------------------------
    Beschreibung:   Benachrichtigung bei geaenderter DocInfo
 --------------------------------------------------------------------*/

void __EXPORT SwNavigationPI::Notify( SfxBroadcaster&, const SfxHint& rHint )
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

            SwView *pActView = ::GetActiveView();
            if(pActView)
            {
                SwWrtShell* pWrtShell = pActView->GetWrtShellPtr();
                aContentTree.SetActiveShell(pWrtShell);
                if(aGlobalTree.IsVisible())
                {
                    if(aGlobalTree.Update())
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

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwNavigationPI, MenuSelectHdl, Menu *, pMenu )
{
    USHORT nMenuId = pMenu->GetCurItemId();
    if(nMenuId != USHRT_MAX)
    {
        if(nMenuId < 100)
            SetRegionDropMode( --nMenuId);
        else
            aContentTree.SetOutlineLevel(nMenuId - 100);
    }
    return 0;
}


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwNavigationPI::UpdateListBox()
{
    aDocListBox.SetUpdateMode(FALSE);
    aDocListBox.Clear();
    SwView *pActView = ::GetActiveView();
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
        String sEntry = pView->GetDocShell()->GetTitle();
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

        pView = SwModule::GetNextView(pView);
        nCount++;
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
/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

SwNavigationChild::SwNavigationChild( Window* pParent,
                        USHORT nId,
                        SfxBindings* pBindings,
                        SfxChildWinInfo* pInfo ) :
                        SfxChildWindowContext( nId )
{
    SwNavigationPI* pNavi  = new SwNavigationPI( pBindings, this, pParent );
    SetWindow( pNavi );
    ::GetActiveView()->GetViewFrame()->GetBindings().Invalidate(SID_NAVIGATOR);
    String sExtra = pInfo->aExtraString;

    SwNavigationConfig* pNaviConfig = SW_MOD()->GetNavigationConfig();

    USHORT nRootType = pNaviConfig->GetRootType();
    if( nRootType < CONTENT_TYPE_MAX )
    {
        pNavi->aContentTree.SetRootType(nRootType);
        pNavi->aContentToolBox.CheckItem(FN_SHOW_ROOT, TRUE);
    }
    pNavi->aContentTree.SetOutlineLevel(pNaviConfig->GetOutlineLevel());
    pNavi->SetRegionDropMode(pNaviConfig->GetRegionMode());


    if(GetFloatingWindow() && pNaviConfig->IsSmall())
    {
        pNavi->_ZoomIn();
    }

}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

BOOL    __EXPORT SwNavigationPI::Drop( const DropEvent& )
{
    if(!aContentTree.IsInDrag())
    {
        String aFileName;

        const USHORT nCount = DragServer::GetItemCount();

        for ( USHORT i = 0; i < nCount; ++i )
        {
            aFileName = SwNavigationPI::CreateDropFileName(i);

            if(aFileName.Len() &&
                STRING_NOTFOUND == aFileName.Search('#')
                    && (!sContentFileName.Len() || sContentFileName != aFileName ))
            {
                aFileName.EraseTrailingChars(char(0));
                sContentFileName = aFileName;
                if(pxObjectShell)
                {
                    aContentTree.SetHiddenShell( 0 );
                    (*pxObjectShell)->DoClose();
                    DELETEZ( pxObjectShell);
                }
                SfxStringItem aFileItem(SID_FILE_NAME, aFileName);
                String sOptions = C2S("HRC");
                SfxStringItem aOptionsItem(SID_OPTIONS,sOptions);
                SfxLinkItem aLink(SID_DONELINK, LINK(this, SwNavigationPI, DoneLink));
                GetActiveView()->GetViewFrame()->GetDispatcher()->Execute(
                    SID_OPENDOC, SFX_CALLMODE_ASYNCHRON,
                    &aFileItem, &aOptionsItem, &aLink, 0L);
            }
        }
    }
    return TRUE;
}

/*-----------------16.06.97 15:05-------------------

--------------------------------------------------*/
String  SwNavigationPI::CreateDropFileName( USHORT nItem )
{
    INetBookmark aBkmk( aEmptyStr, aEmptyStr );
    String aFileName;

    if( aBkmk.PasteDragServer( nItem ) )
        aFileName = aBkmk.GetURL();
    else if(DragServer::HasFormat( nItem, FORMAT_FILE))
        aFileName = DragServer::PasteFile( nItem );
    else if( DragServer::HasFormat( nItem, SOT_FORMATSTR_ID_FILENAME ))
    {
        ULONG nLen = DragServer::GetDataLen( nItem, SOT_FORMATSTR_ID_FILENAME );
        DragServer::PasteData( nItem, aFileName.AllocBuffer( nLen/2 ),
                                nLen, SOT_FORMATSTR_ID_FILENAME );

    }
    return aFileName;
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

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
            DBG_ASSERT(pContentView, "keine SwView")
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
/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

BOOL    __EXPORT SwNavigationPI::QueryDrop( DropEvent& rEvt )
{
    rEvt.SetAction(DROP_COPY);
    if (!aContentTree.IsInDrag())
    {
        const SotDataObject& rData = *rEvt.GetData();
        const SvDataTypeList& rTypeLst = rData.GetTypeList();

        if( rTypeLst.Get( FORMAT_FILE ) )
        {
            String aFileName;
            SvData aData( FORMAT_FILE, MEDIUM_ALL );
            if( aData.GetData( aFileName ) )
            {
                GraphicDescriptor aDesc( aFileName );
                if( aDesc.Detect() )    // keine Grafiken annehmen
                    return FALSE;
                return TRUE;
            }
            return FALSE;
        }

        if( rTypeLst.Get( FORMAT_STRING ) || INetBookmark::HasFormat( rData ))
            return TRUE;
    }

    return FALSE;
}
/*-----------------27.11.96 13.00-------------------

--------------------------------------------------*/

void SwNavigationPI::SetRegionDropMode(USHORT nNewMode)
{
    nRegionMode = nNewMode;
    pConfig->SetRegionMode( nRegionMode );

    USHORT nId = FN_DROP_REGION;
    if(nRegionMode == REGION_MODE_LINK)
        nId = FN_DROP_REGION_LINK;
    else if(nRegionMode == REGION_MODE_EMBEDDED)
        nId = FN_DROP_REGION_COPY;

    aContentToolBox.SetItemImage(FN_DROP_REGION,
                aContentImageList.GetImage(nId));
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

BOOL    __EXPORT SwNavigationChild::Drop( const DropEvent& rEvt)
{
    return GetWindow()->Drop(rEvt);
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

BOOL    __EXPORT SwNavigationChild::QueryDrop( DropEvent& rEvt)
{
    return GetWindow()->QueryDrop(rEvt);
}


/*-----------------12.06.97 09:47-------------------

--------------------------------------------------*/

BOOL    SwNavigationPI::ToggleTree()
{
    BOOL bRet = TRUE;
    BOOL bGlobalDoc = IsGlobalDoc();
    if(!IsGlobalMode() && bGlobalDoc)
    {
        SetUpdateMode(FALSE);
        if(_IsZoomedIn())
            _ZoomOut();
        aGlobalTree.Show();
        aGlobalToolBox.Show();
        aContentTree.Hide();
        aContentToolBox.Hide();
        aDocListBox.Hide();
        SetGlobalMode(TRUE);
        SetUpdateMode(TRUE);
    }
    else
    {
        aGlobalTree.Hide();
        aGlobalToolBox.Hide();
        if(!_IsZoomedIn())
        {
            aContentTree.Show();
            aContentToolBox.Show();
            aDocListBox.Show();
        }
        bRet = FALSE;
        SetGlobalMode(FALSE);
    }
    if(bGlobalDoc)
    {
        Application::PostUserEvent( LINK( this, SwNavigationPI, ReadOnlyHdl ) );
    }
    return bRet;
}

/*-----------------13.06.97 09:42-------------------

--------------------------------------------------*/
BOOL    SwNavigationPI::IsGlobalDoc() const
{
    BOOL bRet = FALSE;
    SwView *pView = ::GetActiveView();
    if(pView)
    {
        SwWrtShell &rSh = pView->GetWrtShell();
        bRet = rSh.IsGlobalDoc();
    }
    return bRet;
}
/*-----------------19.06.97 08:21-------------------

--------------------------------------------------*/
IMPL_LINK( SwNavigationPI, ReadOnlyHdl, void *, EMPTYARG )
{
/*
JP 25.01.99: die Umschaltung auf UI-Readonly wird nicht mehr benoetigt, weil
            der Cursor jetzt in Readonly-Inhalt gesetzt werden kann.

    if(IsGlobalMode())
    {
        SwView *pView = ::GetActiveView();
        SwWrtShell &rSh = pView->GetWrtShell();
        if( !rSh.IsAllProtect() )
            pView->GetDocShell()->SetReadOnlyUI(FALSE);
    }
    else
    {
        SwView *pView = ::GetActiveView();
        SwWrtShell &rSh = pView->GetWrtShell();
        if( !rSh.IsReadOnlyAvailable() )
        {
//          SfxObjectShell* pObjShell = SfxObjectShell::Current();
//          pObjShell->SetReadOnlyUI();
            pView->GetDocShell()->SetReadOnlyUI( TRUE );
        }
    }
*/
    return 0;
}
/* -----------------26.10.98 08:10-------------------
 *
 * --------------------------------------------------*/
IMPL_LINK( SwNavigationPI, ChangePageHdl, Timer*, EMPTYARG )
{
    EditAction(&GetPageEdit());
    GetPageEdit().GrabFocus();
    return 0;
}
/* -----------------26.10.98 08:14-------------------
 *
 * --------------------------------------------------*/
IMPL_LINK( SwNavigationPI, PageEditModifyHdl, Edit*, EMPTYARG )
{
    if(aPageChgTimer.IsActive())
        aPageChgTimer.Stop();
    aPageChgTimer.Start();
    return 0;
}

