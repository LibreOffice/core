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

#include <algorithm>

#include <osl/diagnose.h>
#include <tools/fract.hxx>
#include <tools/helpers.hxx>
#include <vcl/svapp.hxx>
#include <vcl/metaact.hxx>
#include <vcl/GraphicObject.hxx>
#include <vcl/GraphicLoader.hxx>
#include <vcl/outdev.hxx>

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <memory>


using namespace css;
using com::sun::star::uno::Reference;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::Sequence;
using com::sun::star::container::XNameContainer;
using com::sun::star::beans::XPropertySet;

#define WATERMARK_LUM_OFFSET        50
#define WATERMARK_CON_OFFSET        -70

namespace vcl::graphic
{

void SearchForGraphics(uno::Reference<uno::XInterface> const & xInterface,
                       std::vector<uno::Reference<css::graphic::XGraphic>> & raGraphicList)
{
    uno::Reference<beans::XPropertySet> xPropertySet(xInterface, UNO_QUERY);
    if (xPropertySet.is())
    {
        if (xPropertySet->getPropertySetInfo()->hasPropertyByName("ImageURL"))
        {
            OUString sURL;
            xPropertySet->getPropertyValue("ImageURL") >>= sURL;
            if (!sURL.isEmpty() && !GraphicObject::isGraphicObjectUniqueIdURL(sURL))
            {
                Graphic aGraphic = vcl::graphic::loadFromURL(sURL);
                if (!aGraphic.IsNone())
                {
                    raGraphicList.push_back(aGraphic.GetXGraphic());
                }
            }
        } else if (xPropertySet->getPropertySetInfo()->hasPropertyByName("Graphic"))
        {
            uno::Reference<css::graphic::XGraphic> xGraphic;
            xPropertySet->getPropertyValue("Graphic") >>= xGraphic;
            if (xGraphic.is())
            {
                raGraphicList.push_back(xGraphic);
            }
        }
    }
    Reference<XNameContainer> xContainer(xInterface, UNO_QUERY);
    if (xContainer.is())
    {
        const css::uno::Sequence<OUString> aElementNames = xContainer->getElementNames();
        for (OUString const & rName : aElementNames)
        {
            uno::Reference<XInterface> xInnerInterface;
            xContainer->getByName(rName) >>= xInnerInterface;
            SearchForGraphics(xInnerInterface, raGraphicList);
        }
    }
}

} // end namespace vcl::graphic

