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

#ifndef INCLUDED_BASEGFX_TUPLE_B2ITUPLE_HXX
#define INCLUDED_BASEGFX_TUPLE_B2ITUPLE_HXX

#include <sal/types.h>
#include <basegfx/numeric/ftools.hxx>
#include <algorithm>
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

        bool equalZero() const
        {
            return mnX == 0 && mnY == 0;
        }

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



    inline B2ITuple operator+(const B2ITuple& rTupA, const B2ITuple& rTupB)
    {
        B2ITuple aSum(rTupA);
        aSum += rTupB;
        return aSum;
    }

    inline B2ITuple operator-(const B2ITuple& rTupA, const B2ITuple& rTupB)
    {
        B2ITuple aSub(rTupA);
        aSub -= rTupB;
        return aSub;
    }

    inline B2ITuple operator*(sal_Int32 t, const B2ITuple& rTup)
    {
        B2ITuple aNew(rTup);
        aNew *= t;
        return aNew;
    }

} // end of namespace basegfx

#endif // INCLUDED_BASEGFX_TUPLE_B2ITUPLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
