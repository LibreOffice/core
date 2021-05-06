/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <Window.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>

#include <sfx2/viewfrm.hxx>
#include <svx/svxids.hrc>

#include <editeng/outliner.hxx>
#include <editeng/editview.hxx>
#include <editeng/editeng.hxx>

#include <app.hrc>
#include <ViewShell.hxx>
#include <DrawViewShell.hxx>
#include <DrawDocShell.hxx>
#include <PresentationViewShell.hxx>
#include <View.hxx>
#include <FrameView.hxx>
#include <OutlineViewShell.hxx>
#include <OutlineView.hxx>
#include <drawdoc.hxx>
#include <WindowUpdater.hxx>
#include <ViewShellBase.hxx>
#include <uiobject.hxx>

#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/settings.hxx>
#include <comphelper/lok.hxx>
#include <sfx2/lokhelper.hxx>

namespace sd {

#define SCROLL_LINE_FACT   0.05     ///< factor for line scrolling
#define SCROLL_PAGE_FACT   0.5      ///< factor for page scrolling
#define SCROLL_SENSITIVE   20       ///< sensitive area in pixel
#define ZOOM_MULTIPLICATOR 10000    ///< multiplier to avoid rounding errors
#define MIN_ZOOM           5        ///< minimal zoom factor
#define MAX_ZOOM           3000     ///< maximal zoom factor

Window::Window(vcl::Window* pParent)
    : vcl::Window(pParent, WinBits(WB_CLIPCHILDREN | WB_DIALOGCONTROL)),
      DropTargetHelper( this ),
      maWinPos(0, 0),           // precautionary; but the values should be set
      maViewOrigin(0, 0),       // again from the owner of the window
      maViewSize(1000, 1000),
      maPrevSize(-1,-1),
      mnMinZoom(MIN_ZOOM),
      mnMaxZoom(MAX_ZOOM),
      mbMinZoomAutoCalc(false),
      mbCenterAllowed(true),
      mnTicks (0),
      mpViewShell(nullptr),
      mbUseDropScroll (true)
{
    SetDialogControlFlags( DialogControlFlags::Return | DialogControlFlags::WantFocus );

    MapMode aMap(GetMapMode());
    aMap.SetMapUnit(MapUnit::Map100thMM);
    SetMapMode(aMap);

    // with it, the vcl::WindowColor is used in the slide mode
    SetBackground( Wallpaper( GetSettings().GetStyleSettings().GetWindowColor() ) );

    // adjust contrast mode initially
    bool bUseContrast = GetSettings().GetStyleSettings().GetHighContrastMode();
    SetDrawMode( bUseContrast
        ? sd::OUTPUT_DRAWMODE_CONTRAST
        : sd::OUTPUT_DRAWMODE_COLOR );

    // #i78183# Added after discussed with AF
    EnableRTL(false);
}

Window::~Window()
{
    disposeOnce();
}

void Window::dispose()
{
    if (mpViewShell != nullptr)
    {
        WindowUpdater* pWindowUpdater = mpViewShell->GetWindowUpdater();
        if (pWindowUpdater != nullptr)
            pWindowUpdater->UnregisterWindow (this);
    }
    DropTargetHelper::dispose();
    vcl::Window::dispose();
}

void Window::SetViewShell (ViewShell* pViewSh)
{
    WindowUpdater* pWindowUpdater = nullptr;
    // Unregister at device updater of old view shell.
    if (mpViewShell != nullptr)
    {
        pWindowUpdater = mpViewShell->GetWindowUpdater();
        if (pWindowUpdater != nullptr)
            pWindowUpdater->UnregisterWindow (this);
    }

    mpViewShell = pViewSh;

    // Register at device updater of new view shell
    if (mpViewShell != nullptr)
    {
        pWindowUpdater = mpViewShell->GetWindowUpdater();
        if (pWindowUpdater != nullptr)
            pWindowUpdater->RegisterWindow (this);
    }
}

ViewShell* Window::GetViewShell()
{
    return mpViewShell;
}

void Window::CalcMinZoom()
{
    // Are we entitled to change the minimal zoom factor?
    if ( !mbMinZoomAutoCalc )
        return;

    // Get current zoom factor.
    ::tools::Long nZoom = GetZoom();

    // Get the rectangle of the output area in logical coordinates
    // and calculate the scaling factors that would lead to the view
    // area (also called application area) to completely fill the
    // window.
    Size aWinSize = PixelToLogic(GetOutputSizePixel());
    sal_uLong nX = static_cast<sal_uLong>(static_cast<double>(aWinSize.Width())
        * double(ZOOM_MULTIPLICATOR) / static_cast<double>(maViewSize.Width()));
    sal_uLong nY = static_cast<sal_uLong>(static_cast<double>(aWinSize.Height())
        * double(ZOOM_MULTIPLICATOR) / static_cast<double>(maViewSize.Height()));

    // Decide whether to take the larger or the smaller factor.
    sal_uLong nFact = std::min(nX, nY);

    // The factor is transformed according to the current zoom factor.
    nFact = nFact * nZoom / ZOOM_MULTIPLICATOR;
    mnMinZoom = std::max(sal_uInt16(MIN_ZOOM), static_cast<sal_uInt16>(nFact));

    // If the current zoom factor is smaller than the calculated minimal
    // zoom factor then set the new minimal factor as the current zoom
    // factor.
    if ( nZoom < static_cast<::tools::Long>(mnMinZoom) )
        SetZoomFactor(mnMinZoom);
}

void Window::SetMinZoom (::tools::Long nMin)
{
    mnMinZoom = static_cast<sal_uInt16>(nMin);
}

void Window::SetMaxZoom (::tools::Long nMax)
{
    mnMaxZoom = static_cast<sal_uInt16>(nMax);
}

::tools::Long Window::GetZoom() const
{
    if( GetMapMode().GetScaleX().GetDenominator() )
    {
        return ::tools::Long(GetMapMode().GetScaleX() * 100);
    }
    else
    {
        return 0;
    }
}

void Window::Resize()
{
    vcl::Window::Resize();
    CalcMinZoom();

    if( mpViewShell && mpViewShell->GetViewFrame() )
        mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_ATTR_ZOOMSLIDER );
}

