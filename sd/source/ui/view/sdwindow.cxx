/*************************************************************************
 *
 *  $RCSfile: sdwindow.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 17:06:46 $
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

#ifndef _B3D_BASE3D_HXX
#include "goodies/base3d.hxx"
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXREQUEST_HXX
#include <sfx2/request.hxx>
#endif

#pragma hdrstop

#include "app.hrc"
#include "helpids.h"
#include "sdwindow.hxx"
#include "viewshel.hxx"
#include "drviewsh.hxx"
#include "sdview.hxx"
#include "frmview.hxx"
#include "outlnvsh.hxx"
#include "drawdoc.hxx"
#ifndef _SD_ACCESSIBILITY_ACCESSIBLE_DRAW_DOCUMENT_VIEW_HXX
#include "AccessibleDrawDocumentView.hxx"
#endif



#define SCROLL_LINE_FACT   0.05     // Faktor fuer Zeilenscrolling
#define SCROLL_PAGE_FACT   0.5      // Faktor fuer Seitenscrolling
#define SCROLL_SENSITIVE   20       // Sensitiver Bereich (Pixel)
#define ZOOM_MULTIPLICATOR 10000    // Multiplikator um Rundungsfehler zu vermeiden
#define MIN_ZOOM           5        // Minimaler Zoomfaktor
#define MAX_ZOOM           3000     // Maximaler Zoomfaktor


/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

SdWindow::SdWindow(Window* pParent) :
    Window(pParent, WinBits(WB_CLIPCHILDREN | WB_DIALOGCONTROL)),
    DropTargetHelper( this ),
    pShareWin(NULL),
    pViewShell(NULL),
    aWinPos(0, 0),          // vorsichtshalber; die Werte sollten aber
    aViewOrigin(0, 0),      // vom Besitzer des Fensters neu gesetzt
    aViewSize(1000, 1000),  // werden
    nMinZoom(MIN_ZOOM),
    nMaxZoom(MAX_ZOOM),
    bMinZoomAutoCalc(FALSE),
    bCalcMinZoomByMinSide(TRUE),
    bCenterAllowed(TRUE),
    nTicks (0)
{
    SetDialogControlFlags( WINDOW_DLGCTRL_RETURN | WINDOW_DLGCTRL_WANTFOCUS );

    MapMode aMap(GetMapMode());
    aMap.SetMapUnit(MAP_100TH_MM);
    SetMapMode(aMap);

    // Damit im Diamodus die WindowColor genommen wird
    SetBackground( Wallpaper( GetSettings().GetStyleSettings().GetWindowColor() ) );

    // adjust contrast mode initially
    bool bUseContrast = GetSettings().GetStyleSettings().GetHighContrastMode();
    SetDrawMode( bUseContrast ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR );

    // Hilfe-ID setzen
    // SetHelpId(HID_SD_WIN_DOCUMENT);
    SetUniqueId(HID_SD_WIN_DOCUMENT);
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

SdWindow::~SdWindow()
{
}

/*************************************************************************
|*
|* Die Haelfte des Sichtbaren Bereich eines anderen Fensters darstellen
|*
\************************************************************************/

void SdWindow::ShareViewArea(SdWindow* pOtherWin)
{
    pShareWin       = pOtherWin;
    aViewOrigin     = pOtherWin->aViewOrigin;
    aViewSize       = pOtherWin->aViewSize;
    nMinZoom        = pOtherWin->nMinZoom;
    nMaxZoom        = pOtherWin->nMaxZoom;
    bCenterAllowed  = pOtherWin->bCenterAllowed;

    long nZoom = pOtherWin->GetZoom();
    MapMode aMap(GetMapMode());
    aMap.SetScaleX(Fraction(nZoom, 100));
    aMap.SetScaleY(Fraction(nZoom, 100));
    aMap.SetOrigin(pOtherWin->GetMapMode().GetOrigin());
    SetMapMode(aMap);
}




