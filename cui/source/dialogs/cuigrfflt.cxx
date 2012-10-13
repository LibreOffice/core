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

#include <tools/shl.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <dialmgr.hxx>
#include "cuigrfflt.hxx"
#include "grfflt.hrc"
#include <cuires.hrc>
#include <svx/dialogs.hrc> // RID_SVX_GRFFILTER_DLG_...

// --------------------------------------
// - GraphicFilterDialog::PreviewWindow -
// --------------------------------------

GraphicFilterDialog::PreviewWindow::PreviewWindow( Window* pParent, const ResId& rResId ) :
    Control( pParent, rResId )
{
}

// -----------------------------------------------------------------------------

GraphicFilterDialog::PreviewWindow::~PreviewWindow()
{
}

// -----------------------------------------------------------------------------

void GraphicFilterDialog::PreviewWindow::Paint( const Rectangle& rRect )
{
    Control::Paint( rRect );

    const Size  aOutputSize( GetOutputSizePixel() );

    if( maGraphic.IsAnimated() )
    {
        const Size  aGraphicSize( LogicToPixel( maGraphic.GetPrefSize(), maGraphic.GetPrefMapMode() ) );
        const Point aGraphicPosition( ( aOutputSize.Width()  - aGraphicSize.Width()  ) >> 1,
                                      ( aOutputSize.Height() - aGraphicSize.Height() ) >> 1 );
        maGraphic.StartAnimation( this, aGraphicPosition, aGraphicSize );
    }
    else
    {
        const Size  aGraphicSize( maGraphic.GetSizePixel() );
        const Point aGraphicPosition( ( aOutputSize.Width()  - aGraphicSize.Width()  ) >> 1,
                                      ( aOutputSize.Height() - aGraphicSize.Height() ) >> 1 );
        maGraphic.Draw( this, aGraphicPosition, aGraphicSize );
    }
}

// -----------------------------------------------------------------------------

void GraphicFilterDialog::PreviewWindow::SetGraphic( const Graphic& rGraphic )
{
    maGraphic = rGraphic;

    if( maGraphic.IsAnimated() || maGraphic.IsTransparent() )
        Invalidate();
    else
        Paint( Rectangle( Point(), GetOutputSizePixel() ) );
}

// -----------------------
// - GraphicFilterDialog -
// -----------------------

GraphicFilterDialog::GraphicFilterDialog( Window* pParent, const ResId& rResId, const Graphic& rGraphic ) :
    ModalDialog     ( pParent, rResId ),
    maModifyHdl     ( LINK( this, GraphicFilterDialog, ImplModifyHdl ) ),
    mfScaleX        ( 0.0 ),
    mfScaleY        ( 0.0 ),
    maSizePixel     ( LogicToPixel( rGraphic.GetPrefSize(), rGraphic.GetPrefMapMode() ) ),
    maPreview       ( this, CUI_RES( CTL_PREVIEW ) ),
    maBtnOK         ( this, CUI_RES( BTN_OK ) ),
    maBtnCancel     ( this, CUI_RES( BTN_CANCEL ) ),
    maBtnHelp       ( this, CUI_RES( BTN_HELP ) ),
    maFlParameter   ( this, CUI_RES( FL_PARAMETER ) )
{
    const Size  aPreviewSize( maPreview.GetOutputSizePixel() );
    Size        aGrfSize( maSizePixel );

    if( rGraphic.GetType() == GRAPHIC_BITMAP &&
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

        mfScaleX = (double) aGrfSize.Width() / maSizePixel.Width();
        mfScaleY = (double) aGrfSize.Height() / maSizePixel.Height();

        if( !rGraphic.IsAnimated() )
        {
            BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

            if( aBmpEx.Scale( aGrfSize, BMP_SCALE_DEFAULT ) )
                maGraphic = aBmpEx;
        }
    }

    maTimer.SetTimeoutHdl( LINK( this, GraphicFilterDialog, ImplPreviewTimeoutHdl ) );
    maTimer.SetTimeout( 100 );
    ImplModifyHdl( NULL );
}

// -----------------------------------------------------------------------------

GraphicFilterDialog::~GraphicFilterDialog()
{
}

// -----------------------------------------------------------------------------

IMPL_LINK_NOARG(GraphicFilterDialog, ImplPreviewTimeoutHdl)
{
    maTimer.Stop();
    maPreview.SetGraphic( GetFilteredGraphic( maGraphic, mfScaleX, mfScaleY ) );

    return 0;
}

// -----------------------------------------------------------------------------

