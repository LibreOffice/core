/*************************************************************************
 *
 *  $RCSfile: viewmdi.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:49 $
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

#include <string> // HACK: prevent conflict between STLPORT and Workshop headers

#include "hintids.hxx"
#include "uiparam.hxx"

#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SVX_RULER_HXX //autogen
#include <svx/ruler.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_SRCHITEM_HXX //autogen
#include <svx/srchitem.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif


#include "swmodule.hxx"
#include "view.hxx"
#include "wrtsh.hxx"
#include "viewopt.hxx"
#include "frmatr.hxx"
#include "globals.hrc"
#include "docsh.hxx"
#include "uitool.hxx"
#include "cmdid.h"
#include "edtwin.hxx"
#include "pagedesc.hxx"
#include "bookmrk.hxx"
#include "fldbas.hxx"
#include "workctrl.hxx"
#include "usrpref.hxx"
#include "scroll.hxx"
#include "wview.hxx"
#include "view.hrc"
#include "ribbar.hrc"
#include "helpid.h"

USHORT          SwView::nMoveType = NID_PGE;
BYTE            SwView::nActMark = 0;

#define VIEW_IMAGECOLOR COL_LIGHTBLUE

void SwView::SetZoom( SvxZoomType eZoomType, short nFactor, BOOL bViewOnly )
{
    _SetZoom( GetEditWin().GetOutputSizePixel(), eZoomType, nFactor, bViewOnly );
}

void SwView::_SetZoom( const Size &rEditSize, SvxZoomType eZoomType,
                            short nFactor, BOOL bViewOnly )
{
    pWrtShell->LockPaint();
    {
    ACT_KONTEXT(pWrtShell);

    long nFac = nFactor;

    BOOL bWeb = 0 != PTR_CAST(SwWebView, this);
    SwMasterUsrPref *pUsrPref = (SwMasterUsrPref*)SW_MOD()->GetUsrPref(bWeb);

    const SwPageDesc &rDesc = pWrtShell->GetPageDesc(
                                                pWrtShell->GetCurPageDesc() );
    const SvxLRSpaceItem &rLRSpace = rDesc.GetMaster().GetLRSpace();
    const SwViewOption *pOpt = pWrtShell->GetViewOptions();
    long lLeftMargin;

    if( eZoomType != SVX_ZOOM_PERCENT )
    {
        const long nOf = DOCUMENTBORDER * 2L;
        Size aPageSize( pWrtShell->GetAnyCurRect(RECT_PAGE_CALC).SSize() );

        if( PD_MIRROR == rDesc.GetUseOn() ) // gespiegelte Seiten
        {
            const SvxLRSpaceItem &rLeftLRSpace = rDesc.GetLeft().GetLRSpace();
            aPageSize.Width() +=
                Abs( long(rLeftLRSpace.GetLeft()) - long(rLRSpace.GetLeft()) );
        }
        if( SVX_ZOOM_OPTIMAL == eZoomType )
        {
            aPageSize.Width() -=
                ( rLRSpace.GetLeft() + rLRSpace.GetRight() + nLeftOfst * 2 );
        }
        else
        {
            aPageSize.Width() += nOf;
            aPageSize.Height() += nOf;
        }
        lLeftMargin = SVX_ZOOM_PAGEWIDTH != eZoomType ?
            long(rLRSpace.GetLeft()) + DOCUMENTBORDER + nLeftOfst : 0L;

        const MapMode aTmpMap( MAP_TWIP );
        const Size aWindowSize( GetEditWin().PixelToLogic( rEditSize, aTmpMap ) );
        nFac = aWindowSize.Width() * 100 / aPageSize.Width();
        if( SVX_ZOOM_WHOLEPAGE == eZoomType )
        {
            long nVisPercent = aWindowSize.Height() * 100 / aPageSize.Height();
            nFac = Min( nFac, nVisPercent );
        }
    }
    else
        lLeftMargin = long(rLRSpace.GetLeft()) + DOCUMENTBORDER;

    nFac = Max( long( MINZOOM ), nFac );

    SwViewOption aOpt( *pOpt );
    SwDocShell* pDocShell = GetDocShell();
    if ( pDocShell->GetProtocol().IsEmbed() ||
         pDocShell->GetCreateMode() == SFX_CREATE_MODE_STANDARD  )
    {
        //MasterUsrPrefs updaten UND DANACH die ViewOptions der aktuellen
        //View updaten.
        if ( !bViewOnly &&
                (USHORT(nFac)      != pUsrPref->GetZoom() ||
                BYTE  (eZoomType) != pUsrPref->GetZoomType()) )
        {
            pUsrPref->SetZoom    ( USHORT(nFac) );
            pUsrPref->SetZoomType( BYTE( eZoomType ) );
            SW_MOD()->ApplyUsrPref( *pUsrPref, 0 );
            pUsrPref->SetDefault ( FALSE );
        }
        if ( pOpt->GetZoom() != (USHORT) nFac )
        {
            aOpt.SetZoom    ( USHORT(nFac) );
            aOpt.SetReadonly(pOpt->IsReadonly());
            pWrtShell->ApplyViewOptions( aOpt );
        }
        if ( eZoomType != SVX_ZOOM_PERCENT )
        {
            Point aPos;
            if ( eZoomType == SVX_ZOOM_WHOLEPAGE )
                aPos.Y() = pWrtShell->GetAnyCurRect(RECT_PAGE).Top() - DOCUMENTBORDER;
            else
            {
                //sicherstellen, dass sich der Cursor im sichtbaren
                //Bereich befindet, damit nur 1x gescrollt wird
                aPos.X() = lLeftMargin;
                const SwRect &rCharRect = pWrtShell->GetCharRect();
                if ( rCharRect.Top() > GetVisArea().Bottom() ||
                     rCharRect.Bottom() < aPos.Y() )
                    aPos.Y() = rCharRect.Top() - rCharRect.Height();
                else
                    aPos.Y() = GetVisArea().Top();
            }
            SetVisArea( aPos );
        }
        // OS: Notloesung - in CalcVisArea wird u.U. wieder SetZoom gerufen und
        // dann werden falsche Werte eingestellt
        ((SwViewOption*)pWrtShell->GetViewOptions())->SetZoomType((BYTE)eZoomType);
        CalcVisArea( rEditSize );   //fuer das Neuberechnen des sichtbaren Bereiches
    }
    else if ( USHORT(nFac) != pOpt->GetZoom() )
    {
        aOpt.SetZoom    ( USHORT(nFac) );
        pWrtShell->ApplyViewOptions( aOpt );
    }

    const Fraction aFrac( nFac, 100 );
    if ( pVLineal )
    {
        pVLineal->SetZoom( aFrac );
        pVLineal->ForceUpdate();
    }
    if ( pHLineal )
    {
        pHLineal->SetZoom( aFrac );
        pHLineal->ForceUpdate();
    }
    ((SwViewOption*)pWrtShell->GetViewOptions())->SetZoomType((BYTE)eZoomType);
    }
    pWrtShell->UnlockPaint();
//  eZoom = eZoomType;
}

/*
 * Scrollbar - Handler
 */