namespace
{

bool lclDrawObj( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                              GraphicObject const & rObj, const GraphicAttr& rAttr)
{
    Point   aPt( rPt );
    Size    aSz( rSz );
    bool    bRet = false;

    if( ( rObj.GetType() == GraphicType::Bitmap ) || ( rObj.GetType() == GraphicType::GdiMetafile ) )
    {
        // simple output of transformed graphic
        const Graphic aGraphic( rObj.GetTransformedGraphic( &rAttr ) );

        if( aGraphic.IsSupportedGraphic() )
        {
            const Degree10 nRot10 = rAttr.GetRotation() % 3600_deg10;

            if( nRot10 )
            {
                tools::Polygon aPoly( tools::Rectangle( aPt, aSz ) );

                aPoly.Rotate( aPt, nRot10 );
                const tools::Rectangle aRotBoundRect( aPoly.GetBoundRect() );
                aPt = aRotBoundRect.TopLeft();
                aSz = aRotBoundRect.GetSize();
            }

            aGraphic.Draw( pOut, aPt, aSz );
        }

        bRet = true;
    }

    return bRet;
}

void lclImplAdjust( BitmapEx& rBmpEx, const GraphicAttr& rAttr, GraphicAdjustmentFlags nAdjustmentFlags )
{
    GraphicAttr aAttr( rAttr );

    if( ( nAdjustmentFlags & GraphicAdjustmentFlags::DRAWMODE ) && aAttr.IsSpecialDrawMode() )
    {
        switch( aAttr.GetDrawMode() )
        {
            case GraphicDrawMode::Mono:
                rBmpEx.Convert( BmpConversion::N1BitThreshold );
            break;

            case GraphicDrawMode::Greys:
                rBmpEx.Convert( BmpConversion::N8BitGreys );
            break;

            case GraphicDrawMode::Watermark:
            {
                aAttr.SetLuminance( aAttr.GetLuminance() + WATERMARK_LUM_OFFSET );
                aAttr.SetContrast( aAttr.GetContrast() + WATERMARK_CON_OFFSET );
            }
            break;

            default:
            break;
        }
    }

    if( ( nAdjustmentFlags & GraphicAdjustmentFlags::COLORS ) && aAttr.IsAdjusted() )
    {
        rBmpEx.Adjust( aAttr.GetLuminance(), aAttr.GetContrast(),
                       aAttr.GetChannelR(), aAttr.GetChannelG(), aAttr.GetChannelB(),
                       aAttr.GetGamma(), aAttr.IsInvert() );
    }

    if( ( nAdjustmentFlags & GraphicAdjustmentFlags::MIRROR ) && aAttr.IsMirrored() )
    {
        rBmpEx.Mirror( aAttr.GetMirrorFlags() );
    }

    if( ( nAdjustmentFlags & GraphicAdjustmentFlags::ROTATE ) && aAttr.IsRotated() )
    {
        rBmpEx.Rotate( aAttr.GetRotation(), COL_TRANSPARENT );
    }

    if( ( nAdjustmentFlags & GraphicAdjustmentFlags::TRANSPARENCY ) && aAttr.IsTransparent() )
    {
        rBmpEx.AdjustTransparency(aAttr.GetTransparency());
    }
}

void lclImplAdjust( GDIMetaFile& rMtf, const GraphicAttr& rAttr, GraphicAdjustmentFlags nAdjustmentFlags )
{
    GraphicAttr aAttr( rAttr );

    if( ( nAdjustmentFlags & GraphicAdjustmentFlags::DRAWMODE ) && aAttr.IsSpecialDrawMode() )
    {
        switch( aAttr.GetDrawMode() )
        {
            case GraphicDrawMode::Mono:
                rMtf.Convert( MtfConversion::N1BitThreshold );
            break;

            case GraphicDrawMode::Greys:
                rMtf.Convert( MtfConversion::N8BitGreys );
            break;

            case GraphicDrawMode::Watermark:
            {
                aAttr.SetLuminance( aAttr.GetLuminance() + WATERMARK_LUM_OFFSET );
                aAttr.SetContrast( aAttr.GetContrast() + WATERMARK_CON_OFFSET );
            }
            break;

            default:
            break;
        }
    }

    if( ( nAdjustmentFlags & GraphicAdjustmentFlags::COLORS ) && aAttr.IsAdjusted() )
    {
        rMtf.Adjust( aAttr.GetLuminance(), aAttr.GetContrast(),
                     aAttr.GetChannelR(), aAttr.GetChannelG(), aAttr.GetChannelB(),
                     aAttr.GetGamma(), aAttr.IsInvert() );
    }

    if( ( nAdjustmentFlags & GraphicAdjustmentFlags::MIRROR ) && aAttr.IsMirrored() )
    {
        rMtf.Mirror( aAttr.GetMirrorFlags() );
    }

    if( ( nAdjustmentFlags & GraphicAdjustmentFlags::ROTATE ) && aAttr.IsRotated() )
    {
        rMtf.Rotate( aAttr.GetRotation() );
    }

    if( ( nAdjustmentFlags & GraphicAdjustmentFlags::TRANSPARENCY ) && aAttr.IsTransparent() )
    {
        OSL_FAIL( "Missing implementation: Mtf-Transparency" );
    }
}

void lclImplAdjust( Animation& rAnimation, const GraphicAttr& rAttr, GraphicAdjustmentFlags nAdjustmentFlags )
{
    GraphicAttr aAttr( rAttr );

    if( ( nAdjustmentFlags & GraphicAdjustmentFlags::DRAWMODE ) && aAttr.IsSpecialDrawMode() )
    {
        switch( aAttr.GetDrawMode() )
        {
            case GraphicDrawMode::Mono:
                rAnimation.Convert( BmpConversion::N1BitThreshold );
            break;

            case GraphicDrawMode::Greys:
                rAnimation.Convert( BmpConversion::N8BitGreys );
            break;

            case GraphicDrawMode::Watermark:
            {
                aAttr.SetLuminance( aAttr.GetLuminance() + WATERMARK_LUM_OFFSET );
                aAttr.SetContrast( aAttr.GetContrast() + WATERMARK_CON_OFFSET );
            }
            break;

            default:
            break;
        }
    }

    if( ( nAdjustmentFlags & GraphicAdjustmentFlags::COLORS ) && aAttr.IsAdjusted() )
    {
        rAnimation.Adjust( aAttr.GetLuminance(), aAttr.GetContrast(),
                           aAttr.GetChannelR(), aAttr.GetChannelG(), aAttr.GetChannelB(),
                           aAttr.GetGamma(), aAttr.IsInvert() );
    }

    if( ( nAdjustmentFlags & GraphicAdjustmentFlags::MIRROR ) && aAttr.IsMirrored() )
    {
        rAnimation.Mirror( aAttr.GetMirrorFlags() );
    }

    if( ( nAdjustmentFlags & GraphicAdjustmentFlags::ROTATE ) && aAttr.IsRotated() )
    {
        OSL_FAIL( "Missing implementation: Animation-Rotation" );
    }

    if( ( nAdjustmentFlags & GraphicAdjustmentFlags::TRANSPARENCY ) && aAttr.IsTransparent() )
    {
        OSL_FAIL( "Missing implementation: Animation-Transparency" );
    }
}

} // end anonymous namespace