void Window::PrePaint(vcl::RenderContext& /*rRenderContext*/)
{
    if ( mpViewShell )
        mpViewShell->PrePaint();
}

void Window::Paint(vcl::RenderContext& /*rRenderContext*/, const ::tools::Rectangle& rRect)
{
    if ( mpViewShell )
        mpViewShell->Paint(rRect, this);
}

void Window::KeyInput(const KeyEvent& rKEvt)
{
    if (getenv("SD_DEBUG") && rKEvt.GetKeyCode().GetCode() == KEY_F12 && mpViewShell)
    {
        mpViewShell->GetDoc()->dumpAsXml(nullptr);
        if (OutlinerView *pOLV = mpViewShell->GetView()->GetTextEditOutlinerView())
            pOLV->GetEditView().GetEditEngine()->dumpAsXmlEditDoc(nullptr);
        return;
    }

    if (!(mpViewShell && mpViewShell->KeyInput(rKEvt, this)))
    {
        if (mpViewShell && rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE)
        {
            mpViewShell->GetViewShell()->Escape();
        }
        else
        {
            vcl::Window::KeyInput(rKEvt);
        }
    }
}

void Window::MouseButtonDown(const MouseEvent& rMEvt)
{
    if ( mpViewShell )
        mpViewShell->MouseButtonDown(rMEvt, this);
}

void Window::MouseMove(const MouseEvent& rMEvt)
{
    if ( mpViewShell )
        mpViewShell->MouseMove(rMEvt, this);
}

void Window::MouseButtonUp(const MouseEvent& rMEvt)
{
    mnTicks = 0;

    if ( mpViewShell )
        mpViewShell->MouseButtonUp(rMEvt, this);
}

void Window::Command(const CommandEvent& rCEvt)
{
    if (mpViewShell)
        mpViewShell->Command(rCEvt, this);
    //pass at least alt press/release to parent impl
    if (rCEvt.GetCommand() == CommandEventId::ModKeyChange)
        vcl::Window::Command(rCEvt);
    //show the text edit outliner view cursor
    else if (mpViewShell && !HasFocus() && rCEvt.GetCommand() == CommandEventId::CursorPos)
    {
        // tdf#138855 Getting Focus may destroy TextEditOutlinerView so Grab if
        // text editing active, but fetch the TextEditOutlinerView post-grab
        if (mpViewShell->GetView()->IsTextEdit())
        {
            GrabFocus();
            OutlinerView* pOLV = mpViewShell->GetView()->GetTextEditOutlinerView();
            if (pOLV && this == pOLV->GetWindow())
                pOLV->ShowCursor();
        }
    }
}

