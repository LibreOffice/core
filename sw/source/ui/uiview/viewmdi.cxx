/*************************************************************************
 *
 *  $RCSfile: viewmdi.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 15:51:19 $
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

#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _WDOCSH_HXX
#include <wdocsh.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _WORKCTRL_HXX
#include <workctrl.hxx>
#endif
#ifndef _USRPREF_HXX
#include <usrpref.hxx>
#endif
#ifndef _SCROLL_HXX
#include <scroll.hxx>
#endif
#ifndef _WVIEW_HXX
#include <wview.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _VIEW_HRC
#include <view.hrc>
#endif
#ifndef _RIBBAR_HRC
#include <ribbar.hrc>
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif

USHORT  SwView::nMoveType = NID_PGE;
BYTE    SwView::nActMark = 0;

#define VIEW_IMAGECOLOR COL_LIGHTBLUE

void SwView::SetZoom( SvxZoomType eZoomType, short nFactor, BOOL bViewOnly )
{
    _SetZoom( GetEditWin().GetOutputSizePixel(), eZoomType, nFactor, bViewOnly );
}

void SwView::_SetZoom( const Size &rEditSize, SvxZoomType eZoomType,
                        short nFactor, BOOL bViewOnly )
{
    BOOL bUnLockView = !pWrtShell->IsViewLocked();
    pWrtShell->LockView( TRUE );
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
        else if(SVX_ZOOM_PAGEWIDTH_NOBORDER != eZoomType)
        {
            aPageSize.Width() += nOf;
            aPageSize.Height() += nOf;
        }
        lLeftMargin = SVX_ZOOM_PAGEWIDTH != eZoomType && SVX_ZOOM_PAGEWIDTH_NOBORDER != eZoomType ?
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
            SW_MOD()->ApplyUsrPref( *pUsrPref,
                    bViewOnly ? this: 0,
                    bViewOnly ? VIEWOPT_DEST_VIEW_ONLY : 0 );
            pUsrPref->SetModified();
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
    pVRuler->SetZoom( aFrac );
    pVRuler->ForceUpdate();
    pHRuler->SetZoom( aFrac );
    pHRuler->ForceUpdate();
    ((SwViewOption*)pWrtShell->GetViewOptions())->SetZoomType((BYTE)eZoomType);
    }
    pWrtShell->UnlockPaint();
    if( bUnLockView )
        pWrtShell->LockView( FALSE );
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

    *ppScrollbar = new SwScrollbar( pMDI, bHori );
    UpdateScrollbars();
    if(bHori)
        (*ppScrollbar)->SetScrollHdl( LINK( this, SwView, EndScrollHdl ));
    else
        (*ppScrollbar)->SetScrollHdl( LINK( this, SwView, ScrollHdl ));
    (*ppScrollbar)->SetEndScrollHdl( LINK( this, SwView, EndScrollHdl ));

    (*ppScrollbar)->EnableDrag( TRUE );

    if(GetWindow())
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
            GotoObjType eType = FLY_FRM;
            if(nMoveType == NID_GRF)
                eType = FLY_GRF;
            else if(nMoveType == NID_OLE)
                eType = FLY_OLE;
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

int SwView::CreateTab()
{
    pHRuler->SetActive(GetFrame() && IsActive());

    pHRuler->Show();
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

int SwView::KillTab()
{
    pHRuler->Hide();
    InvalidateBorder();
    return 1;
}

void SwView::ChangeTabMetric( FieldUnit eUnit )
{
    if(pHRuler->GetUnit() != eUnit )
    {
        pHRuler->SetUnit( eUnit );
        pHRuler->Invalidate();
    }
}

void SwView::ChangeVLinealMetric( FieldUnit eUnit )
{
    if(pVRuler->GetUnit() != eUnit)
    {
        pVRuler->SetUnit( eUnit );
        pVRuler->Invalidate();
    }
}
/* -----------------------------07.04.01 17:09--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwView::GetVLinealMetric(FieldUnit& eToFill) const
{
    eToFill = pVRuler->GetUnit();
    return pVRuler != 0;
}
/* -----------------------------07.04.01 17:09--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwView::GetHLinealMetric(FieldUnit& eToFill) const
{
    eToFill = pHRuler->GetUnit();
    return pHRuler != 0;
}
/*************************************************************************
|*
|*  SwView::CreateVLineal()
|*
|*  Beschreibung
|*  Ersterstellung  VB 29.05.91
|*
*************************************************************************/

int SwView::CreateVLineal()
{
    pHRuler->SetBorderPos( pVRuler->GetSizePixel().Width()-1 );

    pVRuler->SetActive(GetFrame() && IsActive());
    const SwViewOption* pOpt = pWrtShell->GetViewOptions();
    pVRuler->Show();
    InvalidateBorder();
    return 1;
}

/*************************************************************************
|*
|*  SwView::KillVLineal()
|*
|*  Beschreibung
|*  Ersterstellung  VB 29.05.91
|*
*************************************************************************/

int SwView::KillVLineal()
{
    pVRuler->Hide();
    pHRuler->SetBorderPos( 0 );
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
/* -----------------------------2002/06/26 13:57------------------------------

 ---------------------------------------------------------------------------*/
void SwView::ShowHScrollbar(sal_Bool bShow)
{
    DBG_ASSERT(pHScrollbar, "Scrollbar invalid")
    pHScrollbar->Show(bShow);
}
/* -----------------------------2002/06/26 13:57------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SwView::IsHScrollbarVisible()const
{
    DBG_ASSERT(pHScrollbar, "Scrollbar invalid")
    return pHScrollbar->IsVisible() || pHScrollbar->IsAuto();
}
/* -----------------------------2002/06/26 13:57------------------------------

 ---------------------------------------------------------------------------*/
void SwView::ShowVScrollbar(sal_Bool bShow)
{
    DBG_ASSERT(pVScrollbar, "Scrollbar invalid")
    pVScrollbar->Show(bShow);
    pPageUpBtn->Show(bShow);
    pPageDownBtn->Show(bShow);
    pNaviBtn->Show(bShow);
}
/* -----------------------------2002/06/26 13:57------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SwView::IsVScrollbarVisible()const
{
    DBG_ASSERT(pVScrollbar, "Scrollbar invalid")
    return pVScrollbar->IsVisible();
}


