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
    class B2ITuple;

    /** Base class for all Points/Vectors with two double values

        This class provides all methods common to Point
        and Vector classes which are derived from here.

        @derive Use this class to implement Points or Vectors
        which are based on two double values
    */
    class SAL_WARN_UNUSED B2DTuple : public Tuple2D<double>
    {
    public:

        /** Create a 2D Tuple

            The tuple is initialized to (0.0, 0.0)
        */
        B2DTuple()
         : Tuple2D(0.0, 0.0)
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
            : Tuple2D(fX, fY)
        {}

        /** Create a copy of a 2D integer Tuple

            @param rTup
            The 2D Tuple which will be copied.
        */
        BASEGFX_DLLPUBLIC explicit B2DTuple(const B2ITuple& rTup);

        // operators

        B2DTuple operator-(void) const
        {
            return B2DTuple(-mfX, -mfY);
        }

        BASEGFX_DLLPUBLIC static const B2DTuple& getEmptyTuple();
    };

    // external operators


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
            rtl_math_approxEqual(rOld1.getX(), rOld2.getX()) ? rOld1.getX() : (rOld1.getX() + rOld2.getX()) * 0.5,
            rtl_math_approxEqual(rOld1.getY(), rOld2.getY()) ? rOld1.getY() : (rOld1.getY() + rOld2.getY()) * 0.5);
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

    /** Round double to nearest integer for 2D tuple

        @return the nearest integer for this tuple
    */
    BASEGFX_DLLPUBLIC B2ITuple fround(const B2DTuple& rTup);
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