bool Window::EventNotify( NotifyEvent& rNEvt )
{
    bool bResult = false;
    if ( mpViewShell )
    {
        bResult = mpViewShell->Notify(rNEvt, this);
    }
    if( !bResult )
        bResult = vcl::Window::EventNotify(rNEvt);

    return bResult;
}

void Window::RequestHelp(const HelpEvent& rEvt)
{
    if (!mpViewShell || !mpViewShell->RequestHelp(rEvt))
        vcl::Window::RequestHelp( rEvt );
}

/**
 * Set the position of the upper left corner from the visible area of the
 * window.
 */
void Window::SetWinViewPos(const Point& rPnt)
{
    maWinPos = rPnt;
}

/**
 * Set origin of the representation in respect to the whole working area.
 */
void Window::SetViewOrigin(const Point& rPnt)
{
    maViewOrigin = rPnt;
}

/**
 * Set size of the whole working area which can be seen with the window.
 */
void Window::SetViewSize(const Size& rSize)
{
    maViewSize = rSize;
    CalcMinZoom();
}

void Window::SetCenterAllowed (bool bIsAllowed)
{
    mbCenterAllowed = bIsAllowed;
}

::tools::Long Window::SetZoomFactor(::tools::Long nZoom)
{
    // Clip the zoom factor to the valid range marked by nMinZoom as
    // calculated by CalcMinZoom() and the constant MAX_ZOOM.
    if ( nZoom > MAX_ZOOM )
        nZoom = MAX_ZOOM;
    if ( nZoom < static_cast<::tools::Long>(mnMinZoom) )
        nZoom = mnMinZoom;

    // Set the zoom factor at the window's map mode.
    if (!comphelper::LibreOfficeKit::isActive())
    {
        MapMode aMap(GetMapMode());
        aMap.SetScaleX(Fraction(nZoom, 100));
        aMap.SetScaleY(Fraction(nZoom, 100));
        SetMapMode(aMap);
    }

    // invalidate previous size - it was relative to the old scaling
    maPrevSize = Size(-1,-1);

    // Update the map mode's origin (to what effect?).
    UpdateMapOrigin();

    // Update the view's snapping to the new zoom factor.
    if ( auto pDrawViewShell = dynamic_cast< DrawViewShell *>( mpViewShell ) )
        pDrawViewShell->GetView()->RecalcLogicSnapMagnetic(*this);

    // Return the zoom factor just in case it has been changed above to lie
    // inside the valid range.
    return nZoom;
}

void Window::SetZoomIntegral(::tools::Long nZoom)
{
    // Clip the zoom factor to the valid range marked by nMinZoom as
    // previously calculated by <member>CalcMinZoom()</member> and the
    // MAX_ZOOM constant.
    if ( nZoom > MAX_ZOOM )
        nZoom = MAX_ZOOM;
    if ( nZoom < static_cast<::tools::Long>(mnMinZoom) )
        nZoom = mnMinZoom;

    // Calculate the window's new origin.
    Size aSize = PixelToLogic(GetOutputSizePixel());
    ::tools::Long nW = aSize.Width()  * GetZoom() / nZoom;
    ::tools::Long nH = aSize.Height() * GetZoom() / nZoom;
    maWinPos.AdjustX((aSize.Width()  - nW) / 2 );
    maWinPos.AdjustY((aSize.Height() - nH) / 2 );
    if ( maWinPos.X() < 0 ) maWinPos.setX( 0 );
    if ( maWinPos.Y() < 0 ) maWinPos.setY( 0 );

    // Finally update this window's map mode to the given zoom factor that
    // has been clipped to the valid range.
    SetZoomFactor(nZoom);
}