struct GrfSimpleCacheObj
{
    Graphic     maGraphic;
    GraphicAttr maAttr;

                GrfSimpleCacheObj( const Graphic& rGraphic, const GraphicAttr& rAttr ) :
                    maGraphic( rGraphic ), maAttr( rAttr ) {}
};

GraphicObject::GraphicObject()
{
}

GraphicObject::GraphicObject(const Graphic& rGraphic)
    : maGraphic(rGraphic)
{
}

GraphicObject::GraphicObject(const GraphicObject& rGraphicObj)
    : maGraphic(rGraphicObj.GetGraphic())
    , maAttr(rGraphicObj.maAttr)
    , maUserData(rGraphicObj.maUserData)
{
}

GraphicObject::~GraphicObject()
{
}

GraphicType GraphicObject::GetType() const
{
    return maGraphic.GetType();
}

Size GraphicObject::GetPrefSize() const
{
    return maGraphic.GetPrefSize();
}

MapMode GraphicObject::GetPrefMapMode() const
{
    return maGraphic.GetPrefMapMode();
}

bool GraphicObject::IsTransparent() const
{
    return maGraphic.IsTransparent();
}

bool GraphicObject::IsAnimated() const
{
    return maGraphic.IsAnimated();
}

bool GraphicObject::IsEPS() const
{
    return maGraphic.IsEPS();
}

bool GraphicObject::ImplGetCropParams( OutputDevice const * pOut, Point& rPt, Size& rSz, const GraphicAttr* pAttr,
                                       tools::PolyPolygon& rClipPolyPoly, bool& bRectClipRegion ) const
{
    bool bRet = false;

    if( GetType() != GraphicType::NONE )
    {
        tools::Polygon aClipPoly( tools::Rectangle( rPt, rSz ) );
        const Degree10  nRot10 = pAttr->GetRotation() % 3600_deg10;
        const Point     aOldOrigin( rPt );
        const MapMode   aMap100( MapUnit::Map100thMM );
        Size            aSize100;
        tools::Long            nTotalWidth, nTotalHeight;

        if( nRot10 )
        {
            aClipPoly.Rotate( rPt, nRot10 );
            bRectClipRegion = false;
        }
        else
            bRectClipRegion = true;

        rClipPolyPoly = aClipPoly;

        if (maGraphic.GetPrefMapMode().GetMapUnit() == MapUnit::MapPixel)
            aSize100 = Application::GetDefaultDevice()->PixelToLogic( maGraphic.GetPrefSize(), aMap100 );
        else
        {
            MapMode m(maGraphic.GetPrefMapMode());
            aSize100 = pOut->LogicToLogic( maGraphic.GetPrefSize(), &m, &aMap100 );
        }

        nTotalWidth = aSize100.Width() - pAttr->GetLeftCrop() - pAttr->GetRightCrop();
        nTotalHeight = aSize100.Height() - pAttr->GetTopCrop() - pAttr->GetBottomCrop();

        if( !aSize100.IsEmpty() && nTotalWidth > 0 && nTotalHeight > 0 )
        {
            double fScale = static_cast<double>(aSize100.Width()) / nTotalWidth;
            const tools::Long nNewLeft = -FRound( ( ( pAttr->GetMirrorFlags() & BmpMirrorFlags::Horizontal ) ? pAttr->GetRightCrop() : pAttr->GetLeftCrop() ) * fScale );
            const tools::Long nNewRight = nNewLeft + FRound( aSize100.Width() * fScale ) - 1;

            fScale = static_cast<double>(rSz.Width()) / aSize100.Width();
            rPt.AdjustX(FRound( nNewLeft * fScale ) );
            rSz.setWidth( FRound( ( nNewRight - nNewLeft + 1 ) * fScale ) );

            fScale = static_cast<double>(aSize100.Height()) / nTotalHeight;
            const tools::Long nNewTop = -FRound( ( ( pAttr->GetMirrorFlags() & BmpMirrorFlags::Vertical ) ? pAttr->GetBottomCrop() : pAttr->GetTopCrop() ) * fScale );
            const tools::Long nNewBottom = nNewTop + FRound( aSize100.Height() * fScale ) - 1;

            fScale = static_cast<double>(rSz.Height()) / aSize100.Height();
            rPt.AdjustY(FRound( nNewTop * fScale ) );
            rSz.setHeight( FRound( ( nNewBottom - nNewTop + 1 ) * fScale ) );

            if( nRot10 )
            {
                tools::Polygon aOriginPoly( 1 );

                aOriginPoly[ 0 ] = rPt;
                aOriginPoly.Rotate( aOldOrigin, nRot10 );
                rPt = aOriginPoly[ 0 ];
            }

            bRet = true;
        }
    }

    return bRet;
}

