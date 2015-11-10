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

#include <vcl/builderfactory.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <dialmgr.hxx>
#include "cuigrfflt.hxx"
#include <cuires.hrc>
#include <svx/dialogs.hrc>

GraphicPreviewWindow::GraphicPreviewWindow(vcl::Window* pParent,
    const WinBits nStyle)
    : Control(pParent, nStyle)
    , mpOrigGraphic(nullptr)
    , mfScaleX(0.0)
    , mfScaleY(0.0)
{
}

VCL_BUILDER_DECL_FACTORY(GraphicPreviewWindow)
{
    WinBits nWinBits = WB_TABSTOP;

    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
       nWinBits |= WB_BORDER;

    rRet = VclPtr<GraphicPreviewWindow>::Create(pParent, nWinBits);
}

Size GraphicPreviewWindow::GetOptimalSize() const
{
    return LogicToPixel(Size(81, 73), MAP_APPFONT);
}

void GraphicPreviewWindow::Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect)
{
    Control::Paint(rRenderContext, rRect);

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

void GraphicPreviewWindow::SetPreview(const Graphic& rGraphic)
{
    maPreview = rGraphic;
    Invalidate();
}

void GraphicPreviewWindow::ScaleImageToFit()
{
    if (!mpOrigGraphic)
        return;

    maScaledOrig = *mpOrigGraphic;

    const Size  aPreviewSize( GetOutputSizePixel() );
    Size aSizePixel(LogicToPixel(mpOrigGraphic->GetPrefSize(),
        mpOrigGraphic->GetPrefMapMode()));
    Size aGrfSize(aSizePixel);

    if( mpOrigGraphic->GetType() == GRAPHIC_BITMAP &&
        aPreviewSize.Width() && aPreviewSize.Height() &&
        aGrfSize.Width() && aGrfSize.Height() )
    {
        const double fGrfWH = (double) aGrfSize.Width() / aGrfSize.Height();
        const double fPreWH = (double) aPreviewSize.Width() / aPreviewSize.Height();

        if( fGrfWH < fPreWH )
        {
            aGrfSize.Width()  = (long) ( aPreviewSize.Height() * fGrfWH );
            aGrfSize.Height() = aPreviewSize.Height();
        }
        else
        {
            aGrfSize.Width()  = aPreviewSize.Width();
            aGrfSize.Height() = (long) ( aPreviewSize.Width() / fGrfWH );
        }

        mfScaleX = (double) aGrfSize.Width() / aSizePixel.Width();
        mfScaleY = (double) aGrfSize.Height() / aSizePixel.Height();

        if( !mpOrigGraphic->IsAnimated() )
        {
            BitmapEx aBmpEx( mpOrigGraphic->GetBitmapEx() );

            if( aBmpEx.Scale( aGrfSize ) )
                maScaledOrig = aBmpEx;
        }
    }

    maModifyHdl.Call(nullptr);
}

void GraphicPreviewWindow::Resize()
{
    Control::Resize();
    ScaleImageToFit();
}

GraphicFilterDialog::GraphicFilterDialog(vcl::Window* pParent,
    const OUString& rID, const OUString& rUIXMLDescription,
    const Graphic& rGraphic)
    : ModalDialog(pParent, rID, rUIXMLDescription)
    , maModifyHdl(LINK( this, GraphicFilterDialog, ImplModifyHdl))
    , maSizePixel(LogicToPixel(rGraphic.GetPrefSize(),
        rGraphic.GetPrefMapMode()))
{
    bIsBitmap = rGraphic.GetType() == GRAPHIC_BITMAP;

    maTimer.SetTimeoutHdl( LINK( this, GraphicFilterDialog, ImplPreviewTimeoutHdl ) );
    maTimer.SetTimeout( 5 );

    get(mpPreview, "preview");
    mpPreview->init(&rGraphic, maModifyHdl);
}

GraphicFilterDialog::~GraphicFilterDialog()
{
    disposeOnce();
}

void GraphicFilterDialog::dispose()
{
    mpPreview.clear();
    ModalDialog::dispose();
}


IMPL_LINK_NOARG_TYPED(GraphicFilterDialog, ImplPreviewTimeoutHdl, Timer *, void)
{
    maTimer.Stop();
    mpPreview->SetPreview(GetFilteredGraphic(mpPreview->GetScaledOriginal(),
        mpPreview->GetScaleX(), mpPreview->GetScaleY()));
}



IMPL_LINK_NOARG_TYPED(GraphicFilterDialog, ImplModifyHdl, LinkParamNone*, void)
{
    if (bIsBitmap)
    {
        maTimer.Stop();
        maTimer.Start();
    }
}


// - FilterMosaic -


GraphicFilterMosaic::GraphicFilterMosaic( vcl::Window* pParent, const Graphic& rGraphic,
                                          sal_uInt16 nTileWidth, sal_uInt16 nTileHeight, bool bEnhanceEdges )
    : GraphicFilterDialog(pParent, "MosaicDialog",
        "cui/ui/mosaicdialog.ui", rGraphic)
{
    get(mpMtrWidth, "width");
    get(mpMtrHeight, "height");
    get(mpCbxEdges, "edges");

    mpMtrWidth->SetValue( nTileWidth );
    mpMtrWidth->SetLast( GetGraphicSizePixel().Width() );
    mpMtrWidth->SetModifyHdl( LINK(this, GraphicFilterMosaic, EditModifyHdl) );

    mpMtrHeight->SetValue( nTileHeight );
    mpMtrHeight->SetLast( GetGraphicSizePixel().Height() );
    mpMtrHeight->SetModifyHdl( LINK(this, GraphicFilterMosaic, EditModifyHdl) );

    mpCbxEdges->Check( bEnhanceEdges );
    mpCbxEdges->SetToggleHdl( LINK(this, GraphicFilterMosaic, CheckBoxModifyHdl) );

    mpMtrWidth->GrabFocus();
}

IMPL_LINK_NOARG_TYPED(GraphicFilterMosaic, CheckBoxModifyHdl, CheckBox&, void)
{
    GetModifyHdl().Call(nullptr);
}

IMPL_LINK_NOARG_TYPED(GraphicFilterMosaic, EditModifyHdl, Edit&, void)
{
    GetModifyHdl().Call(nullptr);
}

GraphicFilterMosaic::~GraphicFilterMosaic()
{
    disposeOnce();
}

void GraphicFilterMosaic::dispose()
{
    mpMtrWidth.clear();
    mpMtrHeight.clear();
    mpCbxEdges.clear();
    GraphicFilterDialog::dispose();
}

Graphic GraphicFilterMosaic::GetFilteredGraphic( const Graphic& rGraphic,
                                                 double fScaleX, double fScaleY )
{
    Graphic         aRet;
    const Size      aSize( std::max( FRound( GetTileWidth() * fScaleX ), 1L ),
                           std::max( FRound( GetTileHeight() * fScaleY ), 1L ) );
    BmpFilterParam  aParam( aSize );

    if( rGraphic.IsAnimated() )
    {
        Animation aAnim( rGraphic.GetAnimation() );

        if( aAnim.Filter( BMP_FILTER_MOSAIC, &aParam ) )
        {
            if( IsEnhanceEdges() )
                aAnim.Filter( BMP_FILTER_SHARPEN );

            aRet = aAnim;
        }
    }
    else
    {
        BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

        if( aBmpEx.Filter( BMP_FILTER_MOSAIC, &aParam ) )
        {
            if( IsEnhanceEdges() )
                (void)aBmpEx.Filter( BMP_FILTER_SHARPEN );

            aRet = aBmpEx;
        }
    }

    return aRet;
}


// - GraphicFilterSmooth -


GraphicFilterSmooth::GraphicFilterSmooth( vcl::Window* pParent, const Graphic& rGraphic, double nRadius)
    : GraphicFilterDialog(pParent, "SmoothDialog",
        "cui/ui/smoothdialog.ui", rGraphic)
{
    get(mpMtrRadius, "radius");

    mpMtrRadius->SetValue( nRadius* 10  );
    mpMtrRadius->SetModifyHdl( LINK(this, GraphicFilterSmooth, EditModifyHdl) );
    mpMtrRadius->GrabFocus();
}

IMPL_LINK_NOARG_TYPED(GraphicFilterSmooth, EditModifyHdl, Edit&, void)
{
    GetModifyHdl().Call(nullptr);
}

GraphicFilterSmooth::~GraphicFilterSmooth()
{
    disposeOnce();
}

void GraphicFilterSmooth::dispose()
{
    mpMtrRadius.clear();
    GraphicFilterDialog::dispose();
}


Graphic GraphicFilterSmooth::GetFilteredGraphic( const Graphic& rGraphic, double /*fScaleX*/, double /*fScaleY*/ )
{
    Graphic         aRet;
    BmpFilterParam  aParam( GetRadius() );

    if( rGraphic.IsAnimated() )
    {
        Animation aAnim( rGraphic.GetAnimation() );

        if( aAnim.Filter( BMP_FILTER_SMOOTH, &aParam ) )
        {
            aRet = aAnim;
        }
    }
    else
    {
        BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

        if( aBmpEx.Filter( BMP_FILTER_SMOOTH, &aParam ) )
        {
            aRet = aBmpEx;
        }
    }

    return aRet;
}


// - GraphicFilterSolarize -


GraphicFilterSolarize::GraphicFilterSolarize( vcl::Window* pParent, const Graphic& rGraphic,
                                              sal_uInt8 cGreyThreshold, bool bInvert )
    : GraphicFilterDialog(pParent, "SolarizeDialog",
        "cui/ui/solarizedialog.ui", rGraphic)
{
    get(mpMtrThreshold, "value");
    get(mpCbxInvert, "invert");

    mpMtrThreshold->SetValue( FRound( cGreyThreshold / 2.55 ) );
    mpMtrThreshold->SetModifyHdl( LINK(this, GraphicFilterSolarize, EditModifyHdl) );

    mpCbxInvert->Check( bInvert );
    mpCbxInvert->SetToggleHdl( LINK(this, GraphicFilterSolarize, CheckBoxModifyHdl) );
}

IMPL_LINK_NOARG_TYPED(GraphicFilterSolarize, CheckBoxModifyHdl, CheckBox&, void)
{
    GetModifyHdl().Call(nullptr);
}
IMPL_LINK_NOARG_TYPED(GraphicFilterSolarize, EditModifyHdl, Edit&, void)
{
    GetModifyHdl().Call(nullptr);
}

GraphicFilterSolarize::~GraphicFilterSolarize()
{
    disposeOnce();
}

void GraphicFilterSolarize::dispose()
{
    mpMtrThreshold.clear();
    mpCbxInvert.clear();
    GraphicFilterDialog::dispose();
}


Graphic GraphicFilterSolarize::GetFilteredGraphic( const Graphic& rGraphic,
                                                   double /*fScaleX*/, double /*fScaleY*/ )
{
    Graphic         aRet;
    BmpFilterParam  aParam( GetGreyThreshold() );

    if( rGraphic.IsAnimated() )
    {
        Animation aAnim( rGraphic.GetAnimation() );

        if( aAnim.Filter( BMP_FILTER_SOLARIZE, &aParam ) )
        {
            if( IsInvert() )
                aAnim.Invert();

            aRet = aAnim;
        }
    }
    else
    {
        BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

        if( aBmpEx.Filter( BMP_FILTER_SOLARIZE, &aParam ) )
        {
            if( IsInvert() )
                aBmpEx.Invert();

            aRet = aBmpEx;
        }
    }

    return aRet;
}


// - GraphicFilterSepia -


GraphicFilterSepia::GraphicFilterSepia( vcl::Window* pParent, const Graphic& rGraphic,
                                        sal_uInt16 nSepiaPercent )
    : GraphicFilterDialog(pParent, "AgingDialog",
        "cui/ui/agingdialog.ui", rGraphic)
{
    get(mpMtrSepia, "value");

    mpMtrSepia->SetValue( nSepiaPercent );
    mpMtrSepia->SetModifyHdl( LINK(this, GraphicFilterSepia, EditModifyHdl) );
}

IMPL_LINK_NOARG_TYPED(GraphicFilterSepia, EditModifyHdl, Edit&, void)
{
    GetModifyHdl().Call(nullptr);
}

GraphicFilterSepia::~GraphicFilterSepia()
{
    disposeOnce();
}

void GraphicFilterSepia::dispose()
{
    mpMtrSepia.clear();
    GraphicFilterDialog::dispose();
}


Graphic GraphicFilterSepia::GetFilteredGraphic( const Graphic& rGraphic,
                                                double /*fScaleX*/, double /*fScaleY*/ )
{
    Graphic         aRet;
    BmpFilterParam  aParam( GetSepiaPercent() );

    if( rGraphic.IsAnimated() )
    {
        Animation aAnim( rGraphic.GetAnimation() );

        if( aAnim.Filter( BMP_FILTER_SEPIA, &aParam ) )
            aRet = aAnim;
    }
    else
    {
        BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

        if( aBmpEx.Filter( BMP_FILTER_SEPIA, &aParam ) )
            aRet = aBmpEx;
    }

    return aRet;
}


// - GraphicFilterPoster -


GraphicFilterPoster::GraphicFilterPoster(vcl::Window* pParent, const Graphic& rGraphic,
                                          sal_uInt16 nPosterCount)
    : GraphicFilterDialog(pParent, "PosterDialog",
        "cui/ui/posterdialog.ui", rGraphic)
{
    get(mpNumPoster, "value");

    mpNumPoster->SetFirst( 2 );
    mpNumPoster->SetLast( rGraphic.GetBitmapEx().GetBitCount() );
    mpNumPoster->SetValue( nPosterCount );
    mpNumPoster->SetModifyHdl( LINK(this, GraphicFilterPoster, EditModifyHdl) );
}

IMPL_LINK_NOARG_TYPED(GraphicFilterPoster, EditModifyHdl, Edit&, void)
{
    GetModifyHdl().Call(nullptr);
}

GraphicFilterPoster::~GraphicFilterPoster()
{
    disposeOnce();
}

void GraphicFilterPoster::dispose()
{
    mpNumPoster.clear();
    GraphicFilterDialog::dispose();
}


Graphic GraphicFilterPoster::GetFilteredGraphic( const Graphic& rGraphic,
                                                 double /*fScaleX*/, double /*fScaleY*/ )
{
    Graphic         aRet;
    const sal_uInt16    nPosterCount = GetPosterColorCount();

    if( rGraphic.IsAnimated() )
    {
        Animation aAnim( rGraphic.GetAnimation() );

        if( aAnim.ReduceColors( nPosterCount, BMP_REDUCE_POPULAR ) )
            aRet = aAnim;
    }
    else
    {
        BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

        if( aBmpEx.ReduceColors( nPosterCount, BMP_REDUCE_POPULAR ) )
            aRet = aBmpEx;
    }

    return aRet;
}


// - GraphicFilterEmboss -


void EmbossControl::MouseButtonDown( const MouseEvent& rEvt )
{
    const RECT_POINT eOldRP = GetActualRP();

    SvxRectCtl::MouseButtonDown( rEvt );

    if( GetActualRP() != eOldRP )
        maModifyHdl.Call( nullptr );
}

Size EmbossControl::GetOptimalSize() const
{
    return LogicToPixel(Size(77, 60), MAP_APPFONT);
}

VCL_BUILDER_FACTORY(EmbossControl)

GraphicFilterEmboss::GraphicFilterEmboss(vcl::Window* pParent,
    const Graphic& rGraphic, RECT_POINT eLightSource)
    : GraphicFilterDialog (pParent, "EmbossDialog",
        "cui/ui/embossdialog.ui", rGraphic)
{
    get(mpCtlLight, "lightsource");
    mpCtlLight->SetActualRP(eLightSource);
    mpCtlLight->SetModifyHdl( GetModifyHdl() );
    mpCtlLight->GrabFocus();
}

GraphicFilterEmboss::~GraphicFilterEmboss()
{
    disposeOnce();
}

void GraphicFilterEmboss::dispose()
{
    mpCtlLight.clear();
    GraphicFilterDialog::dispose();
}


Graphic GraphicFilterEmboss::GetFilteredGraphic( const Graphic& rGraphic,
                                                 double /*fScaleX*/, double /*fScaleY*/ )
{
    Graphic aRet;
    sal_uInt16  nAzim, nElev;

    switch( mpCtlLight->GetActualRP() )
    {
        default:       OSL_FAIL("svx::GraphicFilterEmboss::GetFilteredGraphic(), unknown Reference Point!" );
                       /* Fall through */
        case( RP_LT ): nAzim = 4500,    nElev = 4500; break;
        case( RP_MT ): nAzim = 9000,    nElev = 4500; break;
        case( RP_RT ): nAzim = 13500,   nElev = 4500; break;
        case( RP_LM ): nAzim = 0,       nElev = 4500; break;
        case( RP_MM ): nAzim = 0,       nElev = 9000; break;
        case( RP_RM ): nAzim = 18000,   nElev = 4500; break;
        case( RP_LB ): nAzim = 31500,   nElev = 4500; break;
        case( RP_MB ): nAzim = 27000,   nElev = 4500; break;
        case( RP_RB ): nAzim = 22500,   nElev = 4500; break;
    }

    BmpFilterParam aParam( nAzim, nElev );

    if( rGraphic.IsAnimated() )
    {
        Animation aAnim( rGraphic.GetAnimation() );

        if( aAnim.Filter( BMP_FILTER_EMBOSS_GREY, &aParam ) )
            aRet = aAnim;
    }
    else
    {
        BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

        if( aBmpEx.Filter( BMP_FILTER_EMBOSS_GREY, &aParam ) )
            aRet = aBmpEx;
    }

    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
