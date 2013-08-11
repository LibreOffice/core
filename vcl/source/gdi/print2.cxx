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

#include <functional>
#include <algorithm>
#include <utility>
#include <list>
#include <vector>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#include <tools/debug.hxx>

#include <vcl/virdev.hxx>
#include <vcl/metaact.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/print.hxx>
#include <vcl/svapp.hxx>
#include <vcl/bmpacc.hxx>

#include <print.h>

#include "pdfwriter_impl.hxx"

#define MAX_TILE_WIDTH  1024
#define MAX_TILE_HEIGHT 1024

typedef ::std::pair< MetaAction*, int > Component; // MetaAction plus index in metafile

typedef ::std::list< Component > ComponentList;

// List of (intersecting) actions, plus overall bounds
struct ConnectedComponents
{
    ConnectedComponents() :
        aComponentList(),
        aBounds(),
        aBgColor(COL_WHITE),
        bIsSpecial(false),
        bIsFullyTransparent(false)
    {}

    ComponentList   aComponentList;
    Rectangle       aBounds;
    Color           aBgColor;
    bool            bIsSpecial;
    bool            bIsFullyTransparent;
};

typedef ::std::list< ConnectedComponents > ConnectedComponentsList;

/** #i10613# Extracted from Printer::GetPreparedMetaFile. Returns true
    if given action requires special handling (usually because of
    transparency)
*/
static bool ImplIsActionSpecial( const MetaAction& rAct )
{
    switch( rAct.GetType() )
    {
        case META_TRANSPARENT_ACTION:
            return true;

        case META_FLOATTRANSPARENT_ACTION:
            return true;

        case META_BMPEX_ACTION:
            return static_cast<const MetaBmpExAction&>(rAct).GetBitmapEx().IsTransparent();

        case META_BMPEXSCALE_ACTION:
            return static_cast<const MetaBmpExScaleAction&>(rAct).GetBitmapEx().IsTransparent();

        case META_BMPEXSCALEPART_ACTION:
            return static_cast<const MetaBmpExScalePartAction&>(rAct).GetBitmapEx().IsTransparent();

        default:
            return false;
    }
}

/** Check whether rCurrRect rectangle fully covers io_rPrevRect - if
    yes, return true and update o_rBgColor
 */
static bool checkRect( Rectangle&       io_rPrevRect,
                       Color&           o_rBgColor,
                       const Rectangle& rCurrRect,
                       OutputDevice&    rMapModeVDev )
{
    // shape needs to fully cover previous content, and have uniform
    // color
    const bool bRet(
        rMapModeVDev.LogicToPixel(rCurrRect).IsInside(io_rPrevRect) &&
        rMapModeVDev.IsFillColor() );

    if( bRet )
    {
        io_rPrevRect = rCurrRect;
        o_rBgColor = rMapModeVDev.GetFillColor();
    }

    return bRet;
}

/** #107169# Convert BitmapEx to Bitmap with appropriately blended
    color. Convert MetaTransparentAction to plain polygon,
    appropriately colored

    @param o_rMtf
    Add converted actions to this metafile
*/
static void ImplConvertTransparentAction( GDIMetaFile&        o_rMtf,
                                          const MetaAction&   rAct,
                                          const OutputDevice& rStateOutDev,
                                          Color               aBgColor )
{
    if( rAct.GetType() == META_TRANSPARENT_ACTION )
    {
        const MetaTransparentAction* pTransAct = static_cast<const MetaTransparentAction*>(&rAct);
        sal_uInt16                       nTransparency( pTransAct->GetTransparence() );

        // #i10613# Respect transparency for draw color
        if( nTransparency )
        {
            o_rMtf.AddAction( new MetaPushAction( PUSH_LINECOLOR|PUSH_FILLCOLOR ) );

            // assume white background for alpha blending
            Color aLineColor( rStateOutDev.GetLineColor() );
            aLineColor.SetRed( static_cast<sal_uInt8>( (255L*nTransparency + (100L - nTransparency)*aLineColor.GetRed()) / 100L ) );
            aLineColor.SetGreen( static_cast<sal_uInt8>( (255L*nTransparency + (100L - nTransparency)*aLineColor.GetGreen()) / 100L ) );
            aLineColor.SetBlue( static_cast<sal_uInt8>( (255L*nTransparency + (100L - nTransparency)*aLineColor.GetBlue()) / 100L ) );
            o_rMtf.AddAction( new MetaLineColorAction(aLineColor, sal_True) );

            Color aFillColor( rStateOutDev.GetFillColor() );
            aFillColor.SetRed( static_cast<sal_uInt8>( (255L*nTransparency + (100L - nTransparency)*aFillColor.GetRed()) / 100L ) );
            aFillColor.SetGreen( static_cast<sal_uInt8>( (255L*nTransparency + (100L - nTransparency)*aFillColor.GetGreen()) / 100L ) );
            aFillColor.SetBlue( static_cast<sal_uInt8>( (255L*nTransparency + (100L - nTransparency)*aFillColor.GetBlue()) / 100L ) );
            o_rMtf.AddAction( new MetaFillColorAction(aFillColor, sal_True) );
        }

        o_rMtf.AddAction( new MetaPolyPolygonAction(pTransAct->GetPolyPolygon()) );

        if( nTransparency )
            o_rMtf.AddAction( new MetaPopAction() );
    }
    else
    {
        BitmapEx aBmpEx;

        switch( rAct.GetType() )
        {
            case META_BMPEX_ACTION:
                aBmpEx = static_cast<const MetaBmpExAction&>(rAct).GetBitmapEx();
                break;

            case META_BMPEXSCALE_ACTION:
                aBmpEx = static_cast<const MetaBmpExScaleAction&>(rAct).GetBitmapEx();
                break;

            case META_BMPEXSCALEPART_ACTION:
                aBmpEx = static_cast<const MetaBmpExScaleAction&>(rAct).GetBitmapEx();
                break;

            case META_TRANSPARENT_ACTION:

            default:
                OSL_FAIL("Printer::GetPreparedMetafile impossible state reached");
                break;
        }

        Bitmap aBmp( aBmpEx.GetBitmap() );
        if( !aBmpEx.IsAlpha() )
        {
            // blend with mask
            BitmapReadAccess* pRA = aBmp.AcquireReadAccess();

            if( !pRA )
                return; // what else should I do?

            Color aActualColor( aBgColor );

            if( pRA->HasPalette() )
                aActualColor = pRA->GetBestPaletteColor( aBgColor ).operator Color();

            aBmp.ReleaseAccess(pRA);

            // did we get true white?
            if( aActualColor.GetColorError( aBgColor ) )
            {
                // no, create truecolor bitmap, then
                aBmp.Convert( BMP_CONVERSION_24BIT );

                // fill masked out areas white
                aBmp.Replace( aBmpEx.GetMask(), aBgColor );
            }
            else
            {
                // fill masked out areas white
                aBmp.Replace( aBmpEx.GetMask(), aActualColor );
            }
        }
        else
        {
            // blend with alpha channel
            aBmp.Convert( BMP_CONVERSION_24BIT );
            aBmp.Blend(aBmpEx.GetAlpha(),aBgColor);
        }

        // add corresponding action
        switch( rAct.GetType() )
        {
            case META_BMPEX_ACTION:
                o_rMtf.AddAction( new MetaBmpAction(
                                       static_cast<const MetaBmpExAction&>(rAct).GetPoint(),
                                       aBmp ));
                break;
            case META_BMPEXSCALE_ACTION:
                o_rMtf.AddAction( new MetaBmpScaleAction(
                                       static_cast<const MetaBmpExScaleAction&>(rAct).GetPoint(),
                                       static_cast<const MetaBmpExScaleAction&>(rAct).GetSize(),
                                       aBmp ));
                break;
            case META_BMPEXSCALEPART_ACTION:
                o_rMtf.AddAction( new MetaBmpScalePartAction(
                                       static_cast<const MetaBmpExScalePartAction&>(rAct).GetDestPoint(),
                                       static_cast<const MetaBmpExScalePartAction&>(rAct).GetDestSize(),
                                       static_cast<const MetaBmpExScalePartAction&>(rAct).GetSrcPoint(),
                                       static_cast<const MetaBmpExScalePartAction&>(rAct).GetSrcSize(),
                                       aBmp ));
                break;
            default:
                OSL_FAIL("Unexpected case");
                break;
        }
    }
}

