/*************************************************************************
 *
 *  $RCSfile: b2dpolypolygoncutter.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2003-11-10 11:45:51 $
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
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// B2DPolygonNode implementation

namespace basegfx
{
    namespace polygon
    {
        B2DPolygonNode::B2DPolygonNode(const ::basegfx::point::B2DPoint& rPosition, B2DPolygonNode* pPrevious)
        :   maPosition(rPosition)
        {
            mpListPrevious = this;
            mpListNext = this;

            if(pPrevious)
            {
                mpNext = pPrevious->getNext();
                mpPrevious = pPrevious;
                mpNext->mpPrevious = this;
                mpPrevious->mpNext = this;
            }
            else
            {
                mpPrevious = mpNext = this;
            }
        }

        B2DPolygonNode::~B2DPolygonNode()
        {
            if(mpNext != this)
            {
                mpPrevious->mpNext = mpNext;
                mpNext->mpPrevious = mpPrevious;
            }
        }

        void B2DPolygonNode::calcMinMaxX(double& fMaxAX, double& fMinAX) const
        {
            if(maPosition.getX() > mpNext->maPosition.getX())
            {
                fMaxAX = maPosition.getX();
                fMinAX = mpNext->maPosition.getX();
            }
            else
            {
                fMaxAX = mpNext->maPosition.getX();
                fMinAX = maPosition.getX();
            }
        }

        void B2DPolygonNode::calcMinMaxY(double& fMaxAY, double& fMinAY) const
        {
            if(maPosition.getY() > mpNext->maPosition.getY())
            {
                fMaxAY = maPosition.getY();
                fMinAY = mpNext->maPosition.getY();
            }
            else
            {
                fMaxAY = mpNext->maPosition.getY();
                fMinAY = maPosition.getY();
            }
        }

        void B2DPolygonNode::swapNextPointers(B2DPolygonNode* pCand)
        {
            B2DPolygonNode* pTemporary = mpNext;
            mpNext = pCand->mpNext;
            pCand->mpNext = pTemporary;
            mpNext->mpPrevious = this;
            pCand->mpNext->mpPrevious = pCand;
        }

        void B2DPolygonNode::addToList(B2DPolygonNode*& rpList)
        {
            if(rpList)
            {
                mpListNext = rpList->mpListNext;
                rpList->mpListNext = this;
                mpListPrevious = rpList;
                mpListNext->mpListPrevious = this;
            }
            else
            {
                rpList = this;
            }
        }

        void B2DPolygonNode::remFromList(B2DPolygonNode*& rpList)
        {
            if(mpListNext != this)
            {
                if(rpList == this)
                {
                    rpList = mpListPrevious;
                }

                mpListPrevious->mpListNext = mpListNext;
                mpListNext->mpListPrevious = mpListPrevious;
                mpListNext = mpListPrevious = this;
            }
            else
            {
                if(rpList == this)
                {
                    rpList = 0L;
                }
            }
        }

        sal_Bool B2DPolygonNode::getOrientation() const
        {
            const B2DPolygonNode* pOutmost = this;
            const B2DPolygonNode* pCurrent = this->getNext();

            while(pCurrent != this)
            {
                if(::basegfx::numeric::fTools::more(pOutmost->getPosition().getX(), pCurrent->getPosition().getX()))
                {
                    if(pCurrent->getPosition().getX() < pOutmost->getPosition().getX())
                    {
                        pOutmost = pCurrent;
                    }
                    else
                    {
                        if(pCurrent->getPosition().getY() < pOutmost->getPosition().getY())
                        {
                            pOutmost = pCurrent;
                        }
                    }
                }

                // next node
                pCurrent = pCurrent->getNext();
            }

            ::basegfx::vector::B2DVector aVec1(pOutmost->getPrevious()->getPosition() - pOutmost->getPosition());
            ::basegfx::vector::B2DVector aVec2(pOutmost->getNext()->getPosition() - pOutmost->getPosition());
            return sal_Bool(::basegfx::numeric::fTools::more(aVec1.getX() * aVec2.getY(), aVec1.getY() * aVec2.getX()));
        }

        void B2DPolygonNode::swapOrientation()
        {
            B2DPolygonNode* pCurrent = this;

            do {
                pCurrent->swapPreviousNext();
                pCurrent = pCurrent->getPrevious();
            } while(pCurrent != this);
        }

        ::basegfx::range::B2DRange B2DPolygonNode::getRange() const
        {
            ::basegfx::range::B2DRange aRetval;
            const B2DPolygonNode* pCurrent = this;

            do {
                aRetval.expand(pCurrent->getPosition());
                pCurrent = pCurrent->getPrevious();
            } while(pCurrent != this);

            return aRetval;
        }

        sal_Bool B2DPolygonNode::isInside(const ::basegfx::point::B2DPoint& rPoint, sal_Bool bWithBorder) const
        {
            sal_Bool bInside(sal_False);
            const B2DPolygonNode* pCurrent = this;

            do
            {
                if(bWithBorder && pCurrent->getPosition().equal(rPoint))
                {
                    return sal_True;
                }

                B2DPolygonNode* pNext = pCurrent->getNext();
                const sal_Bool bCompYA(::basegfx::numeric::fTools::more(pCurrent->getPosition().getY(), rPoint.getY()));
                const sal_Bool bCompYB(::basegfx::numeric::fTools::more(pNext->getPosition().getY(), rPoint.getY()));

                if(bCompYA != bCompYB)
                {
                    const sal_Bool bCompXA(::basegfx::numeric::fTools::more(pCurrent->getPosition().getX(), rPoint.getX()));
                    const sal_Bool bCompXB(::basegfx::numeric::fTools::more(pNext->getPosition().getX(), rPoint.getX()));

                    if(bCompXA == bCompXB)
                    {
                        if(bCompXA)
                        {
                            bInside = !bInside;
                        }
                    }
                    else
                    {
                        double fCmp =
                            pNext->getPosition().getX() - (pNext->getPosition().getY() - rPoint.getY()) *
                            (pCurrent->getPosition().getX() - pNext->getPosition().getX()) /
                            (pCurrent->getPosition().getY() - pNext->getPosition().getY());

                        if(bWithBorder && ::basegfx::numeric::fTools::more(fCmp, rPoint.getX()))
                        {
                            bInside = !bInside;
                        }
                        else if(::basegfx::numeric::fTools::moreOrEqual(fCmp, rPoint.getX()))
                        {
                            bInside = !bInside;
                        }
                    }
                }

                // next edge
                pCurrent = pNext;

            } while(pCurrent != this);

            return bInside;
        }

        sal_Bool B2DPolygonNode::isPolygonInside(B2DPolygonNode* pPoly, sal_Bool bWithBorder) const
        {
            B2DPolygonNode* pTest = pPoly;
            sal_Bool bAllAInside(sal_True);

            do {
                bAllAInside = isInside(pTest->getPosition(), bWithBorder);
                pTest = pTest->getNext();
            } while(bAllAInside && pTest != pPoly);

            return bAllAInside;
        }
    } // end of namespace polygon
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////
// B2DSimpleCut implementation

namespace basegfx
{
    namespace polygon
    {
        void B2DSimpleCut::solve()
        {
            mpLeft->swapNextPointers(mpRight);

            if(mbCorrectOrientation)
            {
                if(mpLeft->getOrientation() != mbOrientation)
                {
                    mpLeft->swapOrientation();
                }

                if(mpRight->getOrientation() != mbOrientation)
                {
                    mpRight->swapOrientation();
                }
            }
        }
    } // end of namespace polygon
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////
// B2DClipExtraPolygonInfo implementation

namespace basegfx
{
    namespace polygon
    {
        void B2DClipExtraPolygonInfo::init(B2DPolygonNode* pNew)
        {
            maRange = pNew->getRange();
            mbOrientation = pNew->getOrientation();
            mnDepth = (mbOrientation) ? 0L : -1L;
        }

        void B2DClipExtraPolygonInfo::changeDepth(sal_Bool bOrientation)
        {
            if(bOrientation)
            {
                mnDepth++;
            }
            else
            {
                mnDepth--;
            }
        }
    } // end of namespace polygon
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////
// implementation B2DPolyPolygonCutter

namespace basegfx
{
    namespace polygon
    {
        B2DPolyPolygonCutter::~B2DPolyPolygonCutter()
        {
            for(sal_uInt32 a(0L); a < maPolygonList.size(); a++)
            {
                delete maPolygonList[a];
            }

            maPolygonList.clear();
        }

        void B2DPolyPolygonCutter::removeIncludedPolygons(sal_Bool bUseOr)
        {
            const sal_uInt32 aCount(maPolygonList.size());
            B2DClipExtraPolygonInfo* pInfos = new B2DClipExtraPolygonInfo[aCount];
            sal_uInt32 a, b;

            // fill infos
            for(a = 0L; a < aCount; a++)
            {
                pInfos[a].init(maPolygonList[a]);
            }

            // get all includes
            for(a = 0L; a < aCount; a++)
            {
                B2DClipExtraPolygonInfo& rInfoA = pInfos[a];

                for(b = 0L; b < aCount; b++)
                {
                    B2DClipExtraPolygonInfo& rInfoB = pInfos[b];

                    if(a != b && doRangesInclude(rInfoA.getRange(), rInfoB.getRange()))
                    {
                        // volume B in A, test pA, pB for inclusion, with border
                        if(maPolygonList[a]->isPolygonInside(maPolygonList[b], sal_True))
                        {
                            // pB is inside pA
                            rInfoB.changeDepth(rInfoA.getOrientation());
                        }
                    }
                }
            }

            // delete removable
            for(a = 0L, b = 0L; a < aCount; a++)
            {
                B2DClipExtraPolygonInfo& rInfo = pInfos[a];

                if((bUseOr && rInfo.getDepth() != 0L) || (!bUseOr && rInfo.getDepth() < 1L))
                {
                    B2DPolygonNodeVector::iterator aPosition(maPolygonList.begin() + b);
                    B2DPolygonNode* pCandidate = *aPosition;
                    maPolygonList.erase(aPosition);
                    deletePolygon(pCandidate);
                }
                else
                {
                    b++;
                }
            }

            // delete infos
            delete[] pInfos;
        }

        void B2DPolyPolygonCutter::solveAllCuts(B2DSimpleCutVector& rCuts)
        {
            B2DPolygonNode* pNewList = 0L;

            // add all nodes of polys to list
            polysToList(pNewList);

            // solve cuts
            B2DSimpleCutVector::iterator aCandidate(rCuts.begin());

            for(; aCandidate < rCuts.end(); aCandidate++)
            {
                B2DSimpleCut* pCut = *aCandidate;
                pCut->solve();
                delete pCut;
            }

            rCuts.clear();

            // extract polys
            listToPolys(pNewList);
        }

        void B2DPolyPolygonCutter::polysToList(B2DPolygonNode*& rpList)
        {
            B2DPolygonNodeVector::iterator aCandidate(maPolygonList.begin());

            for(; aCandidate != maPolygonList.end(); aCandidate++)
            {
                addAllNodes(*aCandidate, rpList);
            }

            maPolygonList.clear();
        }

        void B2DPolyPolygonCutter::listToPolys(B2DPolygonNode*& rpList)
        {
            while(rpList)
            {
                // get one
                B2DPolygonNode* pNew = extractNextPolygon(rpList);

                if(pNew)
                {
                    maPolygonList.push_back(pNew);
                }
            }
        }

        B2DPolygonNode* B2DPolyPolygonCutter::createNewPolygon(const B2DPolygon& rPolygon)
        {
            B2DPolygonNode* pRetval = NULL;

            for(sal_uInt32 a(0L); a < rPolygon.count(); a++)
            {
                ::basegfx::point::B2DPoint aPoint(rPolygon.getB2DPoint(a));
                pRetval = new B2DPolygonNode(aPoint, pRetval);
            }

            return pRetval;
        }

        void B2DPolyPolygonCutter::deletePolygon(B2DPolygonNode* pCand)
        {
            B2DPolygonNode* pPoly = pCand;

            while(pPoly)
            {
                B2DPolygonNode* pNext = pPoly->getNext();

                if(pNext == pPoly)
                {
                    pNext = 0L;
                }

                delete pPoly;
                pPoly = pNext;
            }
        }

        void B2DPolyPolygonCutter::addAllNodes(B2DPolygonNode* pPolygon, B2DPolygonNode*& rpList)
        {
            B2DPolygonNode* pAct = pPolygon;

            do {
                pAct->addToList(rpList);
                pAct = pAct->getNext();
            } while(pAct != pPolygon);
        }

        void B2DPolyPolygonCutter::addPolyPolygon(const B2DPolyPolygon& rPolyPolygon, sal_Bool bForceOrientation)
        {
            for(sal_uInt32 a(0L); a < rPolyPolygon.count(); a++)
            {
                B2DPolygon aCandidate = rPolyPolygon.getPolygon(a);
                aCandidate.removeDoublePoints();

                if(!aCandidate.isClosed() || aCandidate.count() < 3)
                {
                    maNotClosedPolygons.append(aCandidate);
                }
                else
                {
                    if(bForceOrientation)
                    {
                        ::basegfx::vector::B2DVectorOrientation aOrientation =
                            ::basegfx::polygon::tools::getOrientation(aCandidate);

                        if(::basegfx::vector::ORIENTATION_POSITIVE != aOrientation)
                        {
                            aCandidate.flip();
                        }
                    }

                    B2DPolygonNode* pNew = createNewPolygon(aCandidate);
                    maPolygonList.push_back(pNew);
                }
            }
        }

        void B2DPolyPolygonCutter::getPolyPolygon(B2DPolyPolygon& rPolyPolygon)
        {
            B2DPolygonNodeVector::iterator aCandidate(maPolygonList.begin());

            for(; aCandidate < maPolygonList.end(); aCandidate++)
            {
                B2DPolygonNode* pCand = *aCandidate;
                B2DPolygonNode* pAct = pCand;
                sal_uInt32 nCount(0L);

                do {
                    nCount++;
                    pAct = pAct->getNext();
                } while(pAct != pCand);

                if(nCount > 2L)
                {
                    B2DPolygon aNewPolygon;

                    do {
                        aNewPolygon.append(pAct->getPosition());
                        pAct = pAct->getNext();
                    } while(pAct != pCand);

                    aNewPolygon.setClosed(sal_True);
                    rPolyPolygon.append(aNewPolygon);
                }

                deletePolygon(pCand);
            }

            maPolygonList.clear();

            while(maNotClosedPolygons.count())
            {
                rPolyPolygon.append(maNotClosedPolygons.getPolygon(0L));
                maNotClosedPolygons.remove(0L);
            }
        }

        B2DSimpleCut* B2DPolyPolygonCutter::getExistingCut(B2DSimpleCutVector& rTmpCuts, B2DPolygonNode* pA, B2DPolygonNode* pB)
        {
            for(sal_uInt32 a(0L); a < rTmpCuts.size(); a++)
            {
                B2DSimpleCut* pCand = rTmpCuts[a];

                if(pCand->isSameCut(pA, pB))
                {
                    return pCand;
                }
            }

            return 0L;
        }

        B2DPolygonNode* B2DPolyPolygonCutter::extractNextPolygon(B2DPolygonNode*& rpList)
        {
            B2DPolygonNode* pStart = rpList;

            // remove all nodes of this poly from list
            B2DPolygonNode* pAct = pStart;
            sal_uInt32 nNumNodes(0L);

            do {
                pAct->remFromList(rpList);
                pAct = pAct->getNext();
                nNumNodes++;
            } while(pAct != pStart);

            if(nNumNodes < 3L)
            {
                deletePolygon(pStart);
                return 0L;
            }
            else
            {
                return pStart;
            }
        }

        void B2DPolyPolygonCutter::removeSelfIntersections()
        {
            B2DSimpleCutVector aCuts;
            B2DSimpleCutVector aNewCuts;
            B2DPolygonNode* pCand;
            B2DPolygonNode* pA;
            B2DPolygonNode* pB;
            double fMaxAX, fMinAX, fMaxAY, fMinAY;
            double fMaxBX, fMinBX, fMaxBY, fMinBY;
            double fCut;

            // first job: Find all cuts and add points there
            for(sal_uInt32 a(0L); a < maPolygonList.size(); a++)
            {
                pCand = maPolygonList[a];
                pA = pCand;

                // one run to find same start positions (so there is no need to
                // search for existing cuts in main loop)
                do {
                    pB = pA->getNext();

                    do {
                        if(isSamePos(pA->getPosition(), pB->getPosition()))
                        {
                            aNewCuts.push_back(new B2DSimpleCut(pA, pB, sal_True, pCand->getOrientation()));
                        }

                        // next B
                        pB = pB->getNext();
                    } while(pB != pCand);

                    // next A
                    pA = pA->getNext();
                } while(pA->getNext() != pCand);

                // second run to find real cuts
                pA = pCand;

                do {
                    // get bounds for this edge in poly
                    pA->calcMinMaxX(fMaxAX, fMinAX);
                    pA->calcMinMaxY(fMaxAY, fMinAY);
                    pB = pA->getNext();

                    do {
                        pB->calcMinMaxX(fMaxBX, fMinBX);

                        if(::basegfx::numeric::fTools::more(fMaxBX, fMinAX)
                            && ::basegfx::numeric::fTools::more(fMaxAX, fMinBX))
                        {
                            pB->calcMinMaxY(fMaxBY, fMinBY);

                            if(::basegfx::numeric::fTools::more(fMaxBY, fMinAY)
                                && ::basegfx::numeric::fTools::more(fMaxAY, fMinBY))
                            {
                                if(!isSamePos(pA->getPosition(), pB->getPosition()))
                                {
                                    const ::basegfx::vector::B2DVector aVectorA(pA->getNext()->getPosition() - pA->getPosition());
                                    const ::basegfx::vector::B2DVector aVectorB(pB->getNext()->getPosition() - pB->getPosition());

                                    if(::basegfx::polygon::tools::findCut(pA->getPosition(), aVectorA, pB->getPosition(), aVectorB, CUTFLAG_LINE, &fCut))
                                    {
                                        // crossover, two new points
                                        ::basegfx::point::B2DPoint aNewPos(::basegfx::tuple::interpolate(pA->getPosition(), pA->getNext()->getPosition(), fCut));
                                        B2DPolygonNode* pCutLo = new B2DPolygonNode(aNewPos, pA);
                                        B2DPolygonNode* pCutHi = new B2DPolygonNode(aNewPos, pB);
                                        aNewCuts.push_back(new B2DSimpleCut(pCutLo, pCutHi, sal_True, pCand->getOrientation()));
                                        pA->calcMinMaxX(fMaxAX, fMinAX);
                                        pA->calcMinMaxY(fMaxAY, fMinAY);
                                    }
                                    else
                                    {
                                        if(::basegfx::polygon::tools::isPointOnEdge(pA->getPosition(), pB->getPosition(), aVectorB, &fCut))
                                        {
                                            // startpoint A at edge B, one new point
                                            B2DPolygonNode* pCutHi = new B2DPolygonNode(pA->getPosition(), pB);
                                            aNewCuts.push_back(new B2DSimpleCut(pA, pCutHi, sal_True, pCand->getOrientation()));
                                        }
                                        else if(::basegfx::polygon::tools::isPointOnEdge(pB->getPosition(), pA->getPosition(), aVectorA, &fCut))
                                        {
                                            // startpoint B at edge A, one new point
                                            B2DPolygonNode* pCutLo = new B2DPolygonNode(pB->getPosition(), pA);
                                            aNewCuts.push_back(new B2DSimpleCut(pCutLo, pB, sal_True, pCand->getOrientation()));
                                            pA->calcMinMaxX(fMaxAX, fMinAX);
                                            pA->calcMinMaxY(fMaxAY, fMinAY);
                                        }
                                    }
                                }
                            }
                        }

                        // next B
                        pB = pB->getNext();
                    } while(pB != pCand);

                    // next A
                    pA = pA->getNext();
                } while(pA->getNext() != pCand);

                // copy new cuts to cuts
                aCuts.insert(aCuts.begin(), aNewCuts.begin(), aNewCuts.end());
                aNewCuts.clear();
            }

            // second job: if there were cuts, split polys
            if(aCuts.size())
            {
                solveAllCuts(aCuts);
            }
        }

        sal_Bool B2DPolyPolygonCutter::isCrossover(B2DPolygonNode* pA, B2DPolygonNode* pB)
        {
            // build entering vectors
            ::basegfx::vector::B2DVector aVecA(pA->getPrevious()->getPosition() - pA->getPosition());
            ::basegfx::vector::B2DVector aVecB(pB->getPrevious()->getPosition() - pA->getPosition());
            aVecA.normalize();
            aVecB.normalize();
            double fDegreeA2 = atan2(aVecA.getY(), aVecA.getX());
            double fDegreeB2 = atan2(aVecB.getY(), aVecB.getX());

            // build leaving vectors
            aVecA = pA->getNext()->getPosition() - pA->getPosition();
            aVecB = pB->getNext()->getPosition() - pA->getPosition();
            aVecA.normalize();
            aVecB.normalize();
            double fDegreeA1 = atan2(aVecA.getY(), aVecA.getX());
            double fDegreeB1 = atan2(aVecB.getY(), aVecB.getX());

            // compare
            if(fDegreeA1 > fDegreeA2)
            {
                double fTemp = fDegreeA2;
                fDegreeA2 = fDegreeA1;
                fDegreeA1 = fTemp;
            }

            sal_Bool bB1Inside(::basegfx::numeric::fTools::more(fDegreeB1, fDegreeA1)
                && ::basegfx::numeric::fTools::more(fDegreeA2, fDegreeB1));
            sal_Bool bB2Inside(::basegfx::numeric::fTools::more(fDegreeB2, fDegreeA1)
                && ::basegfx::numeric::fTools::more(fDegreeA2, fDegreeB2));

            if(bB1Inside && bB2Inside)
            {
                return sal_False;
            }

            sal_Bool bB1Outside(::basegfx::numeric::fTools::more(fDegreeA1, fDegreeB1)
                || ::basegfx::numeric::fTools::more(fDegreeB1, fDegreeA2));
            sal_Bool bB2Outside(::basegfx::numeric::fTools::more(fDegreeA1, fDegreeB2)
                || ::basegfx::numeric::fTools::more(fDegreeB2, fDegreeA2));

            return !(bB1Outside && bB2Outside);
        }

        sal_Bool B2DPolyPolygonCutter::isCrossover(B2DSimpleCut* pEnter, B2DSimpleCut* pLeave)
        {
            // build entering vectors
            ::basegfx::vector::B2DVector aVecJ(pEnter->getLeft()->getNext()->getPosition() - pEnter->getLeft()->getPosition());
            ::basegfx::vector::B2DVector aVecA(pEnter->getLeft()->getPrevious()->getPosition() - pEnter->getLeft()->getPosition());
            ::basegfx::vector::B2DVector aVecB(pEnter->getRight()->getPrevious()->getPosition() - pEnter->getLeft()->getPosition());
            aVecJ.normalize();
            aVecA.normalize();
            aVecB.normalize();
            double fDegreeJo = atan2(aVecJ.getY(), aVecJ.getX());
            double fDegreeA2 = atan2(aVecA.getY(), aVecA.getX()) - fDegreeJo;
            double fDegreeB2 = atan2(aVecB.getY(), aVecB.getX()) - fDegreeJo;

            // move to range [0..2PI[
            while(fDegreeA2 < 0.0)
            {
                fDegreeA2 += (2.0 * F_PI);
            }

            while(fDegreeA2 >= (2.0 * F_PI))
            {
                fDegreeA2 -= (2.0 * F_PI);
            }

            // move to range [0..2PI[
            while(fDegreeB2 < 0.0)
            {
                fDegreeB2 += (2.0 * F_PI);
            }

            while(fDegreeB2 >= (2.0 * F_PI))
            {
                fDegreeB2 -= (2.0 * F_PI);
            }

            sal_Bool bA2BiggerB2(::basegfx::numeric::fTools::more(fDegreeA2, fDegreeB2));

            // build leaving vectors
            aVecJ = pLeave->getLeft()->getPrevious()->getPosition() - pLeave->getLeft()->getPosition();
            aVecA = pLeave->getLeft()->getNext()->getPosition() - pLeave->getLeft()->getPosition();
            aVecB = pLeave->getRight()->getNext()->getPosition() - pLeave->getLeft()->getPosition();
            aVecJ.normalize();
            aVecA.normalize();
            aVecB.normalize();
            fDegreeJo = atan2(aVecJ.getY(), aVecJ.getX());
            double fDegreeA1 = atan2(aVecA.getY(), aVecA.getX()) - fDegreeJo;
            double fDegreeB1 = atan2(aVecB.getY(), aVecB.getX()) - fDegreeJo;

            // move to range [0..2PI[
            while(fDegreeA1 < 0.0)
            {
                fDegreeA1 += (2.0 * F_PI);
            }

            while(fDegreeA1 >= (2.0 * F_PI))
            {
                fDegreeA1 -= (2.0 * F_PI);
            }

            // move to range [0..2PI[
            while(fDegreeB1 < 0)
            {
                fDegreeB1 += (2.0 * F_PI);
            }

            while(fDegreeB1 >= (2.0 * F_PI))
            {
                fDegreeB1 -= (2.0 * F_PI);
            }

            sal_Bool bA1BiggerB1(::basegfx::numeric::fTools::more(fDegreeA1, fDegreeB1));

            // compare
            return (bA1BiggerB1 == bA2BiggerB2);
        }

        void B2DPolyPolygonCutter::removeDoubleIntersections()
        {
            double fMaxAX, fMinAX, fMaxAY, fMinAY;
            double fMaxBX, fMinBX, fMaxBY, fMinBY;
            double fCut;
            B2DSimpleCutVector aCuts;
            B2DSimpleCutVector aTmpCuts;
            B2DSimpleCutVector aNewCuts;
            B2DPolygonNode* pCandA;
            B2DPolygonNode* pCandB;
            B2DPolygonNode* pA;
            B2DPolygonNode* pB;
            sal_uInt32 a;

            // create volume list for all polys for faster compares
            ::basegfx::range::B2DRange* pVolumes = new ::basegfx::range::B2DRange[maPolygonList.size()];

            for(a = 0L; a < maPolygonList.size(); a++)
            {
                pVolumes[a] = maPolygonList[a]->getRange();
            }

            // register cuts (and add points for them) between pCandA and pCandB
            for(a = 0L; a + 1L < maPolygonList.size(); a++)
            {
                pCandA = maPolygonList[a];

                for(sal_uInt32 b = a + 1L; b < maPolygonList.size(); b++)
                {
                    if(doRangesIntersect(pVolumes[a], pVolumes[b]))
                    {
                        pCandB = maPolygonList[b];
                        pA = pCandA;

                        // one run to find same start positions (so there is no need to
                        // search for existing cuts in main loop)
                        do {
                            pB = pCandB;

                            do {
                                if(isSamePos(pA->getPosition(), pB->getPosition()))
                                {
                                    aTmpCuts.push_back(new B2DSimpleCut(pA, pB));
                                }

                                // next B
                                pB = pB->getNext();
                            } while(pB != pCandB);

                            // next A
                            pA = pA->getNext();
                        } while(pA != pCandA);

                        // second run to find real cuts
                        pA = pCandA;

                        do {
                            // get bounds for this edge in poly
                            pA->calcMinMaxX(fMaxAX, fMinAX);
                            pA->calcMinMaxY(fMaxAY, fMinAY);
                            pB = pCandB;

                            do {
                                pB->calcMinMaxX(fMaxBX, fMinBX);

                                if(::basegfx::numeric::fTools::more(fMaxBX, fMinAX)
                                    && ::basegfx::numeric::fTools::more(fMaxAX, fMinBX))
                                {
                                    pB->calcMinMaxY(fMaxBY, fMinBY);

                                    if(::basegfx::numeric::fTools::more(fMaxBY, fMinAY)
                                        && ::basegfx::numeric::fTools::more(fMaxAY, fMinBY))
                                    {
                                        if(!isSamePos(pA->getPosition(), pB->getPosition()))
                                        {
                                            const ::basegfx::vector::B2DVector aVectorA(pA->getNext()->getPosition() - pA->getPosition());
                                            const ::basegfx::vector::B2DVector aVectorB(pB->getNext()->getPosition() - pB->getPosition());

                                            if(::basegfx::polygon::tools::findCut(pA->getPosition(), aVectorA, pB->getPosition(), aVectorB, CUTFLAG_LINE, &fCut))
                                            {
                                                // crossover, two new points, use as cutpoint
                                                ::basegfx::point::B2DPoint aNewPos(::basegfx::tuple::interpolate(pA->getPosition(), pA->getNext()->getPosition(), fCut));
                                                B2DPolygonNode* pCutLo = new B2DPolygonNode(aNewPos, pA);
                                                B2DPolygonNode* pCutHi = new B2DPolygonNode(aNewPos, pB);
                                                aNewCuts.push_back(new B2DSimpleCut(pCutLo, pCutHi));
                                                pA->calcMinMaxX(fMaxAX, fMinAX);
                                                pA->calcMinMaxY(fMaxAY, fMinAY);
                                            }
                                            else
                                            {
                                                if(::basegfx::polygon::tools::isPointOnEdge(pA->getPosition(), pB->getPosition(), aVectorB, &fCut))
                                                {
                                                    // startpoint A at edge B, one new point
                                                    // leaves or enters common section
                                                    B2DPolygonNode* pCutHi = new B2DPolygonNode(pA->getPosition(), pB);
                                                    aTmpCuts.push_back(new B2DSimpleCut(pA, pCutHi));
                                                }
                                                else if(::basegfx::polygon::tools::isPointOnEdge(pB->getPosition(), pA->getPosition(), aVectorA, &fCut))
                                                {
                                                    // startpoint B at edge A, one new point
                                                    // leaves or enters common section
                                                    B2DPolygonNode* pCutLo = new B2DPolygonNode(pB->getPosition(), pA);
                                                    aTmpCuts.push_back(new B2DSimpleCut(pCutLo, pB));
                                                    pA->calcMinMaxX(fMaxAX, fMinAX);
                                                    pA->calcMinMaxY(fMaxAY, fMinAY);
                                                }
                                            }
                                        }
                                    }
                                }

                                // next B
                                pB = pB->getNext();
                            } while(pB != pCandB);

                            // next A
                            pA = pA->getNext();
                        } while(pA != pCandA);

                        // test all temporary cuts for simple criteria
                        for(sal_uInt32 c(0L); c < aTmpCuts.size();)
                        {
                            B2DSimpleCut* pCand = aTmpCuts[c];
                            sal_Bool bPrevSamePos(isPrevSamePos(pCand->getLeft(), pCand->getRight()));
                            sal_Bool bNextSamePos(isNextSamePos(pCand->getLeft(), pCand->getRight()));
                            sal_Bool bDelete(sal_False);
                            sal_Bool bIncC(sal_True);

                            if(bPrevSamePos && bNextSamePos)
                            {
                                // single point inside continued same direction section
                                bDelete = sal_True;
                            }
                            else if(!bPrevSamePos && !bNextSamePos)
                            {
                                // this is no same direction section, test for real cut
                                if(isCrossover(pCand->getLeft(), pCand->getRight()))
                                {
                                    // real cut, move to real cutlist
                                    aNewCuts.push_back(pCand);
                                    aTmpCuts.erase(aTmpCuts.begin() + c);
                                    bIncC = sal_False;
                                }
                                else
                                {
                                    // no cut, just a touch in one point
                                    bDelete = sal_True;
                                }
                            }

                            // delete if wanted
                            if(bDelete)
                            {
                                delete pCand;
                                aTmpCuts.erase(aTmpCuts.begin() + c);
                                bIncC = sal_False;
                            }

                            // next candidate
                            if(bIncC)
                                c++;
                        }

                        // are there entering/leaving same direction sections?
                        while(aTmpCuts.size())
                        {
                            // this cuts enter/leave a common same-direction section between
                            // polygons pCandA, pCandB. If it is a real crossover, a cutpoint
                            // for it is needed, else it can be ignored.
                            B2DSimpleCut* pCutA = aTmpCuts[0L];
                            aTmpCuts.erase(aTmpCuts.begin());
                            B2DPolygonNode* pActA = pCutA->getLeft();
                            B2DPolygonNode* pActB = pCutA->getRight();
                            sal_Bool bPrevSamePos(isPrevSamePos(pActA, pActB));
                            sal_Bool bNextSamePos(isNextSamePos(pActA, pActB));

                            if(aTmpCuts.size())
                            {
                                B2DSimpleCut* pCutB = 0L;

                                if(isNextSamePos(pCutA->getLeft(), pCutA->getRight()))
                                {
                                    // this is a start node
                                    B2DPolygonNode* pActA = pCutA->getLeft()->getNext();
                                    B2DPolygonNode* pActB = pCutA->getRight()->getNext();

                                    while(!pCutB && pActA != pCutA->getLeft())
                                    {
                                        if(!isNextSamePos(pActA, pActB))
                                        {
                                            pCutB = getExistingCut(aTmpCuts, pActA, pActB);
                                        }

                                        pActA = pActA->getNext();
                                        pActB = pActB->getNext();
                                    }

                                    if(pCutB)
                                    {
                                        const B2DSimpleCutVector::iterator aFindResult = ::std::find(aTmpCuts.begin(), aTmpCuts.end(), pCutB);
                                        aTmpCuts.erase(aFindResult);

                                        if(isCrossover(pCutA, pCutB))
                                        {
                                            aNewCuts.push_back(pCutB);
                                        }
                                        else
                                        {
                                            delete pCutB;
                                        }
                                    }
                                }
                                else
                                {
                                    // this is a end node
                                    B2DPolygonNode* pActA = pCutA->getLeft()->getPrevious();
                                    B2DPolygonNode* pActB = pCutA->getRight()->getPrevious();

                                    while(!pCutB && pActA != pCutA->getLeft())
                                    {
                                        if(!isPrevSamePos(pActA, pActB))
                                        {
                                            pCutB = getExistingCut(aTmpCuts, pActA, pActB);
                                        }

                                        pActA = pActA->getPrevious();
                                        pActB = pActB->getPrevious();
                                    }

                                    if(pCutB)
                                    {
                                        const B2DSimpleCutVector::iterator aFindResult = ::std::find(aTmpCuts.begin(), aTmpCuts.end(), pCutB);
                                        aTmpCuts.erase(aFindResult);

                                        if(isCrossover(pCutB, pCutA))
                                        {
                                            aNewCuts.push_back(pCutB);
                                        }
                                        else
                                        {
                                            delete pCutB;
                                        }
                                    }
                                }
                            }

                            // delete cut in EVERY case
                            delete pCutA;
                        }

                        // copy new cuts to all cuts
                        aCuts.insert(aCuts.begin(), aNewCuts.begin(), aNewCuts.end());
                        aNewCuts.clear();
                    }
                }
            }

            // delete volume list again
            delete[] pVolumes;

            // are there cuts to solve? Solve them all in one run
            if(aCuts.size())
            {
                solveAllCuts(aCuts);
            }
        }
    } // end of namespace polygon
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////
// eof
