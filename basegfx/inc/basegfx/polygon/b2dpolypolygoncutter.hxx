/*************************************************************************
 *
 *  $RCSfile: b2dpolypolygoncutter.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2003-11-06 16:28:48 $
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

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGONCUTTER_HXX
#define _BGFX_POLYGON_B2DPOLYPOLYGONCUTTER_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif

#ifndef _BGFX_VECTOR_B2DVECTOR_HXX
#include <basegfx/vector/b2dvector.hxx>
#endif

#ifndef _BGFX_RANGE_B2DRANGE_HXX
#include <basegfx/range/b2drange.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

#include <vector>
#include <algorithm>

//////////////////////////////////////////////////////////////////////////////
// B2DPolygonNode class
//
// This class is a pure tooling class used to hold a polygon in double linked
// list form for faster manipulation. It is used from the polygon clipper.

namespace basegfx
{
    namespace polygon
    {
        class B2DPolygonNode
        {
            ::basegfx::point::B2DPoint              maPosition;
            B2DPolygonNode*                         mpPrevious;
            B2DPolygonNode*                         mpNext;

            B2DPolygonNode*                         mpListPrevious;
            B2DPolygonNode*                         mpListNext;

        public:
            B2DPolygonNode(const ::basegfx::point::B2DPoint& rPosition, B2DPolygonNode* pPrevious);
            ~B2DPolygonNode();

            B2DPolygonNode* getPrevious() const { return mpPrevious; }
            B2DPolygonNode* getNext() const { return mpNext; }
            const ::basegfx::point::B2DPoint& getPosition() const { return maPosition; }

            void calcMinMaxX(double& fMaxAX, double& fMinAX) const;
            void calcMinMaxY(double& fMaxAY, double& fMinAY) const;

            void swapPreviousNext() { B2DPolygonNode* pZwi = mpPrevious; mpPrevious = mpNext; mpNext = pZwi; }
            void swapNextPointers(B2DPolygonNode* pCand);

            void addToList(B2DPolygonNode*& rpList);
            void remFromList(B2DPolygonNode*& rpList);

            sal_Bool getOrientation() const;
            void swapOrientation();
            ::basegfx::range::B2DRange getRange() const;

            sal_Bool isInside(const ::basegfx::point::B2DPoint& rPnt) const;
            sal_Bool isPolygonInside(B2DPolygonNode* pPoly) const;
        };

        // a type definition to have a vector of pointers to B2DPolygonNodes
        typedef ::std::vector< B2DPolygonNode* > B2DPolygonNodeVector;

    } // end of namespace polygon
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////
// B2DSimpleCut class

namespace basegfx
{
    namespace polygon
    {
        class B2DSimpleCut
        {
            B2DPolygonNode*                         mpLeft;
            B2DPolygonNode*                         mpRight;

            // bitfield
            unsigned                                mbCorrectOrientation : 1;
            unsigned                                mbOrientation : 1;

        public:
            B2DSimpleCut(B2DPolygonNode* pL, B2DPolygonNode* pR, sal_Bool bCoOr = sal_False, sal_Bool bOr = sal_True)
            :   mpLeft(pL),
                mpRight(pR),
                mbCorrectOrientation(bCoOr),
                mbOrientation(bOr)
            {
            }

            void solve();
            B2DPolygonNode* getLeft() const { return mpLeft; }
            B2DPolygonNode* getRight() const { return mpRight; }

            sal_Bool isSameCut(B2DPolygonNode* pA, B2DPolygonNode* pB) const
            {
                return ((pA == mpLeft && pB == mpRight) || (pB == mpLeft && pA == mpRight));
            }
        };

        // a type definition to have a vector of pointers to B2DSimpleCuts
        typedef ::std::vector< B2DSimpleCut* > B2DSimpleCutVector;

    } // end of namespace polygon
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////
// B2DClipExtraPolygonInfo class

namespace basegfx
{
    namespace polygon
    {
        class B2DClipExtraPolygonInfo
        {
            ::basegfx::range::B2DRange              maRange;
            sal_Int32                               mnDepth;

            // bitfield
            unsigned                                mbOrientation : 1;

        public:
            B2DClipExtraPolygonInfo() {}

            void init(B2DPolygonNode* pNew);
            const ::basegfx::range::B2DRange& getRange() const { return maRange; }
            sal_Bool getOrientation() const { return mbOrientation; }

            sal_Int32 getDepth() const { return mnDepth; }
            void changeDepth(sal_Bool bOrientation);
        };
    } // end of namespace polygon
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////
// class B2DPolyPolygonCutter

namespace basegfx
{
    namespace polygon
    {
        class B2DPolyPolygonCutter
        {
            // list of polys
            B2DPolygonNodeVector                    maPolygonList;
            B2DPolyPolygon                          maNotClosedPolygons;

            // help routines
            sal_Bool isSamePos(const ::basegfx::point::B2DPoint& rPntA, const ::basegfx::point::B2DPoint& rPntB)
            {
                return rPntA.equal(rPntB);
            }

            B2DSimpleCut* getExistingCut(B2DSimpleCutVector& rTmpCuts, B2DPolygonNode* pA, B2DPolygonNode* pB);
            B2DPolygonNode* extractNextPolygon(B2DPolygonNode*& rpList);
            sal_Bool isCrossover(B2DPolygonNode* pA, B2DPolygonNode* pB);
            sal_Bool isCrossover(B2DSimpleCut* pEnter, B2DSimpleCut* pLeave);

            sal_Bool isNextSamePos(B2DPolygonNode* pA, B2DPolygonNode* pB)
            {
                return isSamePos(pA->getNext()->getPosition(), pB->getNext()->getPosition());
            }

            sal_Bool isPrevSamePos(B2DPolygonNode* pA, B2DPolygonNode* pB)
            {
                return isSamePos(pA->getPrevious()->getPosition(), pB->getPrevious()->getPosition());
            }

            void addAllNodes(B2DPolygonNode* pPolygon, B2DPolygonNode*& rpList);
            B2DPolygonNode* createNewPolygon(const B2DPolygon& rPolygon);
            void deletePolygon(B2DPolygonNode* pCand);
            void polysToList(B2DPolygonNode*& rpList);
            void listToPolys(B2DPolygonNode*& rpList);

            sal_Bool doRangesIntersect(const ::basegfx::range::B2DRange& rRange1, const ::basegfx::range::B2DRange& rRange2) const
            {
                return rRange1.overlaps(rRange2);
            }

            sal_Bool doRangesInclude(const ::basegfx::range::B2DRange& rRange1, const ::basegfx::range::B2DRange& rRange2) const
            {
                return rRange1.isInside(rRange2);
            }

            void solveAllCuts(B2DSimpleCutVector& rCuts);

        public:
            B2DPolyPolygonCutter() {}
            ~B2DPolyPolygonCutter();

            // put/get poly
            void addPolyPolygon(const B2DPolyPolygon& rPolyPolygon, sal_Bool bForceOrientation = sal_False);
            void getPolyPolygon(B2DPolyPolygon& rPolyPolygon);

            // transformations
            void removeSelfIntersections();
            void removeDoubleIntersections();

            // remove included
            void removeIncludedPolygons(sal_Bool bUseOr = sal_True);
        };
    } // end of namespace polygon
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////


#endif //   _BGFX_POLYGON_B2DPOLYPOLYGONCUTTER_HXX