IMPL_LINK_NOARG(GraphicFilterDialog, ImplModifyHdl)
{
    if( maGraphic.GetType() == GRAPHIC_BITMAP )
    {
        maTimer.Stop();
        maTimer.Start();
    }

    return 0;
}

// ----------------
// - FilterMosaic -
// ----------------

GraphicFilterMosaic::GraphicFilterMosaic( Window* pParent, const Graphic& rGraphic,
                                          sal_uInt16 nTileWidth, sal_uInt16 nTileHeight, sal_Bool bEnhanceEdges ) :
    GraphicFilterDialog( pParent, CUI_RES( RID_SVX_GRFFILTER_DLG_MOSAIC ), rGraphic ),
    maFtWidth   ( this, CUI_RES( DLG_FILTERMOSAIC_FT_WIDTH ) ),
    maMtrWidth  ( this, CUI_RES( DLG_FILTERMOSAIC_MTR_WIDTH ) ),
    maFtHeight  ( this, CUI_RES( DLG_FILTERMOSAIC_FT_HEIGHT ) ),
    maMtrHeight ( this, CUI_RES( DLG_FILTERMOSAIC_MTR_HEIGHT ) ),
    maCbxEdges  ( this, CUI_RES( DLG_FILTERMOSAIC_CBX_EDGES ) )
{
    FreeResource();

    maMtrWidth.SetValue( nTileWidth );
    maMtrWidth.SetLast( GetGraphicSizePixel().Width() );
    maMtrWidth.SetModifyHdl( GetModifyHdl() );

    maMtrHeight.SetValue( nTileHeight );
    maMtrHeight.SetLast( GetGraphicSizePixel().Height() );
    maMtrHeight.SetModifyHdl( GetModifyHdl() );

    maCbxEdges.Check( bEnhanceEdges );
    maCbxEdges.SetToggleHdl( GetModifyHdl() );

    maMtrWidth.GrabFocus();

    maFtWidth.SetAccessibleRelationMemberOf(&maFlParameter);
    maMtrWidth.SetAccessibleRelationMemberOf(&maFlParameter);
    maFtHeight.SetAccessibleRelationMemberOf(&maFlParameter);
    maMtrHeight.SetAccessibleRelationMemberOf(&maFlParameter);
    maCbxEdges.SetAccessibleRelationMemberOf(&maFlParameter);
}

// -----------------------------------------------------------------------------

GraphicFilterMosaic::~GraphicFilterMosaic()
{
}

// -----------------------------------------------------------------------------

Graphic GraphicFilterMosaic::GetFilteredGraphic( const Graphic& rGraphic,
                                                 double fScaleX, double fScaleY )
{
    Graphic         aRet;
    const Size      aSize( Max( FRound( GetTileWidth() * fScaleX ), 1L ),
                           Max( FRound( GetTileHeight() * fScaleY ), 1L ) );
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
                aBmpEx.Filter( BMP_FILTER_SHARPEN );

            aRet = aBmpEx;
        }
    }

    return aRet;
}

// ------------------
// - GraphicFilterSmooth -
// ------------------

GraphicFilterSmooth::GraphicFilterSmooth( Window* pParent, const Graphic& rGraphic, double nRadius) :
    GraphicFilterDialog ( pParent, CUI_RES( RID_SVX_GRFFILTER_DLG_SMOOTH ), rGraphic ),
    maFtRadius   ( this, CUI_RES( DLG_FILTERSMOOTH_FT_RADIUS ) ),
    maMtrRadius  ( this, CUI_RES( DLG_FILTERSMOOTH_MTR_RADIUS ) )
{
    FreeResource();

    maMtrRadius.SetValue( nRadius* 10  );
    maMtrRadius.SetModifyHdl( GetModifyHdl() );
    maMtrRadius.GrabFocus();
}

// -----------------------------------------------------------------------------

GraphicFilterSmooth::~GraphicFilterSmooth()
{
}

// -----------------------------------------------------------------------------

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

// ------------------
// - GraphicFilterSolarize -
// ------------------

