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

#include <vcl/BitmapMosaicFilter.hxx>
#include <vcl/BitmapSharpenFilter.hxx>
#include <vcl/BitmapEmbossGreyFilter.hxx>
#include <vcl/BitmapSepiaFilter.hxx>
#include <vcl/BitmapSmoothenFilter.hxx>
#include <vcl/BitmapSolarizeFilter.hxx>
#include <vcl/BitmapColorQuantizationFilter.hxx>
#include <vcl/builderfactory.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <cuigrfflt.hxx>
#include <svx/dialogs.hrc>

CuiGraphicPreviewWindow::CuiGraphicPreviewWindow()
    : mpOrigGraphic(nullptr)
    , mfScaleX(0.0)
    , mfScaleY(0.0)
{
}

void CuiGraphicPreviewWindow::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    OutputDevice &rDevice = pDrawingArea->get_ref_device();
    maOutputSizePixel = rDevice.LogicToPixel(Size(81, 73), MapMode(MapUnit::MapAppFont));
    pDrawingArea->set_size_request(maOutputSizePixel.Width(), maOutputSizePixel.Height());
}

void CuiGraphicPreviewWindow::Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle&)
{
    rRenderContext.SetBackground(Wallpaper(Application::GetSettings().GetStyleSettings().GetDialogColor()));
    rRenderContext.Erase();

    const Size aOutputSize(GetOutputSizePixel());

    if (maPreview.IsAnimated())
    {
        const Size aGraphicSize(rRenderContext.LogicToPixel(maPreview.GetPrefSize(), maPreview.GetPrefMapMode()));
        const Point aGraphicPosition((aOutputSize.Width()  - aGraphicSize.Width()  ) >> 1,
                                     (aOutputSize.Height() - aGraphicSize.Height() ) >> 1);
        maPreview.StartAnimation(&rRenderContext, aGraphicPosition, aGraphicSize);
    }
    else
    {
        const Size  aGraphicSize(maPreview.GetSizePixel());
        const Point aGraphicPosition((aOutputSize.Width()  - aGraphicSize.Width())  >> 1,
                                     (aOutputSize.Height() - aGraphicSize.Height()) >> 1);
        maPreview.Draw(&rRenderContext, aGraphicPosition, aGraphicSize);
    }
}

void CuiGraphicPreviewWindow::SetPreview(const Graphic& rGraphic)
{
    maPreview = rGraphic;
    Invalidate();
}

void CuiGraphicPreviewWindow::ScaleImageToFit()
{
    if (!mpOrigGraphic)
        return;

    maScaledOrig = *mpOrigGraphic;

    const Size aPreviewSize(GetOutputSizePixel());
    Size aGrfSize(maOrigGraphicSizePixel);

    if( mpOrigGraphic->GetType() == GraphicType::Bitmap &&
        aPreviewSize.Width() && aPreviewSize.Height() &&
        aGrfSize.Width() && aGrfSize.Height() )
    {
        const double fGrfWH = static_cast<double>(aGrfSize.Width()) / aGrfSize.Height();
        const double fPreWH = static_cast<double>(aPreviewSize.Width()) / aPreviewSize.Height();

        if( fGrfWH < fPreWH )
        {
            aGrfSize.setWidth( static_cast<long>( aPreviewSize.Height() * fGrfWH ) );
            aGrfSize.setHeight( aPreviewSize.Height() );
        }
        else
        {
            aGrfSize.setWidth( aPreviewSize.Width() );
            aGrfSize.setHeight( static_cast<long>( aPreviewSize.Width() / fGrfWH ) );
        }

        mfScaleX = static_cast<double>(aGrfSize.Width()) / maOrigGraphicSizePixel.Width();
        mfScaleY = static_cast<double>(aGrfSize.Height()) / maOrigGraphicSizePixel.Height();

        if( !mpOrigGraphic->IsAnimated() )
        {
            BitmapEx aBmpEx( mpOrigGraphic->GetBitmapEx() );

            if( aBmpEx.Scale( aGrfSize ) )
                maScaledOrig = aBmpEx;
        }
    }

    maModifyHdl.Call(nullptr);
}

void CuiGraphicPreviewWindow::Resize()
{
    maOutputSizePixel = GetOutputSizePixel();
    ScaleImageToFit();
}

