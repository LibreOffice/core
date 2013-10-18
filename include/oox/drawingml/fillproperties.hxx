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

#ifndef OOX_DRAWINGML_FILLPROPERTIES_HXX
#define OOX_DRAWINGML_FILLPROPERTIES_HXX

#include <map>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/geometry/IntegerRectangle2D.hpp>
#include "oox/drawingml/color.hxx"
#include "oox/helper/helper.hxx"
#include "oox/drawingml/embeddedwavaudiofile.hxx"

namespace oox {
    class GraphicHelper;
    class PropertyMap;
    class PropertySet;
}

namespace oox {
namespace drawingml {

class ShapePropertyMap;

// ============================================================================

struct GradientFillProperties
{
    typedef ::std::map< double, Color > GradientStopMap;

    GradientStopMap     maGradientStops;        /// Gradient stops (colors/transparence).
    OptValue< ::com::sun::star::geometry::IntegerRectangle2D > moFillToRect;
    OptValue< ::com::sun::star::geometry::IntegerRectangle2D > moTileRect;
    OptValue< sal_Int32 > moGradientPath;       /// If set, gradient follows rectangle, circle, or shape.
    OptValue< sal_Int32 > moShadeAngle;         /// Rotation angle of linear gradients.
    OptValue< sal_Int32 > moShadeFlip;          /// Flip mode of gradient, if not stretched to shape.
    OptValue< bool >    moShadeScaled;          /// True = scale gradient into shape.
    OptValue< bool >    moRotateWithShape;      /// True = rotate gradient with shape.

    /** Overwrites all members that are explicitly set in rSourceProps. */
    void                assignUsed( const GradientFillProperties& rSourceProps );
};

// ============================================================================

struct PatternFillProperties
{
    Color               maPattFgColor;          /// Pattern foreground color.
    Color               maPattBgColor;          /// Pattern background color.
    OptValue< sal_Int32 > moPattPreset;         /// Preset pattern type.

    /** Overwrites all members that are explicitly set in rSourceProps. */
    void                assignUsed( const PatternFillProperties& rSourceProps );
};

// ============================================================================

struct BlipFillProperties
{
    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >
                        mxGraphic;              /// The fill graphic.
    OptValue< sal_Int32 > moBitmapMode;         /// Bitmap tile or stretch.
    OptValue< ::com::sun::star::geometry::IntegerRectangle2D >
                        moFillRect;             /// Stretch fill offsets.
    OptValue< ::com::sun::star::geometry::IntegerRectangle2D >
                        moClipRect;
    OptValue< sal_Int32 > moTileOffsetX;        /// Width of bitmap tiles (EMUs).
    OptValue< sal_Int32 > moTileOffsetY;        /// Height of bitmap tiles (EMUs).
    OptValue< sal_Int32 > moTileScaleX;         /// Horizontal scaling of bitmap tiles (1/1000 percent).
    OptValue< sal_Int32 > moTileScaleY;         /// Vertical scaling of bitmap tiles (1/1000 percent).
    OptValue< sal_Int32 > moTileAlign;          /// Anchor point inside bitmap.
    OptValue< sal_Int32 > moTileFlip;           /// Flip mode of bitmap tiles.
    OptValue< bool >    moRotateWithShape;      /// True = rotate bitmap with shape.
    // effects
    OptValue< sal_Int32 > moColorEffect;        /// XML token for a color effect.
    OptValue< sal_Int32 > moBrightness;         /// Brightness in the range [-100000,100000].
    OptValue< sal_Int32 > moContrast;           /// Contrast in the range [-100000,100000].
    Color               maColorChangeFrom;      /// Start color of color transformation.
    Color               maColorChangeTo;        /// Destination color of color transformation.
    Color               maDuotoneColors[2];     /// Duotone Colors

    /** Overwrites all members that are explicitly set in rSourceProps. */
    void                assignUsed( const BlipFillProperties& rSourceProps );
};

// ============================================================================

struct OOX_DLLPUBLIC FillProperties
{
    OptValue< sal_Int32 > moFillType;           /// Fill type (OOXML token).
    Color               maFillColor;            /// Solid fill color and transparence.
    GradientFillProperties maGradientProps;     /// Properties for gradient fills.
    PatternFillProperties maPatternProps;       /// Properties for pattern fills.
    BlipFillProperties  maBlipProps;            /// Properties for bitmap fills.

    /** Overwrites all members that are explicitly set in rSourceProps. */
    void                assignUsed( const FillProperties& rSourceProps );

    /** Tries to resolve current settings to a solid color, e.g. returns the
        start color of a gradient. */
    Color               getBestSolidColor() const;

    /** Writes the properties to the passed property map. */
    void                pushToPropMap(
                            ShapePropertyMap& rPropMap,
                            const GraphicHelper& rGraphicHelper,
                            sal_Int32 nShapeRotation = 0,
                            sal_Int32 nPhClr = API_RGB_TRANSPARENT,
                            bool bFlipH = false,
                            bool bFlipV = false ) const;
};

// ============================================================================

struct GraphicProperties
{
    BlipFillProperties      maBlipProps;            /// Properties for the graphic.
    EmbeddedWAVAudioFile    maAudio;                /// Audio file details

    /** Overwrites all members that are explicitly set in rSourceProps. */
    void                assignUsed( const GraphicProperties& rSourceProps );

    /** Writes the properties to the passed property map. */
    void                pushToPropMap(
                            PropertyMap& rPropMap,
                            const GraphicHelper& rGraphicHelper,
                            sal_Int32 nPhClr = API_RGB_TRANSPARENT ) const;
};

// ============================================================================

} // namespace drawingml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
