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

#include <drawingml/fillproperties.hxx>

#include <iterator>

#include <comphelper/propertyvalue.hxx>
#include <drawingml/graphicproperties.hxx>
#include <vcl/graph.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/ColorMode.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/RectanglePoint.hpp>
#include <com/sun/star/graphic/XGraphicTransformer.hpp>
#include <oox/helper/graphichelper.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <oox/drawingml/shapepropertymap.hxx>
#include <drawingml/hatchmap.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::graphic;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::geometry::IntegerRectangle2D;

namespace oox::drawingml {

namespace {

Reference< XGraphic > lclCheckAndApplyDuotoneTransform(const BlipFillProperties& aBlipProps, uno::Reference<graphic::XGraphic> const & xGraphic,
                                                       const GraphicHelper& rGraphicHelper, const ::Color nPhClr)
{
    if (aBlipProps.maDuotoneColors[0].isUsed() && aBlipProps.maDuotoneColors[1].isUsed())
    {
        ::Color nColor1 = aBlipProps.maDuotoneColors[0].getColor( rGraphicHelper, nPhClr );
        ::Color nColor2 = aBlipProps.maDuotoneColors[1].getColor( rGraphicHelper, nPhClr );

        uno::Reference<graphic::XGraphicTransformer> xTransformer(aBlipProps.mxFillGraphic, uno::UNO_QUERY);
        if (xTransformer.is())
            return xTransformer->applyDuotone(xGraphic, sal_Int32(nColor1), sal_Int32(nColor2));
    }
    return xGraphic;
}

Reference< XGraphic > lclRotateGraphic(uno::Reference<graphic::XGraphic> const & xGraphic, Degree10 nRotation)
{
    ::Graphic aGraphic(xGraphic);
    ::Graphic aReturnGraphic;

    assert (aGraphic.GetType() == GraphicType::Bitmap);

    BitmapEx aBitmapEx(aGraphic.GetBitmapEx());
    const ::Color& aColor = ::Color(0x00);
    aBitmapEx.Rotate(nRotation, aColor);
    aReturnGraphic = ::Graphic(aBitmapEx);
    aReturnGraphic.setOriginURL(aGraphic.getOriginURL());

    return aReturnGraphic.GetXGraphic();
}

void lclCalculateCropPercentage(uno::Reference<graphic::XGraphic> const & xGraphic, geometry::IntegerRectangle2D &aFillRect)
{
    ::Graphic aGraphic(xGraphic);
    assert (aGraphic.GetType() == GraphicType::Bitmap);

    BitmapEx aBitmapEx(aGraphic.GetBitmapEx());

    sal_Int32 nScaledWidth = aBitmapEx.GetSizePixel().Width();
    sal_Int32 nScaledHeight = aBitmapEx.GetSizePixel().Height();

    sal_Int32 nOrigWidth = (nScaledWidth * (100000 - aFillRect.X1 - aFillRect.X2)) / 100000;
    sal_Int32 nOrigHeight = (nScaledHeight * (100000 - aFillRect.Y1 - aFillRect.Y2)) / 100000;

    sal_Int32 nLeftPercentage = nScaledWidth * aFillRect.X1 / nOrigWidth;
    sal_Int32 nRightPercentage = nScaledWidth * aFillRect.X2 / nOrigWidth;
    sal_Int32 nTopPercentage = nScaledHeight * aFillRect.Y1 / nOrigHeight;
    sal_Int32 nBottomPercentage = nScaledHeight * aFillRect.Y2 / nOrigHeight;

    aFillRect.X1 = -nLeftPercentage;
    aFillRect.X2 = -nRightPercentage;
    aFillRect.Y1 = -nTopPercentage;
    aFillRect.Y2 = -nBottomPercentage;
}

// Crops a piece of the bitmap. Takes negative aFillRect values. Negative values means "crop",
// positive values means "grow" bitmap with empty spaces. lclCropGraphic doesn't handle growing.
Reference< XGraphic > lclCropGraphic(uno::Reference<graphic::XGraphic> const & xGraphic, geometry::IntegerRectangle2D aFillRect)
{
    ::Graphic aGraphic(xGraphic);
    ::Graphic aReturnGraphic;

    assert (aGraphic.GetType() == GraphicType::Bitmap);

    BitmapEx aBitmapEx(aGraphic.GetBitmapEx());

    sal_Int32 nOrigHeight = aBitmapEx.GetSizePixel().Height();
    sal_Int32 nHeight = nOrigHeight;
    sal_Int32 nTopCorr  = nOrigHeight * -1 * static_cast<double>(aFillRect.Y1) / 100000;
    nHeight += nTopCorr;
    sal_Int32 nBottomCorr = nOrigHeight * -1 * static_cast<double>(aFillRect.Y2) / 100000;
    nHeight += nBottomCorr;

    sal_Int32 nOrigWidth = aBitmapEx.GetSizePixel().Width();
    sal_Int32 nWidth = nOrigWidth;
    sal_Int32 nLeftCorr  = nOrigWidth * -1 * static_cast<double>(aFillRect.X1) / 100000;
    nWidth += nLeftCorr;
    sal_Int32 nRightCorr = nOrigWidth * -1 * static_cast<double>(aFillRect.X2) / 100000;
    nWidth += nRightCorr;

    aBitmapEx.Scale(Size(nWidth, nHeight));
    aBitmapEx.Crop(tools::Rectangle(Point(nLeftCorr, nTopCorr), Size(nOrigWidth, nOrigHeight)));

    aReturnGraphic = ::Graphic(aBitmapEx);
    aReturnGraphic.setOriginURL(aGraphic.getOriginURL());

    return aReturnGraphic.GetXGraphic();
}

Reference< XGraphic > lclMirrorGraphic(uno::Reference<graphic::XGraphic> const & xGraphic, bool bFlipH, bool bFlipV)
{
    ::Graphic aGraphic(xGraphic);
    ::Graphic aReturnGraphic;

    assert (aGraphic.GetType() == GraphicType::Bitmap);

    BitmapEx aBitmapEx(aGraphic.GetBitmapEx());
    BmpMirrorFlags nMirrorFlags = BmpMirrorFlags::NONE;

    if(bFlipH)
        nMirrorFlags |= BmpMirrorFlags::Horizontal;
    if(bFlipV)
        nMirrorFlags |= BmpMirrorFlags::Vertical;

    aBitmapEx.Mirror(nMirrorFlags);

    aReturnGraphic = ::Graphic(aBitmapEx);
    aReturnGraphic.setOriginURL(aGraphic.getOriginURL());

    return aReturnGraphic.GetXGraphic();
}

Reference< XGraphic > lclGreysScaleGraphic(uno::Reference<graphic::XGraphic> const & xGraphic)
{
    ::Graphic aGraphic(xGraphic);
    ::Graphic aReturnGraphic;

    assert (aGraphic.GetType() == GraphicType::Bitmap);

    BitmapEx aBitmapEx(aGraphic.GetBitmapEx());
    aBitmapEx.Convert(BmpConversion::N8BitGreys);

    aReturnGraphic = ::Graphic(aBitmapEx);
    aReturnGraphic.setOriginURL(aGraphic.getOriginURL());

    return aReturnGraphic.GetXGraphic();
}

Reference< XGraphic > lclCheckAndApplyChangeColorTransform(const BlipFillProperties &aBlipProps, uno::Reference<graphic::XGraphic> const & xGraphic,
                                                           const GraphicHelper& rGraphicHelper, const ::Color nPhClr)
{
    if( aBlipProps.maColorChangeFrom.isUsed() && aBlipProps.maColorChangeTo.isUsed() )
    {
        ::Color nFromColor = aBlipProps.maColorChangeFrom.getColor( rGraphicHelper, nPhClr );
        ::Color nToColor = aBlipProps.maColorChangeTo.getColor( rGraphicHelper, nPhClr );
        if ( (nFromColor != nToColor) || aBlipProps.maColorChangeTo.hasTransparency() )
        {
            sal_Int16 nToTransparence = aBlipProps.maColorChangeTo.getTransparency();
            sal_Int8 nToAlpha = static_cast< sal_Int8 >( (100 - nToTransparence) * 2.55 );

            uno::Reference<graphic::XGraphicTransformer> xTransformer(aBlipProps.mxFillGraphic, uno::UNO_QUERY);
            if (xTransformer.is())
                return xTransformer->colorChange(xGraphic, sal_Int32(nFromColor), 9, sal_Int32(nToColor), nToAlpha);
        }
    }
    return xGraphic;
}

uno::Reference<graphic::XGraphic> applyBrightnessContrast(uno::Reference<graphic::XGraphic> const & xGraphic, sal_Int32 brightness, sal_Int32 contrast)
{
    uno::Reference<graphic::XGraphicTransformer> xTransformer(xGraphic, uno::UNO_QUERY);
    if (xTransformer.is())
        return xTransformer->applyBrightnessContrast(xGraphic, brightness, contrast, true);
    return xGraphic;
}

BitmapMode lclGetBitmapMode( sal_Int32 nToken )
{
    OSL_ASSERT((nToken & sal_Int32(0xFFFF0000))==0);
    switch( nToken )
    {
        case XML_tile:      return BitmapMode_REPEAT;
        case XML_stretch:   return BitmapMode_STRETCH;
    }
    return BitmapMode_NO_REPEAT;
}

RectanglePoint lclGetRectanglePoint( sal_Int32 nToken )
{
    OSL_ASSERT((nToken & sal_Int32(0xFFFF0000))==0);
    switch( nToken )
    {
        case XML_tl:    return RectanglePoint_LEFT_TOP;
        case XML_t:     return RectanglePoint_MIDDLE_TOP;
        case XML_tr:    return RectanglePoint_RIGHT_TOP;
        case XML_l:     return RectanglePoint_LEFT_MIDDLE;
        case XML_ctr:   return RectanglePoint_MIDDLE_MIDDLE;
        case XML_r:     return RectanglePoint_RIGHT_MIDDLE;
        case XML_bl:    return RectanglePoint_LEFT_BOTTOM;
        case XML_b:     return RectanglePoint_MIDDLE_BOTTOM;
        case XML_br:    return RectanglePoint_RIGHT_BOTTOM;
    }
    return RectanglePoint_LEFT_TOP;
}

awt::Size lclGetOriginalSize( const GraphicHelper& rGraphicHelper, const Reference< XGraphic >& rxGraphic )
{
    awt::Size aSizeHmm( 0, 0 );
    try
    {
        Reference< beans::XPropertySet > xGraphicPropertySet( rxGraphic, UNO_QUERY_THROW );
        if( xGraphicPropertySet->getPropertyValue( "Size100thMM" ) >>= aSizeHmm )
        {
            if( !aSizeHmm.Width && !aSizeHmm.Height )
            {   // MAPMODE_PIXEL USED :-(
                awt::Size aSourceSizePixel( 0, 0 );
                if( xGraphicPropertySet->getPropertyValue( "SizePixel" ) >>= aSourceSizePixel )
                    aSizeHmm = rGraphicHelper.convertScreenPixelToHmm( aSourceSizePixel );
            }
        }
    }
    catch( Exception& )
    {
    }
    return aSizeHmm;
}

/**
 * Looks for a last gradient transition and possibly sets a gradient border
 * based on that.
 */
void extractGradientBorderFromStops(const GradientFillProperties& rGradientProps,
                                    const GraphicHelper& rGraphicHelper, ::Color nPhClr,
                                    awt::Gradient& rGradient)
{
    if (rGradientProps.maGradientStops.size() <= 1)
        return;

    auto it = rGradientProps.maGradientStops.rbegin();
    double fLastPos = it->first;
    Color aLastColor = it->second;
    ++it;
    double fLastButOnePos = it->first;
    Color aLastButOneColor = it->second;
    if (!aLastColor.equals(aLastButOneColor, rGraphicHelper, nPhClr))
        return;

    // Last transition has the same color, we can map that to a border.
    rGradient.Border = rtl::math::round((fLastPos - fLastButOnePos) * 100);
}

} // namespace

void GradientFillProperties::assignUsed( const GradientFillProperties& rSourceProps )
{
    if( !rSourceProps.maGradientStops.empty() )
        maGradientStops = rSourceProps.maGradientStops;
    moFillToRect.assignIfUsed( rSourceProps.moFillToRect );
    moTileRect.assignIfUsed( rSourceProps.moTileRect );
    moGradientPath.assignIfUsed( rSourceProps.moGradientPath );
    moShadeAngle.assignIfUsed( rSourceProps.moShadeAngle );
    moShadeFlip.assignIfUsed( rSourceProps.moShadeFlip );
    moShadeScaled.assignIfUsed( rSourceProps.moShadeScaled );
    moRotateWithShape.assignIfUsed( rSourceProps.moRotateWithShape );
}

void PatternFillProperties::assignUsed( const PatternFillProperties& rSourceProps )
{
    maPattFgColor.assignIfUsed( rSourceProps.maPattFgColor );
    maPattBgColor.assignIfUsed( rSourceProps.maPattBgColor );
    moPattPreset.assignIfUsed( rSourceProps.moPattPreset );
}

void BlipFillProperties::assignUsed( const BlipFillProperties& rSourceProps )
{
    if(rSourceProps.mxFillGraphic.is())
        mxFillGraphic = rSourceProps.mxFillGraphic;
    moBitmapMode.assignIfUsed( rSourceProps.moBitmapMode );
    moFillRect.assignIfUsed( rSourceProps.moFillRect );
    moTileOffsetX.assignIfUsed( rSourceProps.moTileOffsetX );
    moTileOffsetY.assignIfUsed( rSourceProps.moTileOffsetY );
    moTileScaleX.assignIfUsed( rSourceProps.moTileScaleX );
    moTileScaleY.assignIfUsed( rSourceProps.moTileScaleY );
    moTileAlign.assignIfUsed( rSourceProps.moTileAlign );
    moTileFlip.assignIfUsed( rSourceProps.moTileFlip );
    moRotateWithShape.assignIfUsed( rSourceProps.moRotateWithShape );
    moColorEffect.assignIfUsed( rSourceProps.moColorEffect );
    moBrightness.assignIfUsed( rSourceProps.moBrightness );
    moContrast.assignIfUsed( rSourceProps.moContrast );
    maColorChangeFrom.assignIfUsed( rSourceProps.maColorChangeFrom );
    maColorChangeTo.assignIfUsed( rSourceProps.maColorChangeTo );
    maDuotoneColors[0].assignIfUsed( rSourceProps.maDuotoneColors[0] );
    maDuotoneColors[1].assignIfUsed( rSourceProps.maDuotoneColors[1] );
    maEffect.assignUsed( rSourceProps.maEffect );
    moAlphaModFix.assignIfUsed(rSourceProps.moAlphaModFix);
}

void FillProperties::assignUsed( const FillProperties& rSourceProps )
{
    moFillType.assignIfUsed( rSourceProps.moFillType );
    maFillColor.assignIfUsed( rSourceProps.maFillColor );
    maGradientProps.assignUsed( rSourceProps.maGradientProps );
    maPatternProps.assignUsed( rSourceProps.maPatternProps );
    maBlipProps.assignUsed( rSourceProps.maBlipProps );
}

Color FillProperties::getBestSolidColor() const
{
    Color aSolidColor;
    if( moFillType.has() ) switch( moFillType.get() )
    {
        case XML_solidFill:
            aSolidColor = maFillColor;
        break;
        case XML_gradFill:
            if( !maGradientProps.maGradientStops.empty() )
            {
                GradientFillProperties::GradientStopMap::const_iterator aGradientStop =
                    maGradientProps.maGradientStops.begin();
                if (maGradientProps.maGradientStops.size() > 2)
                    ++aGradientStop;
                aSolidColor = aGradientStop->second;
            }
        break;
        case XML_pattFill:
            aSolidColor = maPatternProps.maPattBgColor.isUsed() ? maPatternProps.maPattBgColor : maPatternProps.maPattFgColor;
        break;
    }
    return aSolidColor;
}

void FillProperties::pushToPropMap( ShapePropertyMap& rPropMap,
        const GraphicHelper& rGraphicHelper, sal_Int32 nShapeRotation, ::Color nPhClr,
        bool bFlipH, bool bFlipV, bool bIsCustomShape) const
{
    if( !moFillType.has() )
        return;

    FillStyle eFillStyle = FillStyle_NONE;
    OSL_ASSERT((moFillType.get() & sal_Int32(0xFFFF0000))==0);
    switch( moFillType.get() )
    {
        case XML_noFill:
            eFillStyle = FillStyle_NONE;
        break;

        case XML_solidFill:
            if( maFillColor.isUsed() )
            {
                rPropMap.setProperty( ShapeProperty::FillColor, maFillColor.getColor( rGraphicHelper, nPhClr ) );
                if( maFillColor.hasTransparency() )
                    rPropMap.setProperty( ShapeProperty::FillTransparency, maFillColor.getTransparency() );
                eFillStyle = FillStyle_SOLID;
            }
        break;

        case XML_gradFill:
            // do not create gradient struct if property is not supported...
            if( rPropMap.supportsProperty( ShapeProperty::FillGradient ) )
            {
                sal_Int32 nEndTrans     = 0;
                sal_Int32 nStartTrans   = 0;
                awt::Gradient aGradient;
                aGradient.Angle = 900;
                aGradient.StartIntensity = 100;
                aGradient.EndIntensity = 100;

                // Old code, values in aGradient overwritten in many cases by newer code below
                if( maGradientProps.maGradientStops.size() > 1 )
                {
                    aGradient.StartColor = sal_Int32(maGradientProps.maGradientStops.begin()->second.getColor( rGraphicHelper, nPhClr ));
                    aGradient.EndColor = sal_Int32(maGradientProps.maGradientStops.rbegin()->second.getColor( rGraphicHelper, nPhClr ));
                    if( maGradientProps.maGradientStops.rbegin()->second.hasTransparency() )
                        nEndTrans = maGradientProps.maGradientStops.rbegin()->second.getTransparency()*255/100;
                    if( maGradientProps.maGradientStops.begin()->second.hasTransparency() )
                        nStartTrans = maGradientProps.maGradientStops.begin()->second.getTransparency()*255/100;
                }

                // "rotate with shape" set to false -> do not rotate
                if ( !maGradientProps.moRotateWithShape.get( true ) )
                    nShapeRotation = 0;

                if( maGradientProps.moGradientPath.has() )
                {
                    IntegerRectangle2D aFillToRect = maGradientProps.moFillToRect.get( IntegerRectangle2D( 0, 0, MAX_PERCENT, MAX_PERCENT ) );
                    sal_Int32 nCenterX = (MAX_PERCENT + aFillToRect.X1 - aFillToRect.X2) / 2;
                    aGradient.XOffset = getLimitedValue<sal_Int16, sal_Int32>(
                        nCenterX / PER_PERCENT, 0, 100);
                    sal_Int32 nCenterY = (MAX_PERCENT + aFillToRect.Y1 - aFillToRect.Y2) / 2;
                    aGradient.YOffset = getLimitedValue<sal_Int16, sal_Int32>(
                        nCenterY / PER_PERCENT, 0, 100);

                    if( maGradientProps.moGradientPath.get() == XML_circle )
                    {
                        // Style should be radial at least when the horizontal center is at 50%.
                        // Otherwise import as a linear gradient, because it is the most similar to the MSO radial style.
                        aGradient.Style = awt::GradientStyle_LINEAR;
                        if( aGradient.XOffset == 100 && aGradient.YOffset == 100 )
                            aGradient.Angle = 450;
                        else if( aGradient.XOffset == 0 && aGradient.YOffset == 100 )
                            aGradient.Angle = 3150;
                        else if( aGradient.XOffset == 100 && aGradient.YOffset == 0 )
                            aGradient.Angle = 1350;
                        else if( aGradient.XOffset == 0 && aGradient.YOffset == 0 )
                            aGradient.Angle = 2250;
                        else
                            aGradient.Style = awt::GradientStyle_RADIAL;
                    }
                    else
                    {
                        aGradient.Style = awt::GradientStyle_RECT;
                    }

                    ::std::swap( aGradient.StartColor, aGradient.EndColor );
                    ::std::swap( nStartTrans, nEndTrans );

                    extractGradientBorderFromStops(maGradientProps, rGraphicHelper, nPhClr,
                                                   aGradient);
                }
                else if (!maGradientProps.maGradientStops.empty())
                {
                    // A copy of the gradient stops for local modification
                    GradientFillProperties::GradientStopMap aGradientStops(maGradientProps.maGradientStops);

                    // Add a fake gradient stop at 0% and 100% if necessary, so that the gradient always starts
                    // at 0% and ends at 100%, to make following logic clearer (?).
                    auto a0 = aGradientStops.find( 0.0 );
                    if( a0 == aGradientStops.end() )
                    {
                        // temp variable required
                        Color aFirstColor(aGradientStops.begin()->second);
                        aGradientStops.emplace( 0.0, aFirstColor );
                    }

                    auto a1 = aGradientStops.find( 1.0 );
                    if( a1 == aGradientStops.end() )
                    {
                        // ditto
                        Color aLastColor(aGradientStops.rbegin()->second);
                        aGradientStops.emplace( 1.0, aLastColor );
                    }

                    // Check if the gradient is symmetric, which we will emulate with an "axial" gradient.
                    bool bSymmetric(true);
                    {
                        GradientFillProperties::GradientStopMap::const_iterator aItA( aGradientStops.begin() );
                        GradientFillProperties::GradientStopMap::const_iterator aItZ(std::prev(aGradientStops.end()));
                        assert(aItZ != aGradientStops.end());
                        while( bSymmetric && aItA->first < aItZ->first )
                        {
                            if (!aItA->second.equals(aItZ->second, rGraphicHelper, nPhClr))
                                bSymmetric = false;
                            else
                            {
                                ++aItA;
                                aItZ = std::prev(aItZ);
                            }
                        }
                        // Don't be fooled if the middlemost stop isn't at 0.5.
                        if( bSymmetric && aItA == aItZ && aItA->first != 0.5 )
                            bSymmetric = false;

                        // If symmetric, do the rest of the logic for just a half.
                        if( bSymmetric )
                        {
                            // aItZ already points to the colour for the middle, but insert a fake stop at the
                            // exact middle if necessary.
                            if( aItA->first != aItZ->first )
                            {
                                Color aMiddleColor = aItZ->second;
                                auto a05 = aGradientStops.find( 0.5 );

                                if( a05 != aGradientStops.end() )
                                    a05->second = aMiddleColor;
                                else
                                    aGradientStops.emplace( 0.5, aMiddleColor );
                            }
                            // Drop the rest of the stops
                            while( aGradientStops.rbegin()->first > 0.5 )
                                aGradientStops.erase( aGradientStops.rbegin()->first );
                        }
                    }

                    SAL_INFO("oox.drawingml.gradient", "symmetric: " << (bSymmetric ? "YES" : "NO") <<
                             ", number of stops: " << aGradientStops.size());
                    size_t nIndex = 0;
                    for (auto const& gradientStop : aGradientStops)
                        SAL_INFO("oox.drawingml.gradient", "  " << nIndex++ << ": " <<
                                 gradientStop.first << ": " <<
                                 std::hex << sal_Int32(gradientStop.second.getColor( rGraphicHelper, nPhClr )) << std::dec <<
                                 "@" << (100 - gradientStop.second.getTransparency()) << "%");

                    // Now estimate the simple LO style gradient (only two stops, at n% and 100%, where n ==
                    // the "border") that best emulates the gradient between begin() and prior(end()).

                    // First look for the largest segment in the gradient.
                    GradientFillProperties::GradientStopMap::iterator aIt(aGradientStops.begin());
                    double nWidestWidth = -1;
                    GradientFillProperties::GradientStopMap::iterator aWidestSegmentStart;
                    ++aIt;
                    while( aIt != aGradientStops.end() )
                    {
                        if (aIt->first - std::prev(aIt)->first > nWidestWidth)
                        {
                            nWidestWidth = aIt->first - std::prev(aIt)->first;
                            aWidestSegmentStart = std::prev(aIt);
                        }
                        ++aIt;
                    }
                    assert( nWidestWidth > 0 );

                    double nBorder = 0;
                    bool bSwap(false);

                    // Do we have just two segments, and either one is of uniform colour, or three or more
                    // segments, and the widest one is the first or last one, and is it of uniform colour? If
                    // so, deduce the border from it, and drop that segment.
                    if( aGradientStops.size() == 3 &&
                        aGradientStops.begin()->second.getColor(rGraphicHelper, nPhClr) == std::next(aGradientStops.begin())->second.getColor(rGraphicHelper, nPhClr) &&
                        aGradientStops.begin()->second.getTransparency() == std::next(aGradientStops.begin())->second.getTransparency())
                    {
                        // Two segments, first is uniformly coloured
                        SAL_INFO("oox.drawingml.gradient", "two segments, first is uniformly coloured");
                        nBorder = std::next(aGradientStops.begin())->first - aGradientStops.begin()->first;
                        aGradientStops.erase(aGradientStops.begin());
                        aWidestSegmentStart = aGradientStops.begin();
                    }
                    else if( !bSymmetric &&
                             aGradientStops.size() == 3 &&
                             std::next(aGradientStops.begin())->second.getColor(rGraphicHelper, nPhClr) == std::prev(aGradientStops.end())->second.getColor(rGraphicHelper, nPhClr) &&
                             std::next(aGradientStops.begin())->second.getTransparency() == std::prev(aGradientStops.end())->second.getTransparency())
                    {
                        // Two segments, second is uniformly coloured
                        SAL_INFO("oox.drawingml.gradient", "two segments, second is uniformly coloured");
                        auto aNext = std::next(aGradientStops.begin());
                        auto aPrev = std::prev(aGradientStops.end());
                        assert(aPrev != aGradientStops.end());
                        nBorder = aPrev->first - aNext->first;
                        aGradientStops.erase(aNext);
                        aWidestSegmentStart = aGradientStops.begin();
                        bSwap = true;
                        nShapeRotation = 180*60000 - nShapeRotation;
                    }
                    else if( !bSymmetric &&
                             aGradientStops.size() >= 4 &&
                             aWidestSegmentStart->second.getColor( rGraphicHelper, nPhClr ) == std::next(aWidestSegmentStart)->second.getColor(rGraphicHelper, nPhClr) &&
                             aWidestSegmentStart->second.getTransparency() == std::next(aWidestSegmentStart)->second.getTransparency() &&
                             ( aWidestSegmentStart == aGradientStops.begin() ||
                               std::next(aWidestSegmentStart) == std::prev(aGradientStops.end())))
                    {
                        // Not symmetric, three or more segments, the widest is first or last and is uniformly coloured
                        SAL_INFO("oox.drawingml.gradient", "first or last segment is widest and is uniformly coloured");
                        nBorder = std::next(aWidestSegmentStart)->first - aWidestSegmentStart->first;

                        // If it's the last segment that is uniformly coloured, rotate the gradient 180
                        // degrees and swap start and end colours
                        if (std::next(aWidestSegmentStart) == std::prev(aGradientStops.end()))
                        {
                            bSwap = true;
                            nShapeRotation = 180*60000 - nShapeRotation;
                        }

                        aGradientStops.erase( aWidestSegmentStart++ );

                        // Look for which is widest now
                        aIt = std::next(aGradientStops.begin());
                        nWidestWidth = -1;
                        while( aIt != aGradientStops.end() )
                        {
                            if (aIt->first - std::prev(aIt)->first > nWidestWidth)
                            {
                                nWidestWidth = aIt->first - std::prev(aIt)->first;
                                aWidestSegmentStart = std::prev(aIt);
                            }
                            ++aIt;
                        }
                    }
                    SAL_INFO("oox.drawingml.gradient", "widest segment start: " << aWidestSegmentStart->first << ", border: " << nBorder);
                    assert( (!bSymmetric && !bSwap) || !(bSymmetric && bSwap) );

                    // Now we have a potential border and a largest segment. Use those.

                    aGradient.Style = bSymmetric ? awt::GradientStyle_AXIAL : awt::GradientStyle_LINEAR;
                    sal_Int32 nShadeAngle = maGradientProps.moShadeAngle.get( 0 );
                    // Adjust for flips
                    if ( bFlipH )
                        nShadeAngle = 180*60000 - nShadeAngle;
                    if ( bFlipV )
                        nShadeAngle = -nShadeAngle;
                    sal_Int32 nDmlAngle = nShadeAngle + nShapeRotation;
                    // convert DrawingML angle (in 1/60000 degrees) to API angle (in 1/10 degrees)
                    aGradient.Angle = static_cast< sal_Int16 >( (8100 - (nDmlAngle / (PER_DEGREE / 10))) % 3600 );
                    Color aStartColor, aEndColor;

                    // Make a note where the widest segment stops, because we will try to grow it next.
                    auto aWidestSegmentEnd = std::next(aWidestSegmentStart);

                    // Try to grow the widest segment backwards: if a previous segment has the same
                    // color, just different transparency, include it.
                    while (aWidestSegmentStart != aGradientStops.begin())
                    {
                        auto it = std::prev(aWidestSegmentStart);
                        if (it->second.getColor(rGraphicHelper, nPhClr)
                            != aWidestSegmentStart->second.getColor(rGraphicHelper, nPhClr))
                        {
                            break;
                        }

                        aWidestSegmentStart = it;
                    }

                    // Try to grow the widest segment forward: if a next segment has the same
                    // color, just different transparency, include it.
                    while (aWidestSegmentEnd != std::prev(aGradientStops.end()))
                    {
                        auto it = std::next(aWidestSegmentEnd);
                        if (it->second.getColor(rGraphicHelper, nPhClr)
                            != aWidestSegmentEnd->second.getColor(rGraphicHelper, nPhClr))
                        {
                            break;
                        }

                        aWidestSegmentEnd = it;
                    }

                    assert(aWidestSegmentEnd != aGradientStops.end());

                    if( bSymmetric )
                    {
                        aStartColor = aWidestSegmentEnd->second;
                        aEndColor = aWidestSegmentStart->second;
                        nBorder *= 2;
                    }
                    else if( bSwap )
                    {
                        aStartColor = aWidestSegmentEnd->second;
                        aEndColor = aWidestSegmentStart->second;
                    }
                    else
                    {
                        aStartColor = aWidestSegmentStart->second;
                        aEndColor = aWidestSegmentEnd->second;
                    }

                    SAL_INFO("oox.drawingml.gradient", "start color: " << std::hex << sal_Int32(aStartColor.getColor( rGraphicHelper, nPhClr )) << std::dec <<
                             "@" << (100-aStartColor.getTransparency()) << "%"
                             ", end color: " << std::hex << sal_Int32(aEndColor.getColor( rGraphicHelper, nPhClr )) << std::dec <<
                             "@" << (100-aEndColor.getTransparency()) << "%");

                    aGradient.StartColor = sal_Int32(aStartColor.getColor( rGraphicHelper, nPhClr ));
                    aGradient.EndColor = sal_Int32(aEndColor.getColor( rGraphicHelper, nPhClr ));

                    nStartTrans = aStartColor.hasTransparency() ? aStartColor.getTransparency()*255/100 : 0;
                    nEndTrans = aEndColor.hasTransparency() ? aEndColor.getTransparency()*255/100 : 0;

                    aGradient.Border = rtl::math::round(100*nBorder);
                }

                // push gradient or named gradient to property map
                if( rPropMap.setProperty( ShapeProperty::FillGradient, aGradient ) )
                    eFillStyle = FillStyle_GRADIENT;

                // push gradient transparency to property map
                if( nStartTrans != 0 || nEndTrans != 0 )
                {
                    awt::Gradient aGrad(aGradient);
                    uno::Any aVal;
                    aGrad.EndColor = static_cast<sal_Int32>( nEndTrans | nEndTrans << 8 | nEndTrans << 16 );
                    aGrad.StartColor = static_cast<sal_Int32>( nStartTrans | nStartTrans << 8 | nStartTrans << 16 );
                    aVal <<= aGrad;
                    rPropMap.setProperty( ShapeProperty::GradientTransparency, aGrad );
                }

            }
        break;

        case XML_blipFill:
            // do not start complex graphic transformation if property is not supported...
            if (maBlipProps.mxFillGraphic.is() && rPropMap.supportsProperty(ShapeProperty::FillBitmap))
            {
                uno::Reference<graphic::XGraphic> xGraphic = lclCheckAndApplyDuotoneTransform(maBlipProps, maBlipProps.mxFillGraphic, rGraphicHelper, nPhClr);
                // TODO: "rotate with shape" is not possible with our current core

                if (xGraphic.is())
                {
                    if (rPropMap.supportsProperty(ShapeProperty::FillBitmapName) &&
                        rPropMap.setProperty(ShapeProperty::FillBitmapName, xGraphic))
                    {
                        eFillStyle = FillStyle_BITMAP;
                    }
                    else if (rPropMap.setProperty(ShapeProperty::FillBitmap, xGraphic))
                    {
                        eFillStyle = FillStyle_BITMAP;
                    }
                }

                // set other bitmap properties, if bitmap has been inserted into the map
                if( eFillStyle == FillStyle_BITMAP )
                {
                    // bitmap mode (single, repeat, stretch)
                    BitmapMode eBitmapMode = lclGetBitmapMode( maBlipProps.moBitmapMode.get( XML_TOKEN_INVALID ) );
                    rPropMap.setProperty( ShapeProperty::FillBitmapMode, eBitmapMode );

                    // additional settings for repeated bitmap
                    if( eBitmapMode == BitmapMode_REPEAT )
                    {
                        // anchor position inside bitmap
                        RectanglePoint eRectPoint = lclGetRectanglePoint( maBlipProps.moTileAlign.get( XML_tl ) );
                        rPropMap.setProperty( ShapeProperty::FillBitmapRectanglePoint, eRectPoint );

                        awt::Size aOriginalSize = lclGetOriginalSize(rGraphicHelper, maBlipProps.mxFillGraphic);
                        if( (aOriginalSize.Width > 0) && (aOriginalSize.Height > 0) )
                        {
                            // size of one bitmap tile (given as 1/1000 percent of bitmap size), convert to 1/100 mm
                            double fScaleX = maBlipProps.moTileScaleX.get( MAX_PERCENT ) / static_cast< double >( MAX_PERCENT );
                            sal_Int32 nFillBmpSizeX = getLimitedValue< sal_Int32, double >( aOriginalSize.Width * fScaleX, 1, SAL_MAX_INT32 );
                            rPropMap.setProperty( ShapeProperty::FillBitmapSizeX, nFillBmpSizeX );
                            double fScaleY = maBlipProps.moTileScaleY.get( MAX_PERCENT ) / static_cast< double >( MAX_PERCENT );
                            sal_Int32 nFillBmpSizeY = getLimitedValue< sal_Int32, double >( aOriginalSize.Height * fScaleY, 1, SAL_MAX_INT32 );
                            rPropMap.setProperty( ShapeProperty::FillBitmapSizeY, nFillBmpSizeY );

                            // offset of the first bitmap tile (given as EMUs), convert to percent
                            sal_Int16 nTileOffsetX = getDoubleIntervalValue< sal_Int16 >( maBlipProps.moTileOffsetX.get( 0 ) / 3.6 / aOriginalSize.Width, 0, 100 );
                            rPropMap.setProperty( ShapeProperty::FillBitmapOffsetX, nTileOffsetX );
                            sal_Int16 nTileOffsetY = getDoubleIntervalValue< sal_Int16 >( maBlipProps.moTileOffsetY.get( 0 ) / 3.6 / aOriginalSize.Height, 0, 100 );
                            rPropMap.setProperty( ShapeProperty::FillBitmapOffsetY, nTileOffsetY );
                        }
                    }
                    else if ( eBitmapMode == BitmapMode_STRETCH && maBlipProps.moFillRect.has() )
                    {
                        geometry::IntegerRectangle2D aFillRect( maBlipProps.moFillRect.get() );
                        awt::Size aOriginalSize( rGraphicHelper.getOriginalSize( xGraphic ) );
                        if ( aOriginalSize.Width && aOriginalSize.Height )
                        {
                            text::GraphicCrop aGraphCrop( 0, 0, 0, 0 );
                            if ( aFillRect.X1 )
                                aGraphCrop.Left = static_cast< sal_Int32 >( ( static_cast< double >( aOriginalSize.Width ) * aFillRect.X1 ) / 100000 );
                            if ( aFillRect.Y1 )
                                aGraphCrop.Top = static_cast< sal_Int32 >( ( static_cast< double >( aOriginalSize.Height ) * aFillRect.Y1 ) / 100000 );
                            if ( aFillRect.X2 )
                                aGraphCrop.Right = static_cast< sal_Int32 >( ( static_cast< double >( aOriginalSize.Width ) * aFillRect.X2 ) / 100000 );
                            if ( aFillRect.Y2 )
                                aGraphCrop.Bottom = static_cast< sal_Int32 >( ( static_cast< double >( aOriginalSize.Height ) * aFillRect.Y2 ) / 100000 );
                            rPropMap.setProperty(PROP_GraphicCrop, aGraphCrop);

                            bool bHasCropValues = aGraphCrop.Left != 0 || aGraphCrop.Right !=0 || aGraphCrop.Top != 0 || aGraphCrop.Bottom != 0;
                            // Negative GraphicCrop values means "crop" here.
                            bool bNeedCrop = aGraphCrop.Left <= 0 && aGraphCrop.Right <= 0 && aGraphCrop.Top <= 0 && aGraphCrop.Bottom <= 0;

                            if(bIsCustomShape && bHasCropValues && bNeedCrop)
                            {
                                xGraphic = lclCropGraphic(xGraphic, aFillRect);
                                if (rPropMap.supportsProperty(ShapeProperty::FillBitmapName))
                                    rPropMap.setProperty(ShapeProperty::FillBitmapName, xGraphic);
                                else
                                    rPropMap.setProperty(ShapeProperty::FillBitmap, xGraphic);
                            }
                        }
                    }
                }

                if (maBlipProps.moAlphaModFix.has())
                    rPropMap.setProperty(ShapeProperty::FillTransparency, static_cast<sal_Int16>(100 - (maBlipProps.moAlphaModFix.get() / PER_PERCENT)));
            }
        break;

        case XML_pattFill:
        {
            if( rPropMap.supportsProperty( ShapeProperty::FillHatch ) )
            {
                Color aColor( maPatternProps.maPattFgColor );
                if( aColor.isUsed() && maPatternProps.moPattPreset.has() )
                {
                    eFillStyle = FillStyle_HATCH;
                    rPropMap.setProperty( ShapeProperty::FillHatch, createHatch( maPatternProps.moPattPreset.get(), aColor.getColor( rGraphicHelper, nPhClr ) ) );

                    // Set background color for hatch
                    if(maPatternProps.maPattBgColor.isUsed())
                    {
                        aColor = maPatternProps.maPattBgColor;
                        rPropMap.setProperty( ShapeProperty::FillBackground, aColor.getTransparency() != 100 );
                        rPropMap.setProperty( ShapeProperty::FillColor, aColor.getColor( rGraphicHelper, nPhClr ) );
                    }
                }
                else if ( maPatternProps.maPattBgColor.isUsed() )
                {
                    aColor = maPatternProps.maPattBgColor;
                    rPropMap.setProperty( ShapeProperty::FillColor, aColor.getColor( rGraphicHelper, nPhClr ) );
                    if( aColor.hasTransparency() )
                        rPropMap.setProperty( ShapeProperty::FillTransparency, aColor.getTransparency() );
                    eFillStyle = FillStyle_SOLID;
                }
            }
        }
        break;

        case XML_grpFill:
            // todo
            eFillStyle = FillStyle_NONE;
        break;
    }

    // set final fill style property
    rPropMap.setProperty( ShapeProperty::FillStyle, eFillStyle );
}

void GraphicProperties::pushToPropMap( PropertyMap& rPropMap, const GraphicHelper& rGraphicHelper, bool bFlipH, bool bFlipV) const
{
    sal_Int16 nBrightness = getLimitedValue< sal_Int16, sal_Int32 >( maBlipProps.moBrightness.get( 0 ) / PER_PERCENT, -100, 100 );
    sal_Int16 nContrast = getLimitedValue< sal_Int16, sal_Int32 >( maBlipProps.moContrast.get( 0 ) / PER_PERCENT, -100, 100 );
    ColorMode eColorMode = ColorMode_STANDARD;

    switch( maBlipProps.moColorEffect.get( XML_TOKEN_INVALID ) )
    {
        case XML_biLevel:   eColorMode = ColorMode_MONO;    break;
        case XML_grayscl:   eColorMode = ColorMode_GREYS;   break;
    }

    if (maBlipProps.mxFillGraphic.is())
    {
        // created transformed graphic
        uno::Reference<graphic::XGraphic> xGraphic = lclCheckAndApplyChangeColorTransform(maBlipProps, maBlipProps.mxFillGraphic, rGraphicHelper, API_RGB_TRANSPARENT);
        xGraphic = lclCheckAndApplyDuotoneTransform(maBlipProps, xGraphic, rGraphicHelper, API_RGB_TRANSPARENT);

        if (eColorMode == ColorMode_STANDARD && nBrightness == 70 && nContrast == -70)
        {
            // map MSO 'washout' to our Watermark colormode
            eColorMode = ColorMode_WATERMARK;
            nBrightness = 0;
            nContrast = 0;
        }
        else if( nBrightness != 0 && nContrast != 0 )
        {
            // MSO uses a different algorithm for contrast+brightness, LO applies contrast before brightness,
            // while MSO apparently applies half of brightness before contrast and half after. So if only
            // contrast or brightness need to be altered, the result is the same, but if both are involved,
            // there's no way to map that, so just force a conversion of the image.
            xGraphic = applyBrightnessContrast( xGraphic, nBrightness, nContrast );
            nBrightness = 0;
            nContrast = 0;
        }

        // cropping
        if ( maBlipProps.moClipRect.has() )
        {
            geometry::IntegerRectangle2D oClipRect( maBlipProps.moClipRect.get() );
            awt::Size aOriginalSize( rGraphicHelper.getOriginalSize( xGraphic ) );
            if ( aOriginalSize.Width && aOriginalSize.Height )
            {
                text::GraphicCrop aGraphCrop( 0, 0, 0, 0 );
                if ( oClipRect.X1 )
                    aGraphCrop.Left = rtl::math::round( ( static_cast< double >( aOriginalSize.Width ) * oClipRect.X1 ) / 100000 );
                if ( oClipRect.Y1 )
                    aGraphCrop.Top = rtl::math::round( ( static_cast< double >( aOriginalSize.Height ) * oClipRect.Y1 ) / 100000 );
                if ( oClipRect.X2 )
                    aGraphCrop.Right = rtl::math::round( ( static_cast< double >( aOriginalSize.Width ) * oClipRect.X2 ) / 100000 );
                if ( oClipRect.Y2 )
                    aGraphCrop.Bottom = rtl::math::round( ( static_cast< double >( aOriginalSize.Height ) * oClipRect.Y2 ) / 100000 );
                rPropMap.setProperty(PROP_GraphicCrop, aGraphCrop);

                bool bHasCropValues = aGraphCrop.Left != 0 || aGraphCrop.Right !=0 || aGraphCrop.Top != 0 || aGraphCrop.Bottom != 0;
                // Positive GraphicCrop values means "crop" here.
                bool bNeedCrop = aGraphCrop.Left >= 0 && aGraphCrop.Right >= 0 && aGraphCrop.Top >= 0 && aGraphCrop.Bottom >= 0;

                if(mbIsCustomShape && bHasCropValues && bNeedCrop)
                {
                    geometry::IntegerRectangle2D aCropRect = oClipRect;
                    lclCalculateCropPercentage(xGraphic, aCropRect);
                    xGraphic = lclCropGraphic(xGraphic, aCropRect);
                }
            }
        }

        if(mbIsCustomShape)
        {
            // it is a cropped graphic.
            rPropMap.setProperty(PROP_FillStyle, FillStyle_BITMAP);
            rPropMap.setProperty(PROP_FillBitmapMode, BitmapMode_STRETCH);

            // It is a bitmap filled and rotated graphic.
            // When custom shape is rotated, bitmap have to be rotated too.
            if(rPropMap.hasProperty(PROP_RotateAngle))
            {
                tools::Long nAngle = rPropMap.getProperty(PROP_RotateAngle).get<tools::Long>();
                xGraphic = lclRotateGraphic(xGraphic, Degree10(nAngle/10) );
            }

            // We have not core feature that flips graphic in the shape.
            // Here we are applying flip property to bitmap directly.
            if(bFlipH || bFlipV)
                xGraphic = lclMirrorGraphic(xGraphic, bFlipH, bFlipV );

            if(eColorMode == ColorMode_GREYS)
                xGraphic = lclGreysScaleGraphic( xGraphic );

            rPropMap.setProperty(PROP_FillBitmap, xGraphic);
        }
        else
            rPropMap.setProperty(PROP_Graphic, xGraphic);


        if ( maBlipProps.moAlphaModFix.has() )
        {
            rPropMap.setProperty(PROP_Transparency, static_cast<sal_Int16>(100 - (maBlipProps.moAlphaModFix.get() / PER_PERCENT)));
        }
    }
    rPropMap.setProperty(PROP_GraphicColorMode, eColorMode);

    // brightness and contrast
    if( nBrightness != 0 )
        rPropMap.setProperty(PROP_AdjustLuminance, nBrightness);
    if( nContrast != 0 )
        rPropMap.setProperty(PROP_AdjustContrast, nContrast);

    // Media content
    assert(m_xMediaStream.is() != m_sMediaPackageURL.isEmpty());
    if (m_xMediaStream.is() && !m_sMediaPackageURL.isEmpty())
    {
        rPropMap.setProperty(PROP_PrivateStream, m_xMediaStream);
        rPropMap.setProperty(PROP_MediaURL, m_sMediaPackageURL);
    }
}

bool ArtisticEffectProperties::isEmpty() const
{
    return msName.isEmpty();
}

css::beans::PropertyValue ArtisticEffectProperties::getEffect()
{
    css::beans::PropertyValue aRet;
    if( msName.isEmpty() )
        return aRet;

    css::uno::Sequence< css::beans::PropertyValue > aSeq( maAttribs.size() + 1 );
    auto pSeq = aSeq.getArray();
    sal_uInt32 i = 0;
    for (auto const& attrib : maAttribs)
    {
        pSeq[i].Name = attrib.first;
        pSeq[i].Value = attrib.second;
        i++;
    }

    if( mrOleObjectInfo.maEmbeddedData.hasElements() )
    {
        css::uno::Sequence< css::beans::PropertyValue > aGraphicSeq{
            comphelper::makePropertyValue("Id", mrOleObjectInfo.maProgId),
            comphelper::makePropertyValue("Data", mrOleObjectInfo.maEmbeddedData)
        };

        pSeq[i].Name = "OriginalGraphic";
        pSeq[i].Value <<= aGraphicSeq;
    }

    aRet.Name = msName;
    aRet.Value <<= aSeq;

    return aRet;
}

void ArtisticEffectProperties::assignUsed( const ArtisticEffectProperties& rSourceProps )
{
    if( !rSourceProps.isEmpty() )
    {
        msName = rSourceProps.msName;
        maAttribs = rSourceProps.maAttribs;
    }
}

OUString ArtisticEffectProperties::getEffectString( sal_Int32 nToken )
{
    switch( nToken )
    {
        // effects
        case OOX_TOKEN( a14, artisticBlur ):                return "artisticBlur";
        case OOX_TOKEN( a14, artisticCement ):              return "artisticCement";
        case OOX_TOKEN( a14, artisticChalkSketch ):         return "artisticChalkSketch";
        case OOX_TOKEN( a14, artisticCrisscrossEtching ):   return "artisticCrisscrossEtching";
        case OOX_TOKEN( a14, artisticCutout ):              return "artisticCutout";
        case OOX_TOKEN( a14, artisticFilmGrain ):           return "artisticFilmGrain";
        case OOX_TOKEN( a14, artisticGlass ):               return "artisticGlass";
        case OOX_TOKEN( a14, artisticGlowDiffused ):        return "artisticGlowDiffused";
        case OOX_TOKEN( a14, artisticGlowEdges ):           return "artisticGlowEdges";
        case OOX_TOKEN( a14, artisticLightScreen ):         return "artisticLightScreen";
        case OOX_TOKEN( a14, artisticLineDrawing ):         return "artisticLineDrawing";
        case OOX_TOKEN( a14, artisticMarker ):              return "artisticMarker";
        case OOX_TOKEN( a14, artisticMosiaicBubbles ):      return "artisticMosiaicBubbles";
        case OOX_TOKEN( a14, artisticPaintStrokes ):        return "artisticPaintStrokes";
        case OOX_TOKEN( a14, artisticPaintBrush ):          return "artisticPaintBrush";
        case OOX_TOKEN( a14, artisticPastelsSmooth ):       return "artisticPastelsSmooth";
        case OOX_TOKEN( a14, artisticPencilGrayscale ):     return "artisticPencilGrayscale";
        case OOX_TOKEN( a14, artisticPencilSketch ):        return "artisticPencilSketch";
        case OOX_TOKEN( a14, artisticPhotocopy ):           return "artisticPhotocopy";
        case OOX_TOKEN( a14, artisticPlasticWrap ):         return "artisticPlasticWrap";
        case OOX_TOKEN( a14, artisticTexturizer ):          return "artisticTexturizer";
        case OOX_TOKEN( a14, artisticWatercolorSponge ):    return "artisticWatercolorSponge";
        case OOX_TOKEN( a14, brightnessContrast ):          return "brightnessContrast";
        case OOX_TOKEN( a14, colorTemperature ):            return "colorTemperature";
        case OOX_TOKEN( a14, saturation ):                  return "saturation";
        case OOX_TOKEN( a14, sharpenSoften ):               return "sharpenSoften";

        // attributes
        case XML_visible:           return "visible";
        case XML_trans:             return "trans";
        case XML_crackSpacing:      return "crackSpacing";
        case XML_pressure:          return "pressure";
        case XML_numberOfShades:    return "numberOfShades";
        case XML_grainSize:         return "grainSize";
        case XML_intensity:         return "intensity";
        case XML_smoothness:        return "smoothness";
        case XML_gridSize:          return "gridSize";
        case XML_pencilSize:        return "pencilSize";
        case XML_size:              return "size";
        case XML_brushSize:         return "brushSize";
        case XML_scaling:           return "scaling";
        case XML_detail:            return "detail";
        case XML_bright:            return "bright";
        case XML_contrast:          return "contrast";
        case XML_colorTemp:         return "colorTemp";
        case XML_sat:               return "sat";
        case XML_amount:            return "amount";
    }
    SAL_WARN( "oox.drawingml", "ArtisticEffectProperties::getEffectString: unexpected token " << nToken );
    return OUString();
}

sal_Int32 ArtisticEffectProperties::getEffectToken( const OUString& sName )
{
    // effects
    if( sName == "artisticBlur" )
        return XML_artisticBlur;
    else if( sName == "artisticCement" )
        return XML_artisticCement;
    else if( sName == "artisticChalkSketch" )
        return XML_artisticChalkSketch;
    else if( sName == "artisticCrisscrossEtching" )
        return XML_artisticCrisscrossEtching;
    else if( sName == "artisticCutout" )
        return XML_artisticCutout;
    else if( sName == "artisticFilmGrain" )
        return XML_artisticFilmGrain;
    else if( sName == "artisticGlass" )
        return XML_artisticGlass;
    else if( sName == "artisticGlowDiffused" )
        return XML_artisticGlowDiffused;
    else if( sName == "artisticGlowEdges" )
        return XML_artisticGlowEdges;
    else if( sName == "artisticLightScreen" )
        return XML_artisticLightScreen;
    else if( sName == "artisticLineDrawing" )
        return XML_artisticLineDrawing;
    else if( sName == "artisticMarker" )
        return XML_artisticMarker;
    else if( sName == "artisticMosiaicBubbles" )
        return XML_artisticMosiaicBubbles;
    else if( sName == "artisticPaintStrokes" )
        return XML_artisticPaintStrokes;
    else if( sName == "artisticPaintBrush" )
        return XML_artisticPaintBrush;
    else if( sName == "artisticPastelsSmooth" )
        return XML_artisticPastelsSmooth;
    else if( sName == "artisticPencilGrayscale" )
        return XML_artisticPencilGrayscale;
    else if( sName == "artisticPencilSketch" )
        return XML_artisticPencilSketch;
    else if( sName == "artisticPhotocopy" )
        return XML_artisticPhotocopy;
    else if( sName == "artisticPlasticWrap" )
        return XML_artisticPlasticWrap;
    else if( sName == "artisticTexturizer" )
        return XML_artisticTexturizer;
    else if( sName == "artisticWatercolorSponge" )
        return XML_artisticWatercolorSponge;
    else if( sName == "brightnessContrast" )
        return XML_brightnessContrast;
    else if( sName == "colorTemperature" )
        return XML_colorTemperature;
    else if( sName == "saturation" )
        return XML_saturation;
    else if( sName == "sharpenSoften" )
        return XML_sharpenSoften;

    // attributes
    else if( sName == "visible" )
        return XML_visible;
    else if( sName == "trans" )
        return XML_trans;
    else if( sName == "crackSpacing" )
        return XML_crackSpacing;
    else if( sName == "pressure" )
        return XML_pressure;
    else if( sName == "numberOfShades" )
        return XML_numberOfShades;
    else if( sName == "grainSize" )
        return XML_grainSize;
    else if( sName == "intensity" )
        return XML_intensity;
    else if( sName == "smoothness" )
        return XML_smoothness;
    else if( sName == "gridSize" )
        return XML_gridSize;
    else if( sName == "pencilSize" )
        return XML_pencilSize;
    else if( sName == "size" )
        return XML_size;
    else if( sName == "brushSize" )
        return XML_brushSize;
    else if( sName == "scaling" )
        return XML_scaling;
    else if( sName == "detail" )
        return XML_detail;
    else if( sName == "bright" )
        return XML_bright;
    else if( sName == "contrast" )
        return XML_contrast;
    else if( sName == "colorTemp" )
        return XML_colorTemp;
    else if( sName == "sat" )
        return XML_sat;
    else if( sName == "amount" )
        return XML_amount;

    SAL_WARN( "oox.drawingml", "ArtisticEffectProperties::getEffectToken - unexpected token name: " << sName );
    return XML_none;
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
