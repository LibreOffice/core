/*************************************************************************
 *
 *  $RCSfile: b2dvector.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: aw $ $Date: 2003-11-11 09:56:09 $
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

#ifndef _BGFX_VECTOR_B2DVECTOR_HXX
#define _BGFX_VECTOR_B2DVECTOR_HXX

#ifndef _BGFX_TUPLE_B2DTUPLE_HXX
#include <basegfx/tuple/b2dtuple.hxx>
#endif

namespace basegfx
{
    namespace matrix
    {
        // predeclaration
        class B2DHomMatrix;
    } // end of namespace matrix;

    namespace vector
    {
        /** Descriptor for the mathematical orientations of two 2D Vectors
        */
        enum B2DVectorOrientation
        {
            /// mathematically positive oriented
            ORIENTATION_POSITIVE = 0,

            /// mathematically negative oriented
            ORIENTATION_NEGATIVE,

            /// mathematically neutral, thus parallel
            ORIENTATION_NEUTRAL
        };

        /** Descriptor for the mathematical continuity of two 2D Vectors
        */
        enum B2DVectorContinuity
        {
            /// none
            CONTINUITY_NONE = 0,

            /// mathematically negative oriented
            CONTINUITY_C1,

            /// mathematically neutral, thus parallel
            CONTINUITY_C2
        };

        /** Base Point class with two double values

            This class derives all operators and common handling for
            a 2D data class from B2DTuple. All necessary extensions
            which are special for 2D Vectors are added here.

            @see B2DTuple
        */
        class B2DVector : public ::basegfx::tuple::B2DTuple
        {
        public:
            /** Create a 2D Vector

                @param fVal
                This parameter is used to initialize the coordinate
                part of the 2D Vector.
            */
            B2DVector(double fVal = 0.0)
            :   B2DTuple(fVal)
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

            /** constructor with tuple to allow copy-constructing
                from B2DTuple-based classes
            */
            B2DVector(const ::basegfx::tuple::B2DTuple& rTuple)
            :   B2DTuple(rTuple)
            {}

            ~B2DVector()
            {}

            /** assignment operator to allow assigning the results
                of B2DTuple calculations
            */
            B2DVector& operator=( const ::basegfx::tuple::B2DTuple& rVec );

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
                sal_True if lenth of vector is equal to 1.0
                sal_False else
            */
            sal_Bool isNormalized() const;

            /** Calculate the Scalar with another 2D Vector

                @param rVec
                The second 2D Vector

                @return
                The Scalar value of the two involved 2D Vectors
            */
            double scalar( const B2DVector& rVec ) const;

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
            B2DVector& operator*=( const matrix::B2DHomMatrix& rMat );

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
        B2DVectorOrientation getOrientation( const B2DVector& rVecA, const B2DVector& rVecB );

        /** Calculate a perpendicular 2D Vector to the given one

            @param rVec
            The source 2D Vector

            @attention This only works if the given 2D Vector is normalized.

            @return
            A 2D Vector perpendicular to the one given in parameter rVec
        */
        B2DVector getPerpendicular( const B2DVector& rNormalizedVec );

        /** Test two vectors which need not to be normalized for parallelism

            @param rVecA
            The first 2D Vector

            @param rVecB
            The second 2D Vector

            @return
            sal_Bool if the two values are parallel. Also sal_True if
            one of the vectors is empty.
        */
        sal_Bool areParallel( const B2DVector& rVecA, const B2DVector& rVecB );

        /** Transform vector by given transformation matrix.

            Since this is a vector, translational components of the
            matrix are disregarded.
        */
        B2DVector operator*( const matrix::B2DHomMatrix& rMat, const B2DVector& rVec );

        /** Test continuity between given vectors.

            The two given vectors are assumed to describe control points on a
            common point. Calculate if there is a continuity between them.
        */
        ::basegfx::vector::B2DVectorContinuity getContinuity( const B2DVector& rBackVector, const B2DVector& rForwardVector );

    } // end of namespace vector
} // end of namespace basegfx

#endif //  _BGFX_VECTOR_B2DVECTOR_HXX
