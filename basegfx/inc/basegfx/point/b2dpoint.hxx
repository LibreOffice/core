/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dpoint.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:26:10 $
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

#ifndef _BGFX_POINT_B2DPOINT_HXX
#define _BGFX_POINT_B2DPOINT_HXX

#ifndef _BGFX_TUPLE_B2DTUPLE_HXX
#include <basegfx/tuple/b2dtuple.hxx>
#endif

#ifndef _BGFX_POINT_B2IPOINT_HXX
#include <basegfx/point/b2ipoint.hxx>
#endif

namespace basegfx
{
    // predeclaration
    class B2DHomMatrix;

    /** Base Point class with two double values

        This class derives all operators and common handling for
        a 2D data class from B2DTuple. All necessary extensions
        which are special for points will be added here.

        @see B2DTuple
    */
    class B2DPoint : public ::basegfx::B2DTuple
    {
    public:
        /** Create a 2D Point

            The point is initialized to (0.0, 0.0)
        */
        B2DPoint()
        :   B2DTuple()
        {}

        /** Create a 2D Point

            @param fX
            This parameter is used to initialize the X-coordinate
            of the 2D Point.

            @param fY
            This parameter is used to initialize the Y-coordinate
            of the 2D Point.
        */
        B2DPoint(double fX, double fY)
        :   B2DTuple(fX, fY)
        {}

        /** Create a copy of a 2D Point

            @param rPoint
            The 2D Point which will be copied.
        */
        B2DPoint(const B2DPoint& rPoint)
        :   B2DTuple(rPoint)
        {}

        /** Create a copy of a 2D Point

            @param rPoint
            The 2D Point which will be copied.
        */
        B2DPoint(const ::basegfx::B2IPoint& rPoint)
        :   B2DTuple(rPoint)
        {}

        /** constructor with tuple to allow copy-constructing
            from B2DTuple-based classes
        */
        B2DPoint(const ::basegfx::B2DTuple& rTuple)
        :   B2DTuple(rTuple)
        {}

        ~B2DPoint()
        {}

        /** *=operator to allow usage from B2DPoint, too
        */
        B2DPoint& operator*=( const B2DPoint& rPnt )
        {
            mfX *= rPnt.mfX;
            mfY *= rPnt.mfY;
            return *this;
        }

        /** *=operator to allow usage from B2DPoint, too
        */
        B2DPoint& operator*=(double t)
        {
            mfX *= t;
            mfY *= t;
            return *this;
        }

        /** assignment operator to allow assigning the results
            of B2DTuple calculations
        */
        B2DPoint& operator=( const ::basegfx::B2DTuple& rPoint );

        /** Transform point by given transformation matrix.

            The translational components of the matrix are, in
            contrast to B2DVector, applied.
        */
        B2DPoint& operator*=( const ::basegfx::B2DHomMatrix& rMat );

        static const B2DPoint& getEmptyPoint()
        {
            return (const B2DPoint&) ::basegfx::B2DTuple::getEmptyTuple();
        }
    };

    // external operators
    //////////////////////////////////////////////////////////////////////////

    /** Transform B2DPoint by given transformation matrix.

        Since this is a Point, translational components of the
        matrix are used.
    */
    B2DPoint operator*( const B2DHomMatrix& rMat, const B2DPoint& rPoint );
} // end of namespace basegfx

#endif /* _BGFX_POINT_B2DPOINT_HXX */
