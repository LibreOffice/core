/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b3dpoint.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:26:58 $
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

#ifndef _BGFX_POINT_B3DPOINT_HXX
#define _BGFX_POINT_B3DPOINT_HXX

#ifndef _BGFX_TUPLE_B3DTUPLE_HXX
#include <basegfx/tuple/b3dtuple.hxx>
#endif

namespace basegfx
{
    // predeclaration
    class B3DHomMatrix;

    /** Base Point class with three double values

        This class derives all operators and common handling for
        a 3D data class from B3DTuple. All necessary extensions
        which are special for points will be added here.

        @see B3DTuple
    */
    class B3DPoint : public ::basegfx::B3DTuple
    {
    public:
        /** Create a 3D Point

            The point is initialized to (0.0, 0.0, 0.0)
        */
        B3DPoint()
        :   B3DTuple()
        {}

        /** Create a 3D Point

            @param fX
            This parameter is used to initialize the X-coordinate
            of the 3D Point.

            @param fY
            This parameter is used to initialize the Y-coordinate
            of the 3D Point.

            @param fZ
            This parameter is used to initialize the Z-coordinate
            of the 3D Point.
        */
        B3DPoint(double fX, double fY, double fZ)
        :   B3DTuple(fX, fY, fZ)
        {}

        /** Create a copy of a 3D Point

            @param rVec
            The 3D Point which will be copied.
        */
        B3DPoint(const B3DPoint& rVec)
        :   B3DTuple(rVec)
        {}

        /** constructor with tuple to allow copy-constructing
            from B3DTuple-based classes
        */
        B3DPoint(const ::basegfx::B3DTuple& rTuple)
        :   B3DTuple(rTuple)
        {}

        ~B3DPoint()
        {}

        /** *=operator to allow usage from B3DPoint, too
        */
        B3DPoint& operator*=( const B3DPoint& rPnt )
        {
            mfX *= rPnt.mfX;
            mfY *= rPnt.mfY;
            mfZ *= rPnt.mfZ;
            return *this;
        }

        /** *=operator to allow usage from B3DPoint, too
        */
        B3DPoint& operator*=(double t)
        {
            mfX *= t;
            mfY *= t;
            mfZ *= t;
            return *this;
        }

        /** assignment operator to allow assigning the results
            of B3DTuple calculations
        */
        B3DPoint& operator=( const ::basegfx::B3DTuple& rVec )
        {
            mfX = rVec.getX();
            mfY = rVec.getY();
            mfZ = rVec.getZ();
            return *this;
        }

        /** Transform point by given transformation matrix.

            The translational components of the matrix are, in
            contrast to B3DVector, applied.
        */
        B3DPoint& operator*=( const ::basegfx::B3DHomMatrix& rMat );

        static const B3DPoint& getEmptyPoint()
        {
            return (const B3DPoint&) ::basegfx::B3DTuple::getEmptyTuple();
        }
    };
} // end of namespace basegfx

#endif /* _BGFX_POINT_B3DPOINT_HXX */
