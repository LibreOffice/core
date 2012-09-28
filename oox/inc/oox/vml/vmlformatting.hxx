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

#ifndef OOX_VML_VMLFORMATTING_HXX
#define OOX_VML_VMLFORMATTING_HXX

#include "oox/helper/helper.hxx"
#include "oox/dllapi.h"
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>

#include <vector>

namespace oox {
    class GraphicHelper;
    namespace drawingml { class Color; }
    namespace drawingml { class ShapePropertyMap; }
}

namespace oox {
namespace vml {

// ============================================================================

typedef ::std::pair< sal_Int32, sal_Int32 > Int32Pair;
typedef ::std::pair< double, double >       DoublePair;

// ============================================================================

class OOX_DLLPUBLIC ConversionHelper
{
public:
    /** Returns two values contained in rValue separated by cSep.
     */
    static bool         separatePair(
                            ::rtl::OUString& orValue1, ::rtl::OUString& orValue2,
                            const ::rtl::OUString& rValue, sal_Unicode cSep );

    /** Returns the boolean value from the passed string of a VML attribute.
        Supported values: 'f', 't', 'false', 'true'. False for anything else.
     */
    static bool         decodeBool( const ::rtl::OUString& rValue );

    /** Converts the passed VML percentage measure string to a normalized
        floating-point value.

        @param rValue  The VML percentage value. This is a floating-point value
            with optional following '%' or 'f' sign. If the sign is missing, the
            floating point value will be returned unmodified. If the '%' sign
            is present, the value will be divided by 100. If the 'f' sign is present,
            the value will be divided by 65536.
     */
    static double       decodePercent(
                            const ::rtl::OUString& rValue,
                            double fDefValue );

    /** Converts the passed VML measure string to EMU (English Metric Units).

        @param rGraphicHelper  The graphic helper needed to perform pixel
            conversion according to the current output device.

        @param rValue  The VML measure value. This is a floating-point value
            with optional measure string following the value.

        @param nRefValue  Reference value needed for percentage measure.

        @param bPixelX  Set to true if the value is oriented horizontally (e.g.
            X coordinates, widths). Set to false if the value is oriented
            vertically (e.g. Y coordinates, heights). This is needed because
            output devices may specify different width and height for a pixel.

        @param bDefaultAsPixel  Set to true if omitted measure unit means
            pixel. Set to false if omitted measure unit means EMU.
     */
    static sal_Int64    decodeMeasureToEmu(
                            const GraphicHelper& rGraphicHelper,
                            const ::rtl::OUString& rValue,
                            sal_Int32 nRefValue,
                            bool bPixelX,
                            bool bDefaultAsPixel );

    /** Converts the passed VML measure string to 1/100 mm.

        @param rGraphicHelper  See above.
        @param rValue  See above.
        @param nRefValue  See above.
        @param bPixelX  See above.
        @param bDefaultAsPixel  See above.
     */
    static sal_Int32    decodeMeasureToHmm(
                            const GraphicHelper& rGraphicHelper,
                            const ::rtl::OUString& rValue,
                            sal_Int32 nRefValue,
                            bool bPixelX,
                            bool bDefaultAsPixel );

    /** Converts VML color attributes to a DrawingML color.

        @param roVmlColor  The VML string representation of the color. If
            existing, this can be a 3-digit or 6-digit hexadecimal RGB value
            with leading '#' character, a predefined color name (e.g. 'black',
            'red', etc.), the index into an application defined color palette
            in brackets with leading color name (e.g. 'red [9]' or
            'windowColor [64]'), or a color modifier used in one-color
            gradients (e.g. 'fill darken(128)' or 'fill lighten(0)').

        @param roVmlOpacity  The opacity of the color. If existing, this should
            be a floating-point value in the range [0.0;1.0].

        @param nDefaultRgb  Deafult RGB color used if the parameter roVmlColor
            is empty.

        @param nPrimaryRgb  If set to something else than API_RGB_TRANSPARENT,
            specifies the color to be used to resolve the color modifiers used
            in one-color gradients.

        @return  The resulting DrawingML color.
     */
    static ::oox::drawingml::Color decodeColor(
                            const GraphicHelper& rGraphicHelper,
                            const OptValue< ::rtl::OUString >& roVmlColor,
                            const OptValue< double >& roVmlOpacity,
                            sal_Int32 nDefaultRgb,
                            sal_Int32 nPrimaryRgb = API_RGB_TRANSPARENT );

