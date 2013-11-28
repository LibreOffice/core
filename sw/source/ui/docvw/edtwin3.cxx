/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#include <hintids.hxx>
#include <vcl/settings.hxx>
#include <svx/ruler.hxx>
#include <viewopt.hxx>
#include "view.hxx"
#include "wrtsh.hxx"
#include "basesh.hxx"
#include "pview.hxx"
#include "mdiexp.hxx"
#include "edtwin.hxx"
#include "swmodule.hxx"
#include "modcfg.hxx"
#include "swtable.hxx"
#include "docsh.hxx"
#include "pagedesc.hxx"     // Aktuelles Seitenformat
#include <frmatr.hxx>
#include <editeng/frmdiritem.hxx>


/*--------------------------------------------------------------------
    Beschreibung:   Core-Notify
 --------------------------------------------------------------------*/



void ScrollMDI( ViewShell* pVwSh, const SwRect &rRect,
                sal_uInt16 nRangeX, sal_uInt16 nRangeY)
{
    SfxViewShell *pSfxVwSh = pVwSh->GetSfxViewShell();
    if (pSfxVwSh && pSfxVwSh->ISA(SwView))
        ((SwView *)pSfxVwSh)->Scroll( rRect.SVRect(), nRangeX, nRangeY );
}

/*--------------------------------------------------------------------
    Beschreibung:   Docmdi - verschiebbar
 --------------------------------------------------------------------*/



sal_Bool IsScrollMDI( ViewShell* pVwSh, const SwRect &rRect )
{
    SfxViewShell *pSfxVwSh = pVwSh->GetSfxViewShell();
    if (pSfxVwSh && pSfxVwSh->ISA(SwView))
        return (((SwView *)pSfxVwSh)->IsScroll(rRect.SVRect()));
    return sal_False;
}

/*--------------------------------------------------------------------
    Beschreibung:   Notify fuer Groessen-Aenderung
 --------------------------------------------------------------------*/



