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

#ifndef _BGFX_TUPLE_B2DTUPLE_HXX
#define _BGFX_TUPLE_B2DTUPLE_HXX

#include <sal/types.h>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
    // predeclarations
    class B2ITuple;

    /** Base class for all Points/Vectors with two double values

        This class provides all methods common to Point
        avd Vector classes which are derived from here.

        @derive Use this class to implement Points or Vectors
        which are based on two double values
    */
    class SAL_WARN_UNUSED B2DTuple
    {
    protected:
        double                                      mfX;
        double                                      mfY;

    public:
        /** Create a 2D Tuple

            The tuple is initialized to (0.0, 0.0)
        */
        B2DTuple()
        :   mfX(0.0),
            mfY(0.0)
        {}

        /** Create a 2D Tuple

            @param fX
            This parameter is used to initialize the X-coordinate
            of the 2D Tuple.

            @param fY
            This parameter is used to initialize the Y-coordinate
            of the 2D Tuple.
        */
        B2DTuple(double fX, double fY)
        :   mfX( fX ),
            mfY( fY )
        {}

        /** Create a copy of a 2D Tuple

            @param rTup
            The 2D Tuple which will be copied.
        */
        B2DTuple(const B2DTuple& rTup)
        :   mfX( rTup.mfX ),
            mfY( rTup.mfY )
        {}

        /** Create a copy of a 2D integer Tuple

            @param rTup
            The 2D Tuple which will be copied.
        */
        BASEGFX_DLLPUBLIC explicit B2DTuple(const B2ITuple& rTup);

        ~B2DTuple()
        {}

        /// Get X-Coordinate of 2D Tuple
        double getX() const
        {
            return mfX;
        }

        /// Get Y-Coordinate of 2D Tuple
        double getY() const
        {
            return mfY;
        }

        /// Set X-Coordinate of 2D Tuple
        void setX(double fX)
        {
            mfX = fX;
        }

        /// Set Y-Coordinate of 2D Tuple
        void setY(double fY)
        {
            mfY = fY;
        }

        /// Array-access to 2D Tuple
        const double& operator[] (int nPos) const
        {
            // Here, normally one if(...) should be used. In the assumption that
            // both double members can be accessed as an array a shortcut is used here.
            // if(0 == nPos) return mfX; return mfY;
            return *((&mfX) + nPos);
        }

        /// Array-access to 2D Tuple
        double& operator[] (int nPos)
        {
            // Here, normally one if(...) should be used. In the assumption that
            // both double members can be accessed as an array a shortcut is used here.
            // if(0 == nPos) return mfX; return mfY;
            return *((&mfX) + nPos);
        }

        // comparators with tolerance
        //////////////////////////////////////////////////////////////////////

        bool equalZero() const
        {
            return (this == &getEmptyTuple() ||
                    (fTools::equalZero(mfX) && fTools::equalZero(mfY)));
        }

        bool equalZero(const double& rfSmallValue) const
        {
            return (this == &getEmptyTuple() ||
                    (fTools::equalZero(mfX, rfSmallValue) && fTools::equalZero(mfY, rfSmallValue)));
        }

        bool equal(const B2DTuple& rTup) const
        {
            return (
                this == &rTup ||
                (fTools::equal(mfX, rTup.mfX) &&
                fTools::equal(mfY, rTup.mfY)));
        }

        bool equal(const B2DTuple& rTup, const double& rfSmallValue) const
        {
            return (
                this == &rTup ||
                (fTools::equal(mfX, rTup.mfX, rfSmallValue) &&
                fTools::equal(mfY, rTup.mfY, rfSmallValue)));
        }

        // operators
        //////////////////////////////////////////////////////////////////////

        B2DTuple& operator+=( const B2DTuple& rTup )
        {
            mfX += rTup.mfX;
            mfY += rTup.mfY;
            return *this;
        }

        B2DTuple& operator-=( const B2DTuple& rTup )
        {
            mfX -= rTup.mfX;
            mfY -= rTup.mfY;
            return *this;
        }

        B2DTuple& operator/=( const B2DTuple& rTup )
        {
            mfX /= rTup.mfX;
            mfY /= rTup.mfY;
            return *this;
        }

        B2DTuple& operator*=( const B2DTuple& rTup )
        {
            mfX *= rTup.mfX;
            mfY *= rTup.mfY;
            return *this;
        }

        B2DTuple& operator*=(double t)
        {
            mfX *= t;
            mfY *= t;
            return *this;
        }

        B2DTuple& operator/=(double t)
        {
            const double fVal(1.0 / t);
            mfX *= fVal;
            mfY *= fVal;
            return *this;
        }

        B2DTuple operator-(void) const
        {
            return B2DTuple(-mfX, -mfY);
        }

        bool operator==( const B2DTuple& rTup ) const
        {
            return equal(rTup);
        }

        bool operator!=( const B2DTuple& rTup ) const
        {
            return !equal(rTup);
        }

        B2DTuple& operator=( const B2DTuple& rTup )
        {
            mfX = rTup.mfX;
            mfY = rTup.mfY;
            return *this;
        }

        BASEGFX_DLLPUBLIC static const B2DTuple& getEmptyTuple();
    };

    // external operators
    //////////////////////////////////////////////////////////////////////////

    inline B2DTuple minimum(const B2DTuple& rTupA, const B2DTuple& rTupB)
    {
        B2DTuple aMin(
            (rTupB.getX() < rTupA.getX()) ? rTupB.getX() : rTupA.getX(),
            (rTupB.getY() < rTupA.getY()) ? rTupB.getY() : rTupA.getY());
        return aMin;
    }

    inline B2DTuple maximum(const B2DTuple& rTupA, const B2DTuple& rTupB)
    {
        B2DTuple aMax(
            (rTupB.getX() > rTupA.getX()) ? rTupB.getX() : rTupA.getX(),
            (rTupB.getY() > rTupA.getY()) ? rTupB.getY() : rTupA.getY());
        return aMax;
    }

    inline B2DTuple absolute(const B2DTuple& rTup)
    {
        B2DTuple aAbs(
            (0.0 > rTup.getX()) ? -rTup.getX() : rTup.getX(),
            (0.0 > rTup.getY()) ? -rTup.getY() : rTup.getY());
        return aAbs;
    }

    inline B2DTuple interpolate(const B2DTuple& rOld1, const B2DTuple& rOld2, double t)
    {
        B2DTuple aInt(
            ((rOld2.getX() - rOld1.getX()) * t) + rOld1.getX(),
            ((rOld2.getY() - rOld1.getY()) * t) + rOld1.getY());
        return aInt;
    }

    inline B2DTuple average(const B2DTuple& rOld1, const B2DTuple& rOld2)
    {
        B2DTuple aAvg(
            (rOld1.getX() + rOld2.getX()) * 0.5,
            (rOld1.getY() + rOld2.getY()) * 0.5);
        return aAvg;
    }

    inline B2DTuple average(const B2DTuple& rOld1, const B2DTuple& rOld2, const B2DTuple& rOld3)
    {
        B2DTuple aAvg(
            (rOld1.getX() + rOld2.getX() + rOld3.getX()) * (1.0 / 3.0),
            (rOld1.getY() + rOld2.getY() + rOld3.getY()) * (1.0 / 3.0));
        return aAvg;
    }

    inline B2DTuple operator+(const B2DTuple& rTupA, const B2DTuple& rTupB)
    {
        B2DTuple aSum(rTupA);
        aSum += rTupB;
        return aSum;
    }

    inline B2DTuple operator-(const B2DTuple& rTupA, const B2DTuple& rTupB)
    {
        B2DTuple aSub(rTupA);
        aSub -= rTupB;
        return aSub;
    }

    inline B2DTuple operator/(const B2DTuple& rTupA, const B2DTuple& rTupB)
    {
        B2DTuple aDiv(rTupA);
        aDiv /= rTupB;
        return aDiv;
    }

    inline B2DTuple operator*(const B2DTuple& rTupA, const B2DTuple& rTupB)
    {
        B2DTuple aMul(rTupA);
        aMul *= rTupB;
        return aMul;
    }

    inline B2DTuple operator*(const B2DTuple& rTup, double t)
    {
        B2DTuple aNew(rTup);
        aNew *= t;
        return aNew;
    }

    inline B2DTuple operator*(double t, const B2DTuple& rTup)
    {
        B2DTuple aNew(rTup);
        aNew *= t;
        return aNew;
    }

    inline B2DTuple operator/(const B2DTuple& rTup, double t)
    {
        B2DTuple aNew(rTup);
        aNew /= t;
        return aNew;
    }

    inline B2DTuple operator/(double t, const B2DTuple& rTup)
    {
        B2DTuple aNew(t, t);
        B2DTuple aTmp(rTup);
        aNew /= aTmp;
        return aNew;
    }

    /** Round double to nearest integer for 2D tuple

        @return the nearest integer for this tuple
    */
    BASEGFX_DLLPUBLIC B2ITuple fround(const B2DTuple& rTup);
} // end of namespace basegfx

#endif /* _BGFX_TUPLE_B2DTUPLE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