GraphicObject& GraphicObject::operator=( const GraphicObject& rGraphicObj )
{
    if( &rGraphicObj != this )
    {
        mxSimpleCache.reset();
        maGraphic = rGraphicObj.GetGraphic();
        maAttr = rGraphicObj.maAttr;
        maUserData = rGraphicObj.maUserData;
    }

    return *this;
}

bool GraphicObject::operator==( const GraphicObject& rGraphicObj ) const
{
    return rGraphicObj.maGraphic == maGraphic
        && rGraphicObj.maAttr == maAttr;
}

OString GraphicObject::GetUniqueID() const
{
    return GetGraphic().getUniqueID();
}

void GraphicObject::SetAttr( const GraphicAttr& rAttr )
{
    maAttr = rAttr;

    if (mxSimpleCache && (mxSimpleCache->maAttr != rAttr))
        mxSimpleCache.reset();
}

void GraphicObject::SetUserData()
{
    maUserData.clear();
}

void GraphicObject::SetUserData( const OUString& rUserData )
{
    maUserData = rUserData;
}

bool GraphicObject::Draw( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                          const GraphicAttr* pAttr )
{
    GraphicAttr         aAttr( pAttr ? *pAttr : GetAttr() );
    Point               aPt( rPt );
    Size                aSz( rSz );
    const DrawModeFlags nOldDrawMode = pOut->GetDrawMode();
    bool                bCropped = aAttr.IsCropped();
    bool bRet;

    pOut->SetDrawMode( nOldDrawMode & ~DrawModeFlags( DrawModeFlags::SettingsLine | DrawModeFlags::SettingsFill | DrawModeFlags::SettingsText | DrawModeFlags::SettingsGradient ) );

    // mirrored horizontically
    if( aSz.Width() < 0 )
    {
        aPt.AdjustX(aSz.Width() + 1 );
        aSz.setWidth( -aSz.Width() );
        aAttr.SetMirrorFlags( aAttr.GetMirrorFlags() ^ BmpMirrorFlags::Horizontal );
    }

    // mirrored vertically
    if( aSz.Height() < 0 )
    {
        aPt.AdjustY(aSz.Height() + 1 );
        aSz.setHeight( -aSz.Height() );
        aAttr.SetMirrorFlags( aAttr.GetMirrorFlags() ^ BmpMirrorFlags::Vertical );
    }

    if( bCropped )
    {
        tools::PolyPolygon aClipPolyPoly;
        bool        bRectClip;
        const bool  bCrop = ImplGetCropParams( pOut, aPt, aSz, &aAttr, aClipPolyPoly, bRectClip );

        pOut->Push( PushFlags::CLIPREGION );

        if( bCrop )
        {
            if( bRectClip )
            {
                // #i29534# Store crop rect for later forwarding to
                // PDF writer
                tools::Rectangle aCropRect = aClipPolyPoly.GetBoundRect();
                pOut->IntersectClipRegion( aCropRect );
            }
            else
            {
                pOut->IntersectClipRegion(vcl::Region(aClipPolyPoly));
            }
        }
    }

    bRet = lclDrawObj(pOut, aPt, aSz, *this, aAttr);

    if( bCropped )
        pOut->Pop();

    pOut->SetDrawMode( nOldDrawMode );

    return bRet;
}

