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

#ifndef _BGFX_TUPLE_B2ITUPLE_HXX
#define _BGFX_TUPLE_B2ITUPLE_HXX

#include <sal/types.h>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
    /** Base class for all Points/Vectors with two sal_Int32 values

        This class provides all methods common to Point
        avd Vector classes which are derived from here.

        @derive Use this class to implement Points or Vectors
        which are based on two sal_Int32 values
    */
    class BASEGFX_DLLPUBLIC SAL_WARN_UNUSED B2ITuple
    {
    protected:
        sal_Int32                                       mnX;
        sal_Int32                                       mnY;

    public:
        /** Create a 2D Tuple

            The tuple is initialized to (0, 0)
        */
        B2ITuple()
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
        B2ITuple(sal_Int32 fX, sal_Int32 fY)
        :   mnX( fX ),
            mnY( fY )
        {}

        /** Create a copy of a 2D Tuple

            @param rTup
            The 2D Tuple which will be copied.
        */
        B2ITuple(const B2ITuple& rTup)
        :   mnX( rTup.mnX ),
            mnY( rTup.mnY )
        {}

        ~B2ITuple()
        {}

        /// Get X-Coordinate of 2D Tuple
        sal_Int32 getX() const
        {
            return mnX;
        }

        /// Get Y-Coordinate of 2D Tuple
        sal_Int32 getY() const
        {
            return mnY;
        }

        /// Set X-Coordinate of 2D Tuple
        void setX(sal_Int32 fX)
        {
            mnX = fX;
        }

        /// Set Y-Coordinate of 2D Tuple
        void setY(sal_Int32 fY)
        {
            mnY = fY;
        }

        /// Array-access to 2D Tuple
        const sal_Int32& operator[] (int nPos) const
        {
            // Here, normally one if(...) should be used. In the assumption that
            // both sal_Int32 members can be accessed as an array a shortcut is used here.
            // if(0 == nPos) return mnX; return mnY;
            return *((&mnX) + nPos);
        }

        /// Array-access to 2D Tuple
        sal_Int32& operator[] (int nPos)
        {
            // Here, normally one if(...) should be used. In the assumption that
            // both sal_Int32 members can be accessed as an array a shortcut is used here.
            // if(0 == nPos) return mnX; return mnY;
            return *((&mnX) + nPos);
        }

        // operators
        //////////////////////////////////////////////////////////////////////

        B2ITuple& operator+=( const B2ITuple& rTup )
        {
            mnX += rTup.mnX;
            mnY += rTup.mnY;
            return *this;
        }

        B2ITuple& operator-=( const B2ITuple& rTup )
        {
            mnX -= rTup.mnX;
            mnY -= rTup.mnY;
            return *this;
        }

        B2ITuple& operator/=( const B2ITuple& rTup )
        {
            mnX /= rTup.mnX;
            mnY /= rTup.mnY;
            return *this;
        }

        B2ITuple& operator*=( const B2ITuple& rTup )
        {
            mnX *= rTup.mnX;
            mnY *= rTup.mnY;
            return *this;
        }

        B2ITuple& operator*=(sal_Int32 t)
        {
            mnX *= t;
            mnY *= t;
            return *this;
        }

        B2ITuple& operator/=(sal_Int32 t)
        {
            mnX /= t;
            mnY /= t;
            return *this;
        }

        B2ITuple operator-(void) const
        {
            return B2ITuple(-mnX, -mnY);
        }

        bool equalZero() const { return mnX == 0 && mnY == 0; }

        bool operator==( const B2ITuple& rTup ) const
        {
            return this == &rTup || (rTup.mnX == mnX && rTup.mnY == mnY);
        }

        bool operator!=( const B2ITuple& rTup ) const
        {
            return !(*this == rTup);
        }

        B2ITuple& operator=( const B2ITuple& rTup )
        {
            mnX = rTup.mnX;
            mnY = rTup.mnY;
            return *this;
        }
    };

    // external operators
    //////////////////////////////////////////////////////////////////////////
    class B2DTuple;

    BASEGFX_DLLPUBLIC B2ITuple operator+(const B2ITuple& rTupA, const B2ITuple& rTupB);
    BASEGFX_DLLPUBLIC B2ITuple operator-(const B2ITuple& rTupA, const B2ITuple& rTupB);
    BASEGFX_DLLPUBLIC B2ITuple operator/(const B2ITuple& rTupA, const B2ITuple& rTupB);
    BASEGFX_DLLPUBLIC B2ITuple operator*(const B2ITuple& rTupA, const B2ITuple& rTupB);
    BASEGFX_DLLPUBLIC B2ITuple operator*(const B2ITuple& rTup, sal_Int32 t);
    BASEGFX_DLLPUBLIC B2ITuple operator*(sal_Int32 t, const B2ITuple& rTup);
    BASEGFX_DLLPUBLIC B2ITuple operator/(const B2ITuple& rTup, sal_Int32 t);
    BASEGFX_DLLPUBLIC B2ITuple operator/(sal_Int32 t, const B2ITuple& rTup);
} // end of namespace basegfx

#endif /* _BGFX_TUPLE_B2ITUPLE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
