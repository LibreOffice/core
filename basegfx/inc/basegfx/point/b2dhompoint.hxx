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
    class B2DHomPoint
    {
    protected:
        /// This member contains the coordinate part of the point
        ::basegfx::B2DTuple                 maTuple;

        /// This Member holds the homogenous part of the point
        double                                      mfW;

        /** Test if this homogen point does have a homogenous part

            @return Returns true if this point has no homogenous part
        */
        bool implIsHomogenized() const;

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
        void implTestAndHomogenize() const;

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
        B2DPoint getB2DPoint() const;

        /** Get X-coordinate

            This method normalizes this homogen point if necessary and
            returns the corresponding X-coordinate for this homogen point.

            @attention Even when this method is const it may change all
            members of this instance.
        */
        double getX() const;

        /** Get Y-coordinate

            This method normalizes this homogen point if necessary and
            returns the corresponding Y-coordinate for this homogen point.

            @attention Even when this method is const it may change all
            members of this instance.
        */
        double getY() const;

        /** Set X-coordinate of the homogen point.

            This method sets the X-coordinate of the homogen point. If
            the point does have a homogenous part this is taken into account.

            @param fX
            The to-be-set X-coordinate without homogenous part.
        */
        void setX(double fX);

        /** Set Y-coordinate of the homogen point.

            This method sets the Y-coordinate of the homogen point. If
            the point does have a homogenous part this is taken into account.

            @param fY
            The to-be-set Y-coordinate without homogenous part.
        */
        void setY(double fY);

        // operators
        //////////////////////////////////////////////////////////////////////

        B2DHomPoint& operator+=( const B2DHomPoint& rPnt );

        B2DHomPoint& operator-=( const B2DHomPoint& rPnt );

        B2DHomPoint& operator*=(double t);

        B2DHomPoint& operator*=( const B2DHomMatrix& rMat );

        B2DHomPoint& operator/=(double t);

        B2DHomPoint& operator-(void);

        bool operator==( const B2DHomPoint& rPnt ) const;

        bool operator!=( const B2DHomPoint& rPnt ) const;

        B2DHomPoint& operator=( const B2DHomPoint& rPnt );
    };

    // external operators
    //////////////////////////////////////////////////////////////////////////

    B2DHomPoint minimum(const B2DHomPoint& rVecA, const B2DHomPoint& rVecB);

    B2DHomPoint maximum(const B2DHomPoint& rVecA, const B2DHomPoint& rVecB);

    B2DHomPoint absolute(const B2DHomPoint& rVec);

    B2DHomPoint interpolate(B2DHomPoint& rOld1, B2DHomPoint& rOld2, double t);

    B2DHomPoint average(B2DHomPoint& rOld1, B2DHomPoint& rOld2);

    B2DHomPoint average(B2DHomPoint& rOld1, B2DHomPoint& rOld2, B2DHomPoint& rOld3);

    B2DHomPoint operator+(const B2DHomPoint& rVecA, const B2DHomPoint& rVecB);

    B2DHomPoint operator-(const B2DHomPoint& rVecA, const B2DHomPoint& rVecB);

    B2DHomPoint operator*(const B2DHomPoint& rVec, double t);

    B2DHomPoint operator*(double t, const B2DHomPoint& rVec);

    B2DHomPoint operator*( const B2DHomMatrix& rMat, const B2DHomPoint& rPoint );

    B2DHomPoint operator/(const B2DHomPoint& rVec, double t);

    B2DHomPoint operator/(double t, const B2DHomPoint& rVec);
} // end of namespace basegfx

#endif /* _BGFX_POINT_B2DHOMPOINT_HXX */
