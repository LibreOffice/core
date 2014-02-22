/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <basegfx/polygon/b2dpolygontriangulator.hxx>
#include <osl/diagnose.h>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/numeric/ftools.hxx>

#include <algorithm>



namespace basegfx
{
    namespace
    {
        class EdgeEntry
        {
            EdgeEntry*                              mpNext;
            B2DPoint                                maStart;
            B2DPoint                                maEnd;
            double                                  mfAtan2;

        public:
            EdgeEntry(const B2DPoint& rStart, const B2DPoint& rEnd)
            :   mpNext(0L),
                maStart(rStart),
                maEnd(rEnd),
                mfAtan2(0.0)
            {
                
                bool bSwap(false);

                if(::basegfx::fTools::equal(maStart.getY(), maEnd.getY()))
                {
                    if(maStart.getX() > maEnd.getX())
                    {
                        bSwap = true;
                    }
                }
                else if(maStart.getY() > maEnd.getY())
                {
                    bSwap = true;
                }

                if(bSwap)
                {
                    maStart = rEnd;
                    maEnd = rStart;
                }

                mfAtan2 = atan2(maEnd.getY() - maStart.getY(), maEnd.getX() - maStart.getX());
            }

            ~EdgeEntry()
            {
            }

            bool operator<(const EdgeEntry& rComp) const
            {
                if(::basegfx::fTools::equal(maStart.getY(), rComp.maStart.getY()))
                {
                    if(::basegfx::fTools::equal(maStart.getX(), rComp.maStart.getX()))
                    {
                        
                        return (mfAtan2 > rComp.mfAtan2);
                    }

                    return (maStart.getX() < rComp.maStart.getX());
                }

                return (maStart.getY() < rComp.maStart.getY());
            }

            bool operator==(const EdgeEntry& rComp) const
            {
                return (maStart.equal(rComp.maStart) && maEnd.equal(rComp.maEnd));
            }

            bool operator!=(const EdgeEntry& rComp) const
            {
                return !(*this == rComp);
            }

            const B2DPoint& getStart() const { return maStart; }
            const B2DPoint& getEnd() const { return maEnd; }

            EdgeEntry* getNext() const { return mpNext; }
            void setNext(EdgeEntry* pNext) { mpNext = pNext; }
        };

        

        typedef ::std::vector< EdgeEntry > EdgeEntries;
        typedef ::std::vector< EdgeEntry* > EdgeEntryPointers;

        

        class Triangulator
        {
            EdgeEntry*                                      mpList;
            EdgeEntries                                     maStartEntries;
            EdgeEntryPointers                               maNewEdgeEntries;
            B2DPolygon                                      maResult;

            void handleClosingEdge(const B2DPoint& rStart, const B2DPoint& rEnd);
            bool CheckPointInTriangle(EdgeEntry* pEdgeA, EdgeEntry* pEdgeB, const B2DPoint& rTestPoint);
            void createTriangle(const B2DPoint& rA, const B2DPoint& rB, const B2DPoint& rC);

        public:
            explicit Triangulator(const B2DPolyPolygon& rCandidate);
            ~Triangulator();

            const B2DPolygon getResult() const { return maResult; }
        };

        void Triangulator::handleClosingEdge(const B2DPoint& rStart, const B2DPoint& rEnd)
        {
            
            EdgeEntry aNew(rStart, rEnd);
            EdgeEntry* pCurr = mpList;
            EdgeEntry* pPrev = 0L;

            while(pCurr
                && pCurr->getStart().getY() <= aNew.getStart().getY()
                && *pCurr != aNew)
            {
                pPrev = pCurr;
                pCurr = pCurr->getNext();
            }

            if(pCurr && *pCurr == aNew)
            {
                
                if(pPrev)
                {
                    pPrev->setNext(pCurr->getNext());
                }
                else
                {
                    mpList = pCurr->getNext();
                }
            }
            else
            {
                
                EdgeEntry* pNew = new EdgeEntry(aNew);
                maNewEdgeEntries.push_back(pNew);
                pCurr = mpList;
                pPrev = 0L;

                while(pCurr && *pCurr < *pNew)
                {
                    pPrev = pCurr;
                    pCurr = pCurr->getNext();
                }

                if(pPrev)
                {
                    pNew->setNext(pPrev->getNext());
                    pPrev->setNext(pNew);
                }
                else
                {
                    pNew->setNext(mpList);
                    mpList = pNew;
                }
            }
        }