GraphicFilterDialog::GraphicFilterDialog(weld::Window* pParent,
    const OUString& rUIXMLDescription, const OString& rID,
    const Graphic& rGraphic)
    : GenericDialogController(pParent, rUIXMLDescription, rID)
    , maModifyHdl(LINK(this, GraphicFilterDialog, ImplModifyHdl))
    , mxPreview(new weld::CustomWeld(*m_xBuilder, "preview", maPreview))
{
    bIsBitmap = rGraphic.GetType() == GraphicType::Bitmap;

    maTimer.SetInvokeHandler(LINK(this, GraphicFilterDialog, ImplPreviewTimeoutHdl));
    maTimer.SetTimeout(5);

    maPreview.init(&rGraphic, maModifyHdl);
}

IMPL_LINK_NOARG(GraphicFilterDialog, ImplPreviewTimeoutHdl, Timer *, void)
{
    maTimer.Stop();
    maPreview.SetPreview(GetFilteredGraphic(maPreview.GetScaledOriginal(),
        maPreview.GetScaleX(), maPreview.GetScaleY()));
}

IMPL_LINK_NOARG(GraphicFilterDialog, ImplModifyHdl, LinkParamNone*, void)
{
    if (bIsBitmap)
    {
        maTimer.Stop();
        maTimer.Start();
    }
}

GraphicFilterMosaic::GraphicFilterMosaic(weld::Window* pParent, const Graphic& rGraphic,
                                         sal_uInt16 nTileWidth, sal_uInt16 nTileHeight, bool bEnhanceEdges)
    : GraphicFilterDialog(pParent, "cui/ui/mosaicdialog.ui", "MosaicDialog", rGraphic)
    , mxMtrWidth(m_xBuilder->weld_metric_spin_button("width", FieldUnit::PIXEL))
    , mxMtrHeight(m_xBuilder->weld_metric_spin_button("height", FieldUnit::PIXEL))
    , mxCbxEdges(m_xBuilder->weld_check_button("edges"))
{
    mxMtrWidth->set_value(nTileWidth, FieldUnit::PIXEL);
    mxMtrWidth->set_max(GetGraphicSizePixel().Width(), FieldUnit::PIXEL);
    mxMtrWidth->connect_value_changed(LINK(this, GraphicFilterMosaic, EditModifyHdl));

    mxMtrHeight->set_value(nTileHeight, FieldUnit::PIXEL);
    mxMtrHeight->set_max(GetGraphicSizePixel().Height(), FieldUnit::PIXEL);
    mxMtrHeight->connect_value_changed(LINK(this, GraphicFilterMosaic, EditModifyHdl));

    mxCbxEdges->set_active(bEnhanceEdges);
    mxCbxEdges->connect_toggled(LINK(this, GraphicFilterMosaic, CheckBoxModifyHdl));

    mxMtrWidth->grab_focus();
}

IMPL_LINK_NOARG(GraphicFilterMosaic, CheckBoxModifyHdl, weld::ToggleButton&, void)
{
    GetModifyHdl().Call(nullptr);
}

IMPL_LINK_NOARG(GraphicFilterMosaic, EditModifyHdl, weld::MetricSpinButton&, void)
{
    GetModifyHdl().Call(nullptr);
}

Graphic GraphicFilterMosaic::GetFilteredGraphic( const Graphic& rGraphic,
                                                 double fScaleX, double fScaleY )
{
    Graphic         aRet;
    long            nTileWidth = static_cast<long>(mxMtrWidth->get_value(FieldUnit::PIXEL));
    long            nTileHeight = static_cast<long>(mxMtrHeight->get_value(FieldUnit::PIXEL));
    const Size      aSize( std::max( FRound( nTileWidth * fScaleX ), 1L ),
                           std::max( FRound( nTileHeight * fScaleY ), 1L ) );

    if( rGraphic.IsAnimated() )
    {
        Animation aAnim( rGraphic.GetAnimation() );

        if (BitmapFilter::Filter(aAnim, BitmapMosaicFilter(aSize.getWidth(), aSize.getHeight())))
        {
            if( IsEnhanceEdges() )
                (void)BitmapFilter::Filter(aAnim, BitmapSharpenFilter());

            aRet = aAnim;
        }
    }
    else
    {
        BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

        if (BitmapFilter::Filter(aBmpEx, BitmapMosaicFilter(aSize.getWidth(), aSize.getHeight())))
        {
            if( IsEnhanceEdges() )
                BitmapFilter::Filter(aBmpEx, BitmapSharpenFilter());

            aRet = aBmpEx;
        }
    }

    return aRet;
}

GraphicFilterSmooth::GraphicFilterSmooth(weld::Window* pParent, const Graphic& rGraphic, double nRadius)
    : GraphicFilterDialog(pParent, "cui/ui/smoothdialog.ui", "SmoothDialog", rGraphic)
    , mxMtrRadius(m_xBuilder->weld_spin_button("radius"))
{
    mxMtrRadius->set_value(nRadius * 10);
    mxMtrRadius->connect_value_changed(LINK(this, GraphicFilterSmooth, EditModifyHdl));
    mxMtrRadius->grab_focus();
}

