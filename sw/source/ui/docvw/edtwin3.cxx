/*************************************************************************
 *
 *  $RCSfile: edtwin3.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 15:23:31 $
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

#include <hintids.hxx>
#include "uiparam.hxx"
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
BOOL SwEditWin::RulerColumnDrag( SwView& rView , const MouseEvent& rMEvt, BOOL bVerticalMode)
{
    SvxRuler& rRuler = bVerticalMode ?  rView.GetVLineal() : rView.GetHLineal();
    return (!rRuler.StartDocDrag( rMEvt, RULER_TYPE_BORDER ) &&
            !rRuler.StartDocDrag( rMEvt, RULER_TYPE_MARGIN1) &&
            !rRuler.StartDocDrag( rMEvt, RULER_TYPE_MARGIN2));
}

Dialog* GetSearchDialog()
{
    return SwView::GetSearchDialog();
}



void JavaScriptScrollMDI( SfxFrame* pFrame, INT32 nX, INT32 nY )
{
    SfxViewShell *pSfxVwSh = pFrame->GetCurrentViewFrame()->GetViewShell();
    if( pSfxVwSh && pSfxVwSh->ISA( SwView ))
    {
        SwView* pView = (SwView *)pSfxVwSh;

        Size aSz( nX, nY );
        aSz = pView->GetEditWin().PixelToLogic( aSz );

        Point aTopLeft( aSz.Width(), aSz.Height() );
        if( aTopLeft.X() < DOCUMENTBORDER ) aTopLeft.X() = DOCUMENTBORDER;
        if( aTopLeft.Y() < DOCUMENTBORDER ) aTopLeft.Y() = DOCUMENTBORDER;

        const Size& rVisSize = pView->GetVisArea().GetSize();
        Size aDocSize( pView->GetDocSz() );
        aDocSize.Width() += DOCUMENTBORDER;
        aDocSize.Height() += DOCUMENTBORDER;

        if( aTopLeft.X() + rVisSize.Width() > aDocSize.Width() )
            aTopLeft.X() = rVisSize.Width() > aDocSize.Width()
                                ? DOCUMENTBORDER
                                : aDocSize.Width() - rVisSize.Width();

        if( aTopLeft.Y() + rVisSize.Height() > aDocSize.Height() )
            aTopLeft.Y() = rVisSize.Height() > aDocSize.Height()
                                ? DOCUMENTBORDER
                                : aDocSize.Height() - rVisSize.Height();

        pView->SetVisArea( aTopLeft );
    }
}



USHORT GetTblChgDefaultMode()
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


