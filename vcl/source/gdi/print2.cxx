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
#include <vcl/bitmapaccess.hxx>

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

namespace {

/** \#i10613# Extracted from Printer::GetPreparedMetaFile. Returns true
    if given action requires special transparency handling
*/
bool IsTransparentAction( const MetaAction& rAct )
{
    switch( rAct.GetType() )
    {
        case MetaActionType::Transparent:
            return true;

        case MetaActionType::FLOATTRANSPARENT:
            return true;

        case MetaActionType::BMPEX:
            return static_cast<const MetaBmpExAction&>(rAct).GetBitmapEx().IsTransparent();

        case MetaActionType::BMPEXSCALE:
            return static_cast<const MetaBmpExScaleAction&>(rAct).GetBitmapEx().IsTransparent();

        case MetaActionType::BMPEXSCALEPART:
            return static_cast<const MetaBmpExScalePartAction&>(rAct).GetBitmapEx().IsTransparent();

        default:
            return false;
    }
}


/** Determines whether the action can handle transparency correctly
  (i.e. when painted on white background, does the action still look
  correct)?
 */
bool DoesActionHandleTransparency( const MetaAction& rAct )
{
    // MetaActionType::FLOATTRANSPARENT can contain a whole metafile,
    // which is to be rendered with the given transparent gradient. We
    // currently cannot emulate transparent painting on a white
    // background reliably.

    // the remainder can handle printing itself correctly on a uniform
    // white background.
    switch( rAct.GetType() )
    {
        case MetaActionType::Transparent:
        case MetaActionType::BMPEX:
        case MetaActionType::BMPEXSCALE:
        case MetaActionType::BMPEXSCALEPART:
            return true;

        default:
            return false;
    }
}

/** Check whether rCurrRect rectangle fully covers io_rPrevRect - if
    yes, return true and update o_rBgColor
 */
bool checkRect( Rectangle&       io_rPrevRect,
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
void ImplConvertTransparentAction( GDIMetaFile&        o_rMtf,
                                          const MetaAction&   rAct,
                                          const OutputDevice& rStateOutDev,
                                          Color               aBgColor )
{
    if( rAct.GetType() == MetaActionType::Transparent )
    {
        const MetaTransparentAction* pTransAct = static_cast<const MetaTransparentAction*>(&rAct);
        sal_uInt16                       nTransparency( pTransAct->GetTransparence() );

        // #i10613# Respect transparency for draw color
        if( nTransparency )
        {
            o_rMtf.AddAction( new MetaPushAction( PushFlags::LINECOLOR|PushFlags::FILLCOLOR ) );

            // assume white background for alpha blending
            Color aLineColor( rStateOutDev.GetLineColor() );
            aLineColor.SetRed( static_cast<sal_uInt8>( (255L*nTransparency + (100L - nTransparency)*aLineColor.GetRed()) / 100L ) );
            aLineColor.SetGreen( static_cast<sal_uInt8>( (255L*nTransparency + (100L - nTransparency)*aLineColor.GetGreen()) / 100L ) );
            aLineColor.SetBlue( static_cast<sal_uInt8>( (255L*nTransparency + (100L - nTransparency)*aLineColor.GetBlue()) / 100L ) );
            o_rMtf.AddAction( new MetaLineColorAction(aLineColor, true) );

            Color aFillColor( rStateOutDev.GetFillColor() );
            aFillColor.SetRed( static_cast<sal_uInt8>( (255L*nTransparency + (100L - nTransparency)*aFillColor.GetRed()) / 100L ) );
            aFillColor.SetGreen( static_cast<sal_uInt8>( (255L*nTransparency + (100L - nTransparency)*aFillColor.GetGreen()) / 100L ) );
            aFillColor.SetBlue( static_cast<sal_uInt8>( (255L*nTransparency + (100L - nTransparency)*aFillColor.GetBlue()) / 100L ) );
            o_rMtf.AddAction( new MetaFillColorAction(aFillColor, true) );
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
            case MetaActionType::BMPEX:
                aBmpEx = static_cast<const MetaBmpExAction&>(rAct).GetBitmapEx();
                break;

            case MetaActionType::BMPEXSCALE:
                aBmpEx = static_cast<const MetaBmpExScaleAction&>(rAct).GetBitmapEx();
                break;

            case MetaActionType::BMPEXSCALEPART:
                aBmpEx = static_cast<const MetaBmpExScaleAction&>(rAct).GetBitmapEx();
                break;

            case MetaActionType::Transparent:

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

            Bitmap::ReleaseAccess(pRA);

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
            case MetaActionType::BMPEX:
                o_rMtf.AddAction( new MetaBmpAction(
                                       static_cast<const MetaBmpExAction&>(rAct).GetPoint(),
                                       aBmp ));
                break;
            case MetaActionType::BMPEXSCALE:
                o_rMtf.AddAction( new MetaBmpScaleAction(
                                       static_cast<const MetaBmpExScaleAction&>(rAct).GetPoint(),
                                       static_cast<const MetaBmpExScaleAction&>(rAct).GetSize(),
                                       aBmp ));
                break;
            case MetaActionType::BMPEXSCALEPART:
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
bool ImplIsNotTransparent( const MetaAction& rAct, const OutputDevice& rOut )
{
    const bool  bLineTransparency( !rOut.IsLineColor() || rOut.GetLineColor().GetTransparency() == 255 );
    const bool  bFillTransparency( !rOut.IsFillColor() || rOut.GetFillColor().GetTransparency() == 255 );
    bool        bRet( false );

    switch( rAct.GetType() )
    {
        case MetaActionType::POINT:
            if( !bLineTransparency )
                bRet = true;
            break;

        case MetaActionType::LINE:
            if( !bLineTransparency )
                bRet = true;
            break;

        case MetaActionType::RECT:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case MetaActionType::ROUNDRECT:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case MetaActionType::ELLIPSE:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case MetaActionType::ARC:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case MetaActionType::PIE:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case MetaActionType::CHORD:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case MetaActionType::POLYLINE:
            if( !bLineTransparency )
                bRet = true;
            break;

        case MetaActionType::POLYGON:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case MetaActionType::POLYPOLYGON:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case MetaActionType::TEXT:
        {
            const MetaTextAction& rTextAct = static_cast<const MetaTextAction&>(rAct);
            const OUString aString( rTextAct.GetText().copy(rTextAct.GetIndex(), rTextAct.GetLen()) );
            if (!aString.isEmpty())
                bRet = true;
        }
        break;

        case MetaActionType::TEXTARRAY:
        {
            const MetaTextArrayAction& rTextAct = static_cast<const MetaTextArrayAction&>(rAct);
            const OUString aString( rTextAct.GetText().copy(rTextAct.GetIndex(), rTextAct.GetLen()) );
            if (!aString.isEmpty())
                bRet = true;
        }
        break;

        case MetaActionType::PIXEL:
        case MetaActionType::BMP:
        case MetaActionType::BMPSCALE:
        case MetaActionType::BMPSCALEPART:
        case MetaActionType::BMPEX:
        case MetaActionType::BMPEXSCALE:
        case MetaActionType::BMPEXSCALEPART:
        case MetaActionType::MASK:
        case MetaActionType::MASKSCALE:
        case MetaActionType::MASKSCALEPART:
        case MetaActionType::GRADIENT:
        case MetaActionType::GRADIENTEX:
        case MetaActionType::HATCH:
        case MetaActionType::WALLPAPER:
        case MetaActionType::Transparent:
        case MetaActionType::FLOATTRANSPARENT:
        case MetaActionType::EPS:
        case MetaActionType::TEXTRECT:
        case MetaActionType::STRETCHTEXT:
        case MetaActionType::TEXTLINE:
            // all other actions: generate non-transparent output
            bRet = true;
            break;

        default:
            break;
    }

    return bRet;
}

// #i10613# Extracted from ImplCheckRect::ImplCreate
Rectangle ImplCalcActionBounds( const MetaAction& rAct, const OutputDevice& rOut )
{
    Rectangle aActionBounds;

    switch( rAct.GetType() )
    {
        case MetaActionType::PIXEL:
            aActionBounds = Rectangle( static_cast<const MetaPixelAction&>(rAct).GetPoint(), Size( 1, 1 ) );
            break;

        case MetaActionType::POINT:
            aActionBounds = Rectangle( static_cast<const MetaPointAction&>(rAct).GetPoint(), Size( 1, 1 ) );
            break;

        case MetaActionType::LINE:
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

        case MetaActionType::RECT:
            aActionBounds = static_cast<const MetaRectAction&>(rAct).GetRect();
            break;

        case MetaActionType::ROUNDRECT:
            aActionBounds = tools::Polygon( static_cast<const MetaRoundRectAction&>(rAct).GetRect(),
                                            static_cast<const MetaRoundRectAction&>(rAct).GetHorzRound(),
                                            static_cast<const MetaRoundRectAction&>(rAct).GetVertRound() ).GetBoundRect();
            break;

        case MetaActionType::ELLIPSE:
        {
            const Rectangle& rRect = static_cast<const MetaEllipseAction&>(rAct).GetRect();
            aActionBounds = tools::Polygon( rRect.Center(),
                                            rRect.GetWidth() >> 1,
                                            rRect.GetHeight() >> 1 ).GetBoundRect();
            break;
        }

        case MetaActionType::ARC:
            aActionBounds = tools::Polygon( static_cast<const MetaArcAction&>(rAct).GetRect(),
                                            static_cast<const MetaArcAction&>(rAct).GetStartPoint(),
                                            static_cast<const MetaArcAction&>(rAct).GetEndPoint(), POLY_ARC ).GetBoundRect();
            break;

        case MetaActionType::PIE:
            aActionBounds = tools::Polygon( static_cast<const MetaPieAction&>(rAct).GetRect(),
                                            static_cast<const MetaPieAction&>(rAct).GetStartPoint(),
                                            static_cast<const MetaPieAction&>(rAct).GetEndPoint(), POLY_PIE ).GetBoundRect();
            break;

        case MetaActionType::CHORD:
            aActionBounds = tools::Polygon( static_cast<const MetaChordAction&>(rAct).GetRect(),
                                            static_cast<const MetaChordAction&>(rAct).GetStartPoint(),
                                            static_cast<const MetaChordAction&>(rAct).GetEndPoint(), POLY_CHORD ).GetBoundRect();
            break;

        case MetaActionType::POLYLINE:
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

        case MetaActionType::POLYGON:
            aActionBounds = static_cast<const MetaPolygonAction&>(rAct).GetPolygon().GetBoundRect();
            break;

        case MetaActionType::POLYPOLYGON:
            aActionBounds = static_cast<const MetaPolyPolygonAction&>(rAct).GetPolyPolygon().GetBoundRect();
            break;

        case MetaActionType::BMP:
            aActionBounds = Rectangle( static_cast<const MetaBmpAction&>(rAct).GetPoint(),
                                       rOut.PixelToLogic( static_cast<const MetaBmpAction&>(rAct).GetBitmap().GetSizePixel() ) );
            break;

        case MetaActionType::BMPSCALE:
            aActionBounds = Rectangle( static_cast<const MetaBmpScaleAction&>(rAct).GetPoint(),
                                       static_cast<const MetaBmpScaleAction&>(rAct).GetSize() );
            break;

        case MetaActionType::BMPSCALEPART:
            aActionBounds = Rectangle( static_cast<const MetaBmpScalePartAction&>(rAct).GetDestPoint(),
                                       static_cast<const MetaBmpScalePartAction&>(rAct).GetDestSize() );
            break;

        case MetaActionType::BMPEX:
            aActionBounds = Rectangle( static_cast<const MetaBmpExAction&>(rAct).GetPoint(),
                                       rOut.PixelToLogic( static_cast<const MetaBmpExAction&>(rAct).GetBitmapEx().GetSizePixel() ) );
            break;

        case MetaActionType::BMPEXSCALE:
            aActionBounds = Rectangle( static_cast<const MetaBmpExScaleAction&>(rAct).GetPoint(),
                                       static_cast<const MetaBmpExScaleAction&>(rAct).GetSize() );
            break;

        case MetaActionType::BMPEXSCALEPART:
            aActionBounds = Rectangle( static_cast<const MetaBmpExScalePartAction&>(rAct).GetDestPoint(),
                                       static_cast<const MetaBmpExScalePartAction&>(rAct).GetDestSize() );
            break;

        case MetaActionType::MASK:
            aActionBounds = Rectangle( static_cast<const MetaMaskAction&>(rAct).GetPoint(),
                                       rOut.PixelToLogic( static_cast<const MetaMaskAction&>(rAct).GetBitmap().GetSizePixel() ) );
            break;

        case MetaActionType::MASKSCALE:
            aActionBounds = Rectangle( static_cast<const MetaMaskScaleAction&>(rAct).GetPoint(),
                                       static_cast<const MetaMaskScaleAction&>(rAct).GetSize() );
            break;

        case MetaActionType::MASKSCALEPART:
            aActionBounds = Rectangle( static_cast<const MetaMaskScalePartAction&>(rAct).GetDestPoint(),
                                       static_cast<const MetaMaskScalePartAction&>(rAct).GetDestSize() );
            break;

        case MetaActionType::GRADIENT:
            aActionBounds = static_cast<const MetaGradientAction&>(rAct).GetRect();
            break;

        case MetaActionType::GRADIENTEX:
            aActionBounds = static_cast<const MetaGradientExAction&>(rAct).GetPolyPolygon().GetBoundRect();
            break;

        case MetaActionType::HATCH:
            aActionBounds = static_cast<const MetaHatchAction&>(rAct).GetPolyPolygon().GetBoundRect();
            break;

        case MetaActionType::WALLPAPER:
            aActionBounds = static_cast<const MetaWallpaperAction&>(rAct).GetRect();
            break;

        case MetaActionType::Transparent:
            aActionBounds = static_cast<const MetaTransparentAction&>(rAct).GetPolyPolygon().GetBoundRect();
            break;

        case MetaActionType::FLOATTRANSPARENT:
            aActionBounds = Rectangle( static_cast<const MetaFloatTransparentAction&>(rAct).GetPoint(),
                                       static_cast<const MetaFloatTransparentAction&>(rAct).GetSize() );
            break;

        case MetaActionType::EPS:
            aActionBounds = Rectangle( static_cast<const MetaEPSAction&>(rAct).GetPoint(),
                                       static_cast<const MetaEPSAction&>(rAct).GetSize() );
            break;

        case MetaActionType::TEXT:
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

        case MetaActionType::TEXTARRAY:
        {
            const MetaTextArrayAction&  rTextAct = static_cast<const MetaTextArrayAction&>(rAct);
            const OUString              aString( rTextAct.GetText().copy(rTextAct.GetIndex(), rTextAct.GetLen()) );

            if( !aString.isEmpty() )
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

        case MetaActionType::TEXTRECT:
            aActionBounds = static_cast<const MetaTextRectAction&>(rAct).GetRect();
            break;

        case MetaActionType::STRETCHTEXT:
        {
            const MetaStretchTextAction& rTextAct = static_cast<const MetaStretchTextAction&>(rAct);
            const OUString               aString( rTextAct.GetText().copy(rTextAct.GetIndex(), rTextAct.GetLen()) );

            // #i16195# Literate copy from TextArray action, the
            // semantics for the ImplLayout call are copied from the
            // OutDev::DrawStretchText() code. Unfortunately, also in
            // this case, public outdev methods such as GetTextWidth()
            // don't provide enough info.
            if( !aString.isEmpty() )
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

        case MetaActionType::TEXTLINE:
            OSL_FAIL("MetaActionType::TEXTLINE not supported");
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

} // end anon namespace

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
            // #i10613# determine if the action is transparency capable

            // #107169# Also examine metafiles with masked bitmaps in
            // detail. Further down, this is optimized in such a way
            // that there's no unnecessary painting of masked bitmaps
            // (which are _always_ subdivided into rectangular regions
            // of uniform opacity): if a masked bitmap is printed over
            // empty background, we convert to a plain bitmap with
            // white background.
            if( IsTransparentAction( *pCurrAct ) )
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

        //  STAGE 1: Detect background

        // Receives uniform background content, and is _not_ merged
        // nor checked for intersection against other aCCList elements
        ConnectedComponents aBackgroundComponent;

        // create an OutputDevice to record mapmode changes and the like
        ScopedVclPtrInstance< VirtualDevice > aMapModeVDev;
        aMapModeVDev->mnDPIX = mnDPIX;
        aMapModeVDev->mnDPIY = mnDPIY;
        aMapModeVDev->EnableOutput(false);

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
                case MetaActionType::RECT:
                {
                    if( !checkRect(
                            aBackgroundComponent.aBounds,
                            aBackgroundComponent.aBgColor,
                            static_cast<const MetaRectAction*>(pCurrAct)->GetRect(),
                            *aMapModeVDev.get()) )
                        bStillBackground=false; // incomplete occlusion of background
                    else
                        nLastBgAction=nActionNum; // this _is_ background
                    break;
                }
                case MetaActionType::POLYGON:
                {
                    const tools::Polygon aPoly(
                        static_cast<const MetaPolygonAction*>(pCurrAct)->GetPolygon());
                    if( !basegfx::tools::isRectangle(
                            aPoly.getB2DPolygon()) ||
                        !checkRect(
                            aBackgroundComponent.aBounds,
                            aBackgroundComponent.aBgColor,
                            aPoly.GetBoundRect(),
                            *aMapModeVDev.get()) )
                        bStillBackground=false; // incomplete occlusion of background
                    else
                        nLastBgAction=nActionNum; // this _is_ background
                    break;
                }
                case MetaActionType::POLYPOLYGON:
                {
                    const tools::PolyPolygon aPoly(
                        static_cast<const MetaPolyPolygonAction*>(pCurrAct)->GetPolyPolygon());
                    if( aPoly.Count() != 1 ||
                        !basegfx::tools::isRectangle(
                            aPoly[0].getB2DPolygon()) ||
                        !checkRect(
                            aBackgroundComponent.aBounds,
                            aBackgroundComponent.aBgColor,
                            aPoly.GetBoundRect(),
                            *aMapModeVDev.get()) )
                        bStillBackground=false; // incomplete occlusion of background
                    else
                        nLastBgAction=nActionNum; // this _is_ background
                    break;
                }
                case MetaActionType::WALLPAPER:
                {
                    if( !checkRect(
                            aBackgroundComponent.aBounds,
                            aBackgroundComponent.aBgColor,
                            static_cast<const MetaWallpaperAction*>(pCurrAct)->GetRect(),
                            *aMapModeVDev.get()) )
                        bStillBackground=false; // incomplete occlusion of background
                    else
                        nLastBgAction=nActionNum; // this _is_ background
                    break;
                }
                default:
                {
                    if( ImplIsNotTransparent( *pCurrAct,
                                              *aMapModeVDev.get() ) )
                        bStillBackground=false; // non-transparent action, possibly
                                                // not uniform
                    else
                        // extend current bounds (next uniform action
                        // needs to fully cover this area)
                        aBackgroundComponent.aBounds.Union(
                            ImplCalcActionBounds(*pCurrAct, *aMapModeVDev.get()) );
                    break;
                }
            }

            // execute action to get correct MapModes etc.
            pCurrAct->Execute( aMapModeVDev.get() );

            pCurrAct=const_cast<GDIMetaFile&>(rInMtf).NextAction();
            ++nActionNum;
        }

        // clean up aMapModeVDev
        sal_uInt32 nCount = aMapModeVDev->GetGCStackDepth();
        while( nCount-- )
            aMapModeVDev->Pop();

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
            pCurrAct->Execute( aMapModeVDev.get() );
            pCurrAct=const_cast<GDIMetaFile&>(rInMtf).NextAction();
            ++nActionNum;
        }

        //  STAGE 2: Generate connected components list

        // iterate over all actions (start where background action
        // search left off)
        for( ;
             pCurrAct;
             pCurrAct=const_cast<GDIMetaFile&>(rInMtf).NextAction(), ++nActionNum )
        {
            // execute action to get correct MapModes etc.
            pCurrAct->Execute( aMapModeVDev.get() );

            // cache bounds of current action
            const Rectangle aBBCurrAct( ImplCalcActionBounds(*pCurrAct, *aMapModeVDev.get()) );

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

            //  STAGE 2.1: Search for intersecting cc entries

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
            aTotalComponents.bIsFullyTransparent = !ImplIsNotTransparent(*pCurrAct, *aMapModeVDev.get());

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

            //  STAGE 2.2: Determine special state for cc element

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
            else if( !IsTransparentAction( *pCurrAct ) )
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
                if( !DoesActionHandleTransparency( *pCurrAct ) )
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
                        // #107169# Fixes above now ensure that _no_
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

            //  STAGE 2.3: Add newly generated CC list element

            // set new bounds and add action to list
            aTotalComponents.aBounds = aTotalBounds;
            aTotalComponents.aComponentList.push_back(
                ::std::make_pair(
                    pCurrAct, nActionNum) );

            // add aTotalComponents as a new entry to aCCList
            aCCList.push_back( aTotalComponents );

            DBG_ASSERT( !aTotalComponents.aComponentList.empty(),
                        "Printer::GetPreparedMetaFile empty component" );
            DBG_ASSERT( !aTotalComponents.aBounds.IsEmpty() || (aTotalComponents.aComponentList.size() == 1),
                        "Printer::GetPreparedMetaFile non-output generating actions must be solitary");
            DBG_ASSERT( !aTotalComponents.bIsFullyTransparent || (aTotalComponents.aComponentList.size() == 1),
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

        //  STAGE 3.1: Output background mtf actions (if there are any)

        ComponentList::iterator       aCurrAct( aBackgroundComponent.aComponentList.begin() );
        const ComponentList::iterator aLastAct( aBackgroundComponent.aComponentList.end() );
        for( ; aCurrAct != aLastAct; ++aCurrAct )
        {
            // simply add this action (above, we inserted the actions
            // starting at index 0 up to and including nLastBgAction)
            rOutMtf.AddAction( ( aCurrAct->first->Duplicate(), aCurrAct->first ) );
        }

        //  STAGE 3.2: Generate banded bitmaps for special regions

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
        bool bTiling = dynamic_cast<Printer*>(this) != nullptr;

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

                        ScopedVclPtrInstance<VirtualDevice> aMapVDev;   // here, we record only mapmode information
                        aMapVDev->EnableOutput(false);

                        ScopedVclPtrInstance<VirtualDevice> aPaintVDev; // into this one, we render.
                        aPaintVDev->SetBackground( aBackgroundComponent.aBgColor );

                        rOutMtf.AddAction( new MetaPushAction( PushFlags::MAPMODE ) );
                        rOutMtf.AddAction( new MetaMapModeAction() );

                        aPaintVDev->SetDrawMode( GetDrawMode() );

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
                                    aPaintVDev->SetOutputSizePixel( aDstSzPix ) )
                                {
                                    aPaintVDev->Push();
                                    aMapVDev->Push();

                                    aMapVDev->mnDPIX = aPaintVDev->mnDPIX = mnDPIX;
                                    aMapVDev->mnDPIY = aPaintVDev->mnDPIY = mnDPIY;

                                    aPaintVDev->EnableOutput(false);

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
                                            aPaintVDev->EnableOutput();

                                        // but process every action
                                        const MetaActionType nType( pCurrAct->GetType() );

                                        if( MetaActionType::MAPMODE == nType )
                                        {
                                            pCurrAct->Execute( aMapVDev.get() );

                                            MapMode     aMtfMap( aMapVDev->GetMapMode() );
                                            const Point aNewOrg( aMapVDev->PixelToLogic( aDstPtPix ) );

                                            aMtfMap.SetOrigin( Point( -aNewOrg.X(), -aNewOrg.Y() ) );
                                            aPaintVDev->SetMapMode( aMtfMap );
                                        }
                                        else if( ( MetaActionType::PUSH == nType ) || ( MetaActionType::POP ) == nType )
                                        {
                                            pCurrAct->Execute( aMapVDev.get() );
                                            pCurrAct->Execute( aPaintVDev.get() );
                                        }
                                        else if( MetaActionType::GRADIENT == nType )
                                        {
                                            MetaGradientAction* pGradientAction = static_cast<MetaGradientAction*>(pCurrAct);
                                            Printer* pPrinter = dynamic_cast< Printer* >(this);
                                            if( pPrinter )
                                                pPrinter->DrawGradientEx( aPaintVDev.get(), pGradientAction->GetRect(), pGradientAction->GetGradient() );
                                            else
                                                DrawGradient( pGradientAction->GetRect(), pGradientAction->GetGradient() );
                                        }
                                        else
                                        {
                                            pCurrAct->Execute( aPaintVDev.get() );
                                        }

                                        if( !( nActionNum % 8 ) )
                                            Application::Reschedule();
                                    }

                                    const bool bOldMap = mbMap;
                                    mbMap = aPaintVDev->mbMap = false;

                                    Bitmap aBandBmp( aPaintVDev->GetBitmap( Point(), aDstSzPix ) );

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

                                    aPaintVDev->mbMap = true;
                                    mbMap = bOldMap;
                                    aMapVDev->Pop();
                                    aPaintVDev->Pop();
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
        nCount = aMapModeVDev->GetGCStackDepth();
        while( nCount-- )
            aMapModeVDev->Pop();

        //  STAGE 4: Copy actions to output metafile

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
                if( DoesActionHandleTransparency( *pCurrAct ) &&
                    pCurrAssociatedComponent->aComponentList.begin()->first == pCurrAct )
                {
                    // convert actions, where masked-out parts are of
                    // given background color
                    ImplConvertTransparentAction(rOutMtf,
                                                 *pCurrAct,
                                                 *aMapModeVDev.get(),
                                                 aBackgroundComponent.aBgColor);
                }
                else
                {
                    // simply add this action
                    rOutMtf.AddAction( ( pCurrAct->Duplicate(), pCurrAct ) );
                }

                pCurrAct->Execute(aMapModeVDev.get());
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

            rOutMtf.AddAction( new MetaRectAction( aMapModeVDev->PixelToLogic( aCurr->aBounds ) ) );
        }
#endif
    }
    return bTransparent;
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

            pOut->Push( PushFlags::LINECOLOR | PushFlags::FILLCOLOR );
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
