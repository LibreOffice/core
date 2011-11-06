/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef INCLUDED_SLIDESHOW_HSLCOLOR_HXX
#define INCLUDED_SLIDESHOW_HSLCOLOR_HXX

#include <cppcanvas/color.hxx>


/* Definition of HSLColor class */

namespace slideshow
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

#endif /* INCLUDED_SLIDESHOW_HSLCOLOR_HXX */
