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

#ifndef INCLUDED_BASEGFX_TUPLE_B2I64TUPLE_HXX
#define INCLUDED_BASEGFX_TUPLE_B2I64TUPLE_HXX

#include <sal/types.h>
#include <basegfx/tuple/b2dtuple.hxx>
#include <basegfx/basegfxdllapi.h>


namespace basegfx
{
    /** Base class for all Points/Vectors with two sal_Int64 values

        This class provides all methods common to Point
        avd Vector classes which are derived from here.

        @derive Use this class to implement Points or Vectors
        which are based on two sal_Int64 values
    */
    class BASEGFX_DLLPUBLIC SAL_WARN_UNUSED B2I64Tuple
    {
    protected:
        sal_Int64                                       mnX;
        sal_Int64                                       mnY;

    public:
        /** Create a 2D Tuple

            The tuple is initialized to (0, 0)
        */
        B2I64Tuple()
        :   mnX(0),
            mnY(0)
        {}

        /** Create a 2D Tuple

            @param fX
            This parameter is used to initialize the X-coordinate
            of the 2D Tuple.

            @param fY
            This parameter is used to initialize the Y-coordinate
            of the 2D Tuple.
        */
        B2I64Tuple(sal_Int64 fX, sal_Int64 fY)
        :   mnX( fX ),
            mnY( fY )
        {}

        /** Create a copy of a 2D Tuple

            @param rTup
            The 2D Tuple which will be copied.
        */
        B2I64Tuple(const B2I64Tuple& rTup)
        :   mnX( rTup.mnX ),
            mnY( rTup.mnY )
        {}

        ~B2I64Tuple()
        {}

        /// Get X-Coordinate of 2D Tuple
        sal_Int64 getX() const
        {
            return mnX;
        }

        /// Get Y-Coordinate of 2D Tuple
        sal_Int64 getY() const
        {
            return mnY;
        }

        /// Set X-Coordinate of 2D Tuple
        void setX(sal_Int64 fX)
        {
            mnX = fX;
        }

        /// Set Y-Coordinate of 2D Tuple
        void setY(sal_Int64 fY)
        {
            mnY = fY;
        }

        /// Array-access to 2D Tuple
        const sal_Int64& operator[] (int nPos) const
        {
            // Here, normally one if(...) should be used. In the assumption that
            // both sal_Int64 members can be accessed as an array a shortcut is used here.
            // if(0 == nPos) return mnX; return mnY;
            return *((&mnX) + nPos);
        }

        /// Array-access to 2D Tuple
        sal_Int64& operator[] (int nPos)
        {
            // Here, normally one if(...) should be used. In the assumption that
            // both sal_Int64 members can be accessed as an array a shortcut is used here.
            // if(0 == nPos) return mnX; return mnY;
            return *((&mnX) + nPos);
        }

        // operators


        B2I64Tuple& operator+=( const B2I64Tuple& rTup )
        {
            mnX += rTup.mnX;
            mnY += rTup.mnY;
            return *this;
        }

        B2I64Tuple& operator-=( const B2I64Tuple& rTup )
        {
            mnX -= rTup.mnX;
            mnY -= rTup.mnY;
            return *this;
        }

        B2I64Tuple& operator/=( const B2I64Tuple& rTup )
        {
            mnX /= rTup.mnX;
            mnY /= rTup.mnY;
            return *this;
        }

        B2I64Tuple& operator*=( const B2I64Tuple& rTup )
        {
            mnX *= rTup.mnX;
            mnY *= rTup.mnY;
            return *this;
        }

        B2I64Tuple& operator*=(sal_Int64 t)
        {
            mnX *= t;
            mnY *= t;
            return *this;
        }

        B2I64Tuple& operator/=(sal_Int64 t)
        {
            mnX /= t;
            mnY /= t;
            return *this;
        }

        B2I64Tuple operator-(void) const
        {
            return B2I64Tuple(-mnX, -mnY);
        }

        bool equalZero() const { return mnX == 0 && mnY == 0; }

        bool operator==( const B2I64Tuple& rTup ) const
        {
            return this == &rTup || (rTup.mnX == mnX && rTup.mnY == mnY);
        }

        bool operator!=( const B2I64Tuple& rTup ) const
        {
            return !(*this == rTup);
        }

        B2I64Tuple& operator=( const B2I64Tuple& rTup )
        {
            mnX = rTup.mnX;
            mnY = rTup.mnY;
            return *this;
        }
    };