void GraphicObject::DrawTiled( OutputDevice* pOut, const tools::Rectangle& rArea, const Size& rSize,
                               const Size& rOffset, int nTileCacheSize1D )
{
    if( pOut == nullptr || rSize.IsEmpty() )
        return;

    const MapMode   aOutMapMode( pOut->GetMapMode() );
    // #106258# Clamp size to 1 for zero values. This is okay, since
    // logical size of zero is handled above already
    const Size      aOutTileSize( ::std::max( tools::Long(1), pOut->LogicToPixel( rSize, aOutMapMode ).Width() ),
                                  ::std::max( tools::Long(1), pOut->LogicToPixel( rSize, aOutMapMode ).Height() ) );

    //#i69780 clip final tile size to a sane max size
    while ((static_cast<sal_Int64>(rSize.Width()) * nTileCacheSize1D) > SAL_MAX_UINT16)
        nTileCacheSize1D /= 2;
    while ((static_cast<sal_Int64>(rSize.Height()) * nTileCacheSize1D) > SAL_MAX_UINT16)
        nTileCacheSize1D /= 2;

    ImplDrawTiled( pOut, rArea, aOutTileSize, rOffset, nullptr, nTileCacheSize1D );
}

bool GraphicObject::StartAnimation( OutputDevice* pOut, const Point& rPt, const Size& rSz,
                                    tools::Long nExtraData,
                                    OutputDevice* pFirstFrameOutDev )
{
    bool bRet = false;

    GetGraphic();

    const GraphicAttr aAttr( GetAttr() );

    if (IsAnimated())
    {
        Point   aPt( rPt );
        Size    aSz( rSz );
        bool    bCropped = aAttr.IsCropped();

        if( bCropped )
        {
            tools::PolyPolygon aClipPolyPoly;
            bool        bRectClip;
            const bool  bCrop = ImplGetCropParams( pOut, aPt, aSz, &aAttr, aClipPolyPoly, bRectClip );

            pOut->Push( PushFlags::CLIPREGION );

            if( bCrop )
            {
                if( bRectClip )
                    pOut->IntersectClipRegion( aClipPolyPoly.GetBoundRect() );
                else
                    pOut->IntersectClipRegion(vcl::Region(aClipPolyPoly));
            }
        }

        if (!mxSimpleCache || (mxSimpleCache->maAttr != aAttr) || pFirstFrameOutDev)
        {
            mxSimpleCache.reset(new GrfSimpleCacheObj(GetTransformedGraphic(&aAttr), aAttr));
            mxSimpleCache->maGraphic.SetAnimationNotifyHdl(GetGraphic().GetAnimationNotifyHdl());
        }

        mxSimpleCache->maGraphic.StartAnimation(pOut, aPt, aSz, nExtraData, pFirstFrameOutDev);

        if( bCropped )
            pOut->Pop();

        bRet = true;
    }
    else
        bRet = Draw( pOut, rPt, rSz, &aAttr );

    return bRet;
}

void GraphicObject::StopAnimation( const OutputDevice* pOut, tools::Long nExtraData )
{
    if (mxSimpleCache)
        mxSimpleCache->maGraphic.StopAnimation(pOut, nExtraData);
}

const Graphic& GraphicObject::GetGraphic() const
{
    return maGraphic;
}

void GraphicObject::SetGraphic( const Graphic& rGraphic)
{
    maGraphic = rGraphic;
}