IMPL_LINK_NOARG(GraphicFilterSmooth, EditModifyHdl, weld::SpinButton&, void)
{
    GetModifyHdl().Call(nullptr);
}

Graphic GraphicFilterSmooth::GetFilteredGraphic( const Graphic& rGraphic, double, double )
{
    Graphic         aRet;
    double          nRadius = mxMtrRadius->get_value() / 10.0;

    if( rGraphic.IsAnimated() )
    {
        Animation aAnim( rGraphic.GetAnimation() );

        if (BitmapFilter::Filter(aAnim, BitmapSmoothenFilter(nRadius)))
        {
            aRet = aAnim;
        }
    }
    else
    {
        BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

        if (BitmapFilter::Filter(aBmpEx, BitmapSmoothenFilter(nRadius)))
        {
            aRet = aBmpEx;
        }
    }

    return aRet;
}

GraphicFilterSolarize::GraphicFilterSolarize(weld::Window* pParent, const Graphic& rGraphic,
                                             sal_uInt8 cGreyThreshold, bool bInvert)
    : GraphicFilterDialog(pParent, "cui/ui/solarizedialog.ui", "SolarizeDialog", rGraphic)
    , mxMtrThreshold(m_xBuilder->weld_metric_spin_button("value", FieldUnit::PERCENT))
    , mxCbxInvert(m_xBuilder->weld_check_button("invert"))
{
    mxMtrThreshold->set_value(FRound(cGreyThreshold / 2.55), FieldUnit::PERCENT);
    mxMtrThreshold->connect_value_changed(LINK(this, GraphicFilterSolarize, EditModifyHdl));

    mxCbxInvert->set_active(bInvert);
    mxCbxInvert->connect_toggled(LINK(this, GraphicFilterSolarize, CheckBoxModifyHdl));
}

IMPL_LINK_NOARG(GraphicFilterSolarize, CheckBoxModifyHdl, weld::ToggleButton&, void)
{
    GetModifyHdl().Call(nullptr);
}

IMPL_LINK_NOARG(GraphicFilterSolarize, EditModifyHdl, weld::MetricSpinButton&, void)
{
    GetModifyHdl().Call(nullptr);
}

Graphic GraphicFilterSolarize::GetFilteredGraphic( const Graphic& rGraphic, double, double )
{
    Graphic         aRet;
    sal_uInt8       nGreyThreshold = static_cast<sal_uInt8>(FRound(mxMtrThreshold->get_value(FieldUnit::PERCENT) * 2.55));

    if( rGraphic.IsAnimated() )
    {
        Animation aAnim( rGraphic.GetAnimation() );

        if (BitmapFilter::Filter(aAnim, BitmapSolarizeFilter(nGreyThreshold)))
        {
            if( IsInvert() )
                aAnim.Invert();

            aRet = aAnim;
        }
    }
    else
    {
        BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

        if (BitmapFilter::Filter(aBmpEx, BitmapSolarizeFilter(nGreyThreshold)))
        {
            if( IsInvert() )
                aBmpEx.Invert();

            aRet = aBmpEx;
        }
    }

    return aRet;
}

GraphicFilterSepia::GraphicFilterSepia(weld::Window* pParent, const Graphic& rGraphic,
                                       sal_uInt16 nSepiaPercent)
    : GraphicFilterDialog(pParent, "cui/ui/agingdialog.ui", "AgingDialog", rGraphic)
    , mxMtrSepia(m_xBuilder->weld_metric_spin_button("value", FieldUnit::PERCENT))
{
    mxMtrSepia->set_value(nSepiaPercent, FieldUnit::PERCENT);
    mxMtrSepia->connect_value_changed(LINK(this, GraphicFilterSepia, EditModifyHdl));
}

IMPL_LINK_NOARG(GraphicFilterSepia, EditModifyHdl, weld::MetricSpinButton&, void)
{
    GetModifyHdl().Call(nullptr);
}

Graphic GraphicFilterSepia::GetFilteredGraphic( const Graphic& rGraphic, double, double )
{
    Graphic         aRet;
    sal_uInt16      nSepiaPct = sal::static_int_cast< sal_uInt16 >(mxMtrSepia->get_value(FieldUnit::PERCENT));

    if( rGraphic.IsAnimated() )
    {
        Animation aAnim( rGraphic.GetAnimation() );

        if (BitmapFilter::Filter(aAnim, BitmapSepiaFilter(nSepiaPct)))
            aRet = aAnim;
    }
    else
    {
        BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

        if (BitmapFilter::Filter(aBmpEx, BitmapSepiaFilter(nSepiaPct)))
            aRet = aBmpEx;
    }

    return aRet;
}