::tools::Long Window::GetZoomForRect( const ::tools::Rectangle& rZoomRect )
{
    ::tools::Long nRetZoom = 100;

    if( (rZoomRect.GetWidth() != 0) && (rZoomRect.GetHeight() != 0))
    {
        // Calculate the scale factors which will lead to the given
        // rectangle being fully visible (when translated accordingly) as
        // large as possible in the output area independently in both
        // coordinate directions .
        sal_uLong nX(0);
        sal_uLong nY(0);

        const Size aWinSize( PixelToLogic(GetOutputSizePixel()) );
        if(rZoomRect.GetHeight())
        {
            nX = static_cast<sal_uLong>(static_cast<double>(aWinSize.Height())
               * double(ZOOM_MULTIPLICATOR) / static_cast<double>(rZoomRect.GetHeight()));
        }

        if(rZoomRect.GetWidth())
        {
            nY = static_cast<sal_uLong>(static_cast<double>(aWinSize.Width())
                * double(ZOOM_MULTIPLICATOR) / static_cast<double>(rZoomRect.GetWidth()));
        }

        // Use the smaller one of both so that the zoom rectangle will be
        // fully visible with respect to both coordinate directions.
        sal_uLong nFact = std::min(nX, nY);

        // Transform the current zoom factor so that it leads to the desired
        // scaling.
        nRetZoom = nFact * GetZoom() / ZOOM_MULTIPLICATOR;

        // Calculate the new origin.
        if ( nFact == 0 )
        {
            // Don't change anything if the scale factor is degenerate.
            nRetZoom = GetZoom();
        }
        else
        {
            // Clip the zoom factor to the valid range marked by nMinZoom as
            // previously calculated by <member>CalcMinZoom()</member> and the
            // MAX_ZOOM constant.
            if ( nRetZoom > MAX_ZOOM )
                nRetZoom = MAX_ZOOM;
            if ( nRetZoom < static_cast<::tools::Long>(mnMinZoom) )
                nRetZoom = mnMinZoom;
       }
    }

    return nRetZoom;
}

/** Recalculate the zoom factor and translation so that the given rectangle
    is displayed centered and as large as possible while still being fully
    visible in the window.
*/
::tools::Long Window::SetZoomRect (const ::tools::Rectangle& rZoomRect)
{
    ::tools::Long nNewZoom = 100;

    if (rZoomRect.GetWidth() == 0 || rZoomRect.GetHeight() == 0)
    {
        // The given rectangle is degenerate.  Use the default zoom factor
        // (above) of 100%.
        SetZoomIntegral(nNewZoom);
    }
    else
    {
        Point aPos = rZoomRect.TopLeft();
        // Transform the output area from pixel coordinates into logical
        // coordinates.
        Size aWinSize = PixelToLogic(GetOutputSizePixel());
        // Paranoia!  The degenerate case of zero width or height has been
        // taken care of above.
        DBG_ASSERT(rZoomRect.GetWidth(), "ZoomRect-Width = 0!");
        DBG_ASSERT(rZoomRect.GetHeight(), "ZoomRect-Height = 0!");

        // Calculate the scale factors which will lead to the given
        // rectangle being fully visible (when translated accordingly) as
        // large as possible in the output area independently in both
        // coordinate directions .
        sal_uLong nX(0);
        sal_uLong nY(0);

        if(rZoomRect.GetHeight())
        {
            nX = static_cast<sal_uLong>(static_cast<double>(aWinSize.Height())
               * double(ZOOM_MULTIPLICATOR) / static_cast<double>(rZoomRect.GetHeight()));
        }

        if(rZoomRect.GetWidth())
        {
            nY = static_cast<sal_uLong>(static_cast<double>(aWinSize.Width())
                * double(ZOOM_MULTIPLICATOR) / static_cast<double>(rZoomRect.GetWidth()));
        }

        // Use the smaller one of both so that the zoom rectangle will be
        // fully visible with respect to both coordinate directions.
        sal_uLong nFact = std::min(nX, nY);

        // Transform the current zoom factor so that it leads to the desired
        // scaling.
        ::tools::Long nZoom = nFact * GetZoom() / ZOOM_MULTIPLICATOR;

        // Calculate the new origin.
        if ( nFact == 0 )
        {
            // Don't change anything if the scale factor is degenerate.
            nNewZoom = GetZoom();
        }
        else
        {
            // Calculate the new window position that centers the given
            // rectangle on the screen.
            if ( nZoom > MAX_ZOOM )
                nFact = nFact * MAX_ZOOM / nZoom;

            maWinPos = maViewOrigin + aPos;

            aWinSize.setWidth( static_cast<::tools::Long>(static_cast<double>(aWinSize.Width()) * double(ZOOM_MULTIPLICATOR) / static_cast<double>(nFact)) );
            maWinPos.AdjustX((rZoomRect.GetWidth() - aWinSize.Width()) / 2 );
            aWinSize.setHeight( static_cast<::tools::Long>(static_cast<double>(aWinSize.Height()) * double(ZOOM_MULTIPLICATOR) / static_cast<double>(nFact)) );
            maWinPos.AdjustY((rZoomRect.GetHeight() - aWinSize.Height()) / 2 );

            if ( maWinPos.X() < 0 ) maWinPos.setX( 0 );
            if ( maWinPos.Y() < 0 ) maWinPos.setY( 0 );

            // Adapt the window's map mode to the new zoom factor.
            nNewZoom = SetZoomFactor(nZoom);
        }
    }

    return nNewZoom;
}

