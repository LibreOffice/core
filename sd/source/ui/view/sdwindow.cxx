/*************************************************************************
 *
 *  $RCSfile: sdwindow.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 15:02:02 $
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

#include "Window.hxx"

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
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef SD_FRAME_VIEW_HXX
#include "FrameView.hxx"
#endif
#ifndef SD_OUTLINE_VIEW_SHELL_HXX
#include "OutlineViewShell.hxx"
#endif
#include "drawdoc.hxx"
#ifndef SD_ACCESSIBILITY_ACCESSIBLE_DRAW_DOCUMENT_VIEW_HXX
#include "AccessibleDrawDocumentView.hxx"
#endif
#ifndef SD_WINDOW_UPDATER_HXX
#include "WindowUpdater.hxx"
#endif

namespace sd {

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

Window::Window(::Window* pParent)
    : ::Window(pParent, WinBits(WB_CLIPCHILDREN | WB_DIALOGCONTROL)),
      DropTargetHelper( this ),
      mpShareWin(NULL),
      maWinPos(0, 0),           // vorsichtshalber; die Werte sollten aber
      maViewOrigin(0, 0),       // vom Besitzer des Fensters neu gesetzt
      maViewSize(1000, 1000),   // werden
      mnMinZoom(MIN_ZOOM),
      mnMaxZoom(MAX_ZOOM),
      mbMinZoomAutoCalc(false),
      mbCalcMinZoomByMinSide(true),
      mbCenterAllowed(true),
      mnTicks (0),
      mbDraggedFrom(false),
      mpViewShell(NULL),
      mbUseDropScroll (true)
{
    SetDialogControlFlags( WINDOW_DLGCTRL_RETURN | WINDOW_DLGCTRL_WANTFOCUS );

    MapMode aMap(GetMapMode());
    aMap.SetMapUnit(MAP_100TH_MM);
    SetMapMode(aMap);

    // Damit im Diamodus die ::WindowColor genommen wird
    SetBackground( Wallpaper( GetSettings().GetStyleSettings().GetWindowColor() ) );

    // adjust contrast mode initially
    bool bUseContrast = GetSettings().GetStyleSettings().GetHighContrastMode();
    SetDrawMode( bUseContrast
        ? ViewShell::OUTPUT_DRAWMODE_CONTRAST
        : ViewShell::OUTPUT_DRAWMODE_COLOR );

    // Hilfe-ID setzen
    // SetHelpId(HID_SD_WIN_DOCUMENT);
    SetUniqueId(HID_SD_WIN_DOCUMENT);
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

Window::~Window (void)
{
    if (mpViewShell != NULL)
    {
        WindowUpdater* pWindowUpdater = mpViewShell->GetWindowUpdater();
        if (pWindowUpdater != NULL)
            pWindowUpdater->UnregisterWindow (this);
    }
}




void Window::SetViewShell (ViewShell* pViewSh)
{
    WindowUpdater* pWindowUpdater = NULL;
    // Unregister at device updater of old view shell.
    if (mpViewShell != NULL)
    {
        pWindowUpdater = mpViewShell->GetWindowUpdater();
        if (pWindowUpdater != NULL)
            pWindowUpdater->UnregisterWindow (this);
    }

    mpViewShell = pViewSh;

    // Register at device updater of new view shell
    if (mpViewShell != NULL)
    {
        pWindowUpdater = mpViewShell->GetWindowUpdater();
        if (pWindowUpdater != NULL)
            pWindowUpdater->RegisterWindow (this);
    }
}




/*************************************************************************
|*
|* Die Haelfte des Sichtbaren Bereich eines anderen Fensters darstellen
|*
\************************************************************************/

void Window::ShareViewArea(Window* pOtherWin)
{
    mpShareWin      = pOtherWin;
    maViewOrigin    = pOtherWin->maViewOrigin;
    maViewSize      = pOtherWin->maViewSize;
    mnMinZoom       = pOtherWin->mnMinZoom;
    mnMaxZoom       = pOtherWin->mnMaxZoom;
    mbCenterAllowed = pOtherWin->mbCenterAllowed;

    long nZoom = pOtherWin->GetZoom();
    MapMode aMap(GetMapMode());
    aMap.SetScaleX(Fraction(nZoom, 100));
    aMap.SetScaleY(Fraction(nZoom, 100));
    aMap.SetOrigin(pOtherWin->GetMapMode().GetOrigin());
    SetMapMode(aMap);
}




