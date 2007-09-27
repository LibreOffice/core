/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: edtwin3.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:40:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#include <hintids.hxx>
#ifndef _SV_SETTINGS_HXX
#include <vcl/settings.hxx>
#endif
#ifndef _SVX_RULER_HXX //autogen
#include <svx/ruler.hxx>
#endif

#ifndef _VIEWOPT_HXX //autogen
#include <viewopt.hxx>
#endif
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
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _SVX_FRMDIRITEM_HXX
#include <svx/frmdiritem.hxx>
#endif


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
// --> OD 2005-02-18 #i42921# - add 3rd parameter <bVerticalMode> in order
// to consider vertical layout
BOOL SwEditWin::RulerMarginDrag( const MouseEvent& rMEvt,
                                 const bool bVerticalMode )
{
    SvxRuler& rRuler = bVerticalMode ?  rView.GetVLineal() : rView.GetHLineal();
    return !rRuler.StartDocDrag( rMEvt, RULER_TYPE_INDENT);
}
// <--

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
    //#99906#   DataChanged() is sometimes called prior to creating
    //          the SwWrtShell
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

