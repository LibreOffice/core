/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "Window.hxx"
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>

#include <sfx2/viewfrm.hxx>
#include <svx/svxids.hrc>

#include <editeng/outliner.hxx>
#include <editeng/editview.hxx>

#include "app.hrc"
#include "helpids.h"
#include "ViewShell.hxx"
#include "DrawViewShell.hxx"
#include "View.hxx"
#include "FrameView.hxx"
#include "OutlineViewShell.hxx"
#include "drawdoc.hxx"
#include "AccessibleDrawDocumentView.hxx"
#include "WindowUpdater.hxx"

//IAccessibility2 Implementation 2009-----
#include <vcl/svapp.hxx>
//-----IAccessibility2 Implementation 2009

namespace sd {

#define SCROLL_LINE_FACT   0.05     // Faktor fuer Zeilenscrolling
#define SCROLL_PAGE_FACT   0.5      // Faktor fuer Seitenscrolling
#define SCROLL_SENSITIVE   20       // Sensitiver Bereich (Pixel)
#define ZOOM_MULTIPLICATOR 10000    // Multiplikator um Rundungsfehler zu vermeiden
#define MIN_ZOOM           5.0      // Minimaler Zoomfaktor
#define MAX_ZOOM           3000.0   // Maximaler Zoomfaktor


/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

Window::Window(::Window* pParent)
    : ::Window(pParent, WinBits(WB_CLIPCHILDREN | WB_DIALOGCONTROL)),
      DropTargetHelper( this ),
      mpShareWin(NULL),
      maWinPos(0.0, 0.0),           // vorsichtshalber; die Werte sollten aber
      maViewOrigin(0.0, 0.0),       // vom Besitzer des Fensters neu gesetzt
      maViewSize(1000.0, 1000.0),   // werden
      mfMinZoom(MIN_ZOOM),
      mfMaxZoom(MAX_ZOOM),
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
        ? SD_OUTPUT_DRAWMODE_CONTRAST
        : SD_OUTPUT_DRAWMODE_COLOR );

    // Hilfe-ID setzen
    // SetHelpId(HID_SD_WIN_DOCUMENT);
    SetUniqueId(HID_SD_WIN_DOCUMENT);

    // #i78183# Added after discussed with AF
    EnableRTL(false);
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

void Window::CalcMinZoom()
{
    // Are we entitled to change the minimal zoom factor?
    if ( mbMinZoomAutoCalc )
    {
        // Get current zoom factor.
        const double fZoom(GetZoom());

        if ( mpShareWin )
        {
            mpShareWin->CalcMinZoom();
            mfMinZoom = mpShareWin->mfMinZoom;
        }
        else
        {
            // Get the rectangle of the output area in logical coordinates
            // and calculate the scaling factors that would lead to the view
            // area (also called application area) to completely fill the
            // window.
            const basegfx::B2DVector aWinSize(GetLogicVector());
            const double fX(aWinSize.getX() / maViewSize.getX());
            const double fY(aWinSize.getY() / maViewSize.getY());

            // Decide whether to take the larger or the smaller factor.
            double fFact;

            if (mbCalcMinZoomByMinSide)
                fFact = std::min(fX, fY);
            else
                fFact = std::max(fX, fY);

            // The factor is tansfomed according to the current zoom factor.
            fFact *= fZoom;
            mfMinZoom = std::max(MIN_ZOOM, fFact);
        }
        // If the current zoom factor is smaller than the calculated minimal
        // zoom factor then set the new minimal factor as the current zoom
        // factor.
        if ( fZoom < mfMinZoom )
            SetZoomFactor(mfMinZoom);
    }
}




void Window::SetMinZoom (double fMin)
{
    mfMinZoom = fMin;
}




double Window::GetMinZoom (void) const
{
    return mfMinZoom;
}




void Window::SetMaxZoom (double fMax)
{
    mfMaxZoom = fMax;
}




double Window::GetMaxZoom (void) const
{
    return mfMaxZoom;
}




