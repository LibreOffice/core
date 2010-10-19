/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fillproperties.hxx,v $
 * $Revision: 1.7 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
    ::rtl::OUString     maFillStyle;
    ::rtl::OUString     maFillColor;
    ::rtl::OUString     maFillTransparence;
    ::rtl::OUString     maFillGradient;
    ::rtl::OUString     maFillBitmap;
    ::rtl::OUString     maFillBitmapMode;
    ::rtl::OUString     maFillBitmapTile;
    ::rtl::OUString     maFillBitmapStretch;
    ::rtl::OUString     maFillBitmapLogicalSize;
    ::rtl::OUString     maFillBitmapSizeX;
    ::rtl::OUString     maFillBitmapSizeY;
    ::rtl::OUString     maFillBitmapOffsetX;
    ::rtl::OUString     maFillBitmapOffsetY;
    ::rtl::OUString     maFillBitmapRectanglePoint;
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
