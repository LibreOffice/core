/*************************************************************************
 *
 *  $RCSfile: b3dvector.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2003-11-06 16:30:26 $
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

#ifndef _BGFX_VECTOR_B3DVECTOR_HXX
#define _BGFX_VECTOR_B3DVECTOR_HXX

#ifndef _BGFX_TUPLE_B3DTUPLE_HXX
#include <basegfx/tuple/b3dtuple.hxx>
#endif

namespace basegfx
{
    namespace vector
    {
        /** Base Point class with three double values

            This class derives all operators and common handling for
            a 3D data class from B3DTuple. All necessary extensions
            which are special for 3D Vectors are added here.

            @see B3DTuple
        */
        class B3DVector : public ::basegfx::tuple::B3DTuple
        {
        public:
            /** Create a 3D Vector

                @param fVal
                This parameter is used to initialize the coordinate
                part of the 3D Vector.
            */
            B3DVector(double fVal = 0.0)
            :   B3DTuple(fVal)
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
            B3DVector(const ::basegfx::tuple::B3DTuple& rTuple)
            :   B3DTuple(rTuple)
            {}

            ~B3DVector()
            {}

            /** assignment operator to allow assigning the results
                of B3DTuple calculations
            */
            B3DVector& operator=( const ::basegfx::tuple::B3DTuple& rVec )
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
                double fLen((mfX * mfZ) + (mfY * mfZ));
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

                if(!::basegfx::numeric::fTools::equalZero(fLenNow))
                {
                    const double fOne(1.0);

                    if(!::basegfx::numeric::fTools::equal(fOne, fLenNow))
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
                sal_True if lenth of vector is equal to 1.0
                sal_False else
            */
            sal_Bool isNormalized() const
            {
                const double fOne(1.0);
                const double fScalar(scalar(*this));

                return (::basegfx::numeric::fTools::equal(fOne, fScalar));
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

            static const B3DVector& getEmptyVector()
            {
                return (const B3DVector&) ::basegfx::tuple::B3DTuple::getEmptyTuple();
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
    } // end of namespace vector
} // end of namespace basegfx

#endif // _BGFX_VECTOR_B3DVECTOR_HXX
