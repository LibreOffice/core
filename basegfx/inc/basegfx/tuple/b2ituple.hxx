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



#ifndef _BGFX_TUPLE_B2ITUPLE_HXX
#define _BGFX_TUPLE_B2ITUPLE_HXX

#include <sal/types.h>
#include <basegfx/numeric/ftools.hxx>
#undef min
#undef max
#include <algorithm>

namespace basegfx
{
    /** Base class for all Points/Vectors with two sal_Int32 values

        This class provides all methods common to Point
        avd Vector classes which are derived from here.

        @derive Use this class to implement Points or Vectors
        which are based on two sal_Int32 values
    */
    class BASEGFX_DLLPUBLIC B2ITuple
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

        static const B2ITuple& getEmptyTuple();
    };

    // external operators
    //////////////////////////////////////////////////////////////////////////

    inline B2ITuple minimum(const B2ITuple& rTupA, const B2ITuple& rTupB)
    {
        return B2ITuple(
            std::min(rTupB.getX(), rTupA.getX()),
            std::min(rTupB.getY(), rTupA.getY()));
    }

    inline B2ITuple maximum(const B2ITuple& rTupA, const B2ITuple& rTupB)
    {
        return B2ITuple(
            std::max(rTupB.getX(), rTupA.getX()),
            std::max(rTupB.getY(), rTupA.getY()));
    }

    inline B2ITuple absolute(const B2ITuple& rTup)
    {
        B2ITuple aAbs(
            (0 > rTup.getX()) ? -rTup.getX() : rTup.getX(),
            (0 > rTup.getY()) ? -rTup.getY() : rTup.getY());
        return aAbs;
    }

    inline B2ITuple interpolate(const B2ITuple& rOld1, const B2ITuple& rOld2, double t)
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
            return B2ITuple(
                basegfx::fround(((rOld2.getX() - rOld1.getX()) * t) + rOld1.getX()),
                basegfx::fround(((rOld2.getY() - rOld1.getY()) * t) + rOld1.getY()));
        }
    }

    inline B2ITuple average(const B2ITuple& rOld1, const B2ITuple& rOld2)
    {
        return B2ITuple(
            rOld1.getX() == rOld2.getX() ? rOld1.getX() : basegfx::fround((rOld1.getX() + rOld2.getX()) * 0.5),
            rOld1.getY() == rOld2.getY() ? rOld1.getY() : basegfx::fround((rOld1.getY() + rOld2.getY()) * 0.5));
    }

    inline B2ITuple average(const B2ITuple& rOld1, const B2ITuple& rOld2, const B2ITuple& rOld3)
    {
        return B2ITuple(
            (rOld1.getX() == rOld2.getX() && rOld2.getX() == rOld3.getX()) ? rOld1.getX() : basegfx::fround((rOld1.getX() + rOld2.getX() + rOld3.getX()) * (1.0 / 3.0)),
            (rOld1.getY() == rOld2.getY() && rOld2.getY() == rOld3.getY()) ? rOld1.getY() : basegfx::fround((rOld1.getY() + rOld2.getY() + rOld3.getY()) * (1.0 / 3.0)));
    }

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

    inline B2ITuple operator/(const B2ITuple& rTupA, const B2ITuple& rTupB)
    {
        B2ITuple aDiv(rTupA);
        aDiv /= rTupB;
        return aDiv;
    }

    inline B2ITuple operator*(const B2ITuple& rTupA, const B2ITuple& rTupB)
    {
        B2ITuple aMul(rTupA);
        aMul *= rTupB;
        return aMul;
    }

    inline B2ITuple operator*(const B2ITuple& rTup, sal_Int32 t)
    {
        B2ITuple aNew(rTup);
        aNew *= t;
        return aNew;
    }

    inline B2ITuple operator*(sal_Int32 t, const B2ITuple& rTup)
    {
        B2ITuple aNew(rTup);
        aNew *= t;
        return aNew;
    }

    inline B2ITuple operator/(const B2ITuple& rTup, sal_Int32 t)
    {
        B2ITuple aNew(rTup);
        aNew /= t;
        return aNew;
    }

    inline B2ITuple operator/(sal_Int32 t, const B2ITuple& rTup)
    {
        B2ITuple aNew(t, t);
        B2ITuple aTmp(rTup);
        aNew /= aTmp;
        return aNew;
    }
} // end of namespace basegfx

#endif /* _BGFX_TUPLE_B2ITUPLE_HXX */
