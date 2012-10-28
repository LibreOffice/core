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

#ifndef _BGFX_TUPLE_B3ITUPLE_HXX
#define _BGFX_TUPLE_B3ITUPLE_HXX

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
        //////////////////////////////////////////////////////////////////////

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
    //////////////////////////////////////////////////////////////////////////

    inline B3ITuple minimum(const B3ITuple& rTupA, const B3ITuple& rTupB)
    {
        B3ITuple aMin(
            (rTupB.getX() < rTupA.getX()) ? rTupB.getX() : rTupA.getX(),
            (rTupB.getY() < rTupA.getY()) ? rTupB.getY() : rTupA.getY(),
            (rTupB.getZ() < rTupA.getZ()) ? rTupB.getZ() : rTupA.getZ());
        return aMin;
    }

    inline B3ITuple maximum(const B3ITuple& rTupA, const B3ITuple& rTupB)
    {
        B3ITuple aMax(
            (rTupB.getX() > rTupA.getX()) ? rTupB.getX() : rTupA.getX(),
            (rTupB.getY() > rTupA.getY()) ? rTupB.getY() : rTupA.getY(),
            (rTupB.getZ() > rTupA.getZ()) ? rTupB.getZ() : rTupA.getZ());
        return aMax;
    }

    inline B3ITuple absolute(const B3ITuple& rTup)
    {
        B3ITuple aAbs(
            (0 > rTup.getX()) ? -rTup.getX() : rTup.getX(),
            (0 > rTup.getY()) ? -rTup.getY() : rTup.getY(),
            (0 > rTup.getZ()) ? -rTup.getZ() : rTup.getZ());
        return aAbs;
    }

    inline B3DTuple interpolate(const B3ITuple& rOld1, const B3ITuple& rOld2, double t)
    {
        B3DTuple aInt(
            ((rOld2.getX() - rOld1.getX()) * t) + rOld1.getX(),
            ((rOld2.getY() - rOld1.getY()) * t) + rOld1.getY(),
            ((rOld2.getZ() - rOld1.getZ()) * t) + rOld1.getZ());
        return aInt;
    }

    inline B3DTuple average(const B3ITuple& rOld1, const B3ITuple& rOld2)
    {
        B3DTuple aAvg(
            (rOld1.getX() + rOld2.getX()) * 0.5,
            (rOld1.getY() + rOld2.getY()) * 0.5,
            (rOld1.getZ() + rOld2.getZ()) * 0.5);
        return aAvg;
    }

    inline B3DTuple average(const B3ITuple& rOld1, const B3ITuple& rOld2, const B3ITuple& rOld3)
    {
        B3DTuple aAvg(
            (rOld1.getX() + rOld2.getX() + rOld3.getX()) * (1.0 / 3.0),
            (rOld1.getY() + rOld2.getY() + rOld3.getY()) * (1.0 / 3.0),
            (rOld1.getZ() + rOld2.getZ() + rOld3.getZ()) * (1.0 / 3.0));
        return aAvg;
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

#endif /* _BGFX_TUPLE_B3ITUPLE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
