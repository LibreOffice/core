/*************************************************************************
 *
 *  $RCSfile: viewshe2.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2000-09-21 16:12:19 $
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

#ifndef _SVXIDS_HRC

#include <svx/svxids.hrc>
#endif
#ifndef _SCRBAR_HXX //autogen
#include <vcl/scrbar.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SCH_DLL_HXX //autogen
#include <sch/schdll.hxx>
#endif
#ifndef _SCHDLL0_HXX
#include <sch/schdll0.hxx>
#endif
#ifndef _SMDLL_HXX //autogen
#include <starmath/smdll.hxx>
#endif
#ifndef SC_SCDLL_HXX //autogen
#include <sc/scdll.hxx>
#endif
#ifndef _SVX_RULER_HXX //autogen
#include <svx/ruler.hxx>
#endif
#ifndef _OUTLINER_HXX //autogen
#include <svx/outliner.hxx>
#endif
#ifndef _EHDL_HXX //autogen
#include <svtools/ehdl.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _SVDOOLE2_HXX //autogen
#include <svx/svdoole2.hxx>
#endif
#ifndef _SOERR_HXX //autogen
#include <so3/soerr.hxx>
#endif
#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif
#ifndef _IPENV_HXX //autogen
#include <so3/ipenv.hxx>
#endif
#ifndef _SVX_FMSHELL_HXX            // XXX nur temp (dg)
#include <svx/fmshell.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif

#include "misc.hxx"

#ifdef STARIMAGE_AVAILABLE
#ifndef _SIMDLL_HXX
#include <sim2/simdll.hxx>
#endif
#endif

#include "strings.hrc"
#include "app.hrc"

#include "sdundogr.hxx"
#include "frmview.hxx"
#include "undopage.hxx"
#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "viewshel.hxx"
#include "sdview.hxx"
#include "fupoor.hxx"
#include "sdclient.hxx"
#include "docshell.hxx"
#include "fusearch.hxx"
#include "fuslshow.hxx"
#include "sdpage.hxx"
#include "drviewsh.hxx"

#ifndef SO2_DECL_SVINPLACEOBJECT_DEFINED
#define SO2_DECL_SVINPLACEOBJECT_DEFINED
SO2_DECL_REF(SvInPlaceObject)
#endif
#ifndef SO2_DECL_SVSTORAGE_DEFINED
#define SO2_DECL_SVSTORAGE_DEFINED
SO2_DECL_REF(SvStorage)
#endif

#ifdef WNT
#pragma optimize ( "", off )
#endif



/*************************************************************************
|*
|* Scrollbar-Update: Thumbpos und VisibleSize anpassen
|*
\************************************************************************/

void SdViewShell::UpdateScrollBars()
{
    short i;

    for (i = 0; i < MAX_HSPLIT_CNT && pHScrlArray[i]; i++)
    {
        long nW = (long)(pWinArray[i][0]->GetVisibleWidth() * 32000);
        long nX = (long)(pWinArray[i][0]->GetVisibleX() * 32000);
        pHScrlArray[i]->SetVisibleSize(nW);
        pHScrlArray[i]->SetThumbPos(nX);
        nW = 32000 - nW;
        long nLine = (long) (pWinArray[i][0]->GetScrlLineWidth() * nW);
        long nPage = (long) (pWinArray[i][0]->GetScrlPageWidth() * nW);
        pHScrlArray[i]->SetLineSize(nLine);
        pHScrlArray[i]->SetPageSize(nPage);
    }

    for (i = 0; i < MAX_VSPLIT_CNT && pVScrlArray[i]; i++)
    {
        long nH = (long)(pWinArray[0][i]->GetVisibleHeight() * 32000);
        long nY = (long)(pWinArray[0][i]->GetVisibleY() * 32000);
        pVScrlArray[i]->SetVisibleSize(nH);
        pVScrlArray[i]->SetThumbPos(nY);
        nH = 32000 - nH;
        long nLine = (long) (pWinArray[0][i]->GetScrlLineHeight() * nH);
        long nPage = (long) (pWinArray[0][i]->GetScrlPageHeight() * nH);
        pVScrlArray[i]->SetLineSize(nLine);
        pVScrlArray[i]->SetPageSize(nPage);
    }
    if ( bHasRuler )
    {
        UpdateHRuler();
        UpdateVRuler();
    }
}
/*************************************************************************
|*
|* Handling fuer horizontale Scrollbars
|*
\************************************************************************/

IMPL_LINK_INLINE_START( SdViewShell, HScrollHdl, ScrollBar *, pHScroll )
{
    return VirtHScrollHdl(pHScroll);
}
IMPL_LINK_INLINE_END( SdViewShell, HScrollHdl, ScrollBar *, pHScroll )

/*************************************************************************
|*
|* virtueller Scroll-Handler fuer horizontale Scrollbars
|*
\************************************************************************/

