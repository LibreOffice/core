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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_RGBCOLOR_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_RGBCOLOR_HXX

#include <cppcanvas/color.hxx>


/* Definition of RGBColor class */

namespace slideshow
{
    namespace internal
    {
        class HSLColor;

        /** RGB color space class.
         */
        class RGBColor
        {
        public:
            RGBColor();
            explicit RGBColor( ::cppcanvas::Color::IntSRGBA nRGBColor );
            RGBColor( double nRed, double nGreen, double nBlue );
            explicit RGBColor( const HSLColor& rColor );

            /** Get the RGB red value.
             */
            double getRed() const {  return maRGBTriple.mnRed; }

            /** Get the RGB green value.
             */
            double getGreen() const {  return maRGBTriple.mnGreen; }

            /** Get the RGB blue value.
             */
            double getBlue() const {  return maRGBTriple.mnBlue; }

            /** Create an integer sRGBA color.
             */
            ::cppcanvas::Color::IntSRGBA getIntegerColor() const;

            struct RGBTriple
            {
                RGBTriple();
                RGBTriple( double nRed, double nGreen, double nBlue );

                double mnRed;
                double mnGreen;
                double mnBlue;
            };

        private:
            // default copy/assignment are okay
            // RGBColor(const RGBColor&);
            // RGBColor& operator=( const RGBColor& );

            RGBTriple   maRGBTriple;
        };

        bool operator==( const RGBColor& rLHS, const RGBColor& rRHS );
        bool operator!=( const RGBColor& rLHS, const RGBColor& rRHS );
        RGBColor operator+( const RGBColor& rLHS, const RGBColor& rRHS );
        RGBColor operator*( const RGBColor& rLHS, const RGBColor& rRHS );
        RGBColor operator*( double nFactor, const RGBColor& rRHS );


        /** RGB color linear interpolator.

            @param t
            As usual, t must be in the [0,1] range
        */
        RGBColor interpolate( const RGBColor& rFrom, const RGBColor& rTo, double t );
    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_RGBCOLOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
