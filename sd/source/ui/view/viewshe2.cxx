/*************************************************************************
 *
 *  $RCSfile: viewshe2.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: ka $ $Date: 2001-09-13 11:18:58 $
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

#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

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

#ifndef _SD_UNOKYWDS_HXX_
#include "unokywds.hxx"
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

using namespace com::sun::star;

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
                             SchModuleDummy::GetID(SOFFICE_FILEFORMAT_CURRENT), aStor);
        }
        else if( aName.EqualsAscii( "StarOrg" ))
        {
            // z.Z noch Nummer vom StarChart!
            aNewIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit(
                             SchModuleDummy::GetID(SOFFICE_FILEFORMAT_CURRENT), aStor);
        }
        else if( aName.EqualsAscii( "StarCalc" ))
        {
            aNewIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit(
                             ScModuleDummy::GetID(SOFFICE_FILEFORMAT_CURRENT), aStor);
        }
#ifdef STARIMAGE_AVAILABLE
        else if( aName.EqualsAscii( "StarImage" ))
        {
            aNewIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit(
                             SimModuleDummy::GetID(SOFFICE_FILEFORMAT_CURRENT), aStor);
        }
#endif
        else if( aName.EqualsAscii( "StarMath" ))
        {
            aNewIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit(
                             SmModuleDummy::GetID(SOFFICE_FILEFORMAT_CURRENT), aStor);
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
                pObj->SetPersistName(aObjName);
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

/*************************************************************************
|*
|* AcceptDrop
|*
\************************************************************************/

sal_Int8 SdViewShell::AcceptDrop( const AcceptDropEvent& rEvt, DropTargetHelper& rTargetHelper,
                                  SdWindow* pTargetWindow, USHORT nPage, USHORT nLayer )
{
    SdView* pView = GetView();
    return( pView ? pView->AcceptDrop( rEvt, rTargetHelper, pTargetWindow, nPage, nLayer ) : DND_ACTION_NONE );
}

/*************************************************************************
|*
|* ExecuteDrop
|*
\************************************************************************/

sal_Int8 SdViewShell::ExecuteDrop( const ExecuteDropEvent& rEvt, DropTargetHelper& rTargetHelper,
                                   SdWindow* pTargetWindow, USHORT nPage, USHORT nLayer )
{
    SdView* pView = GetView();
    return( pView ? pView->ExecuteDrop( rEvt, rTargetHelper, pTargetWindow, nPage, nLayer ) : DND_ACTION_NONE );
}

#ifdef WNT
#pragma optimize ( "", on )
#endif

static rtl::OUString createHelpLinesString( const SdrHelpLineList& rHelpLines )
{
    rtl::OUStringBuffer aLines;

    const USHORT nCount = rHelpLines.GetCount();
    for( USHORT nHlpLine = 0; nHlpLine < nCount; nHlpLine++ )
    {
        const SdrHelpLine& rHelpLine = rHelpLines[nHlpLine];
        const Point& rPos = rHelpLine.GetPos();

        switch( rHelpLine.GetKind() )
        {
            case SDRHELPLINE_POINT:
                aLines.append( (sal_Unicode)'P' );
                aLines.append( (sal_Int32)rPos.X() );
                aLines.append( (sal_Unicode)',' );
                aLines.append( (sal_Int32)rPos.Y() );
                break;
            case SDRHELPLINE_VERTICAL:
                aLines.append( (sal_Unicode)'V' );
                aLines.append( (sal_Int32)rPos.X() );
                break;
            case SDRHELPLINE_HORIZONTAL:
                aLines.append( (sal_Unicode)'H' );
                aLines.append( (sal_Int32)rPos.Y() );
                break;
            default:
                DBG_ERROR( "Unsupported helpline Kind!" );
        }
    }

    return aLines.makeStringAndClear();
}

