/*************************************************************************
 *
 *  $RCSfile: navipi.cxx,v $
 *
 *  $Revision: 1.29 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:10:07 $
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



#pragma hdrstop

#define NAVIPI_CXX

#include <string> // HACK: prevent conflict between STLPORT and Workshop headers

#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _URLBMK_HXX
#include <svtools/urlbmk.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _FILTER_HXX
#include <svtools/filter.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif
#ifndef _FILELIST_HXX
#include <sot/filelist.hxx>
#endif
#ifndef _SFXEVENT_HXX
#include <sfx2/event.hxx>
#endif
#ifndef _SFXIMGMGR_HXX
#include <sfx2/imgmgr.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXDOCKWIN_HXX
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
    const USHORT nId = pBox->GetCurItemId();
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
            CreateNavigationTool(pBox->GetItemRect(FN_CREATE_NAVIGATION), TRUE);
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
    const USHORT nId = pBox->GetCurItemId();
    switch (nId)
    {
        case FN_GLOBAL_UPDATE:
        case FN_GLOBAL_OPEN:
        {
            aGlobalTree.TbxMenuHdl(nId, pBox);
        }
        break;
    }

    return TRUE;
}

/*-----------------13.07.04 -------------------
 ----------------------------------------------*/

IMPL_LINK( SwNavigationPI, ToolBoxDropdownClickHdl, ToolBox*, pBox )
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
            pBox->SetItemDown( nId, TRUE );
            pMenu->Execute( pBox,
                    pBox->GetItemRect(FN_DROP_REGION),
                    POPUPMENU_EXECUTE_DOWN );
            pBox->SetItemDown( nId, FALSE );
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
            pBox->SetItemDown( nId, TRUE );
            pMenu->Execute( pBox,
                    pBox->GetItemRect(FN_OUTLINE_LEVEL),
                    POPUPMENU_EXECUTE_DOWN );
            pBox->SetItemDown( nId, FALSE );
            delete pMenu;
            pBox->EndSelection();
            pBox->Invalidate();
        }
        break;
    }
    return TRUE;
}

/*-----------------13.07.04 -------------------
--------------------------------------------------*/

SwNavHelpToolBox::SwNavHelpToolBox(SwNavigationPI* pParent, const ResId &rResId) :
            SwHelpToolBox(pParent, rResId)
{}
/*-----------------19.06.97 09:09-------------------

--------------------------------------------------*/
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
/* -----------------------------12.03.2002 16:55------------------------------

 ---------------------------------------------------------------------------*/
void SwNavigationPI::CreateNavigationTool(const Rectangle& rRect, BOOL bSetFocus)
{
//    SfxBindings& rBind = GetCreateView()->GetViewFrame()->GetBindings();
//    rBind.ENTERREGISTRATIONS();
    Reference< XFrame > xFrame = GetCreateView()->GetViewFrame()->GetFrame()->GetFrameInterface();
    SwScrollNaviPopup* pPopup = new
        SwScrollNaviPopup(FN_SCROLL_NAVIGATION,
                          xFrame );
//    rBind.LEAVEREGISTRATIONS();

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

/*-----------------19.06.97 10:12-------------------

--------------------------------------------------*/
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
    SwView *pView = GetCreateView();
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
    aContentImageListH(SW_RES(ILH_CONTENT)),
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
    pCreateView(0),
    pPopupWindow(0),
    pFloatingWindow(0)
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

    FloatingWindow* pFloat =  ((DockingWindow*)pParent)->GetFloatingWindow();
    Size aMinSize(pFloat ? nWishWidth : 0, pFloat ? nZoomOutInit : 0);
    ((SfxDockingWindow*)pParent)->SetMinOutputSizePixel(aMinSize);
    SetOutputSizePixel( Size( nWishWidth, nZoomOutInit));
    Size aTmpParentSize(((SfxDockingWindow*)pParent)->GetSizePixel());
    if(aTmpParentSize.Width() < aMinSize.Width() ||
        aTmpParentSize.Height() < aMinSize.Height() &&
            ((SfxDockingWindow*)pParent)->GetFloatingWindow() &&
                !((SfxDockingWindow*)pParent)->GetFloatingWindow()->IsRollUp())
        ((SfxDockingWindow*)pParent)->SetOutputSizePixel(aMinSize);

    aContentTree.SetWindowBits( WB_HASBUTTONS|WB_HASBUTTONSATROOT|
                            WB_CLIPCHILDREN|WB_HSCROLL|WB_FORCE_MAKEVISIBLE );
    aContentTree.SetSpaceBetweenEntries(3);
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
    SfxImageManager* pImgMan = pBindings->GetImageManager();
    pImgMan->RegisterToolBox(&aContentToolBox, SFX_TOOLBOX_CHANGEOUTSTYLE);
    pImgMan->RegisterToolBox(&aGlobalToolBox, SFX_TOOLBOX_CHANGEOUTSTYLE);
    if(IsGlobalDoc())
    {
        SwView *pActView = GetCreateView();
        aGlobalToolBox.CheckItem(FN_GLOBAL_SAVE_CONTENT,
                    pActView->GetWrtShellPtr()->IsGlblDocSaveLinks());
        if(pConfig->IsGlobalActive())
            ToggleTree();
    }
    UsePage(0);
    aPageChgTimer.SetTimeoutHdl(LINK(this, SwNavigationPI, ChangePageHdl));
    aPageChgTimer.SetTimeout(PAGE_CHANGE_TIMEOUT);
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

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

    SfxImageManager* pImgMan = GetBindings().GetImageManager();
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
    delete pPopupWindow;
    delete pFloatingWindow;

}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

