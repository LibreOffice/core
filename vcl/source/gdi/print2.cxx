/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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

typedef ::std::pair< MetaAction*, int > Component; 

typedef ::std::list< Component > ComponentList;


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

/** \#i10613# Extracted from Printer::GetPreparedMetaFile. Returns true
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

        
        if( nTransparency )
        {
            o_rMtf.AddAction( new MetaPushAction( PUSH_LINECOLOR|PUSH_FILLCOLOR ) );

            
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
            
            BitmapReadAccess* pRA = aBmp.AcquireReadAccess();

            if( !pRA )
                return; 

            Color aActualColor( aBgColor );

            if( pRA->HasPalette() )
                aActualColor = pRA->GetBestPaletteColor( aBgColor ).operator Color();

            aBmp.ReleaseAccess(pRA);

            
            if( aActualColor.GetColorError( aBgColor ) )
            {
                
                aBmp.Convert( BMP_CONVERSION_24BIT );

                
                aBmp.Replace( aBmpEx.GetMask(), aBgColor );
            }
            else
            {
                
                aBmp.Replace( aBmpEx.GetMask(), aActualColor );
            }
        }
        else
        {
            
            aBmp.Convert( BMP_CONVERSION_24BIT );
            aBmp.Blend(aBmpEx.GetAlpha(),aBgColor);
        }

        
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
            
            bRet = true;
            break;

        default:
            break;
    }

    return bRet;
}


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

            if( !aString.isEmpty() )
            {
                
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

            
            
            
            
            
            if( !aString.isEmpty() )
            {
                
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
        
        for( pCurrAct = ( (GDIMetaFile&) rInMtf ).FirstAction();
             pCurrAct && !bTransparent;
             pCurrAct = ( (GDIMetaFile&) rInMtf ).NextAction() )
        {
            

            
            
            
            
            
            
            
            if( ImplIsActionSpecial( *pCurrAct ) )
            {
                bTransparent = true;
            }
        }
    }

    
    
    if( !bTransparent )
    {
        
        rOutMtf = rInMtf;
    }
    else
    {
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        //

        //
        
        
        //

        
        
        ConnectedComponents aBackgroundComponent;

        
        VirtualDevice aMapModeVDev;
        aMapModeVDev.mnDPIX = mnDPIX;
        aMapModeVDev.mnDPIY = mnDPIY;
        aMapModeVDev.EnableOutput(false);

        int nLastBgAction, nActionNum;

        
        
        
        
        bool bStillBackground=true; 
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
                        bStillBackground=false; 
                    else
                        nLastBgAction=nActionNum; 
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
                        bStillBackground=false; 
                    else
                        nLastBgAction=nActionNum; 
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
                        bStillBackground=false; 
                    else
                        nLastBgAction=nActionNum; 
                    break;
                }
                case META_WALLPAPER_ACTION:
                {
                    if( !checkRect(
                            aBackgroundComponent.aBounds,
                            aBackgroundComponent.aBgColor,
                            static_cast<const MetaWallpaperAction*>(pCurrAct)->GetRect(),
                            aMapModeVDev) )
                        bStillBackground=false; 
                    else
                        nLastBgAction=nActionNum; 
                    break;
                }
                default:
                {
                    if( ImplIsNotTransparent( *pCurrAct,
                                              aMapModeVDev ) )
                        bStillBackground=false; 
                                                
                    else
                        
                        
                        aBackgroundComponent.aBounds.Union(
                            ImplCalcActionBounds(*pCurrAct, aMapModeVDev) );
                    break;
                }
            }

            
            pCurrAct->Execute( &aMapModeVDev );

            pCurrAct=const_cast<GDIMetaFile&>(rInMtf).NextAction();
            ++nActionNum;
        }

        
        sal_uInt32 nCount = aMapModeVDev.GetGCStackDepth();
        while( nCount-- )
            aMapModeVDev.Pop();

        ConnectedComponentsList aCCList; 

        
        
        nActionNum=0;
        pCurrAct=const_cast<GDIMetaFile&>(rInMtf).FirstAction();
        while( pCurrAct && nActionNum<=nLastBgAction )
        {
            
            
            aBackgroundComponent.aComponentList.push_back(
                ::std::make_pair(
                    pCurrAct, nActionNum) );

            
            pCurrAct->Execute( &aMapModeVDev );
            pCurrAct=const_cast<GDIMetaFile&>(rInMtf).NextAction();
            ++nActionNum;
        }

        //
        
        
        //

        
        
        for( ;
             pCurrAct;
             pCurrAct=const_cast<GDIMetaFile&>(rInMtf).NextAction(), ++nActionNum )
        {
            
            pCurrAct->Execute( &aMapModeVDev );

            
            const Rectangle aBBCurrAct( ImplCalcActionBounds(*pCurrAct, aMapModeVDev) );

            
            Rectangle                               aTotalBounds( aBBCurrAct ); 
                                                                                
                                                                                
                                                                                
                                                                                
                                                                                
                                                                                
            bool                                    bTreatSpecial( false );
            ConnectedComponents                     aTotalComponents;

            //
            
            
            //

            
            
            
            
            
            

            
            
            
            
            aTotalComponents.bIsFullyTransparent = !ImplIsNotTransparent(*pCurrAct, aMapModeVDev);

            if( !aBBCurrAct.IsEmpty() &&
                !aTotalComponents.bIsFullyTransparent )
            {
                if( !aBackgroundComponent.aComponentList.empty() &&
                    !aBackgroundComponent.aBounds.IsInside(aTotalBounds) )
                {
                    
                    
                    aTotalBounds.Union( aBackgroundComponent.aBounds );

                    
                    aTotalComponents.aComponentList.splice( aTotalComponents.aComponentList.end(),
                                                            aBackgroundComponent.aComponentList );

                    if( aBackgroundComponent.bIsSpecial )
                        bTreatSpecial = true;
                }

                ConnectedComponentsList::iterator       aCurrCC;
                const ConnectedComponentsList::iterator aLastCC( aCCList.end() );
                bool                                    bSomeComponentsChanged;

                
                
                
                
                
                
                
                do
                {
                    
                    bSomeComponentsChanged = false;

                    
                    for( aCurrCC=aCCList.begin(); aCurrCC != aLastCC; )
                    {
                        
                        
                        
                        

                        
                        
                        
                        
                        if( !aCurrCC->aBounds.IsEmpty() &&
                            !aCurrCC->bIsFullyTransparent &&
                            aCurrCC->aBounds.IsOver( aTotalBounds ) )
                        {
                            

                            
                            aTotalBounds.Union( aCurrCC->aBounds );

                            
                            aTotalComponents.aComponentList.splice( aTotalComponents.aComponentList.end(),
                                                                    aCurrCC->aComponentList );

                            if( aCurrCC->bIsSpecial )
                                bTreatSpecial = true;

                            
                            aCurrCC = aCCList.erase( aCurrCC );

                            
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
            
            
            //

            
            
            
            
            
            
            
            
            
            
            
            
            
            
            if( bTreatSpecial )
            {
                
                aTotalComponents.bIsSpecial = true;
            }
            else if( !ImplIsActionSpecial( *pCurrAct ) )
            {
                
                
                aTotalComponents.bIsSpecial = false;
            }
            else
            {
                
                

                
                
                
                if( !ImplIsActionHandlingTransparency( *pCurrAct ) )
                {
                    
                    
                    aTotalComponents.bIsSpecial = true;
                }
                else
                {
                    
                    
                    if( aTotalComponents.aComponentList.empty() )
                    {
                        
                        
                        aTotalComponents.bIsSpecial = false;
                    }
                    else
                    {
                        
                        
                        
                        
                        

                        
                        
                        
                        aTotalComponents.bIsSpecial = true;
                    }
                }
            }


            //
            
            
            //

            
            aTotalComponents.aBounds = aTotalBounds;
            aTotalComponents.aComponentList.push_back(
                ::std::make_pair(
                    pCurrAct, nActionNum) );

            
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

        
        
        
        
        
        
        
        

        
        ::std::vector< const ConnectedComponents* > aCCList_MemberMap( rInMtf.GetActionSize() );

        
        ConnectedComponentsList::iterator       aCurr( aCCList.begin() );
        const ConnectedComponentsList::iterator aLast( aCCList.end() );
        for( ; aCurr != aLast; ++aCurr )
        {
            ComponentList::iterator       aCurrentAction( aCurr->aComponentList.begin() );
            const ComponentList::iterator aLastAction( aCurr->aComponentList.end() );
            for( ; aCurrentAction != aLastAction; ++aCurrentAction )
            {
                
                aCCList_MemberMap[ aCurrentAction->second ] = &(*aCurr);
            }
        }

        //
        
        
        //

        ComponentList::iterator       aCurrAct( aBackgroundComponent.aComponentList.begin() );
        const ComponentList::iterator aLastAct( aBackgroundComponent.aComponentList.end() );
        for( ; aCurrAct != aLastAct; ++aCurrAct )
        {
            
            
            rOutMtf.AddAction( ( aCurrAct->first->Duplicate(), aCurrAct->first ) );
        }


        //
        
        
        //

        Point aPageOffset;
        Size aTmpSize( GetOutputSizePixel() );
        if( mpPDFWriter )
        {
            aTmpSize = mpPDFWriter->getCurPageSize();
            aTmpSize = LogicToPixel( aTmpSize, MapMode( MAP_POINT ) );

            
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

        
        for( aCurr = aCCList.begin(); aCurr != aLast; ++aCurr )
        {
            if( aCurr->bIsSpecial )
            {
                Rectangle aBoundRect( aCurr->aBounds );
                aBoundRect.Intersection( aOutputRect );

                const double fBmpArea( (double) aBoundRect.GetWidth() * aBoundRect.GetHeight() );
                const double fOutArea( (double) aOutputRect.GetWidth() * aOutputRect.GetHeight() );

                
                if( bReduceTransparency && bTransparencyAutoMode && ( fBmpArea > ( 0.25 * fOutArea ) ) )
                {
                    
                    
                    
                    aCurr->bIsSpecial = false;
                }
                else
                {
                    
                    if( aBoundRect.GetWidth() && aBoundRect.GetHeight() )
                    {
                        Point           aDstPtPix( aBoundRect.TopLeft() );
                        Size            aDstSzPix;

                        VirtualDevice   aMapVDev;   
                        aMapVDev.EnableOutput(false);

                        VirtualDevice   aPaintVDev; 
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

                                    aPaintVDev.EnableOutput(false);

                                    
                                    for( pCurrAct=const_cast<GDIMetaFile&>(rInMtf).FirstAction(), nActionNum=0;
                                         pCurrAct;
                                         pCurrAct=const_cast<GDIMetaFile&>(rInMtf).NextAction(), ++nActionNum )
                                    {
                                        
                                        
                                        
                                        
                                        if( aCCList_MemberMap[nActionNum] == &(*aCurr) )
                                            aPaintVDev.EnableOutput(true);

                                        
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

                                    const bool bOldMap = mbMap;
                                    mbMap = aPaintVDev.mbMap = false;

                                    Bitmap aBandBmp( aPaintVDev.GetBitmap( Point(), aDstSzPix ) );

                                    
                                    if( bDownsampleBitmaps )
                                    {
                                        aBandBmp = GetDownsampledBitmap( aDstSzPix,
                                                                         Point(), aBandBmp.GetSizePixel(),
                                                                         aBandBmp, nMaxBmpDPIX, nMaxBmpDPIY );
                                    }

                                    rOutMtf.AddAction( new MetaCommentAction( "PRNSPOOL_TRANSPARENTBITMAP_BEGIN" ) );
                                    rOutMtf.AddAction( new MetaBmpScaleAction( aDstPtPix, aDstSzPix, aBandBmp ) );
                                    rOutMtf.AddAction( new MetaCommentAction( "PRNSPOOL_TRANSPARENTBITMAP_END" ) );

                                    aPaintVDev.mbMap = true;
                                    mbMap = bOldMap;
                                    aMapVDev.Pop();
                                    aPaintVDev.Pop();
                                }

                                
                                aDstPtPix.X() += aDstSzPix.Width();
                            }

                            
                            aDstPtPix.Y() += aDstSzPix.Height();
                        }

                        rOutMtf.AddAction( new MetaPopAction() );
                    }
                }
            }
        }

        
        nCount = aMapModeVDev.GetGCStackDepth();
        while( nCount-- )
            aMapModeVDev.Pop();

        //
        
        
        //

        
        
        for( pCurrAct=const_cast<GDIMetaFile&>(rInMtf).FirstAction(), nActionNum=0;
             pCurrAct;
             pCurrAct=const_cast<GDIMetaFile&>(rInMtf).NextAction(), ++nActionNum )
        {
            const ConnectedComponents* pCurrAssociatedComponent = aCCList_MemberMap[nActionNum];

            
            
            
            
            if( pCurrAssociatedComponent &&
                (pCurrAssociatedComponent->aBounds.IsEmpty() ||
                 !pCurrAssociatedComponent->bIsSpecial) )
            {
                
                
                
                
                if( ImplIsActionHandlingTransparency( *pCurrAct ) &&
                    pCurrAssociatedComponent->aComponentList.begin()->first == pCurrAct )
                {
                    
                    
                    ImplConvertTransparentAction(rOutMtf,
                                                 *pCurrAct,
                                                 aMapModeVDev,
                                                 aBackgroundComponent.aBgColor);
                }
                else
                {
                    
                    rOutMtf.AddAction( ( pCurrAct->Duplicate(), pCurrAct ) );
                }

                pCurrAct->Execute(&aMapModeVDev);
            }
        }

        rOutMtf.SetPrefMapMode( rInMtf.GetPrefMapMode() );
        rOutMtf.SetPrefSize( rInMtf.GetPrefSize() );

#if OSL_DEBUG_LEVEL > 1
        
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

        
        if( aSrcRect.Intersection( aBmpRect ) != aBmpRect )
        {
            if( !aSrcRect.IsEmpty() )
                aBmp.Crop( aSrcRect );
            else
                aBmp.SetEmpty();
        }

        if( !aBmp.IsEmpty() )
        {
            
            Size aDstSizeTwip( PixelToLogic( LogicToPixel( rDstSz ), MAP_TWIP ) );

            
            aDstSizeTwip = Size( labs(aDstSizeTwip.Width()), labs(aDstSizeTwip.Height()) );

            const Size      aBmpSize( aBmp.GetSizePixel() );
            const double    fBmpPixelX = aBmpSize.Width();
            const double    fBmpPixelY = aBmpSize.Height();
            const double    fMaxPixelX = aDstSizeTwip.Width() * nMaxBmpDPIX / 1440.0;
            const double    fMaxPixelY = aDstSizeTwip.Height() * nMaxBmpDPIY / 1440.0;

            
            if( ( ( fBmpPixelX > ( fMaxPixelX + 4 ) ) ||
                  ( fBmpPixelY > ( fMaxPixelY + 4 ) ) ) &&
                ( fBmpPixelY > 0.0 ) && ( fMaxPixelY > 0.0 ) )
            {
                
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
