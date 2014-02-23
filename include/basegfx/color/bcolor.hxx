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

#include <basegfx/tuple/b3dtuple.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <vector>
#include <basegfx/basegfxdllapi.h>


// predeclarations

namespace com { namespace sun { namespace star { namespace rendering {
    class XGraphicDevice;
}}}}



namespace basegfx
{
    /** Base Color class with three double values

        This class derives all operators and common handling for
        a 3D data class from B3DTuple. All necessary extensions
        which are special for colors will be added here.

        @see B3DTuple
    */
    class BASEGFX_DLLPUBLIC SAL_WARN_UNUSED BColor : public B3DTuple
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

        /** Create a copy of a Color

            @param rVec
            The Color which will be copied.
        */
        BColor(const BColor& rVec)
        :   B3DTuple(rVec)
        {}

        /** constructor with tuple to allow copy-constructing
            from B3DTuple-based classes
        */
        BColor(const ::basegfx::B3DTuple& rTuple)
        :   B3DTuple(rTuple)
        {}

        ~BColor()
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

        // blend to another color using luminance
        void blend(const BColor& rColor)
        {
            const double fLuminance(luminance());
            mfX = rColor.getRed() * fLuminance;
            mfY = rColor.getGreen() * fLuminance;
            mfZ = rColor.getBlue() * fLuminance;
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

        double getMinimumDistance(const BColor& rColor) const
        {
            const double fDistR(getDistanceRed(rColor));
            const double fDistG(getDistanceGreen(rColor));
            const double fDistB(getDistanceBlue(rColor));

            double fRetval(fDistR < fDistG ? fDistR : fDistG);
            return (fRetval < fDistB ? fRetval : fDistB);
        }

        double getMaximumDistance(const BColor& rColor) const
        {
            const double fDistR(getDistanceRed(rColor));
            const double fDistG(getDistanceGreen(rColor));
            const double fDistB(getDistanceBlue(rColor));

            double fRetval(fDistR > fDistG ? fDistR : fDistG);
            return (fRetval > fDistB ? fRetval : fDistB);
        }

        // clamp color to [0.0..1.0] values in all three intensity components
        BColor& clamp()
        {
            mfX = basegfx::clamp(mfX, 0.0, 1.0);
            mfY = basegfx::clamp(mfY, 0.0, 1.0);
            mfZ = basegfx::clamp(mfZ, 0.0, 1.0);
            return *this;
        }

        BColor& invert()
        {
            mfX = 1.0 - mfX;
            mfY = 1.0 - mfY;
            mfZ = 1.0 - mfZ;
            return *this;
        }

        static const BColor& getEmptyBColor()
        {
            return (const BColor&) ::basegfx::B3DTuple::getEmptyTuple();
        }

        com::sun::star::uno::Sequence< double > colorToDoubleSequence(const com::sun::star::uno::Reference< com::sun::star::rendering::XGraphicDevice >& /*xGraphicDevice*/) const
        {
            com::sun::star::uno::Sequence< double > aRet(4);
            double* pRet = aRet.getArray();

            pRet[0] = mfX;
            pRet[1] = mfY;
            pRet[2] = mfZ;
            pRet[3] = 1.0;

            return aRet;
        }
    };
} // end of namespace basegfx

#endif // INCLUDED_BASEGFX_COLOR_BCOLOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
