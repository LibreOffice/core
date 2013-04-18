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

#ifndef OOX_DRAWINGML_SHAPE3DPROPERTIES_HXX
#define OOX_DRAWINGML_SHAPE3DPROPERTIES_HXX

#include <map>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/geometry/IntegerRectangle2D.hpp>
#include "oox/core/xmlfilterbase.hxx"
#include "oox/drawingml/color.hxx"
#include "oox/helper/helper.hxx"

namespace oox { class PropertyMap; }
namespace oox { class PropertySet; }
namespace oox { namespace core { class ModelObjectContainer; } }

namespace oox {
namespace drawingml {

// ============================================================================

struct Shape3DPropertyNames
{
    OUString     maFillStyle;
    OUString     maFillColor;
    OUString     maFillTransparence;
    OUString     maFillGradient;
    OUString     maFillBitmap;
    OUString     maFillBitmapMode;
    OUString     maFillBitmapTile;
    OUString     maFillBitmapStretch;
    OUString     maFillBitmapLogicalSize;
    OUString     maFillBitmapSizeX;
    OUString     maFillBitmapSizeY;
    OUString     maFillBitmapOffsetX;
    OUString     maFillBitmapOffsetY;
    OUString     maFillBitmapRectanglePoint;
    bool                mbNamedFillGradient;
    bool                mbNamedFillBitmap;
    bool                mbTransformGraphic;

    Shape3DPropertyNames();
};

// ============================================================================

struct Shape3DProperties
{
    typedef ::std::map< double, Color > GradientStopMap;

    OptValue< sal_Int32 > moFillType;           /// Fill type (OOXML token).
    OptValue< bool >    moRotateWithShape;      /// True = rotate gradient/bitmap with shape.
    Color               maFillColor;            /// Solid fill color and transparence.
    GradientStopMap     maGradientStops;        /// Gradient stops (colors/transparence).
    OptValue< sal_Int32 > moGradientPath;       /// If set, gradient follows rectangle, circle, or shape.
    OptValue< sal_Int32 > moShadeAngle;         /// Rotation angle of linear gradients.
    OptValue< bool >    moShadeScaled;
    OptValue< sal_Int32 > moFlipModeToken;
    OptValue< com::sun::star::geometry::IntegerRectangle2D > moFillToRect;
    OptValue< com::sun::star::geometry::IntegerRectangle2D > moTileRect;
    OptValue< sal_Int32 > moPattPreset;         /// Preset pattern type.
    Color               maPattFgColor;          /// Pattern foreground color.
    Color               maPattBgColor;          /// Pattern background color.
    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > mxGraphic;
    Color               maColorChangeFrom;      /// Start color of color transformation.
    Color               maColorChangeTo;        /// Destination color of color transformation.
    OptValue< sal_Int32 > moBitmapMode;         /// Bitmap tile or stretch.
    OptValue< sal_Int32 > moTileX;              /// Width of bitmap tiles.
    OptValue< sal_Int32 > moTileY;              /// Height of bitmap tiles.
    OptValue< sal_Int32 > moTileSX;
    OptValue< sal_Int32 > moTileSY;
    OptValue< sal_Int32 > moTileAlign;          /// Anchor point inside bitmap.

    static Shape3DPropertyNames DEFAULTNAMES;      /// Default fill property names for shape fill.
    static Shape3DPropertyNames DEFAULTPICNAMES;   /// Default fill property names for pictures.

    /** Overwrites all members that are explicitly set in rSourceProps. */
    void                assignUsed( const Shape3DProperties& rSourceProps );

    /** Tries to resolve current settings to a solid color, e.g. returns the
        start color of a gradient. */
    Color               getBestSolidColor() const;

    /** Writes the properties to the passed property map. */
    void                pushToPropMap(
                            PropertyMap& rPropMap,
                            const Shape3DPropertyNames& rPropNames,
                            const ::oox::core::XmlFilterBase& rFilter,
                            ::oox::core::ModelObjectContainer& rObjContainer,
                            sal_Int32 nShapeRotation, sal_Int32 nPhClr ) const;

    /** Writes the properties to the passed property set. */
    void                pushToPropSet(
                            PropertySet& rPropSet,
                            const Shape3DPropertyNames& rPropNames,
                            const ::oox::core::XmlFilterBase& rFilter,
                            ::oox::core::ModelObjectContainer& rObjContainer,
                            sal_Int32 nShapeRotation, sal_Int32 nPhClr ) const;
};

// ============================================================================

} // namespace drawingml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