        bool Triangulator::CheckPointInTriangle(EdgeEntry* pEdgeA, EdgeEntry* pEdgeB, const B2DPoint& rTestPoint)
        {
            
            if(tools::isPointInTriangle(pEdgeA->getStart(), pEdgeA->getEnd(), pEdgeB->getEnd(), rTestPoint, true))
            {
                
                if(!rTestPoint.equal(pEdgeA->getEnd()) && !rTestPoint.equal(pEdgeB->getEnd()))
                {
                    
                    EdgeEntry* pStart = new EdgeEntry(pEdgeA->getStart(), rTestPoint);
                    EdgeEntry* pEnd = new EdgeEntry(*pStart);
                    maNewEdgeEntries.push_back(pStart);
                    maNewEdgeEntries.push_back(pEnd);

                    pStart->setNext(pEnd);
                    pEnd->setNext(pEdgeA->getNext());
                    pEdgeA->setNext(pStart);

                    return false;
                }
            }

            return true;
        }

        void Triangulator::createTriangle(const B2DPoint& rA, const B2DPoint& rB, const B2DPoint& rC)
        {
            maResult.append(rA);
            maResult.append(rB);
            maResult.append(rC);
        }

        
        Triangulator::Triangulator(const B2DPolyPolygon& rCandidate)
        :   mpList(0L)
        {
            
            
            if(rCandidate.count())
            {
                for(sal_uInt32 a(0L); a < rCandidate.count(); a++)
                {
                    const B2DPolygon aPolygonCandidate(rCandidate.getB2DPolygon(a));
                    const sal_uInt32 nCount(aPolygonCandidate.count());

                    if(nCount > 2L)
                    {
                        B2DPoint aPrevPnt(aPolygonCandidate.getB2DPoint(nCount - 1L));

                        for(sal_uInt32 b(0L); b < nCount; b++)
                        {
                            B2DPoint aNextPnt(aPolygonCandidate.getB2DPoint(b));

                            if( !aPrevPnt.equal(aNextPnt) )
                            {
                                maStartEntries.push_back(EdgeEntry(aPrevPnt, aNextPnt));
                            }

                            aPrevPnt = aNextPnt;
                        }
                    }
                }

                if(!maStartEntries.empty())
                {
                    
                    ::std::sort(maStartEntries.begin(), maStartEntries.end());

                    
                    EdgeEntries::iterator aPos(maStartEntries.begin());
                    mpList = &(*aPos++);
                    EdgeEntry* pLast = mpList;

                    while(aPos != maStartEntries.end())
                    {
                        EdgeEntry* pEntry = &(*aPos++);
                        pLast->setNext(pEntry);
                        pLast = pEntry;
                    }
                }
            }

            while(mpList)
            {
                if(mpList->getNext() && mpList->getNext()->getStart().equal(mpList->getStart()))
                {
                    
                    
                    EdgeEntry* pEdgeA = mpList;
                    EdgeEntry* pEdgeB = pEdgeA->getNext();

                    if( pEdgeA->getEnd().equal(pEdgeB->getEnd()) )
                    {
                        
                        mpList = pEdgeB->getNext();
                    }
                    else
                    {
                        const B2DVector aLeft(pEdgeA->getEnd() - pEdgeA->getStart());
                        const B2DVector aRight(pEdgeB->getEnd() - pEdgeA->getStart());

                        if(ORIENTATION_NEUTRAL == getOrientation(aLeft, aRight))
                        {
                            
                            
                            mpList = pEdgeB->getNext();
                            handleClosingEdge(pEdgeA->getEnd(), pEdgeB->getEnd());
                        }
                        else
                        {
                            
                            B2DRange aRange(pEdgeA->getStart(), pEdgeA->getEnd());
                            aRange.expand(pEdgeB->getEnd());
                            EdgeEntry* pTestEdge = pEdgeB->getNext();
                            bool bNoPointInTriangle(true);

                            
                            while(bNoPointInTriangle && pTestEdge)
                            {
                                if(aRange.getMaxY() < pTestEdge->getStart().getY())
                                {
                                    
                                    break;
                                }
                                else
                                {
                                    
                                    if(!pTestEdge->getStart().equal(pEdgeA->getStart()))
                                    {
                                        if(aRange.isInside(pTestEdge->getStart()))
                                        {
                                            bNoPointInTriangle = CheckPointInTriangle(pEdgeA, pEdgeB, pTestEdge->getStart());
                                        }
                                    }
                                }

                                
                                pTestEdge = pTestEdge->getNext();
                            }

                            if(bNoPointInTriangle)
                            {
                                
                                pTestEdge = pEdgeB->getNext();

                                while(bNoPointInTriangle && pTestEdge)
                                {
                                    if(aRange.getMaxY() < pTestEdge->getStart().getY())
                                    {
                                        
                                        break;
                                    }
                                    else
                                    {
                                        
                                        if(!pTestEdge->getEnd().equal(pEdgeA->getStart()))
                                        {
                                            if(aRange.isInside(pTestEdge->getEnd()))
                                            {
                                                bNoPointInTriangle = CheckPointInTriangle(pEdgeA, pEdgeB, pTestEdge->getEnd());
                                            }
                                        }
                                    }

                                    
                                    pTestEdge = pTestEdge->getNext();
                                }
                            }

                            if(bNoPointInTriangle)
                            {
                                
                                mpList = pEdgeB->getNext();
                                createTriangle(pEdgeA->getStart(), pEdgeB->getEnd(), pEdgeA->getEnd());
                                handleClosingEdge(pEdgeA->getEnd(), pEdgeB->getEnd());
                            }
                        }
                    }
                }
                else
                {
                    
                    mpList = mpList->getNext();
                }
            }
        }

