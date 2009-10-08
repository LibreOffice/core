/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: color.hxx,v $
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

#ifndef OOX_DRAWINGML_COLOR_HXX
#define OOX_DRAWINGML_COLOR_HXX

#include <vector>
#include <boost/shared_ptr.hpp>
#include <sal/types.h>

namespace oox { namespace core {
    class XmlFilterBase;
} }

namespace oox {
namespace drawingml {

// ============================================================================

class Color
{
public:
                        Color();
                        ~Color();

    /** Sets the color to unused state. */
    void                setUnused();
    /** Sets an RGB value (hexadecimal RRGGBB) from the a:srgbClr element. */
    void                setSrgbClr( sal_Int32 nRgb );
    /** Sets the percentual RGB values from the a:scrgbClr element. */
    void                setScrgbClr( sal_Int32 nR, sal_Int32 nG, sal_Int32 nB );
    /** Sets the HSL values from the a:hslClr element. */
    void                setHslClr( sal_Int32 nHue, sal_Int32 nSat, sal_Int32 nLum );
    /** Sets a predefined color from the a:prstClr element. */
    void                setPrstClr( sal_Int32 nToken );
    /** Sets a scheme color from the a:schemeClr element. */
    void                setSchemeClr( sal_Int32 nToken );
    /** Sets a system color from the a:sysClr element. */
    void                setSysClr( sal_Int32 nToken, sal_Int32 nLastRgb );

    /** Inserts the passed color transformation. */
    void                addTransformation( sal_Int32 nElement, sal_Int32 nValue = -1 );
    /** Inserts Chart specific color tint (-1.0...0.0 = shade, 0.0...1.0 = tint). */
    void                addChartTintTransformation( double fTint );
    /** Inserts Excel specific color tint (-1.0...0.0 = shade, 0.0...1.0 = tint). */
    void                addExcelTintTransformation( double fTint );

    /** Overwrites this color with the passed color, if it is used. */
    inline void         assignIfUsed( const Color& rColor ) { if( rColor.isUsed() ) *this = rColor; }
    /** Removes transparence from the color. */
    void                clearTransparence();

    /** Returns true, if the color is initialized. */
    bool                isUsed() const { return meMode != COLOR_UNUSED; }
    /** Returns the final RGB color value.
        @param nPhClr  Actual color for the phClr placeholder color used in theme style lists. */
    sal_Int32           getColor( const ::oox::core::XmlFilterBase& rFilter, sal_Int32 nPhClr = -1 ) const;

    /** Returns true, if the color has a transparence set. */
    bool                hasTransparence() const;
    /** Returns the transparence of the color (0 = opaque, 100 = full transparent). */
    sal_Int16           getTransparence() const;

private:
    /** Converts the color components to RGB values. */
    void                toRgb() const;
    /** Converts the color components to CRGB values (gamma corrected percentage). */
    void                toCrgb() const;
    /** Converts the color components to HSL values. */
    void                toHsl() const;

private:
    enum ColorMode
    {
        COLOR_UNUSED,       /// Color is not used, or undefined.
        COLOR_RGB,          /// Absolute RGB (r/g/b: 0...255).
        COLOR_CRGB,         /// Relative RGB (r/g/b: 0...100000).
        COLOR_HSL,          /// HSL (hue: 0...21600000, sat/lum: 0...100000).
        COLOR_SCHEME,       /// Color from scheme.
        COLOR_PH,           /// Placeholder color in theme style lists.
        COLOR_SYSTEM,       /// Color from system palette.
        COLOR_FINAL         /// Finalized RGB color.
    };

    struct Transformation
    {
        sal_Int32           mnToken;
        sal_Int32           mnValue;

        explicit            Transformation( sal_Int32 nToken, sal_Int32 nValue ) : mnToken( nToken ), mnValue( nValue ) {}
    };
    typedef ::std::vector< Transformation > TransformVec;

    mutable ColorMode   meMode;         /// Current color mode.
    mutable TransformVec maTransforms;  /// Color transformations.
    mutable sal_Int32   mnC1;           /// Red, red%, hue, scheme token, system token, or final RGB.
    mutable sal_Int32   mnC2;           /// Green, green%, saturation, or system default RGB.
    mutable sal_Int32   mnC3;           /// Blue, blue%, or luminance.
    sal_Int32           mnAlpha;        /// Alpha value (color opacity).
};

typedef boost::shared_ptr< Color > ColorPtr;

// ============================================================================

} // namespace drawingml
} // namespace oox

#endif