GraphicFilterSolarize::GraphicFilterSolarize( Window* pParent, const Graphic& rGraphic,
                                              sal_uInt8 cGreyThreshold, sal_Bool bInvert ) :
    GraphicFilterDialog ( pParent, CUI_RES( RID_SVX_GRFFILTER_DLG_SOLARIZE ), rGraphic ),
    maFtThreshold   ( this, CUI_RES( DLG_FILTERSOLARIZE_FT_THRESHOLD ) ),
    maMtrThreshold  ( this, CUI_RES( DLG_FILTERSOLARIZE_MTR_THRESHOLD ) ),
    maCbxInvert     ( this, CUI_RES( DLG_FILTERSOLARIZE_CBX_INVERT ) )
{
    FreeResource();

    maMtrThreshold.SetValue( FRound( cGreyThreshold / 2.55 ) );
    maMtrThreshold.SetModifyHdl( GetModifyHdl() );

    maCbxInvert.Check( bInvert );
    maCbxInvert.SetToggleHdl( GetModifyHdl() );

    maMtrThreshold.GrabFocus();
}

// -----------------------------------------------------------------------------

GraphicFilterSolarize::~GraphicFilterSolarize()
{
}

// -----------------------------------------------------------------------------

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

// ----------------------
// - GraphicFilterSepia -
// ----------------------

GraphicFilterSepia::GraphicFilterSepia( Window* pParent, const Graphic& rGraphic,
                                        sal_uInt16 nSepiaPercent ) :
    GraphicFilterDialog ( pParent, CUI_RES( RID_SVX_GRFFILTER_DLG_SEPIA ), rGraphic ),
    maFtSepia       ( this, CUI_RES( DLG_FILTERSEPIA_FT_SEPIA ) ),
    maMtrSepia      ( this, CUI_RES( DLG_FILTERSEPIA_MTR_SEPIA ) )
{
    FreeResource();

    maMtrSepia.SetValue( nSepiaPercent );
    maMtrSepia.SetModifyHdl( GetModifyHdl() );

    maMtrSepia.GrabFocus();
}

// -----------------------------------------------------------------------------

GraphicFilterSepia::~GraphicFilterSepia()
{
}

// -----------------------------------------------------------------------------

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

// -----------------------
// - GraphicFilterPoster -
// -----------------------

GraphicFilterPoster::GraphicFilterPoster( Window* pParent, const Graphic& rGraphic,
                                          sal_uInt16 nPosterCount ) :
    GraphicFilterDialog ( pParent, CUI_RES( RID_SVX_GRFFILTER_DLG_POSTER ), rGraphic ),
    maFtPoster      ( this, CUI_RES( DLG_FILTERPOSTER_FT_POSTER ) ),
    maNumPoster     ( this, CUI_RES( DLG_FILTERPOSTER_NUM_POSTER ) )
{
    FreeResource();

    maNumPoster.SetFirst( 2 );
    maNumPoster.SetLast( rGraphic.GetBitmapEx().GetBitCount() );
    maNumPoster.SetValue( nPosterCount );
    maNumPoster.SetModifyHdl( GetModifyHdl() );
    maNumPoster.GrabFocus();
}

// -----------------------------------------------------------------------------

GraphicFilterPoster::~GraphicFilterPoster()
{
}

// -----------------------------------------------------------------------------

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

// -----------------------
// - GraphicFilterEmboss -
// -----------------------

void GraphicFilterEmboss::EmbossControl::MouseButtonDown( const MouseEvent& rEvt )
{
    const RECT_POINT eOldRP = GetActualRP();

    SvxRectCtl::MouseButtonDown( rEvt );

    if( GetActualRP() != eOldRP )
        maModifyHdl.Call( this );
}

// -----------------------------------------------------------------------------

GraphicFilterEmboss::GraphicFilterEmboss( Window* pParent, const Graphic& rGraphic,
                                          RECT_POINT eLightSource ) :
    GraphicFilterDialog ( pParent, CUI_RES( RID_SVX_GRFFILTER_DLG_EMBOSS ), rGraphic ),
    maFtLight       ( this, CUI_RES( DLG_FILTEREMBOSS_FT_LIGHT ) ),
    maCtlLight      ( this, CUI_RES( DLG_FILTEREMBOSS_CTL_LIGHT ), eLightSource )
{
    FreeResource();

    maCtlLight.SetModifyHdl( GetModifyHdl() );
    maCtlLight.GrabFocus();
}

// -----------------------------------------------------------------------------

GraphicFilterEmboss::~GraphicFilterEmboss()
{
}

// -----------------------------------------------------------------------------

Graphic GraphicFilterEmboss::GetFilteredGraphic( const Graphic& rGraphic,
                                                 double /*fScaleX*/, double /*fScaleY*/ )
{
    Graphic aRet;
    sal_uInt16  nAzim, nElev;

    switch( maCtlLight.GetActualRP() )
    {
        default:       OSL_FAIL("svx::GraphicFilterEmboss::GetFilteredGraphic(), unknown Reference Point!" );
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