long SdViewShell::VirtHScrollHdl(ScrollBar* pHScroll)
{
    long nDelta = pHScroll->GetDelta();

    if (nDelta != 0)
    {
        short nX = 0, nY = 0;

        // Spalte der zu scrollenden Fenster bestimmen
        while ( nX < MAX_HSPLIT_CNT &&  pHScroll != pHScrlArray[nX] )
            ++nX;

        double fX = (double) pHScroll->GetThumbPos() / pHScroll->GetRange().Len();

        // alle Fenster der Spalte scrollen
        while ( nY < MAX_VSPLIT_CNT && pWinArray[nX][nY] )
        {
            SdView* pView = GetView();
            OutlinerView* pOLV = NULL;

            if (pView)
                pOLV = pView->GetTextEditOutlinerView();

            if (pOLV)
                pOLV->HideCursor();

            pWinArray[nX][nY++]->SetVisibleXY(fX, -1);

            Rectangle aVisArea = pDocSh->GetVisArea(ASPECT_CONTENT);
            Point aVisAreaPos = pWindow->PixelToLogic( Point(0,0) );
            aVisArea.SetPos(aVisAreaPos);
            pDocSh->SetVisArea(aVisArea);

            Size aVisSizePixel = pWindow->GetOutputSizePixel();
            Rectangle aVisAreaWin = pWindow->PixelToLogic( Rectangle( Point(0,0), aVisSizePixel) );
            VisAreaChanged(aVisAreaWin);

            if (pView)
            {
                pView->VisAreaChanged(pWindow);
            }

            if (pOLV)
                pOLV->ShowCursor();
        }

        if ( bHasRuler )
            UpdateHRuler();

    }

    return 0;
}

/*************************************************************************
|*
|* Handling fuer vertikale Scrollbars
|*
\************************************************************************/

IMPL_LINK_INLINE_START( SdViewShell, VScrollHdl, ScrollBar *, pVScroll )
{
    return VirtVScrollHdl(pVScroll);
}
IMPL_LINK_INLINE_END( SdViewShell, VScrollHdl, ScrollBar *, pVScroll )

/*************************************************************************
|*
|* Handling fuer vertikale Scrollbars
|*
\************************************************************************/

long SdViewShell::VirtVScrollHdl(ScrollBar* pVScroll)
{
    long nDelta = pVScroll->GetDelta();

    if (nDelta != 0)
    {
        short nX = 0, nY = 0;

        // Zeile der zu scrollenden Fenster bestimmen
        while ( nY < MAX_VSPLIT_CNT &&  pVScroll != pVScrlArray[nY] )
            ++nY;

        double fY = (double) pVScroll->GetThumbPos() / pVScroll->GetRange().Len();

        // alle Fenster der Zeile scrollen
        while ( nX < MAX_HSPLIT_CNT && pWinArray[nX][nY] )
        {
            SdView* pView = GetView();
            OutlinerView* pOLV = NULL;

            if (pView)
                pOLV = pView->GetTextEditOutlinerView();

            if (pOLV)
                pOLV->HideCursor();

            pWinArray[nX++][nY]->SetVisibleXY(-1, fY);

            Rectangle aVisArea = pDocSh->GetVisArea(ASPECT_CONTENT);
            Point aVisAreaPos = pWindow->PixelToLogic( Point(0,0) );
            aVisArea.SetPos(aVisAreaPos);
            pDocSh->SetVisArea(aVisArea);

            Size aVisSizePixel = pWindow->GetOutputSizePixel();
            Rectangle aVisAreaWin = pWindow->PixelToLogic( Rectangle( Point(0,0), aVisSizePixel) );
            VisAreaChanged(aVisAreaWin);

            if (pView)
            {
                pView->VisAreaChanged(pWindow);
            }

            if (pOLV)
                pOLV->ShowCursor();
        }

        if ( bHasRuler )
            UpdateVRuler();

    }

    return 0;
}

/*************************************************************************
|*
|* Eine bestimmte Anzahl von Zeilen scrollen (wird beim automatischen
|* Scrollen (Zeichen/Draggen) verwendet)
|*
\************************************************************************/

void SdViewShell::ScrollLines(long nLinesX, long nLinesY)
{
    short nX, nY;
    for (nX = 0; nX < MAX_HSPLIT_CNT; nX++)
    {
        nY = 0;
        while ( nY < MAX_VSPLIT_CNT && pWindow != pWinArray[nX][nY] )
            nY++;
        if ( nY < MAX_VSPLIT_CNT )
            break;
    }
    if ( nLinesX )
    {
        nLinesX *= pHScrlArray[nX]->GetLineSize();
    }
    if ( nLinesY )
    {
        nLinesY *= pVScrlArray[nY]->GetLineSize();
    }

    Scroll(nLinesX, nLinesY);
}

/*************************************************************************
|*
|* Window um nScrollX, nScrollY scrollen
|*
\************************************************************************/