#define NUM_VIEW_SETTINGS 53
void SdViewShell::WriteUserDataSequence ( ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >& rSequence, sal_Bool bBrowse )
{
    rSequence.realloc ( NUM_VIEW_SETTINGS );
    sal_Int16 nIndex = 0;
    com::sun::star::beans::PropertyValue *pValue = rSequence.getArray();

    sal_uInt16 nViewID( GetViewFrame()->GetCurViewId());
    pValue->Name = rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_ViewId ) );
    rtl::OUStringBuffer sBuffer ( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "view" ) ) );
    sBuffer.append( static_cast<sal_Int32>(nViewID));
    pValue->Value <<= sBuffer.makeStringAndClear();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_GridIsVisible ) );
    pValue->Value <<= (sal_Bool)pFrameView->IsGridVisible();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_GridIsFront ) );
    pValue->Value <<= (sal_Bool)pFrameView->IsGridFront();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsSnapToGrid ) );
    pValue->Value <<= (sal_Bool)pFrameView->IsGridSnap();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsSnapToPageMargins ) );
    pValue->Value <<= (sal_Bool)pFrameView->IsBordSnap();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsSnapToSnapLines ) );
    pValue->Value <<= (sal_Bool)pFrameView->IsHlplSnap();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsSnapToObjectFrame ) );
    pValue->Value <<= (sal_Bool)pFrameView->IsOFrmSnap();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsSnapToObjectPoints ) );
    pValue->Value <<= (sal_Bool)pFrameView->IsOPntSnap();
    pValue++;nIndex++;

//  pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsSnapLinesVisible ) );
//  pValue->Value <<= (sal_Bool)pFrameView->IsHlplVisible();
//  pValue++;nIndex++;

//  pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsDragStripes ) );
//  pValue->Value <<= (sal_Bool)pFrameView->IsDragStripes();
//  pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsPlusHandlesAlwaysVisible ) );
    pValue->Value <<= (sal_Bool)pFrameView->IsPlusHandlesAlwaysVisible();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsFrameDragSingles ) );
    pValue->Value <<= (sal_Bool)pFrameView->IsFrameDragSingles();
    pValue++;nIndex++;

//  pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsMarkedHitMovesAlways ) );
//  pValue->Value <<= (sal_Bool)pFrameView->IsMarkedHitMovesAlways();
//  pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_EliminatePolyPointLimitAngle ) );
    pValue->Value <<= (sal_Int32)pFrameView->GetEliminatePolyPointLimitAngle();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsEliminatePolyPoints ) );
    pValue->Value <<= (sal_Bool)pFrameView->IsEliminatePolyPoints();
    pValue++;nIndex++;

//  pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsLineDraft ) );
//  pValue->Value <<= (sal_Bool)pFrameView->IsLineDraft();
//  pValue++;nIndex++;

//  pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsFillDraft ) );
//  pValue->Value <<= (sal_Bool)pFrameView->IsFillDraft();
//  pValue++;nIndex++;

//  pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsTextDraft ) );
//  pValue->Value <<= (sal_Bool)pFrameView->IsTextDraft();
//  pValue++;nIndex++;

//  pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsGrafDraft ) );
//  pValue->Value <<= (sal_Bool)pFrameView->IsGrafDraft();
//  pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_VisibleLayers ) );
    pFrameView->GetVisibleLayers().QueryValue( pValue->Value );
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_PrintableLayers ) );
    pFrameView->GetPrintableLayers().QueryValue( pValue->Value );
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_LockedLayers ) );
    pFrameView->GetLockedLayers().QueryValue( pValue->Value );
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_NoAttribs ) );
    pValue->Value <<= (sal_Bool)pFrameView->IsNoAttribs();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_NoColors ) );
    pValue->Value <<= (sal_Bool)pFrameView->IsNoColors();
    pValue++;nIndex++;

    if( pFrameView->GetStandardHelpLines().GetCount() )
    {
        pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_SnapLinesDrawing ) );
        pValue->Value <<= createHelpLinesString( pFrameView->GetStandardHelpLines() );
        pValue++;nIndex++;
    }

    if( pFrameView->GetNotesHelpLines().GetCount() )
    {
        pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_SnapLinesNotes ) );
        pValue->Value <<= createHelpLinesString( pFrameView->GetNotesHelpLines() );
        pValue++;nIndex++;
    }

    if( pFrameView->GetHandoutHelpLines().GetCount() )
    {
        pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_SnapLinesHandout ) );
        pValue->Value <<= createHelpLinesString( pFrameView->GetHandoutHelpLines() );
        pValue++;nIndex++;
    }

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_RulerIsVisible ) );
    pValue->Value <<= (sal_Bool)pFrameView->HasRuler();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_PageKind ) );
    pValue->Value <<= (sal_Int16)pFrameView->GetPageKind();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_SelectedPage ) );
    pValue->Value <<= (sal_Int16)pFrameView->GetSelectedPage();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsLayerMode ) );
    pValue->Value <<= (sal_Bool)pFrameView->IsLayerMode();
    pValue++;nIndex++;

