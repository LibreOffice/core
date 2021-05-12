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
#include <basegfx/tuple/Tuple2D.hxx>

namespace basegfx
{
    /** Base class for all Points/Vectors with two sal_Int32 values

        This class provides all methods common to Point
        and Vector classes which are derived from here.

        @derive Use this class to implement Points or Vectors
        which are based on two sal_Int32 values
    */
    class SAL_WARN_UNUSED BASEGFX_DLLPUBLIC B2ITuple : public Tuple2D<sal_Int32>
    {
    public:
        /** Create a 2D Tuple

            The tuple is initialized to (0, 0)
        */
        B2ITuple()
            : Tuple2D(0, 0)
        {}

        /** Create a 2D Tuple

            @param nX
            This parameter is used to initialize the X-coordinate
            of the 2D Tuple.

            @param nY
            This parameter is used to initialize the Y-coordinate
            of the 2D Tuple.
        */
        B2ITuple(sal_Int32 nX, sal_Int32 nY)
            : Tuple2D(nX, nY)
        {}

        // operators

        B2ITuple operator-(void) const
        {
            return B2ITuple(-mnX, -mnY);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