void SdViewShell::Scroll(long nScrollX, long nScrollY)
{
    short nX, nY;
    for (nX = 0; nX < MAX_HSPLIT_CNT; nX++)
    {
        nY = 0;
        while ( nY < MAX_VSPLIT_CNT && pWindow != pWinArray[nX][nY] )
            nY++;
        if ( nY < MAX_VSPLIT_CNT )
            break;
    }

    if (nScrollX)
    {
        long nLineSize = pHScrlArray[nX]->GetLineSize();

        long nNewThumb = pHScrlArray[nX]->GetThumbPos() + nScrollX;
        pHScrlArray[nX]->SetThumbPos(nNewThumb);
    }
    if (nScrollY)
    {
        long nNewThumb = pVScrlArray[nY]->GetThumbPos() + nScrollY;
        pVScrlArray[nY]->SetThumbPos(nNewThumb);
    }
    double  fX = (double) pHScrlArray[nX]->GetThumbPos() /
                            pHScrlArray[nX]->GetRange().Len();
    double  fY = (double) pVScrlArray[nY]->GetThumbPos() /
                            pVScrlArray[nY]->GetRange().Len();

    pWindow->SetVisibleXY(fX, fY);

    if ( bIsHSplit || bIsVSplit )   // alle Fenster der gleichen Spalte
    {                               // sowie Zeile scrollen
        short i;

        for (i = 0; i < MAX_HSPLIT_CNT; i++)
            if ( i != nX && pWinArray[i][nY] )
                pWinArray[i][nY]->SetVisibleXY(-1, fY);

        for (i = 0; i < MAX_VSPLIT_CNT; i++)
            if ( i != nY && pWinArray[nX][i] )
            pWinArray[nX][i]->SetVisibleXY(fX, -1);
    }

    Rectangle aVisArea = pDocSh->GetVisArea(ASPECT_CONTENT);
    Point aVisAreaPos = pWindow->PixelToLogic( Point(0,0) );
    aVisArea.SetPos(aVisAreaPos);
    pDocSh->SetVisArea(aVisArea);

    Size aVisSizePixel = pWindow->GetOutputSizePixel();
    Rectangle aVisAreaWin = pWindow->PixelToLogic( Rectangle( Point(0,0), aVisSizePixel) );
    VisAreaChanged(aVisAreaWin);

    SdView* pView = GetView();
    if (pView)
    {
        pView->VisAreaChanged(pWindow);
    }

    if ( bHasRuler )
    {
        UpdateHRuler();
        UpdateVRuler();
    }
}

/*************************************************************************
|*
|* Den Zoomfaktor fuer alle Split-Windows setzen
|*
\************************************************************************/

void SdViewShell::SetZoom(long nZoom)
{
    Fraction aUIScale(nZoom, 100);
    aUIScale *= pDoc->GetUIScale();

    for (short nX = 0; nX < MAX_HSPLIT_CNT; nX++)
    {
        if ( pHRulerArray[nX] )
            pHRulerArray[nX]->SetZoom(aUIScale);

        for (short nY = 0; nY < MAX_VSPLIT_CNT; nY++)
        {
            if ( nX == 0 && pVRulerArray[nY] )
                pVRulerArray[nY]->SetZoom(aUIScale);

            if ( pWinArray[nX][nY] )
            {
                pWinArray[nX][nY]->SetZoom(nZoom);
                pWinArray[nX][nY]->Invalidate();
            }
        }
    }

    Size aVisSizePixel = pWindow->GetOutputSizePixel();
    Rectangle aVisAreaWin = pWindow->PixelToLogic( Rectangle( Point(0,0), aVisSizePixel) );
    VisAreaChanged(aVisAreaWin);

    SdView* pView = GetView();
    if (pView)
    {
        pView->VisAreaChanged(pWindow);
    }

    UpdateScrollBars();
}

/*************************************************************************
|*
|* Zoomrechteck fuer aktives Fenster einstellen und alle Split-Windows
|* auf den gleichen Zoomfaktor setzen
|*
\************************************************************************/

void SdViewShell::SetZoomRect(const Rectangle& rZoomRect)
{
    short nX, nY, nCol, nRow;
    long nZoom = pWindow->SetZoomRect(rZoomRect);
    Fraction aUIScale(nZoom, 100);
    aUIScale *= pDoc->GetUIScale();

    for (nX = 0; nX < MAX_HSPLIT_CNT; nX++)
        for (nY = 0; nY < MAX_VSPLIT_CNT; nY++)
            if ( pWinArray[nX][nY] == pWindow )
            {   nCol = nX; nRow = nY; }

    Point aPos = pWindow->GetWinViewPos();

    for (nX = 0; nX < MAX_HSPLIT_CNT; nX++)
    {
        if ( pHRulerArray[nX] )
            pHRulerArray[nX]->SetZoom(aUIScale);

        for (nY = 0; nY < MAX_VSPLIT_CNT; nY++)
        {
            if ( nX == 0 && pVRulerArray[nY] )
                pVRulerArray[nY]->SetZoom(aUIScale);

            if ( pWinArray[nX][nY] )
            {
                Point aNewPos = pWinArray[nX][nY]->GetWinViewPos();
                if ( nX == nCol )   aNewPos.X() = aPos.X();
                if ( nY == nRow )   aNewPos.Y() = aPos.Y();
                pWinArray[nX][nY]->SetZoom(nZoom);
                pWinArray[nX][nY]->SetWinViewPos(aNewPos);
                pWinArray[nX][nY]->UpdateMapOrigin();
                pWinArray[nX][nY]->Invalidate();
            }
        }
    }

    Size aVisSizePixel = pWindow->GetOutputSizePixel();
    Rectangle aVisAreaWin = pWindow->PixelToLogic( Rectangle( Point(0,0), aVisSizePixel) );
    VisAreaChanged(aVisAreaWin);

    SdView* pView = GetView();
    if (pView)
    {
        pView->VisAreaChanged(pWindow);
    }

    UpdateScrollBars();
}

