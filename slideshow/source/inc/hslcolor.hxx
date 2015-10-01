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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_HSLCOLOR_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_HSLCOLOR_HXX

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
            HSLColor( double nHue, double nSaturation, double nLuminance );
            explicit HSLColor( const RGBColor& rColor );

            /** Hue of the color.

                @return hue, is in the range [0,360]
             */
            double getHue() const {  return maHSLTriple.mnHue; }

            /** Saturation of the color.

                @return saturation, is in the range [0,1]
             */
            double getSaturation() const {  return maHSLTriple.mnSaturation; }

            /** Luminance of the color.

                @return luminance, is in the range [0,1]
             */
            double getLuminance() const {  return maHSLTriple.mnLuminance; }

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

        bool operator==( const HSLColor& rLHS, const HSLColor& rRHS );
        bool operator!=( const HSLColor& rLHS, const HSLColor& rRHS );
        HSLColor operator+( const HSLColor& rLHS, const HSLColor& rRHS );
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

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_HSLCOLOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
