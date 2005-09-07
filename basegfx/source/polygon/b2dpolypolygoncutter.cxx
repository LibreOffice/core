/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dpolypolygoncutter.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:47:08 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGONCUTTER_HXX
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
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

#include <algorithm>

//////////////////////////////////////////////////////////////////////////////
// B2DPolygonNode

namespace basegfx
{
    class B2DPolygonNode
    {
        B2DPoint                                maPosition;
        B2DPolygonNode*                         mpPrevious;
        B2DPolygonNode*                         mpNext;

        B2DPolygonNode*                         mpListPrevious;
        B2DPolygonNode*                         mpListNext;

    public:
        B2DPolygonNode(const B2DPoint& rPosition, B2DPolygonNode* pPrevious);
        ~B2DPolygonNode();

        B2DPolygonNode* getPrevious() const { return mpPrevious; }
        B2DPolygonNode* getNext() const { return mpNext; }
        const B2DPoint& getPosition() const { return maPosition; }

        void calcMinMaxX(double& fMaxAX, double& fMinAX) const;
        void calcMinMaxY(double& fMaxAY, double& fMinAY) const;

        void swapPreviousNext() { B2DPolygonNode* pZwi = mpPrevious; mpPrevious = mpNext; mpNext = pZwi; }
        void swapNextPointers(B2DPolygonNode* pCand);

        void addToList(B2DPolygonNode*& rpList);
        void remFromList(B2DPolygonNode*& rpList);

        B2DRange getRange() const;
    };

    B2DPolygonNode::B2DPolygonNode(const B2DPoint& rPosition, B2DPolygonNode* pPrevious)
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

    B2DRange B2DPolygonNode::getRange() const
    {
        B2DRange aRetval;
        const B2DPolygonNode* pCurrent = this;

        do {
            aRetval.expand(pCurrent->getPosition());
            pCurrent = pCurrent->getPrevious();
        } while(pCurrent != this);

        return aRetval;
    }
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////
// B2DSimpleCut

namespace basegfx
{
    class B2DSimpleCut
    {
        B2DPolygonNode*                         mpLeft;
        B2DPolygonNode*                         mpRight;

    public:
        B2DSimpleCut(B2DPolygonNode* pL, B2DPolygonNode* pR)
        :   mpLeft(pL),
            mpRight(pR)
        {
        }

        void solve()
        {
            mpLeft->swapNextPointers(mpRight);
        }

        B2DPolygonNode* getLeft() const { return mpLeft; }
        B2DPolygonNode* getRight() const { return mpRight; }