void Window::SetMinZoomAutoCalc (bool bAuto)
{
    mbMinZoomAutoCalc = bAuto;
}

/**
 * Calculate and set new MapMode origin.
 * If aWinPos.X()/Y() == -1, then we center the corresponding position (e.g. for
 * initialization).
 */
void Window::UpdateMapOrigin(bool bInvalidate)
{
    bool       bChanged = false;
    const Size aWinSize = PixelToLogic(GetOutputSizePixel());

    if ( mbCenterAllowed )
    {
        if( maPrevSize != Size(-1,-1) )
        {
            // keep view centered around current pos, when window
            // resizes
            maWinPos.AdjustX( -((aWinSize.Width() - maPrevSize.Width()) / 2) );
            maWinPos.AdjustY( -((aWinSize.Height() - maPrevSize.Height()) / 2) );
            bChanged = true;
        }

        if ( maWinPos.X() > maViewSize.Width() - aWinSize.Width() )
        {
            maWinPos.setX( maViewSize.Width() - aWinSize.Width() );
            bChanged = true;
        }
        if ( maWinPos.Y() > maViewSize.Height() - aWinSize.Height() )
        {
            maWinPos.setY( maViewSize.Height() - aWinSize.Height() );
            bChanged = true;
        }
        if ( aWinSize.Width() > maViewSize.Width() || maWinPos.X() < 0 )
        {
            maWinPos.setX( maViewSize.Width()  / 2 - aWinSize.Width()  / 2 );
            bChanged = true;
        }
        if ( aWinSize.Height() > maViewSize.Height() || maWinPos.Y() < 0 )
        {
            maWinPos.setY( maViewSize.Height() / 2 - aWinSize.Height() / 2 );
            bChanged = true;
        }
    }

    UpdateMapMode ();

    maPrevSize = aWinSize;

    // When tiled rendering, the above UpdateMapMode() call doesn't touch the map mode.
    if (bChanged && bInvalidate && !comphelper::LibreOfficeKit::isActive())
        Invalidate();
}

void Window::UpdateMapMode()
{
    maWinPos -= maViewOrigin;
    Size aPix(maWinPos.X(), maWinPos.Y());
    aPix = LogicToPixel(aPix);
    // Size has to be a multiple of BRUSH_SIZE due to the correct depiction of
    // pattern
    // #i2237#
    // removed old stuff here which still forced zoom to be
    // %BRUSH_SIZE which is outdated now

    if (dynamic_cast< DrawViewShell *>( mpViewShell ))
    {
        // page should not "stick" to the window border
        if (aPix.Width() == 0)
        {
            // #i2237#
            // Since BRUSH_SIZE alignment is outdated now, i use the
            // former constant here directly
            aPix.AdjustWidth( -8 );
        }
        if (aPix.Height() == 0)
        {
            // #i2237#
            // Since BRUSH_SIZE alignment is outdated now, i use the
            // former constant here directly
            aPix.AdjustHeight( -8 );
        }
    }

    aPix = PixelToLogic(aPix);
    maWinPos.setX( aPix.Width() );
    maWinPos.setY( aPix.Height() );
    Point aNewOrigin (-maWinPos.X(), -maWinPos.Y());
    maWinPos += maViewOrigin;

    if (!comphelper::LibreOfficeKit::isActive())
    {
        MapMode aMap(GetMapMode());
        aMap.SetOrigin(aNewOrigin);
        SetMapMode(aMap);
    }
}

/**
 * @returns X position of the visible area as fraction (< 1) of the whole
 * working area.
 */
double Window::GetVisibleX() const
{
    return (static_cast<double>(maWinPos.X()) / maViewSize.Width());
}

/**
 * @returns Y position of the visible area as fraction (< 1) of the whole
 * working area.
 */
double Window::GetVisibleY() const
{
    return (static_cast<double>(maWinPos.Y()) / maViewSize.Height());
}