void SdWindow::CalcMinZoom()
{
    // Are we entitled to change the minimal zoom factor?
    if ( bMinZoomAutoCalc )
    {
        // Get current zoom factor.
        long nZoom = GetZoom();

        if ( pShareWin )
        {
            pShareWin->CalcMinZoom();
            nMinZoom = pShareWin->nMinZoom;
        }
        else
        {
            // Get the rectangle of the output area in logical coordinates
            // and calculate the scaling factors that would lead to the view
            // area (also called application area) to completely fill the
            // window.
            Size aWinSize = PixelToLogic(GetOutputSizePixel());
            ULONG nX = (ULONG) ((double) aWinSize.Width()
                * (double) ZOOM_MULTIPLICATOR / (double) aViewSize.Width());
            ULONG nY = (ULONG) ((double) aWinSize.Height()
                * (double) ZOOM_MULTIPLICATOR / (double) aViewSize.Height());

            // Decide whether to take the larger or the smaller factor.
            ULONG nFact;
            if ( bCalcMinZoomByMinSide )    nFact = Min(nX, nY);
            else                            nFact = Max(nX, nY);

            // The factor is tansfomed according to the current zoom factor.
            nFact = nFact * nZoom / ZOOM_MULTIPLICATOR;
            nMinZoom = Max((USHORT) MIN_ZOOM, (USHORT) nFact);
        }
        // If the current zoom factor is smaller than the calculated minimal
        // zoom factor then set the new minimal factor as the current zoom
        // factor.
        if ( nZoom < (long) nMinZoom )
            SetZoomFactor(nMinZoom);
    }
}

/*************************************************************************
|*
|* Resize event
|*
\************************************************************************/

void SdWindow::Resize()
{
    Window::Resize();
    CalcMinZoom();
}

/*************************************************************************
|*
|* Paint event
|*
\************************************************************************/

void SdWindow::Paint(const Rectangle& rRect)
{
    if ( pViewShell )
        pViewShell->Paint(rRect, this);
}

/*************************************************************************
|*
|* Keyboard event
|*
\************************************************************************/

void SdWindow::KeyInput(const KeyEvent& rKEvt)
{
    BOOL aReturn = FALSE;

    if (!(pViewShell && pViewShell->KeyInput(rKEvt, this)))
    {
        if (pViewShell && rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE)
        {
            // Wenn IP aktiv, wird der IP-Modus abgebrochen
            pViewShell->GetDocSh()->DoInPlaceActivate(FALSE);
        }
        else
        {
            Window::KeyInput(rKEvt);
        }
    }
}

/*************************************************************************
|*
|* MouseButtonDown event
|*
\************************************************************************/

void SdWindow::MouseButtonDown(const MouseEvent& rMEvt)
{
    if ( pViewShell )
        pViewShell->MouseButtonDown(rMEvt, this);
}

/*************************************************************************
|*
|* MouseMove event
|*
\************************************************************************/

void SdWindow::MouseMove(const MouseEvent& rMEvt)
{
    if ( pViewShell )
        pViewShell->MouseMove(rMEvt, this);
}

/*************************************************************************
|*
|* MouseButtonUp event
|*
\************************************************************************/

void SdWindow::MouseButtonUp(const MouseEvent& rMEvt)
{
    nTicks = 0;

    if ( pViewShell )
        pViewShell->MouseButtonUp(rMEvt, this);
}

/*************************************************************************
|*
|* Command event
|*
\************************************************************************/

void SdWindow::Command(const CommandEvent& rCEvt)
{
    if ( pViewShell )
        pViewShell->Command(rCEvt, this);
}

/*************************************************************************
|*
|* RequestHelp event
|*
\************************************************************************/

void SdWindow::RequestHelp(const HelpEvent& rEvt)
{
    if ( pViewShell )
    {
        if( !pViewShell->RequestHelp( rEvt, this) )
            Window::RequestHelp( rEvt );
    }
    else
        Window::RequestHelp( rEvt );
}

/*************************************************************************
|*
|* Position der linken oberen Ecke des im Fenster sichtbaren Bereichs
|* setzen
|*
\************************************************************************/

void SdWindow::SetWinViewPos(const Point& rPnt)
{
    aWinPos = rPnt;
}

/*************************************************************************
|*
|* Ursprung der Darstellung in Bezug zur gesamten Arbeitsflaeche setzen
|*
\************************************************************************/

void SdWindow::SetViewOrigin(const Point& rPnt)
{
    aViewOrigin = rPnt;
}

/*************************************************************************
|*
|* Groesse der gesamten Arbeitsflaeche, die mit dem Fenster betrachtet
|* werden kann, setzen
|*
\************************************************************************/