        bool isSameCut(B2DPolygonNode* pA, B2DPolygonNode* pB) const
        {
            return ((pA == mpLeft && pB == mpRight) || (pB == mpLeft && pA == mpRight));
        }
    };
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////
// implementation B2DPolyPolygonCutter

namespace basegfx
{
    B2DPolyPolygonCutter::~B2DPolyPolygonCutter()
    {
        for(sal_uInt32 a(0L); a < maPolygonList.size(); a++)
        {
            delete maPolygonList[a];
        }

        maPolygonList.clear();
    }

//BFS08 void B2DPolyPolygonCutter::removeIncludedPolygons(bool bUseOr)
//BFS08 {
//BFS08     const sal_uInt32 aCount(maPolygonList.size());
//BFS08     B2DClipExtraPolygonInfo* pInfos = new B2DClipExtraPolygonInfo[aCount];
//BFS08     sal_uInt32 a, b;
//BFS08
//BFS08     // fill infos
//BFS08     for(a = 0L; a < aCount; a++)
//BFS08     {
//BFS08         pInfos[a].init(maPolygonList[a]);
//BFS08     }
//BFS08
//BFS08     // get all includes
//BFS08     for(a = 0L; a < aCount; a++)
//BFS08     {
//BFS08         B2DClipExtraPolygonInfo& rInfoA = pInfos[a];
//BFS08
//BFS08         for(b = 0L; b < aCount; b++)
//BFS08         {
//BFS08             B2DClipExtraPolygonInfo& rInfoB = pInfos[b];
//BFS08
//BFS08             if(a != b && doRangesInclude(rInfoA.getRange(), rInfoB.getRange()))
//BFS08             {
//BFS08                 // volume B in A, test pA, pB for inclusion, with border
//BFS08                 if(maPolygonList[a]->isPolygonInside(maPolygonList[b], true))
//BFS08                 {
//BFS08                     // pB is inside pA
//BFS08                     rInfoB.changeDepth(rInfoA.getOrientation());
//BFS08                 }
//BFS08             }
//BFS08         }
//BFS08     }
//BFS08
//BFS08     // delete removable
//BFS08     for(a = 0L, b = 0L; a < aCount; a++)
//BFS08     {
//BFS08         B2DClipExtraPolygonInfo& rInfo = pInfos[a];
//BFS08
//BFS08         if((bUseOr && rInfo.getDepth() != 0L) || (!bUseOr && rInfo.getDepth() < 1L))
//BFS08         {
//BFS08             B2DPolygonNodeVector::iterator aPosition(maPolygonList.begin() + b);
//BFS08             B2DPolygonNode* pCandidate = *aPosition;
//BFS08             maPolygonList.erase(aPosition);
//BFS08             deletePolygon(pCandidate);
//BFS08         }
//BFS08         else
//BFS08         {
//BFS08             b++;
//BFS08         }
//BFS08     }
//BFS08
//BFS08     // delete infos
//BFS08     delete[] pInfos;
//BFS08 }

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
            B2DPoint aPoint(rPolygon.getB2DPoint(a));
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

    void B2DPolyPolygonCutter::addPolygon(const B2DPolygon& rPolygon)
    {
        if(rPolygon.isClosed() && rPolygon.count() > 2)
        {
            B2DPolygonNode* pNew = createNewPolygon(rPolygon);
            maPolygonList.push_back(pNew);
        }
    }

    void B2DPolyPolygonCutter::addPolyPolygon(const B2DPolyPolygon& rPolyPolygon)
    {
        for(sal_uInt32 a(0L); a < rPolyPolygon.count(); a++)
        {
            B2DPolygon aCandidate = rPolyPolygon.getB2DPolygon(a);
            addPolygon(aCandidate);
        }
    }

    B2DPolyPolygon B2DPolyPolygonCutter::getPolyPolygon()
    {
        B2DPolyPolygon aRetval;
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

                aNewPolygon.setClosed(true);
                aRetval.append(aNewPolygon);
            }

            deletePolygon(pCand);
        }

        maPolygonList.clear();

