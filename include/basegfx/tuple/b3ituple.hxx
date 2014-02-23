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

#ifndef INCLUDED_BASEGFX_TUPLE_B3ITUPLE_HXX
#define INCLUDED_BASEGFX_TUPLE_B3ITUPLE_HXX

#include <sal/types.h>
#include <basegfx/tuple/b3dtuple.hxx>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
    /** Base class for all Points/Vectors with three sal_Int32 values

        This class provides all methods common to Point
        avd Vector classes which are derived from here.

        @derive Use this class to implement Points or Vectors
        which are based on three sal_Int32 values
    */
    class BASEGFX_DLLPUBLIC SAL_WARN_UNUSED B3ITuple
    {
    protected:
        sal_Int32                                       mnX;
        sal_Int32                                       mnY;
        sal_Int32                                       mnZ;

    public:
        /** Create a 3D Tuple

            The tuple is initialized to (0, 0, 0)
        */
        B3ITuple()
        :   mnX(0),
            mnY(0),
            mnZ(0)
        {}

        /** Create a 3D Tuple

            @param nX
            This parameter is used to initialize the X-coordinate
            of the 3D Tuple.

            @param nY
            This parameter is used to initialize the Y-coordinate
            of the 3D Tuple.

            @param nZ
            This parameter is used to initialize the Z-coordinate
            of the 3D Tuple.
        */
        B3ITuple(sal_Int32 nX, sal_Int32 nY, sal_Int32 nZ)
        :   mnX(nX),
            mnY(nY),
            mnZ(nZ)
        {}

        /** Create a copy of a 3D Tuple

            @param rTup
            The 3D Tuple which will be copied.
        */
        B3ITuple(const B3ITuple& rTup)
        :   mnX( rTup.mnX ),
            mnY( rTup.mnY ),
            mnZ( rTup.mnZ )
        {}

        ~B3ITuple()
        {}

        /// get X-Coordinate of 3D Tuple
        sal_Int32 getX() const
        {
            return mnX;
        }

        /// get Y-Coordinate of 3D Tuple
        sal_Int32 getY() const
        {
            return mnY;
        }

        /// get Z-Coordinate of 3D Tuple
        sal_Int32 getZ() const
        {
            return mnZ;
        }

        /// set X-Coordinate of 3D Tuple
        void setX(sal_Int32 nX)
        {
            mnX = nX;
        }

        /// set Y-Coordinate of 3D Tuple
        void setY(sal_Int32 nY)
        {
            mnY = nY;
        }

        /// set Z-Coordinate of 3D Tuple
        void setZ(sal_Int32 nZ)
        {
            mnZ = nZ;
        }

        /// Array-access to 3D Tuple
        const sal_Int32& operator[] (int nPos) const
        {
            // Here, normally two if(...)'s should be used. In the assumption that
            // both sal_Int32 members can be accessed as an array a shortcut is used here.
            // if(0 == nPos) return mnX; if(1 == nPos) return mnY; return mnZ;
            return *((&mnX) + nPos);
        }

        /// Array-access to 3D Tuple
        sal_Int32& operator[] (int nPos)
        {
            // Here, normally two if(...)'s should be used. In the assumption that
            // both sal_Int32 members can be accessed as an array a shortcut is used here.
            // if(0 == nPos) return mnX; if(1 == nPos) return mnY; return mnZ;
            return *((&mnX) + nPos);
        }

        // operators


        B3ITuple& operator+=( const B3ITuple& rTup )
        {
            mnX += rTup.mnX;
            mnY += rTup.mnY;
            mnZ += rTup.mnZ;
            return *this;
        }

        B3ITuple& operator-=( const B3ITuple& rTup )
        {
            mnX -= rTup.mnX;
            mnY -= rTup.mnY;
            mnZ -= rTup.mnZ;
            return *this;
        }

        B3ITuple& operator/=( const B3ITuple& rTup )
        {
            mnX /= rTup.mnX;
            mnY /= rTup.mnY;
            mnZ /= rTup.mnZ;
            return *this;
        }

        B3ITuple& operator*=( const B3ITuple& rTup )
        {
            mnX *= rTup.mnX;
            mnY *= rTup.mnY;
            mnZ *= rTup.mnZ;
            return *this;
        }

        B3ITuple& operator*=(sal_Int32 t)
        {
            mnX *= t;
            mnY *= t;
            mnZ *= t;
            return *this;
        }

        B3ITuple& operator/=(sal_Int32 t)
        {
            mnX /= t;
            mnY /= t;
            mnZ /= t;
            return *this;
        }

        B3ITuple operator-(void) const
        {
            return B3ITuple(-mnX, -mnY, -mnZ);
        }

        bool operator==( const B3ITuple& rTup ) const
        {
            return this == &rTup || (rTup.mnX == mnX && rTup.mnY == mnY && rTup.mnZ == mnZ);
        }

        bool operator!=( const B3ITuple& rTup ) const
        {
            return !(*this == rTup);
        }

        B3ITuple& operator=( const B3ITuple& rTup )
        {
            mnX = rTup.mnX;
            mnY = rTup.mnY;
            mnZ = rTup.mnZ;
            return *this;
        }
    };

    // external operators


    inline B3ITuple minimum(const B3ITuple& rTupA, const B3ITuple& rTupB)
    {
        return B3ITuple(
            std::min(rTupB.getX(), rTupA.getX()),
            std::min(rTupB.getY(), rTupA.getY()),
            std::min(rTupB.getZ(), rTupA.getZ()));
    }

    inline B3ITuple maximum(const B3ITuple& rTupA, const B3ITuple& rTupB)
    {
        return B3ITuple(
            std::max(rTupB.getX(), rTupA.getX()),
            std::max(rTupB.getY(), rTupA.getY()),
            std::max(rTupB.getZ(), rTupA.getZ()));
    }

    inline B3ITuple absolute(const B3ITuple& rTup)
    {
        B3ITuple aAbs(
            (0 > rTup.getX()) ? -rTup.getX() : rTup.getX(),
            (0 > rTup.getY()) ? -rTup.getY() : rTup.getY(),
            (0 > rTup.getZ()) ? -rTup.getZ() : rTup.getZ());
        return aAbs;
    }

    inline B3ITuple interpolate(const B3ITuple& rOld1, const B3ITuple& rOld2, double t)
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
            return B3ITuple(
                basegfx::fround(((rOld2.getX() - rOld1.getX()) * t) + rOld1.getX()),
                basegfx::fround(((rOld2.getY() - rOld1.getY()) * t) + rOld1.getY()),
                basegfx::fround(((rOld2.getZ() - rOld1.getZ()) * t) + rOld1.getZ()));
        }
    }

    inline B3ITuple average(const B3ITuple& rOld1, const B3ITuple& rOld2)
    {
        return B3ITuple(
            rOld1.getX() == rOld2.getX() ? rOld1.getX() : basegfx::fround((rOld1.getX() + rOld2.getX()) * 0.5),
            rOld1.getY() == rOld2.getY() ? rOld1.getY() : basegfx::fround((rOld1.getY() + rOld2.getY()) * 0.5),
            rOld1.getZ() == rOld2.getZ() ? rOld1.getZ() : basegfx::fround((rOld1.getZ() + rOld2.getZ()) * 0.5));
    }

    inline B3ITuple average(const B3ITuple& rOld1, const B3ITuple& rOld2, const B3ITuple& rOld3)
    {
        return B3ITuple(
            (rOld1.getX() == rOld2.getX() && rOld2.getX() == rOld3.getX()) ? rOld1.getX() : basegfx::fround((rOld1.getX() + rOld2.getX() + rOld3.getX()) * (1.0 / 3.0)),
            (rOld1.getY() == rOld2.getY() && rOld2.getY() == rOld3.getY()) ? rOld1.getX() : basegfx::fround((rOld1.getY() + rOld2.getY() + rOld3.getY()) * (1.0 / 3.0)),
            (rOld1.getZ() == rOld2.getZ() && rOld2.getZ() == rOld3.getZ()) ? rOld1.getX() : basegfx::fround((rOld1.getZ() + rOld2.getZ() + rOld3.getZ()) * (1.0 / 3.0)));
    }

    inline B3ITuple operator+(const B3ITuple& rTupA, const B3ITuple& rTupB)
    {
        B3ITuple aSum(rTupA);
        aSum += rTupB;
        return aSum;
    }

    inline B3ITuple operator-(const B3ITuple& rTupA, const B3ITuple& rTupB)
    {
        B3ITuple aSub(rTupA);
        aSub -= rTupB;
        return aSub;
    }

    inline B3ITuple operator/(const B3ITuple& rTupA, const B3ITuple& rTupB)
    {
        B3ITuple aDiv(rTupA);
        aDiv /= rTupB;
        return aDiv;
    }

    inline B3ITuple operator*(const B3ITuple& rTupA, const B3ITuple& rTupB)
    {
        B3ITuple aMul(rTupA);
        aMul *= rTupB;
        return aMul;
    }

    inline B3ITuple operator*(const B3ITuple& rTup, sal_Int32 t)
    {
        B3ITuple aNew(rTup);
        aNew *= t;
        return aNew;
    }

    inline B3ITuple operator*(sal_Int32 t, const B3ITuple& rTup)
    {
        B3ITuple aNew(rTup);
        aNew *= t;
        return aNew;
    }

    inline B3ITuple operator/(const B3ITuple& rTup, sal_Int32 t)
    {
        B3ITuple aNew(rTup);
        aNew /= t;
        return aNew;
    }

    inline B3ITuple operator/(sal_Int32 t, const B3ITuple& rTup)
    {
        B3ITuple aNew(t, t, t);
        B3ITuple aTmp(rTup);
        aNew /= aTmp;
        return aNew;
    }
} // end of namespace basegfx

#endif // INCLUDED_BASEGFX_TUPLE_B3ITUPLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