int SwView::_CreateScrollbar( int bHori )
{
    Window *pMDI = &GetViewFrame()->GetWindow();
    SwScrollbar** ppScrollbar = bHori ? &pHScrollbar : &pVScrollbar;

    ASSERT( !*ppScrollbar, "vorher abpruefen!" )

    if( !bHori )
        CreatePageButtons( !bShowAtResize );

    // wenn beide Scrollbar eingeschaltet werden, dann auch die ScrollbarBox
    // anlegen
    if( !pScrollFill && (bHori ? pVScrollbar : pHScrollbar) )
    {
        pScrollFill = new ScrollBarBox( pMDI, GetDocShell()->IsInFrame()
                                                ? 0 : WB_SIZEABLE );
        if ( !bShowAtResize )
            pScrollFill->Show();
    }

    *ppScrollbar = new SwScrollbar( pMDI, bHori );
    UpdateScrollbars();
    if(bHori)
        (*ppScrollbar)->SetScrollHdl( LINK( this, SwView, EndScrollHdl ));
    else
        (*ppScrollbar)->SetScrollHdl( LINK( this, SwView, ScrollHdl ));
    (*ppScrollbar)->SetEndScrollHdl( LINK( this, SwView, EndScrollHdl ));

    (*ppScrollbar)->EnableDrag( TRUE );
    (*ppScrollbar)->SetAuto( pWrtShell->IsBrowseMode() &&
                             !GetDocShell()->GetProtocol().IsInPlaceActive() );

    InvalidateBorder();

    // Scrollbar muss nochmals getestet werden, da im InvalidateBorder u.U. der
    // Scrollbar wieder geloescht wurde
    if ( !bShowAtResize && (*ppScrollbar))
        (*ppScrollbar)->Show();

    return 1;
}