void SdWindow::SetViewSize(const Size& rSize)
{
    aViewSize = rSize;
    CalcMinZoom();
}




long SdWindow::SetZoomFactor(long nZoom)
{
    // Clip the zoom factor to the valid range marked by nMinZoom as
    // calculated by CalcMinZoom() and the constant MAX_ZOOM.
    if ( nZoom > MAX_ZOOM )
        nZoom = MAX_ZOOM;
    if ( nZoom < (long) nMinZoom )
        nZoom = nMinZoom;

    // Set the zoom factor at the window's map mode.
    MapMode aMap(GetMapMode());
    aMap.SetScaleX(Fraction(nZoom, 100));
    aMap.SetScaleY(Fraction(nZoom, 100));
    SetMapMode(aMap);

    // Update the map mode's origin (to what effect?).
    UpdateMapOrigin();

    // Update the view's snapping to the the new zoom factor.
    if ( pViewShell && pViewShell->ISA(SdDrawViewShell) )
        ((SdDrawViewShell*) pViewShell)->GetView()->
                                        RecalcLogicSnapMagnetic(*this);

    // Return the zoom factor just in case it has been changed above to lie
    // inside the valid range.
    return nZoom;
}




void SdWindow::SetZoom(long nZoom)
{
    // Clip the zoom factor to the valid range marked by nMinZoom as
    // previously calculated by <member>CalcMinZoom()</member> and the
    // MAX_ZOOM constant.
    if ( nZoom > MAX_ZOOM )
        nZoom = MAX_ZOOM;
    if ( nZoom < (long) nMinZoom )
        nZoom = nMinZoom;

    // Calculate the window's new origin.
    Size aSize = PixelToLogic(GetOutputSizePixel());
    long nW = aSize.Width()  * GetZoom() / nZoom;
    long nH = aSize.Height() * GetZoom() / nZoom;
    aWinPos.X() += (aSize.Width()  - nW) / 2;
    aWinPos.Y() += (aSize.Height() - nH) / 2;
    if ( aWinPos.X() < 0 ) aWinPos.X() = 0;
    if ( aWinPos.Y() < 0 ) aWinPos.Y() = 0;

    // Finally update this window's map mode to the given zoom factor that
    // has been clipped to the valid range.
    SetZoomFactor(nZoom);
}




/** Recalculate the zoom factor and translation so that the given rectangle
    is displayed centered and as large as possible while still being fully
    visible in the window.
*/
long SdWindow::SetZoomRect(const Rectangle& rZoomRect)
{
    long nNewZoom = 100;

    if (rZoomRect.GetWidth() == 0 || rZoomRect.GetHeight() == 0)
    {
        // The given rectangle is degenerate.  Use the default zoom factor
        // (above) of 100%.
        SetZoom(nNewZoom);
    }
    else
    {
        Point aPos = rZoomRect.TopLeft();
        // Transform the output area from pixel coordinates into logical
        // coordinates.
        Size aWinSize = PixelToLogic(GetOutputSizePixel());
        // Paranoia!  The degenerate case of zero width or height has been
        // taken care of above.
        DBG_ASSERT(rZoomRect.GetWidth(), "ZoomRect-Breite = 0!");
        DBG_ASSERT(rZoomRect.GetHeight(), "ZoomRect-Hoehe = 0!");

        // Calculate the scale factors which will lead to the given
        // rectangle being fully visible (when translated accordingly) as
        // large as possible in the output area independently in both
        // coordinate directions .
        ULONG nX = (ULONG) ((double) aWinSize.Width()
            * (double) ZOOM_MULTIPLICATOR / (double) rZoomRect.GetWidth());
        ULONG nY = (ULONG) ((double) aWinSize.Height()
            * (double) ZOOM_MULTIPLICATOR / (double) rZoomRect.GetHeight());
        // Use the smaller one of both so that the zoom rectangle will be
        // fully visible with respect to both coordinate directions.
        ULONG nFact = Min(nX, nY);

        // Transform the current zoom factor so that it leads to the desired
        // scaling.
        long nZoom = nFact * GetZoom() / ZOOM_MULTIPLICATOR;

        // Calculate the new origin.
        if ( nFact == 0 )
        {
            // Don't change anything if the scale factor is degenrate.
            nNewZoom = GetZoom();
        }
        else
        {
            // Calculate the new window position that centers the given
            // rectangle on the screen.
            if ( nZoom > MAX_ZOOM )
                nFact = nFact * MAX_ZOOM / nZoom;

            aWinPos = aViewOrigin + aPos;

            aWinSize.Width() = (long) ((double) aWinSize.Width() * (double) ZOOM_MULTIPLICATOR / (double) nFact);
            aWinPos.X() += (rZoomRect.GetWidth() - aWinSize.Width()) / 2;
            aWinSize.Height() = (long) ((double) aWinSize.Height() * (double) ZOOM_MULTIPLICATOR / (double) nFact);
            aWinPos.Y() += (rZoomRect.GetHeight() - aWinSize.Height()) / 2;

            if ( aWinPos.X() < 0 )  aWinPos.X() = 0;
            if ( aWinPos.Y() < 0 )  aWinPos.Y() = 0;

            // Adapt the window's map mode to the new zoom factor.
            nNewZoom = SetZoomFactor(nZoom);
        }
    }

    return(nNewZoom);
}