/**
 * Set x and y position of the visible area as fraction (< 1) of the whole
 * working area. Negative values are ignored.
 */
void Window::SetVisibleXY(double fX, double fY)
{
    ::tools::Long nOldX = maWinPos.X();
    ::tools::Long nOldY = maWinPos.Y();

    if ( fX >= 0 )
        maWinPos.setX( static_cast<::tools::Long>(fX * maViewSize.Width()) );
    if ( fY >= 0 )
        maWinPos.setY( static_cast<::tools::Long>(fY * maViewSize.Height()) );
    UpdateMapOrigin(false);
    Scroll(nOldX - maWinPos.X(), nOldY - maWinPos.Y(), ScrollFlags::Children);
    PaintImmediately();
}

/**
 * @returns width of the visible area in proportion to the width of the whole
 * working area.
 */
double Window::GetVisibleWidth() const
{
    Size aWinSize = PixelToLogic(GetOutputSizePixel());
    if ( aWinSize.Width() > maViewSize.Width() )
        aWinSize.setWidth( maViewSize.Width() );
    return (static_cast<double>(aWinSize.Width()) / maViewSize.Width());
}

/**
 * @returns height of the visible area in proportion to the height of the whole
 * working area.
 */
double Window::GetVisibleHeight() const
{
    Size aWinSize = PixelToLogic(GetOutputSizePixel());
    if ( aWinSize.Height() > maViewSize.Height() )
        aWinSize.setHeight( maViewSize.Height() );
    return (static_cast<double>(aWinSize.Height()) / maViewSize.Height());
}

Point Window::GetVisibleCenter()
{
    Point aPos = ::tools::Rectangle(Point(), GetOutputSizePixel()).Center();

    // For LOK
    bool bMapModeWasEnabled(IsMapModeEnabled());
    EnableMapMode(/*true*/);
    aPos = PixelToLogic(aPos);
    EnableMapMode(bMapModeWasEnabled);

    return aPos;
}

/**
 * @returns width of a scroll column in proportion to the width of the whole
 * working area.
 */
double Window::GetScrlLineWidth() const
{
    return (GetVisibleWidth() * SCROLL_LINE_FACT);
}

/**
 * @returns height of a scroll column in proportion to the height of the whole
 * working area.
 */
double Window::GetScrlLineHeight() const
{
    return (GetVisibleHeight() * SCROLL_LINE_FACT);
}

/**
 * @returns width of a scroll page in proportion to the width of the whole
 * working area.
 */
double Window::GetScrlPageWidth() const
{
    return (GetVisibleWidth() * SCROLL_PAGE_FACT);
}

/**
 * @returns height of a scroll page in proportion to the height of the whole
 * working area.
 */
double Window::GetScrlPageHeight() const
{
    return (GetVisibleHeight() * SCROLL_PAGE_FACT);
}

/**
 * Deactivate window.
 */
void Window::LoseFocus()
{
    mnTicks = 0;
    vcl::Window::LoseFocus ();
}

/**
 * Activate window.
 */
void Window::GrabFocus()
{
    mnTicks      = 0;
    vcl::Window::GrabFocus ();
}

