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

#include <basegfx/numeric/ftools.hxx>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/color/bcolortools.hxx>

namespace basegfx { namespace utils
{
    BColor rgb2hsl(const BColor& rRGBColor)
    {
        const double r=rRGBColor.getRed(), g=rRGBColor.getGreen(), b=rRGBColor.getBlue();
        const double minVal = std::min( std::min( r, g ), b );
        const double maxVal = std::max( std::max( r, g ), b );
        const double d = maxVal - minVal;

        double h=0, s=0, l=0;

        l = (maxVal + minVal) / 2.0;

        if( ::basegfx::fTools::equalZero(d) )
        {
            s = h = 0; // hue undefined (achromatic case)
        }
        else
        {
            s = l > 0.5 ? d/(2.0-maxVal-minVal) :
                d/(maxVal + minVal);

            if( rtl::math::approxEqual(r, maxVal) )
                h = (g - b)/d;
            else if( rtl::math::approxEqual(g, maxVal) )
                h = 2.0 + (b - r)/d;
            else
                h = 4.0 + (r - g)/d;

            h *= 60.0;

            if( h < 0.0 )
                h += 360.0;
        }

        return BColor(h,s,l);
    }

    static inline double hsl2rgbHelper( double nValue1, double nValue2, double nHue )
    {
        // clamp hue to [0,360]
        nHue = fmod( nHue, 360.0 );

        // cope with wrap-arounds
        if( nHue < 0.0 )
            nHue += 360.0;

        if( nHue < 60.0 )
            return nValue1 + (nValue2 - nValue1)*nHue/60.0;
        else if( nHue < 180.0 )
            return nValue2;
        else if( nHue < 240.0 )
            return nValue1 + (nValue2 - nValue1)*(240.0 - nHue)/60.0;
        else
            return nValue1;
    }

    BColor hsl2rgb(const BColor& rHSLColor)
    {
        const double h=rHSLColor.getRed(), s=rHSLColor.getGreen(), l=rHSLColor.getBlue();

        if( fTools::equalZero(s) )
            return BColor(l, l, l ); // achromatic case

        const double nVal1( l <= 0.5 ? l*(1.0 + s) : l + s - l*s );
        const double nVal2( 2.0*l - nVal1 );

        return BColor(
            hsl2rgbHelper(nVal2,
                          nVal1,
                          h + 120.0),
            hsl2rgbHelper(nVal2,
                          nVal1,
                          h),
            hsl2rgbHelper(nVal2,
                          nVal1,
                          h - 120.0) );
    }

    BColor rgb2hsv(const BColor& rRGBColor)
    {
        const double r=rRGBColor.getRed(), g=rRGBColor.getGreen(), b=rRGBColor.getBlue();
        const double maxVal = std::max(std::max(r,g),b);
        const double minVal = std::min(std::min(r,g),b);
        const double delta = maxVal-minVal;

        double h=0, s=0, v=0;

        v = maxVal;
        if( fTools::equalZero(v) )
            s = 0;
        else
            s = delta / v;

        if( !fTools::equalZero(s) )
        {
            if( rtl::math::approxEqual(maxVal, r) )
            {
                h = (g - b) / delta;
            }
            else if( rtl::math::approxEqual(maxVal, g) )
            {
                h = 2.0 + (b - r) / delta;
            }
            else
            {
                h = 4.0 + (r - g) / delta;
            }

            h *= 60.0;

            if( h < 0 )
                h += 360;
        }

        return BColor(h,s,v);
    }

    BColor hsv2rgb(const BColor& rHSVColor)
    {
        double h=rHSVColor.getRed();
        const double s=rHSVColor.getGreen(), v=rHSVColor.getBlue();

        if( fTools::equalZero(s) )
        {
            // achromatic case: no hue.
            return BColor(v,v,v);
        }
        else
        {
            if( fTools::equal(h,360) )
                h = 0; // 360 degrees is equivalent to 0 degrees

            h /= 60.0;
            const sal_Int32 intval = static_cast< sal_Int32 >( h );
            const double f = h - intval;
            const double p = v*(1.0-s);
            const double q = v*(1.0-(s*f));
            const double t = v*(1.0-(s*(1.0-f)));

            /* which hue area? */
            switch( intval )
            {
                case 0:
                    return BColor(v,t,p);

                case 1:
                    return BColor(q,v,p);

                case 2:
                    return BColor(p,v,t);

                case 3:
                    return BColor(p,q,v);

                case 4:
                    return BColor(t,p,v);

                case 5:
                    return BColor(v,p,q);

                default:
                    // hue overflow
                    return BColor();
            }
        }
    }

} } // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
