/*************************************************************************
 *
 *  $RCSfile: sdwindow.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ka $ $Date: 2001-03-08 12:38:30 $
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

#pragma hdrstop

#include "helpids.h"
#include "sdwindow.hxx"
#include "viewshel.hxx"
#include "drviewsh.hxx"
#include "sdview.hxx"
#include "outlnvsh.hxx"


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
    bMinZoomAutoCalc(TRUE),
    bCalcMinZoomByMinSide(TRUE),
    bCenterAllowed(TRUE),
    nTicks (0)
{
    SetDialogControlFlags( WINDOW_DLGCTRL_RETURN | WINDOW_DLGCTRL_WANTFOCUS );

    MapMode aMap(GetMapMode());
    aMap.SetMapUnit(MAP_100TH_MM);
    SetMapMode(aMap);

    // Damit im Diamodus die Wiese weiss ist
    SetBackground(Wallpaper(GetSettings().GetStyleSettings().GetWindowColor()));

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

/*************************************************************************
|*
|* Resize event
|*
\************************************************************************/

void SdWindow::CalcMinZoom()
{
    if ( bMinZoomAutoCalc )
    {
        long nZoom = GetZoom();

        if ( pShareWin )
        {
            pShareWin->CalcMinZoom();
            nMinZoom = pShareWin->nMinZoom;
        }
        else
        {
            Size aWinSize = PixelToLogic(GetOutputSizePixel());
            ULONG nX = (ULONG) ((double) aWinSize.Width()  * (double) ZOOM_MULTIPLICATOR / (double) aViewSize.Width());
            ULONG nY = (ULONG) ((double) aWinSize.Height() * (double) ZOOM_MULTIPLICATOR / (double) aViewSize.Height());
            ULONG nFact;

            if ( bCalcMinZoomByMinSide )    nFact = Min(nX, nY);
            else                            nFact = Max(nX, nY);
            nFact = nFact * nZoom / ZOOM_MULTIPLICATOR;
            nMinZoom = Max((USHORT) MIN_ZOOM, (USHORT) nFact);
        }
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

/*************************************************************************
|*
|* den eigentlichen Zoomfaktor in Prozent setzen; protected, wird
|* nur intern benutzt
|*
\************************************************************************/

long SdWindow::SetZoomFactor(long nZoom)
{
    if ( nZoom > MAX_ZOOM ) nZoom = MAX_ZOOM;
    if ( nZoom < (long) nMinZoom ) nZoom = nMinZoom;

    MapMode aMap(GetMapMode());
    aMap.SetScaleX(Fraction(nZoom, 100));
    aMap.SetScaleY(Fraction(nZoom, 100));
    SetMapMode(aMap);
    UpdateMapOrigin();
    if ( pViewShell && pViewShell->ISA(SdDrawViewShell) )
        ((SdDrawViewShell*) pViewShell)->GetView()->
                                        RecalcLogicSnapMagnetic(*this);
    return nZoom;
}

/*************************************************************************
|*
|* Zoomfaktor in Prozent setzen und Darstellungsbereich um den
|* Zoom-Mittelpunkt zentrieren
|*
\************************************************************************/

void SdWindow::SetZoom(long nZoom)
{
    if ( nZoom > MAX_ZOOM ) nZoom = MAX_ZOOM;
    if ( nZoom < (long) nMinZoom ) nZoom = nMinZoom;

    Size aSize = PixelToLogic(GetOutputSizePixel());
    long nW = aSize.Width()  * GetZoom() / nZoom;
    long nH = aSize.Height() * GetZoom() / nZoom;
    aWinPos.X() += (aSize.Width()  - nW) / 2;
    aWinPos.Y() += (aSize.Height() - nH) / 2;
    if ( aWinPos.X() < 0 ) aWinPos.X() = 0;
    if ( aWinPos.Y() < 0 ) aWinPos.Y() = 0;
    SetZoomFactor(nZoom);
}

/*************************************************************************
|*
|* Fensterposition und Zoomfaktor nach uebergebenem Rechteck setzen
|* obere linke Ecke des Rechtecks muss relativ zur linken oberen
|* Ecke des Fensters angegeben werden; gibt den berechneten Zoom-
|* faktor zurueck
|*
\************************************************************************/

long SdWindow::SetZoomRect(const Rectangle& rZoomRect)
{
    long nNewZoom = 100;

    if (rZoomRect.GetWidth() == 0 || rZoomRect.GetHeight() == 0)
    {
        SetZoom(nNewZoom);
    }
    else
    {
        Point aPos = rZoomRect.TopLeft();
        Size aWinSize = PixelToLogic(GetOutputSizePixel());
        DBG_ASSERT(rZoomRect.GetWidth(), "ZoomRect-Breite = 0!");
        DBG_ASSERT(rZoomRect.GetHeight(), "ZoomRect-Hoehe = 0!");
        ULONG nX = (ULONG) ((double) aWinSize.Width()  * (double) ZOOM_MULTIPLICATOR / (double) rZoomRect.GetWidth());
        ULONG nY = (ULONG) ((double) aWinSize.Height() * (double) ZOOM_MULTIPLICATOR / (double) rZoomRect.GetHeight());
        ULONG nFact = Min(nX, nY);
        long nZoom = nFact * GetZoom() / ZOOM_MULTIPLICATOR;

        if ( nFact == 0 )
        {
            nNewZoom = GetZoom();
        }
        else
        {
            if ( nZoom > MAX_ZOOM )
                nFact = nFact * MAX_ZOOM / nZoom;

            aWinPos = aViewOrigin + aPos;

            aWinSize.Width() = (long) ((double) aWinSize.Width() * (double) ZOOM_MULTIPLICATOR / (double) nFact);
            aWinPos.X() += (rZoomRect.GetWidth() - aWinSize.Width()) / 2;
            aWinSize.Height() = (long) ((double) aWinSize.Height() * (double) ZOOM_MULTIPLICATOR / (double) nFact);
            aWinPos.Y() += (rZoomRect.GetHeight() - aWinSize.Height()) / 2;

            if ( aWinPos.X() < 0 )  aWinPos.X() = 0;
            if ( aWinPos.Y() < 0 )  aWinPos.Y() = 0;
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
        aPix.Width()  -= aPix.Width() % BRUSH_SIZE;
        aPix.Height() -= aPix.Height() % BRUSH_SIZE;

        if (pViewShell && pViewShell->ISA(SdDrawViewShell))
        {
            Size aViewSizePixel = LogicToPixel(aViewSize);
            Size aWinSizePixel = LogicToPixel(aWinSize);

            // Seite soll nicht am Fensterrand "kleben"
            if (aPix.Width() == 0)
            {
                aPix.Width() -= BRUSH_SIZE;
            }
            if (aPix.Height() == 0)
            {
                aPix.Height() -= BRUSH_SIZE;
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
|* Scrolling bei QueryDrop-Events
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
            // ScrollBars neu anordnen bzw. Resize ausloesen, da sich
            // ScrollBar-Groesse geaendert haben kann. Dazu muss dann im
            // Resize-Handler aber auch die Groesse der ScrollBars aus
            // den Settings abgefragt werden.
            Resize();

            // Daten neu Setzen, die aus den Systemeinstellungen bzw. aus
            // den Settings uebernommen werden. Evtl. weitere Daten neu
            // berechnen, da sich auch die Aufloesung hierdurch geaendert
            // haben kann.
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
            nRet = pViewShell->AcceptDrop( rEvt, this, SDRPAGE_NOTFOUND, SDRLAYER_NOTFOUND );

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
        nRet = pViewShell->ExecuteDrop( rEvt, this, SDRPAGE_NOTFOUND, SDRLAYER_NOTFOUND );
    }

    return nRet;
}