// #i10613# Extracted from ImplCheckRect::ImplCreate
// Returns true, if given action creates visible (i.e. non-transparent) output
static bool ImplIsNotTransparent( const MetaAction& rAct, const OutputDevice& rOut )
{
    const bool  bLineTransparency( rOut.IsLineColor() ? rOut.GetLineColor().GetTransparency() == 255 : true );
    const bool  bFillTransparency( rOut.IsFillColor() ? rOut.GetFillColor().GetTransparency() == 255 : true );
    bool        bRet( false );

    switch( rAct.GetType() )
    {
        case META_POINT_ACTION:
            if( !bLineTransparency )
                bRet = true;
            break;

        case META_LINE_ACTION:
            if( !bLineTransparency )
                bRet = true;
            break;

        case META_RECT_ACTION:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case META_ROUNDRECT_ACTION:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case META_ELLIPSE_ACTION:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case META_ARC_ACTION:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case META_PIE_ACTION:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case META_CHORD_ACTION:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case META_POLYLINE_ACTION:
            if( !bLineTransparency )
                bRet = true;
            break;

        case META_POLYGON_ACTION:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case META_POLYPOLYGON_ACTION:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case META_TEXT_ACTION:
        {
            const MetaTextAction& rTextAct = static_cast<const MetaTextAction&>(rAct);
            const OUString aString( rTextAct.GetText().copy(rTextAct.GetIndex(), rTextAct.GetLen()) );
            if (!aString.isEmpty())
                bRet = true;
        }
        break;

        case META_TEXTARRAY_ACTION:
        {
            const MetaTextArrayAction& rTextAct = static_cast<const MetaTextArrayAction&>(rAct);
            const OUString aString( rTextAct.GetText().copy(rTextAct.GetIndex(), rTextAct.GetLen()) );
            if (!aString.isEmpty())
                bRet = true;
        }
        break;

        case META_PIXEL_ACTION:
        case META_BMP_ACTION:
        case META_BMPSCALE_ACTION:
        case META_BMPSCALEPART_ACTION:
        case META_BMPEX_ACTION:
        case META_BMPEXSCALE_ACTION:
        case META_BMPEXSCALEPART_ACTION:
        case META_MASK_ACTION:
        case META_MASKSCALE_ACTION:
        case META_MASKSCALEPART_ACTION:
        case META_GRADIENT_ACTION:
        case META_GRADIENTEX_ACTION:
        case META_HATCH_ACTION:
        case META_WALLPAPER_ACTION:
        case META_TRANSPARENT_ACTION:
        case META_FLOATTRANSPARENT_ACTION:
        case META_EPS_ACTION:
        case META_TEXTRECT_ACTION:
        case META_STRETCHTEXT_ACTION:
        case META_TEXTLINE_ACTION:
            // all other actions: generate non-transparent output
            bRet = true;
            break;

        default:
            break;
    }

    return bRet;
}