void Window::DataChanged( const DataChangedEvent& rDCEvt )
{
    vcl::Window::DataChanged( rDCEvt );

    /* Omit PRINTER by all documents which are not using a printer.
       Omit FONTS and FONTSUBSTITUTION if no text output is available or if the
       document does not allow text.  */

    if ( !((rDCEvt.GetType() == DataChangedEventType::PRINTER) ||
         (rDCEvt.GetType() == DataChangedEventType::DISPLAY) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
          (rDCEvt.GetFlags() & AllSettingsFlags::STYLE))) )
        return;

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        /* Rearrange or initiate Resize for scroll bars since the size of
           the scroll bars my have changed. Within this, inside the resize-
           handler, the size of the scroll bars will be asked from the
           Settings. */
        Resize();

        /* Re-set data, which are from system control or from Settings. May
           have to re-set more data since the resolution may also has
           changed. */
        if( mpViewShell )
        {
            const StyleSettings&    rStyleSettings = GetSettings().GetStyleSettings();
            SvtAccessibilityOptions aAccOptions;
            DrawModeFlags           nOutputMode;
            sal_uInt16              nPreviewSlot;

            if( rStyleSettings.GetHighContrastMode() )
                nOutputMode = sd::OUTPUT_DRAWMODE_CONTRAST;
            else
                nOutputMode = sd::OUTPUT_DRAWMODE_COLOR;

            if( rStyleSettings.GetHighContrastMode() && aAccOptions.GetIsForPagePreviews() )
                nPreviewSlot = SID_PREVIEW_QUALITY_CONTRAST;
            else
                nPreviewSlot = SID_PREVIEW_QUALITY_COLOR;

            if( dynamic_cast< DrawViewShell *>( mpViewShell ) !=  nullptr )
            {
                SetDrawMode( nOutputMode );
                mpViewShell->GetFrameView()->SetDrawMode( nOutputMode );
                Invalidate();
            }

            // Overwrite window color for OutlineView
            if( dynamic_cast< OutlineViewShell *>( mpViewShell ) !=  nullptr )
            {
                svtools::ColorConfig aColorConfig;
                const Color aDocColor( aColorConfig.GetColorValue( svtools::DOCCOLOR ).nColor );
                SetBackground( Wallpaper( aDocColor ) );
            }

            SfxRequest aReq( nPreviewSlot, SfxCallMode::SLOT, mpViewShell->GetDocSh()->GetDoc()->GetItemPool() );
            mpViewShell->ExecReq( aReq );
            mpViewShell->Invalidate();
            mpViewShell->ArrangeGUIElements();

            // re-create handles to show new outfit
            if(dynamic_cast< DrawViewShell *>( mpViewShell ) !=  nullptr)
            {
                mpViewShell->GetView()->AdjustMarkHdl();
            }
        }
    }

    if ( (rDCEvt.GetType() == DataChangedEventType::DISPLAY) ||
         ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
          (rDCEvt.GetFlags() & AllSettingsFlags::STYLE)) )
    {
        /* Virtual devices, which also depends on the resolution or the
           system control, should be updated. Otherwise, we should update
           the virtual devices at least at DataChangedEventType::DISPLAY since some
           systems allow to change the resolution and color depth during
           runtime. Or the virtual devices have to be updated when the color
           palette has changed since a different color matching can be used
           when outputting. */
    }

    if ( rDCEvt.GetType() == DataChangedEventType::FONTS )
    {
        /* If the document provides font choose boxes, we have to update
           them. I don't know how this looks like (also not really me, I
           only translated the comment ;). We may can handle it global. We
           have to discuss it with PB, but he is ill at the moment.
           Before we handle it here, discuss it with PB and me. */
    }

    if ( (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) )
    {
        /* Do reformatting since the fonts of the document may no longer
           exist, or exist now, or are replaced with others. */
        if( mpViewShell )
        {
            DrawDocShell* pDocSh = mpViewShell->GetDocSh();
            if( pDocSh )
                pDocSh->SetPrinter( pDocSh->GetPrinter( true ) );
        }
    }

    if ( rDCEvt.GetType() == DataChangedEventType::PRINTER )
    {
        /* I don't know how the handling should look like. Maybe we delete a
           printer and look what we have to do. Maybe I have to add
           something to the VCL, in case the used printer is deleted.
           Otherwise I may recalculate the formatting here if the current
           printer is destroyed. */
        if( mpViewShell )
        {
            DrawDocShell* pDocSh = mpViewShell->GetDocSh();
            if( pDocSh )
                pDocSh->SetPrinter( pDocSh->GetPrinter( true ) );
        }
    }

    // Update everything
    Invalidate();
}

sal_Int8 Window::AcceptDrop( const AcceptDropEvent& rEvt )
{
    sal_Int8 nRet = DND_ACTION_NONE;

    if( mpViewShell && !mpViewShell->GetDocSh()->IsReadOnly() )
    {
        nRet = mpViewShell->AcceptDrop( rEvt, *this, this, SDRPAGE_NOTFOUND, SDRLAYER_NOTFOUND );

        if (mbUseDropScroll && dynamic_cast< OutlineViewShell *>( mpViewShell ) ==  nullptr)
            DropScroll( rEvt.maPosPixel );
    }

    return nRet;
}

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

