/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _BGFX_VECTOR_B3DVECTOR_HXX
#define _BGFX_VECTOR_B3DVECTOR_HXX

#include <basegfx/tuple/b3dtuple.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    // predeclaration
    class B3DHomMatrix;

    /** Base Point class with three double values

        This class derives all operators and common handling for
        a 3D data class from B3DTuple. All necessary extensions
        which are special for 3D Vectors are added here.

        @see B3DTuple
    */
    class B3DVector : public ::basegfx::B3DTuple
    {
    public:
        /** Create a 3D Vector

            The vector is initialized to (0.0, 0.0, 0.0)
        */
        B3DVector()
        :   B3DTuple()
        {}

        /** Create a 3D Vector

            @param fX
            This parameter is used to initialize the X-coordinate
            of the 3D Vector.

            @param fY
            This parameter is used to initialize the Y-coordinate
            of the 3D Vector.

            @param fZ
            This parameter is used to initialize the Z-coordinate
            of the 3D Vector.
        */
        B3DVector(double fX, double fY, double fZ)
        :   B3DTuple(fX, fY, fZ)
        {}

        /** Create a copy of a 3D Vector

            @param rVec
            The 3D Vector which will be copied.
        */
        B3DVector(const B3DVector& rVec)
        :   B3DTuple(rVec)
        {}

        /** constructor with tuple to allow copy-constructing
            from B3DTuple-based classes
        */
        B3DVector(const ::basegfx::B3DTuple& rTuple)
        :   B3DTuple(rTuple)
        {}

        ~B3DVector()
        {}

        /** *=operator to allow usage from B3DVector, too
        */
        B3DVector& operator*=( const B3DVector& rPnt )
        {
            mfX *= rPnt.mfX;
            mfY *= rPnt.mfY;
            mfZ *= rPnt.mfZ;
            return *this;
        }

        /** *=operator to allow usage from B3DVector, too
        */
        B3DVector& operator*=(double t)
        {
            mfX *= t;
            mfY *= t;
            mfZ *= t;
            return *this;
        }

        /** assignment operator to allow assigning the results
            of B3DTuple calculations
        */
        B3DVector& operator=( const ::basegfx::B3DTuple& rVec )
        {
            mfX = rVec.getX();
            mfY = rVec.getY();
            mfZ = rVec.getZ();
            return *this;
        }

        /** Calculate the length of this 3D Vector

            @return The Length of the 3D Vector
        */
        double getLength(void) const
        {
            double fLen(scalar(*this));
            if((0.0 == fLen) || (1.0 == fLen))
                return fLen;
            return sqrt(fLen);
        }

        /** Calculate the length in the XY-Plane for this 3D Vector

            @return The XY-Plane Length of the 3D Vector
        */
        double getXYLength(void) const
        {
            double fLen((mfX * mfX) + (mfY * mfY));
            if((0.0 == fLen) || (1.0 == fLen))
                return fLen;
            return sqrt(fLen);
        }

        /** Calculate the length in the XZ-Plane for this 3D Vector

            @return The XZ-Plane Length of the 3D Vector
        */
        double getXZLength(void) const
        {
            double fLen((mfX * mfX) + (mfZ * mfZ)); // #i73040#
            if((0.0 == fLen) || (1.0 == fLen))
                return fLen;
            return sqrt(fLen);
        }

        /** Calculate the length in the YZ-Plane for this 3D Vector

            @return The YZ-Plane Length of the 3D Vector
        */
        double getYZLength(void) const
        {
            double fLen((mfY * mfY) + (mfZ * mfZ));
            if((0.0 == fLen) || (1.0 == fLen))
                return fLen;
            return sqrt(fLen);
        }

        /** Set the length of this 3D Vector

            @param fLen
            The to be achieved length of the 3D Vector
        */
        B3DVector& setLength(double fLen)
        {
            double fLenNow(scalar(*this));

            if(!::basegfx::fTools::equalZero(fLenNow))
            {
                const double fOne(1.0);

                if(!::basegfx::fTools::equal(fOne, fLenNow))
                {
                    fLen /= sqrt(fLenNow);
                }

                mfX *= fLen;
                mfY *= fLen;
                mfZ *= fLen;
            }

            return *this;
        }

        /** Normalize this 3D Vector

            The length of the 3D Vector is set to 1.0
        */
        B3DVector& normalize();

        /** Test if this 3D Vector is normalized

            @return
            true if lenth of vector is equal to 1.0
            false else
        */
        bool isNormalized() const
        {
            const double fOne(1.0);
            const double fScalar(scalar(*this));

            return (::basegfx::fTools::equal(fOne, fScalar));
        }

        /** get a 3D Vector which is perpendicular to this and a given 3D Vector

            @attention This only works if this and the given 3D Vector are
            both normalized.

            @param rNormalizedVec
            A normalized 3D Vector.

            @return
            A 3D Vector perpendicular to this and the given one
        */
        B3DVector getPerpendicular(const B3DVector& rNormalizedVec) const;

        /** get the projection of this Vector on the given Plane

            @attention This only works if the given 3D Vector defining
            the Plane is normalized.

            @param rNormalizedPlane
            A normalized 3D Vector defining a Plane.

            @return
            The projected 3D Vector
        */
        B3DVector getProjectionOnPlane(const B3DVector& rNormalizedPlane) const;

        /** Calculate the Scalar product

            This method calculates the Scalar product between this
            and the given 3D Vector.

            @param rVec
            A second 3D Vector.

            @return
            The Scalar Product of two 3D Vectors
        */
        double scalar(const B3DVector& rVec) const
        {
            return ((mfX * rVec.mfX) + (mfY * rVec.mfY) + (mfZ * rVec.mfZ));
        }

        /** Transform vector by given transformation matrix.

            Since this is a vector, translational components of the
            matrix are disregarded.
        */
        B3DVector& operator*=( const B3DHomMatrix& rMat );

        static const B3DVector& getEmptyVector()
        {
            return (const B3DVector&) ::basegfx::B3DTuple::getEmptyTuple();
        }
    };

    // external operators
    //////////////////////////////////////////////////////////////////////////

    /** get a 3D Vector which is in 2D (ignoring
        the Z-Coordinate) perpendicular to a given 3D Vector

        @attention This only works if the given 3D Vector is normalized.

        @param rNormalizedVec
        A normalized 3D Vector.

        @return
        A 3D Vector perpendicular to the given one in X,Y (2D).
    */
    inline B3DVector getPerpendicular2D( const B3DVector& rNormalizedVec )
    {
        B3DVector aPerpendicular(-rNormalizedVec.getY(), rNormalizedVec.getX(), rNormalizedVec.getZ());
        return aPerpendicular;
    }

    /** Test two vectors which need not to be normalized for parallelism

        @param rVecA
        The first 3D Vector

        @param rVecB
        The second 3D Vector

        @return
        bool if the two values are parallel. Also true if
        one of the vectors is empty.
    */
    bool areParallel( const B3DVector& rVecA, const B3DVector& rVecB );

    /** Transform vector by given transformation matrix.

        Since this is a vector, translational components of the
        matrix are disregarded.
    */
    B3DVector operator*( const B3DHomMatrix& rMat, const B3DVector& rVec );

    /** Calculate the Cross Product of two 3D Vectors

        @param rVecA
        A first 3D Vector.

        @param rVecB
        A second 3D Vector.

        @return
        The Cross Product of both 3D Vectors
    */
    inline B3DVector cross(const B3DVector& rVecA, const B3DVector& rVecB)
    {
        B3DVector aVec(
            rVecA.getY() * rVecB.getZ() - rVecA.getZ() * rVecB.getY(),
            rVecA.getZ() * rVecB.getX() - rVecA.getX() * rVecB.getZ(),
            rVecA.getX() * rVecB.getY() - rVecA.getY() * rVecB.getX());
        return aVec;
    }
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

#endif /* _BGFX_VECTOR_B3DVECTOR_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