Graphic GraphicObject::GetTransformedGraphic( const Size& rDestSize, const MapMode& rDestMap, const GraphicAttr& rAttr ) const
{
    // #104550# Extracted from svx/source/svdraw/svdograf.cxx
    Graphic             aTransGraphic( GetGraphic() );
    const GraphicType   eType = GetType();
    const Size          aSrcSize( aTransGraphic.GetPrefSize() );

    // #104115# Convert the crop margins to graphic object mapmode
    const MapMode aMapGraph( aTransGraphic.GetPrefMapMode() );
    const MapMode aMap100( MapUnit::Map100thMM );

    Size aCropLeftTop;
    Size aCropRightBottom;

    if( GraphicType::GdiMetafile == eType )
    {
        GDIMetaFile aMtf( aTransGraphic.GetGDIMetaFile() );

        if (aMapGraph.GetMapUnit() == MapUnit::MapPixel)
        {
            // crops are in 1/100th mm -> to aMapGraph -> to MapUnit::MapPixel
            aCropLeftTop = Application::GetDefaultDevice()->LogicToPixel(
                Size(rAttr.GetLeftCrop(), rAttr.GetTopCrop()),
                aMap100);
            aCropRightBottom = Application::GetDefaultDevice()->LogicToPixel(
                Size(rAttr.GetRightCrop(), rAttr.GetBottomCrop()),
                aMap100);
        }
        else
        {
            // crops are in GraphicObject units -> to aMapGraph
            aCropLeftTop = OutputDevice::LogicToLogic(
                Size(rAttr.GetLeftCrop(), rAttr.GetTopCrop()),
                aMap100,
                aMapGraph);
            aCropRightBottom = OutputDevice::LogicToLogic(
                Size(rAttr.GetRightCrop(), rAttr.GetBottomCrop()),
                aMap100,
                aMapGraph);
        }

        // #104115# If the metafile is cropped, give it a special
        // treatment: clip against the remaining area, scale up such
        // that this area later fills the desired size, and move the
        // origin to the upper left edge of that area.
        if( rAttr.IsCropped() )
        {
            const MapMode aMtfMapMode( aMtf.GetPrefMapMode() );

            tools::Rectangle aClipRect( aMtfMapMode.GetOrigin().X() + aCropLeftTop.Width(),
                                 aMtfMapMode.GetOrigin().Y() + aCropLeftTop.Height(),
                                 aMtfMapMode.GetOrigin().X() + aSrcSize.Width() - aCropRightBottom.Width(),
                                 aMtfMapMode.GetOrigin().Y() + aSrcSize.Height() - aCropRightBottom.Height() );

            // #104115# To correctly crop rotated metafiles, clip by view rectangle
            aMtf.AddAction( new MetaISectRectClipRegionAction( aClipRect ), 0 );

            // #104115# To crop the metafile, scale larger than the output rectangle
            aMtf.Scale( static_cast<double>(rDestSize.Width()) / (aSrcSize.Width() - aCropLeftTop.Width() - aCropRightBottom.Width()),
                        static_cast<double>(rDestSize.Height()) / (aSrcSize.Height() - aCropLeftTop.Height() - aCropRightBottom.Height()) );

            // #104115# Adapt the pref size by hand (scale changes it
            // proportionally, but we want it to be smaller than the
            // former size, to crop the excess out)
            aMtf.SetPrefSize( Size( static_cast<tools::Long>(static_cast<double>(rDestSize.Width()) *  (1.0 + (aCropLeftTop.Width() + aCropRightBottom.Width()) / aSrcSize.Width())  + .5),
                                    static_cast<tools::Long>(static_cast<double>(rDestSize.Height()) * (1.0 + (aCropLeftTop.Height() + aCropRightBottom.Height()) / aSrcSize.Height()) + .5) ) );

            // #104115# Adapt the origin of the new mapmode, such that it
            // is shifted to the place where the cropped output starts
            Point aNewOrigin( static_cast<tools::Long>(static_cast<double>(aMtfMapMode.GetOrigin().X()) + rDestSize.Width() * aCropLeftTop.Width() / (aSrcSize.Width() - aCropLeftTop.Width() - aCropRightBottom.Width()) + .5),
                              static_cast<tools::Long>(static_cast<double>(aMtfMapMode.GetOrigin().Y()) + rDestSize.Height() * aCropLeftTop.Height() / (aSrcSize.Height() - aCropLeftTop.Height() - aCropRightBottom.Height()) + .5) );
            MapMode aNewMap( rDestMap );
            aNewMap.SetOrigin( OutputDevice::LogicToLogic(aNewOrigin, aMtfMapMode, rDestMap) );
            aMtf.SetPrefMapMode( aNewMap );
        }
        else
        {
            aMtf.Scale( Fraction( rDestSize.Width(), aSrcSize.Width() ), Fraction( rDestSize.Height(), aSrcSize.Height() ) );
            aMtf.SetPrefMapMode( rDestMap );
        }

        aTransGraphic = aMtf;
    }
    else if( GraphicType::Bitmap == eType )
    {
        BitmapEx aBitmapEx( aTransGraphic.GetBitmapEx() );
        tools::Rectangle aCropRect;

        // convert crops to pixel
        if(rAttr.IsCropped())
        {
            if (aMapGraph.GetMapUnit() == MapUnit::MapPixel)
            {
                // crops are in 1/100th mm -> to MapUnit::MapPixel
                aCropLeftTop = Application::GetDefaultDevice()->LogicToPixel(
                    Size(rAttr.GetLeftCrop(), rAttr.GetTopCrop()),
                    aMap100);
                aCropRightBottom = Application::GetDefaultDevice()->LogicToPixel(
                    Size(rAttr.GetRightCrop(), rAttr.GetBottomCrop()),
                    aMap100);
            }
            else
            {
                // crops are in GraphicObject units -> to MapUnit::MapPixel
                aCropLeftTop = Application::GetDefaultDevice()->LogicToPixel(
                    Size(rAttr.GetLeftCrop(), rAttr.GetTopCrop()),
                    aMapGraph);
                aCropRightBottom = Application::GetDefaultDevice()->LogicToPixel(
                    Size(rAttr.GetRightCrop(), rAttr.GetBottomCrop()),
                    aMapGraph);
            }

            // convert from prefmapmode to pixel
            Size aSrcSizePixel(
                Application::GetDefaultDevice()->LogicToPixel(
                    aSrcSize,
                    aMapGraph));

            if(rAttr.IsCropped()
                && (aSrcSizePixel.Width() != aBitmapEx.GetSizePixel().Width() || aSrcSizePixel.Height() != aBitmapEx.GetSizePixel().Height())
                && aSrcSizePixel.Width())
            {
                // the size in pixels calculated from Graphic's internal MapMode (aTransGraphic.GetPrefMapMode())
                // and its internal size (aTransGraphic.GetPrefSize()) is different from its real pixel size.
                // This can be interpreted as this values to be set wrong, but needs to be corrected since e.g.
                // existing cropping is calculated based on this logic values already.
                // aBitmapEx.Scale(aSrcSizePixel);

                // another possibility is to adapt the values created so far with a factor; this
                // will keep the original Bitmap untouched and thus quality will not change
                // caution: convert to double first, else pretty big errors may occur
                const double fFactorX(static_cast<double>(aBitmapEx.GetSizePixel().Width()) / aSrcSizePixel.Width());
                const double fFactorY(static_cast<double>(aBitmapEx.GetSizePixel().Height()) / aSrcSizePixel.Height());

                aCropLeftTop.setWidth( basegfx::fround(aCropLeftTop.Width() * fFactorX) );
                aCropLeftTop.setHeight( basegfx::fround(aCropLeftTop.Height() * fFactorY) );
                aCropRightBottom.setWidth( basegfx::fround(aCropRightBottom.Width() * fFactorX) );
                aCropRightBottom.setHeight( basegfx::fround(aCropRightBottom.Height() * fFactorY) );

                aSrcSizePixel = aBitmapEx.GetSizePixel();
            }

            // setup crop rectangle in pixel
            aCropRect = tools::Rectangle( aCropLeftTop.Width(), aCropLeftTop.Height(),
                                 aSrcSizePixel.Width() - aCropRightBottom.Width(),
                                 aSrcSizePixel.Height() - aCropRightBottom.Height() );
        }

        // #105641# Also crop animations
        if( aTransGraphic.IsAnimated() )
        {
            Animation aAnim( aTransGraphic.GetAnimation() );

            for( size_t nFrame=0; nFrame<aAnim.Count(); ++nFrame )
            {
                AnimationBitmap aAnimationBitmap( aAnim.Get( nFrame ) );

                if( !aCropRect.IsInside( tools::Rectangle(aAnimationBitmap.maPositionPixel, aAnimationBitmap.maSizePixel) ) )
                {
                    // setup actual cropping (relative to frame position)
                    tools::Rectangle aCropRectRel( aCropRect );
                    aCropRectRel.Move( -aAnimationBitmap.maPositionPixel.X(),
                                       -aAnimationBitmap.maPositionPixel.Y() );

                    // cropping affects this frame, apply it then
                    // do _not_ apply enlargement, this is done below
                    ImplTransformBitmap( aAnimationBitmap.maBitmapEx, rAttr, Size(), Size(),
                                         aCropRectRel, rDestSize, false );

                    aAnim.Replace( aAnimationBitmap, nFrame );
                }
                // else: bitmap completely within crop area,
                // i.e. nothing is cropped away
            }

            // now, apply enlargement (if any) through global animation size
            if( aCropLeftTop.Width() < 0 ||
                aCropLeftTop.Height() < 0 ||
                aCropRightBottom.Width() < 0 ||
                aCropRightBottom.Height() < 0 )
            {
                Size aNewSize( aAnim.GetDisplaySizePixel() );
                aNewSize.AdjustWidth(aCropRightBottom.Width() < 0 ? -aCropRightBottom.Width() : 0 );
                aNewSize.AdjustWidth(aCropLeftTop.Width() < 0 ? -aCropLeftTop.Width() : 0 );
                aNewSize.AdjustHeight(aCropRightBottom.Height() < 0 ? -aCropRightBottom.Height() : 0 );
                aNewSize.AdjustHeight(aCropLeftTop.Height() < 0 ? -aCropLeftTop.Height() : 0 );
                aAnim.SetDisplaySizePixel( aNewSize );
            }

            // if topleft has changed, we must move all frames to the
            // right and bottom, resp.
            if( aCropLeftTop.Width() < 0 ||
                aCropLeftTop.Height() < 0 )
            {
                Point aPosOffset( aCropLeftTop.Width() < 0 ? -aCropLeftTop.Width() : 0,
                                  aCropLeftTop.Height() < 0 ? -aCropLeftTop.Height() : 0 );

                for( size_t nFrame=0; nFrame<aAnim.Count(); ++nFrame )
                {
                    AnimationBitmap aAnimationBitmap( aAnim.Get( nFrame ) );

                    aAnimationBitmap.maPositionPixel += aPosOffset;

                    aAnim.Replace( aAnimationBitmap, nFrame );
                }
            }

            aTransGraphic = aAnim;
        }
        else
        {
            ImplTransformBitmap( aBitmapEx, rAttr, aCropLeftTop, aCropRightBottom,
                                 aCropRect, rDestSize, true );

            aTransGraphic = aBitmapEx;
        }

        aTransGraphic.SetPrefSize( rDestSize );
        aTransGraphic.SetPrefMapMode( rDestMap );
    }

    GraphicObject aGrfObj( aTransGraphic );
    aTransGraphic = aGrfObj.GetTransformedGraphic( &rAttr );

    return aTransGraphic;
}

