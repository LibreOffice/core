/*************************************************************************
 *
 *  $RCSfile: b3dtuple.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2003-10-31 10:06:25 $
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

#ifndef _BGFX_TUPLE_B3DTUPLE_HXX
#define _BGFX_TUPLE_B3DTUPLE_HXX

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

namespace basegfx
{
    namespace tuple
    {
        /** Base class for all Points/Vectors with three double values

            This class provides all methods common to Point
            avd Vector classes which are derived from here.

            @derive Use this class to implement Points or Vectors
            which are based on three double values
        */
        class B3DTuple
        {
        private:
            static B3DTuple                             maEmptyTuple;

        protected:
            double                                      mfX;
            double                                      mfY;
            double                                      mfZ;

        public:
            /** Create a 3D Tuple

                @param fVal
                This parameter is used to initialize the coordinate
                part of the 3D Tuple.
            */
            B3DTuple(double fVal = 0.0)
            :   mfX(fVal),
                mfY(fVal),
                mfZ(fVal)
            {}

            /** Create a 3D Tuple

                @param fX
                This parameter is used to initialize the X-coordinate
                of the 3D Tuple.

                @param fY
                This parameter is used to initialize the Y-coordinate
                of the 3D Tuple.

                @param fZ
                This parameter is used to initialize the Z-coordinate
                of the 3D Tuple.
            */
            B3DTuple(double fX, double fY, double fZ)
            :   mfX(fX),
                mfY(fY),
                mfZ(fZ)
            {}

            /** Create a copy of a 3D Tuple

                @param rTup
                The 3D Tuple which will be copied.
            */
            B3DTuple(const B3DTuple& rTup)
            :   mfX( rTup.mfX ),
                mfY( rTup.mfY ),
                mfZ( rTup.mfZ )
            {}

            ~B3DTuple()
            {}

            /// get X-Coordinate of 3D Tuple
            double getX() const
            {
                return mfX;
            }

            /// get Y-Coordinate of 3D Tuple
            double getY() const
            {
                return mfY;
            }

            /// get Z-Coordinate of 3D Tuple
            double getZ() const
            {
                return mfZ;
            }

            /// set X-Coordinate of 3D Tuple
            void setX(double fX)
            {
                mfX = fX;
            }

            /// set Y-Coordinate of 3D Tuple
            void setY(double fY)
            {
                mfY = fY;
            }

            /// set Z-Coordinate of 3D Tuple
            void setZ(double fZ)
            {
                mfZ = fZ;
            }

            /// Array-access to 3D Tuple
            const double& operator[] (int nPos) const
            {
                // Here, normally two if(...)'s should be used. In the assumption that
                // both double members can be accessed as an array a shortcut is used here.
                // if(0 == nPos) return mfX; if(1 == nPos) return mfY; return mfZ;
                return *((&mfX) + nPos);
            }

            /// Array-access to 3D Tuple
            double& operator[] (int nPos)
            {
                // Here, normally two if(...)'s should be used. In the assumption that
                // both double members can be accessed as an array a shortcut is used here.
                // if(0 == nPos) return mfX; if(1 == nPos) return mfY; return mfZ;
                return *((&mfX) + nPos);
            }

            // comparators with tolerance
            //////////////////////////////////////////////////////////////////////

            bool EqualZero() const
            {
                return (this == &maEmptyTuple ||
                    (::basegfx::numeric::fTools::equalZero(mfX) && ::basegfx::numeric::fTools::equalZero(mfY) && ::basegfx::numeric::fTools::equalZero(mfZ)));
            }

            bool EqualZero(const double& rfSmallValue) const
            {
                return (this == &maEmptyTuple ||
                    (::basegfx::numeric::fTools::equalZero(mfX, rfSmallValue)
                    && ::basegfx::numeric::fTools::equalZero(mfY, rfSmallValue)
                    && ::basegfx::numeric::fTools::equalZero(mfZ, rfSmallValue)));
            }

            bool Equal(const B3DTuple& rTup) const
            {
                return (
                    ::basegfx::numeric::fTools::equal(mfX, rTup.mfX) &&
                    ::basegfx::numeric::fTools::equal(mfY, rTup.mfY) &&
                    ::basegfx::numeric::fTools::equal(mfZ, rTup.mfZ));
            }

            bool Equal(const B3DTuple& rTup, const double& rfSmallValue) const
            {
                return (
                    ::basegfx::numeric::fTools::equal(mfX, rTup.mfX, rfSmallValue) &&
                    ::basegfx::numeric::fTools::equal(mfY, rTup.mfY, rfSmallValue) &&
                    ::basegfx::numeric::fTools::equal(mfZ, rTup.mfZ, rfSmallValue));
            }

            // operators
            //////////////////////////////////////////////////////////////////////

            B3DTuple& operator+=( const B3DTuple& rTup )
            {
                mfX += rTup.mfX;
                mfY += rTup.mfY;
                mfZ += rTup.mfZ;
                return *this;
            }

            B3DTuple& operator-=( const B3DTuple& rTup )
            {
                mfX -= rTup.mfX;
                mfY -= rTup.mfY;
                mfZ -= rTup.mfZ;
                return *this;
            }

            B3DTuple& operator/=( const B3DTuple& rTup )
            {
                mfX /= rTup.mfX;
                mfY /= rTup.mfY;
                mfZ /= rTup.mfZ;
                return *this;
            }

            B3DTuple& operator*=( const B3DTuple& rTup )
            {
                mfX *= rTup.mfX;
                mfY *= rTup.mfY;
                mfZ *= rTup.mfZ;
                return *this;
            }

            B3DTuple& operator*=(double t)
            {
                mfX *= t;
                mfY *= t;
                mfZ *= t;
                return *this;
            }

            B3DTuple& operator/=(double t)
            {
                const double fVal(1.0 / t);
                mfX *= t;
                mfY *= t;
                mfZ *= t;
                return *this;
            }

            B3DTuple operator-(void) const
            {
                return B3DTuple(-mfX, -mfY, -mfZ);
            }

            bool operator==( const B3DTuple& rTup ) const
            {
                return Equal(rTup);
            }

            bool operator!=( const B3DTuple& rTup ) const
            {
                return !Equal(rTup);
            }

            B3DTuple& operator=( const B3DTuple& rTup )
            {
                mfX = rTup.mfX;
                mfY = rTup.mfY;
                mfZ = rTup.mfZ;
                return *this;
            }

            void correctValues(const double fCompareValue = 0.0)
            {
                if(0.0 == fCompareValue)
                {
                    if(::basegfx::numeric::fTools::equalZero(mfX))
                    {
                        mfX = 0.0;
                    }

                    if(::basegfx::numeric::fTools::equalZero(mfY))
                    {
                        mfY = 0.0;
                    }

                    if(::basegfx::numeric::fTools::equalZero(mfZ))
                    {
                        mfZ = 0.0;
                    }
                }
                else
                {
                    if(::basegfx::numeric::fTools::equal(mfX, fCompareValue))
                    {
                        mfX = fCompareValue;
                    }

                    if(::basegfx::numeric::fTools::equal(mfY, fCompareValue))
                    {
                        mfY = fCompareValue;
                    }

                    if(::basegfx::numeric::fTools::equal(mfZ, fCompareValue))
                    {
                        mfZ = fCompareValue;
                    }
                }
            }

            static const B3DTuple& getEmptyTuple()
            {
                return maEmptyTuple;
            }
        };

        // external operators
        //////////////////////////////////////////////////////////////////////////

        inline B3DTuple min(const B3DTuple& rTupA, const B3DTuple& rTupB)
        {
            B3DTuple aMin(
                (rTupB.getX() < rTupA.getX()) ? rTupB.getX() : rTupA.getX(),
                (rTupB.getY() < rTupA.getY()) ? rTupB.getY() : rTupA.getY(),
                (rTupB.getZ() < rTupA.getZ()) ? rTupB.getZ() : rTupA.getZ());
            return aMin;
        }

        inline B3DTuple max(const B3DTuple& rTupA, const B3DTuple& rTupB)
        {
            B3DTuple aMax(
                (rTupB.getX() > rTupA.getX()) ? rTupB.getX() : rTupA.getX(),
                (rTupB.getY() > rTupA.getY()) ? rTupB.getY() : rTupA.getY(),
                (rTupB.getZ() > rTupA.getZ()) ? rTupB.getZ() : rTupA.getZ());
            return aMax;
        }

        inline B3DTuple abs(const B3DTuple& rTup)
        {
            B3DTuple aAbs(
                (0.0 > rTup.getX()) ? -rTup.getX() : rTup.getX(),
                (0.0 > rTup.getY()) ? -rTup.getY() : rTup.getY(),
                (0.0 > rTup.getZ()) ? -rTup.getZ() : rTup.getZ());
            return aAbs;
        }

        inline B3DTuple interpolate(const B3DTuple& rOld1, const B3DTuple& rOld2, double t)
        {
            B3DTuple aInt(
                ((rOld2.getX() - rOld1.getX()) + t) + rOld1.getX(),
                ((rOld2.getY() - rOld1.getY()) + t) + rOld1.getY(),
                ((rOld2.getZ() - rOld1.getZ()) + t) + rOld1.getZ());
            return aInt;
        }

        inline B3DTuple average(const B3DTuple& rOld1, const B3DTuple& rOld2)
        {
            B3DTuple aAvg(
                (rOld1.getX() + rOld2.getX()) * 0.5,
                (rOld1.getY() + rOld2.getY()) * 0.5,
                (rOld1.getZ() + rOld2.getZ()) * 0.5);
            return aAvg;
        }

        inline B3DTuple average(const B3DTuple& rOld1, const B3DTuple& rOld2, const B3DTuple& rOld3)
        {
            B3DTuple aAvg(
                (rOld1.getX() + rOld2.getX() + rOld3.getX()) * (1.0 / 3.0),
                (rOld1.getY() + rOld2.getY() + rOld3.getY()) * (1.0 / 3.0),
                (rOld1.getZ() + rOld2.getZ() + rOld3.getZ()) * (1.0 / 3.0));
            return aAvg;
        }

        inline B3DTuple operator+(const B3DTuple& rTupA, const B3DTuple& rTupB)
        {
            B3DTuple aSum(rTupA);
            aSum += rTupB;
            return aSum;
        }

        inline B3DTuple operator-(const B3DTuple& rTupA, const B3DTuple& rTupB)
        {
            B3DTuple aSub(rTupA);
            aSub -= rTupB;
            return aSub;
        }

        inline B3DTuple operator/(const B3DTuple& rTupA, const B3DTuple& rTupB)
        {
            B3DTuple aDiv(rTupA);
            aDiv /= rTupB;
            return aDiv;
        }

        inline B3DTuple operator*(const B3DTuple& rTupA, const B3DTuple& rTupB)
        {
            B3DTuple aMul(rTupA);
            aMul *= rTupB;
            return aMul;
        }

        inline B3DTuple operator*(const B3DTuple& rTup, double t)
        {
            B3DTuple aNew(rTup);
            aNew *= t;
            return aNew;
        }

        inline B3DTuple operator*(double t, const B3DTuple& rTup)
        {
            B3DTuple aNew(rTup);
            aNew *= t;
            return aNew;
        }

        inline B3DTuple operator/(const B3DTuple& rTup, double t)
        {
            B3DTuple aNew(rTup);
            aNew /= t;
            return aNew;
        }

        inline B3DTuple operator/(double t, const B3DTuple& rTup)
        {
            B3DTuple aNew(rTup);
            aNew /= t;
            return aNew;
        }
    } // end of namespace tuple
} // end of namespace basegfx

#endif // _BGFX_TUPLE_B3DTUPLE_HXX