/*************************************************************************
|*
|* Abbildungsparameter fuer alle Split-Windows initialisieren
|*
\************************************************************************/

void SdViewShell::InitWindows(const Point& rViewOrigin, const Size& rViewSize,
                              const Point& rWinPos, BOOL bUpdate)
{
    for (short nX = 0; nX < MAX_HSPLIT_CNT; nX++)
    {
        for (short nY = 0; nY < MAX_VSPLIT_CNT; nY++)
        {
            if ( pWinArray[nX][nY] )
            {
                pWinArray[nX][nY]->SetViewOrigin(rViewOrigin);
                pWinArray[nX][nY]->SetViewSize(rViewSize);
                pWinArray[nX][nY]->SetWinViewPos(rWinPos);

                if ( bUpdate )
                {
                    pWinArray[nX][nY]->UpdateMapOrigin();
                    pWinArray[nX][nY]->Invalidate();
                }
            }
        }
    }

    Size aVisSizePixel = pWindow->GetOutputSizePixel();
    Rectangle aVisAreaWin = pWindow->PixelToLogic( Rectangle( Point(0,0), aVisSizePixel) );
    VisAreaChanged(aVisAreaWin);

    SdView* pView = GetView();
    if (pView)
    {
        pView->VisAreaChanged(pWindow);
    }
}

/*************************************************************************
|*
|* Alle Split-Windows unter dem uebergebenen Rechteck invalidieren
|*
\************************************************************************/

void SdViewShell::InvalidateWindows()
{
    for (short nX = 0; nX < MAX_HSPLIT_CNT; nX++)
        for (short nY = 0; nY < MAX_VSPLIT_CNT; nY++)
            if ( pWinArray[nX][nY] )
                pWinArray[nX][nY]->Invalidate();
}

/*************************************************************************
|*
|* ObjectBar umschalten, ResourceID der alten ObjectBar-ID zurueckgeben
|*
\************************************************************************/

USHORT SdViewShell::SwitchObjectBar(USHORT nSdResId)
{
    USHORT nReturn = nCurrentObjectBar;

    if (nCurrentObjectBar != nSdResId && bObjectBarSwitchEnabled)
    {
        nCurrentObjectBar = nSdResId;

        if (nCurrentObjectBar)
        {
            //  SfxDispatcher* pDispatcher = GetViewFrame()->GetDispatcher();
            //pDispatcher->Push(*(SfxShell*)aShellTable.Get(nCurrentObjectBar));
            RemoveSubShell();

            if ( ISA(SdDrawViewShell) )
            {
                AddSubShell( *(SfxShell*) aShellTable.Get( RID_FORMLAYER_TOOLBOX ) );

                if ( nCurrentObjectBar == RID_DRAW_TEXT_TOOLBOX )
                    AddSubShell( *(SfxShell*) aShellTable.Get( RID_DRAW_OBJ_TOOLBOX ) );
            }

            AddSubShell( *(SfxShell*) aShellTable.Get( nCurrentObjectBar ) );
        }
    }
    return nReturn;
}

/*************************************************************************
|*
|* Auf allen Split-Windows ein Markierungsrechteck mit dem
|* uebergebenen Pen zeichnen
|*
\************************************************************************/

void SdViewShell::DrawMarkRect(const Rectangle& rRect) const
{
    for (short nX = 0; nX < MAX_HSPLIT_CNT; nX++)
    {
        for (short nY = 0; nY < MAX_VSPLIT_CNT; nY++)
        {
            if ( pWinArray[nX][nY] )
            {
                pWinArray[nX][nY]->InvertTracking(rRect, SHOWTRACK_OBJECT | SHOWTRACK_WINDOW);
            }
        }
    }
}


/*************************************************************************
|*
|* Auf allen Split-Windows ein Rechteck zeichnen. Fuer den Rahmen wird der
|* uebergebene Pen, zum Fuellen die uebergebene Brush benutzt.
|*
\************************************************************************/

