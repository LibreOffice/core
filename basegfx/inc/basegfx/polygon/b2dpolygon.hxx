/*************************************************************************
 *
 *  $RCSfile: b2dpolygon.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2003-11-05 12:25:39 $
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

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#define _BGFX_POLYGON_B2DPOLYGON_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

//////////////////////////////////////////////////////////////////////////////
// predeclarations
class ImplB2DPolygon;

namespace basegfx
{
    namespace polygon
    {
        class B2DPolygon;
    } // end of namespace polygon

    namespace point
    {
        class B2DPoint;
    } // end of namespace point
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace polygon
    {
        class B2DPolygon
        {
        private:
            // internal data.
            ImplB2DPolygon*                             mpPolygon;

            // internal method to force a ref-counted instance to be copied
            // to a modifyable unique copy.
            void implForceUniqueCopy();

        public:
            B2DPolygon();
            B2DPolygon(const B2DPolygon& rPolygon);
            B2DPolygon(const B2DPolygon& rPolygon, sal_uInt32 nIndex, sal_uInt32 nCount);
            ~B2DPolygon();

            // assignment operator
            B2DPolygon& operator=(const B2DPolygon& rPolygon);

            // compare operators
            sal_Bool operator==(const B2DPolygon& rPolygon) const;
            sal_Bool operator!=(const B2DPolygon& rPolygon) const;

            // member count
            sal_uInt32 count() const;

            // Coordinate interface
            point::B2DPoint getB2DPoint(sal_uInt32 nIndex) const;
            void setB2DPoint(sal_uInt32 nIndex, const point::B2DPoint& rValue);

            // Coordinate insert/append
            void insert(sal_uInt32 nIndex, const point::B2DPoint& rPoint, sal_uInt32 nCount = 1);
            void append(const point::B2DPoint& rPoint, sal_uInt32 nCount = 1);

            // ControlPoint interface
            point::B2DPoint getControlPointA(sal_uInt32 nIndex) const;
            void setControlPointA(sal_uInt32 nIndex, const point::B2DPoint& rValue);
            point::B2DPoint getControlPointB(sal_uInt32 nIndex) const;
            void setControlPointB(sal_uInt32 nIndex, const point::B2DPoint& rValue);
            sal_Bool areControlPointsUsed() const;

            // insert/append other 2D polygons
            void insert(sal_uInt32 nIndex, const B2DPolygon& rPoly, sal_uInt32 nIndex2 = 0, sal_uInt32 nCount = 0);
            void append(const B2DPolygon& rPoly, sal_uInt32 nIndex = 0, sal_uInt32 nCount = 0);

            // remove
            void remove(sal_uInt32 nIndex, sal_uInt32 nCount = 1);

            // clear all points
            void clear();

            // closed state
            sal_Bool isClosed() const;
            void setClosed(sal_Bool bNew);

            // flip polygon direction
            void flip();

            // test if Polygon has double points
            sal_Bool hasDoublePoints() const;

            // remove double points, at the begin/end and follow-ups, too
            void removeDoublePoints();

            // isInside tests for B2dPoint and other B2dPolygon. On border is not inside.
            sal_Bool isInside(const point::B2DPoint& rPoint);
            sal_Bool isInside(const B2DPolygon& rPolygon);
        };
    } // end of namespace polygon
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

#endif //   _BGFX_POLYGON_B2DPOLYGON_HXX