Graphic GraphicObject::GetTransformedGraphic( const GraphicAttr* pAttr ) const
{
    GetGraphic();

    Graphic     aGraphic;
    GraphicAttr aAttr( pAttr ? *pAttr : GetAttr() );

    if (maGraphic.IsSupportedGraphic())
    {
        if( aAttr.IsSpecialDrawMode() || aAttr.IsAdjusted() || aAttr.IsMirrored() || aAttr.IsRotated() || aAttr.IsTransparent() )
        {
            if( GetType() == GraphicType::Bitmap )
            {
                if( IsAnimated() )
                {
                    Animation aAnimation( maGraphic.GetAnimation() );
                    lclImplAdjust( aAnimation, aAttr, GraphicAdjustmentFlags::ALL );
                    aAnimation.SetLoopCount(maGraphic.GetAnimationLoopCount());
                    aGraphic = aAnimation;
                }
                else
                {
                    BitmapEx aBmpEx( maGraphic.GetBitmapEx() );
                    lclImplAdjust( aBmpEx, aAttr, GraphicAdjustmentFlags::ALL );
                    aGraphic = aBmpEx;
                }
            }
            else
            {
                GDIMetaFile aMtf( maGraphic.GetGDIMetaFile() );
                lclImplAdjust( aMtf, aAttr, GraphicAdjustmentFlags::ALL );
                aGraphic = aMtf;
            }
        }
        else
        {
            if( ( GetType() == GraphicType::Bitmap ) && IsAnimated() )
            {
                Animation aAnimation( maGraphic.GetAnimation() );
                aAnimation.SetLoopCount(maGraphic.GetAnimationLoopCount());
                aGraphic = aAnimation;
            }
            else
                aGraphic = maGraphic;
        }
    }

    return aGraphic;
}

