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

#ifndef _BGFX_TUPLE_B2DTUPLE_HXX
#define _BGFX_TUPLE_B2DTUPLE_HXX

#include <sal/types.h>
#include <basegfx/numeric/ftools.hxx>
#include <algorithm>
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
            return mfX == rTup.mfX && mfY == rTup.mfY;
        }

        bool operator!=( const B2DTuple& rTup ) const
        {
            return mfX != rTup.mfX || mfY != rTup.mfY;
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
        return B2DTuple(
            std::min(rTupB.getX(), rTupA.getX()),
            std::min(rTupB.getY(), rTupA.getY()));
    }

    inline B2DTuple maximum(const B2DTuple& rTupA, const B2DTuple& rTupB)
    {
        return B2DTuple(
            std::max(rTupB.getX(), rTupA.getX()),
            std::max(rTupB.getY(), rTupA.getY()));
    }

    inline B2DTuple absolute(const B2DTuple& rTup)
    {
        B2DTuple aAbs(
            fabs(rTup.getX()),
            fabs(rTup.getY()));
        return aAbs;
    }

    inline B2DTuple interpolate(const B2DTuple& rOld1, const B2DTuple& rOld2, double t)
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
            return B2DTuple(
                ((rOld2.getX() - rOld1.getX()) * t) + rOld1.getX(),
                ((rOld2.getY() - rOld1.getY()) * t) + rOld1.getY());
        }
    }

    inline B2DTuple average(const B2DTuple& rOld1, const B2DTuple& rOld2)
    {
        return B2DTuple(
            rOld1.getX() == rOld2.getX() ? rOld1.getX() : (rOld1.getX() + rOld2.getX()) * 0.5,
            rOld1.getY() == rOld2.getY() ? rOld1.getY() : (rOld1.getY() + rOld2.getY()) * 0.5);
    }

    inline B2DTuple average(const B2DTuple& rOld1, const B2DTuple& rOld2, const B2DTuple& rOld3)
    {
        return B2DTuple(
            (rOld1.getX() == rOld2.getX() && rOld2.getX() == rOld3.getX()) ? rOld1.getX() : (rOld1.getX() + rOld2.getX() + rOld3.getX()) * (1.0 / 3.0),
            (rOld1.getY() == rOld2.getY() && rOld2.getY() == rOld3.getY()) ? rOld1.getY() : (rOld1.getY() + rOld2.getY() + rOld3.getY()) * (1.0 / 3.0));
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
