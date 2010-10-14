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

#ifndef _BGFX_VECTOR_B2DVECTOR_HXX
#define _BGFX_VECTOR_B2DVECTOR_HXX

#include <basegfx/tuple/b2dtuple.hxx>
#include <basegfx/vector/b2ivector.hxx>
#include <basegfx/vector/b2enums.hxx>

namespace basegfx
{
    // predeclaration
    class B2DHomMatrix;

    /** Base Point class with two double values

        This class derives all operators and common handling for
        a 2D data class from B2DTuple. All necessary extensions
        which are special for 2D Vectors are added here.

        @see B2DTuple
    */
    class B2DVector : public ::basegfx::B2DTuple
    {
    public:
        /** Create a 2D Vector

            The vector is initialized to (0.0, 0.0)
        */
        B2DVector()
        :   B2DTuple()
        {}

        /** Create a 2D Vector

            @param fX
            This parameter is used to initialize the X-coordinate
            of the 2D Vector.

            @param fY
            This parameter is used to initialize the Y-coordinate
            of the 2D Vector.
        */
        B2DVector(double fX, double fY)
        :   B2DTuple(fX, fY)
        {}

        /** Create a copy of a 2D Vector

            @param rVec
            The 2D Vector which will be copied.
        */
        B2DVector(const B2DVector& rVec)
        :   B2DTuple(rVec)
        {}

        /** Create a copy of a 2D Vector

            @param rVec
            The 2D Vector which will be copied.
        */
        B2DVector(const ::basegfx::B2IVector& rVec)
        :   B2DTuple(rVec)
        {}

        /** constructor with tuple to allow copy-constructing
            from B2DTuple-based classes
        */
        B2DVector(const ::basegfx::B2DTuple& rTuple)
        :   B2DTuple(rTuple)
        {}

        ~B2DVector()
        {}

        /** *=operator to allow usage from B2DVector, too
        */
        B2DVector& operator*=( const B2DVector& rPnt )
        {
            mfX *= rPnt.mfX;
            mfY *= rPnt.mfY;
            return *this;
        }

        /** *=operator to allow usage from B2DVector, too
        */
        B2DVector& operator*=(double t)
        {
            mfX *= t;
            mfY *= t;
            return *this;
        }

        /** assignment operator to allow assigning the results
            of B2DTuple calculations
        */
        B2DVector& operator=( const ::basegfx::B2DTuple& rVec );

        /** Calculate the length of this 2D Vector

            @return The Length of the 2D Vector
        */
        double getLength() const;

        /** Set the length of this 2D Vector

            @param fLen
            The to be achieved length of the 2D Vector
        */
        B2DVector& setLength(double fLen);

        /** Normalize this 2D Vector

            The length of the 2D Vector is set to 1.0
        */
        B2DVector& normalize();

        /** Test if this 2D Vector is normalized

            @return
            true if lenth of vector is equal to 1.0
            false else
        */
        bool isNormalized() const;

        /** Calculate the Scalar with another 2D Vector

            @param rVec
            The second 2D Vector

            @return
            The Scalar value of the two involved 2D Vectors
        */
        double scalar( const B2DVector& rVec ) const;

        /** Calculate the length of the cross product with another 2D Vector

            In 2D, returning an actual vector does not make much
            sense here. The magnitude, although, can be readily
            used for tasks such as angle calculations, since for
            the returned value, the following equation holds:
            retVal = getLength(this)*getLength(rVec)*sin(theta),
            with theta being the angle between the two vectors.

            @param rVec
            The second 2D Vector

            @return
            The length of the cross product of the two involved 2D Vectors
        */
        double cross( const B2DVector& rVec ) const;

        /** Calculate the Angle with another 2D Vector

            @param rVec
            The second 2D Vector

            @return
            The Angle value of the two involved 2D Vectors in -pi/2 < return < pi/2
        */
        double angle( const B2DVector& rVec ) const;

        /** Transform vector by given transformation matrix.

            Since this is a vector, translational components of the
            matrix are disregarded.
        */
        B2DVector& operator*=( const B2DHomMatrix& rMat );

        static const B2DVector& getEmptyVector();
    };

    // external operators
    //////////////////////////////////////////////////////////////////////////

    /** Calculate the orientation to another 2D Vector

        @param rVecA
        The first 2D Vector

        @param rVecB
        The second 2D Vector

        @return
        The mathematical Orientation of the two involved 2D Vectors
    */
    B2VectorOrientation getOrientation( const B2DVector& rVecA, const B2DVector& rVecB );

    /** Calculate a perpendicular 2D Vector to the given one

        @param rVec
        The source 2D Vector

        @attention This only works if the given 2D Vector is normalized.

        @return
        A 2D Vector perpendicular to the one given in parameter rVec
    */
    B2DVector getPerpendicular( const B2DVector& rNormalizedVec );

    /** Calculate a perpendicular 2D Vector to the given one,
        normalize the given one as preparation

        @param rVec
        The source 2D Vector

        @return
        A normalized 2D Vector perpendicular to the one given in parameter rVec
    */
    B2DVector getNormalizedPerpendicular( const B2DVector& rVec );

    /** Test two vectors which need not to be normalized for parallelism

        @param rVecA
        The first 2D Vector

        @param rVecB
        The second 2D Vector

        @return
        bool if the two values are parallel. Also true if
        one of the vectors is empty.
    */
    bool areParallel( const B2DVector& rVecA, const B2DVector& rVecB );

    /** Transform vector by given transformation matrix.

        Since this is a vector, translational components of the
        matrix are disregarded.
    */
    B2DVector operator*( const B2DHomMatrix& rMat, const B2DVector& rVec );

    /** Test continuity between given vectors.

        The two given vectors are assumed to describe control points on a
        common point. Calculate if there is a continuity between them.
    */
    B2VectorContinuity getContinuity( const B2DVector& rBackVector, const B2DVector& rForwardVector );

} // end of namespace basegfx

#endif /* _BGFX_VECTOR_B2DVECTOR_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