    // external operators


    inline B2I64Tuple minimum(const B2I64Tuple& rTupA, const B2I64Tuple& rTupB)
    {
        return B2I64Tuple(
            std::min(rTupB.getX(), rTupA.getX()),
            std::min(rTupB.getY(), rTupA.getY()));
    }

    inline B2I64Tuple maximum(const B2I64Tuple& rTupA, const B2I64Tuple& rTupB)
    {
        return B2I64Tuple(
            std::max(rTupB.getX(), rTupA.getX()),
            std::max(rTupB.getY(), rTupA.getY()));
    }

    inline B2I64Tuple absolute(const B2I64Tuple& rTup)
    {
        B2I64Tuple aAbs(
            (0 > rTup.getX()) ? -rTup.getX() : rTup.getX(),
            (0 > rTup.getY()) ? -rTup.getY() : rTup.getY());
        return aAbs;
    }

    inline B2I64Tuple interpolate(const B2I64Tuple& rOld1, const B2I64Tuple& rOld2, double t)
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
            return B2I64Tuple(
                basegfx::fround64(((rOld2.getX() - rOld1.getX()) * t) + rOld1.getX()),
                basegfx::fround64(((rOld2.getY() - rOld1.getY()) * t) + rOld1.getY()));
        }
    }

    inline B2I64Tuple average(const B2I64Tuple& rOld1, const B2I64Tuple& rOld2)
    {
        return B2I64Tuple(
            rOld1.getX() == rOld2.getX() ? rOld1.getX() : basegfx::fround64((rOld1.getX() + rOld2.getX()) * 0.5),
            rOld1.getY() == rOld2.getY() ? rOld1.getY() : basegfx::fround64((rOld1.getY() + rOld2.getY()) * 0.5));
    }

    inline B2I64Tuple average(const B2I64Tuple& rOld1, const B2I64Tuple& rOld2, const B2I64Tuple& rOld3)
    {
        return B2I64Tuple(
            (rOld1.getX() == rOld2.getX() && rOld2.getX() == rOld3.getX()) ? rOld1.getX() : basegfx::fround64((rOld1.getX() + rOld2.getX() + rOld3.getX()) * (1.0 / 3.0)),
            (rOld1.getY() == rOld2.getY() && rOld2.getY() == rOld3.getY()) ? rOld1.getY() : basegfx::fround64((rOld1.getY() + rOld2.getY() + rOld3.getY()) * (1.0 / 3.0)));
    }

    inline B2I64Tuple operator+(const B2I64Tuple& rTupA, const B2I64Tuple& rTupB)
    {
        B2I64Tuple aSum(rTupA);
        aSum += rTupB;
        return aSum;
    }

    inline B2I64Tuple operator-(const B2I64Tuple& rTupA, const B2I64Tuple& rTupB)
    {
        B2I64Tuple aSub(rTupA);
        aSub -= rTupB;
        return aSub;
    }

    inline B2I64Tuple operator/(const B2I64Tuple& rTupA, const B2I64Tuple& rTupB)
    {
        B2I64Tuple aDiv(rTupA);
        aDiv /= rTupB;
        return aDiv;
    }

    inline B2I64Tuple operator*(const B2I64Tuple& rTupA, const B2I64Tuple& rTupB)
    {
        B2I64Tuple aMul(rTupA);
        aMul *= rTupB;
        return aMul;
    }

    inline B2I64Tuple operator*(const B2I64Tuple& rTup, sal_Int64 t)
    {
        B2I64Tuple aNew(rTup);
        aNew *= t;
        return aNew;
    }

    inline B2I64Tuple operator*(sal_Int64 t, const B2I64Tuple& rTup)
    {
        B2I64Tuple aNew(rTup);
        aNew *= t;
        return aNew;
    }

    inline B2I64Tuple operator/(const B2I64Tuple& rTup, sal_Int64 t)
    {
        B2I64Tuple aNew(rTup);
        aNew /= t;
        return aNew;
    }

    inline B2I64Tuple operator/(sal_Int64 t, const B2I64Tuple& rTup)
    {
        B2I64Tuple aNew(t, t);
        B2I64Tuple aTmp(rTup);
        aNew /= aTmp;
        return aNew;
    }
} // end of namespace basegfx

#endif // INCLUDED_BASEGFX_TUPLE_B2I64TUPLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
