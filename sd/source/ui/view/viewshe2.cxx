/*************************************************************************
 *
 *  $RCSfile: viewshe2.cxx,v $
 *
 *  $Revision: 1.27 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 07:46:51 $
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

#include "ViewShell.hxx"

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
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif
#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif
#include <sot/clsids.hxx>

#include "misc.hxx"
#include "strings.hrc"
#include "app.hrc"

#ifndef _SD_UNOKYWDS_HXX_
#include "unokywds.hxx"
#endif

#include "sdundogr.hxx"
#ifndef SD_FRAME_VIEW_HXX
#include "FrameView.hxx"
#endif
#include "undopage.hxx"
#include "sdresid.hxx"
#include "drawdoc.hxx"
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef SD_FU_POOR_HXX
#include "fupoor.hxx"
#endif
#ifndef SD_CLIENT_HXX
#include "Client.hxx"
#endif
#include "DrawDocShell.hxx"
#ifndef SD_FU_SEARCH_HXX
#include "fusearch.hxx"
#endif
#ifndef SD_FU_SLIDE_SHOW_HXX
#include "fuslshow.hxx"
#endif
#include "sdpage.hxx"
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif
#ifndef SD_SUB_SHELL_MANAGER_HXX
#include "SubShellManager.hxx"
#endif

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

namespace sd {

/*************************************************************************
|*
|* Scrollbar-Update: Thumbpos und VisibleSize anpassen
|*
\************************************************************************/

void ViewShell::UpdateScrollBars()
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

IMPL_LINK_INLINE_START(ViewShell, HScrollHdl, ScrollBar *, pHScroll )
{
    return VirtHScrollHdl(pHScroll);
}
IMPL_LINK_INLINE_END(ViewShell, HScrollHdl, ScrollBar *, pHScroll )

/*************************************************************************
|*
|* virtueller Scroll-Handler fuer horizontale Scrollbars
|*
\************************************************************************/

long ViewShell::VirtHScrollHdl(ScrollBar* pHScroll)
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
            ::sd::View* pView = GetView();
            OutlinerView* pOLV = NULL;

            if (pView)
                pOLV = pView->GetTextEditOutlinerView();

            if (pOLV)
                pOLV->HideCursor();

            pWinArray[nX][nY++]->SetVisibleXY(fX, -1);

            Rectangle aVisArea = GetDocSh()->GetVisArea(ASPECT_CONTENT);
            Point aVisAreaPos = pWindow->PixelToLogic( Point(0,0) );
            aVisArea.SetPos(aVisAreaPos);
            GetDocSh()->SetVisArea(aVisArea);

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

IMPL_LINK_INLINE_START(ViewShell, VScrollHdl, ScrollBar *, pVScroll )
{
    return VirtVScrollHdl(pVScroll);
}
IMPL_LINK_INLINE_END(ViewShell, VScrollHdl, ScrollBar *, pVScroll )

/*************************************************************************
|*
|* Handling fuer vertikale Scrollbars
|*
\************************************************************************/

long ViewShell::VirtVScrollHdl(ScrollBar* pVScroll)
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
            ::sd::View* pView = GetView();
            OutlinerView* pOLV = NULL;

            if (pView)
                pOLV = pView->GetTextEditOutlinerView();

            if (pOLV)
                pOLV->HideCursor();

            pWinArray[nX++][nY]->SetVisibleXY(-1, fY);

            Rectangle aVisArea = GetDocSh()->GetVisArea(ASPECT_CONTENT);
            Point aVisAreaPos = pWindow->PixelToLogic( Point(0,0) );
            aVisArea.SetPos(aVisAreaPos);
            GetDocSh()->SetVisArea(aVisArea);

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

void ViewShell::ScrollLines(long nLinesX, long nLinesY)
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

