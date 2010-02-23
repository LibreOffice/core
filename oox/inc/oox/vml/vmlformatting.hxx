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

#ifndef OOX_VML_VMLFORMATTING_HXX
#define OOX_VML_VMLFORMATTING_HXX

#include "oox/helper/helper.hxx"

namespace oox { class PropertyMap; }
namespace oox { namespace core { class FilterBase; } }

namespace oox {
namespace vml {

// ============================================================================

typedef ::std::pair< sal_Int32, sal_Int32 > Int32Pair;
typedef ::std::pair< double, double >       DoublePair;

// ============================================================================

class ConversionHelper
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
            with optional following '%' sign. If the '%' sign is missing, the
            floating point value will be returned unmodified. If the '%' sign
            is present, the value will be divided by 100.
     */
    static double       decodePercent(
                            const ::rtl::OUString& rValue,
                            double fDefValue );

    /** Converts the passed VML measure string to EMU (English Metric Units).

        @param rFilter  The core filter object needed to perform pixel
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
    static sal_Int32    decodeMeasureToEmu(
                            const ::oox::core::FilterBase& rFilter,
                            const ::rtl::OUString& rValue,
                            sal_Int32 nRefValue,
                            bool bPixelX,
                            bool bDefaultAsPixel );

    /** Converts the passed VML measure string to 1/100 mm.

        @param rFilter  See above.
        @param rValue  See above.
        @param nRefValue  See above.
        @param bPixelX  See above.
        @param bDefaultAsPixel  See above.
     */
    static sal_Int32    decodeMeasureToHmm(
                            const ::oox::core::FilterBase& rFilter,
                            const ::rtl::OUString& rValue,
                            sal_Int32 nRefValue,
                            bool bPixelX,
                            bool bDefaultAsPixel );

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
    OptValue< bool >    moStroked;              /// Shape border line on/off.
    StrokeArrowModel    maStartArrow;           /// Start line arrow style.
    StrokeArrowModel    maEndArrow;             /// End line arrow style.
    OptValue< ::rtl::OUString > moColor;        /// Solid line color.
    OptValue< double > moOpacity;               /// Solid line color opacity.
    OptValue< ::rtl::OUString > moWeight;       /// Line width.
    OptValue< ::rtl::OUString > moDashStyle;    /// Line dash (predefined or manually).
    OptValue< sal_Int32 > moLineStyle;          /// Line style (single, double, ...).
    OptValue< sal_Int32 > moEndCap;             /// Type of line end cap.
    OptValue< sal_Int32 > moJoinStyle;          /// Type of line join.

    void                assignUsed( const StrokeModel& rSource );

    /** Writes the properties to the passed property map. */
    void                pushToPropMap(
                            PropertyMap& rPropMap,
                            const ::oox::core::FilterBase& rFilter ) const;
};

// ============================================================================

/** The fill model structure contains all shape fill properties. */
struct FillModel
{
    OptValue< bool >    moFilled;               /// Shape fill on/off.
    OptValue< ::rtl::OUString > moColor;        /// Solid fill color.
    OptValue< double >  moOpacity;              /// Solid fill color opacity.
    OptValue< ::rtl::OUString > moColor2;       /// End color of gradient.
    OptValue< double >  moOpacity2;             /// End color opycity of gradient.
    OptValue< sal_Int32 > moType;               /// Fill type.
    OptValue< sal_Int32 > moAngle;              /// Gradient rotation angle.
    OptValue< double >  moFocus;                /// Linear gradient focus of second color.
    OptValue< DoublePair > moFocusPos;          /// Rectanguar gradient focus position of second color.
    OptValue< DoublePair > moFocusSize;         /// Rectanguar gradient focus size of second color.
    OptValue< bool >    moRotate;               /// True = rotate gradient/bitmap with shape.

    void                assignUsed( const FillModel& rSource );

    /** Writes the properties to the passed property map. */
    void                pushToPropMap(
                            PropertyMap& rPropMap,
                            const ::oox::core::FilterBase& rFilter ) const;
};

// ============================================================================

} // namespace vml
} // namespace oox

#endif