// #i10613# Extracted from ImplCheckRect::ImplCreate
static Rectangle ImplCalcActionBounds( const MetaAction& rAct, const OutputDevice& rOut )
{
    Rectangle aActionBounds;

    switch( rAct.GetType() )
    {
        case META_PIXEL_ACTION:
            aActionBounds = Rectangle( static_cast<const MetaPixelAction&>(rAct).GetPoint(), Size( 1, 1 ) );
            break;

        case META_POINT_ACTION:
            aActionBounds = Rectangle( static_cast<const MetaPointAction&>(rAct).GetPoint(), Size( 1, 1 ) );
            break;

        case META_LINE_ACTION:
        {
            const MetaLineAction& rMetaLineAction = static_cast<const MetaLineAction&>(rAct);
            aActionBounds = Rectangle( rMetaLineAction.GetStartPoint(),  rMetaLineAction.GetEndPoint() );
            aActionBounds.Justify();
            const long nLineWidth(rMetaLineAction.GetLineInfo().GetWidth());
            if(nLineWidth)
            {
                const long nHalfLineWidth((nLineWidth + 1) / 2);
                aActionBounds.Left() -= nHalfLineWidth;
                aActionBounds.Top() -= nHalfLineWidth;
                aActionBounds.Right() += nHalfLineWidth;
                aActionBounds.Bottom() += nHalfLineWidth;
            }
            break;
        }

        case META_RECT_ACTION:
            aActionBounds = static_cast<const MetaRectAction&>(rAct).GetRect();
            break;

        case META_ROUNDRECT_ACTION:
            aActionBounds = Polygon( static_cast<const MetaRoundRectAction&>(rAct).GetRect(),
                                     static_cast<const MetaRoundRectAction&>(rAct).GetHorzRound(),
                                     static_cast<const MetaRoundRectAction&>(rAct).GetVertRound() ).GetBoundRect();
            break;

        case META_ELLIPSE_ACTION:
        {
            const Rectangle& rRect = static_cast<const MetaEllipseAction&>(rAct).GetRect();
            aActionBounds = Polygon( rRect.Center(),
                                     rRect.GetWidth() >> 1,
                                     rRect.GetHeight() >> 1 ).GetBoundRect();
            break;
        }

        case META_ARC_ACTION:
            aActionBounds = Polygon( static_cast<const MetaArcAction&>(rAct).GetRect(),
                                     static_cast<const MetaArcAction&>(rAct).GetStartPoint(),
                                     static_cast<const MetaArcAction&>(rAct).GetEndPoint(), POLY_ARC ).GetBoundRect();
            break;

        case META_PIE_ACTION:
            aActionBounds = Polygon( static_cast<const MetaPieAction&>(rAct).GetRect(),
                                     static_cast<const MetaPieAction&>(rAct).GetStartPoint(),
                                     static_cast<const MetaPieAction&>(rAct).GetEndPoint(), POLY_PIE ).GetBoundRect();
            break;

        case META_CHORD_ACTION:
            aActionBounds = Polygon( static_cast<const MetaChordAction&>(rAct).GetRect(),
                                     static_cast<const MetaChordAction&>(rAct).GetStartPoint(),
                                     static_cast<const MetaChordAction&>(rAct).GetEndPoint(), POLY_CHORD ).GetBoundRect();
            break;

        case META_POLYLINE_ACTION:
        {
            const MetaPolyLineAction& rMetaPolyLineAction = static_cast<const MetaPolyLineAction&>(rAct);
            aActionBounds = rMetaPolyLineAction.GetPolygon().GetBoundRect();
            const long nLineWidth(rMetaPolyLineAction.GetLineInfo().GetWidth());
            if(nLineWidth)
            {
                const long nHalfLineWidth((nLineWidth + 1) / 2);
                aActionBounds.Left() -= nHalfLineWidth;
                aActionBounds.Top() -= nHalfLineWidth;
                aActionBounds.Right() += nHalfLineWidth;
                aActionBounds.Bottom() += nHalfLineWidth;
            }
            break;
        }

        case META_POLYGON_ACTION:
            aActionBounds = static_cast<const MetaPolygonAction&>(rAct).GetPolygon().GetBoundRect();
            break;

        case META_POLYPOLYGON_ACTION:
            aActionBounds = static_cast<const MetaPolyPolygonAction&>(rAct).GetPolyPolygon().GetBoundRect();
            break;

        case META_BMP_ACTION:
            aActionBounds = Rectangle( static_cast<const MetaBmpAction&>(rAct).GetPoint(),
                                       rOut.PixelToLogic( static_cast<const MetaBmpAction&>(rAct).GetBitmap().GetSizePixel() ) );
            break;

        case META_BMPSCALE_ACTION:
            aActionBounds = Rectangle( static_cast<const MetaBmpScaleAction&>(rAct).GetPoint(),
                                       static_cast<const MetaBmpScaleAction&>(rAct).GetSize() );
            break;

        case META_BMPSCALEPART_ACTION:
            aActionBounds = Rectangle( static_cast<const MetaBmpScalePartAction&>(rAct).GetDestPoint(),
                                       static_cast<const MetaBmpScalePartAction&>(rAct).GetDestSize() );
            break;

        case META_BMPEX_ACTION:
            aActionBounds = Rectangle( static_cast<const MetaBmpExAction&>(rAct).GetPoint(),
                                       rOut.PixelToLogic( static_cast<const MetaBmpExAction&>(rAct).GetBitmapEx().GetSizePixel() ) );
            break;

        case META_BMPEXSCALE_ACTION:
            aActionBounds = Rectangle( static_cast<const MetaBmpExScaleAction&>(rAct).GetPoint(),
                                       static_cast<const MetaBmpExScaleAction&>(rAct).GetSize() );
            break;

        case META_BMPEXSCALEPART_ACTION:
            aActionBounds = Rectangle( static_cast<const MetaBmpExScalePartAction&>(rAct).GetDestPoint(),
                                       static_cast<const MetaBmpExScalePartAction&>(rAct).GetDestSize() );
            break;

        case META_MASK_ACTION:
            aActionBounds = Rectangle( static_cast<const MetaMaskAction&>(rAct).GetPoint(),
                                       rOut.PixelToLogic( static_cast<const MetaMaskAction&>(rAct).GetBitmap().GetSizePixel() ) );
            break;

        case META_MASKSCALE_ACTION:
            aActionBounds = Rectangle( static_cast<const MetaMaskScaleAction&>(rAct).GetPoint(),
                                       static_cast<const MetaMaskScaleAction&>(rAct).GetSize() );
            break;

        case META_MASKSCALEPART_ACTION:
            aActionBounds = Rectangle( static_cast<const MetaMaskScalePartAction&>(rAct).GetDestPoint(),
                                       static_cast<const MetaMaskScalePartAction&>(rAct).GetDestSize() );
            break;

        case META_GRADIENT_ACTION:
            aActionBounds = static_cast<const MetaGradientAction&>(rAct).GetRect();
            break;

        case META_GRADIENTEX_ACTION:
            aActionBounds = static_cast<const MetaGradientExAction&>(rAct).GetPolyPolygon().GetBoundRect();
            break;

        case META_HATCH_ACTION:
            aActionBounds = static_cast<const MetaHatchAction&>(rAct).GetPolyPolygon().GetBoundRect();
            break;

        case META_WALLPAPER_ACTION:
            aActionBounds = static_cast<const MetaWallpaperAction&>(rAct).GetRect();
            break;

        case META_TRANSPARENT_ACTION:
            aActionBounds = static_cast<const MetaTransparentAction&>(rAct).GetPolyPolygon().GetBoundRect();
            break;

        case META_FLOATTRANSPARENT_ACTION:
            aActionBounds = Rectangle( static_cast<const MetaFloatTransparentAction&>(rAct).GetPoint(),
                                       static_cast<const MetaFloatTransparentAction&>(rAct).GetSize() );
            break;

        case META_EPS_ACTION:
            aActionBounds = Rectangle( static_cast<const MetaEPSAction&>(rAct).GetPoint(),
                                       static_cast<const MetaEPSAction&>(rAct).GetSize() );
            break;

        case META_TEXT_ACTION:
        {
            const MetaTextAction& rTextAct = static_cast<const MetaTextAction&>(rAct);
            const OUString aString( rTextAct.GetText().copy(rTextAct.GetIndex(), rTextAct.GetLen()) );

            if (!aString.isEmpty())
            {
                const Point aPtLog( rTextAct.GetPoint() );

                // #105987# Use API method instead of Impl* methods
                // #107490# Set base parameter equal to index parameter
                rOut.GetTextBoundRect( aActionBounds, rTextAct.GetText(), rTextAct.GetIndex(),
                                       rTextAct.GetIndex(), rTextAct.GetLen() );
                aActionBounds.Move( aPtLog.X(), aPtLog.Y() );
            }
        }
        break;

        case META_TEXTARRAY_ACTION:
        {
            const MetaTextArrayAction&  rTextAct = static_cast<const MetaTextArrayAction&>(rAct);
            const OUString              aString( rTextAct.GetText().copy(rTextAct.GetIndex(), rTextAct.GetLen()) );
            const sal_Int32             nLen = aString.getLength();

            if( nLen )
            {
                // #105987# ImplLayout takes everything in logical coordinates
                SalLayout* pSalLayout = rOut.ImplLayout( rTextAct.GetText(), rTextAct.GetIndex(),
                                                         rTextAct.GetLen(), rTextAct.GetPoint(),
                                                         0, rTextAct.GetDXArray() );
                if( pSalLayout )
                {
                    Rectangle aBoundRect( const_cast<OutputDevice&>(rOut).ImplGetTextBoundRect( *pSalLayout ) );
                    aActionBounds = rOut.PixelToLogic( aBoundRect );
                    pSalLayout->Release();
                }
            }
        }
        break;

        case META_TEXTRECT_ACTION:
            aActionBounds = static_cast<const MetaTextRectAction&>(rAct).GetRect();
            break;

        case META_STRETCHTEXT_ACTION:
        {
            const MetaStretchTextAction& rTextAct = static_cast<const MetaStretchTextAction&>(rAct);
            const OUString               aString( rTextAct.GetText().copy(rTextAct.GetIndex(), rTextAct.GetLen()) );
            const sal_Int32              nLen = aString.getLength();

            // #i16195# Literate copy from TextArray action, the
            // semantics for the ImplLayout call are copied from the
            // OutDev::DrawStretchText() code. Unfortunately, also in
            // this case, public outdev methods such as GetTextWidth()
            // don't provide enough info.
            if( nLen )
            {
                // #105987# ImplLayout takes everything in logical coordinates
                SalLayout* pSalLayout = rOut.ImplLayout( rTextAct.GetText(), rTextAct.GetIndex(),
                                                         rTextAct.GetLen(), rTextAct.GetPoint(),
                                                         rTextAct.GetWidth() );
                if( pSalLayout )
                {
                    Rectangle aBoundRect( const_cast<OutputDevice&>(rOut).ImplGetTextBoundRect( *pSalLayout ) );
                    aActionBounds = rOut.PixelToLogic( aBoundRect );
                    pSalLayout->Release();
                }
            }
        }
        break;

        case META_TEXTLINE_ACTION:
            OSL_FAIL("META_TEXTLINE_ACTION not supported");
        break;

        default:
            break;
    }

    if( !aActionBounds.IsEmpty() )
    {
        // fdo#40421 limit current action's output to clipped area
        if( rOut.IsClipRegion() )
            return rOut.LogicToPixel(
                rOut.GetClipRegion().GetBoundRect().Intersection( aActionBounds ) );
        else
            return rOut.LogicToPixel( aActionBounds );
    }
    else
        return Rectangle();
}