void Window::DropScroll(const Point& rMousePos)
{
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

css::uno::Reference<css::accessibility::XAccessible>
    Window::CreateAccessible()
{
    // If current viewshell is PresentationViewShell, just return empty because the correct ShowWin will be created later.
    if (dynamic_cast< PresentationViewShell *>( mpViewShell ))
    {
        return vcl::Window::CreateAccessible ();
    }
    css::uno::Reference< css::accessibility::XAccessible > xAcc = GetAccessible(false);
    if (xAcc)
    {
        return xAcc;
    }
    if (mpViewShell != nullptr)
    {
        xAcc = mpViewShell->CreateAccessibleDocumentView (this);
        SetAccessible(xAcc);
        return xAcc;
    }
    else
    {
        SAL_WARN("sd", "::sd::Window::CreateAccessible: no view shell");
        return vcl::Window::CreateAccessible ();
    }
}

OutlinerView* Window::GetOutlinerView() const
{
    OutlinerView *pOLV = nullptr;
    sd::View* pView = mpViewShell->GetView();
    if (mpViewShell->GetShellType() == ViewShell::ST_OUTLINE)
    {
        if (OutlineView* pOView = dynamic_cast<OutlineView*>(pView))
            pOLV = pOView->GetViewByWindow(this);
    }
    else if (pView->IsTextEdit())
    {
        pOLV = pView->GetTextEditOutlinerView();
    }
    return pOLV;
}

OUString Window::GetSurroundingText() const
{
    OutlinerView *pOLV = GetOutlinerView();
    if (pOLV)
        return pOLV->GetEditView().GetSurroundingText();
    return OUString();
}

Selection Window::GetSurroundingTextSelection() const
{
    OutlinerView *pOLV = GetOutlinerView();
    if (pOLV)
        return pOLV->GetEditView().GetSurroundingTextSelection();
    return Selection( 0, 0 );
}

bool Window::DeleteSurroundingText(const Selection& rSelection)
{
    OutlinerView *pOLV = GetOutlinerView();
    if (pOLV)
        return pOLV->GetEditView().DeleteSurroundingText(rSelection);
    return false;
}

void Window::LogicInvalidate(const ::tools::Rectangle* pRectangle)
{
    DrawViewShell* pDrawViewShell = dynamic_cast<DrawViewShell*>(mpViewShell);
    if (!pDrawViewShell || pDrawViewShell->IsInSwitchPage())
        return;

    if (!comphelper::LibreOfficeKit::isActive())
        return;
    OString sRectangle;
    if (!pRectangle)
        sRectangle = "EMPTY";
    else
    {
        ::tools::Rectangle aRectangle(*pRectangle);
        if (GetMapMode().GetMapUnit() == MapUnit::Map100thMM)
            aRectangle = OutputDevice::LogicToLogic(aRectangle, MapMode(MapUnit::Map100thMM), MapMode(MapUnit::MapTwip));
        sRectangle = aRectangle.toString();
    }
    SfxViewShell& rSfxViewShell = pDrawViewShell->GetViewShellBase();
    SfxLokHelper::notifyInvalidation(&rSfxViewShell, sRectangle);
}

void Window::LogicMouseButtonDown(const MouseEvent& rMouseEvent)
{
    // When we're not doing tiled rendering, then positions must be passed as pixels.
    assert(comphelper::LibreOfficeKit::isActive());

    Point aPoint = GetPointerPosPixel();
    SetLastMousePos(rMouseEvent.GetPosPixel());

    mpViewShell->MouseButtonDown(rMouseEvent, this);

    SetPointerPosPixel(aPoint);
}

void Window::LogicMouseButtonUp(const MouseEvent& rMouseEvent)
{
    // When we're not doing tiled rendering, then positions must be passed as pixels.
    assert(comphelper::LibreOfficeKit::isActive());

    Point aPoint = GetPointerPosPixel();
    SetLastMousePos(rMouseEvent.GetPosPixel());

    mpViewShell->MouseButtonUp(rMouseEvent, this);

    SetPointerPosPixel(aPoint);
}

void Window::LogicMouseMove(const MouseEvent& rMouseEvent)
{
    // When we're not doing tiled rendering, then positions must be passed as pixels.
    assert(comphelper::LibreOfficeKit::isActive());

    Point aPoint = GetPointerPosPixel();
    SetLastMousePos(rMouseEvent.GetPosPixel());

    mpViewShell->MouseMove(rMouseEvent, this);

    SetPointerPosPixel(aPoint);
}

FactoryFunction Window::GetUITestFactory() const
{
    if (get_id() == "impress_win")
        return ImpressWindowUIObject::create;

    return WindowUIObject::create;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
