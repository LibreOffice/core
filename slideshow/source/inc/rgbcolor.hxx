/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rgbcolor.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:17:21 $
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

#ifndef _SLIDESHOW_RGBCOLOR_HXX
#define _SLIDESHOW_RGBCOLOR_HXX

#ifndef _CPPCANVAS_COLOR_HXX
#include <cppcanvas/color.hxx>
#endif


/* Definition of RGBColor class */

namespace presentation
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

            /** Create an HSL color object.
             */
            HSLColor getHSLColor() const;

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

#endif /* _SLIDESHOW_RGBCOLOR_HXX */