static bool ImplIsActionHandlingTransparency( const MetaAction& rAct )
{
    // META_FLOATTRANSPARENT_ACTION can contain a whole metafile,
    // which is to be rendered with the given transparent gradient. We
    // currently cannot emulate transparent painting on a white
    // background reliably.

    // the remainder can handle printing itself correctly on a uniform
    // white background.
    switch( rAct.GetType() )
    {
        case META_TRANSPARENT_ACTION:
        case META_BMPEX_ACTION:
        case META_BMPEXSCALE_ACTION:
        case META_BMPEXSCALEPART_ACTION:
            return true;

        default:
            return false;
    }
}

bool OutputDevice::RemoveTransparenciesFromMetaFile( const GDIMetaFile& rInMtf, GDIMetaFile& rOutMtf,
                                                     long nMaxBmpDPIX, long nMaxBmpDPIY,
                                                     bool bReduceTransparency, bool bTransparencyAutoMode,
                                                     bool bDownsampleBitmaps,
                                                     const Color& rBackground
                                                     )
{
    MetaAction*             pCurrAct;
    bool                    bTransparent( false );

    rOutMtf.Clear();

    if( ! bReduceTransparency || bTransparencyAutoMode )
    {
        // watch for transparent drawing actions
        for( pCurrAct = ( (GDIMetaFile&) rInMtf ).FirstAction();
             pCurrAct && !bTransparent;
             pCurrAct = ( (GDIMetaFile&) rInMtf ).NextAction() )
        {
            // #i10613# Extracted "specialness" predicate into extra method

            // #107169# Also examine metafiles with masked bitmaps in
            // detail. Further down, this is optimized in such a way
            // that there's no unnecessary painting of masked bitmaps
            // (which are _always_ subdivided into rectangular regions
            // of uniform opacity): if a masked bitmap is printed over
            // empty background, we convert to a plain bitmap with
            // white background.
            if( ImplIsActionSpecial( *pCurrAct ) )
            {
                bTransparent = true;
            }
        }
    }

    // #i10613# Determine set of connected components containing transparent objects. These are
    // then processed as bitmaps, the original actions are removed from the metafile.
    if( !bTransparent )
    {
        // nothing transparent -> just copy
        rOutMtf = rInMtf;
    }
    else
    {
        // #i10613#
        // This works as follows: we want a number of distinct sets of
        // connected components, where each set contains metafile
        // actions that are intersecting (note: there are possibly
        // more actions contained as are directly intersecting,
        // because we can only produce rectangular bitmaps later
        // on. Thus, each set of connected components is the smallest
        // enclosing, axis-aligned rectangle that completely bounds a
        // number of intersecting metafile actions, plus any action
        // that would otherwise be cut in two). Therefore, we
        // iteratively add metafile actions from the original metafile
        // to this connected components list (aCCList), by checking
        // each element's bounding box against intersection with the
        // metaaction at hand.
        // All those intersecting elements are removed from aCCList
        // and collected in a temporary list (aCCMergeList). After all
        // elements have been checked, the aCCMergeList elements are
        // merged with the metaaction at hand into one resulting
        // connected component, with one big bounding box, and
        // inserted into aCCList again.
        // The time complexity of this algorithm is O(n^3), where n is
        // the number of metafile actions, and it finds all distinct
        // regions of rectangle-bounded connected components. This
        // algorithm was designed by AF.
        //

        //
        //  STAGE 1: Detect background
        //  ==========================
        //

        // Receives uniform background content, and is _not_ merged
        // nor checked for intersection against other aCCList elements
        ConnectedComponents aBackgroundComponent;

        // create an OutputDevice to record mapmode changes and the like
        VirtualDevice aMapModeVDev;
        aMapModeVDev.mnDPIX = mnDPIX;
        aMapModeVDev.mnDPIY = mnDPIY;
        aMapModeVDev.EnableOutput(sal_False);

        int nLastBgAction, nActionNum;

        // weed out page-filling background objects (if they are
        // uniformly coloured). Keeping them outside the other
        // connected components often prevents whole-page bitmap
        // generation.
        bool bStillBackground=true; // true until first non-bg action
        nActionNum=0; nLastBgAction=-1;
        pCurrAct=const_cast<GDIMetaFile&>(rInMtf).FirstAction();
        if( rBackground != Color( COL_TRANSPARENT ) )
        {
            aBackgroundComponent.aBgColor = rBackground;
            if( meOutDevType == OUTDEV_PRINTER )
            {
                Printer* pThis = dynamic_cast<Printer*>(this);
                Point aPageOffset = pThis->GetPageOffsetPixel();
                aPageOffset = Point( 0, 0 ) - aPageOffset;
                Size aSize  = pThis->GetPaperSizePixel();
                aBackgroundComponent.aBounds = Rectangle( aPageOffset, aSize );
            }
            else
                aBackgroundComponent.aBounds = Rectangle( Point( 0, 0 ), GetOutputSizePixel() );
        }
        while( pCurrAct && bStillBackground )
        {
            switch( pCurrAct->GetType() )
            {
                case META_RECT_ACTION:
                {
                    if( !checkRect(
                            aBackgroundComponent.aBounds,
                            aBackgroundComponent.aBgColor,
                            static_cast<const MetaRectAction*>(pCurrAct)->GetRect(),
                            aMapModeVDev) )
                        bStillBackground=false; // incomplete occlusion of background
                    else
                        nLastBgAction=nActionNum; // this _is_ background
                    break;
                }
                case META_POLYGON_ACTION:
                {
                    const Polygon aPoly(
                        static_cast<const MetaPolygonAction*>(pCurrAct)->GetPolygon());
                    if( !basegfx::tools::isRectangle(
                            aPoly.getB2DPolygon()) ||
                        !checkRect(
                            aBackgroundComponent.aBounds,
                            aBackgroundComponent.aBgColor,
                            aPoly.GetBoundRect(),
                            aMapModeVDev) )
                        bStillBackground=false; // incomplete occlusion of background
                    else
                        nLastBgAction=nActionNum; // this _is_ background
                    break;
                }
                case META_POLYPOLYGON_ACTION:
                {
                    const PolyPolygon aPoly(
                        static_cast<const MetaPolyPolygonAction*>(pCurrAct)->GetPolyPolygon());
                    if( aPoly.Count() != 1 ||
                        !basegfx::tools::isRectangle(
                            aPoly[0].getB2DPolygon()) ||
                        !checkRect(
                            aBackgroundComponent.aBounds,
                            aBackgroundComponent.aBgColor,
                            aPoly.GetBoundRect(),
                            aMapModeVDev) )
                        bStillBackground=false; // incomplete occlusion of background
                    else
                        nLastBgAction=nActionNum; // this _is_ background
                    break;
                }
                case META_WALLPAPER_ACTION:
                {
                    if( !checkRect(
                            aBackgroundComponent.aBounds,
                            aBackgroundComponent.aBgColor,
                            static_cast<const MetaWallpaperAction*>(pCurrAct)->GetRect(),
                            aMapModeVDev) )
                        bStillBackground=false; // incomplete occlusion of background
                    else
                        nLastBgAction=nActionNum; // this _is_ background
                    break;
                }
                default:
                {
                    if( ImplIsNotTransparent( *pCurrAct,
                                              aMapModeVDev ) )
                        bStillBackground=false; // non-transparent action, possibly
                                                // not uniform
                    else
                        // extend current bounds (next uniform action
                        // needs to fully cover this area)
                        aBackgroundComponent.aBounds.Union(
                            ImplCalcActionBounds(*pCurrAct, aMapModeVDev) );
                    break;
                }
            }

            // execute action to get correct MapModes etc.
            pCurrAct->Execute( &aMapModeVDev );

            pCurrAct=const_cast<GDIMetaFile&>(rInMtf).NextAction();
            ++nActionNum;
        }

        // clean up aMapModeVDev
        sal_uInt32 nCount = aMapModeVDev.GetGCStackDepth();
        while( nCount-- )
            aMapModeVDev.Pop();

        ConnectedComponentsList aCCList; // list containing distinct sets of connected components as elements.

        // fast-forward until one after the last background action
        // (need to reconstruct map mode vdev state)
        nActionNum=0;
        pCurrAct=const_cast<GDIMetaFile&>(rInMtf).FirstAction();
        while( pCurrAct && nActionNum<=nLastBgAction )
        {
            // up to and including last ink-generating background
            // action go to background component
            aBackgroundComponent.aComponentList.push_back(
                ::std::make_pair(
                    pCurrAct, nActionNum) );

            // execute action to get correct MapModes etc.
            pCurrAct->Execute( &aMapModeVDev );
            pCurrAct=const_cast<GDIMetaFile&>(rInMtf).NextAction();
            ++nActionNum;
        }

        //
        //  STAGE 2: Generate connected components list
        //  ===========================================
        //

        // iterate over all actions (start where background action
        // search left off)
        for( ;
             pCurrAct;
             pCurrAct=const_cast<GDIMetaFile&>(rInMtf).NextAction(), ++nActionNum )
        {
            // execute action to get correct MapModes etc.
            pCurrAct->Execute( &aMapModeVDev );

            // cache bounds of current action
            const Rectangle aBBCurrAct( ImplCalcActionBounds(*pCurrAct, aMapModeVDev) );

            // accumulate collected bounds here, initialize with current action
            Rectangle                               aTotalBounds( aBBCurrAct ); // thus,
                                                                                // aTotalComponents.aBounds
                                                                                // is
                                                                                // empty
                                                                                // for
                                                                                // non-output-generating
                                                                                // actions
            bool                                    bTreatSpecial( false );
            ConnectedComponents                     aTotalComponents;

            //
            //  STAGE 2.1: Search for intersecting cc entries
            //  =============================================
            //

            // if aBBCurrAct is empty, it will intersect with no
            // aCCList member. Thus, we can save the check.
            // Furthermore, this ensures that non-output-generating
            // actions get their own aCCList entry, which is necessary
            // when copying them to the output metafile (see stage 4
            // below).

            // #107169# Wholly transparent objects need
            // not be considered for connected components,
            // too. Just put each of them into a separate
            // component.
            aTotalComponents.bIsFullyTransparent = !ImplIsNotTransparent(*pCurrAct, aMapModeVDev);

            if( !aBBCurrAct.IsEmpty() &&
                !aTotalComponents.bIsFullyTransparent )
            {
                if( !aBackgroundComponent.aComponentList.empty() &&
                    !aBackgroundComponent.aBounds.IsInside(aTotalBounds) )
                {
                    // it seems the background is not large enough. to
                    // be on the safe side, combine with this component.
                    aTotalBounds.Union( aBackgroundComponent.aBounds );

                    // extract all aCurr actions to aTotalComponents
                    aTotalComponents.aComponentList.splice( aTotalComponents.aComponentList.end(),
                                                            aBackgroundComponent.aComponentList );

                    if( aBackgroundComponent.bIsSpecial )
                        bTreatSpecial = true;
                }

                ConnectedComponentsList::iterator       aCurrCC;
                const ConnectedComponentsList::iterator aLastCC( aCCList.end() );
                bool                                    bSomeComponentsChanged;

                // now, this is unfortunate: since changing anyone of
                // the aCCList elements (e.g. by merging or addition
                // of an action) might generate new intersection with
                // other aCCList elements, have to repeat the whole
                // element scanning, until nothing changes anymore.
                // Thus, this loop here makes us O(n^3) in the worst
                // case.
                do
                {
                    // only loop here if 'intersects' branch below was hit
                    bSomeComponentsChanged = false;

                    // iterate over all current members of aCCList
                    for( aCurrCC=aCCList.begin(); aCurrCC != aLastCC; )
                    {
                        // first check if current element's bounds are
                        // empty. This ensures that empty actions are not
                        // merged into one component, as a matter of fact,
                        // they have no position.

                        // #107169# Wholly transparent objects need
                        // not be considered for connected components,
                        // too. Just put each of them into a separate
                        // component.
                        if( !aCurrCC->aBounds.IsEmpty() &&
                            !aCurrCC->bIsFullyTransparent &&
                            aCurrCC->aBounds.IsOver( aTotalBounds ) )
                        {
                            // union the intersecting aCCList element into aTotalComponents

                            // calc union bounding box
                            aTotalBounds.Union( aCurrCC->aBounds );

                            // extract all aCurr actions to aTotalComponents
                            aTotalComponents.aComponentList.splice( aTotalComponents.aComponentList.end(),
                                                                    aCurrCC->aComponentList );

                            if( aCurrCC->bIsSpecial )
                                bTreatSpecial = true;

                            // remove and delete aCurrCC element from list (we've now merged its content)
                            aCurrCC = aCCList.erase( aCurrCC );

                            // at least one component changed, need to rescan everything
                            bSomeComponentsChanged = true;
                        }
                        else
                        {
                            ++aCurrCC;
                        }
                    }
                }
                while( bSomeComponentsChanged );
            }

            //
            //  STAGE 2.2: Determine special state for cc element
            //  =================================================
            //

            // now test whether the whole connected component must be
            // treated specially (i.e. rendered as a bitmap): if the
            // added action is the very first action, or all actions
            // before it are completely transparent, the connected
            // component need not be treated specially, not even if
            // the added action contains transparency. This is because
            // painting of transparent objects on _white background_
            // works without alpha compositing (you just calculate the
            // color). Note that for the test "all objects before me
            // are transparent" no sorting is necessary, since the
            // added metaaction pCurrAct is always in the order the
            // metafile is painted. Generally, the order of the
            // metaactions in the ConnectedComponents are not
            // guaranteed to be the same as in the metafile.
            if( bTreatSpecial )
            {
                // prev component(s) special -> this one, too
                aTotalComponents.bIsSpecial = true;
            }
            else if( !ImplIsActionSpecial( *pCurrAct ) )
            {
                // added action and none of prev components special ->
                // this one normal, too
                aTotalComponents.bIsSpecial = false;
            }
            else
            {
                // added action is special and none of prev components
                // special -> do the detailed tests

                // can the action handle transparency correctly
                // (i.e. when painted on white background, does the
                // action still look correct)?
                if( !ImplIsActionHandlingTransparency( *pCurrAct ) )
                {
                    // no, action cannot handle its transparency on
                    // a printer device, render to bitmap
                    aTotalComponents.bIsSpecial = true;
                }
                else
                {
                    // yes, action can handle its transparency, so
                    // check whether we're on white background
                    if( aTotalComponents.aComponentList.empty() )
                    {
                        // nothing between pCurrAct and page
                        // background -> don't be special
                        aTotalComponents.bIsSpecial = false;
                    }
                    else
                    {
                        // #107169# Fixes abnove now ensure that _no_
                        // object in the list is fully transparent. Thus,
                        // if the component list is not empty above, we
                        // must assume that we have to treat this
                        // component special.

                        // there are non-transparent objects between
                        // pCurrAct and the empty sheet of paper -> be
                        // special, then
                        aTotalComponents.bIsSpecial = true;
                    }
                }
            }


            //
            //  STAGE 2.3: Add newly generated CC list element
            //  ==============================================
            //

            // set new bounds and add action to list
            aTotalComponents.aBounds = aTotalBounds;
            aTotalComponents.aComponentList.push_back(
                ::std::make_pair(
                    pCurrAct, nActionNum) );

            // add aTotalComponents as a new entry to aCCList
            aCCList.push_back( aTotalComponents );

            DBG_ASSERT( !aTotalComponents.aComponentList.empty(),
                        "Printer::GetPreparedMetaFile empty component" );
            DBG_ASSERT( !aTotalComponents.aBounds.IsEmpty() ||
                        (aTotalComponents.aBounds.IsEmpty() && aTotalComponents.aComponentList.size() == 1),
                        "Printer::GetPreparedMetaFile non-output generating actions must be solitary");
            DBG_ASSERT( !aTotalComponents.bIsFullyTransparent ||
                        (aTotalComponents.bIsFullyTransparent && aTotalComponents.aComponentList.size() == 1),
                        "Printer::GetPreparedMetaFile fully transparent actions must be solitary");
        }

        // well now, we've got the list of disjunct connected
        // components. Now we've got to create a map, which contains
        // the corresponding aCCList element for every
        // metaaction. Later on, we always process the complete
        // metafile for each bitmap to be generated, but switch on
        // output only for actions contained in the then current
        // aCCList element. This ensures correct mapmode and attribute
        // settings for all cases.

        // maps mtf actions to CC list entries
        ::std::vector< const ConnectedComponents* > aCCList_MemberMap( rInMtf.GetActionSize() );

        // iterate over all aCCList members and their contained metaactions
        ConnectedComponentsList::iterator       aCurr( aCCList.begin() );
        const ConnectedComponentsList::iterator aLast( aCCList.end() );
        for( ; aCurr != aLast; ++aCurr )
        {
            ComponentList::iterator       aCurrentAction( aCurr->aComponentList.begin() );
            const ComponentList::iterator aLastAction( aCurr->aComponentList.end() );
            for( ; aCurrentAction != aLastAction; ++aCurrentAction )
            {
                // set pointer to aCCList element for corresponding index
                aCCList_MemberMap[ aCurrentAction->second ] = &(*aCurr);
            }
        }

        //
        //  STAGE 3.1: Output background mtf actions (if there are any)
        //  ===========================================================
        //

        ComponentList::iterator       aCurrAct( aBackgroundComponent.aComponentList.begin() );
        const ComponentList::iterator aLastAct( aBackgroundComponent.aComponentList.end() );
        for( ; aCurrAct != aLastAct; ++aCurrAct )
        {
            // simply add this action (above, we inserted the actions
            // starting at index 0 up to and including nLastBgAction)
            rOutMtf.AddAction( ( aCurrAct->first->Duplicate(), aCurrAct->first ) );
        }


        //
        //  STAGE 3.2: Generate banded bitmaps for special regions
        //  ====================================================
        //

        Point aPageOffset;
        Size aTmpSize( GetOutputSizePixel() );
        if( mpPDFWriter )
        {
            aTmpSize = mpPDFWriter->getCurPageSize();
            aTmpSize = LogicToPixel( aTmpSize, MapMode( MAP_POINT ) );

            // also add error code to PDFWriter
            mpPDFWriter->insertError( vcl::PDFWriter::Warning_Transparency_Converted );
        }
        else if( meOutDevType == OUTDEV_PRINTER )
        {
            Printer* pThis = dynamic_cast<Printer*>(this);
            aPageOffset = pThis->GetPageOffsetPixel();
            aPageOffset = Point( 0, 0 ) - aPageOffset;
            aTmpSize  = pThis->GetPaperSizePixel();
        }
        const Rectangle aOutputRect( aPageOffset, aTmpSize );
        bool bTiling = dynamic_cast<Printer*>(this) != NULL;

        // iterate over all aCCList members and generate bitmaps for the special ones
        for( aCurr = aCCList.begin(); aCurr != aLast; ++aCurr )
        {
            if( aCurr->bIsSpecial )
            {
                Rectangle aBoundRect( aCurr->aBounds );
                aBoundRect.Intersection( aOutputRect );

                const double fBmpArea( (double) aBoundRect.GetWidth() * aBoundRect.GetHeight() );
                const double fOutArea( (double) aOutputRect.GetWidth() * aOutputRect.GetHeight() );

                // check if output doesn't exceed given size
                if( bReduceTransparency && bTransparencyAutoMode && ( fBmpArea > ( 0.25 * fOutArea ) ) )
                {
                    // output normally. Therefore, we simply clear the
                    // special attribute, as everything non-special is
                    // copied to rOutMtf further below.
                    aCurr->bIsSpecial = false;
                }
                else
                {
                    // create new bitmap action first
                    if( aBoundRect.GetWidth() && aBoundRect.GetHeight() )
                    {
                        Point           aDstPtPix( aBoundRect.TopLeft() );
                        Size            aDstSzPix;

                        VirtualDevice   aMapVDev;   // here, we record only mapmode information
                        aMapVDev.EnableOutput(sal_False);

                        VirtualDevice   aPaintVDev; // into this one, we render.
                        aPaintVDev.SetBackground( aBackgroundComponent.aBgColor );

                        rOutMtf.AddAction( new MetaPushAction( PUSH_MAPMODE ) );
                        rOutMtf.AddAction( new MetaMapModeAction() );

                        aPaintVDev.SetDrawMode( GetDrawMode() );

                        while( aDstPtPix.Y() <= aBoundRect.Bottom() )
                        {
                            aDstPtPix.X() = aBoundRect.Left();
                            aDstSzPix = bTiling ? Size( MAX_TILE_WIDTH, MAX_TILE_HEIGHT ) : aBoundRect.GetSize();

                            if( ( aDstPtPix.Y() + aDstSzPix.Height() - 1L ) > aBoundRect.Bottom() )
                                aDstSzPix.Height() = aBoundRect.Bottom() - aDstPtPix.Y() + 1L;

                            while( aDstPtPix.X() <= aBoundRect.Right() )
                            {
                                if( ( aDstPtPix.X() + aDstSzPix.Width() - 1L ) > aBoundRect.Right() )
                                    aDstSzPix.Width() = aBoundRect.Right() - aDstPtPix.X() + 1L;

                                if( !Rectangle( aDstPtPix, aDstSzPix ).Intersection( aBoundRect ).IsEmpty() &&
                                    aPaintVDev.SetOutputSizePixel( aDstSzPix ) )
                                {
                                    aPaintVDev.Push();
                                    aMapVDev.Push();

                                    aMapVDev.mnDPIX = aPaintVDev.mnDPIX = mnDPIX;
                                    aMapVDev.mnDPIY = aPaintVDev.mnDPIY = mnDPIY;

                                    aPaintVDev.EnableOutput(sal_False);

                                    // iterate over all actions
                                    for( pCurrAct=const_cast<GDIMetaFile&>(rInMtf).FirstAction(), nActionNum=0;
                                         pCurrAct;
                                         pCurrAct=const_cast<GDIMetaFile&>(rInMtf).NextAction(), ++nActionNum )
                                    {
                                        // enable output only for
                                        // actions that are members of
                                        // the current aCCList element
                                        // (aCurr)
                                        if( aCCList_MemberMap[nActionNum] == &(*aCurr) )
                                            aPaintVDev.EnableOutput(sal_True);

                                        // but process every action
                                        const sal_uInt16 nType( pCurrAct->GetType() );

                                        if( META_MAPMODE_ACTION == nType )
                                        {
                                            pCurrAct->Execute( &aMapVDev );

                                            MapMode     aMtfMap( aMapVDev.GetMapMode() );
                                            const Point aNewOrg( aMapVDev.PixelToLogic( aDstPtPix ) );

                                            aMtfMap.SetOrigin( Point( -aNewOrg.X(), -aNewOrg.Y() ) );
                                            aPaintVDev.SetMapMode( aMtfMap );
                                        }
                                        else if( ( META_PUSH_ACTION == nType ) || ( META_POP_ACTION ) == nType )
                                        {
                                            pCurrAct->Execute( &aMapVDev );
                                            pCurrAct->Execute( &aPaintVDev );
                                        }
                                        else if( META_GRADIENT_ACTION == nType )
                                        {
                                            MetaGradientAction* pGradientAction = static_cast<MetaGradientAction*>(pCurrAct);
                                            Printer* pPrinter = dynamic_cast< Printer* >(this);
                                            if( pPrinter )
                                                pPrinter->DrawGradientEx( &aPaintVDev, pGradientAction->GetRect(), pGradientAction->GetGradient() );
                                            else
                                                DrawGradient( pGradientAction->GetRect(), pGradientAction->GetGradient() );
                                        }
                                        else
                                        {
                                            pCurrAct->Execute( &aPaintVDev );
                                        }

                                        if( !( nActionNum % 8 ) )
                                            Application::Reschedule();
                                    }

                                    const sal_Bool bOldMap = mbMap;
                                    mbMap = aPaintVDev.mbMap = sal_False;

                                    Bitmap aBandBmp( aPaintVDev.GetBitmap( Point(), aDstSzPix ) );

                                    // scale down bitmap, if requested
                                    if( bDownsampleBitmaps )
                                    {
                                        aBandBmp = GetDownsampledBitmap( aDstSzPix,
                                                                         Point(), aBandBmp.GetSizePixel(),
                                                                         aBandBmp, nMaxBmpDPIX, nMaxBmpDPIY );
                                    }

                                    rOutMtf.AddAction( new MetaCommentAction( "PRNSPOOL_TRANSPARENTBITMAP_BEGIN" ) );
                                    rOutMtf.AddAction( new MetaBmpScaleAction( aDstPtPix, aDstSzPix, aBandBmp ) );
                                    rOutMtf.AddAction( new MetaCommentAction( "PRNSPOOL_TRANSPARENTBITMAP_END" ) );

                                    aPaintVDev.mbMap = sal_True;
                                    mbMap = bOldMap;
                                    aMapVDev.Pop();
                                    aPaintVDev.Pop();
                                }

                                // overlapping bands to avoid missing lines (e.g. PostScript)
                                aDstPtPix.X() += aDstSzPix.Width();
                            }

                            // overlapping bands to avoid missing lines (e.g. PostScript)
                            aDstPtPix.Y() += aDstSzPix.Height();
                        }

                        rOutMtf.AddAction( new MetaPopAction() );
                    }
                }
            }
        }

        // clean up aMapModeVDev
        nCount = aMapModeVDev.GetGCStackDepth();
        while( nCount-- )
            aMapModeVDev.Pop();

        //
        //  STAGE 4: Copy actions to output metafile
        //  ========================================
        //

        // iterate over all actions and duplicate the ones not in a
        // special aCCList member into rOutMtf
        for( pCurrAct=const_cast<GDIMetaFile&>(rInMtf).FirstAction(), nActionNum=0;
             pCurrAct;
             pCurrAct=const_cast<GDIMetaFile&>(rInMtf).NextAction(), ++nActionNum )
        {
            const ConnectedComponents* pCurrAssociatedComponent = aCCList_MemberMap[nActionNum];

            // NOTE: This relies on the fact that map-mode or draw
            // mode changing actions are solitary aCCList elements and
            // have empty bounding boxes, see comment on stage 2.1
            // above
            if( pCurrAssociatedComponent &&
                (pCurrAssociatedComponent->aBounds.IsEmpty() ||
                 !pCurrAssociatedComponent->bIsSpecial) )
            {
                // #107169# Treat transparent bitmaps special, if they
                // are the first (or sole) action in their bounds
                // list. Note that we previously ensured that no
                // fully-transparent objects are before us here.
                if( ImplIsActionHandlingTransparency( *pCurrAct ) &&
                    pCurrAssociatedComponent->aComponentList.begin()->first == pCurrAct )
                {
                    // convert actions, where masked-out parts are of
                    // given background color
                    ImplConvertTransparentAction(rOutMtf,
                                                 *pCurrAct,
                                                 aMapModeVDev,
                                                 aBackgroundComponent.aBgColor);
                }
                else
                {
                    // simply add this action
                    rOutMtf.AddAction( ( pCurrAct->Duplicate(), pCurrAct ) );
                }

                pCurrAct->Execute(&aMapModeVDev);
            }
        }

        rOutMtf.SetPrefMapMode( rInMtf.GetPrefMapMode() );
        rOutMtf.SetPrefSize( rInMtf.GetPrefSize() );

#if OSL_DEBUG_LEVEL > 1
        // iterate over all aCCList members and generate rectangles for the bounding boxes
        rOutMtf.AddAction( new MetaFillColorAction( COL_WHITE, false ) );
        for( aCurr = aCCList.begin(); aCurr != aLast; ++aCurr )
        {
            if( aCurr->bIsSpecial )
                rOutMtf.AddAction( new MetaLineColorAction( COL_RED, true) );
            else
                rOutMtf.AddAction( new MetaLineColorAction( COL_BLUE, true) );

            rOutMtf.AddAction( new MetaRectAction( aMapModeVDev.PixelToLogic( aCurr->aBounds ) ) );
        }
#endif
    }
    return bTransparent;
}

