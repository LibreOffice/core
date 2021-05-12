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

#pragma once

#include <sal/types.h>
#include <basegfx/basegfxdllapi.h>
#include <basegfx/tuple/Tuple3D.hxx>

namespace basegfx
{
    /** Base class for all Points/Vectors with three sal_Int32 values

        This class provides all methods common to Point
        and Vector classes which are derived from here.

        @derive Use this class to implement Points or Vectors
        which are based on three sal_Int32 values
    */
    class SAL_WARN_UNUSED BASEGFX_DLLPUBLIC B3ITuple : public Tuple3D<sal_Int32>
    {
    public:
        /** Create a 3D Tuple

            The tuple is initialized to (0, 0, 0)
        */
        B3ITuple()
            : Tuple3D(0, 0, 0)
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
            : Tuple3D(nX, nY, nZ)
        {}

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

        B3ITuple operator-(void) const
        {
            return B3ITuple(-mnX, -mnY, -mnZ);
        }
    };

} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
