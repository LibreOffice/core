/*************************************************************************
 *
 *  $RCSfile: b2dtuple.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2003-11-05 12:25:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _BGFX_TUPLE_B2DTUPLE_HXX
#define _BGFX_TUPLE_B2DTUPLE_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _INC_MATH
#include <math.h>
#endif

namespace basegfx
{
    namespace tuple
    {
        /** Base class for all Points/Vectors with two double values

            This class provides all methods common to Point
            avd Vector classes which are derived from here.

            @derive Use this class to implement Points or Vectors
            which are based on two double values
        */
        class B2DTuple
        {
        private:
            static B2DTuple                             maEmptyTuple;

        protected:
            double                                      mfX;
            double                                      mfY;

        public:
            /** Create a 2D Tuple

                @param fVal
                This parameter is used to initialize the coordinate
                part of the 2D Tuple.
            */
            B2DTuple(double fVal = 0.0)
            :   mfX(fVal),
                mfY(fVal)
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
            :   mfX( fX ),
                mfY( fY )
            {}

            /** Create a copy of a 2D Tuple

                @param rTup
                The 2D Tuple which will be copied.
            */
            B2DTuple(const B2DTuple& rTup)
            :   mfX( rTup.mfX ),
                mfY( rTup.mfY )
            {}

            ~B2DTuple()
            {}

            /// Get X-Coordinate of 2D Tuple
            double getX() const
            {
                return mfX;
            }

            /// Get Y-Coordinate of 2D Tuple
            double getY() const
            {
                return mfY;
            }

            /// Set X-Coordinate of 2D Tuple
            void setX(double fX)
            {
                mfX = fX;
            }

            /// Set Y-Coordinate of 2D Tuple
            void setY(double fY)
            {
                mfY = fY;
            }

            /// Array-access to 2D Tuple
            const double& operator[] (int nPos) const
            {
                // Here, normally one if(...) should be used. In the assumption that
                // both double members can be accessed as an array a shortcut is used here.
                // if(0 == nPos) return mfX; return mfY;
                return *((&mfX) + nPos);
            }

            /// Array-access to 2D Tuple
            double& operator[] (int nPos)
            {
                // Here, normally one if(...) should be used. In the assumption that
                // both double members can be accessed as an array a shortcut is used here.
                // if(0 == nPos) return mfX; return mfY;
                return *((&mfX) + nPos);
            }

            // comparators with tolerance
            //////////////////////////////////////////////////////////////////////

            sal_Bool equalZero() const;

            sal_Bool equalZero(const double& rfSmallValue) const;

            sal_Bool equal(const B2DTuple& rTup) const;

            sal_Bool equal(const B2DTuple& rTup, const double& rfSmallValue) const;

            // operators
            //////////////////////////////////////////////////////////////////////

            B2DTuple& operator+=( const B2DTuple& rTup )
            {
                mfX += rTup.mfX;
                mfY += rTup.mfY;
                return *this;
            }

            B2DTuple& operator-=( const B2DTuple& rTup )
            {
                mfX -= rTup.mfX;
                mfY -= rTup.mfY;
                return *this;
            }

            B2DTuple& operator/=( const B2DTuple& rTup )
            {
                mfX /= rTup.mfX;
                mfY /= rTup.mfY;
                return *this;
            }

            B2DTuple& operator*=( const B2DTuple& rTup )
            {
                mfX *= rTup.mfX;
                mfY *= rTup.mfY;
                return *this;
            }

            B2DTuple& operator*=(double t)
            {
                mfX *= t;
                mfY *= t;
                return *this;
            }

            B2DTuple& operator/=(double t)
            {
                const double fVal(1.0 / t);
                mfX *= fVal;
                mfY *= fVal;
                return *this;
            }

            B2DTuple operator-(void) const
            {
                return B2DTuple(-mfX, -mfY);
            }

            sal_Bool operator==( const B2DTuple& rTup ) const
            {
                return equal(rTup);
            }

            sal_Bool operator!=( const B2DTuple& rTup ) const
            {
                return !equal(rTup);
            }

            B2DTuple& operator=( const B2DTuple& rTup )
            {
                mfX = rTup.mfX;
                mfY = rTup.mfY;
                return *this;
            }

            void correctValues(const double fCompareValue = 0.0);

            static const B2DTuple& getEmptyTuple()
            {
                return maEmptyTuple;
            }
        };

        // external operators
        //////////////////////////////////////////////////////////////////////////

        inline B2DTuple min(const B2DTuple& rTupA, const B2DTuple& rTupB)
        {
            B2DTuple aMin(
                (rTupB.getX() < rTupA.getX()) ? rTupB.getX() : rTupA.getX(),
                (rTupB.getY() < rTupA.getY()) ? rTupB.getY() : rTupA.getY());
            return aMin;
        }

        inline B2DTuple max(const B2DTuple& rTupA, const B2DTuple& rTupB)
        {
            B2DTuple aMax(
                (rTupB.getX() > rTupA.getX()) ? rTupB.getX() : rTupA.getX(),
                (rTupB.getY() > rTupA.getY()) ? rTupB.getY() : rTupA.getY());
            return aMax;
        }

        inline B2DTuple abs(const B2DTuple& rTup)
        {
            B2DTuple aAbs(
                (0.0 > rTup.getX()) ? -rTup.getX() : rTup.getX(),
                (0.0 > rTup.getY()) ? -rTup.getY() : rTup.getY());
            return aAbs;
        }

        inline B2DTuple interpolate(const B2DTuple& rOld1, const B2DTuple& rOld2, double t)
        {
            B2DTuple aInt(
                ((rOld2.getX() - rOld1.getX()) + t) + rOld1.getX(),
                ((rOld2.getY() - rOld1.getY()) + t) + rOld1.getY());
            return aInt;
        }

        inline B2DTuple average(const B2DTuple& rOld1, const B2DTuple& rOld2)
        {
            B2DTuple aAvg(
                (rOld1.getX() + rOld2.getX()) * 0.5,
                (rOld1.getY() + rOld2.getY()) * 0.5);
            return aAvg;
        }

        inline B2DTuple average(const B2DTuple& rOld1, const B2DTuple& rOld2, const B2DTuple& rOld3)
        {
            B2DTuple aAvg(
                (rOld1.getX() + rOld2.getX() + rOld3.getX()) * (1.0 / 3.0),
                (rOld1.getY() + rOld2.getY() + rOld3.getY()) * (1.0 / 3.0));
            return aAvg;
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

        inline B2DTuple operator/(double t, const B2DTuple& rTup)
        {
            B2DTuple aNew(rTup);
            aNew /= t;
            return aNew;
        }
    } // end of namespace tuple
} // end of namespace basegfx

#endif //  _BGFX_TUPLE_B2DTUPLE_HXX
