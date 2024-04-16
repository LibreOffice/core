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

#include <sal/config.h>

#include <o3tl/unit_conversion.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <osl/diagnose.h>

#include <output.hxx>
#include <drwlayer.hxx>
#include <document.hxx>
#include <tabvwsh.hxx>

#include <svx/fmview.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/sdrpagewindow.hxx>

// #i72502#
Point ScOutputData::PrePrintDrawingLayer(tools::Long nLogStX, tools::Long nLogStY )
{
    tools::Rectangle aRect;
    SCCOL nCol;
    Point aOffset;
    tools::Long nLayoutSign(bLayoutRTL ? -1 : 1);

    for (nCol=0; nCol<nX1; nCol++)
        aOffset.AdjustX( -(mpDoc->GetColWidth( nCol, nTab ) * nLayoutSign) );
    aOffset.AdjustY( -sal_Int32(mpDoc->GetRowHeight( 0, nY1-1, nTab )) );

    tools::Long nDataWidth = 0;
    for (nCol=nX1; nCol<=nX2; nCol++)
        nDataWidth += mpDoc->GetColWidth( nCol, nTab );

    if ( bLayoutRTL )
        aOffset.AdjustX(nDataWidth );

    aRect.SetLeft( -aOffset.X() );
    aRect.SetRight( -aOffset.X() );
    aRect.SetTop( -aOffset.Y() );
    aRect.SetBottom( -aOffset.Y() );

    Point aMMOffset( aOffset );
    aMMOffset.setX(o3tl::convert(aMMOffset.X(), o3tl::Length::twip, o3tl::Length::mm100));
    aMMOffset.setY(o3tl::convert(aMMOffset.Y(), o3tl::Length::twip, o3tl::Length::mm100));

    if (!bMetaFile)
        aMMOffset += Point( nLogStX, nLogStY );

    for (nCol=nX1; nCol<=nX2; nCol++)
        aRect.AdjustRight(mpDoc->GetColWidth( nCol, nTab ) );
    aRect.AdjustBottom(mpDoc->GetRowHeight( nY1, nY2, nTab ) );

    aRect.SetLeft(o3tl::convert(aRect.Left(), o3tl::Length::twip, o3tl::Length::mm100));
    aRect.SetTop(o3tl::convert(aRect.Top(), o3tl::Length::twip, o3tl::Length::mm100));
    aRect.SetRight(o3tl::convert(aRect.Right(), o3tl::Length::twip, o3tl::Length::mm100));
    aRect.SetBottom(o3tl::convert(aRect.Bottom(), o3tl::Length::twip, o3tl::Length::mm100));

    if(pViewShell || pDrawView)
    {
        SdrView* pLocalDrawView = pDrawView ? pDrawView : pViewShell->GetScDrawView();

        if(pLocalDrawView)
        {
            // #i76114# MapMode has to be set because BeginDrawLayers uses GetPaintRegion
            MapMode aOldMode = mpDev->GetMapMode();
            if (!bMetaFile)
                mpDev->SetMapMode( MapMode( MapUnit::Map100thMM, aMMOffset, aOldMode.GetScaleX(), aOldMode.GetScaleY() ) );

            // #i74769# work with SdrPaintWindow directly
            // #i76114# pass bDisableIntersect = true, because the intersection of the table area
            // with the Window's paint region can be empty
            vcl::Region aRectRegion(aRect);
            mpTargetPaintWindow = pLocalDrawView->BeginDrawLayers(mpDev, aRectRegion, true);
            OSL_ENSURE(mpTargetPaintWindow, "BeginDrawLayers: Got no SdrPaintWindow (!)");

            if (!bMetaFile)
                mpDev->SetMapMode( aOldMode );
        }
    }

    return aMMOffset;
}

// #i72502#
void ScOutputData::PostPrintDrawingLayer(const Point& rMMOffset) // #i74768#
{
    // #i74768# just use offset as in PrintDrawingLayer() to also get the form controls
    // painted with offset
    MapMode aOldMode = mpDev->GetMapMode();

    if (!bMetaFile)
    {
        mpDev->SetMapMode( MapMode( MapUnit::Map100thMM, rMMOffset, aOldMode.GetScaleX(), aOldMode.GetScaleY() ) );
    }

    if(pViewShell || pDrawView)
    {
        SdrView* pLocalDrawView = pDrawView ? pDrawView : pViewShell->GetScDrawView();

        if(pLocalDrawView)
        {
            // #i74769# work with SdrPaintWindow directly
            pLocalDrawView->EndDrawLayers(*mpTargetPaintWindow, true);
            mpTargetPaintWindow = nullptr;
        }
    }

    // #i74768#
    if (!bMetaFile)
    {
        mpDev->SetMapMode( aOldMode );
    }
}

// #i72502#
void ScOutputData::PrintDrawingLayer(SdrLayerID nLayer, const Point& rMMOffset)
{
    bool bHideAllDrawingLayer(false);

    if(pViewShell || pDrawView)
    {
        SdrView* pLocalDrawView = pDrawView ? pDrawView : pViewShell->GetScDrawView();

        if(pLocalDrawView)
        {
            bHideAllDrawingLayer = pLocalDrawView->getHideOle() && pLocalDrawView->getHideChart()
                    && pLocalDrawView->getHideDraw() && pLocalDrawView->getHideFormControl();
        }
    }

    if(bHideAllDrawingLayer || (!mpDoc->GetDrawLayer()))
    {
        return;
    }

    MapMode aOldMode = mpDev->GetMapMode();

    if (!bMetaFile)
    {
        mpDev->SetMapMode( MapMode( MapUnit::Map100thMM, rMMOffset, aOldMode.GetScaleX(), aOldMode.GetScaleY() ) );
    }

    DrawSelectiveObjects( nLayer );

    if (!bMetaFile)
    {
        mpDev->SetMapMode( aOldMode );
    }
}

