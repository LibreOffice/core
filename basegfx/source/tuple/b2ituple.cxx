/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basegfx.hxx"
#include <basegfx/tuple/b2ituple.hxx>
#include <basegfx/tuple/b2dtuple.hxx>
#include <rtl/instance.hxx>

namespace { struct EmptyTuple : public rtl::Static<basegfx::B2ITuple, EmptyTuple> {}; }

namespace basegfx
{
    const B2ITuple& B2ITuple::getEmptyTuple()
    {
            return EmptyTuple::get();
    }

    // external operators
    //////////////////////////////////////////////////////////////////////////

    B2ITuple minimum(const B2ITuple& rTupA, const B2ITuple& rTupB)
    {
        B2ITuple aMin(
            (rTupB.getX() < rTupA.getX()) ? rTupB.getX() : rTupA.getX(),
            (rTupB.getY() < rTupA.getY()) ? rTupB.getY() : rTupA.getY());
        return aMin;
    }

    B2ITuple maximum(const B2ITuple& rTupA, const B2ITuple& rTupB)
    {
        B2ITuple aMax(
            (rTupB.getX() > rTupA.getX()) ? rTupB.getX() : rTupA.getX(),
            (rTupB.getY() > rTupA.getY()) ? rTupB.getY() : rTupA.getY());
        return aMax;
    }

    B2ITuple absolute(const B2ITuple& rTup)
    {
        B2ITuple aAbs(
            (0 > rTup.getX()) ? -rTup.getX() : rTup.getX(),
            (0 > rTup.getY()) ? -rTup.getY() : rTup.getY());
        return aAbs;
    }

    B2DTuple interpolate(const B2ITuple& rOld1, const B2ITuple& rOld2, double t)
    {
        B2DTuple aInt(
            ((rOld2.getX() - rOld1.getX()) * t) + rOld1.getX(),
            ((rOld2.getY() - rOld1.getY()) * t) + rOld1.getY());
        return aInt;
    }

    B2DTuple average(const B2ITuple& rOld1, const B2ITuple& rOld2)
    {
        B2DTuple aAvg(
            (rOld1.getX() + rOld2.getX()) * 0.5,
            (rOld1.getY() + rOld2.getY()) * 0.5);
        return aAvg;
    }

    B2DTuple average(const B2ITuple& rOld1, const B2ITuple& rOld2, const B2ITuple& rOld3)
    {
        B2DTuple aAvg(
            (rOld1.getX() + rOld2.getX() + rOld3.getX()) * (1.0 / 3.0),
            (rOld1.getY() + rOld2.getY() + rOld3.getY()) * (1.0 / 3.0));
        return aAvg;
    }

    B2ITuple operator+(const B2ITuple& rTupA, const B2ITuple& rTupB)
    {
        B2ITuple aSum(rTupA);
        aSum += rTupB;
        return aSum;
    }

    B2ITuple operator-(const B2ITuple& rTupA, const B2ITuple& rTupB)
    {
        B2ITuple aSub(rTupA);
        aSub -= rTupB;
        return aSub;
    }

    B2ITuple operator/(const B2ITuple& rTupA, const B2ITuple& rTupB)
    {
        B2ITuple aDiv(rTupA);
        aDiv /= rTupB;
        return aDiv;
    }

    B2ITuple operator*(const B2ITuple& rTupA, const B2ITuple& rTupB)
    {
        B2ITuple aMul(rTupA);
        aMul *= rTupB;
        return aMul;
    }

    B2ITuple operator*(const B2ITuple& rTup, sal_Int32 t)
    {
        B2ITuple aNew(rTup);
        aNew *= t;
        return aNew;
    }

    B2ITuple operator*(sal_Int32 t, const B2ITuple& rTup)
    {
        B2ITuple aNew(rTup);
        aNew *= t;
        return aNew;
    }

    B2ITuple operator/(const B2ITuple& rTup, sal_Int32 t)
    {
        B2ITuple aNew(rTup);
        aNew /= t;
        return aNew;
    }

    B2ITuple operator/(sal_Int32 t, const B2ITuple& rTup)
    {
        B2ITuple aNew(t, t);
        B2ITuple aTmp(rTup);
        aNew /= aTmp;
        return aNew;
    }

} // end of namespace basegfx

// eof