void SizeNotify(ViewShell* pVwSh, const Size &rSize)
{
    SfxViewShell *pSfxVwSh = pVwSh->GetSfxViewShell();
    if (pSfxVwSh)
    {
        if (pSfxVwSh->ISA(SwView))
            ((SwView *)pSfxVwSh)->DocSzChgd(rSize);
        else if (pSfxVwSh->ISA(SwPagePreView))
            ((SwPagePreView *)pSfxVwSh)->DocSzChgd( rSize );
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   Notify fuer Seitenzahl-Update
 --------------------------------------------------------------------*/



void PageNumNotify( ViewShell* pVwSh, sal_uInt16 nPhyNum, sal_uInt16 nVirtNum,
                                                    const String& rPgStr)
{
    SfxViewShell *pSfxVwSh = pVwSh->GetSfxViewShell();
    if ( pSfxVwSh && pSfxVwSh->ISA(SwView) &&
         ((SwView*)pSfxVwSh)->GetCurShell() )
            ((SwView *)pSfxVwSh)->UpdatePageNums(nPhyNum, nVirtNum, rPgStr);
}

/******************************************************************************
 *  Methode     :   void FrameNotify( DocMDIBase *pWin, FlyMode eMode )
 *  Beschreibung:
 *  Erstellt    :   OK 08.02.94 13:49
 *  Aenderung   :
 ******************************************************************************/



void FrameNotify( ViewShell* pVwSh, FlyMode eMode )
{
    if ( pVwSh->ISA(SwCrsrShell) )
        SwBaseShell::SetFrmMode( eMode, (SwWrtShell*)pVwSh );
}

/*--------------------------------------------------------------------
    Beschreibung:   Notify fuer Seitenzahl-Update
 --------------------------------------------------------------------*/
sal_Bool SwEditWin::RulerColumnDrag( const MouseEvent& rMEvt, sal_Bool bVerticalMode)
{
    SvxRuler& rRuler = bVerticalMode ?  rView.GetVLineal() : rView.GetHLineal();
    return (!rRuler.StartDocDrag( rMEvt, RULER_TYPE_BORDER ) &&
            !rRuler.StartDocDrag( rMEvt, RULER_TYPE_MARGIN1) &&
            !rRuler.StartDocDrag( rMEvt, RULER_TYPE_MARGIN2));
}

// #i23726#
// --> OD 2005-02-18 #i42921# - add 3rd parameter <bVerticalMode> in order
// to consider vertical layout
sal_Bool SwEditWin::RulerMarginDrag( const MouseEvent& rMEvt,
                                 const bool bVerticalMode )
{
    SvxRuler& rRuler = bVerticalMode ?  rView.GetVLineal() : rView.GetHLineal();
    return !rRuler.StartDocDrag( rMEvt, RULER_TYPE_INDENT);
}
// <--
void AccessibilityScrollMDI(ViewShell* pVwSh, const SwRect& rRect , sal_uInt16 nRangeX, sal_uInt16 nRangeY, sal_Bool isLeftTop)
{
    SfxViewShell *pSfxVwSh = pVwSh->GetSfxViewShell();
    if (pSfxVwSh && pSfxVwSh->ISA(SwView))
    {
        SwView* pView = (SwView *)pSfxVwSh;

        Size aSz( rRect.Left(), rRect.Top());
        aSz = pView->GetEditWin().PixelToLogic( aSz );

        Point aTopLeft( aSz.Width(), aSz.Height() );
        Point aPoint;
        Rectangle aRect(aTopLeft,aPoint);

        sal_Bool bIsCrsrAtTop = pView->IsCrsrAtTop();
        sal_Bool bIsCrsrAtCenter = pView->IsCrsrAtCenter();
        if(isLeftTop)
            pView->SetCrsrAtTop(sal_True);

        pView->Scroll( aRect, nRangeX, nRangeY );

        if(isLeftTop)
            pView->SetCrsrAtTop(bIsCrsrAtTop, bIsCrsrAtCenter);
    }

}

LAYOUT_NS Dialog* GetSearchDialog()
{
    return SwView::GetSearchDialog();
}

TblChgMode GetTblChgDefaultMode()
{
    SwModuleOptions* pOpt = SW_MOD()->GetModuleConfig();
    return pOpt ? pOpt->GetTblMode() : TBLVAR_CHGABS;
}

void RepaintPagePreview( ViewShell* pVwSh, const SwRect& rRect )
{
    SfxViewShell *pSfxVwSh = pVwSh->GetSfxViewShell();
    if (pSfxVwSh && pSfxVwSh->ISA( SwPagePreView ))
        ((SwPagePreView *)pSfxVwSh)->RepaintCoreRect( rRect );
}

sal_Bool JumpToSwMark( ViewShell* pVwSh, const String& rMark )
{
    SfxViewShell *pSfxVwSh = pVwSh->GetSfxViewShell();
    if( pSfxVwSh && pSfxVwSh->ISA( SwView ) )
        return ((SwView *)pSfxVwSh)->JumpToSwMark( rMark );
    return sal_False;
}

void SwEditWin::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    SwWrtShell* pSh = GetView().GetWrtShellPtr();
    //#99906#   DataChanged() is sometimes called prior to creating
    //          the SwWrtShell
    if(!pSh)
        return;
    sal_Bool bViewWasLocked = pSh->IsViewLocked(), bUnlockPaint = sal_False;
    pSh->LockView( sal_True );
    switch( rDCEvt.GetType() )
    {
    case DATACHANGED_SETTINGS:
        // ScrollBars neu anordnen bzw. Resize ausloesen, da sich
        // ScrollBar-Groesse geaendert haben kann. Dazu muss dann im
        // Resize-Handler aber auch die Groesse der ScrollBars aus
        // den Settings abgefragt werden.
        if( rDCEvt.GetFlags() & SETTINGS_STYLE )
        {
            pSh->LockPaint();
            bUnlockPaint = sal_True;
            ViewShell::DeleteReplacementBitmaps();
            GetView().InvalidateBorder();               //Scrollbarbreiten
        }
        break;

    case DATACHANGED_PRINTER:
    case DATACHANGED_DISPLAY:
    case DATACHANGED_FONTS:
    case DATACHANGED_FONTSUBSTITUTION:
        pSh->LockPaint();
        bUnlockPaint = sal_True;
        GetView().GetDocShell()->UpdateFontList();  //z.B. Druckerwechsel
        break;
    }
    pSh->LockView( bViewWasLocked );
    if( bUnlockPaint )
        pSh->UnlockPaint();
}

