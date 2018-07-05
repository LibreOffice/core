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

#ifndef INCLUDED_BASEGFX_VECTOR_B3DVECTOR_HXX
#define INCLUDED_BASEGFX_VECTOR_B3DVECTOR_HXX

#include <basegfx/tuple/b3dtuple.hxx>
#include <basegfx/basegfxdllapi.h>

namespace basegfx
{
    class B3DHomMatrix;

    /** Base Point class with three double values

        This class derives all operators and common handling for
        a 3D data class from B3DTuple. All necessary extensions
        which are special for 3D Vectors are added here.

        @see B3DTuple
    */
    class SAL_WARN_UNUSED BASEGFX_DLLPUBLIC B3DVector : public ::basegfx::B3DTuple
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

        /** constructor with tuple to allow copy-constructing
            from B3DTuple-based classes
        */
        B3DVector(const ::basegfx::B3DTuple& rTuple)
        :   B3DTuple(rTuple)
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
        double getLength() const
        {
            double fLen(scalar(*this));
            if((0.0 == fLen) || (1.0 == fLen))
                return fLen;
            return sqrt(fLen);
        }

        /** Calculate the length in the XZ-Plane for this 3D Vector

            @return The XZ-Plane Length of the 3D Vector
        */
        double getXZLength() const
        {
            double fLen((mfX * mfX) + (mfZ * mfZ)); // #i73040#
            if((0.0 == fLen) || (1.0 == fLen))
                return fLen;
            return sqrt(fLen);
        }

        /** Calculate the length in the YZ-Plane for this 3D Vector

            @return The YZ-Plane Length of the 3D Vector
        */
        double getYZLength() const
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

        /** get a 3D Vector which is perpendicular to this and a given 3D Vector

            @attention This only works if this and the given 3D Vector are
            both normalized.

            @param rNormalizedVec
            A normalized 3D Vector.

            @return
            A 3D Vector perpendicular to this and the given one
        */
        B3DVector getPerpendicular(const B3DVector& rNormalizedVec) const;

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
            return static_cast<const B3DVector&>( ::basegfx::B3DTuple::getEmptyTuple() );
        }
    };

    // external operators


    /** Test two vectors which need not to be normalized for parallelism

        @param rVecA
        The first 3D Vector

        @param rVecB
        The second 3D Vector

        @return
        bool if the two values are parallel. Also true if
        one of the vectors is empty.
    */
    BASEGFX_DLLPUBLIC bool areParallel( const B3DVector& rVecA, const B3DVector& rVecB );

    /** Transform vector by given transformation matrix.

        Since this is a vector, translational components of the
        matrix are disregarded.
    */
    BASEGFX_DLLPUBLIC B3DVector operator*( const B3DHomMatrix& rMat, const B3DVector& rVec );

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

#endif // INCLUDED_BASEGFX_VECTOR_B3DVECTOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