/*************************************************************************
|*
|* Neuen MapMode-Origin berechnen und setzen; wenn aWinPos.X()/Y()
|* gleich -1 ist, wird die entsprechende Position zentriert
|* (z.B. fuer Initialisierung)
|*
\************************************************************************/

void SdWindow::UpdateMapOrigin(BOOL bInvalidate)
{
    MapMode aMap(GetMapMode());
    Point   aNewOrigin;
    BOOL    bChanged = FALSE;
    Size    aWinSize = PixelToLogic(GetOutputSizePixel());

    if ( bCenterAllowed )
    {
        if ( aWinPos.X() > aViewSize.Width() - aWinSize.Width() )
        {
            aWinPos.X() = aViewSize.Width() - aWinSize.Width();
            bChanged = TRUE;
        }
        if ( aWinPos.Y() > aViewSize.Height() - aWinSize.Height() )
        {
            aWinPos.Y() = aViewSize.Height() - aWinSize.Height();
            bChanged = TRUE;
        }
        if ( aWinSize.Width() > aViewSize.Width() || aWinPos.X() < 0 )
        {
            aWinPos.X() = aViewSize.Width()  / 2 - aWinSize.Width()  / 2;
            bChanged = TRUE;
        }
        if ( aWinSize.Height() > aViewSize.Height() || aWinPos.Y() < 0 )
        {
            aWinPos.Y() = aViewSize.Height() / 2 - aWinSize.Height() / 2;
            bChanged = TRUE;
        }
        aWinPos -= aViewOrigin;
        Size aPix(aWinPos.X(), aWinPos.Y());
        aPix = LogicToPixel(aPix);
        // Groesse muss vielfaches von BRUSH_SIZE sein, damit Muster
        // richtig dargestellt werden
        // #i2237#
        // removed old stuff here which still forced zoom to be
        // %BRUSH_SIZE which is outdated now

        if (pViewShell && pViewShell->ISA(SdDrawViewShell))
        {
            Size aViewSizePixel = LogicToPixel(aViewSize);
            Size aWinSizePixel = LogicToPixel(aWinSize);

            // Seite soll nicht am Fensterrand "kleben"
            if (aPix.Width() == 0)
            {
                // #i2237#
                // Since BRUSH_SIZE alignment is outdated now, i use the
                // former constant here directly
                aPix.Width() -= 8;
            }
            if (aPix.Height() == 0)
            {
                // #i2237#
                // Since BRUSH_SIZE alignment is outdated now, i use the
                // former constant here directly
                aPix.Height() -= 8;
            }
        }

        aPix = PixelToLogic(aPix);
        aWinPos.X() = aPix.Width();
        aWinPos.Y() = aPix.Height();
        aNewOrigin.X() = - aWinPos.X();
        aNewOrigin.Y() = - aWinPos.Y();
        aWinPos += aViewOrigin;

        aMap.SetOrigin(aNewOrigin);
        SetMapMode(aMap);

        if ( bChanged && bInvalidate )
            Invalidate();
    }
}

/*************************************************************************
|*
|* X-Position des sichtbaren Bereichs als Bruchteil (< 1)
|* der gesamten Arbeitsbereichbreite zuruegeben
|*
\************************************************************************/

