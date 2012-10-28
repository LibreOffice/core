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

#ifndef _BGFX_TUPLE_B3DTUPLE_HXX
#define _BGFX_TUPLE_B3DTUPLE_HXX

#include <sal/types.h>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
    // predeclarations
    class B3ITuple;

    /** Base class for all Points/Vectors with three double values

        This class provides all methods common to Point
        avd Vector classes which are derived from here.

        @derive Use this class to implement Points or Vectors
        which are based on three double values
    */
    class BASEGFX_DLLPUBLIC SAL_WARN_UNUSED B3DTuple
    {
    protected:
        double                                      mfX;
        double                                      mfY;
        double                                      mfZ;

    public:
        /** Create a 3D Tuple

            The tuple is initialized to (0.0, 0.0, 0.0)
        */
        B3DTuple()
        :   mfX(0.0),
            mfY(0.0),
            mfZ(0.0)
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
        :   mfX(fX),
            mfY(fY),
            mfZ(fZ)
        {}

        /** Create a copy of a 3D Tuple

            @param rTup
            The 3D Tuple which will be copied.
        */
        B3DTuple(const B3DTuple& rTup)
        :   mfX( rTup.mfX ),
            mfY( rTup.mfY ),
            mfZ( rTup.mfZ )
        {}

        ~B3DTuple()
        {}

        /// get X-Coordinate of 3D Tuple
        double getX() const
        {
            return mfX;
        }

        /// get Y-Coordinate of 3D Tuple
        double getY() const
        {
            return mfY;
        }

        /// get Z-Coordinate of 3D Tuple
        double getZ() const
        {
            return mfZ;
        }

        /// set X-Coordinate of 3D Tuple
        void setX(double fX)
        {
            mfX = fX;
        }

        /// set Y-Coordinate of 3D Tuple
        void setY(double fY)
        {
            mfY = fY;
        }

        /// set Z-Coordinate of 3D Tuple
        void setZ(double fZ)
        {
            mfZ = fZ;
        }

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
        //////////////////////////////////////////////////////////////////////

        bool equalZero() const
        {
            return (this == &getEmptyTuple() ||
                (::basegfx::fTools::equalZero(mfX)
                && ::basegfx::fTools::equalZero(mfY)
                && ::basegfx::fTools::equalZero(mfZ)));
        }

        bool equalZero(const double& rfSmallValue) const
        {
            return (this == &getEmptyTuple() ||
                (::basegfx::fTools::equalZero(mfX, rfSmallValue)
                && ::basegfx::fTools::equalZero(mfY, rfSmallValue)
                && ::basegfx::fTools::equalZero(mfZ, rfSmallValue)));
        }

        bool equal(const B3DTuple& rTup) const
        {
            return (
                this == &rTup ||
                (::basegfx::fTools::equal(mfX, rTup.mfX) &&
                ::basegfx::fTools::equal(mfY, rTup.mfY) &&
                ::basegfx::fTools::equal(mfZ, rTup.mfZ)));
        }

        bool equal(const B3DTuple& rTup, const double& rfSmallValue) const
        {
            return (
                this == &rTup ||
                (::basegfx::fTools::equal(mfX, rTup.mfX, rfSmallValue) &&
                ::basegfx::fTools::equal(mfY, rTup.mfY, rfSmallValue) &&
                ::basegfx::fTools::equal(mfZ, rTup.mfZ, rfSmallValue)));
        }

        // operators
        //////////////////////////////////////////////////////////////////////

        B3DTuple& operator+=( const B3DTuple& rTup )
        {
            mfX += rTup.mfX;
            mfY += rTup.mfY;
            mfZ += rTup.mfZ;
            return *this;
        }

        B3DTuple& operator-=( const B3DTuple& rTup )
        {
            mfX -= rTup.mfX;
            mfY -= rTup.mfY;
            mfZ -= rTup.mfZ;
            return *this;
        }

        B3DTuple& operator/=( const B3DTuple& rTup )
        {
            mfX /= rTup.mfX;
            mfY /= rTup.mfY;
            mfZ /= rTup.mfZ;
            return *this;
        }

        B3DTuple& operator*=( const B3DTuple& rTup )
        {
            mfX *= rTup.mfX;
            mfY *= rTup.mfY;
            mfZ *= rTup.mfZ;
            return *this;
        }

        B3DTuple& operator*=(double t)
        {
            mfX *= t;
            mfY *= t;
            mfZ *= t;
            return *this;
        }

        B3DTuple& operator/=(double t)
        {
            const double fVal(1.0 / t);
            mfX *= fVal;
            mfY *= fVal;
            mfZ *= fVal;
            return *this;
        }

        B3DTuple operator-(void) const
        {
            return B3DTuple(-mfX, -mfY, -mfZ);
        }

        bool operator==( const B3DTuple& rTup ) const
        {
            return equal(rTup);
        }

        bool operator!=( const B3DTuple& rTup ) const
        {
            return !equal(rTup);
        }

        B3DTuple& operator=( const B3DTuple& rTup )
        {
            mfX = rTup.mfX;
            mfY = rTup.mfY;
            mfZ = rTup.mfZ;
            return *this;
        }

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
    //////////////////////////////////////////////////////////////////////////

    inline B3DTuple minimum(const B3DTuple& rTupA, const B3DTuple& rTupB)
    {
        B3DTuple aMin(
            (rTupB.getX() < rTupA.getX()) ? rTupB.getX() : rTupA.getX(),
            (rTupB.getY() < rTupA.getY()) ? rTupB.getY() : rTupA.getY(),
            (rTupB.getZ() < rTupA.getZ()) ? rTupB.getZ() : rTupA.getZ());
        return aMin;
    }

    inline B3DTuple maximum(const B3DTuple& rTupA, const B3DTuple& rTupB)
    {
        B3DTuple aMax(
            (rTupB.getX() > rTupA.getX()) ? rTupB.getX() : rTupA.getX(),
            (rTupB.getY() > rTupA.getY()) ? rTupB.getY() : rTupA.getY(),
            (rTupB.getZ() > rTupA.getZ()) ? rTupB.getZ() : rTupA.getZ());
        return aMax;
    }

    inline B3DTuple absolute(const B3DTuple& rTup)
    {
        B3DTuple aAbs(
            (0.0 > rTup.getX()) ? -rTup.getX() : rTup.getX(),
            (0.0 > rTup.getY()) ? -rTup.getY() : rTup.getY(),
            (0.0 > rTup.getZ()) ? -rTup.getZ() : rTup.getZ());
        return aAbs;
    }

    inline B3DTuple interpolate(const B3DTuple& rOld1, const B3DTuple& rOld2, double t)
    {
        B3DTuple aInt(
            ((rOld2.getX() - rOld1.getX()) * t) + rOld1.getX(),
            ((rOld2.getY() - rOld1.getY()) * t) + rOld1.getY(),
            ((rOld2.getZ() - rOld1.getZ()) * t) + rOld1.getZ());
        return aInt;
    }

    inline B3DTuple average(const B3DTuple& rOld1, const B3DTuple& rOld2)
    {
        B3DTuple aAvg(
            (rOld1.getX() + rOld2.getX()) * 0.5,
            (rOld1.getY() + rOld2.getY()) * 0.5,
            (rOld1.getZ() + rOld2.getZ()) * 0.5);
        return aAvg;
    }

    inline B3DTuple average(const B3DTuple& rOld1, const B3DTuple& rOld2, const B3DTuple& rOld3)
    {
        B3DTuple aAvg(
            (rOld1.getX() + rOld2.getX() + rOld3.getX()) * (1.0 / 3.0),
            (rOld1.getY() + rOld2.getY() + rOld3.getY()) * (1.0 / 3.0),
            (rOld1.getZ() + rOld2.getZ() + rOld3.getZ()) * (1.0 / 3.0));
        return aAvg;
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

    inline B3DTuple operator/(const B3DTuple& rTupA, const B3DTuple& rTupB)
    {
        B3DTuple aDiv(rTupA);
        aDiv /= rTupB;
        return aDiv;
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

    inline B3DTuple operator*(double t, const B3DTuple& rTup)
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

    inline B3DTuple operator/(double t, const B3DTuple& rTup)
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

#endif /* _BGFX_TUPLE_B3DTUPLE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
