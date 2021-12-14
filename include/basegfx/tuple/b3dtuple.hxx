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

#include <sal/types.h>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/basegfxdllapi.h>
#include <basegfx/tuple/Tuple3D.hxx>

namespace basegfx
{
    class B3ITuple;

    /** Base class for all Points/Vectors with three double values

        This class provides all methods common to Point
        and Vector classes which are derived from here.

        @derive Use this class to implement Points or Vectors
        which are based on three double values
    */
    class SAL_WARN_UNUSED BASEGFX_DLLPUBLIC B3DTuple : public Tuple3D<double>
    {
    public:
        /** Create a 3D Tuple

            The tuple is initialized to (0.0, 0.0, 0.0)
        */
        B3DTuple()
            : Tuple3D(0.0, 0.0, 0.0)
        {}

        /** Create a 3D Tuple

            @param fX
            This parameter is used to initialize the X-coordinate
            of the 3D Tuple.

            @param fY
            This parameter is used to initialize the Y-coordinate
            of the 3D Tuple.

            @param fZ
            This parameter is used to initialize the Z-coordinate
            of the 3D Tuple.
        */
        B3DTuple(double fX, double fY, double fZ)
            : Tuple3D(fX, fY, fZ)
        {}

        /// Array-access to 3D Tuple
        const double& operator[] (int nPos) const
        {
            // Here, normally two if(...)'s should be used. In the assumption that
            // both double members can be accessed as an array a shortcut is used here.
            // if(0 == nPos) return mfX; if(1 == nPos) return mfY; return mfZ;
            return *((&mfX) + nPos);
        }

        /// Array-access to 3D Tuple
        double& operator[] (int nPos)
        {
            // Here, normally two if(...)'s should be used. In the assumption that
            // both double members can be accessed as an array a shortcut is used here.
            // if(0 == nPos) return mfX; if(1 == nPos) return mfY; return mfZ;
            return *((&mfX) + nPos);
        }

        // comparators with tolerance


        bool equalZero() const
        {
            return (this == &getEmptyTuple() ||
                (::basegfx::fTools::equalZero(mfX)
                && ::basegfx::fTools::equalZero(mfY)
                && ::basegfx::fTools::equalZero(mfZ)));
        }

        bool equal(const B3DTuple& rTup) const
        {
            return (
                this == &rTup ||
                (::basegfx::fTools::equal(mfX, rTup.mfX) &&
                ::basegfx::fTools::equal(mfY, rTup.mfY) &&
                ::basegfx::fTools::equal(mfZ, rTup.mfZ)));
        }

        // operators

        B3DTuple operator-(void) const
        {
            return B3DTuple(-mfX, -mfY, -mfZ);
        }

        bool operator==(const B3DTuple& rTup) const
        {
            return mfX == rTup.mfX && mfY == rTup.mfY && mfZ == rTup.mfZ;
        }

        bool operator!=(const B3DTuple& rTup) const { return !operator==(rTup); }

        void correctValues(const double fCompareValue = 0.0)
        {
            if(0.0 == fCompareValue)
            {
                if(::basegfx::fTools::equalZero(mfX))
                {
                    mfX = 0.0;
                }

                if(::basegfx::fTools::equalZero(mfY))
                {
                    mfY = 0.0;
                }

                if(::basegfx::fTools::equalZero(mfZ))
                {
                    mfZ = 0.0;
                }
            }
            else
            {
                if(::basegfx::fTools::equal(mfX, fCompareValue))
                {
                    mfX = fCompareValue;
                }

                if(::basegfx::fTools::equal(mfY, fCompareValue))
                {
                    mfY = fCompareValue;
                }

                if(::basegfx::fTools::equal(mfZ, fCompareValue))
                {
                    mfZ = fCompareValue;
                }
            }
        }

        static const B3DTuple& getEmptyTuple();
    };

    // external operators


    inline B3DTuple interpolate(const B3DTuple& rOld1, const B3DTuple& rOld2, double t)
    {
        if(rOld1 == rOld2)
        {
            return rOld1;
        }
        else if(0.0 >= t)
        {
            return rOld1;
        }
        else if(1.0 <= t)
        {
            return rOld2;
        }
        else
        {
            return B3DTuple(
                ((rOld2.getX() - rOld1.getX()) * t) + rOld1.getX(),
                ((rOld2.getY() - rOld1.getY()) * t) + rOld1.getY(),
                ((rOld2.getZ() - rOld1.getZ()) * t) + rOld1.getZ());
        }
    }

    inline B3DTuple average(const B3DTuple& rOld1, const B3DTuple& rOld2)
    {
        return B3DTuple(
            rtl_math_approxEqual(rOld1.getX(), rOld2.getX()) ? rOld1.getX() : (rOld1.getX() + rOld2.getX()) * 0.5,
            rtl_math_approxEqual(rOld1.getY(), rOld2.getY()) ? rOld1.getY() : (rOld1.getY() + rOld2.getY()) * 0.5,
            rtl_math_approxEqual(rOld1.getZ(), rOld2.getZ()) ? rOld1.getZ() : (rOld1.getZ() + rOld2.getZ()) * 0.5);
    }

    inline B3DTuple operator+(const B3DTuple& rTupA, const B3DTuple& rTupB)
    {
        B3DTuple aSum(rTupA);
        aSum += rTupB;
        return aSum;
    }

    inline B3DTuple operator-(const B3DTuple& rTupA, const B3DTuple& rTupB)
    {
        B3DTuple aSub(rTupA);
        aSub -= rTupB;
        return aSub;
    }

    inline B3DTuple operator*(const B3DTuple& rTupA, const B3DTuple& rTupB)
    {
        B3DTuple aMul(rTupA);
        aMul *= rTupB;
        return aMul;
    }

    inline B3DTuple operator*(const B3DTuple& rTup, double t)
    {
        B3DTuple aNew(rTup);
        aNew *= t;
        return aNew;
    }

    inline B3DTuple operator/(const B3DTuple& rTup, double t)
    {
        B3DTuple aNew(rTup);
        aNew /= t;
        return aNew;
    }

    /** Round double to nearest integer for 3D tuple

        @return the nearest integer for this tuple
    */
    BASEGFX_DLLPUBLIC B3ITuple fround(const B3DTuple& rTup);
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
