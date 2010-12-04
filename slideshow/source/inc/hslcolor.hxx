/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