    /** Converts VML path string into point and flag vectors.

        @param rPoints  The point vector to fill with coordinates.

        @param rFlags  The flag vector to fill. PolygonFlags_NORMAL indicates
            a corresponding plain shape coordinate in rPoints and
            PolygonFlags_CONTROL indicates a bezier curve control point.

        @param rPath  The VML path string.

        @param rGraphicHelper  See above.
    */
    static void         decodeVmlPath(
                            ::std::vector< ::std::vector< ::com::sun::star::awt::Point > >& rPoints,
                            ::std::vector< ::std::vector< ::com::sun::star::drawing::PolygonFlags > >& rFlags,
                            const OUString& rPath );

private:
                        ConversionHelper();
                        ~ConversionHelper();
};

// ============================================================================

/** The stroke arrow model structure contains all properties for an line end arrow. */
struct StrokeArrowModel
{
    OptValue< sal_Int32 > moArrowType;
    OptValue< sal_Int32 > moArrowWidth;
    OptValue< sal_Int32 > moArrowLength;

    void                assignUsed( const StrokeArrowModel& rSource );
};

// ============================================================================

/** The stroke model structure contains all shape border properties. */
struct StrokeModel
{
    OptValue< bool >    moStroked;              ///< Shape border line on/off.
    StrokeArrowModel    maStartArrow;           ///< Start line arrow style.
    StrokeArrowModel    maEndArrow;             ///< End line arrow style.
    OptValue< ::rtl::OUString > moColor;        ///< Solid line color.
    OptValue< double > moOpacity;               ///< Solid line color opacity.
    OptValue< ::rtl::OUString > moWeight;       ///< Line width.
    OptValue< ::rtl::OUString > moDashStyle;    ///< Line dash (predefined or manually).
    OptValue< sal_Int32 > moLineStyle;          ///< Line style (single, double, ...).
    OptValue< sal_Int32 > moEndCap;             ///< Type of line end cap.
    OptValue< sal_Int32 > moJoinStyle;          ///< Type of line join.

    void                assignUsed( const StrokeModel& rSource );

    /** Writes the properties to the passed property map. */
    void                pushToPropMap(
                            ::oox::drawingml::ShapePropertyMap& rPropMap,
                            const GraphicHelper& rGraphicHelper ) const;
};

// ============================================================================

/** The fill model structure contains all shape fill properties. */
struct FillModel
{
    OptValue< bool >    moFilled;               ///< Shape fill on/off.
    OptValue< ::rtl::OUString > moColor;        ///< Solid fill color.
    OptValue< double >  moOpacity;              ///< Solid fill color opacity.
    OptValue< ::rtl::OUString > moColor2;       ///< End color of gradient.
    OptValue< double >  moOpacity2;             ///< End color opacity of gradient.
    OptValue< sal_Int32 > moType;               ///< Fill type.
    OptValue< sal_Int32 > moAngle;              ///< Gradient rotation angle.
    OptValue< double >  moFocus;                ///< Linear gradient focus of second color.
    OptValue< DoublePair > moFocusPos;          ///< Rectangular gradient focus position of second color.
    OptValue< DoublePair > moFocusSize;         ///< Rectangular gradient focus size of second color.
    OptValue< ::rtl::OUString > moBitmapPath;   ///< Path to fill bitmap fragment.
    OptValue< bool >    moRotate;               ///< True = rotate gradient/bitmap with shape.

    void                assignUsed( const FillModel& rSource );

    /** Writes the properties to the passed property map. */
    void                pushToPropMap(
                            ::oox::drawingml::ShapePropertyMap& rPropMap,
                            const GraphicHelper& rGraphicHelper ) const;
};

// ============================================================================

} // namespace vml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