void SwView::CreatePageButtons(BOOL bShow)
{
    Window *pMDI = &GetViewFrame()->GetWindow();
    pPageUpBtn      = new SwHlpImageButton(pMDI, SW_RES( BTN_PAGEUP ), TRUE );
    pPageUpBtn->SetHelpId(HID_SCRL_PAGEUP);
    pPageDownBtn    = new SwHlpImageButton(pMDI, SW_RES( BTN_PAGEDOWN ), FALSE );
    pPageDownBtn->SetHelpId(HID_SCRL_PAGEDOWN);
    pNaviBtn = new SwNaviImageButton(pMDI);
    pNaviBtn->SetHelpId(HID_SCRL_NAVI);
    Link aLk( LINK( this, SwView, BtnPage ) );
    pPageUpBtn->SetClickHdl( aLk );
    pPageDownBtn->SetClickHdl( aLk );
    if(nMoveType != NID_PGE)
    {
        Color aColor(VIEW_IMAGECOLOR);
        SetImageButtonColor(aColor);
    }

    if(bShow)
    {
        pPageUpBtn->Show();
        pPageDownBtn->Show();
        pNaviBtn->Show();
    }
};

int SwView::_KillScrollbar( int bHori )
{
    SwScrollbar** ppScrBar;
    if( bHori )
    {
        if( 0 == *( ppScrBar = &pHScrollbar ) )
            return 1;
    }
    else
    {
        if( 0 == *( ppScrBar = &pVScrollbar ) )
            return 1;
        DELETEZ(pNaviBtn);
        DELETEZ(pPageUpBtn);
        DELETEZ(pPageDownBtn);
    }
    DELETEZ( *ppScrBar );

    // wird einer der Scrollbar ausgeschaltet, muss auch die ScrollbarBox
    // entfernt werden
    if( pScrollFill )
        DELETEZ( pScrollFill );

    InvalidateBorder();
    return 1;
}

/*
 * Button-Handler
 */