void Window::CalcMinZoom()
{
    // Are we entitled to change the minimal zoom factor?
    if ( mbMinZoomAutoCalc )
    {
        // Get current zoom factor.
        long nZoom = GetZoom();

        if ( mpShareWin )
        {
            mpShareWin->CalcMinZoom();
            mnMinZoom = mpShareWin->mnMinZoom;
        }
        else
        {
            // Get the rectangle of the output area in logical coordinates
            // and calculate the scaling factors that would lead to the view
            // area (also called application area) to completely fill the
            // window.
            Size aWinSize = PixelToLogic(GetOutputSizePixel());
            ULONG nX = (ULONG) ((double) aWinSize.Width()
                * (double) ZOOM_MULTIPLICATOR / (double) maViewSize.Width());
            ULONG nY = (ULONG) ((double) aWinSize.Height()
                * (double) ZOOM_MULTIPLICATOR / (double) maViewSize.Height());

            // Decide whether to take the larger or the smaller factor.
            ULONG nFact;
            if (mbCalcMinZoomByMinSide)
                nFact = Min(nX, nY);
            else
                nFact = Max(nX, nY);

            // The factor is tansfomed according to the current zoom factor.
            nFact = nFact * nZoom / ZOOM_MULTIPLICATOR;
            mnMinZoom = Max((USHORT) MIN_ZOOM, (USHORT) nFact);
        }
        // If the current zoom factor is smaller than the calculated minimal
        // zoom factor then set the new minimal factor as the current zoom
        // factor.
        if ( nZoom < (long) mnMinZoom )
            SetZoomFactor(mnMinZoom);
    }
}




void Window::SetMinZoom (long int nMin)
{
    mnMinZoom = (USHORT) nMin;
}




long Window::GetMinZoom (void) const
{
    return mnMinZoom;
}




void Window::SetMaxZoom (long int nMax)
{
    mnMaxZoom = (USHORT) nMax;
}




long Window::GetMaxZoom (void) const
{
    return mnMaxZoom;
}




long Window::GetZoom (void) const
{
    return GetMapMode().GetScaleX().GetNumerator() * 100L
        / GetMapMode().GetScaleX().GetDenominator();
}




/*************************************************************************
|*
|* Resize event
|*
\************************************************************************/

void Window::Resize()
{
    ::Window::Resize();
    CalcMinZoom();
}

/*************************************************************************
|*
|* Paint event
|*
\************************************************************************/

void Window::Paint(const Rectangle& rRect)
{
    if ( mpViewShell )
        mpViewShell->Paint(rRect, this);
}

/*************************************************************************
|*
|* Keyboard event
|*
\************************************************************************/

void Window::KeyInput(const KeyEvent& rKEvt)
{
    BOOL aReturn = FALSE;

    if (!(mpViewShell && mpViewShell->KeyInput(rKEvt, this)))
    {
        if (mpViewShell && rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE)
        {
            // Wenn IP aktiv, wird der IP-Modus abgebrochen
            mpViewShell->GetDocSh()->DoInPlaceActivate(FALSE);
        }
        else
        {
            ::Window::KeyInput(rKEvt);
        }
    }
}

/*************************************************************************
|*
|* MouseButtonDown event
|*
\************************************************************************/

void Window::MouseButtonDown(const MouseEvent& rMEvt)
{
    if ( mpViewShell )
        mpViewShell->MouseButtonDown(rMEvt, this);
}

/*************************************************************************
|*
|* MouseMove event
|*
\************************************************************************/

void Window::MouseMove(const MouseEvent& rMEvt)
{
    if ( mpViewShell )
        mpViewShell->MouseMove(rMEvt, this);
}

/*************************************************************************
|*
|* MouseButtonUp event
|*
\************************************************************************/