void SwNavigationPI::SetPopupWindow( SfxPopupWindow* pWindow )
{
    pPopupWindow = pWindow;
    pPopupWindow->SetPopupModeEndHdl( LINK( this, SwNavigationPI, PopupModeEndHdl ));
    pPopupWindow->SetDeleteLink_Impl( LINK( this, SwNavigationPI, ClosePopupWindow ));
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

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

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

IMPL_LINK( SwNavigationPI, ClosePopupWindow, SfxPopupWindow *, pWindow )
{
    if ( pWindow == pFloatingWindow )
        pFloatingWindow = 0;
    else
        pPopupWindow = 0;

    return 1;
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

void SwNavigationPI::StateChanged( USHORT nSID, SfxItemState eState,
                                            const SfxPoolItem* pState )
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

/*-----------------16.06.97 15:05-------------------

--------------------------------------------------*/

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
        sFileName = URIHelper::SmartRelToAbs( sFileName );
    return sFileName;
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

sal_Int8 SwNavigationPI::AcceptDrop( const AcceptDropEvent& rEvt )
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
        GraphicDescriptor aDesc( sFileName );
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

    ImageList& rImgLst = aContentToolBox.GetDisplayBackground().GetColor().IsDark()
                ? aContentImageListH : aContentImageList;

    aContentToolBox.SetItemImage( FN_DROP_REGION,
                                    rImgLst.GetImage(nId));
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
    return bRet;
}

/*-----------------13.06.97 09:42-------------------

--------------------------------------------------*/
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

/* -----------------------------23.04.01 07:34--------------------------------

 ---------------------------------------------------------------------------*/
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


/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

SwNavigationChild::SwNavigationChild( Window* pParent,
                        USHORT nId,
                        SfxBindings* pBindings,
                        SfxChildWinInfo* pInfo )
    : SfxChildWindowContext( nId )
{
    SwNavigationPI* pNavi  = new SwNavigationPI( pBindings, this, pParent );
    SetWindow( pNavi );
    pBindings->Invalidate(SID_NAVIGATOR);
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
/* -----------------------------06.05.2002 10:06------------------------------

 ---------------------------------------------------------------------------*/
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
/* -----------------------------06.05.2002 10:07------------------------------

 ---------------------------------------------------------------------------*/
void SwNavigationPI::InitImageList()
{
    USHORT k;

    ImageList& rImgLst = aContentToolBox.GetDisplayBackground().GetColor().IsDark() ?
                aContentImageListH : aContentImageList;
    for( k = 0; k < aContentToolBox.GetItemCount(); k++)
            aContentToolBox.SetItemImage(aContentToolBox.GetItemId(k),
                    rImgLst.GetImage(aContentToolBox.GetItemId(k)));

    for( k = 0; k < aGlobalToolBox.GetItemCount(); k++)
            aGlobalToolBox.SetItemImage(aGlobalToolBox.GetItemId(k),
                    rImgLst.GetImage(aGlobalToolBox.GetItemId(k)));

    USHORT nId = FN_DROP_REGION;
    if(nRegionMode == REGION_MODE_LINK)
        nId = FN_DROP_REGION_LINK;
    else if(nRegionMode == REGION_MODE_EMBEDDED)
        nId = FN_DROP_REGION_COPY;
    aContentToolBox.SetItemImage( FN_DROP_REGION,
                                    rImgLst.GetImage(nId));
}