Bitmap OutputDevice::GetDownsampledBitmap( const Size& rDstSz,
                                           const Point& rSrcPt, const Size& rSrcSz,
                                           const Bitmap& rBmp, long nMaxBmpDPIX, long nMaxBmpDPIY )
{
    Bitmap aBmp( rBmp );

    if( !aBmp.IsEmpty() )
    {
        Point           aPoint;
        const Rectangle aBmpRect( aPoint, aBmp.GetSizePixel() );
        Rectangle       aSrcRect( rSrcPt, rSrcSz );

        // do cropping if necessary
        if( aSrcRect.Intersection( aBmpRect ) != aBmpRect )
        {
            if( !aSrcRect.IsEmpty() )
                aBmp.Crop( aSrcRect );
            else
                aBmp.SetEmpty();
        }

        if( !aBmp.IsEmpty() )
        {
            // do downsampling if necessary
            Size aDstSizeTwip( PixelToLogic( LogicToPixel( rDstSz ), MAP_TWIP ) );

            // #103209# Normalize size (mirroring has to happen outside of this method)
            aDstSizeTwip = Size( labs(aDstSizeTwip.Width()), labs(aDstSizeTwip.Height()) );

            const Size      aBmpSize( aBmp.GetSizePixel() );
            const double    fBmpPixelX = aBmpSize.Width();
            const double    fBmpPixelY = aBmpSize.Height();
            const double    fMaxPixelX = aDstSizeTwip.Width() * nMaxBmpDPIX / 1440.0;
            const double    fMaxPixelY = aDstSizeTwip.Height() * nMaxBmpDPIY / 1440.0;

            // check, if the bitmap DPI exceeds the maximum DPI (allow 4 pixel rounding tolerance)
            if( ( ( fBmpPixelX > ( fMaxPixelX + 4 ) ) ||
                  ( fBmpPixelY > ( fMaxPixelY + 4 ) ) ) &&
                ( fBmpPixelY > 0.0 ) && ( fMaxPixelY > 0.0 ) )
            {
                // do scaling
                Size            aNewBmpSize;
                const double    fBmpWH = fBmpPixelX / fBmpPixelY;
                const double    fMaxWH = fMaxPixelX / fMaxPixelY;

                if( fBmpWH < fMaxWH )
                {
                    aNewBmpSize.Width() = FRound( fMaxPixelY * fBmpWH );
                    aNewBmpSize.Height() = FRound( fMaxPixelY );
                }
                else if( fBmpWH > 0.0 )
                {
                    aNewBmpSize.Width() = FRound( fMaxPixelX );
                    aNewBmpSize.Height() = FRound( fMaxPixelX / fBmpWH);
                }

                if( aNewBmpSize.Width() && aNewBmpSize.Height() )
                    aBmp.Scale( aNewBmpSize );
                else
                    aBmp.SetEmpty();
            }
        }
    }

    return aBmp;
}