//  pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsQuickEdit ) );
//  pValue->Value <<= (sal_Bool)pFrameView->IsQuickEdit();
//  pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsBigHandles ) );
    pValue->Value <<= (sal_Bool)pFrameView->IsBigHandles();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsDoubleClickTextEdit ) );
    pValue->Value <<= (sal_Bool)pFrameView->IsDoubleClickTextEdit();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsClickChangeRotation ) );
    pValue->Value <<= (sal_Bool)pFrameView->IsClickChangeRotation();
    pValue++;nIndex++;

//  pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsDragWithCopy ) );
//  pValue->Value <<= (sal_Bool)pFrameView->IsDragWithCopy();
//  pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_SlidesPerRow ) );
    pValue->Value <<= (sal_Int16)pFrameView->GetSlidesPerRow();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_DrawMode ) );
    pValue->Value <<= (sal_Int32)pFrameView->GetDrawMode();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_PreviewDrawMode ) );
    pValue->Value <<= (sal_Int32)pFrameView->GetPreviewDrawMode();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsShowPreviewInPageMode ) );
    pValue->Value <<= (sal_Bool)pFrameView->IsShowPreviewInPageMode();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsShowPreviewInMasterPageMode ) );
    pValue->Value <<= (sal_Bool)pFrameView->IsShowPreviewInMasterPageMode();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_SetShowPreviewInOutlineMode ) );
    pValue->Value <<= (sal_Bool)pFrameView->IsShowPreviewInOutlineMode();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_EditModeStandard ) );
    pValue->Value <<= (sal_Int32)pFrameView->GetViewShEditMode( PK_STANDARD );
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_EditModeNotes ) );
    pValue->Value <<= (sal_Int32)pFrameView->GetViewShEditMode( PK_NOTES );
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_EditModeHandout ) );
    pValue->Value <<= (sal_Int32)pFrameView->GetViewShEditMode( PK_HANDOUT );
    pValue++;nIndex++;

    {
        const Rectangle aVisArea = pFrameView->GetVisArea();

        pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_VisibleAreaTop ) );
        pValue->Value <<= (sal_Int32)aVisArea.Top();
        pValue++;nIndex++;

        pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_VisibleAreaLeft ) );
        pValue->Value <<= (sal_Int32)aVisArea.Left();
        pValue++;nIndex++;

        pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_VisibleAreaWidth ) );
        pValue->Value <<= (sal_Int32)aVisArea.GetWidth();
        pValue++;nIndex++;

        pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_VisibleAreaHeight ) );
        pValue->Value <<= (sal_Int32)aVisArea.GetHeight();
        pValue++;nIndex++;
    }

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_GridCoarseWidth ) );
    pValue->Value <<= (sal_Int32)pFrameView->GetGridCoarse().Width();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_GridCoarseHeight ) );
    pValue->Value <<= (sal_Int32)pFrameView->GetGridCoarse().Height();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_GridFineWidth ) );
    pValue->Value <<= (sal_Int32)pFrameView->GetGridFine().Width();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_GridFineHeight ) );
    pValue->Value <<= (sal_Int32)pFrameView->GetGridFine().Height();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_GridSnapWidth ) );
    pValue->Value <<= (sal_Int32)pFrameView->GetSnapGrid().Width();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_GridSnapHeight ) );
    pValue->Value <<= (sal_Int32)pFrameView->GetSnapGrid().Height();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_GridSnapWidthXNumerator ) );
    pValue->Value <<= (sal_Int32)pFrameView->GetSnapGridWidthX().GetNumerator();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_GridSnapWidthXDenominator ) );
    pValue->Value <<= (sal_Int32)pFrameView->GetSnapGridWidthX().GetDenominator();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_GridSnapWidthYNumerator ) );
    pValue->Value <<= (sal_Int32)pFrameView->GetSnapGridWidthY().GetNumerator();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_GridSnapWidthYDenominator ) );
    pValue->Value <<= (sal_Int32)pFrameView->GetSnapGridWidthY().GetDenominator();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_IsAngleSnapEnabled ) );
    pValue->Value <<= (sal_Bool)pFrameView->IsAngleSnapEnabled();
    pValue++;nIndex++;

    pValue->Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( sUNO_View_SnapAngle ) );
    pValue->Value <<= (sal_Int32)pFrameView->GetSnapAngle();
    pValue++;nIndex++;

    DBG_ASSERT( nIndex <= NUM_VIEW_SETTINGS, "Incorrect number of view settings" );

    if ( nIndex < NUM_VIEW_SETTINGS )
        rSequence.realloc ( nIndex );
}

