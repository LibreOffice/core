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

#pragma once

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
        bool bAutomatic = false;

    public:
        /** Create a Color with red, green and blue components from [0.0 to 1.0]

            The color is initialized to (0.0, 0.0, 0.0)
        */
        BColor()
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
        double getRed() const { return mnX; }
        double getGreen() const { return mnY; }
        double getBlue() const { return mnZ; }
        bool isAutomatic() const { return bAutomatic; }

        // data access write
        void setRed(double fNew) { mnX = fNew; }
        void setGreen(double fNew) { mnY = fNew; }
        void setBlue(double fNew) { mnZ = fNew; }
        void setAutomatic(bool bNew) { bAutomatic = bNew; }

        /** *=operator to allow usage from BColor, too
        */
        BColor& operator*=( const BColor& rPnt )
        {
            mnX *= rPnt.mnX;
            mnY *= rPnt.mnY;
            mnZ *= rPnt.mnZ;
            return *this;
        }

        /** *=operator to allow usage from BColor, too
        */
        BColor& operator*=(double t)
        {
            mnX *= t;
            mnY *= t;
            mnZ *= t;
            return *this;
        }

        /** assignment operator to allow assigning the results
            of B3DTuple calculations
        */
        BColor& operator=( const ::basegfx::B3DTuple& rVec )
        {
            mnX = rVec.getX();
            mnY = rVec.getY();
            mnZ = rVec.getZ();
            return *this;
        }

        // luminance
        double luminance() const
        {
            const double fRedWeight(77.0 / 256.0);      // 0.30
            const double fGreenWeight(151.0 / 256.0);   // 0.59
            const double fBlueWeight(28.0 / 256.0);     // 0.11

            return (mnX * fRedWeight + mnY * fGreenWeight + mnZ * fBlueWeight);
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

            return std::hypot(fDistR, fDistG, fDistB);
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
            mnX = std::clamp(mnX, 0.0, 1.0);
            mnY = std::clamp(mnY, 0.0, 1.0);
            mnZ = std::clamp(mnZ, 0.0, 1.0);
            return *this;
        }

        void invert()
        {
            mnX = 1.0 - mnX;
            mnY = 1.0 - mnY;
            mnZ = 1.0 - mnZ;
        }

        static const BColor& getEmptyBColor()
        {
            static BColor const singleton;
            return singleton;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