void ScOutputData::DrawSelectiveObjects(SdrLayerID nLayer)
{
    ScDrawLayer* pModel = mpDoc->GetDrawLayer();
    if (!pModel)
        return;

    //  #i46362# high contrast mode (and default text direction) must be handled
    //  by the application, so it's still needed when using DrawLayer().

    SdrOutliner& rOutl = pModel->GetDrawOutliner();
    rOutl.EnableAutoColor( mbUseStyleColor );
    rOutl.SetDefaultHorizontalTextDirection(
                mpDoc->GetEditTextDirection( nTab ) );

    //  #i69767# The hyphenator must be set (used to be before drawing a text shape with hyphenation).
    //  LinguMgr::GetHyphenator (EditEngine) uses a wrapper now that creates the real hyphenator on demand,
    //  so it's not a performance problem to call UseHyphenator even when it's not needed.

    pModel->UseHyphenator();

    DrawModeFlags nOldDrawMode = mpDev->GetDrawMode();
    if ( mbUseStyleColor && Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
    {
        mpDev->SetDrawMode( nOldDrawMode | DrawModeFlags::SettingsLine | DrawModeFlags::SettingsFill |
                            DrawModeFlags::SettingsText | DrawModeFlags::SettingsGradient );
    }

    if(pViewShell || pDrawView)
    {
        SdrView* pLocalDrawView = pDrawView ? pDrawView : pViewShell->GetScDrawView();

        if(pLocalDrawView)
        {
            SdrPageView* pPageView = pLocalDrawView->GetSdrPageView();

            if(pPageView)
            {
                // tdf#160589 need to check for registered PaintWindow using the
                // 'original' TragetDevice, mpDev might have been changed by a
                // call to ::SetContentDevice. That again might patch in a
                // pre-render device fetched from SdrPaintWindow::GetTargetOutputDevice
                // and thus the test if target is aregistered PageWindow would fail
                assert(nullptr != mpOriginalTargetDevice && "mpOriginalTargetDevice *must* be set when constructing ScOutputData (!)");
                if (nullptr != pPageView->FindPageWindow(*mpOriginalTargetDevice))
                {
                    // Target OutputDevice is registered for this view
                    // (as it should be), we can just render
                    pPageView->DrawLayer(sal::static_int_cast<SdrLayerID>(nLayer), mpDev);
                }
                else if (0 != pPageView->PageWindowCount())
                {
                    // We need to temporarily make the target OutputDevice being
                    // 'known/registered' in the paint mechanism so that
                    // SdrPageView::DrawLayer can find it.
                    // This situation can occur when someone interprets the
                    // OutputDevice parameter that gets handed over to DrawLayer
                    // (or other SdrPageView repaint methods) to be there to
                    // define a new render target.
                    // This is *not* the case: This parameter is used to
                    // *identify* an already registered target-OutputDevice.
                    // The default is even to call with a nullptr -> that triggers
                    // the repaint for *all* registered OutputDevices/Windows.
                    // Since this is a common and known misinterpretation it
                    // is good to offer workarounds in the code - there are some
                    // already.
                    // For now - use an already existing 'patch mechanism' and
                    // 'smuggle' the unknown/temporary OutputDevice as a
                    // temporary SdrPaintWindow to the SdrPageWindow, that is
                    // not very expensive.
                    // NOTE: Just using the 1st SdrPageWindow here will be OK
                    //   in most cases, the splitting of a view is only used
                    //   in calc nowadays and should have identical zoom.
                    //   Still, trigger a warning...
                    OSL_ENSURE(1 == pPageView->PageWindowCount(),
                        "ScOutputData::DrawSelectiveObjects: More than one SdrPageView, still using 1st one (!)");
                    SdrPageWindow* patchedPageWindow(pPageView->GetPageWindow(0));
                    assert(nullptr != patchedPageWindow && "SdrPageWindow *must* exist when 0 != PageWindowCount()");
                    SdrPaintWindow temporaryPaintWindow(*pLocalDrawView, *mpDev);
                    SdrPaintWindow* previousPaintWindow(patchedPageWindow->patchPaintWindow(temporaryPaintWindow));
                    pPageView->DrawLayer(sal::static_int_cast<SdrLayerID>(nLayer), mpDev);
                    patchedPageWindow->unpatchPaintWindow(previousPaintWindow);
                }
                else
                {
                    // There does not even exist a SdrPageWindow. Still call the
                    // paint to get the paint done, but be aware that this will create
                    // temporary SdrPaintWindow and SdrPageWindow and - due to the
                    // former - will not be able to use the decomposition buffering
                    // in the VC/VOC/OC mechanism. For that reason this might be
                    // somewhat 'expensive'.
                    // You will also get a warning about it (see OSL_FAIL in
                    // SdrPageView::DrawLayer)
                    pPageView->DrawLayer(sal::static_int_cast<SdrLayerID>(nLayer), mpDev);
                }
            }
        }
    }

    mpDev->SetDrawMode(nOldDrawMode);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