GraphicFilterPoster::GraphicFilterPoster(weld::Window* pParent, const Graphic& rGraphic,
                                          sal_uInt16 nPosterCount)
    : GraphicFilterDialog(pParent, "cui/ui/posterdialog.ui", "PosterDialog", rGraphic)
    , mxNumPoster(m_xBuilder->weld_spin_button("value"))
{
    mxNumPoster->set_range(2, rGraphic.GetBitmapEx().GetBitCount());
    mxNumPoster->set_value(nPosterCount);
    mxNumPoster->connect_value_changed(LINK(this, GraphicFilterPoster, EditModifyHdl));
}

IMPL_LINK_NOARG(GraphicFilterPoster, EditModifyHdl, weld::SpinButton&, void)
{
    GetModifyHdl().Call(nullptr);
}

Graphic GraphicFilterPoster::GetFilteredGraphic( const Graphic& rGraphic, double, double )
{
    Graphic          aRet;
    const sal_uInt16 nPosterCount = static_cast<sal_uInt16>(mxNumPoster->get_value());

    if( rGraphic.IsAnimated() )
    {
        Animation aAnim( rGraphic.GetAnimation() );

        if( aAnim.ReduceColors( nPosterCount ) )
            aRet = aAnim;
    }
    else
    {
        BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

        if (BitmapFilter::Filter(aBmpEx, BitmapColorQuantizationFilter(nPosterCount)))
            aRet = aBmpEx;
    }

    return aRet;
}

bool EmbossControl::MouseButtonDown( const MouseEvent& rEvt )
{
    const RectPoint eOldRP = GetActualRP();

    SvxRectCtl::MouseButtonDown( rEvt );

    if( GetActualRP() != eOldRP )
        maModifyHdl.Call( nullptr );

    return true;
}

void EmbossControl::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    SvxRectCtl::SetDrawingArea(pDrawingArea);
    Size aSize(pDrawingArea->get_ref_device().LogicToPixel(Size(77, 60), MapMode(MapUnit::MapAppFont)));
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());
}

GraphicFilterEmboss::GraphicFilterEmboss(weld::Window* pParent,
    const Graphic& rGraphic, RectPoint eLightSource)
    : GraphicFilterDialog(pParent, "cui/ui/embossdialog.ui", "EmbossDialog", rGraphic)
    , mxCtlLight(new weld::CustomWeld(*m_xBuilder, "lightsource", maCtlLight))
{
    maCtlLight.SetActualRP(eLightSource);
    maCtlLight.SetModifyHdl( GetModifyHdl() );
    maCtlLight.GrabFocus();
}

GraphicFilterEmboss::~GraphicFilterEmboss()
{
}

Graphic GraphicFilterEmboss::GetFilteredGraphic( const Graphic& rGraphic, double, double )
{
    Graphic aRet;
    sal_uInt16  nAzim, nElev;

    switch (maCtlLight.GetActualRP())
    {
        default:       OSL_FAIL("svx::GraphicFilterEmboss::GetFilteredGraphic(), unknown Reference Point!" );
                       [[fallthrough]];
        case RectPoint::LT: nAzim = 4500;    nElev = 4500; break;
        case RectPoint::MT: nAzim = 9000;    nElev = 4500; break;
        case RectPoint::RT: nAzim = 13500;   nElev = 4500; break;
        case RectPoint::LM: nAzim = 0;       nElev = 4500; break;
        case RectPoint::MM: nAzim = 0;       nElev = 9000; break;
        case RectPoint::RM: nAzim = 18000;   nElev = 4500; break;
        case RectPoint::LB: nAzim = 31500;   nElev = 4500; break;
        case RectPoint::MB: nAzim = 27000;   nElev = 4500; break;
        case RectPoint::RB: nAzim = 22500;   nElev = 4500; break;
    }

    if( rGraphic.IsAnimated() )
    {
        Animation aAnim( rGraphic.GetAnimation() );

        if (BitmapFilter::Filter(aAnim, BitmapEmbossGreyFilter(nAzim, nElev)))
            aRet = aAnim;
    }
    else
    {
        BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

        if (BitmapFilter::Filter(aBmpEx, BitmapEmbossGreyFilter(nAzim, nElev)))
            aRet = aBmpEx;
    }

    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