static void createHelpLinesFromString( const rtl::OUString& rLines, SdrHelpLineList& rHelpLines )
{
    const sal_Unicode * pStr = rLines.getStr();
    SdrHelpLine aNewHelpLine;
    rtl::OUStringBuffer sBuffer;

    while( *pStr )
    {
        Point aPoint;

        switch( *pStr )
        {
        case (sal_Unicode)'P':
            aNewHelpLine.SetKind( SDRHELPLINE_POINT );
            break;
        case (sal_Unicode)'V':
            aNewHelpLine.SetKind( SDRHELPLINE_VERTICAL );
            break;
        case (sal_Unicode)'H':
            aNewHelpLine.SetKind( SDRHELPLINE_HORIZONTAL );
            break;
        default:
            DBG_ERROR( "syntax error in snap lines settings string" );
            return;
        }

        pStr++;

        while( *pStr >= sal_Unicode('0') && *pStr <= sal_Unicode('9') )
        {
            sBuffer.append( *pStr++ );
        }

        sal_Int32 nValue = sBuffer.makeStringAndClear().toInt32();

        if( aNewHelpLine.GetKind() == SDRHELPLINE_HORIZONTAL )
        {
            aPoint.Y() = nValue;
        }
        else
        {
            aPoint.X() = nValue;

            if( aNewHelpLine.GetKind() == SDRHELPLINE_POINT )
            {
                if( *pStr++ != ',' )
                    return;

                while( *pStr >= sal_Unicode('0') && *pStr <= sal_Unicode('9') )
                {
                    sBuffer.append( *pStr++ );
                }

                aPoint.Y() = sBuffer.makeStringAndClear().toInt32();

            }
        }

        aNewHelpLine.SetPos( aPoint );
        rHelpLines.Insert( aNewHelpLine );
    }
}

void SdViewShell::ReadUserDataSequence ( const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >& rSequence, sal_Bool bBrowse )
{
    const sal_Int32 nLength = rSequence.getLength();
    if (nLength)
    {
        sal_Bool bBool;
        sal_Int32 nInt32;
        sal_Int16 nInt16;
        rtl::OUString aString;

        sal_Int32 aSnapGridWidthXNum = pFrameView->GetSnapGridWidthX().GetNumerator();
        sal_Int32 aSnapGridWidthXDom = pFrameView->GetSnapGridWidthX().GetDenominator();

        sal_Int32 aSnapGridWidthYNum = pFrameView->GetSnapGridWidthY().GetNumerator();
        sal_Int32 aSnapGridWidthYDom = pFrameView->GetSnapGridWidthY().GetDenominator();

        const com::sun::star::beans::PropertyValue *pValue = rSequence.getConstArray();
        for (sal_Int16 i = 0 ; i < nLength; i++, pValue++ )
        {
            if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_ViewId ) ) )
            {
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_SnapLinesDrawing ) ) )
            {
                if( pValue->Value >>= aString )
                {
                    SdrHelpLineList aHelpLines;
                    createHelpLinesFromString( aString, aHelpLines );
                    pFrameView->SetStandardHelpLines( aHelpLines );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_SnapLinesNotes ) ) )
            {
                if( pValue->Value >>= aString )
                {
                    SdrHelpLineList aHelpLines;
                    createHelpLinesFromString( aString, aHelpLines );
                    pFrameView->SetNotesHelpLines( aHelpLines );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_SnapLinesHandout ) ) )
            {
                if( pValue->Value >>= aString )
                {
                    SdrHelpLineList aHelpLines;
                    createHelpLinesFromString( aString, aHelpLines );
                    pFrameView->SetHandoutHelpLines( aHelpLines );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_RulerIsVisible ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    pFrameView->SetRuler( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_PageKind ) ) )
            {
                if( pValue->Value >>= nInt16 )
                {
                    pFrameView->SetPageKind( (PageKind)nInt16 );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_SelectedPage ) ) )
            {
                if( pValue->Value >>= nInt16 )
                {
                    pFrameView->SetSelectedPage( (USHORT)nInt16 );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsLayerMode ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    pFrameView->SetLayerMode( bBool );
                }
            }
