/*************************************************************************
 *
 *  $RCSfile: b2dpoint.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2003-11-06 16:30:23 $
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

#ifndef _BGFX_POINT_B2DPOINT_HXX
#define _BGFX_POINT_B2DPOINT_HXX

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

    namespace point
    {
        /** Base Point class with two double values

            This class derives all operators and common handling for
            a 2D data class from B2DTuple. All necessary extensions
            which are special for points will be added here.

            @see B2DTuple
        */
        class B2DPoint : public ::basegfx::tuple::B2DTuple
        {
        public:
            /** Create a 2D Point

                @param fVal
                This parameter is used to initialize the coordinate
                part of the 2D Point.
            */
            B2DPoint(double fVal = 0.0)
            :   B2DTuple(fVal)
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

            /** constructor with tuple to allow copy-constructing
                from B2DTuple-based classes
            */
            B2DPoint(const ::basegfx::tuple::B2DTuple& rTuple)
            :   B2DTuple(rTuple)
            {}

            ~B2DPoint()
            {}

            /** assignment operator to allow assigning the results
                of B2DTuple calculations
            */
            B2DPoint& operator=( const ::basegfx::tuple::B2DTuple& rPoint );

            /** Transform point by given transformation matrix.

                The translational components of the matrix are, in
                contrast to B2DVector, applied.
            */
            B2DPoint& operator*=( const ::basegfx::matrix::B2DHomMatrix& rMat );

            static const B2DPoint& getEmptyPoint()
            {
                return (const B2DPoint&) ::basegfx::tuple::B2DTuple::getEmptyTuple();
            }
        };
    } // end of namespace point
} // end of namespace basegfx

#endif //  _BGFX_POINT_B2DPOINT_HXX
