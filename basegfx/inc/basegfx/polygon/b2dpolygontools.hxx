/*************************************************************************
 *
 *  $RCSfile: b2dpolygontools.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2003-11-05 12:25:40 $
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

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#define _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX

#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif

#ifndef _BGFX_VECTOR_B2DVECTOR_HXX
#include <basegfx/vector/b2dvector.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace polygon
    {
        // predefinitions
        class B2DPolygon;
    } // end of namespace polygon

    namespace range
    {
        class B2DRange;
    } // end of namespace range

    namespace polygon
    {
        namespace tools
        {
            // B2DPolygon tools

            /** Check if given polygon is closed. This is kind of a
                'classic' method to support old polygon definitions.
                Those old polygon definitions define the closed state
                of the polygon using identical start and endpoints. This
                method corrects this (removes double start/end points)
                and sets the Closed()-state of the polygon correctly.
            */
            void checkClosed(polygon::B2DPolygon& rCandidate);

            // Get index of outmost point (e.g. biggest X and biggest Y)
            sal_uInt32 getIndexOfOutmostPoint(const polygon::B2DPolygon& rCandidate);

            // Get successor and predecessor indices. Returning the same index means there
            // is none. Same for successor.
            sal_uInt32 getIndexOfPredecessor(sal_uInt32 nIndex, const polygon::B2DPolygon& rCandidate);
            sal_uInt32 getIndexOfSuccessor(sal_uInt32 nIndex, const polygon::B2DPolygon& rCandidate);

            // Get index of first different predecessor. Returning the same index means there
            // is none. Same for successor.
            sal_uInt32 getIndexOfDifferentPredecessor(sal_uInt32 nIndex, const polygon::B2DPolygon& rCandidate);
            sal_uInt32 getIndexOfDifferentSuccessor(sal_uInt32 nIndex, const polygon::B2DPolygon& rCandidate);

            // Get orientation of Polygon
            ::basegfx::vector::B2DVectorOrientation getOrientation(const ::basegfx::polygon::B2DPolygon& rCandidate);

            // isInside tests for B2dPoint and other B2dPolygon. On border is not inside.
            sal_Bool isInside(const ::basegfx::polygon::B2DPolygon& rCandidate, const ::basegfx::point::B2DPoint& rPoint);
            sal_Bool isInside(const ::basegfx::polygon::B2DPolygon& rCandidate, const ::basegfx::polygon::B2DPolygon& rPolygon);

            // get size of polygon
            ::basegfx::range::B2DRange getRange(const ::basegfx::polygon::B2DPolygon& rCandidate);

            // get area of polygon
            double getArea(const ::basegfx::polygon::B2DPolygon& rCandidate);

            // get length of polygon edge from point nIndex to nIndex + 1
            double getEdgeLength(const ::basegfx::polygon::B2DPolygon& rCandidate, sal_uInt32 nIndex);

            // get length of polygon
            double getLength(const ::basegfx::polygon::B2DPolygon& rCandidate);

            // get position on polygon for absolute given distance. If
            // length is given, it is assumed the correct polygon length, if 0.0 it is calculated
            // using getLength(...)
            ::basegfx::point::B2DPoint getPositionAbsolute(const ::basegfx::polygon::B2DPolygon& rCandidate, double fDistance, double fLength = 0.0);

            // get position on polygon for relative given distance in range [0.0 .. 1.0]. If
            // length is given, it is assumed the correct polygon length, if 0.0 it is calculated
            // using getLength(...)
            ::basegfx::point::B2DPoint getPositionRelative(const ::basegfx::polygon::B2DPolygon& rCandidate, double fDistance, double fLength = 0.0);

            // get orientation at given polygon point
            ::basegfx::vector::B2DVectorOrientation getPointOrientation(const ::basegfx::polygon::B2DPolygon& rCandidate, sal_uInt32 nIndex);

            /* Still missing:
            void transform(const Matrix4D& rTfMatrix);
            Polygon3D getExpandedPolygon(sal_uInt32 nNum);
            */


        } // end of namespace tools
    } // end of namespace polygon
} // end of namespace basegfx

#endif //   _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