IMPL_LINK( SwView, BtnPage, Button *, pButton )
{
    MoveNavigation(pButton == pPageDownBtn);
    return 0;
}
/*-----------------20.06.97 10:46-------------------

--------------------------------------------------*/
void SwView::MoveNavigation(BOOL bNext)
{
    SwWrtShell& rSh = GetWrtShell();
    switch( nMoveType )
    {
        case NID_PGE:
            bNext ? PhyPageDown() : PhyPageUp();
        break;
        case NID_TBL :
            rSh.EnterStdMode();
            if(bNext)
                rSh.MoveTable(fnTableNext, fnTableStart);
            else
                rSh.MoveTable(fnTablePrev, fnTableStart);
        break;
        case NID_FRM :
        case NID_GRF:
        case NID_OLE:
        {
            FlyCntType eType = FLYCNTTYPE_FRM;
            if(nMoveType == NID_GRF)
                eType = FLYCNTTYPE_GRF;
            else if(nMoveType == NID_OLE)
                eType = FLYCNTTYPE_OLE;
            BOOL bSuccess = bNext ?
                    rSh.GotoNextFly(eType) :
                        rSh.GotoPrevFly(eType);
            if(bSuccess)
            {
                rSh.HideCrsr();
                rSh.EnterSelFrmMode();
            }
        }
        break;
        case NID_DRW :
        case NID_CTRL:
            rSh.GotoObj(bNext,
                    nMoveType == NID_DRW ?
                        DRAW_SIMPLE :
                            DRAW_CONTROL);
        break;
        case NID_REG :
            rSh.EnterStdMode();
            if(bNext)
                rSh.MoveRegion(fnRegionNext, fnRegionStart);
            else
                rSh.MoveRegion(fnRegionPrev, fnRegionStart);

        break;
        case NID_BKM :
            rSh.EnterStdMode();
            GetViewFrame()->GetDispatcher()->Execute(bNext ?
                                        FN_NEXT_BOOKMARK :
                                            FN_PREV_BOOKMARK);
        break;
        case NID_OUTL:
            rSh.EnterStdMode();
            bNext ? rSh.GotoNextOutline() : rSh.GotoPrevOutline();
        break;
        case NID_SEL :
            bNext ? rSh.GoNextCrsr() : rSh.GoPrevCrsr();
        break;
        case NID_FTN:
            rSh.EnterStdMode();
            bNext ?
                rSh.GotoNextFtnAnchor() :
                    rSh.GotoPrevFtnAnchor();
        break;
        case NID_MARK:
        {
            // Selektionen aufheben
            rSh.MoveCrsr();
            rSh.EnterStdMode();
            const USHORT nBookCnt = rSh.GetBookmarkCnt();
            USHORT nMarkCount = 0;
            USHORT nRealIdx[MAX_MARKS];
            for( USHORT nCount = 0; nCount < nBookCnt; ++nCount )
            {
                if( rSh.GetBookmark( nCount ).IsMark() )
                {
                    nRealIdx[nMarkCount] = nCount;
                    ++nMarkCount;
                }
            }
            if (nMarkCount)
            {
                if(!bNext)
                {
                    if (nActMark > 1)
                        --nActMark;
                    else
                        nActMark = nMarkCount;
                }
                else
                {
                    ++nActMark;
                    if (nActMark > MAX_MARKS || nActMark > nMarkCount)
                        nActMark = 1;
                }

                rSh.GotoBookmark( nRealIdx[nActMark - 1] );
            }
        }
        break;
        case NID_POSTIT:
        {

            SwFieldType* pFldType = rSh.GetFldType(0, RES_POSTITFLD);
            rSh.MoveFldType( pFldType, bNext );
        }
        break;
        case NID_SRCH_REP:
        if(pSrchItem)
        {
            BOOL bBackward = pSrchItem->GetBackward();
            if(rSh.HasSelection() && !bNext == rSh.IsCrsrPtAtEnd())
                rSh.SwapPam();
            pSrchItem->SetBackward(!bNext);
            SfxRequest aReq(FN_REPEAT_SEARCH, SFX_CALLMODE_SLOT, GetPool());
            ExecSearch(aReq);
            pSrchItem->SetBackward(bBackward);
        }
        break;
        case NID_INDEX_ENTRY:
            rSh.GotoNxtPrvTOXMark(bNext);
        break;

        case NID_TABLE_FORMULA:
            rSh.GotoNxtPrvTblFormula( bNext );
            break;

        case NID_TABLE_FORMULA_ERROR:
            rSh.GotoNxtPrvTblFormula( bNext, TRUE );
            break;
    }
    pEditWin->GrabFocus();
}

/*************************************************************************
|*
|*  SwView::CreateTab()
|*
|*  Beschreibung
|*  Ersterstellung      VB 29.05.91
|*  Letzte Aenderung    OS 09.05.96
|*
*************************************************************************/

int SwView::_CreateTab()
{
    ASSERT( !StatTab(), "vorher abpruefen!" )

    pHLineal->SetActive(GetFrame() && IsActive());

    pHLineal->Show();
    InvalidateBorder();
    return 1;
}

/*************************************************************************
|*
|*  SwView::KillTab()
|*
|*  Beschreibung
|*  Ersterstellung      VB 29.05.91
|*  Letzte Aenderung    OS 09.05.96
|*
*************************************************************************/

