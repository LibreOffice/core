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

#ifndef INCLUDED_OOX_INC_DRAWINGML_FILLPROPERTIES_HXX
#define INCLUDED_OOX_INC_DRAWINGML_FILLPROPERTIES_HXX

#include <map>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/geometry/IntegerRectangle2D.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <oox/drawingml/color.hxx>
#include <oox/helper/helper.hxx>
#include <oox/ole/oleobjecthelper.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace com::sun::star {
    namespace graphic { class XGraphic; }
}

namespace oox {
    class GraphicHelper;
}

namespace oox::drawingml {

class ShapePropertyMap;

struct GradientFillProperties
{
    typedef ::std::multimap< double, Color > GradientStopMap;

    GradientStopMap     maGradientStops;        /// Gradient stops (colors/transparence).
    OptValue< css::geometry::IntegerRectangle2D > moFillToRect;
    OptValue< css::geometry::IntegerRectangle2D > moTileRect;
    OptValue< sal_Int32 > moGradientPath;       /// If set, gradient follows rectangle, circle, or shape.
    OptValue< sal_Int32 > moShadeAngle;         /// Rotation angle of linear gradients.
    OptValue< sal_Int32 > moShadeFlip;          /// Flip mode of gradient, if not stretched to shape.
    OptValue< bool >    moShadeScaled;          /// True = scale gradient into shape.
    OptValue< bool >    moRotateWithShape;      /// True = rotate gradient with shape.

    /** Overwrites all members that are explicitly set in rSourceProps. */
    void                assignUsed( const GradientFillProperties& rSourceProps );
};

struct PatternFillProperties
{
    Color               maPattFgColor;          /// Pattern foreground color.
    Color               maPattBgColor;          /// Pattern background color.
    OptValue< sal_Int32 > moPattPreset;         /// Preset pattern type.

    /** Overwrites all members that are explicitly set in rSourceProps. */
    void                assignUsed( const PatternFillProperties& rSourceProps );
};

struct ArtisticEffectProperties
{
    OUString            msName;
    std::map< OUString, css::uno::Any >
                        maAttribs;
    ::oox::ole::OleObjectInfo mrOleObjectInfo;  /// The original graphic as embedded object.

    bool                isEmpty() const;

    /** Returns the struct as a PropertyValue with Name = msName and
     *  Value = maAttribs as a Sequence< PropertyValue >. */
    css::beans::PropertyValue getEffect();

    /** Overwrites all members that are explicitly set in rSourceProps. */
    void                assignUsed( const ArtisticEffectProperties& rSourceProps );

    /** Translate effect tokens to strings. */
    static OUString     getEffectString( sal_Int32 nToken );

    /** Translate effect strings to tokens. */
    static sal_Int32    getEffectToken( const OUString& sName );
};

struct BlipFillProperties
{
    css::uno::Reference<css::graphic::XGraphic> mxFillGraphic; /// The fill graphic.
    OptValue< sal_Int32 > moBitmapMode;         /// Bitmap tile or stretch.
    OptValue< css::geometry::IntegerRectangle2D >
                          moFillRect;             /// Stretch fill offsets.
    OptValue< css::geometry::IntegerRectangle2D >
                          moClipRect;
    OptValue< sal_Int32 > moTileOffsetX;        /// Width of bitmap tiles (EMUs).
    OptValue< sal_Int32 > moTileOffsetY;        /// Height of bitmap tiles (EMUs).
    OptValue< sal_Int32 > moTileScaleX;         /// Horizontal scaling of bitmap tiles (1/1000 percent).
    OptValue< sal_Int32 > moTileScaleY;         /// Vertical scaling of bitmap tiles (1/1000 percent).
    OptValue< sal_Int32 > moTileAlign;          /// Anchor point inside bitmap.
    OptValue< sal_Int32 > moTileFlip;           /// Flip mode of bitmap tiles.
    OptValue< bool >      moRotateWithShape;      /// True = rotate bitmap with shape.
    // effects
    OptValue< sal_Int32 > moColorEffect;        /// XML token for a color effect.
    OptValue< sal_Int32 > moBrightness;         /// Brightness in the range [-100000,100000].
    OptValue< sal_Int32 > moContrast;           /// Contrast in the range [-100000,100000].
    Color                 maColorChangeFrom;      /// Start color of color transformation.
    Color                 maColorChangeTo;        /// Destination color of color transformation.
    Color                 maDuotoneColors[2];     /// Duotone Colors

    ArtisticEffectProperties maEffect;          /// Artistic effect, not supported by core.
    OptValue<sal_Int32> moAlphaModFix; ///< Alpha Modulate Fixed Effect.

    /** Overwrites all members that are explicitly set in rSourceProps. */
    void                assignUsed( const BlipFillProperties& rSourceProps );
};

struct FillProperties
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
                            ::Color nPhClr = API_RGB_TRANSPARENT,
                            bool bFlipH = false,
                            bool bFlipV = false,
                            bool bIsCustomShape = false ) const;
};

} // namespace oox::drawingml

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