double SdWindow::GetVisibleX()
{
    return ((double) aWinPos.X() / aViewSize.Width());
}

/*************************************************************************
|*
|* Y-Position des sichtbaren Bereichs als Bruchteil (< 1)
|* der gesamten Arbeitsbereichhoehe zuruegeben
|*
\************************************************************************/

double SdWindow::GetVisibleY()
{
    return ((double) aWinPos.Y() / aViewSize.Height());
}

/*************************************************************************
|*
|* X- und Y-Position des sichtbaren Bereichs als Bruchteile (< 1)
|* der gesamten Arbeitsbereichgroesse setzen
|* negative Werte werden ignoriert
|*
\************************************************************************/

void SdWindow::SetVisibleXY(double fX, double fY)
{
    long nOldX = aWinPos.X();
    long nOldY = aWinPos.Y();

    if ( fX >= 0 )
        aWinPos.X() = (long) (fX * aViewSize.Width());
    if ( fY >= 0 )
        aWinPos.Y() = (long) (fY * aViewSize.Height());
    UpdateMapOrigin(FALSE);
    Size sz(nOldX - aWinPos.X(), nOldY - aWinPos.Y());
    sz = LogicToPixel(sz);
    Scroll(nOldX - aWinPos.X(), nOldY - aWinPos.Y(), SCROLL_CHILDREN);
    Update();
}

/*************************************************************************
|*
|* Breite des sichtbaren Bereichs im Verhaeltnis zur
|* gesamten Arbeitsbereichbreite zuruegeben
|*
\************************************************************************/

double SdWindow::GetVisibleWidth()
{
    Size aWinSize = PixelToLogic(GetOutputSizePixel());
    if ( aWinSize.Width() > aViewSize.Width() )
        aWinSize.Width() = aViewSize.Width();
    return ((double) aWinSize.Width() / aViewSize.Width());
}

/*************************************************************************
|*
|* Hoehe des sichtbaren Bereichs im Verhaeltnis zur
|* gesamten Arbeitsbereichhoehe zuruegeben
|*
\************************************************************************/

double SdWindow::GetVisibleHeight()
{
    Size aWinSize = PixelToLogic(GetOutputSizePixel());
    if ( aWinSize.Height() > aViewSize.Height() )
        aWinSize.Height() = aViewSize.Height();
    return ((double) aWinSize.Height() / aViewSize.Height());
}

/*************************************************************************
|*
|* Breite einer Scrollspalte im Verhaeltnis zur gesamten
|* Arbeitsbereichbreite zuruegeben
|*
\************************************************************************/

double SdWindow::GetScrlLineWidth()
{
    return (GetVisibleWidth() * SCROLL_LINE_FACT);
}

/*************************************************************************
|*
|* Breite einer Scrollspalte im Verhaeltnis zur gesamten
|* Arbeitsbereichhoehe zuruegeben
|*
\************************************************************************/

double SdWindow::GetScrlLineHeight()
{
    return (GetVisibleHeight() * SCROLL_LINE_FACT);
}

/*************************************************************************
|*
|* Breite einer Scrollpage im Verhaeltnis zur gesamten
|* Arbeitsbereichbreite zuruegeben
|*
\************************************************************************/

double SdWindow::GetScrlPageWidth()
{
    return (GetVisibleWidth() * SCROLL_PAGE_FACT);
}

/*************************************************************************
|*
|* Breite einer Scrollpage im Verhaeltnis zur gesamten
|* Arbeitsbereichhoehe zuruegeben
|*
\************************************************************************/

double SdWindow::GetScrlPageHeight()
{
    return (GetVisibleHeight() * SCROLL_PAGE_FACT);
}

/*************************************************************************
|*
|* Scrolling bei AcceptDrop-Events
|*
\************************************************************************/