void Window::MouseButtonUp(const MouseEvent& rMEvt)
{
    mnTicks = 0;

    if ( mpViewShell )
        mpViewShell->MouseButtonUp(rMEvt, this);
}

/*************************************************************************
|*
|* Command event
|*
\************************************************************************/

void Window::Command(const CommandEvent& rCEvt)
{
    if ( mpViewShell )
        mpViewShell->Command(rCEvt, this);
}

/*************************************************************************
|*
|* RequestHelp event
|*
\************************************************************************/

void Window::RequestHelp(const HelpEvent& rEvt)
{
    if ( mpViewShell )
    {
        if( !mpViewShell->RequestHelp( rEvt, this) )
            ::Window::RequestHelp( rEvt );
    }
    else
        ::Window::RequestHelp( rEvt );
}




Point Window::GetWinViewPos (void) const
{
    return maWinPos;
}




Point Window::GetViewOrigin (void) const
{
    return maViewOrigin;
}




Size Window::GetViewSize (void) const
{
    return maViewSize;
}




/*************************************************************************
|*
|* Position der linken oberen Ecke des im Fenster sichtbaren Bereichs
|* setzen
|*
\************************************************************************/

void Window::SetWinViewPos(const Point& rPnt)
{
    maWinPos = rPnt;
}

/*************************************************************************
|*
|* Ursprung der Darstellung in Bezug zur gesamten Arbeitsflaeche setzen
|*
\************************************************************************/

void Window::SetViewOrigin(const Point& rPnt)
{
    maViewOrigin = rPnt;
}

/*************************************************************************
|*
|* Groesse der gesamten Arbeitsflaeche, die mit dem Fenster betrachtet
|* werden kann, setzen
|*
\************************************************************************/

void Window::SetViewSize(const Size& rSize)
{
    maViewSize = rSize;
    CalcMinZoom();
}




void Window::SetCenterAllowed (bool bIsAllowed)
{
    mbCenterAllowed = bIsAllowed;
}




long Window::SetZoomFactor(long nZoom)
{
    // Clip the zoom factor to the valid range marked by nMinZoom as
    // calculated by CalcMinZoom() and the constant MAX_ZOOM.
    if ( nZoom > MAX_ZOOM )
        nZoom = MAX_ZOOM;
    if ( nZoom < (long) mnMinZoom )
        nZoom = mnMinZoom;

    // Set the zoom factor at the window's map mode.
    MapMode aMap(GetMapMode());
    aMap.SetScaleX(Fraction(nZoom, 100));
    aMap.SetScaleY(Fraction(nZoom, 100));
    SetMapMode(aMap);

    // Update the map mode's origin (to what effect?).
    UpdateMapOrigin();

    // Update the view's snapping to the the new zoom factor.
    if ( mpViewShell && mpViewShell->ISA(DrawViewShell) )
        ((DrawViewShell*) mpViewShell)->GetView()->
                                        RecalcLogicSnapMagnetic(*this);

    // Return the zoom factor just in case it has been changed above to lie
    // inside the valid range.
    return nZoom;
}




void Window::SetZoom(long nZoom)
{
    // Clip the zoom factor to the valid range marked by nMinZoom as
    // previously calculated by <member>CalcMinZoom()</member> and the
    // MAX_ZOOM constant.
    if ( nZoom > MAX_ZOOM )
        nZoom = MAX_ZOOM;
    if ( nZoom < (long) mnMinZoom )
        nZoom = mnMinZoom;

    // Calculate the window's new origin.
    Size aSize = PixelToLogic(GetOutputSizePixel());
    long nW = aSize.Width()  * GetZoom() / nZoom;
    long nH = aSize.Height() * GetZoom() / nZoom;
    maWinPos.X() += (aSize.Width()  - nW) / 2;
    maWinPos.Y() += (aSize.Height() - nH) / 2;
    if ( maWinPos.X() < 0 ) maWinPos.X() = 0;
    if ( maWinPos.Y() < 0 ) maWinPos.Y() = 0;

    // Finally update this window's map mode to the given zoom factor that
    // has been clipped to the valid range.
    SetZoomFactor(nZoom);
}




