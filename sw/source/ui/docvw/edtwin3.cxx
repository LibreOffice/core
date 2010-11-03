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
                USHORT nRangeX, USHORT nRangeY)
{
    SfxViewShell *pSfxVwSh = pVwSh->GetSfxViewShell();
    if (pSfxVwSh && pSfxVwSh->ISA(SwView))
        ((SwView *)pSfxVwSh)->Scroll( rRect.SVRect(), nRangeX, nRangeY );
}

/*--------------------------------------------------------------------
    Beschreibung:   Docmdi - verschiebbar
 --------------------------------------------------------------------*/



BOOL IsScrollMDI( ViewShell* pVwSh, const SwRect &rRect )
{
    SfxViewShell *pSfxVwSh = pVwSh->GetSfxViewShell();
    if (pSfxVwSh && pSfxVwSh->ISA(SwView))
        return (((SwView *)pSfxVwSh)->IsScroll(rRect.SVRect()));
    return FALSE;
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



void PageNumNotify( ViewShell* pVwSh, USHORT nPhyNum, USHORT nVirtNum,
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
BOOL SwEditWin::RulerColumnDrag( const MouseEvent& rMEvt, BOOL bVerticalMode)
{
    SvxRuler& rRuler = bVerticalMode ?  rView.GetVLineal() : rView.GetHLineal();
    return (!rRuler.StartDocDrag( rMEvt, RULER_TYPE_BORDER ) &&
            !rRuler.StartDocDrag( rMEvt, RULER_TYPE_MARGIN1) &&
            !rRuler.StartDocDrag( rMEvt, RULER_TYPE_MARGIN2));
}

// #i23726#
// #i42921# - add 3rd parameter <bVerticalMode> in order
// to consider vertical layout
BOOL SwEditWin::RulerMarginDrag( const MouseEvent& rMEvt,
                                 const bool bVerticalMode )
{
    SvxRuler& rRuler = bVerticalMode ?  rView.GetVLineal() : rView.GetHLineal();
    return !rRuler.StartDocDrag( rMEvt, RULER_TYPE_INDENT);
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

BOOL JumpToSwMark( ViewShell* pVwSh, const String& rMark )
{
    SfxViewShell *pSfxVwSh = pVwSh->GetSfxViewShell();
    if( pSfxVwSh && pSfxVwSh->ISA( SwView ) )
        return ((SwView *)pSfxVwSh)->JumpToSwMark( rMark );
    return FALSE;
}

void SwEditWin::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    SwWrtShell* pSh = GetView().GetWrtShellPtr();
    // DataChanged() is sometimes called prior to creating
    // the SwWrtShell
    if(!pSh)
        return;
    BOOL bViewWasLocked = pSh->IsViewLocked(), bUnlockPaint = FALSE;
    pSh->LockView( TRUE );
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
            bUnlockPaint = TRUE;
            ViewShell::DeleteReplacementBitmaps();
            GetView().InvalidateBorder();               //Scrollbarbreiten
        }
        break;

    case DATACHANGED_PRINTER:
    case DATACHANGED_DISPLAY:
    case DATACHANGED_FONTS:
    case DATACHANGED_FONTSUBSTITUTION:
        pSh->LockPaint();
        bUnlockPaint = TRUE;
        GetView().GetDocShell()->UpdateFontList();  //z.B. Druckerwechsel
        break;
    }
    pSh->LockView( bViewWasLocked );
    if( bUnlockPaint )
        pSh->UnlockPaint();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
