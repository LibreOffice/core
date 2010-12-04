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

#ifndef INCLUDED_SLIDESHOW_RGBCOLOR_HXX
#define INCLUDED_SLIDESHOW_RGBCOLOR_HXX

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
            double getRed() const;

            /** Get the RGB green value.
             */
            double getGreen() const;

            /** Get the RGB blue value.
             */
            double getBlue() const;

            /** Create an integer sRGBA color.
             */
            ::cppcanvas::Color::IntSRGBA getIntegerColor() const;

            RGBColor(const RGBColor& rLHS);
            RGBColor& operator=( const RGBColor& rLHS);

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

#endif /* INCLUDED_SLIDESHOW_RGBCOLOR_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