int SwView::_KillTab()
{
    ASSERT( StatTab(), "vorher abpruefen!" )

    pHLineal->Hide();
//  DELETEZ(pHLineal);
    InvalidateBorder();
    return 1;
}

void SwView::ChangeTabMetric( FieldUnit eUnit )
{
    if ( pHLineal )
    {
        if(eUnit == FUNIT_MM)
            eUnit = FUNIT_CM;
        if(pHLineal->GetUnit() != eUnit )
        {
            pHLineal->SetUnit( eUnit );
            pHLineal->Invalidate();
        }
    }
}

void SwView::ChangeVLinealMetric( FieldUnit eUnit )
{
    if ( pVLineal )
    {
        if(eUnit == FUNIT_MM)
            eUnit = FUNIT_CM;
        if(pVLineal->GetUnit() != eUnit)
        {
            pVLineal->SetUnit( eUnit );
            pVLineal->Invalidate();
        }
    }
}

/*************************************************************************
|*
|*  SwView::CreateVLineal()
|*
|*  Beschreibung
|*  Ersterstellung  VB 29.05.91
|*  Letzte Aenderung  VB 19.05.92
|*
*************************************************************************/

int SwView::_CreateVLineal()
{
    ASSERT( !StatVLineal(), "vorher abpruefen!" )

    Window *pMDI = &GetViewFrame()->GetWindow();

    pVLineal = new SvxRuler(pMDI, pEditWin,
                            0,
                            GetViewFrame()->GetBindings(),
                            WB_VSCROLL |  WB_3DLOOK | WB_BORDER );
    if( pVLineal && pHLineal )
        pHLineal->SetBorderPos( pVLineal->GetSizePixel().Width()-1 );

    pVLineal->SetActive(GetFrame() && IsActive());
    const SwViewOption* pOpt = pWrtShell->GetViewOptions();
    pVLineal->SetZoom(Fraction(pOpt->GetZoom(), 100));
    BOOL bWeb = 0 != PTR_CAST(SwWebView, this);
    FieldUnit eMetric = ::GetDfltMetric(bWeb);
    if(eMetric == FUNIT_MM)
        eMetric = FUNIT_CM;
    pVLineal->SetUnit(eMetric);

    InvalidateBorder();
    if ( !bShowAtResize )
        pVLineal->Show();
    return 1;
}

/*************************************************************************
|*
|*  SwView::KillVLineal()
|*
|*  Beschreibung
|*  Ersterstellung  VB 29.05.91
|*  Letzte Aenderung  VB 29.05.91
|*
*************************************************************************/

int SwView::_KillVLineal()
{
    ASSERT( StatVLineal(), "vorher abpruefen!" )

    pVLineal->Hide();

    if ( pHLineal )
        pHLineal->SetBorderPos( 0 );

    DELETEZ(pVLineal);
    InvalidateBorder();
    return 1;
}
/*************************************************************************
|*
|*  SwView::ExecRulerClick()
|*
|*  Beschreibung
|*  Ersterstellung  OS 15.06.95
|*  Letzte Aenderung
|*
*************************************************************************/

IMPL_LINK( SwView, ExecRulerClick, Ruler *, pRuler )
{
    USHORT nDefPage = 0;
    switch( pRuler->GetClickType() )
    {
        case RULER_TYPE_DONTKNOW:
        case RULER_TYPE_OUTSIDE:
        case RULER_TYPE_INDENT:
        case RULER_TYPE_MARGIN1:
        case RULER_TYPE_MARGIN2:
            nDefPage = TP_PARA_STD;
        break;
        default:
            nDefPage = TP_TABULATOR;

    }

    SfxUInt16Item aDefPage(SID_PARA_DLG, nDefPage);
    GetViewFrame()->GetDispatcher()->Execute( SID_PARA_DLG,
                                SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD,
                                &aDefPage, 0L );
    return 0;
}


/*-----------------20.02.97 09:11-------------------

--------------------------------------------------*/