/** Recalculate the zoom factor and translation so that the given rectangle
    is displayed centered and as large as possible while still being fully
    visible in the window.
*/
long Window::SetZoomRect (const Rectangle& rZoomRect)
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

            maWinPos = maViewOrigin + aPos;

            aWinSize.Width() = (long) ((double) aWinSize.Width() * (double) ZOOM_MULTIPLICATOR / (double) nFact);
            maWinPos.X() += (rZoomRect.GetWidth() - aWinSize.Width()) / 2;
            aWinSize.Height() = (long) ((double) aWinSize.Height() * (double) ZOOM_MULTIPLICATOR / (double) nFact);
            maWinPos.Y() += (rZoomRect.GetHeight() - aWinSize.Height()) / 2;

            if ( maWinPos.X() < 0 ) maWinPos.X() = 0;
            if ( maWinPos.Y() < 0 ) maWinPos.Y() = 0;

            // Adapt the window's map mode to the new zoom factor.
            nNewZoom = SetZoomFactor(nZoom);
        }
    }

    return(nNewZoom);
}




void Window::SetMinZoomAutoCalc (bool bAuto)
{
    mbMinZoomAutoCalc = bAuto;
}




/*************************************************************************
|*
|* Neuen MapMode-Origin berechnen und setzen; wenn aWinPos.X()/Y()
|* gleich -1 ist, wird die entsprechende Position zentriert
|* (z.B. fuer Initialisierung)
|*
\************************************************************************/

void Window::UpdateMapOrigin(BOOL bInvalidate)
{
    BOOL    bChanged = FALSE;
    Size    aWinSize = PixelToLogic(GetOutputSizePixel());

    if ( mbCenterAllowed )
    {
        if ( maWinPos.X() > maViewSize.Width() - aWinSize.Width() )
        {
            maWinPos.X() = maViewSize.Width() - aWinSize.Width();
            bChanged = TRUE;
        }
        if ( maWinPos.Y() > maViewSize.Height() - aWinSize.Height() )
        {
            maWinPos.Y() = maViewSize.Height() - aWinSize.Height();
            bChanged = TRUE;
        }
        if ( aWinSize.Width() > maViewSize.Width() || maWinPos.X() < 0 )
        {
            maWinPos.X() = maViewSize.Width()  / 2 - aWinSize.Width()  / 2;
            bChanged = TRUE;
        }
        if ( aWinSize.Height() > maViewSize.Height() || maWinPos.Y() < 0 )
        {
            maWinPos.Y() = maViewSize.Height() / 2 - aWinSize.Height() / 2;
            bChanged = TRUE;
        }
    }

    UpdateMapMode ();

    if (bChanged && bInvalidate)
        Invalidate();
}




