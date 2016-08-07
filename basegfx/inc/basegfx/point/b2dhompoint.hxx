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



#ifndef _BGFX_POINT_B2DHOMPOINT_HXX
#define _BGFX_POINT_B2DHOMPOINT_HXX

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
    /** Basic homogen Point class with two double values and one homogen factor

        This class provides access to homogen coordinates in 2D.
        For this purpose all the operators which need to do specific
        action due to their homogenity are implemented here.
        The only caveat are member methods which are declared as const
        but do change the content. These are documented for that reason.
        The class is designed to provide homogenous coordinates without
        direct access to the homogen part (mfW). This is also the reason
        for leaving out the [] operators which return references to members.

        @see B2DTuple
    */
    class BASEGFX_DLLPUBLIC B2DHomPoint
    {
    protected:
        /// This member contains the coordinate part of the point
        ::basegfx::B2DTuple                 maTuple;

        /// This Member holds the homogenous part of the point
        double                              mfW;

        /** Test if this homogen point does have a homogenous part

            @return Returns true if this point has no homogenous part
        */
        inline bool implIsHomogenized() const
        {
            const double fOne(1.0);
            return ::basegfx::fTools::equal(fOne, mfW);
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
        inline void implTestAndHomogenize() const
        {
            if(!implIsHomogenized())
                ((B2DHomPoint*)this)->implHomogenize();
        }

    public:
        /** Create a homogen point

            The point is initialized to (0.0, 0.0)
        */
        B2DHomPoint()
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
        */
        B2DHomPoint(double fX, double fY)
        :   maTuple(fX, fY),
            mfW(1.0)
        {}

        /** Create a copy of a 2D Point

            @param rVec
            The 2D point which will be copied. The homogenous part
            is initialized to 1.0.
        */
        B2DHomPoint(const B2DPoint& rVec)
        :   maTuple(rVec),
            mfW(1.0)
        {}

        /** Create a copy of a homogen point

            @param rVec
            The homogen point which will be copied. The homogenous part
            is copied, too.
        */
        B2DHomPoint(const B2DHomPoint& rVec)
        :   maTuple(rVec.maTuple.getX(), rVec.maTuple.getY()),
            mfW(rVec.mfW)
        {}

        ~B2DHomPoint()
        {}

        /** Get a 2D point from this homogenous point

            This method normalizes this homogen point if necessary and
            returns the corresponding 2D point for this homogen point.

            @attention Even when this method is const it may change all
            members of this instance.
        */
        inline B2DPoint getB2DPoint() const
        {
            implTestAndHomogenize();
            return B2DPoint(maTuple.getX(), maTuple.getY());
        }

        /** Get X-coordinate

            This method normalizes this homogen point if necessary and
            returns the corresponding X-coordinate for this homogen point.

            @attention Even when this method is const it may change all
            members of this instance.
        */
        inline double getX() const
        {
            implTestAndHomogenize();
            return maTuple.getX();
        }

        /** Get Y-coordinate

            This method normalizes this homogen point if necessary and
            returns the corresponding Y-coordinate for this homogen point.

            @attention Even when this method is const it may change all
            members of this instance.
        */
        inline double getY() const
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
        inline void setX(double fX)
        {
            maTuple.setX(implIsHomogenized() ? fX : fX * mfW );
        }

        /** Set Y-coordinate of the homogen point.

            This method sets the Y-coordinate of the homogen point. If
            the point does have a homogenous part this is taken into account.

            @param fY
            The to-be-set Y-coordinate without homogenous part.
        */
        inline void setY(double fY)
        {
            maTuple.setY(implIsHomogenized() ? fY : fY * mfW );
        }

        // operators
        //////////////////////////////////////////////////////////////////////

        inline B2DHomPoint& operator+=( const B2DHomPoint& rPnt )
        {
            maTuple.setX(getX() * rPnt.mfW + rPnt.getX() * mfW);
            maTuple.setY(getY() * rPnt.mfW + rPnt.getY() * mfW);
            mfW = mfW * rPnt.mfW;

            return *this;
        }

        inline B2DHomPoint& operator-=( const B2DHomPoint& rPnt )
        {
            maTuple.setX(getX() * rPnt.mfW - rPnt.getX() * mfW);
            maTuple.setY(getY() * rPnt.mfW - rPnt.getY() * mfW);
            mfW = mfW * rPnt.mfW;

            return *this;
        }

        inline B2DHomPoint& operator*=(double t)
        {
            if(!::basegfx::fTools::equalZero(t))
            {
                mfW /= t;
            }

            return *this;
        }

        B2DHomPoint& operator*=( const B2DHomMatrix& rMat );

        inline B2DHomPoint& operator/=(double t)
        {
            mfW *= t;
            return *this;
        }

        inline B2DHomPoint& operator-(void)
        {
            mfW = -mfW;
            return *this;
        }

        inline bool operator==( const B2DHomPoint& rPnt ) const
        {
            implTestAndHomogenize();
            return (maTuple == rPnt.maTuple);
        }

        inline bool operator!=( const B2DHomPoint& rPnt ) const
        {
            implTestAndHomogenize();
            return (maTuple != rPnt.maTuple);
        }

        inline B2DHomPoint& operator=( const B2DHomPoint& rPnt )
        {
            maTuple = rPnt.maTuple;
            mfW = rPnt.mfW;
            return *this;
        }
    };

    // external operators
    //////////////////////////////////////////////////////////////////////////

    inline B2DHomPoint minimum(const B2DHomPoint& rVecA, const B2DHomPoint& rVecB)
    {
        return B2DHomPoint( // getX()/getY() homogenizes already
            std::min(rVecB.getX(), rVecA.getX()),
            std::min(rVecB.getY(), rVecA.getY()));
    }

    inline B2DHomPoint maximum(const B2DHomPoint& rVecA, const B2DHomPoint& rVecB)
    {
        return B2DHomPoint( // getX()/getY() homogenizes already
            std::max(rVecB.getX(), rVecA.getX()),
            std::max(rVecB.getY(), rVecA.getY()));
    }

    inline B2DHomPoint absolute(const B2DHomPoint& rVec)
    {
        return B2DHomPoint( // getX()/getY() homogenizes already
            fabs(rVec.getX()),
            fabs(rVec.getY()));
    }

    inline B2DHomPoint interpolate(B2DHomPoint& rOld1, B2DHomPoint& rOld2, double t)
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
            return B2DHomPoint(
                ((rOld2.getX() - rOld1.getX()) * t) + rOld1.getX(),
                ((rOld2.getY() - rOld1.getY()) * t) + rOld1.getY());
        }
    }

    inline B2DHomPoint average(B2DHomPoint& rOld1, B2DHomPoint& rOld2)
    {
        return B2DHomPoint( // getX()/ getY() homogenizes already
            rOld1.getX() == rOld2.getX() ? rOld1.getX() : (rOld1.getX() + rOld2.getX()) * 0.5,
            rOld1.getY() == rOld2.getY() ? rOld1.getY() : (rOld1.getY() + rOld2.getY()) * 0.5);
    }

    inline B2DHomPoint average(B2DHomPoint& rOld1, B2DHomPoint& rOld2, B2DHomPoint& rOld3)
    {
        return B2DHomPoint( // getX()/ getY() homogenizes already
            (rOld1.getX() == rOld2.getX() && rOld2.getX() == rOld3.getX()) ? rOld1.getX() : (rOld1.getX() + rOld2.getX() + rOld3.getX()) * (1.0 / 3.0),
            (rOld1.getY() == rOld2.getY() && rOld2.getY() == rOld3.getY()) ? rOld1.getY() : (rOld1.getY() + rOld2.getY() + rOld3.getY()) * (1.0 / 3.0));
    }

    inline B2DHomPoint operator+(const B2DHomPoint& rVecA, const B2DHomPoint& rVecB)
    {
        B2DHomPoint aSum(rVecA);
        aSum += rVecB;
        return aSum;
    }

    inline B2DHomPoint operator-(const B2DHomPoint& rVecA, const B2DHomPoint& rVecB)
    {
        B2DHomPoint aSub(rVecA);
        aSub -= rVecB;
        return aSub;
    }

    inline B2DHomPoint operator*(const B2DHomPoint& rVec, double t)
    {
        B2DHomPoint aNew(rVec);
        aNew *= t;
        return aNew;
    }

    inline B2DHomPoint operator*(double t, const B2DHomPoint& rVec)
    {
        B2DHomPoint aNew(rVec);
        aNew *= t;
        return aNew;
    }

    inline B2DHomPoint operator*( const B2DHomMatrix& rMat, const B2DHomPoint& rPoint )
    {
        B2DHomPoint aNew(rPoint);
        return aNew*=rMat;
    }

    inline B2DHomPoint operator/(const B2DHomPoint& rVec, double t)
    {
        B2DHomPoint aNew(rVec);
        aNew /= t;
        return aNew;
    }

    inline B2DHomPoint operator/(double t, const B2DHomPoint& rVec)
    {
        B2DHomPoint aNew(rVec);
        aNew /= t;
        return aNew;
    }
} // end of namespace basegfx

#endif /* _BGFX_POINT_B2DHOMPOINT_HXX */