USHORT SwView::GetMoveType()
{
    return nMoveType;
}
/*-----------------20.02.97 09:11-------------------

--------------------------------------------------*/

void SwView::SetMoveType(USHORT nSet)
{
    BOOL bLastPage = nMoveType == NID_PGE;
    nMoveType = nSet;
    BOOL bNewPage = nMoveType == NID_PGE;
    if(bNewPage != bLastPage)
    {
        Color aColor(bNewPage ? COL_BLACK : VIEW_IMAGECOLOR);
        const TypeId aTypeId = TYPE(SwView);
        SwView* pView = (SwView*)SfxViewShell::GetFirst(&aTypeId);
        while( pView )
        {
            pView->SetImageButtonColor(aColor);
            pView = (SwView*)SfxViewShell::GetNext(*pView, &aTypeId);
        }
    }
}

/*-----------------20.06.97 11:18-------------------

--------------------------------------------------*/
void SwView::SetActMark(BYTE nSet)
{
    nActMark = nSet;
}

/*-----------------15.03.97 10:53-------------------

--------------------------------------------------*/

void SwView::SetImageButtonColor(Color& rColor)
{
    if(pPageUpBtn)
    {
        pPageUpBtn->SetControlForeground(rColor);
        pPageDownBtn->SetControlForeground(rColor);
    }
}