void Window::UpdateMapMode (void)
{
    Size aWinSize = PixelToLogic(GetOutputSizePixel());
    maWinPos -= maViewOrigin;
    Size aPix(maWinPos.X(), maWinPos.Y());
    aPix = LogicToPixel(aPix);
    // Groesse muss vielfaches von BRUSH_SIZE sein, damit Muster
    // richtig dargestellt werden
    // #i2237#
    // removed old stuff here which still forced zoom to be
    // %BRUSH_SIZE which is outdated now

    if (mpViewShell && mpViewShell->ISA(DrawViewShell))
    {
        Size aViewSizePixel = LogicToPixel(maViewSize);
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
    maWinPos.X() = aPix.Width();
    maWinPos.Y() = aPix.Height();
    Point aNewOrigin (-maWinPos.X(), -maWinPos.Y());
    maWinPos += maViewOrigin;

    MapMode aMap(GetMapMode());
    aMap.SetOrigin(aNewOrigin);
    SetMapMode(aMap);
}




/*************************************************************************
|*
|* X-Position des sichtbaren Bereichs als Bruchteil (< 1)
|* der gesamten Arbeitsbereichbreite zuruegeben
|*
\************************************************************************/

double Window::GetVisibleX()
{
    return ((double) maWinPos.X() / maViewSize.Width());
}

/*************************************************************************
|*
|* Y-Position des sichtbaren Bereichs als Bruchteil (< 1)
|* der gesamten Arbeitsbereichhoehe zuruegeben
|*
\************************************************************************/

double Window::GetVisibleY()
{
    return ((double) maWinPos.Y() / maViewSize.Height());
}

/*************************************************************************
|*
|* X- und Y-Position des sichtbaren Bereichs als Bruchteile (< 1)
|* der gesamten Arbeitsbereichgroesse setzen
|* negative Werte werden ignoriert
|*
\************************************************************************/

void Window::SetVisibleXY(double fX, double fY)
{
    long nOldX = maWinPos.X();
    long nOldY = maWinPos.Y();

    if ( fX >= 0 )
        maWinPos.X() = (long) (fX * maViewSize.Width());
    if ( fY >= 0 )
        maWinPos.Y() = (long) (fY * maViewSize.Height());
    UpdateMapOrigin(FALSE);
    //  Size sz(nOldX - aWinPos.X(), nOldY - aWinPos.Y());
    //  sz = LogicToPixel(sz);
    Scroll(nOldX - maWinPos.X(), nOldY - maWinPos.Y(), SCROLL_CHILDREN);
    Update();
}

/*************************************************************************
|*
|* Breite des sichtbaren Bereichs im Verhaeltnis zur
|* gesamten Arbeitsbereichbreite zuruegeben
|*
\************************************************************************/

double Window::GetVisibleWidth()
{
    Size aWinSize = PixelToLogic(GetOutputSizePixel());
    if ( aWinSize.Width() > maViewSize.Width() )
        aWinSize.Width() = maViewSize.Width();
    return ((double) aWinSize.Width() / maViewSize.Width());
}

/*************************************************************************
|*
|* Hoehe des sichtbaren Bereichs im Verhaeltnis zur
|* gesamten Arbeitsbereichhoehe zuruegeben
|*
\************************************************************************/

double Window::GetVisibleHeight()
{
    Size aWinSize = PixelToLogic(GetOutputSizePixel());
    if ( aWinSize.Height() > maViewSize.Height() )
        aWinSize.Height() = maViewSize.Height();
    return ((double) aWinSize.Height() / maViewSize.Height());
}

/*************************************************************************
|*
|* Breite einer Scrollspalte im Verhaeltnis zur gesamten
|* Arbeitsbereichbreite zuruegeben
|*
\************************************************************************/

double Window::GetScrlLineWidth()
{
    return (GetVisibleWidth() * SCROLL_LINE_FACT);
}

/*************************************************************************
|*
|* Breite einer Scrollspalte im Verhaeltnis zur gesamten
|* Arbeitsbereichhoehe zuruegeben
|*
\************************************************************************/

double Window::GetScrlLineHeight()
{
    return (GetVisibleHeight() * SCROLL_LINE_FACT);
}

/*************************************************************************
|*
|* Breite einer Scrollpage im Verhaeltnis zur gesamten
|* Arbeitsbereichbreite zuruegeben
|*
\************************************************************************/

double Window::GetScrlPageWidth()
{
    return (GetVisibleWidth() * SCROLL_PAGE_FACT);
}

/*************************************************************************
|*
|* Breite einer Scrollpage im Verhaeltnis zur gesamten
|* Arbeitsbereichhoehe zuruegeben
|*
\************************************************************************/

double Window::GetScrlPageHeight()
{
    return (GetVisibleHeight() * SCROLL_PAGE_FACT);
}

/*************************************************************************
|*
|* Fenster deaktivieren
|*
\************************************************************************/

void Window::LoseFocus()
{
    mnTicks = 0;
    ::Window::LoseFocus ();
}

/*************************************************************************
|*
|* Fenster aktivieren
|*
\************************************************************************/

void Window::GrabFocus()
{
    mnTicks      = 0;
    ::Window::GrabFocus ();
}


/*************************************************************************
|*
|* DataChanged
|*
\************************************************************************/

void Window::DataChanged( const DataChangedEvent& rDCEvt )
{
    ::Window::DataChanged( rDCEvt );

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
                    mpViewShell->GetViewFrame()->GetDispatcher()->
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
            if( mpViewShell )
            {
                const StyleSettings&    rStyleSettings = GetSettings().GetStyleSettings();
                SvtAccessibilityOptions aAccOptions;
                ULONG                   nOutputMode;
                USHORT                  nPreviewSlot;

                if( rStyleSettings.GetHighContrastMode() )
                    nOutputMode = ViewShell::OUTPUT_DRAWMODE_CONTRAST;
                else
                    nOutputMode = ViewShell::OUTPUT_DRAWMODE_COLOR;

                if( rStyleSettings.GetHighContrastMode() && aAccOptions.GetIsForPagePreviews() )
                    nPreviewSlot = SID_PREVIEW_QUALITY_CONTRAST;
                else
                    nPreviewSlot = SID_PREVIEW_QUALITY_COLOR;

                if( mpViewShell->ISA( DrawViewShell ) )
                {
                    SetDrawMode( nOutputMode );
                    mpViewShell->GetFrameView()->SetDrawMode( nOutputMode );
// #110094#-7
//                  mpViewShell->GetView()->ReleaseMasterPagePaintCache();
                    Invalidate();
                }

                // #103100# Overwrite window color for OutlineView
                if( mpViewShell->ISA(OutlineViewShell ) )
                {
                    svtools::ColorConfig aColorConfig;
                    const Color aDocColor( aColorConfig.GetColorValue( svtools::DOCCOLOR ).nColor );
                    SetBackground( Wallpaper( aDocColor ) );
                }

                SfxRequest aReq( nPreviewSlot, 0, mpViewShell->GetDocSh()->GetDoc()->GetItemPool() );
                mpViewShell->ExecReq( aReq );
                mpViewShell->Invalidate();
                mpViewShell->ArrangeGUIElements();

                // #101928# re-create handles to show new outfit
                if(mpViewShell->ISA(DrawViewShell))
                {
                    mpViewShell->GetView()->AdjustMarkHdl();
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
            if( mpViewShell )
            {
                DrawDocShell* pDocSh = mpViewShell->GetDocSh();
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
            if( mpViewShell )
            {
                DrawDocShell* pDocSh = mpViewShell->GetDocSh();
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

sal_Int8 Window::AcceptDrop( const AcceptDropEvent& rEvt )
{
    sal_Int8 nRet = DND_ACTION_NONE;

    if( mpViewShell && !mpViewShell->GetDocSh()->IsReadOnly() )
    {
        if( mpViewShell )
            nRet = mpViewShell->AcceptDrop( rEvt, *this, this, SDRPAGE_NOTFOUND, SDRLAYER_NOTFOUND );

        if (mbUseDropScroll && ! mpViewShell->ISA(OutlineViewShell))
            DropScroll( rEvt.maPosPixel );
    }

    return nRet;
}

/*************************************************************************
|*
|* DropTargetHelper::ExecuteDrop
|*
\************************************************************************/

sal_Int8 Window::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    sal_Int8 nRet = DND_ACTION_NONE;

    if( mpViewShell )
    {
        nRet = mpViewShell->ExecuteDrop( rEvt, *this, this, SDRPAGE_NOTFOUND, SDRLAYER_NOTFOUND );
    }

    return nRet;
}




void Window::SetUseDropScroll (bool bUseDropScroll)
{
    mbUseDropScroll = bUseDropScroll;
}




/*************************************************************************
|*
|* Scrolling bei AcceptDrop-Events
|*
\************************************************************************/

void Window::DropScroll(const Point& rMousePos)
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
        if (mnTicks > 20)
            mpViewShell->ScrollLines(nDx, nDy);
        else
            mnTicks ++;
    }
}




::com::sun::star::uno::Reference<
    ::com::sun::star::accessibility::XAccessible>
    Window::CreateAccessible (void)
{
    if (mpViewShell != NULL)
    return mpViewShell->CreateAccessibleDocumentView (this);
    else
    {
        OSL_TRACE ("::sd::Window::CreateAccessible: no view shell");
    return ::Window::CreateAccessible ();
    }
}

} // end of namespace sd