        Triangulator::~Triangulator()
        {
            EdgeEntryPointers::iterator aIter(maNewEdgeEntries.begin());

            while(aIter != maNewEdgeEntries.end())
            {
                delete (*aIter++);
            }
        }

    } 
} 



namespace basegfx
{
    namespace triangulator
    {
        B2DPolygon triangulate(const B2DPolygon& rCandidate)
        {
            B2DPolygon aRetval;

            
            B2DPolygon aCandidate(rCandidate.areControlPointsUsed() ? tools::adaptiveSubdivideByAngle(rCandidate) : rCandidate);
            aCandidate.removeDoublePoints();
            aCandidate = tools::removeNeutralPoints(aCandidate);

            if(2L == aCandidate.count())
            {
                
                aRetval.append(aCandidate);
            }
            else if(aCandidate.count() > 2L)
            {
                if(tools::isConvex(aCandidate))
                {
                    
                    tools::addTriangleFan(aCandidate, aRetval);
                }
                else
                {
                    
                    const B2DPolyPolygon aCandPolyPoly(aCandidate);
                    Triangulator aTriangulator(aCandPolyPoly);
                    aRetval = aTriangulator.getResult();
                }
            }

            return aRetval;
        }

        B2DPolygon triangulate(const B2DPolyPolygon& rCandidate)
        {
            B2DPolygon aRetval;

            
            B2DPolyPolygon aCandidate(rCandidate.areControlPointsUsed() ? tools::adaptiveSubdivideByAngle(rCandidate) : rCandidate);

            if(1L == aCandidate.count())
            {
                
                const B2DPolygon aSinglePolygon(aCandidate.getB2DPolygon(0L));
                aRetval = triangulate(aSinglePolygon);
            }
            else
            {
                Triangulator aTriangulator(aCandidate);
                aRetval = aTriangulator.getResult();
            }

            return aRetval;
        }
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
