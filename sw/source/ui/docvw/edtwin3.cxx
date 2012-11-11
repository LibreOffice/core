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
    Description:    Core-Notify
 --------------------------------------------------------------------*/



void ScrollMDI( ViewShell* pVwSh, const SwRect &rRect,
                sal_uInt16 nRangeX, sal_uInt16 nRangeY)
{
    SfxViewShell *pSfxVwSh = pVwSh->GetSfxViewShell();
    if (pSfxVwSh && pSfxVwSh->ISA(SwView))
        ((SwView *)pSfxVwSh)->Scroll( rRect.SVRect(), nRangeX, nRangeY );
}

/*--------------------------------------------------------------------
    Description:    Docmdi - movable
 --------------------------------------------------------------------*/



sal_Bool IsScrollMDI( ViewShell* pVwSh, const SwRect &rRect )
{
    SfxViewShell *pSfxVwSh = pVwSh->GetSfxViewShell();
    if (pSfxVwSh && pSfxVwSh->ISA(SwView))
        return (((SwView *)pSfxVwSh)->IsScroll(rRect.SVRect()));
    return sal_False;
}

/*--------------------------------------------------------------------
    Description:    Notify for size change
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
    Description:    Notify for page number update
 --------------------------------------------------------------------*/

void PageNumNotify( ViewShell* pVwSh, sal_uInt16 nPhyNum, sal_uInt16 nVirtNum,
                                                    const rtl::OUString& rPgStr)
{
    SfxViewShell *pSfxVwSh = pVwSh->GetSfxViewShell();
    if ( pSfxVwSh && pSfxVwSh->ISA(SwView) &&
         ((SwView*)pSfxVwSh)->GetCurShell() )
            ((SwView *)pSfxVwSh)->UpdatePageNums(nPhyNum, nVirtNum, rPgStr);
}

/******************************************************************************
 *  Method      :   void FrameNotify( DocMDIBase *pWin, FlyMode eMode )
 *  Description:
 ******************************************************************************/



void FrameNotify( ViewShell* pVwSh, FlyMode eMode )
{
    if ( pVwSh->ISA(SwCrsrShell) )
        SwBaseShell::SetFrmMode( eMode, (SwWrtShell*)pVwSh );
}

/*--------------------------------------------------------------------
    Description:    Notify for page number update
 --------------------------------------------------------------------*/
sal_Bool SwEditWin::RulerColumnDrag( const MouseEvent& rMEvt, sal_Bool bVerticalMode)
{
    SvxRuler& rRuler = bVerticalMode ?  rView.GetVLineal() : rView.GetHLineal();
    return (!rRuler.StartDocDrag( rMEvt, RULER_TYPE_BORDER ) &&
            !rRuler.StartDocDrag( rMEvt, RULER_TYPE_MARGIN1) &&
            !rRuler.StartDocDrag( rMEvt, RULER_TYPE_MARGIN2));
}

// #i23726#
// #i42921# - add 3rd parameter <bVerticalMode> in order
// to consider vertical layout
sal_Bool SwEditWin::RulerMarginDrag( const MouseEvent& rMEvt,
                                 const bool bVerticalMode )
{
    SvxRuler& rRuler = bVerticalMode ?  rView.GetVLineal() : rView.GetHLineal();
    return !rRuler.StartDocDrag( rMEvt, RULER_TYPE_INDENT);
}

Dialog* GetSearchDialog()
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

bool JumpToSwMark( ViewShell* pVwSh, const rtl::OUString& rMark )
{
    SfxViewShell *pSfxVwSh = pVwSh->GetSfxViewShell();
    if( pSfxVwSh && pSfxVwSh->ISA( SwView ) )
        return ((SwView *)pSfxVwSh)->JumpToSwMark( rMark );
    return false;
}

void SwEditWin::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    SwWrtShell* pSh = GetView().GetWrtShellPtr();
    // DataChanged() is sometimes called prior to creating
    // the SwWrtShell
    if(!pSh)
        return;
    sal_Bool bViewWasLocked = pSh->IsViewLocked(), bUnlockPaint = sal_False;
    pSh->LockView( sal_True );
    switch( rDCEvt.GetType() )
    {
    case DATACHANGED_SETTINGS:
        // rearrange ScrollBars, respectively trigger resize, because
        // the ScrollBar size can have change. For that, in the reset
        // handler, the size of the ScrollBars also has to be queried
        // from the settings.
        if( rDCEvt.GetFlags() & SETTINGS_STYLE )
        {
            pSh->LockPaint();
            bUnlockPaint = sal_True;
            ViewShell::DeleteReplacementBitmaps();
            GetView().InvalidateBorder();               //Scrollbar work
        }
        break;

    case DATACHANGED_PRINTER:
    case DATACHANGED_DISPLAY:
    case DATACHANGED_FONTS:
    case DATACHANGED_FONTSUBSTITUTION:
        pSh->LockPaint();
        bUnlockPaint = sal_True;
        GetView().GetDocShell()->UpdateFontList();  //e.g. printer change
        break;
    }
    pSh->LockView( bViewWasLocked );
    if( bUnlockPaint )
        pSh->UnlockPaint();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