/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.126  2000/09/18 16:06:13  willem.vandorp
    OpenOffice header added.

    Revision 1.125  2000/09/07 15:59:33  os
    change: SFX_DISPATCHER/SFX_BINDINGS removed

    Revision 1.124  2000/05/24 13:13:57  hr
    conflict between STLPORT and Workshop header

    Revision 1.123  2000/05/10 11:53:20  os
    Basic API removed

    Revision 1.122  2000/03/03 15:17:04  os
    StarView remainders removed

    Revision 1.121  1999/07/20 07:44:28  OS
    #67585# ZoomValue/ZoomType :SetZoom changed


      Rev 1.120   20 Jul 1999 09:44:28   OS
   #67585# ZoomValue/ZoomType :SetZoom changed

      Rev 1.119   14 Jan 1999 14:19:54   JP
   Bug #60794#: Fehlererkennung beim Tabellenrechnen und anspringen von Formeln

      Rev 1.118   03 Dec 1998 10:26:18   OS
   #59441# Verzeichniseintrag in der Navigation

      Rev 1.117   22 Sep 1998 11:36:28   MA
   #56856# Kontext besser

      Rev 1.116   22 Sep 1998 11:28:16   MA
   #56856# Paint locken fuer Anderung vom Zoom

      Rev 1.115   08 Sep 1998 17:05:12   OS
   #56134# Metric fuer Text und HTML getrennt

      Rev 1.114   14 Jun 1998 16:13:52   MA
   chg: Navi-Tool auch fuer Browse-View

      Rev 1.113   20 Mar 1998 13:23:54   MA
   OleVis2Page durch BrowseMode ersetzt

      Rev 1.112   24 Feb 1998 18:06:38   OS
   includes, SwapPam

      Rev 1.111   24 Feb 1998 12:02:02   OS
   Navigationstool erweitert

      Rev 1.110   07 Feb 1998 10:39:36   OS
   GrabFocus nach Navigations-Move #47160#

      Rev 1.109   21 Nov 1997 15:00:20   MA
   includes

      Rev 1.108   03 Nov 1997 13:58:30   MA
   precomp entfernt

      Rev 1.107   04 Sep 1997 17:14:42   MA
   includes

      Rev 1.106   01 Sep 1997 13:13:50   OS
   DLL-Umstellung

      Rev 1.105   23 Aug 1997 10:12:02   OS
   Zoom ganze Seite: Factor nicht zusaetzlich mit den ViewOpt-Einstellungen multiplizieren #43052#

      Rev 1.104   12 Aug 1997 15:57:26   OS
   frmitems/textitem/paraitem aufgeteilt

      Rev 1.103   08 Aug 1997 17:25:56   OM
   Headerfile-Umstellung

      Rev 1.102   07 Aug 1997 14:58:56   OM
   Headerfile-Umstellung

      Rev 1.101   16 Jul 1997 17:25:54   AMA
   Fix: Rundungsfehler minimieren durch PixelToLogic mit 100%-MapMode

      Rev 1.100   20 Jun 1997 13:55:36   OS
   neu: MoveNavigation

      Rev 1.99   09 Jun 1997 14:28:06   MA
   chg: Browse-Flag nur noch am Doc

      Rev 1.98   08 Apr 1997 10:22:46   MA
   includes

      Rev 1.97   24 Mar 1997 15:57:54   OS
   SetZoomType vor CalcVisArea

      Rev 1.96   15 Mar 1997 11:27:24   OS
   PageUp/Down-Buttons faerben

      Rev 1.95   04 Mar 1997 19:11:42   OS
   Spruenge im Text: zunaechst EnterStdMode

      Rev 1.94   22 Feb 1997 20:15:48   OS
   eigenes Image fuer Button

      Rev 1.93   21 Feb 1997 17:03:00   OS
   ImageButtons veraendert; Handler erweitert

      Rev 1.92   20 Feb 1997 16:46:16   OS
   Navigation funktioniert

      Rev 1.91   19 Feb 1997 16:55:54   OS
   dritter Button/Navigation

      Rev 1.90   05 Feb 1997 08:29:14   MA
   chg: unn?tzes label entfernt

      Rev 1.89   30 Jan 1997 11:03:10   OS
   UsrPrefs verdoppelt

      Rev 1.88   11 Dec 1996 08:42:18   OS
   ClickType am Ruler auswerten -> gfs. Einzuege-TabPage statt Tabulator

      Rev 1.87   10 Dec 1996 19:07:06   MA
   VertScrollbar

      Rev 1.86   11 Nov 1996 11:10:52   MA
   ResMgr

      Rev 1.85   14 Oct 1996 09:02:02   OS
   Readonly an den ViewOptions vor ApplyViewOptions einstellen

      Rev 1.84   07 Oct 1996 09:58:08   OS
   Aktivierung der Lineale im _Create abhaengig von der Aktivierung der View

      Rev 1.83   23 Sep 1996 15:37:34   MA
   fix: Zoomtype und Browser

      Rev 1.82   12 Sep 1996 17:00:32   OS
   GetAnyCurRect() ersetzt GetCur*Rect

      Rev 1.81   28 Aug 1996 14:19:18   JP
   ScrollBarBox: im FrameSet-Doc nicht das Sizeable-Flag setzen

      Rev 1.80   14 Aug 1996 15:15:30   OS
   SetAuto fuer BrowseMode nicht nur fuer HScrollbar

      Rev 1.79   29 Jul 1996 15:47:14   MA
   includes

      Rev 1.78   12 Jul 1996 14:44:22   OS
   vert. Scrollbar geht auch im BrowseMode zum ScrollHdl

      Rev 1.77   05 Jul 1996 14:55:38   OS
   Anzeige der akt. Seitennummer w„hrend des Scrollens in der nicht-Browse-View

      Rev 1.76   26 Jun 1996 15:02:04   OS
   Aufruf von Dispatcher.Execute an 324 angepasst

      Rev 1.75   19 Jun 1996 12:30:04   OM
   Umstellung auf 323

      Rev 1.74   10 Jun 1996 17:55:50   MA
   Breite+HScroll fuer Browser

      Rev 1.73   31 May 1996 07:52:04   OS
   _CreateScrollbar: nach InvalidateBorder Pointer nochmal testen

      Rev 1.72   24 May 1996 09:20:20   OS
   neu: CreatePageButtons

      Rev 1.71   09 May 1996 15:18:58   OS
   HLineal ist immer existent und wird nur noch versteckt

      Rev 1.70   06 May 1996 17:00:36   MA
   chg: Scrollbars fuer browse und ole richtig

      Rev 1.69   24 Apr 1996 15:02:12   OS
   Umstellung UsrPref/ViewOption

------------------------------------------------------------------------*/

