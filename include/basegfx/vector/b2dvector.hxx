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

#ifndef INCLUDED_BASEGFX_VECTOR_B2DVECTOR_HXX
#define INCLUDED_BASEGFX_VECTOR_B2DVECTOR_HXX

#include <basegfx/tuple/b2dtuple.hxx>
#include <basegfx/vector/b2ivector.hxx>
#include <basegfx/vector/b2enums.hxx>
#include <basegfx/basegfxdllapi.h>

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
    class BASEGFX_DLLPUBLIC B2DVector : public ::basegfx::B2DTuple
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
        explicit B2DVector(const ::basegfx::B2IVector& rVec)
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


    /** Calculate the orientation to another 2D Vector

        @param rVecA
        The first 2D Vector

        @param rVecB
        The second 2D Vector

        @return
        The mathematical Orientation of the two involved 2D Vectors
    */
    BASEGFX_DLLPUBLIC B2VectorOrientation getOrientation( const B2DVector& rVecA, const B2DVector& rVecB );

    /** Calculate a perpendicular 2D Vector to the given one

        @param rVec
        The source 2D Vector

        @attention This only works if the given 2D Vector is normalized.

        @return
        A 2D Vector perpendicular to the one given in parameter rVec
    */
    BASEGFX_DLLPUBLIC B2DVector getPerpendicular( const B2DVector& rNormalizedVec );

    /** Calculate a perpendicular 2D Vector to the given one,
        normalize the given one as preparation

        @param rVec
        The source 2D Vector

        @return
        A normalized 2D Vector perpendicular to the one given in parameter rVec
    */
    BASEGFX_DLLPUBLIC B2DVector getNormalizedPerpendicular( const B2DVector& rVec );

    /** Test two vectors which need not to be normalized for parallelism

        @param rVecA
        The first 2D Vector

        @param rVecB
        The second 2D Vector

        @return
        bool if the two values are parallel. Also true if
        one of the vectors is empty.
    */
    BASEGFX_DLLPUBLIC bool areParallel( const B2DVector& rVecA, const B2DVector& rVecB );

    /** Transform vector by given transformation matrix.

        Since this is a vector, translational components of the
        matrix are disregarded.
    */
    BASEGFX_DLLPUBLIC B2DVector operator*( const B2DHomMatrix& rMat, const B2DVector& rVec );

    /** Test continuity between given vectors.

        The two given vectors are assumed to describe control points on a
        common point. Calculate if there is a continuity between them.
    */
    BASEGFX_DLLPUBLIC B2VectorContinuity getContinuity( const B2DVector& rBackVector, const B2DVector& rForwardVector );

} // end of namespace basegfx

#endif // INCLUDED_BASEGFX_VECTOR_B2DVECTOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