void SdWindow::DropScroll(const Point& rMousePos)
{
    BOOL bReturn = FALSE;

    short nDx = 0;
    short nDy = 0;

    Size aSize = GetOutputSizePixel();

    if (aSize.Width() > SCROLL_SENSITIVE * 3)
    {
        if ( rMousePos.X() < SCROLL_SENSITIVE )
        {
            nDx = -1;
        }

        if ( rMousePos.X() >= aSize.Width() - SCROLL_SENSITIVE )
        {
            nDx = 1;
        }
    }

    if (aSize.Height() > SCROLL_SENSITIVE * 3)
    {
        if ( rMousePos.Y() < SCROLL_SENSITIVE )
        {
            nDy = -1;
        }

        if ( rMousePos.Y() >= aSize.Height() - SCROLL_SENSITIVE )
        {
            nDy = 1;
        }
    }

    if ( (nDx || nDy) && (rMousePos.X()!=0 || rMousePos.Y()!=0 ) )
    {
                if (nTicks > 20) pViewShell->ScrollLines(nDx, nDy);
        else nTicks ++;
    }
}

/*************************************************************************
|*
|* Fenster deaktivieren
|*
\************************************************************************/

void SdWindow::LoseFocus()
{
    nTicks = 0;
        Window::LoseFocus ();
}

/*************************************************************************
|*
|* Fenster aktivieren
|*
\************************************************************************/

void SdWindow::GrabFocus()
{
    nTicks       = 0;
        Window::GrabFocus ();
}


/*************************************************************************
|*
|* DataChanged
|*
\************************************************************************/

void SdWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    // PRINTER bei allen Dokumenten weglassen, die keinen Printer benutzen.
    // FONTS und FONTSUBSTITUTION weglassen, wenn keine Textausgaben
    // vorhanden sind, bzw. wenn das Dokument keinen Text zulaesst.

    if ( (rDCEvt.GetType() == DATACHANGED_PRINTER) ||
         (rDCEvt.GetType() == DATACHANGED_DISPLAY) ||
         (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
             (rDCEvt.GetFlags() & SETTINGS_STYLE) )
        {
            // When the screen zoom factor has changed then reset the zoom
            // factor of the frame to allways display the whole page.
            const AllSettings* pOldSettings = rDCEvt.GetOldSettings ();
            const AllSettings& rNewSettings = GetSettings ();
            if (pOldSettings)
                if (pOldSettings->GetStyleSettings().GetScreenZoom()
                    != rNewSettings.GetStyleSettings().GetScreenZoom())
                    pViewShell->GetViewFrame()->GetDispatcher()->
                        Execute(SID_SIZE_PAGE, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);

            // ScrollBars neu anordnen bzw. Resize ausloesen, da sich
            // ScrollBar-Groesse geaendert haben kann. Dazu muss dann im
            // Resize-Handler aber auch die Groesse der ScrollBars aus
            // den Settings abgefragt werden.
            Resize();

            // Daten neu Setzen, die aus den Systemeinstellungen bzw. aus
            // den Settings uebernommen werden. Evtl. weitere Daten neu
            // berechnen, da sich auch die Aufloesung hierdurch geaendert
            // haben kann.
            if( pViewShell )
            {
                const StyleSettings&    rStyleSettings = GetSettings().GetStyleSettings();
                SvtAccessibilityOptions aAccOptions;
                ULONG                   nOutputMode;
                USHORT                  nPreviewSlot;

                if( rStyleSettings.GetHighContrastMode() )
                    nOutputMode = OUTPUT_DRAWMODE_CONTRAST;
                else
                    nOutputMode = OUTPUT_DRAWMODE_COLOR;

                if( rStyleSettings.GetHighContrastMode() && aAccOptions.GetIsForPagePreviews() )
                    nPreviewSlot = SID_PREVIEW_QUALITY_CONTRAST;
                else
                    nPreviewSlot = SID_PREVIEW_QUALITY_COLOR;

                if( pViewShell->ISA( SdDrawViewShell ) )
                {
                    SetDrawMode( nOutputMode );
                    pViewShell->GetFrameView()->SetDrawMode( nOutputMode );
                    pViewShell->GetView()->ReleaseMasterPagePaintCache();
                    Invalidate();
                }

                // #103100# Overwrite window color for OutlineView
                if( pViewShell->ISA( SdOutlineViewShell ) )
                {
                    svtools::ColorConfig aColorConfig;
                    const Color aDocColor( aColorConfig.GetColorValue( svtools::DOCCOLOR ).nColor );
                    SetBackground( Wallpaper( aDocColor ) );
                }

                SfxRequest aReq( nPreviewSlot, 0, pViewShell->GetDocSh()->GetDoc()->GetItemPool() );
                pViewShell->ExecReq( aReq );
                pViewShell->Invalidate();
                pViewShell->ArrangeGUIElements();

                // #101928# re-create handles to show new outfit
                if(pViewShell->ISA(SdDrawViewShell))
                {
                    pViewShell->GetView()->AdjustMarkHdl();
                }
            }
        }

        if ( (rDCEvt.GetType() == DATACHANGED_DISPLAY) ||
             ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
              (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
        {
            // Virtuelle Device die auch von der Aufloesung oder von
            // Systemeinstellungen abhaengen, sollten geupdatet werden.
            // Ansonsten sollte zumindest bei DATACHANGED_DISPLAY
            // die virtuellen Devices geupdatet werden, da es einige
            // Systeme erlauben die Aufloesung und Farbtiefe waehrend
            // der Laufzeit zu aendern oder eben bei Palettenaenderungen
            // die virtuellen Device geupdatet werden muessen, da bei
            // Ausgaben ein anderes Farbmatching stattfinden kann.
        }

        if ( rDCEvt.GetType() == DATACHANGED_FONTS )
        {
            // Wenn das Dokument Font-AuswahlBoxen anbietet, muessen
            // diese geupdatet werden. Wie dies genau aussehen muss,
            // weiss ich leider auch nicht. Aber evtl. kann man das
            // ja global handeln. Dies muessten wir evtl. mal
            // mit PB absprechen, aber der ist derzeit leider Krank.
            // Also bevor dies hier gehandelt wird, vorher mit
            // PB und mir absprechen.
        }

        if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
             (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) )
        {
            // Formatierung neu durchfuehren, da Fonts die im Dokument
            // vorkommen, nicht mehr vorhanden sein muessen oder
            // jetzt vorhanden sind oder durch andere ersetzt wurden
            // sind.
            if( pViewShell )
            {
                SdDrawDocShell* pDocSh = pViewShell->GetDocSh();
                if( pDocSh )
                    pDocSh->SetPrinter( pDocSh->GetPrinter( TRUE ) );
            }
        }

        if ( rDCEvt.GetType() == DATACHANGED_PRINTER )
        {
            // Wie hier die Behandlung aussehen soll, weiss ich leider
            // selbst noch nicht. Evtl. mal einen Printer loeschen und
            // schauen was gemacht werden muss. Evtl. muesste ich in
            // VCL dafuer noch etwas einbauen, wenn der benutze Printer
            // geloescht wird. Ansonsten wuerde ich hier evtl. die
            // Formatierung neu berechnen, wenn der aktuelle Drucker
            // zerstoert wurde.
            if( pViewShell )
            {
                SdDrawDocShell* pDocSh = pViewShell->GetDocSh();
                if( pDocSh )
                    pDocSh->SetPrinter( pDocSh->GetPrinter( TRUE ) );
            }
        }

        // Alles neu ausgeben
        Invalidate();
    }
}

