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

#ifndef INCLUDED_BASEGFX_COLOR_BCOLOR_HXX
#define INCLUDED_BASEGFX_COLOR_BCOLOR_HXX

#include <sal/config.h>

#include <algorithm>
#include <ostream>

#include <basegfx/tuple/b3dtuple.hxx>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
    /** Base Color class with three double values

        This class derives all operators and common handling for
        a 3D data class from B3DTuple. All necessary extensions
        which are special for colors will be added here.

        @see B3DTuple
    */
    class SAL_WARN_UNUSED BASEGFX_DLLPUBLIC BColor : public B3DTuple
    {
    public:
        /** Create a Color with red, green and blue components from [0.0 to 1.0]

            The color is initialized to (0.0, 0.0, 0.0)
        */
        BColor()
        :   B3DTuple()
        {}

        /** Create a 3D Color

            @param fRed
            @param fGreen
            @param fBlue
            These parameters are used to initialize the red, green and blue intensities of the color
        */
        BColor(double fRed, double fGreen, double fBlue)
        :   B3DTuple(fRed, fGreen, fBlue)
        {}

        /** Create a 3D Color

            @param fLuminosity
            The parameter is used to initialize the red, green and blue intensities of the color
        */
        explicit BColor(double fLuminosity)
        :   B3DTuple(fLuminosity, fLuminosity, fLuminosity)
        {}

        /** constructor with tuple to allow copy-constructing
            from B3DTuple-based classes
        */
        BColor(const ::basegfx::B3DTuple& rTuple)
        :   B3DTuple(rTuple)
        {}

        // data access read
        double getRed() const { return mfX; }
        double getGreen() const { return mfY; }
        double getBlue() const { return mfZ; }

        // data access write
        void setRed(double fNew) { mfX = fNew; }
        void setGreen(double fNew) { mfY = fNew; }
        void setBlue(double fNew) { mfZ = fNew; }

        /** *=operator to allow usage from BColor, too
        */
        BColor& operator*=( const BColor& rPnt )
        {
            mfX *= rPnt.mfX;
            mfY *= rPnt.mfY;
            mfZ *= rPnt.mfZ;
            return *this;
        }

        /** *=operator to allow usage from BColor, too
        */
        BColor& operator*=(double t)
        {
            mfX *= t;
            mfY *= t;
            mfZ *= t;
            return *this;
        }

        /** assignment operator to allow assigning the results
            of B3DTuple calculations
        */
        BColor& operator=( const ::basegfx::B3DTuple& rVec )
        {
            mfX = rVec.getX();
            mfY = rVec.getY();
            mfZ = rVec.getZ();
            return *this;
        }

        // luminance
        double luminance() const
        {
            const double fRedWeight(77.0 / 256.0);      // 0.30
            const double fGreenWeight(151.0 / 256.0);   // 0.59
            const double fBlueWeight(28.0 / 256.0);     // 0.11

            return (mfX * fRedWeight + mfY * fGreenWeight + mfZ * fBlueWeight);
        }

        // distances in color space
        double getDistanceRed(const BColor& rColor) const { return (getRed() > rColor.getRed() ? getRed() - rColor.getRed() : rColor.getRed() - getRed()); }
        double getDistanceGreen(const BColor& rColor) const { return (getGreen() > rColor.getGreen() ? getGreen() - rColor.getGreen() : rColor.getGreen() - getGreen()); }
        double getDistanceBlue(const BColor& rColor) const { return (getBlue() > rColor.getBlue() ? getBlue() - rColor.getBlue() : rColor.getBlue() - getBlue()); }

        double getDistance(const BColor& rColor) const
        {
            const double fDistR(getDistanceRed(rColor));
            const double fDistG(getDistanceGreen(rColor));
            const double fDistB(getDistanceBlue(rColor));

            return sqrt(fDistR * fDistR + fDistG * fDistG + fDistB * fDistB);
        }

        double getMaximumDistance(const BColor& rColor) const
        {
            const double fDistR(getDistanceRed(rColor));
            const double fDistG(getDistanceGreen(rColor));
            const double fDistB(getDistanceBlue(rColor));

            double fRetval(std::max(fDistR, fDistG));
            return std::max(fRetval, fDistB);
        }

        // clamp color to [0.0..1.0] values in all three intensity components
        BColor& clamp()
        {
            mfX = std::clamp(mfX, 0.0, 1.0);
            mfY = std::clamp(mfY, 0.0, 1.0);
            mfZ = std::clamp(mfZ, 0.0, 1.0);
            return *this;
        }

        void invert()
        {
            mfX = 1.0 - mfX;
            mfY = 1.0 - mfY;
            mfZ = 1.0 - mfZ;
        }

        static const BColor& getEmptyBColor()
        {
            return static_cast<const BColor&>( ::basegfx::B3DTuple::getEmptyTuple() );
        }

    };

    template<typename charT, typename traits>
    std::basic_ostream<charT, traits> & operator <<(
        std::basic_ostream<charT, traits> & stream, BColor const & color)
    {
        return stream
            << '[' << color.getRed() << ", " << color.getGreen() << ", "
            << color.getBlue() << ']';
    }
} // end of namespace basegfx

#endif // INCLUDED_BASEGFX_COLOR_BCOLOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
