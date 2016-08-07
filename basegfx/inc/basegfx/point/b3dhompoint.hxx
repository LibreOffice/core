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



#ifndef _BGFX_POINT_B3DHOMPOINT_HXX
#define _BGFX_POINT_B3DHOMPOINT_HXX

#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
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
    class BASEGFX_DLLPUBLIC B3DHomPoint
    {
    protected:
        /// This member contains the coordinate part of the point
        ::basegfx::B3DTuple                 maTuple;

        /// This Member holds the homogenous part of the point
        double                                      mfW;

        /** Test if this homogen point does have a homogenous part

            @return Returns true if this point has no homogenous part
        */
        bool implIsHomogenized() const
        {
            const double fOne(1.0);
            return ::basegfx::fTools::equal(mfW, fOne);
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

            The point is initialized to (0.0, 0.0, 0.0)
        */
        B3DHomPoint()
        :   maTuple(),
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
            if(!::basegfx::fTools::equalZero(t))
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

        bool operator==( const B3DHomPoint& rPnt ) const
        {
            implTestAndHomogenize();
            return (maTuple == rPnt.maTuple);
        }

        bool operator!=( const B3DHomPoint& rPnt ) const
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

    inline B3DHomPoint minimum(const B3DHomPoint& rVecA, const B3DHomPoint& rVecB)
    {
        return B3DHomPoint( // getX()/getY()/getZ() homogenizes already
            std::min(rVecB.getX(), rVecA.getX()),
            std::min(rVecB.getY(), rVecA.getY()),
            std::min(rVecB.getZ(), rVecA.getZ()));
    }

    inline B3DHomPoint maximum(const B3DHomPoint& rVecA, const B3DHomPoint& rVecB)
    {
        return B3DHomPoint(// getX()/getY()/getZ() homogenizes already
            std::max(rVecB.getX(), rVecA.getX()),
            std::max(rVecB.getY(), rVecA.getY()),
            std::max(rVecB.getZ(), rVecA.getZ()));
    }

    inline B3DHomPoint absolute(const B3DHomPoint& rVec)
    {
        return B3DHomPoint(// getX()/getY()/getZ() homogenizes already
            fabs(rVec.getX()),
            fabs(rVec.getY()),
            fabs(rVec.getZ()));
    }

    inline B3DHomPoint interpolate(B3DHomPoint& rOld1, B3DHomPoint& rOld2, double t)
    {
        if(0.0 >= t)
        {
            return rOld1;
        }
        else if(1.0 <= t)
        {
            return rOld2;
        }
        else if(rOld1 == rOld2) // this call homogenizes already
        {
            return rOld1;
        }
        else
        {
            return B3DHomPoint(
                ((rOld2.getX() - rOld1.getX()) * t) + rOld1.getX(),
                ((rOld2.getY() - rOld1.getY()) * t) + rOld1.getY(),
                ((rOld2.getZ() - rOld1.getZ()) * t) + rOld1.getZ());
        }
    }

    inline B3DHomPoint average(B3DHomPoint& rOld1, B3DHomPoint& rOld2)
    {
        return B3DHomPoint( // getX()/getY()/getZ() homogenizes already
            rOld1.getX() == rOld2.getX() ? rOld1.getX() : (rOld1.getX() + rOld2.getX()) * 0.5,
            rOld1.getY() == rOld2.getY() ? rOld1.getY() : (rOld1.getY() + rOld2.getY()) * 0.5,
            rOld1.getY() == rOld2.getY() ? rOld1.getY() : (rOld1.getY() + rOld2.getY()) * 0.5);
    }

    inline B3DHomPoint average(B3DHomPoint& rOld1, B3DHomPoint& rOld2, B3DHomPoint& rOld3)
    {
        return B3DHomPoint( // getX()/getY()/getZ() homogenizes already
            (rOld1.getX() == rOld2.getX() && rOld2.getX() == rOld3.getX()) ? rOld1.getX() : (rOld1.getX() + rOld2.getX() + rOld3.getX()) * (1.0 / 3.0),
            (rOld1.getY() == rOld2.getY() && rOld2.getY() == rOld3.getY()) ? rOld1.getY() : (rOld1.getY() + rOld2.getY() + rOld3.getY()) * (1.0 / 3.0),
            (rOld1.getZ() == rOld2.getZ() && rOld2.getZ() == rOld3.getZ()) ? rOld1.getZ() : (rOld1.getZ() + rOld2.getZ() + rOld3.getZ()) * (1.0 / 3.0));
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
} // end of namespace basegfx

#endif /* _BGFX_POINT_B3DHOMPOINT_HXX */