bool GraphicObject::isGraphicObjectUniqueIdURL(OUString const & rURL)
{
    return rURL.startsWith("vnd.sun.star.GraphicObject:");
}

// calculate scalings between real image size and logic object size. This
// is necessary since the crop values are relative to original bitmap size
basegfx::B2DVector GraphicObject::calculateCropScaling(
    double fWidth,
    double fHeight,
    double fLeftCrop,
    double fTopCrop,
    double fRightCrop,
    double fBottomCrop) const
{
    const MapMode aMapMode100thmm(MapUnit::Map100thMM);
    Size aBitmapSize(GetPrefSize());
    double fFactorX(1.0);
    double fFactorY(1.0);

    if(MapUnit::MapPixel == GetPrefMapMode().GetMapUnit())
    {
        aBitmapSize = Application::GetDefaultDevice()->PixelToLogic(aBitmapSize, aMapMode100thmm);
    }
    else
    {
        aBitmapSize = OutputDevice::LogicToLogic(aBitmapSize, GetPrefMapMode(), aMapMode100thmm);
    }

    const double fDivX(aBitmapSize.Width() - fLeftCrop - fRightCrop);
    const double fDivY(aBitmapSize.Height() - fTopCrop - fBottomCrop);

    if(!basegfx::fTools::equalZero(fDivX))
    {
        fFactorX = fabs(fWidth) / fDivX;
    }

    if(!basegfx::fTools::equalZero(fDivY))
    {
        fFactorY = fabs(fHeight) / fDivY;
    }

    return basegfx::B2DVector(fFactorX,fFactorY);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
