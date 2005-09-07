/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b3ivector.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:39:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _BGFX_VECTOR_B3IVECTOR_HXX
#define _BGFX_VECTOR_B3IVECTOR_HXX

#ifndef _BGFX_TUPLE_B3ITUPLE_HXX
#include <basegfx/tuple/b3ituple.hxx>
#endif

namespace basegfx
{
    // predeclaration
    class B3DHomMatrix;

    /** Base Point class with three sal_Int32 values

        This class derives all operators and common handling for
        a 3D data class from B3ITuple. All necessary extensions
        which are special for 3D Vectors are added here.

        @see B3ITuple
    */
    class B3IVector : public ::basegfx::B3ITuple
    {
    public:
        /** Create a 3D Vector

            The vector is initialized to (0, 0, 0)
        */
        B3IVector()
        :   B3ITuple()
        {}

        /** Create a 3D Vector

            @param nX
            This parameter is used to initialize the X-coordinate
            of the 3D Vector.

            @param nY
            This parameter is used to initialize the Y-coordinate
            of the 3D Vector.

            @param nZ
            This parameter is used to initialize the Z-coordinate
            of the 3D Vector.
        */
        B3IVector(sal_Int32 nX, sal_Int32 nY, sal_Int32 nZ)
        :   B3ITuple(nX, nY, nZ)
        {}

        /** Create a copy of a 3D Vector

            @param rVec
            The 3D Vector which will be copied.
        */
        B3IVector(const B3IVector& rVec)
        :   B3ITuple(rVec)
        {}

        /** constructor with tuple to allow copy-constructing
            from B3ITuple-based classes
        */
        B3IVector(const ::basegfx::B3ITuple& rTuple)
        :   B3ITuple(rTuple)
        {}

        ~B3IVector()
        {}

        /** *=operator to allow usage from B3IVector, too
        */
        B3IVector& operator*=( const B3IVector& rPnt )
        {
            mnX *= rPnt.mnX;
            mnY *= rPnt.mnY;
            mnZ *= rPnt.mnZ;
            return *this;
        }

        /** *=operator to allow usage from B3IVector, too
        */
        B3IVector& operator*=(sal_Int32 t)
        {
            mnX *= t;
            mnY *= t;
            mnZ *= t;
            return *this;
        }

        /** assignment operator to allow assigning the results
            of B3ITuple calculations
        */
        B3IVector& operator=( const ::basegfx::B3ITuple& rVec )
        {
            mnX = rVec.getX();
            mnY = rVec.getY();
            mnZ = rVec.getZ();
            return *this;
        }

        /** Calculate the length of this 3D Vector

            @return The Length of the 3D Vector
        */
        double getLength(void) const
        {
            double fLen(scalar(*this));
            if((0 == fLen) || (1.0 == fLen))
                return fLen;
            return sqrt(fLen);
        }

        /** Calculate the length in the XY-Plane for this 3D Vector

            @return The XY-Plane Length of the 3D Vector
        */
        double getXYLength(void) const
        {
            double fLen((mnX * mnX) + (mnY * mnY));
            if((0 == fLen) || (1.0 == fLen))
                return fLen;
            return sqrt(fLen);
        }

        /** Calculate the length in the XZ-Plane for this 3D Vector

            @return The XZ-Plane Length of the 3D Vector
        */
        double getXZLength(void) const
        {
            double fLen((mnX * mnZ) + (mnY * mnZ));
            if((0 == fLen) || (1.0 == fLen))
                return fLen;
            return sqrt(fLen);
        }

        /** Calculate the length in the YZ-Plane for this 3D Vector

            @return The YZ-Plane Length of the 3D Vector
        */
        double getYZLength(void) const
        {
            double fLen((mnY * mnY) + (mnZ * mnZ));
            if((0 == fLen) || (1.0 == fLen))
                return fLen;
            return sqrt(fLen);
        }

        /** Set the length of this 3D Vector

            @param fLen
            The to be achieved length of the 3D Vector
        */
        B3IVector& setLength(double fLen)
        {
            double fLenNow(scalar(*this));

            if(!::basegfx::fTools::equalZero(fLenNow))
            {
                const double fOne(1.0);

                if(!::basegfx::fTools::equal(fOne, fLenNow))
                {
                    fLen /= sqrt(fLenNow);
                }

                mnX = fround(mnX*fLen);
                mnY = fround(mnY*fLen);
                mnZ = fround(mnZ*fLen);
            }

            return *this;
        }

        /** Calculate the Scalar product

            This method calculates the Scalar product between this
            and the given 3D Vector.

            @param rVec
            A second 3D Vector.

            @return
            The Scalar Product of two 3D Vectors
        */
        double scalar(const B3IVector& rVec) const
        {
            return ((mnX * rVec.mnX) + (mnY * rVec.mnY) + (mnZ * rVec.mnZ));
        }

        /** Transform vector by given transformation matrix.

            Since this is a vector, translational components of the
            matrix are disregarded.
        */
        B3IVector& operator*=( const B3DHomMatrix& rMat );

        static const B3IVector& getEmptyVector()
        {
            return (const B3IVector&) ::basegfx::B3ITuple::getEmptyTuple();
        }
    };

    // external operators
    //////////////////////////////////////////////////////////////////////////

    /** Transform vector by given transformation matrix.

        Since this is a vector, translational components of the
        matrix are disregarded.
    */
    B3IVector operator*( const B3DHomMatrix& rMat, const B3IVector& rVec );

    /** Calculate the Cross Product of two 3D Vectors

        @param rVecA
        A first 3D Vector.

        @param rVecB
        A second 3D Vector.

        @return
        The Cross Product of both 3D Vectors
    */
    inline B3IVector cross(const B3IVector& rVecA, const B3IVector& rVecB)
    {
        B3IVector aVec(
            rVecA.getY() * rVecB.getZ() - rVecA.getZ() * rVecB.getY(),
            rVecA.getZ() * rVecB.getX() - rVecA.getX() * rVecB.getZ(),
            rVecA.getX() * rVecB.getY() - rVecA.getY() * rVecB.getX());
        return aVec;
    }
} // end of namespace basegfx

#endif /* _BGFX_VECTOR_B3DVECTOR_HXX */
