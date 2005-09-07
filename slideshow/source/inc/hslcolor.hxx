/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hslcolor.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:13:03 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SLIDESHOW_HSLCOLOR_HXX
#define _SLIDESHOW_HSLCOLOR_HXX

#ifndef _CPPCANVAS_COLOR_HXX
#include <cppcanvas/color.hxx>
#endif


/* Definition of HSLColor class */

namespace presentation
{
    namespace internal
    {
        class RGBColor;

        /** HSL color space class.
         */
        class HSLColor
        {
        public:
            HSLColor();
            explicit HSLColor( ::cppcanvas::Color::IntSRGBA nRGBColor );
            HSLColor( double nHue, double nSaturation, double nLuminance );
            explicit HSLColor( const RGBColor& rColor );

            /** Hue of the color.

                @return hue, is in the range [0,360]
             */
            double getHue() const;

            /** Saturation of the color.

                @return saturation, is in the range [0,1]
             */
            double getSaturation() const;

            /** Luminance of the color.

                @return luminance, is in the range [0,1]
             */
            double getLuminance() const;

            /** Get the RGB red value.
             */
            double getRed() const;

            /** Get the RGB green value.
             */
            double getGreen() const;

            /** Get the RGB blue value.
             */
            double getBlue() const;

            /** Create an RGB color object.
             */
            RGBColor getRGBColor() const;

            struct HSLTriple
            {
                HSLTriple();
                HSLTriple( double nHue, double nSaturation, double nLuminance );

                double mnHue;
                double mnSaturation;
                double mnLuminance;
            };

        private:
            // default copy/assignment are okay
            // HSLColor(const HSLColor&);
            // HSLColor& operator=( const HSLColor& );

            HSLTriple   maHSLTriple;

            /// Pre-calculated value, needed for conversion back to RGB
            double      mnMagicValue;
        };

        HSLColor operator+( const HSLColor& rLHS, const HSLColor& rRHS );
        HSLColor operator*( const HSLColor& rLHS, const HSLColor& rRHS );
        HSLColor operator*( double nFactor, const HSLColor& rRHS );

        /** HSL color linear interpolator.

            @param t
            As usual, t must be in the [0,1] range

            @param bCCW
            When true, hue interpolation happens counter-clockwise
        */
        HSLColor interpolate( const HSLColor& rFrom, const HSLColor& rTo, double t, bool bCCW=true );
    }
}

#endif /* _SLIDESHOW_HSLCOLOR_HXX */