void Printer::DrawGradientEx( OutputDevice* pOut, const Rectangle& rRect, const Gradient& rGradient )
{
    const PrinterOptions& rPrinterOptions = GetPrinterOptions();

    if( rPrinterOptions.IsReduceGradients() )
    {
        if( PRINTER_GRADIENT_STRIPES == rPrinterOptions.GetReducedGradientMode() )
        {
            if( !rGradient.GetSteps() || ( rGradient.GetSteps() > rPrinterOptions.GetReducedGradientStepCount() ) )
            {
                Gradient aNewGradient( rGradient );

                aNewGradient.SetSteps( rPrinterOptions.GetReducedGradientStepCount() );
                pOut->DrawGradient( rRect, aNewGradient );
            }
            else
                pOut->DrawGradient( rRect, rGradient );
        }
        else
        {
            const Color&    rStartColor = rGradient.GetStartColor();
            const Color&    rEndColor = rGradient.GetEndColor();
            const long      nR = ( ( (long) rStartColor.GetRed() * rGradient.GetStartIntensity() ) / 100L +
                                   ( (long) rEndColor.GetRed() * rGradient.GetEndIntensity() ) / 100L ) >> 1;
            const long      nG = ( ( (long) rStartColor.GetGreen() * rGradient.GetStartIntensity() ) / 100L +
                                   ( (long) rEndColor.GetGreen() * rGradient.GetEndIntensity() ) / 100L ) >> 1;
            const long      nB = ( ( (long) rStartColor.GetBlue() * rGradient.GetStartIntensity() ) / 100L +
                                   ( (long) rEndColor.GetBlue() * rGradient.GetEndIntensity() ) / 100L ) >> 1;
            const Color     aColor( (sal_uInt8) nR, (sal_uInt8) nG, (sal_uInt8) nB );

            pOut->Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );
            pOut->SetLineColor( aColor );
            pOut->SetFillColor( aColor );
            pOut->DrawRect( rRect );
            pOut->Pop();
        }
    }
    else
        pOut->DrawGradient( rRect, rGradient );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