double Window::GetZoom (void) const
{
    if( GetMapMode().GetScaleX().GetDenominator() )
    {
        return GetMapMode().GetScaleX().GetNumerator() * 100.0
            / GetMapMode().GetScaleX().GetDenominator();
    }
    else
    {
        return 0.0;
    }
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

    if( mpViewShell && mpViewShell->GetViewFrame() )
        mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_ATTR_ZOOMSLIDER );
}

/*************************************************************************
|*
|* PrePaint event
|*
\************************************************************************/

void Window::PrePaint()
{
    if ( mpViewShell )
        mpViewShell->PrePaint();
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
    if (!(mpViewShell && mpViewShell->KeyInput(rKEvt, this)))
    {
        if (mpViewShell && rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE)
        {
            mpViewShell->GetViewShell()->Escape();
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

long Window::Notify( NotifyEvent& rNEvt )
{
    long nResult = false;
    if ( mpViewShell )
    {
        nResult = mpViewShell->Notify(rNEvt, this);
    }
    if( !nResult )
        nResult = ::Window::Notify( rNEvt );

    return nResult;
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




basegfx::B2DPoint Window::GetWinViewPos (void) const
{
    return maWinPos;
}




basegfx::B2DPoint Window::GetViewOrigin (void) const
{
    return maViewOrigin;
}




basegfx::B2DVector Window::GetViewSize (void) const
{
    return maViewSize;
}




/*************************************************************************
|*
|* Position der linken oberen Ecke des im Fenster sichtbaren Bereichs
|* setzen
|*
\************************************************************************/

void Window::SetWinViewPos(const basegfx::B2DPoint& rPnt)
{
    maWinPos = rPnt;
}

/*************************************************************************
|*
|* Ursprung der Darstellung in Bezug zur gesamten Arbeitsflaeche setzen
|*
\************************************************************************/

void Window::SetViewOrigin(const basegfx::B2DPoint& rPnt)
{
    maViewOrigin = rPnt;
}

/*************************************************************************
|*
|* Groesse der gesamten Arbeitsflaeche, die mit dem Fenster betrachtet
|* werden kann, setzen
|*
\************************************************************************/

void Window::SetViewSize(const basegfx::B2DVector& rSize)
{
    maViewSize = rSize;
    CalcMinZoom();
}




void Window::SetCenterAllowed (bool bIsAllowed)
{
    mbCenterAllowed = bIsAllowed;
}




double Window::SetZoomFactor(double fZoom)
{
    // Clip the zoom factor to the valid range marked by nMinZoom as
    // calculated by CalcMinZoom() and the constant MAX_ZOOM.
    if ( fZoom > MAX_ZOOM )
    {
        fZoom = MAX_ZOOM;
    }

    if ( fZoom < mfMinZoom )
    {
        fZoom = mfMinZoom;
    }

    // Set the zoom factor at the window's map mode.
    MapMode aMap(GetMapMode());
    aMap.SetScaleX(Fraction(fZoom * 0.01));
    aMap.SetScaleY(Fraction(fZoom * 0.01));
    SetMapMode(aMap);

    // Update the map mode's origin (to what effect?).
    UpdateMapOrigin();

    // Return the zoom factor just in case it has been changed above to lie
    // inside the valid range.
    return fZoom;
}

void Window::SetZoomIntegral(double fZoom)
{
    // Clip the zoom factor to the valid range marked by nMinZoom as
    // previously calculated by <member>CalcMinZoom()</member> and the
    // MAX_ZOOM constant.
    if ( fZoom > MAX_ZOOM )
    {
        fZoom = MAX_ZOOM;
    }

    if ( fZoom < mfMinZoom )
    {
        fZoom = mfMinZoom;
    }

    // Calculate the window's new origin.
    const basegfx::B2DVector aWinSize(GetLogicVector());
    const double fW(aWinSize.getX() * GetZoom() / fZoom);
    const double fH(aWinSize.getY() * GetZoom() / fZoom);

    maWinPos.setX(maWinPos.getX() + ((aWinSize.getX() - fW) * 0.5));
    maWinPos.setY(maWinPos.getY() + ((aWinSize.getY() - fH) * 0.5));

    if(maWinPos.getX() < 0.0)
    {
        maWinPos.setX(0.0);
    }

    if(maWinPos.getY() < 0.0)
    {
        maWinPos.setY(0.0);
    }

    // Finally update this window's map mode to the given zoom factor that
    // has been clipped to the valid range.
    SetZoomFactor(fZoom);
}

double Window::GetZoomForRange(const basegfx::B2DRange& rZoomRange)
{
    const basegfx::B2DVector aZoomScale(rZoomRange.getRange());
    double fRetZoom(100.0);

    if(!aZoomScale.equalZero())
    {
        // Calculate the scale factors which will lead to the given
        // rectangle being fully visible (when translated accordingly) as
        // large as possible in the output area independently in both
        // coordinate directions .
        double fX(0.0);
        double fY(0.0);
        const basegfx::B2DVector aWinSize(GetLogicVector());

        if(!basegfx::fTools::equalZero(aZoomScale.getY()))
        {
            fX = aWinSize.getY() / aZoomScale.getY();
        }

        if(!basegfx::fTools::equalZero(aZoomScale.getX()))
        {
            fY = aWinSize.getX() / aZoomScale.getX();
        }

        // Use the smaller one of both so that the zoom rectangle will be
        // fully visible with respect to both coordinate directions.
        double fFact(std::min(fX, fY));

        // Transform the current zoom factor so that it leads to the desired
        // scaling.
        fRetZoom = fFact * GetZoom();

        // Calculate the new origin.
        if(basegfx::fTools::equalZero(fFact))
        {
            // Don't change anything if the scale factor is degenrate.
            fRetZoom = GetZoom();
        }
        else
        {
            // Clip the zoom factor to the valid range marked by nMinZoom as
            // previously calculated by <member>CalcMinZoom()</member> and the
            // MAX_ZOOM constant.
            if(fRetZoom > MAX_ZOOM)
            {
                fRetZoom = MAX_ZOOM;
            }

            if(fRetZoom < mfMinZoom)
            {
                fRetZoom = mfMinZoom;
            }
       }
    }

    return fRetZoom;
}

/** Recalculate the zoom factor and translation so that the given rectangle
    is displayed centered and as large as possible while still being fully
    visible in the window.
*/
double Window::SetZoomRange(const basegfx::B2DRange& rZoomRange)
{
    const basegfx::B2DVector aZoomScale(rZoomRange.getRange());
    double fNewZoom(100.0);

    if(aZoomScale.equalZero())
    {
        // The given rectangle is degenerate.  Use the default zoom factor
        // (above) of 100%.
        SetZoomIntegral(fNewZoom);
    }
    else
    {
        const basegfx::B2DPoint aPos(rZoomRange.getMinimum());
        // Transform the output area from pixel coordinates into logical
        // coordinates.
        const basegfx::B2DVector aWinSize(GetLogicVector());

        // Calculate the scale factors which will lead to the given
        // rectangle being fully visible (when translated accordingly) as
        // large as possible in the output area independently in both
        // coordinate directions .
        double fX(0.0);
        double fY(0.0);

        if(!basegfx::fTools::equalZero(aZoomScale.getX()))
        {
            fX = aWinSize.getX() / aZoomScale.getX();
        }

        if(!basegfx::fTools::equalZero(aZoomScale.getY()))
        {
            fY = aWinSize.getY() / aZoomScale.getY();
        }

        // Use the smaller one of both so that the zoom rectangle will be
        // fully visible with respect to both coordinate directions.
        double fFact(std::min(fX, fY));

        // Transform the current zoom factor so that it leads to the desired
        // scaling.
        const double fZoom(fFact * GetZoom());

        // Calculate the new origin.
        if(basegfx::fTools::equalZero(fFact))
        {
            // Don't change anything if the scale factor is degenrate.
            fNewZoom = GetZoom();
        }
        else
        {
            // Calculate the new window position that centers the given
            // rectangle on the screen.
            if ( fZoom > MAX_ZOOM )
            {
                fFact = fFact * MAX_ZOOM / fZoom;
            }

            maWinPos = maViewOrigin + aPos;

            const double fNewWinX(aWinSize.getX() / fFact);
            const double fNewWinY(aWinSize.getY() / fFact);

            maWinPos.setX(maWinPos.getX() + ((aZoomScale.getX() - fNewWinX) * 0.5));
            maWinPos.setY(maWinPos.getY() + ((aZoomScale.getY() - fNewWinY) * 0.5));

            if(maWinPos.getX() < 0.0)
            {
                maWinPos.setX(0.0);
            }

            if(maWinPos.getY() < 0.0)
            {
                maWinPos.setY(0.0);
            }

            // Adapt the window's map mode to the new zoom factor.
            fNewZoom = SetZoomFactor(fZoom);
        }
    }

    return(fNewZoom);
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

void Window::UpdateMapOrigin(bool bInvalidate)
{
    bool bChanged(false);

    if ( mbCenterAllowed )
    {
        const basegfx::B2DVector aWinSize(GetLogicVector());

        if(basegfx::fTools::more(maWinPos.getX(), maViewSize.getX() - aWinSize.getX()))
        {
            maWinPos.setX(maViewSize.getX() - aWinSize.getX());
            bChanged = true;
        }

        if(basegfx::fTools::more(maWinPos.getY(), maViewSize.getY() - aWinSize.getY()))
        {
            maWinPos.setY(maViewSize.getY() - aWinSize.getY());
            bChanged = true;
        }

        if(basegfx::fTools::more(aWinSize.getX(), maViewSize.getX()) || (maWinPos.getX() < 0.0))
        {
            maWinPos.setX((maViewSize.getX() - aWinSize.getX()) * 0.5);
            bChanged = true;
        }

        if(basegfx::fTools::more(aWinSize.getY(), maViewSize.getY()) || (maWinPos.getY() < 0.0 ))
        {
            maWinPos.setY((maViewSize.getY() - aWinSize.getY()) * 0.5);
            bChanged = true;
        }
    }

    UpdateMapMode();

    if(bChanged && bInvalidate)
    {
        Invalidate();
    }
}

void Window::UpdateMapMode(void)
{
    const Point aNewOffset(
        basegfx::fround(maViewOrigin.getX() - maWinPos.getX()),
        basegfx::fround(maViewOrigin.getY() - maWinPos.getY()));
    MapMode aMap(GetMapMode());

    if(aMap.GetOrigin() != aNewOffset)
    {
        aMap.SetOrigin(aNewOffset);

        SetMapMode(aMap);
    }
}

/*************************************************************************
|*
|* X-Position des sichtbaren Bereichs als Bruchteil (< 1)
|* der gesamten Arbeitsbereichbreite zuruegeben
|*
\************************************************************************/

double Window::GetVisibleX()
{
    return maWinPos.getX() / maViewSize.getX();
}

/*************************************************************************
|*
|* Y-Position des sichtbaren Bereichs als Bruchteil (< 1)
|* der gesamten Arbeitsbereichhoehe zuruegeben
|*
\************************************************************************/

double Window::GetVisibleY()
{
    return maWinPos.getY() / maViewSize.getY();
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
    const basegfx::B2DPoint aOldWinPos(maWinPos);

    if(basegfx::fTools::moreOrEqual(fX, 0.0))
    {
        maWinPos.setX(fX * maViewSize.getX());
    }

    if(basegfx::fTools::moreOrEqual(fY, 0.0))
    {
        maWinPos.setY(fY * maViewSize.getY());
    }

    UpdateMapOrigin(false);
    Scroll(
        basegfx::fround(aOldWinPos.getX() - maWinPos.getX()),
        basegfx::fround(aOldWinPos.getY() - maWinPos.getY()),
        SCROLL_CHILDREN);
    Update();
}

/*************************************************************************
|*
|* Breite des sichtbaren Bereichs im Verhaeltnis zur
|* gesamten Arbeitsbereichbreite zuruegeben
|*
\************************************************************************/

double Window::GetVisibleWidthRelativeToView()
{
    basegfx::B2DVector aWinSize(GetLogicVector());

    if(basegfx::fTools::more(aWinSize.getX(), maViewSize.getX()))
    {
        aWinSize.setX(maViewSize.getX());
    }

    return aWinSize.getX() / maViewSize.getX();
}

/*************************************************************************
|*
|* Hoehe des sichtbaren Bereichs im Verhaeltnis zur
|* gesamten Arbeitsbereichhoehe zuruegeben
|*
\************************************************************************/

double Window::GetVisibleHeightRelativeToView()
{
    basegfx::B2DVector aWinSize(GetLogicVector());

    if(basegfx::fTools::more(aWinSize.getY(), maViewSize.getY()))
    {
        aWinSize.setY(maViewSize.getY());
    }

    return aWinSize.getY() / maViewSize.getY();
}

/*************************************************************************
|*
|* Breite einer Scrollspalte im Verhaeltnis zur gesamten
|* Arbeitsbereichbreite zuruegeben
|*
\************************************************************************/

double Window::GetScrlLineWidth()
{
    return (GetVisibleWidthRelativeToView() * SCROLL_LINE_FACT);
}

/*************************************************************************
|*
|* Breite einer Scrollspalte im Verhaeltnis zur gesamten
|* Arbeitsbereichhoehe zuruegeben
|*
\************************************************************************/

double Window::GetScrlLineHeight()
{
    return (GetVisibleHeightRelativeToView() * SCROLL_LINE_FACT);
}

/*************************************************************************
|*
|* Breite einer Scrollpage im Verhaeltnis zur gesamten
|* Arbeitsbereichbreite zuruegeben
|*
\************************************************************************/

double Window::GetScrlPageWidth()
{
    return (GetVisibleWidthRelativeToView() * SCROLL_PAGE_FACT);
}

/*************************************************************************
|*
|* Breite einer Scrollpage im Verhaeltnis zur gesamten
|* Arbeitsbereichhoehe zuruegeben
|*
\************************************************************************/

double Window::GetScrlPageHeight()
{
    return (GetVisibleHeightRelativeToView() * SCROLL_PAGE_FACT);
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
                sal_uLong                   nOutputMode;
                sal_uInt16                  nPreviewSlot;

                if( rStyleSettings.GetHighContrastMode() )
                    nOutputMode = SD_OUTPUT_DRAWMODE_CONTRAST;
                else
                    nOutputMode = SD_OUTPUT_DRAWMODE_COLOR;

                if( rStyleSettings.GetHighContrastMode() && aAccOptions.GetIsForPagePreviews() )
                    nPreviewSlot = SID_PREVIEW_QUALITY_CONTRAST;
                else
                    nPreviewSlot = SID_PREVIEW_QUALITY_COLOR;

                if( dynamic_cast< DrawViewShell* >(mpViewShell) )
                {
                    SetDrawMode( nOutputMode );
                    mpViewShell->GetFrameView()->SetDrawMode( nOutputMode );
// #110094#-7
//                  mpViewShell->GetView()->ReleaseMasterPagePaintCache();
                    Invalidate();
                }

                // #103100# Overwrite window color for OutlineView
                if( dynamic_cast< OutlineViewShell* >(mpViewShell) )
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
                if(dynamic_cast< DrawViewShell* >(mpViewShell))
                {
                    mpViewShell->GetView()->RecreateAllMarkHandles();
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
                    pDocSh->SetPrinter( pDocSh->GetPrinter( true ) );
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
                    pDocSh->SetPrinter( pDocSh->GetPrinter( true ) );
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

        if (mbUseDropScroll && !dynamic_cast< OutlineViewShell* >(mpViewShell))
        {
            DropScroll( basegfx::B2DPoint(rEvt.maPosPixel.X(), rEvt.maPosPixel.Y()) );
        }
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

void Window::DropScroll(const basegfx::B2DPoint& rMousePos)
{
    const basegfx::B2DVector aDiscretePixels(GetDiscreteVector());
    basegfx::B2DVector fDelta(0.0, 0.0);

    if (aDiscretePixels.getX() > SCROLL_SENSITIVE * 3)
    {
        if ( rMousePos.getX() < SCROLL_SENSITIVE )
        {
            fDelta.setX(-1.0);
        }

        if ( rMousePos.getX() >= aDiscretePixels.getX() - SCROLL_SENSITIVE )
        {
            fDelta.setX(1.0);
        }
    }

    if (aDiscretePixels.getY() > SCROLL_SENSITIVE * 3)
    {
        if ( rMousePos.getY() < SCROLL_SENSITIVE )
        {
            fDelta.setY(-1.0);
        }

        if ( rMousePos.getY() >= aDiscretePixels.getY() - SCROLL_SENSITIVE )
        {
            fDelta.setY(1.0);
        }
    }

    if(!fDelta.equalZero() && !rMousePos.equalZero())
    {
        if (mnTicks > 20)
        {
            mpViewShell->ScrollLines(fDelta);
        }
        else
        {
            mnTicks ++;
        }
    }
}




::com::sun::star::uno::Reference<
    ::com::sun::star::accessibility::XAccessible>
    Window::CreateAccessible (void)
{
//IAccessibility2 Implementation 2009-----
    // If current viewshell is PresentationViewShell, just return empty because the correct ShowWin will be created later.
    if (mpViewShell && dynamic_cast< PresentationViewShell* >(mpViewShell))
    {
        return ::Window::CreateAccessible ();
    }
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > xAcc = GetAccessible(sal_False);
    if (xAcc.get())
    {
        return xAcc;
    }
    if (mpViewShell != NULL)
    //return mpViewShell->CreateAccessibleDocumentView (this);
    {
        xAcc = mpViewShell->CreateAccessibleDocumentView (this);
        SetAccessible(xAcc);
        return xAcc;
    }
//-----IAccessibility2 Implementation 2009
    else
    {
        OSL_TRACE ("::sd::Window::CreateAccessible: no view shell");
        return ::Window::CreateAccessible ();
    }
}

//IAccessibility2 Implementation 2009-----
// MT: Removed Windows::SwitchView() introduced with IA2 CWS.
// There are other notifications for this when the active view has chnaged, so please update the code to use that event mechanism
void Window::SwitchView()
{
    if (!Application::IsAccessibilityEnabled())
    {
        return ;
    }
    if (mpViewShell)
    {
        mpViewShell->SwitchViewFireFocus(GetAccessible(sal_False));
    }
}
//-----IAccessibility2 Implementation 2009

XubString Window::GetSurroundingText() const
{
    if ( mpViewShell->GetShellType() == ViewShell::ST_OUTLINE )
    {
        return XubString();
    }
    else if ( mpViewShell->GetView()->IsTextEdit() )
    {
        OutlinerView *pOLV = mpViewShell->GetView()->GetTextEditOutlinerView();
        return pOLV->GetEditView().GetSurroundingText();
    }
    else
    {
        return XubString();
    }
}

Selection Window::GetSurroundingTextSelection() const
{
    if ( mpViewShell->GetShellType() == ViewShell::ST_OUTLINE )
    {
        return Selection( 0, 0 );
    }
    else if ( mpViewShell->GetView()->IsTextEdit() )
    {
        OutlinerView *pOLV = mpViewShell->GetView()->GetTextEditOutlinerView();
        return pOLV->GetEditView().GetSurroundingTextSelection();
    }
    else
    {
        return Selection( 0, 0 );
    }
}

} // end of namespace sd
