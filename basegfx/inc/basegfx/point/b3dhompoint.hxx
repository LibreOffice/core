/*************************************************************************
 *
 *  $RCSfile: b3dhompoint.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2003-11-05 12:25:38 $
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

#ifndef _BGFX_POINT_B3DHOMPOINT_HXX
#define _BGFX_POINT_B3DHOMPOINT_HXX

#ifndef _BGFX_POINT_B3DPOINT_HXX
#include <basegfx/point/b3dpoint.hxx>
#endif

namespace basegfx
{
    namespace point
    {
        /** Basic homogen Point class with three double values and one homogen factor

            This class provides access to homogen coordinates in 3D.
            For this purpose all the operators which need to do specific
            action due to their homogenity are implemented here.
            The only caveat are member methods which are declared as const
            but do change the content. These are documented for that reason.
            The class is designed to provide homogenous coordinates without
            direct access to the homogen part (mfW). This is also the reason
            for leaving out the [] operators which return references to members.

            @see B3DTuple
        */
        class B3DHomPoint
        {
        protected:
            /// This member contains the coordinate part of the point
            tuple::B3DTuple                             maTuple;

            /// This Member holds the homogenous part of the point
            double                                      mfW;

            /** Test if this homogen point does have a homogenous part

                @return Returns sal_True if this point has no homogenous part
            */
            sal_Bool implIsHomogenized() const
            {
                const double fOne(1.0);
                return ::basegfx::numeric::fTools::equal(mfW, fOne);
            }

            /** Remove homogenous part of this Point

                This method does necessary calculations to remove
                the evtl. homogenous part of this Point. This may
                change all members.
            */
            void implHomogenize();

            /** Test and on demand remove homogenous part

                This method tests if this Point does have a homogenous part
                and then evtl. takes actions to remove that part.

                @attention Even when this method is const it may change all
                members of this instance. This is due to the fact that changing
                the homogenous part of a homogenous point does from a mathematical
                point of view not change the point at all.
            */
            void implTestAndHomogenize() const
            {
                if(!implIsHomogenized())
                    ((B3DHomPoint*)this)->implHomogenize();
            }

        public:
            /** Create a homogen point

                @param fVal
                This parameter is used to initialize the coordinate
                part of the Point. The homogenous part is initialized to 1.0.
            */
            B3DHomPoint(double fVal = 0.0)
            :   maTuple(fVal),
                mfW(1.0)
            {}

            /** Create a homogen point

                @param fX
                This parameter is used to initialize the X-coordinate
                of the Point. The homogenous part is initialized to 1.0.

                @param fY
                This parameter is used to initialize the Y-coordinate
                of the Point. The homogenous part is initialized to 1.0.

                @param fZ
                This parameter is used to initialize the Z-coordinate
                of the Point. The homogenous part is initialized to 1.0.
            */
            B3DHomPoint(double fX, double fY, double fZ)
            :   maTuple(fX, fY, fZ),
                mfW(1.0)
            {}

            /** Create a copy of a 3D Point

                @param rVec
                The 3D point which will be copied. The homogenous part
                is initialized to 1.0.
            */
            B3DHomPoint(const B3DPoint& rVec)
            :   maTuple(rVec),
                mfW(1.0)
            {}

            /** Create a copy of a homogen point

                @param rVec
                The homogen point which will be copied. The homogenous part
                is copied, too.
            */
            B3DHomPoint(const B3DHomPoint& rVec)
            :   maTuple(rVec.maTuple.getX(), rVec.maTuple.getY(), rVec.maTuple.getZ()),
                mfW(rVec.mfW)
            {}

            ~B3DHomPoint()
            {}

            /** get a 3D point from this homogenous point

                This method normalizes this homogen point if necessary and
                returns the corresponding 3D point for this homogen point.

                @attention Even when this method is const it may change all
                members of this instance.
            */
            B3DPoint getB3DPoint() const
            {
                implTestAndHomogenize();
                return B3DPoint(maTuple.getX(), maTuple.getY(), maTuple.getZ());
            }

            /** get X-coordinate

                This method normalizes this homogen point if necessary and
                returns the corresponding X-coordinate for this homogen point.

                @attention Even when this method is const it may change all
                members of this instance.
            */
            double getX() const
            {
                implTestAndHomogenize();
                return maTuple.getX();
            }

            /** get Y-coordinate

                This method normalizes this homogen point if necessary and
                returns the corresponding Y-coordinate for this homogen point.

                @attention Even when this method is const it may change all
                members of this instance.
            */
            double getY() const
            {
                implTestAndHomogenize();
                return maTuple.getY();
            }

            /** get Z-coordinate

                This method normalizes this homogen point if necessary and
                returns the corresponding Z-coordinate for this homogen point.

                @attention Even when this method is const it may change all
                members of this instance.
            */
            double getZ() const
            {
                implTestAndHomogenize();
                return maTuple.getY();
            }

            /** Set X-coordinate of the homogen point.

                This method sets the X-coordinate of the homogen point. If
                the point does have a homogenous part this is taken into account.

                @param fX
                The to-be-set X-coordinate without homogenous part.
            */
            void setX(double fX)
            {
                maTuple.setX(implIsHomogenized() ? fX : fX * mfW );
            }

            /** Set Y-coordinate of the homogen point.

                This method sets the Y-coordinate of the homogen point. If
                the point does have a homogenous part this is taken into account.

                @param fY
                The to-be-set Y-coordinate without homogenous part.
            */
            void setY(double fY)
            {
                maTuple.setY(implIsHomogenized() ? fY : fY * mfW );
            }

            /** Set Z-coordinate of the homogen point.

                This method sets the Z-coordinate of the homogen point. If
                the point does have a homogenous part this is taken into account.

                @param fZ
                The to-be-set Z-coordinate without homogenous part.
            */
            void setZ(double fZ)
            {
                maTuple.setZ(implIsHomogenized() ? fZ : fZ * mfW );
            }

            // operators
            //////////////////////////////////////////////////////////////////////

            B3DHomPoint& operator+=( const B3DHomPoint& rPnt )
            {
                maTuple.setX(getX() * rPnt.mfW + rPnt.getX() * mfW);
                maTuple.setY(getY() * rPnt.mfW + rPnt.getY() * mfW);
                maTuple.setZ(getZ() * rPnt.mfW + rPnt.getZ() * mfW);
                mfW = mfW * rPnt.mfW;

                return *this;
            }

            B3DHomPoint& operator-=( const B3DHomPoint& rPnt )
            {
                maTuple.setX(getX() * rPnt.mfW - rPnt.getX() * mfW);
                maTuple.setY(getY() * rPnt.mfW - rPnt.getY() * mfW);
                maTuple.setZ(getZ() * rPnt.mfW - rPnt.getZ() * mfW);
                mfW = mfW * rPnt.mfW;

                return *this;
            }

            B3DHomPoint& operator*=(double t)
            {
                if(!::basegfx::numeric::fTools::equalZero(t))
                {
                    mfW /= t;
                }

                return *this;
            }

            B3DHomPoint& operator/=(double t)
            {
                mfW *= t;
                return *this;
            }

            B3DHomPoint& operator-(void)
            {
                mfW = -mfW;
                return *this;
            }

            sal_Bool operator==( const B3DHomPoint& rPnt ) const
            {
                implTestAndHomogenize();
                return (maTuple == rPnt.maTuple);
            }

            sal_Bool operator!=( const B3DHomPoint& rPnt ) const
            {
                implTestAndHomogenize();
                return (maTuple != rPnt.maTuple);
            }

            B3DHomPoint& operator=( const B3DHomPoint& rPnt )
            {
                maTuple = rPnt.maTuple;
                mfW = rPnt.mfW;
                return *this;
            }
        };

        // external operators
        //////////////////////////////////////////////////////////////////////////

        inline B3DHomPoint min(const B3DHomPoint& rVecA, const B3DHomPoint& rVecB)
        {
            B3DHomPoint aMin(
                (rVecB.getX() < rVecA.getX()) ? rVecB.getX() : rVecA.getX(),
                (rVecB.getY() < rVecA.getY()) ? rVecB.getY() : rVecA.getY(),
                (rVecB.getZ() < rVecA.getZ()) ? rVecB.getZ() : rVecA.getZ());
            return aMin;
        }

        inline B3DHomPoint max(const B3DHomPoint& rVecA, const B3DHomPoint& rVecB)
        {
            B3DHomPoint aMax(
                (rVecB.getX() > rVecA.getX()) ? rVecB.getX() : rVecA.getX(),
                (rVecB.getY() > rVecA.getY()) ? rVecB.getY() : rVecA.getY(),
                (rVecB.getZ() > rVecA.getZ()) ? rVecB.getZ() : rVecA.getZ());
            return aMax;
        }

        inline B3DHomPoint abs(const B3DHomPoint& rVec)
        {
            B3DHomPoint aAbs(
                (0.0 > rVec.getX()) ? -rVec.getX() : rVec.getX(),
                (0.0 > rVec.getY()) ? -rVec.getY() : rVec.getY(),
                (0.0 > rVec.getZ()) ? -rVec.getZ() : rVec.getZ());
            return aAbs;
        }

        inline B3DHomPoint interpolate(B3DHomPoint& rOld1, B3DHomPoint& rOld2, double t)
        {
            B3DHomPoint aInt(
                ((rOld2.getX() - rOld1.getX()) + t) + rOld1.getX(),
                ((rOld2.getY() - rOld1.getY()) + t) + rOld1.getY(),
                ((rOld2.getZ() - rOld1.getZ()) + t) + rOld1.getZ());
            return aInt;
        }

        inline B3DHomPoint average(B3DHomPoint& rOld1, B3DHomPoint& rOld2)
        {
            B3DHomPoint aAvg(
                (rOld1.getX() + rOld2.getX()) * 0.5,
                (rOld1.getY() + rOld2.getY()) * 0.5,
                (rOld1.getZ() + rOld2.getZ()) * 0.5);
            return aAvg;
        }

        inline B3DHomPoint average(B3DHomPoint& rOld1, B3DHomPoint& rOld2, B3DHomPoint& rOld3)
        {
            B3DHomPoint aAvg(
                (rOld1.getX() + rOld2.getX() + rOld3.getX()) * (1.0 / 3.0),
                (rOld1.getY() + rOld2.getY() + rOld3.getY()) * (1.0 / 3.0),
                (rOld1.getZ() + rOld2.getZ() + rOld3.getZ()) * (1.0 / 3.0));
            return aAvg;
        }

        inline B3DHomPoint operator+(const B3DHomPoint& rVecA, const B3DHomPoint& rVecB)
        {
            B3DHomPoint aSum(rVecA);
            aSum += rVecB;
            return aSum;
        }

        inline B3DHomPoint operator-(const B3DHomPoint& rVecA, const B3DHomPoint& rVecB)
        {
            B3DHomPoint aSub(rVecA);
            aSub -= rVecB;
            return aSub;
        }

        inline B3DHomPoint operator*(const B3DHomPoint& rVec, double t)
        {
            B3DHomPoint aNew(rVec);
            aNew *= t;
            return aNew;
        }

        inline B3DHomPoint operator*(double t, const B3DHomPoint& rVec)
        {
            B3DHomPoint aNew(rVec);
            aNew *= t;
            return aNew;
        }

        inline B3DHomPoint operator/(const B3DHomPoint& rVec, double t)
        {
            B3DHomPoint aNew(rVec);
            aNew /= t;
            return aNew;
        }

        inline B3DHomPoint operator/(double t, const B3DHomPoint& rVec)
        {
            B3DHomPoint aNew(rVec);
            aNew /= t;
            return aNew;
        }
    } // end of namespace point
} // end of namespace basegfx

#endif // _BGFX_POINT_B3DHOMPOINT_HXX