void ViewShell::Scroll(long nScrollX, long nScrollY)
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

    Rectangle aVisArea = GetDocSh()->GetVisArea(ASPECT_CONTENT);
    Point aVisAreaPos = pWindow->PixelToLogic( Point(0,0) );
    aVisArea.SetPos(aVisAreaPos);
    GetDocSh()->SetVisArea(aVisArea);

    Size aVisSizePixel = pWindow->GetOutputSizePixel();
    Rectangle aVisAreaWin = pWindow->PixelToLogic( Rectangle( Point(0,0), aVisSizePixel) );
    VisAreaChanged(aVisAreaWin);

    ::sd::View* pView = GetView();
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

void ViewShell::SetZoom(long nZoom)
{
    Fraction aUIScale(nZoom, 100);
    aUIScale *= GetDoc()->GetUIScale();

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

    ::sd::View* pView = GetView();
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

void ViewShell::SetZoomRect(const Rectangle& rZoomRect)
{
    short nX, nY, nCol, nRow;
    long nZoom = pWindow->SetZoomRect(rZoomRect);
    Fraction aUIScale(nZoom, 100);
    aUIScale *= GetDoc()->GetUIScale();

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

    ::sd::View* pView = GetView();
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

void ViewShell::InitWindows(const Point& rViewOrigin, const Size& rViewSize,
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

    ::sd::View* pView = GetView();
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

void ViewShell::InvalidateWindows()
{
    for (short nX = 0; nX < MAX_HSPLIT_CNT; nX++)
        for (short nY = 0; nY < MAX_VSPLIT_CNT; nY++)
            if ( pWinArray[nX][nY] )
                pWinArray[nX][nY]->Invalidate();
}


/*************************************************************************
|*
|* Auf allen Split-Windows ein Markierungsrechteck mit dem
|* uebergebenen Pen zeichnen
|*
\************************************************************************/

void ViewShell::DrawMarkRect(const Rectangle& rRect) const
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

void ViewShell::DrawFilledRect( const Rectangle& rRect, const Color& rLColor,
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

IMPL_LINK(ViewShell, ModeBtnHdl, Button *, pButton )
{
    if  ( !((ImageButton*) pButton)->IsChecked() )
    {
        SubShellManager& rSubShellManager =
            GetViewShellBase().GetSubShellManager();
        ViewShell::ShellType eType =
            rSubShellManager.GetMainSubShellType();
        USHORT nSlotId = 0;
        if ( pButton == &aDrawBtn )
        {
            eType = ViewShell::ST_IMPRESS;
            nSlotId = SID_DRAWINGMODE;
        }
        else if ( pButton == &aNotesBtn )
        {
            eType = ViewShell::ST_NOTES;
            nSlotId = SID_NOTESMODE;
            /*af
            pFrameView->SetPageKind(PK_NOTES);
            pFrameView->SetLayerMode(FALSE);
            */
        }
        else if ( pButton == &aHandoutBtn )
        {
            eType = ViewShell::ST_HANDOUT;
            nSlotId = SID_HANDOUTMODE;
            /*af
                    pFrameView->SetPageKind(PK_HANDOUT);
            pFrameView->SetLayerMode(FALSE);
            */
        }
        else if ( pButton == &aSlideBtn )
        {
            eType = ViewShell::ST_SLIDE;
            nSlotId = SID_DIAMODE;
        }
        else if ( pButton == &aOutlineBtn )
        {
            eType = ViewShell::ST_OUTLINE;
            nSlotId = SID_OUTLINEMODE;
        }
        else if ( pButton == &aPresentationBtn )
        {
            eType = ViewShell::ST_PRESENTATION;
            nSlotId = SID_PRESENTATION;
        }
        if (nSlotId != 0)
            GetDispatcher()->Execute(
                nSlotId,
                SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
        //        rSubShellManager.SetMainSubShellType (eType);
    }
    return 0;
}

/*************************************************************************
|*
|* Groesse und Raender aller Seiten setzen
|*
\************************************************************************/

void ViewShell::SetPageSizeAndBorder(PageKind ePageKind, const Size& rNewSize,
                                       long nLeft, long nRight,
                                       long nUpper, long nLower, BOOL bScaleAll,
                                       Orientation eOrientation, USHORT nPaperBin,
                                       BOOL bBackgroundFullSize)
{
    SdPage* pPage;
    SdUndoGroup* pUndoGroup = NULL;
    pUndoGroup = new SdUndoGroup(GetDoc());
    String aString(SdResId(STR_UNDO_CHANGE_PAGEFORMAT));
    pUndoGroup->SetComment(aString);
    SfxViewShell* pViewShell = GetViewShell();
    OSL_ASSERT (pViewShell!=NULL);

    USHORT i, nPageCnt = GetDoc()->GetMasterSdPageCount(ePageKind);

    for (i = 0; i < nPageCnt; i++)
    {
        /**********************************************************************
        * Erst alle MasterPages bearbeiten
        **********************************************************************/
        pPage = GetDoc()->GetMasterSdPage(i, ePageKind);

        SdUndoAction* pUndo = new SdPageFormatUndoAction(GetDoc(), pPage,
                            pPage->GetSize(),
                            pPage->GetLftBorder(), pPage->GetRgtBorder(),
                            pPage->GetUppBorder(), pPage->GetLwrBorder(),
                            pPage->IsScaleObjects(),
                            pPage->GetOrientation(),
                            pPage->GetPaperBin(),
                            pPage->IsBackgroundFullSize(),
                            rNewSize,
                            nLeft, nRight,
                            nUpper, nLower,
                            bScaleAll,
                            eOrientation,
                            nPaperBin,
                            bBackgroundFullSize);
        pUndoGroup->AddAction(pUndo);

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
        pPage->SetPaperBin( nPaperBin );
        pPage->SetBackgroundFullSize( bBackgroundFullSize );

        if ( ePageKind == PK_STANDARD )
            GetDoc()->GetMasterSdPage(i, PK_NOTES)->CreateTitleAndLayout();

        pPage->CreateTitleAndLayout();
    }

    nPageCnt = GetDoc()->GetSdPageCount(ePageKind);

    for (i = 0; i < nPageCnt; i++)
    {
        /**********************************************************************
        * Danach alle Pages bearbeiten
        **********************************************************************/
        pPage = GetDoc()->GetSdPage(i, ePageKind);

        SdUndoAction* pUndo = new SdPageFormatUndoAction(GetDoc(), pPage,
                                pPage->GetSize(),
                                pPage->GetLftBorder(), pPage->GetRgtBorder(),
                                pPage->GetUppBorder(), pPage->GetLwrBorder(),
                                pPage->IsScaleObjects(),
                                pPage->GetOrientation(),
                                pPage->GetPaperBin(),
                                pPage->IsBackgroundFullSize(),
                                rNewSize,
                                nLeft, nRight,
                                nUpper, nLower,
                                bScaleAll,
                                eOrientation,
                                nPaperBin,
                                bBackgroundFullSize);
        pUndoGroup->AddAction(pUndo);

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
        pPage->SetPaperBin( nPaperBin );
        pPage->SetBackgroundFullSize( bBackgroundFullSize );

        if ( ePageKind == PK_STANDARD )
        {
            SdPage* pNotesPage = GetDoc()->GetSdPage(i, PK_NOTES);
            pNotesPage->SetAutoLayout( pNotesPage->GetAutoLayout() );
        }

        pPage->SetAutoLayout( pPage->GetAutoLayout() );
    }

    // Handoutseite an neues Format der Standardseiten anpassen
    if ( ePageKind == PK_STANDARD )
        GetDoc()->GetSdPage(0, PK_HANDOUT)->CreateTitleAndLayout(TRUE);

    // Undo Gruppe dem Undo Manager uebergeben
    pViewShell->GetViewFrame()->GetObjectShell()
        ->GetUndoManager()->AddUndoAction(pUndoGroup);

    long nWidth = pPage->GetSize().Width();
    long nHeight = pPage->GetSize().Height();

    Point aPageOrg = Point(nWidth, nHeight / 2);
    Size aViewSize = Size(nWidth * 3, nHeight * 2);

    InitWindows(aPageOrg, aViewSize, Point(-1, -1), TRUE);

    Point aVisAreaPos;

    if ( GetDocSh()->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
    {
        aVisAreaPos = GetDocSh()->GetVisArea(ASPECT_CONTENT).TopLeft();
    }

    ::sd::View* pView = GetView();
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

    pViewShell->GetViewFrame()->GetBindings().Invalidate(SID_RULER_NULL_OFFSET);

    // auf (neue) Seitengroesse zoomen
    pViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_SIZE_PAGE,
            SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
}

/*************************************************************************
|*
|* Zoom-Faktor fuer InPlace einstellen
|*
\************************************************************************/

void ViewShell::SetZoomFactor(const Fraction& rZoomX, const Fraction& rZoomY)
{
    long nZoom = (long)((double) rZoomX * 100);
    SetZoom(nZoom);
}


/*************************************************************************
|*
|* Aktives Fenster setzen
|*
\************************************************************************/

void ViewShell::SetActiveWindow (::sd::Window* pWin)
{
    SfxViewShell* pViewShell = GetViewShell();
    OSL_ASSERT (pViewShell!=NULL);

    if (pViewShell->GetWindow() != pWin)
    {
        if (pWin)
        {
            pWin->EnableChildTransparentMode();
        }

        pViewShell->SetWindow(pWin);
    }

    if (pWindow != pWin)
        pWindow = pWin;

    // The rest of this function is not guarded anymore against calling this
    // method with an already active window because the functions may still
    // point to the old window when the new one has already been assigned to
    // pWindow elsewhere.
    ::sd::View* pView = GetView();
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


/*************************************************************************
|*
|* RequestHelp event
|*
\************************************************************************/

BOOL ViewShell::RequestHelp(const HelpEvent& rHEvt, ::sd::Window* pWin)
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




FrameView* ViewShell::GetFrameView (void)
{
    return pFrameView;
}




/*************************************************************************
|*
|* Read FrameViews data and set actual views data
|*
\************************************************************************/

void ViewShell::ReadFrameViewData(FrameView* pView)
{
}



/*************************************************************************
|*
|* Write actual views data to FrameView
|*
\************************************************************************/

void ViewShell::WriteFrameViewData()
{
}

/*************************************************************************
|*
|* Auf allen Split-Windows ein Update erzwingen.
|*
\************************************************************************/

void ViewShell::UpdateWindows()
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

BOOL ViewShell::ActivateObject(SdrOle2Obj* pObj, long nVerb)
{
    ErrCode aErrCode = 0;
    SfxErrorContext aEC(ERRCTX_SO_DOVERB, pWindow, RID_SO_ERRCTX);
    BOOL bAbort = FALSE;
    BOOL bChartActive = FALSE;
    GetDocSh()->SetWaitCursor( TRUE );
    SfxViewShell* pViewShell = GetViewShell();
    OSL_ASSERT (pViewShell!=NULL);

    SvInPlaceObjectRef aIPObj = pObj->GetObjRef();

    if ( !aIPObj.Is() )
    {
        /**********************************************************
        * Leeres OLE-Objekt mit OLE-Objekt versehen
        **********************************************************/
        SvInPlaceObjectRef aNewIPObj;
        String aName = pObj->GetProgName();

        if( aName.EqualsAscii( "StarChart" ))
        {
            if( SvtModuleOptions().IsChart() )
                aNewIPObj = SvInPlaceObject::CreateObject( SvGlobalName( SO3_SCH_CLASSID ) );
        }
        else if( aName.EqualsAscii( "StarOrg" ))
        {
            if( SvtModuleOptions().IsChart() )
                aNewIPObj = SvInPlaceObject::CreateObject( SvGlobalName( SO3_SCH_CLASSID ) );
        }
        else if( aName.EqualsAscii( "StarCalc" ))
        {
            if( SvtModuleOptions().IsCalc() )
                aNewIPObj = SvInPlaceObject::CreateObject( SvGlobalName( SO3_SC_CLASSID ) );
        }
        else if( aName.EqualsAscii( "StarMath" ))
        {
            if( SvtModuleOptions().IsMath() )
                aNewIPObj = SvInPlaceObject::CreateObject( SvGlobalName( SO3_SM_CLASSID ) );
        }

        if( !aNewIPObj.Is() )
        {
            aName = String();

            // Dialog "OLE-Objekt einfuegen" aufrufen
            GetDocSh()->SetWaitCursor( FALSE );
            pViewShell->GetViewFrame()->GetDispatcher()->Execute(
                SID_INSERT_OBJECT,
                SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD);
            aNewIPObj = pObj->GetObjRef();
            GetDocSh()->SetWaitCursor( TRUE );

            if (!aNewIPObj.Is())
            {
                bAbort = TRUE;
            }
        }

        if( aNewIPObj.Is() )
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
                String aObjName = GetDocSh()->InsertObject(aNewIPObj, String())->GetObjName();
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

    if( aErrCode == 0 )
    {
        ::sd::View* pView = GetView();

        if (pView->IsTextEdit())
        {
            pView->EndTextEdit();
        }

        const SvInPlaceObjectRef& rIPObjRef = pObj->GetObjRef();
        SfxInPlaceClientRef pSdClient =
            static_cast<Client*>(pViewShell->FindIPClient(
                rIPObjRef, pWindow));

        if ( !pSdClient.Is() )
        {
            pSdClient = new Client(pObj, this, pWindow);
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
                                                   GetDoc()->GetScaleUnit() );

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
        SfxViewShell* pViewShell = GetViewShell();
        OSL_ASSERT (pViewShell!=NULL);
        pViewShell->DoVerb(pSdClient, nVerb);   // ErrCode wird ggf. vom Sfx ausgegeben
        pViewShell->GetViewFrame()->GetBindings().Invalidate(
            SID_NAVIGATOR_STATE, TRUE, FALSE);
    }

    GetDocSh()->SetWaitCursor( FALSE );

    if (aErrCode != 0 && !bAbort)
    {
        ErrorHandler::HandleError(* new StringErrorInfo(aErrCode, String() ) );
    }

    return aErrCode == 0;
}

/*************************************************************************
|*
|* umschliessendes Rechteck aller (Split-)Fenster zurueckgeben.
|*
\************************************************************************/

const Rectangle& ViewShell::GetAllWindowRect()
{
    aAllWindowRect.SetPos(pWinArray[0][0]->OutputToScreenPixel(Point(0,0)));
    return aAllWindowRect;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void ViewShell::CancelSearching()
{
    delete pFuSearch;
    pFuSearch = NULL;
}

/*************************************************************************
|*
|* Read user data
|*
\************************************************************************/
    GetDispatcher()->Execute(
        SID_SIZE_VISAREA,
        SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
    GetDispatcher()->Execute(
    SID_SIZE_VISAREA,
    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
>>>>>>> 1.25.72.1
}

/*************************************************************************
|*
|* Write user data
|*
\************************************************************************/

void ViewShell::WriteUserData(String& rString)
{
    // Das Schreiben unserer Daten erfolgt stets in WriteFrameViewData()
    WriteFrameViewData();
}


/*************************************************************************
|*
|* Lineale ein- / ausschalten
|*
\************************************************************************/

void ViewShell::SetRuler(BOOL bRuler)
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

    OSL_ASSERT(GetViewShell()!=NULL);
    GetViewShell()->InvalidateBorder();
}

/*************************************************************************
|*
|* AcceptDrop
|*
\************************************************************************/

sal_Int8 ViewShell::AcceptDrop (
    const AcceptDropEvent& rEvt,
    DropTargetHelper& rTargetHelper,
    ::sd::Window* pTargetWindow,
    USHORT nPage,
    USHORT nLayer)
{
    ::sd::View* pView = GetView();
    return( pView ? pView->AcceptDrop( rEvt, rTargetHelper, pTargetWindow, nPage, nLayer ) : DND_ACTION_NONE );
}

/*************************************************************************
|*
|* ExecuteDrop
|*
\************************************************************************/

sal_Int8 ViewShell::ExecuteDrop (
    const ExecuteDropEvent& rEvt,
    DropTargetHelper& rTargetHelper,
    ::sd::Window* pTargetWindow,
    USHORT nPage,
    USHORT nLayer)
{
    ::sd::View* pView = GetView();
    return( pView ? pView->ExecuteDrop( rEvt, rTargetHelper, pTargetWindow, nPage, nLayer ) : DND_ACTION_NONE );
}

#ifdef WNT
#pragma optimize ( "", on )
#endif

void ViewShell::WriteUserDataSequence ( ::com::sun::star::uno::Sequence <
    ::com::sun::star::beans::PropertyValue >& rSequence, sal_Bool bBrowse)
{
    const sal_Int32 nIndex = rSequence.getLength();
    rSequence.realloc( nIndex + 1 );

    OSL_ASSERT (GetViewShell()!=NULL);
    sal_uInt16 nViewID(GetViewShell()->GetViewFrame()->GetCurViewId());
    rSequence[nIndex].Name = rtl::OUString (
        RTL_CONSTASCII_USTRINGPARAM( sUNO_View_ViewId ) );
    rtl::OUStringBuffer sBuffer (
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "view" ) ) );
    sBuffer.append( static_cast<sal_Int32>(nViewID));
    rSequence[nIndex].Value <<= sBuffer.makeStringAndClear();

    pFrameView->WriteUserDataSequence( rSequence, bBrowse );
}


void ViewShell::ReadUserDataSequence ( const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >& rSequence, sal_Bool bBrowse )
{
    pFrameView->ReadUserDataSequence( rSequence, bBrowse );
}

void ViewShell::VisAreaChanged(const Rectangle& rRect)
{
    OSL_ASSERT (GetViewShell()!=NULL);
    GetViewShell()->VisAreaChanged(rRect);
}

void ViewShell::SetWinViewPos(const Point& rWinPos, bool bUpdate)
{
    for (short nX = 0; nX < MAX_HSPLIT_CNT; nX++)
    {
        for (short nY = 0; nY < MAX_VSPLIT_CNT; nY++)
        {
            if ( pWinArray[nX][nY] )
            {
                pWinArray[nX][nY]->SetWinViewPos(rWinPos);

                if ( bUpdate )
                {
                    pWinArray[nX][nY]->UpdateMapOrigin();
                    pWinArray[nX][nY]->Invalidate();
                }
            }
        }
    }

    if ( bHasRuler )
    {
        UpdateHRuler();
        UpdateVRuler();
    }

    UpdateScrollBars();

    Size aVisSizePixel = pWindow->GetOutputSizePixel();
    Rectangle aVisAreaWin = pWindow->PixelToLogic( Rectangle( Point(0,0), aVisSizePixel) );
    VisAreaChanged(aVisAreaWin);

    ::sd::View* pView = GetView();
    if (pView)
    {
        pView->VisAreaChanged(pWindow);
    }
}

Point ViewShell::GetWinViewPos() const
{
    return pWinArray[0][0]->GetWinViewPos();
}

Point ViewShell::GetViewOrigin() const
{
    return pWinArray[0][0]->GetViewOrigin();
}

} // end of namespace sd