/*************************************************************************
|*
|* DropTargetHelper::AcceptDrop
|*
\************************************************************************/

sal_Int8 SdWindow::AcceptDrop( const AcceptDropEvent& rEvt )
{
    sal_Int8 nRet = DND_ACTION_NONE;

    if( pViewShell && !pViewShell->GetDocSh()->IsReadOnly() )
    {
        if( pViewShell )
            nRet = pViewShell->AcceptDrop( rEvt, *this, this, SDRPAGE_NOTFOUND, SDRLAYER_NOTFOUND );

        if( !pViewShell->ISA( SdOutlineViewShell ) )
            DropScroll( rEvt.maPosPixel );
    }

    return nRet;
}

/*************************************************************************
|*
|* DropTargetHelper::ExecuteDrop
|*
\************************************************************************/

sal_Int8 SdWindow::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    sal_Int8 nRet = DND_ACTION_NONE;

    if( pViewShell )
    {
        nRet = pViewShell->ExecuteDrop( rEvt, *this, this, SDRPAGE_NOTFOUND, SDRLAYER_NOTFOUND );
    }

    return nRet;
}




::com::sun::star::uno::Reference<
    ::com::sun::star::accessibility::XAccessible>
    SdWindow::CreateAccessible (void)
{
    if (pViewShell != NULL)
    return pViewShell->CreateAccessibleDocumentView (this);
    else
    {
        OSL_TRACE ("SdWindow::CreateAccessible: no view shell");
    return Window::CreateAccessible ();
    }
}