/*          else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsQuickEdit ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    pFrameView->SetQuickEdit( bBool );
                }
            }
*/          else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsBigHandles ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    pFrameView->SetBigHandles( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsDoubleClickTextEdit ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    pFrameView->SetDoubleClickTextEdit( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsClickChangeRotation ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    pFrameView->SetClickChangeRotation( bBool );
                }
            }
/*          else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsDragWithCopy ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    pFrameView->SetDragWithCopy( bBool );
                }
            }
*/          else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_SlidesPerRow ) ) )
            {
                if( pValue->Value >>= nInt16 )
                {
                    pFrameView->SetSlidesPerRow( (USHORT)nInt16 );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_DrawMode ) ) )
            {
                if( pValue->Value >>= nInt32 )
                {
                    pFrameView->SetDrawMode( (ULONG)nInt32 );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_PreviewDrawMode ) ) )
            {
                if( pValue->Value >>= nInt32 )
                {
                    pFrameView->SetPreviewDrawMode( (ULONG)nInt32 );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsShowPreviewInPageMode ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    pFrameView->SetShowPreviewInPageMode( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsShowPreviewInMasterPageMode ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    pFrameView->SetShowPreviewInMasterPageMode( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_SetShowPreviewInOutlineMode ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    pFrameView->SetShowPreviewInOutlineMode( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_EditModeStandard ) ) )
            {
                if( pValue->Value >>= nInt32 )
                {
                    pFrameView->SetViewShEditMode( (EditMode)nInt32, PK_STANDARD );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_EditModeNotes ) ) )
            {
                if( pValue->Value >>= nInt32 )
                {
                    pFrameView->SetViewShEditMode( (EditMode)nInt32, PK_NOTES );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_EditModeHandout ) ) )
            {
                if( pValue->Value >>= nInt32 )
                {
                    pFrameView->SetViewShEditMode( (EditMode)nInt32, PK_HANDOUT );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_VisibleAreaTop ) ) )
            {
                sal_Int32 nTop;
                if( pValue->Value >>= nTop )
                {
                    Rectangle aVisArea( pFrameView->GetVisArea() );
                    aVisArea.nBottom += nTop - aVisArea.nTop;
                    aVisArea.nTop = nTop;
                    pFrameView->SetVisArea( aVisArea );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_VisibleAreaLeft ) ) )
            {
                sal_Int32 nLeft;
                if( pValue->Value >>= nLeft )
                {
                    Rectangle aVisArea( pFrameView->GetVisArea() );
                    aVisArea.nRight += nLeft - aVisArea.nLeft;
                    aVisArea.nLeft = nLeft;
                    pFrameView->SetVisArea( aVisArea );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_VisibleAreaWidth ) ) )
            {
                sal_Int32 nWidth;
                if( pValue->Value >>= nWidth )
                {
                    Rectangle aVisArea( pFrameView->GetVisArea() );
                    aVisArea.nRight = aVisArea.nLeft + nWidth - 1;
                    pFrameView->SetVisArea( aVisArea );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_VisibleAreaHeight ) ) )
            {
                sal_Int32 nHeight;
                if( pValue->Value >>= nHeight )
                {
                    Rectangle aVisArea( pFrameView->GetVisArea() );
                    aVisArea.nBottom = nHeight + aVisArea.nTop - 1;
                    pFrameView->SetVisArea( aVisArea );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_GridIsVisible ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    pFrameView->SetGridVisible( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsSnapToGrid ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    pFrameView->SetGridSnap( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_GridIsFront ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    pFrameView->SetGridFront( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsSnapToPageMargins ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    pFrameView->SetBordSnap( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsSnapToSnapLines ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    pFrameView->SetHlplSnap( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsSnapToObjectFrame ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    pFrameView->SetOFrmSnap( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsSnapToObjectPoints ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    pFrameView->SetOPntSnap( bBool );
                }
            }
/*          else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsSnapLinesVisible ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    pFrameView->SetHlplVisible( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsDragStripes ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    pFrameView->SetDragStripes( bBool );
                }
            }
*/          else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsPlusHandlesAlwaysVisible ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    pFrameView->SetPlusHandlesAlwaysVisible( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsFrameDragSingles ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    pFrameView->SetFrameDragSingles( bBool );
                }
            }
/*          else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsMarkedHitMovesAlways ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    pFrameView->SetMarkedHitMovesAlways( bBool );
                }
            }
*/          else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_EliminatePolyPointLimitAngle ) ) )
            {
                if( pValue->Value >>= nInt32 )
                {
                    pFrameView->SetEliminatePolyPointLimitAngle( nInt32 );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsEliminatePolyPoints ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    pFrameView->SetEliminatePolyPoints( bBool );
                }
            }
/*
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsLineDraft ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    pFrameView->SetLineDraft( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsFillDraft ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    pFrameView->SetFillDraft( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsTextDraft ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    pFrameView->SetTextDraft( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsGrafDraft ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    pFrameView->SetGrafDraft( bBool );
                }
            }
*/
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_ActiveLayer ) ) )
            {
                if( pValue->Value >>= aString )
                {
                    pFrameView->SetActiveLayer( aString );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_NoAttribs ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    pFrameView->SetNoAttribs( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_NoColors ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    pFrameView->SetNoColors( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_GridCoarseWidth ) ) )
            {
                if( pValue->Value >>= nInt32 )
                {
                    const Size aCoarse( nInt32, pFrameView->GetGridCoarse().Height() );
                    pFrameView->SetGridCoarse( aCoarse );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_GridCoarseHeight ) ) )
            {
                if( pValue->Value >>= nInt32 )
                {
                    const Size aCoarse( pFrameView->GetGridCoarse().Width(), nInt32 );
                    pFrameView->SetGridCoarse( aCoarse );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_GridFineWidth ) ) )
            {
                if( pValue->Value >>= nInt32 )
                {
                    const Size aCoarse( nInt32, pFrameView->GetGridFine().Height() );
                    pFrameView->SetGridFine( aCoarse );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_GridFineHeight ) ) )
            {
                if( pValue->Value >>= nInt32 )
                {
                    const Size aCoarse( pFrameView->GetGridFine().Width(), nInt32 );
                    pFrameView->SetGridFine( aCoarse );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_GridSnapWidth ) ) )
            {
                if( pValue->Value >>= nInt32 )
                {
                    const Size aCoarse( nInt32, pFrameView->GetSnapGrid().Height() );
                    pFrameView->SetSnapGrid( aCoarse );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_GridSnapHeight ) ) )
            {
                if( pValue->Value >>= nInt32 )
                {
                    const Size aCoarse( pFrameView->GetSnapGrid().Width(), nInt32 );
                    pFrameView->SetSnapGrid( aCoarse );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_IsAngleSnapEnabled ) ) )
            {
                if( pValue->Value >>= bBool )
                {
                    pFrameView->SetAngleSnapEnabled( bBool );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_SnapAngle ) ) )
            {
                if( pValue->Value >>= nInt32 )
                {
                    pFrameView->SetSnapAngle( nInt32 );
                }
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_GridSnapWidthXNumerator ) ) )
            {
                pValue->Value >>= aSnapGridWidthXNum;
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_GridSnapWidthXDenominator ) ) )
            {
                pValue->Value >>= aSnapGridWidthXDom;
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_GridSnapWidthYNumerator ) ) )
            {
                pValue->Value >>= aSnapGridWidthYNum;
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_GridSnapWidthYDenominator ) ) )
            {
                pValue->Value >>= aSnapGridWidthYDom;
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_VisibleLayers ) ) )
            {
                SetOfByte aSetOfBytes;
                aSetOfBytes.PutValue( pValue->Value );
                pFrameView->SetVisibleLayers( aSetOfBytes );
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_PrintableLayers ) ) )
            {
                SetOfByte aSetOfBytes;
                aSetOfBytes.PutValue( pValue->Value );
                pFrameView->SetPrintableLayers( aSetOfBytes );
            }
            else if (pValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_View_LockedLayers ) ) )
            {
                SetOfByte aSetOfBytes;
                aSetOfBytes.PutValue( pValue->Value );
                pFrameView->SetLockedLayers( aSetOfBytes );
            }
        }

        const Fraction aSnapGridWidthX( aSnapGridWidthXNum, aSnapGridWidthXDom );
        const Fraction aSnapGridWidthY( aSnapGridWidthYNum, aSnapGridWidthYDom );

        pFrameView->SetSnapGridWidth( aSnapGridWidthX, aSnapGridWidthY );
    }
}
#undef NUM_VIEW_SETTINGS
