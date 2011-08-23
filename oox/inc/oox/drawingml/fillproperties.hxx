/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef OOX_DRAWINGML_FILLPROPERTIES_HXX
#define OOX_DRAWINGML_FILLPROPERTIES_HXX

#include <map>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/geometry/IntegerRectangle2D.hpp>
#include "oox/drawingml/color.hxx"
#include "oox/helper/helper.hxx"

namespace oox {
    class GraphicHelper;
    class ModelObjectHelper;
    class PropertyMap;
    class PropertySet;
}

namespace oox {
namespace drawingml {

// ============================================================================

enum FillPropertyId
{
    FillStyleId,
    FillColorId,
    FillTransparenceId,
    FillGradientId,
    FillBitmapUrlId,
    FillBitmapModeId,
    FillBitmapSizeXId,
    FillBitmapSizeYId,
    FillBitmapOffsetXId,
    FillBitmapOffsetYId,
    FillBitmapRectanglePointId,
    FillId_END
};

struct FillPropertyIds
{
    const sal_Int32*    mpnPropertyIds;
    bool                mbNamedFillGradient;
    bool                mbNamedFillBitmap;

    explicit            FillPropertyIds(
                            const sal_Int32* pnPropertyIds,
                            bool bNamedFillGradient,
                            bool bNamedFillBitmap );

    inline bool         has( FillPropertyId ePropId ) const { return mpnPropertyIds[ ePropId ] >= 0; }
    inline sal_Int32    operator[]( FillPropertyId ePropId ) const { return mpnPropertyIds[ ePropId ]; }
};

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

    /** Overwrites all members that are explicitly set in rSourceProps. */
    void                assignUsed( const BlipFillProperties& rSourceProps );
};

// ============================================================================

struct FillProperties
{
    OptValue< sal_Int32 > moFillType;           /// Fill type (OOXML token).
    Color               maFillColor;            /// Solid fill color and transparence.
    GradientFillProperties maGradientProps;     /// Properties for gradient fills.
    PatternFillProperties maPatternProps;       /// Properties for pattern fills.
    BlipFillProperties  maBlipProps;            /// Properties for bitmap fills.

    static FillPropertyIds DEFAULT_IDS;         /// Default fill property identifiers for shape fill.

    /** Overwrites all members that are explicitly set in rSourceProps. */
    void                assignUsed( const FillProperties& rSourceProps );

    /** Tries to resolve current settings to a solid color, e.g. returns the
        start color of a gradient. */
    Color               getBestSolidColor() const;

    /** Writes the properties to the passed property map. */
    void                pushToPropMap(
                            PropertyMap& rPropMap,
                            ModelObjectHelper& rModelObjHelper,
                            const GraphicHelper& rGraphicHelper,
                            const FillPropertyIds& rPropIds = DEFAULT_IDS,
                            sal_Int32 nShapeRotation = 0,
                            sal_Int32 nPhClr = API_RGB_TRANSPARENT ) const;

    /** Writes the properties to the passed property set. */
    void                pushToPropSet(
                            PropertySet& rPropSet,
                            ModelObjectHelper& rModelObjHelper,
                            const GraphicHelper& rGraphicHelper,
                            const FillPropertyIds& rPropIds = DEFAULT_IDS,
                            sal_Int32 nShapeRotation = 0,
                            sal_Int32 nPhClr = API_RGB_TRANSPARENT ) const;
};

// ============================================================================

struct GraphicProperties
{
    BlipFillProperties  maBlipProps;            /// Properties for the graphic.

    /** Overwrites all members that are explicitly set in rSourceProps. */
    void                assignUsed( const GraphicProperties& rSourceProps );

    /** Writes the properties to the passed property map. */
    void                pushToPropMap(
                            PropertyMap& rPropMap,
                            const GraphicHelper& rGraphicHelper,
                            sal_Int32 nPhClr = API_RGB_TRANSPARENT ) const;

    /** Writes the properties to the passed property set. */
    void                pushToPropSet(
                            PropertySet& rPropSet,
                            const GraphicHelper& rGraphicHelper,
                            sal_Int32 nPhClr = API_RGB_TRANSPARENT ) const;
};

// ============================================================================

} // namespace drawingml
} // namespace oox

#endif

