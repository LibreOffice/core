/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dpolypolygoncutter.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:29:09 $
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

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGONCUTTER_HXX
#define _BGFX_POLYGON_B2DPOLYPOLYGONCUTTER_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif

#ifndef _BGFX_RANGE_B2DRANGE_HXX
#include <basegfx/range/b2drange.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

#include <vector>

//////////////////////////////////////////////////////////////////////////////
// class B2DPolyPolygonCutter

namespace basegfx
{
    // predeclarations
    class B2DPolygonNode;
    class B2DSimpleCut;

    // a type definition to have a vector of pointers to B2DPolygonNodes
    typedef ::std::vector< B2DPolygonNode* > B2DPolygonNodeVector;

    // a type definition to have a vector of pointers to B2DSimpleCuts
    typedef ::std::vector< B2DSimpleCut* > B2DSimpleCutVector;

    class B2DPolyPolygonCutter
    {
        // list of polys
        B2DPolygonNodeVector                    maPolygonList;

        // help routines
        B2DSimpleCut* getExistingCut(B2DSimpleCutVector& rTmpCuts, B2DPolygonNode* pA, B2DPolygonNode* pB);
        B2DPolygonNode* extractNextPolygon(B2DPolygonNode*& rpList);
        bool isCrossover(B2DPolygonNode* pA, B2DPolygonNode* pB);
        bool isCrossover(B2DSimpleCut* pEnter, B2DSimpleCut* pLeave);
        bool isNextSamePos(B2DPolygonNode* pA, B2DPolygonNode* pB);
        bool isPrevSamePos(B2DPolygonNode* pA, B2DPolygonNode* pB);
        void addAllNodes(B2DPolygonNode* pPolygon, B2DPolygonNode*& rpList);
        B2DPolygonNode* createNewPolygon(const ::basegfx::B2DPolygon& rPolygon);
        void deletePolygon(B2DPolygonNode* pCand);
        void polysToList(B2DPolygonNode*& rpList);
        void listToPolys(B2DPolygonNode*& rpList);
        void solveAllCuts(B2DSimpleCutVector& rCuts);

    public:
        B2DPolyPolygonCutter() {}
        ~B2DPolyPolygonCutter();

        // put/get poly
        void addPolygon(const ::basegfx::B2DPolygon& rPolygon);
        void addPolyPolygon(const ::basegfx::B2DPolyPolygon& rPolyPolygon);
        ::basegfx::B2DPolyPolygon getPolyPolygon();

        // transformations
        void removeSelfIntersections();
        void removeDoubleIntersections();
    };
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////


#endif /* _BGFX_POLYGON_B2DPOLYPOLYGONCUTTER_HXX */