        return aRetval;
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
                    if(pA->getPosition().equal(pB->getPosition()))
                    {
//BFS08                     aNewCuts.push_back(new B2DSimpleCut(pA, pB, true, pCand->getOrientation()));
                        aNewCuts.push_back(new B2DSimpleCut(pA, pB));
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

                    if(fTools::moreOrEqual(fMaxBX, fMinAX) // #116732#
                        && fTools::moreOrEqual(fMaxAX, fMinBX)) // #116732#
                    {
                        pB->calcMinMaxY(fMaxBY, fMinBY);

                        if(fTools::moreOrEqual(fMaxBY, fMinAY) // #116732#
                            && fTools::moreOrEqual(fMaxAY, fMinBY)) // #116732#
                        {
                            if(!pA->getPosition().equal(pB->getPosition()))
                            {
                                const B2DVector aVectorA(pA->getNext()->getPosition() - pA->getPosition());
                                const B2DVector aVectorB(pB->getNext()->getPosition() - pB->getPosition());

                                if(tools::findCut(pA->getPosition(), aVectorA, pB->getPosition(), aVectorB, CUTFLAG_LINE, &fCut))
                                {
                                    // crossover, two new points
                                    B2DPoint aNewPos(interpolate(pA->getPosition(), pA->getNext()->getPosition(), fCut));
                                    B2DPolygonNode* pCutLo = new B2DPolygonNode(aNewPos, pA);
                                    B2DPolygonNode* pCutHi = new B2DPolygonNode(aNewPos, pB);
//BFS08                                 aNewCuts.push_back(new B2DSimpleCut(pCutLo, pCutHi, true, pCand->getOrientation()));
                                    aNewCuts.push_back(new B2DSimpleCut(pCutLo, pCutHi));
                                    pA->calcMinMaxX(fMaxAX, fMinAX);
                                    pA->calcMinMaxY(fMaxAY, fMinAY);
                                }
                                else
                                {
                                    if(tools::isPointOnEdge(pA->getPosition(), pB->getPosition(), aVectorB, &fCut))
                                    {
                                        // startpoint A at edge B, one new point
                                        B2DPolygonNode* pCutHi = new B2DPolygonNode(pA->getPosition(), pB);
//BFS08                                     aNewCuts.push_back(new B2DSimpleCut(pA, pCutHi, true, pCand->getOrientation()));
                                        aNewCuts.push_back(new B2DSimpleCut(pA, pCutHi));
                                    }
                                    else if(tools::isPointOnEdge(pB->getPosition(), pA->getPosition(), aVectorA, &fCut))
                                    {
                                        // startpoint B at edge A, one new point
                                        B2DPolygonNode* pCutLo = new B2DPolygonNode(pB->getPosition(), pA);
//BFS08                                     aNewCuts.push_back(new B2DSimpleCut(pCutLo, pB, true, pCand->getOrientation()));
                                        aNewCuts.push_back(new B2DSimpleCut(pCutLo, pB));
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

    bool B2DPolyPolygonCutter::isCrossover(B2DPolygonNode* pA, B2DPolygonNode* pB)
    {
        // build entering vectors
        B2DVector aVecA(pA->getPrevious()->getPosition() - pA->getPosition());
        B2DVector aVecB(pB->getPrevious()->getPosition() - pA->getPosition());
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

        bool bB1Inside(fTools::more(fDegreeB1, fDegreeA1)
            && fTools::more(fDegreeA2, fDegreeB1));
        bool bB2Inside(fTools::more(fDegreeB2, fDegreeA1)
            && fTools::more(fDegreeA2, fDegreeB2));

        if(bB1Inside && bB2Inside)
        {
            return false;
        }

        bool bB1Outside(fTools::more(fDegreeA1, fDegreeB1)
            || fTools::more(fDegreeB1, fDegreeA2));
        bool bB2Outside(fTools::more(fDegreeA1, fDegreeB2)
            || fTools::more(fDegreeB2, fDegreeA2));

        return !(bB1Outside && bB2Outside);
    }

    bool B2DPolyPolygonCutter::isCrossover(B2DSimpleCut* pEnter, B2DSimpleCut* pLeave)
    {
        // build entering vectors
        B2DVector aVecJ(pEnter->getLeft()->getNext()->getPosition() - pEnter->getLeft()->getPosition());
        B2DVector aVecA(pEnter->getLeft()->getPrevious()->getPosition() - pEnter->getLeft()->getPosition());
        B2DVector aVecB(pEnter->getRight()->getPrevious()->getPosition() - pEnter->getLeft()->getPosition());
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

        bool bA2BiggerB2(fTools::more(fDegreeA2, fDegreeB2));

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

        bool bA1BiggerB1(fTools::more(fDegreeA1, fDegreeB1));

        // compare
        return (bA1BiggerB1 == bA2BiggerB2);
    }

    bool B2DPolyPolygonCutter::isNextSamePos(B2DPolygonNode* pA, B2DPolygonNode* pB)
    {
        return pA->getNext()->getPosition().equal(pB->getNext()->getPosition());
    }

    bool B2DPolyPolygonCutter::isPrevSamePos(B2DPolygonNode* pA, B2DPolygonNode* pB)
    {
        return pA->getPrevious()->getPosition().equal(pB->getPrevious()->getPosition());
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
        B2DRange* pVolumes = new B2DRange[maPolygonList.size()];

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
                if(pVolumes[a].overlaps(pVolumes[b]))
                {
                    pCandB = maPolygonList[b];
                    pA = pCandA;

                    // one run to find same start positions (so there is no need to
                    // search for existing cuts in main loop)
                    do {
                        pB = pCandB;

                        do {
                            if(pA->getPosition().equal(pB->getPosition()))
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

                            if(fTools::moreOrEqual(fMaxBX, fMinAX) // #116732#
                                && fTools::moreOrEqual(fMaxAX, fMinBX)) // #116732#
                            {
                                pB->calcMinMaxY(fMaxBY, fMinBY);

                                if(fTools::moreOrEqual(fMaxBY, fMinAY) // #116732#
                                    && fTools::moreOrEqual(fMaxAY, fMinBY)) // #116732#
                                {
                                    if(!pA->getPosition().equal(pB->getPosition()))
                                    {
                                        const B2DVector aVectorA(pA->getNext()->getPosition() - pA->getPosition());
                                        const B2DVector aVectorB(pB->getNext()->getPosition() - pB->getPosition());

                                        if(tools::findCut(pA->getPosition(), aVectorA, pB->getPosition(), aVectorB, CUTFLAG_LINE, &fCut))
                                        {
                                            // crossover, two new points, use as cutpoint
                                            B2DPoint aNewPos(interpolate(pA->getPosition(), pA->getNext()->getPosition(), fCut));
                                            B2DPolygonNode* pCutLo = new B2DPolygonNode(aNewPos, pA);
                                            B2DPolygonNode* pCutHi = new B2DPolygonNode(aNewPos, pB);
                                            aNewCuts.push_back(new B2DSimpleCut(pCutLo, pCutHi));
                                            pA->calcMinMaxX(fMaxAX, fMinAX);
                                            pA->calcMinMaxY(fMaxAY, fMinAY);
                                        }
                                        else
                                        {
                                            if(tools::isPointOnEdge(pA->getPosition(), pB->getPosition(), aVectorB, &fCut))
                                            {
                                                // startpoint A at edge B, one new point
                                                // leaves or enters common section
                                                B2DPolygonNode* pCutHi = new B2DPolygonNode(pA->getPosition(), pB);
                                                aTmpCuts.push_back(new B2DSimpleCut(pA, pCutHi));
                                            }
                                            else if(tools::isPointOnEdge(pB->getPosition(), pA->getPosition(), aVectorA, &fCut))
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
                        bool bPrevSamePos(isPrevSamePos(pCand->getLeft(), pCand->getRight()));
                        bool bNextSamePos(isNextSamePos(pCand->getLeft(), pCand->getRight()));
                        bool bDelete(false);
                        bool bIncC(true);

                        if(bPrevSamePos && bNextSamePos)
                        {
                            // single point inside continued same direction section
                            bDelete = true;
                        }
                        else if(!bPrevSamePos && !bNextSamePos)
                        {
                            // this is no same direction section, test for real cut
                            if(isCrossover(pCand->getLeft(), pCand->getRight()))
                            {
                                // real cut, move to real cutlist
                                aNewCuts.push_back(pCand);
                                aTmpCuts.erase(aTmpCuts.begin() + c);
                                bIncC = false;
                            }
                            else
                            {
                                // no cut, just a touch in one point
                                bDelete = true;
                            }
                        }

                        // delete if wanted
                        if(bDelete)
                        {
                            delete pCand;
                            aTmpCuts.erase(aTmpCuts.begin() + c);
                            bIncC = false;
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
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////
// eof
