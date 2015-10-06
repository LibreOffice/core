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

} // end of namespace basegfx

#endif // INCLUDED_BASEGFX_TUPLE_B3ITUPLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
