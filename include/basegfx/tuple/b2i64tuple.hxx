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

} // end of namespace basegfx

#endif // INCLUDED_BASEGFX_TUPLE_B2I64TUPLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