void SdViewShell::DrawFilledRect( const Rectangle& rRect, const Color& rLColor,
                                  const Color& rFColor ) const
{
    for (short nX = 0; nX < MAX_HSPLIT_CNT; nX++)
    {
        for (short nY = 0; nY < MAX_VSPLIT_CNT; nY++)
        {
            if ( pWinArray[nX][nY] )
            {
                const Color& rOldLineColor = pWinArray[nX][nY]->GetLineColor();
                const Color& rOldFillColor = pWinArray[nX][nY]->GetFillColor();

                pWinArray[nX][nY]->SetLineColor( rLColor );
                pWinArray[nX][nY]->SetFillColor( rFColor );

                pWinArray[nX][nY]->DrawRect(rRect);

                pWinArray[nX][nY]->SetLineColor( rOldLineColor );
                pWinArray[nX][nY]->SetFillColor( rOldFillColor );
            }
        }
    }
}

/*************************************************************************
|*
|* Modus-Umschaltung (Draw, Slide, Outline)
|*
\************************************************************************/

IMPL_LINK( SdViewShell, ModeBtnHdl, Button *, pButton )
{
    if  ( !((ImageButton*) pButton)->IsChecked() )
    {
        if ( pButton == &aDrawBtn )
        {
            pFrameView->SetPageKind(PK_STANDARD);
            GetViewFrame()->GetDispatcher()->Execute(SID_VIEWSHELL0,
                    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
        }
        else if ( pButton == &aNotesBtn )
        {
            pFrameView->SetPageKind(PK_NOTES);
            pFrameView->SetLayerMode(FALSE);
            GetViewFrame()->GetDispatcher()->Execute(SID_VIEWSHELL0,
                    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
        }
        else if ( pButton == &aHandoutBtn )
        {
            pFrameView->SetPageKind(PK_HANDOUT);
            pFrameView->SetLayerMode(FALSE);
            GetViewFrame()->GetDispatcher()->Execute(SID_VIEWSHELL0,
                    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
        }
        else if ( pButton == &aSlideBtn )
        {
            GetViewFrame()->GetDispatcher()->Execute(SID_VIEWSHELL1,
                    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
        }
        else if ( pButton == &aOutlineBtn )
        {
            GetViewFrame()->GetDispatcher()->Execute(SID_VIEWSHELL2,
                    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
        }
        else if ( pButton == &aPresentationBtn )
        {
            GetViewFrame()->GetDispatcher()->Execute(SID_PRESENTATION,
                    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
        }
    }
    return 0;
}

/*************************************************************************
|*
|* Groesse und Raender aller Seiten setzen
|*
\************************************************************************/

void SdViewShell::SetPageSizeAndBorder(PageKind ePageKind, const Size& rNewSize,
                                       long nLeft, long nRight,
                                       long nUpper, long nLower, BOOL bScaleAll,
                                       BOOL bUndo, Orientation eOrientation)
{
    SdPage* pPage;
    SdUndoGroup* pUndoGroup = NULL;

    if (bUndo)
    {
        pUndoGroup = new SdUndoGroup(pDoc);
        String aString(SdResId(STR_UNDO_CHANGE_PAGEFORMAT));
        pUndoGroup->SetComment(aString);
    }

    USHORT i, nPageCnt = pDoc->GetMasterSdPageCount(ePageKind);

    for (i = 0; i < nPageCnt; i++)
    {
        /**********************************************************************
        * Erst alle MasterPages bearbeiten
        **********************************************************************/
        pPage = pDoc->GetMasterSdPage(i, ePageKind);

        SdUndoAction* pUndo = NULL;

        if (bUndo)
        {
            pUndo = new SdPageFormatUndoAction(pDoc, pPage,
                                pPage->GetSize(),
                                pPage->GetLftBorder(), pPage->GetRgtBorder(),
                                pPage->GetUppBorder(), pPage->GetLwrBorder(),
                                pPage->IsScaleObjects(),
                                pPage->GetOrientation(),
                                rNewSize,
                                nLeft, nRight,
                                nUpper, nLower,
                                bScaleAll,
                                eOrientation);
            pUndoGroup->AddAction(pUndo);
        }

        const SfxPoolItem* pPoolItem = NULL;

        if (rNewSize.Width() > 0 ||
            nLeft  >= 0 || nRight >= 0 || nUpper >= 0 || nLower >= 0)
        {
            Rectangle aNewBorderRect(nLeft, nUpper, nRight, nLower);
            pPage->ScaleObjects(rNewSize, aNewBorderRect, bScaleAll);

            if (rNewSize.Width() > 0)
                pPage->SetSize(rNewSize);
        }

        if( nLeft  >= 0 || nRight >= 0 || nUpper >= 0 || nLower >= 0 )
        {
            pPage->SetBorder(nLeft, nUpper, nRight, nLower);
        }

        pPage->SetOrientation(eOrientation);

        if ( ePageKind == PK_STANDARD )
            pDoc->GetMasterSdPage(i, PK_NOTES)->CreateTitleAndLayout();

        pPage->CreateTitleAndLayout();
    }

    nPageCnt = pDoc->GetSdPageCount(ePageKind);

    for (i = 0; i < nPageCnt; i++)
    {
        /**********************************************************************
        * Danach alle Pages bearbeiten
        **********************************************************************/
        pPage = pDoc->GetSdPage(i, ePageKind);

        SdUndoAction* pUndo = NULL;

        if (bUndo)
        {
            pUndo = new SdPageFormatUndoAction(pDoc, pPage,
                                pPage->GetSize(),
                                pPage->GetLftBorder(), pPage->GetRgtBorder(),
                                pPage->GetUppBorder(), pPage->GetLwrBorder(),
                                pPage->IsScaleObjects(),
                                pPage->GetOrientation(),
                                rNewSize,
                                nLeft, nRight,
                                nUpper, nLower,
                                bScaleAll,
                                eOrientation);
            pUndoGroup->AddAction(pUndo);
        }

        if (rNewSize.Width() > 0 ||
            nLeft  >= 0 || nRight >= 0 || nUpper >= 0 || nLower >= 0)
        {
            Rectangle aNewBorderRect(nLeft, nUpper, nRight, nLower);
            pPage->ScaleObjects(rNewSize, aNewBorderRect, bScaleAll);

            if (rNewSize.Width() > 0)
                pPage->SetSize(rNewSize);
        }

        if( nLeft  >= 0 || nRight >= 0 || nUpper >= 0 || nLower >= 0 )
        {
            pPage->SetBorder(nLeft, nUpper, nRight, nLower);
        }

        pPage->SetOrientation(eOrientation);

        if ( ePageKind == PK_STANDARD )
        {
            SdPage* pNotesPage = pDoc->GetSdPage(i, PK_NOTES);
            pNotesPage->SetAutoLayout( pNotesPage->GetAutoLayout() );
        }

        pPage->SetAutoLayout( pPage->GetAutoLayout() );
    }

    // Handoutseite an neues Format der Standardseiten anpassen
    if ( ePageKind == PK_STANDARD )
        pDoc->GetSdPage(0, PK_HANDOUT)->CreateTitleAndLayout(TRUE);

    if (bUndo)
    {
        // Undo Gruppe dem Undo Manager uebergeben
        GetViewFrame()->GetObjectShell()->GetUndoManager()->AddUndoAction(pUndoGroup);
    }

    long nWidth = pPage->GetSize().Width();
    long nHeight = pPage->GetSize().Height();

    Point aPageOrg = Point(nWidth, nHeight / 2);
    Size aViewSize = Size(nWidth * 3, nHeight * 2);

    InitWindows(aPageOrg, aViewSize, Point(-1, -1), TRUE);

    Point aVisAreaPos;

    if ( pDocSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
    {
        aVisAreaPos = pDocSh->GetVisArea(ASPECT_CONTENT).TopLeft();
    }

    SdView* pView = GetView();
    if (pView)
    {
        pView->SetWorkArea(Rectangle(Point() - aVisAreaPos - aPageOrg, aViewSize));
    }

    UpdateScrollBars();

    Point aNewOrigin(pPage->GetLftBorder(), pPage->GetUppBorder());

    if (pView)
    {
        pView->GetPageViewPvNum(0)->SetPageOrigin(aNewOrigin);
    }

    GetViewFrame()->GetBindings().Invalidate(SID_RULER_NULL_OFFSET);

    // auf (neue) Seitengroesse zoomen
    GetViewFrame()->GetDispatcher()->Execute(SID_SIZE_PAGE,
            SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
}



/*************************************************************************
|*
|* QueryDrop-Event
|*
\************************************************************************/

BOOL SdViewShell::QueryDrop(DropEvent& rEvt, SdWindow* pWin,
                            USHORT nPage, USHORT nLayer)
{
    BOOL bReturn = FALSE;

    SdView* pView = GetView();

    if (pView)
    {
        bReturn = pView->QueryDrop(rEvt, pWin, nPage, nLayer);
    }

    return (bReturn);
}

/*************************************************************************
|*
|* Drop-Event
|*
\************************************************************************/

BOOL SdViewShell::Drop(const DropEvent& rEvt, SdWindow* pWin,
                       USHORT nPage, USHORT nLayer)
{
    BOOL bReturn = FALSE;

    SdView* pView = GetView();

    if (pView)
    {
        bReturn = pView->Drop(rEvt, pWin, nPage, nLayer);
    }

    return (bReturn);
}


/*************************************************************************
|*
|* Zoom-Faktor fuer InPlace einstellen
|*
\************************************************************************/

void __EXPORT SdViewShell::SetZoomFactor(const Fraction& rZoomX, const Fraction& rZoomY)
{
    long nZoom = (long)((double) rZoomX * 100);
    SetZoom(nZoom);
}


/*************************************************************************
|*
|* Aktives Fenster setzen
|*
\************************************************************************/

void SdViewShell::SetActiveWindow(SdWindow* pWin)
{
    if (GetWindow() != pWin)
    {
        if (pWin)
        {
            pWin->EnableChildTransparentMode();
        }

        SetWindow(pWin);
    }

    if (pWindow != pWin)
    {
        pWindow = pWin;

        SdView* pView = GetView();

        if (pView)
        {
            pView->SetActualWin(pWin);
        }
        if (pFuSlideShow)
        {
            pFuSlideShow->SetWindow(pWin);
        }
        if (pFuActual)
        {
            pFuActual->SetWindow(pWin);
        }
    }
}

/*************************************************************************
|*
|* RequestHelp event
|*
\************************************************************************/

BOOL SdViewShell::RequestHelp(const HelpEvent& rHEvt, SdWindow* pWin)
{
    BOOL bReturn = FALSE;

    if (rHEvt.GetMode())
    {
        if (pFuSlideShow)
        {
            bReturn = pFuSlideShow->RequestHelp(rHEvt);
        }
        else if (pFuActual)
        {
            bReturn = pFuActual->RequestHelp(rHEvt);
        }
    }

    return(bReturn);
}



/*************************************************************************
|*
|* Read FrameViews data and set actual views data
|*
\************************************************************************/

void SdViewShell::ReadFrameViewData(FrameView* pView)
{
}



/*************************************************************************
|*
|* Write actual views data to FrameView
|*
\************************************************************************/

void SdViewShell::WriteFrameViewData()
{
}

/*************************************************************************
|*
|* Auf allen Split-Windows ein Update erzwingen.
|*
\************************************************************************/

void SdViewShell::UpdateWindows()
{
    for (short nX = 0; nX < MAX_HSPLIT_CNT; nX++)
        for (short nY = 0; nY < MAX_VSPLIT_CNT; nY++)
            if ( pWinArray[nX][nY] )
                pWinArray[nX][nY]->Update();
}

/*************************************************************************
|*
|* OLE-Object aktivieren
|*
\************************************************************************/

BOOL SdViewShell::ActivateObject(SdrOle2Obj* pObj, long nVerb)
{
    ErrCode aErrCode = 0;
    SfxErrorContext aEC(ERRCTX_SO_DOVERB, pWindow, RID_SO_ERRCTX);
    BOOL bAbort = FALSE;
    BOOL bChartActive = FALSE;
    pDocSh->SetWaitCursor( TRUE );

    SvInPlaceObjectRef aIPObj = pObj->GetObjRef();

    if ( !aIPObj.Is() )
    {
        /**********************************************************
        * Leeres OLE-Objekt mit OLE-Objekt versehen
        **********************************************************/
        SvInPlaceObjectRef aNewIPObj;
        SvStorageRef aStor = new SvStorage( String(), STREAM_STD_READWRITE);

        String aName = pObj->GetProgName();

        if( aName.EqualsAscii( "StarChart" ))
        {
            aNewIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit(
                             SchModuleDummy::GetID(SOFFICE_FILEFORMAT_50), aStor);
        }
        else if( aName.EqualsAscii( "StarOrg" ))
        {
            // z.Z noch Nummer vom StarChart!
            aNewIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit(
                             SchModuleDummy::GetID(SOFFICE_FILEFORMAT_50), aStor);
        }
        else if( aName.EqualsAscii( "StarCalc" ))
        {
            aNewIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit(
                             ScModuleDummy::GetID(SOFFICE_FILEFORMAT_50), aStor);
        }
#ifdef STARIMAGE_AVAILABLE
        else if( aName.EqualsAscii( "StarImage" ))
        {
            aNewIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit(
                             SimModuleDummy::GetID(SOFFICE_FILEFORMAT_50), aStor);
        }
#endif
        else if( aName.EqualsAscii( "StarMath" ))
        {
            aNewIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit(
                             SmModuleDummy::GetID(SOFFICE_FILEFORMAT_50), aStor);
        }
        else
        {
            // Dialog "OLE-Objekt einfuegen" aufrufen
            pDocSh->SetWaitCursor( FALSE );
            GetViewFrame()->GetDispatcher()->Execute(SID_INSERT_OBJECT,
                            SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD);
            aNewIPObj = pObj->GetObjRef();
            pDocSh->SetWaitCursor( TRUE );

            if (!aNewIPObj.Is())
            {
                bAbort = TRUE;
            }
        }

        if ( aNewIPObj.Is() )
        {
            /******************************************************
            * OLE-Objekt ist nicht mehr leer
            ******************************************************/
            pObj->SetEmptyPresObj(FALSE);
            pObj->SetOutlinerParaObject(NULL);
            pObj->SetGraphic(NULL);

            /******************************************************
            * Das leere OLE-Objekt bekommt ein neues IPObj
            ******************************************************/
            if (aName.Len())
            {
                String aObjName = pDocSh->InsertObject(aNewIPObj, String())->GetObjName();
                pObj->SetObjRef(aNewIPObj);
                pObj->SetName(aObjName);
            }
            else
            {
                // Das Einfuegen hat der Dialog schon gemacht
                pObj->SetObjRef(aNewIPObj);
            }

            Rectangle aRect = pObj->GetLogicRect();
            aNewIPObj->SetVisAreaSize( aRect.GetSize() );

            SetVerbs( &aNewIPObj->GetVerbList() );

            if( aName.EqualsAscii( "StarChart" ))
            {
                bChartActive = TRUE;
                SchDLL::Update(aNewIPObj, NULL, pWindow);       // BM: use different DLL-call
            }

            nVerb = SVVERB_SHOW;
        }
        else
        {
            aErrCode = ERRCODE_SFX_OLEGENERAL;
        }
    }

    if ( aErrCode == 0  )
    {
        SdView* pView = GetView();

        if (pView->IsTextEdit())
        {
            pView->EndTextEdit();
        }

        const SvInPlaceObjectRef& rIPObjRef = pObj->GetObjRef();

        if (!rIPObjRef->IsLink())
        {
            SfxInPlaceClientRef pSdClient = (SdClient*) FindIPClient(rIPObjRef, pWindow);

            if ( !pSdClient.Is() )
            {
                pSdClient = new SdClient(pObj, this, pWindow);
            }

            rIPObjRef->DoConnect(pSdClient);
            Rectangle aRect = pObj->GetLogicRect();
            SvClientData* pClientData = pSdClient->GetEnv();

            if (pClientData)
            {
                Size aDrawSize = aRect.GetSize();
                Size aObjAreaSize = rIPObjRef->GetVisArea().GetSize();
                aObjAreaSize = OutputDevice::LogicToLogic( aObjAreaSize,
                                                       rIPObjRef->GetMapUnit(),
                                                       pDoc->GetScaleUnit() );

                // sichtbarer Ausschnitt wird nur inplace veraendert!
                aRect.SetSize(aObjAreaSize);
                pClientData->SetObjArea(aRect);

                Fraction aScaleWidth (aDrawSize.Width(),  aObjAreaSize.Width() );
                Fraction aScaleHeight(aDrawSize.Height(), aObjAreaSize.Height() );
                aScaleWidth.ReduceInaccurate(10);       // kompatibel zum SdrOle2Obj
                aScaleHeight.ReduceInaccurate(10);
                pClientData->SetSizeScale(aScaleWidth, aScaleHeight);
            }

            // switching to edit mode for OLEs was disabled when OLE
            // is member of a group all the time. I dont know why it
            // was possible in previous versions. But I see no
            // reason not to allow it. (src539)
//          if( !pView->IsGroupEntered() )
            DoVerb(pSdClient, nVerb);   // ErrCode wird ggf. vom Sfx ausgegeben
        }
        else
        {
            aErrCode = rIPObjRef->DoVerb(nVerb);
        }

        GetViewFrame()->GetBindings().Invalidate( SID_NAVIGATOR_STATE, TRUE, FALSE );
    }

    pDocSh->SetWaitCursor( FALSE );

    if (aErrCode != 0 && !bAbort)
    {
        ErrorHandler::HandleError(* new StringErrorInfo(aErrCode, String() ) );
    }

    BOOL bActivated = FALSE;

    if (aErrCode == 0)
    {
        bActivated = TRUE;
    }

    return(bActivated);
}

/*************************************************************************
|*
|* umschliessendes Rechteck aller (Split-)Fenster zurueckgeben.
|*
\************************************************************************/

const Rectangle& SdViewShell::GetAllWindowRect()
{
    aAllWindowRect.SetPos(pWinArray[0][0]->OutputToScreenPixel(Point(0,0)));
    return aAllWindowRect;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void SdViewShell::CancelSearching()
{
    delete pFuSearch;
    pFuSearch = NULL;
}

/*************************************************************************
|*
|* Read user data
|*
\************************************************************************/

void __EXPORT SdViewShell::ReadUserData(const String& rString)
{
    SfxViewShell::ReadUserData(rString);

    // Auf an FrameView gemerkte VisArea zoomen
    GetViewFrame()->GetDispatcher()->Execute(SID_SIZE_VISAREA,
                    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
}

/*************************************************************************
|*
|* Write user data
|*
\************************************************************************/

void __EXPORT SdViewShell::WriteUserData(String& rString)
{
    SfxViewShell::WriteUserData(rString);

    // Das Schreiben unserer Daten erfolgt stets in WriteFrameViewData()
    WriteFrameViewData();
}


/*************************************************************************
|*
|* Lineale ein- / ausschalten
|*
\************************************************************************/

void SdViewShell::SetRuler(BOOL bRuler)
{
    bHasRuler = bRuler;

    for (short nX = 0; nX < MAX_HSPLIT_CNT; nX++)
    {
        if (pHRulerArray[nX])
        {
            if (bHasRuler)
            {
                pHRulerArray[nX]->Show();
            }
            else
            {
                pHRulerArray[nX]->Hide();
            }
        }

        for (short nY = 0; nY < MAX_VSPLIT_CNT; nY++)
        {
            if (nX == 0 && pVRulerArray[nY])
            {
                if (bHasRuler)
                {
                    pVRulerArray[nY]->Show();
                }
                else
                {
                    pVRulerArray[nY]->Hide();
                }
            }
        }
    }

    InvalidateBorder();
}


#ifdef WNT
#pragma optimize ( "", on )
#endif



