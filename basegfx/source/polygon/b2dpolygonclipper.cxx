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

#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <osl/diagnose.h>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <basegfx/polygon/b2dpolygoncutandtouch.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>
#include <basegfx/tools/rectcliptools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>



namespace basegfx
{
    namespace tools
    {
        B2DPolyPolygon clipPolygonOnParallelAxis(const B2DPolygon& rCandidate, bool bParallelToXAxis, bool bAboveAxis, double fValueOnOtherAxis, bool bStroke)
        {
            B2DPolyPolygon aRetval;

            if(rCandidate.count())
            {
                const B2DRange aCandidateRange(getRange(rCandidate));

                if(bParallelToXAxis && fTools::moreOrEqual(aCandidateRange.getMinY(), fValueOnOtherAxis))
                {
                    
                    if(bAboveAxis)
                    {
                        
                        aRetval.append(rCandidate);
                    }
                }
                else if(bParallelToXAxis && fTools::lessOrEqual(aCandidateRange.getMaxY(), fValueOnOtherAxis))
                {
                    
                    if(!bAboveAxis)
                    {
                        
                        aRetval.append(rCandidate);
                    }
                }
                else if(!bParallelToXAxis && fTools::moreOrEqual(aCandidateRange.getMinX(), fValueOnOtherAxis))
                {
                    
                    if(bAboveAxis)
                    {
                        
                        aRetval.append(rCandidate);
                    }
                }
                else if(!bParallelToXAxis && fTools::lessOrEqual(aCandidateRange.getMaxX(), fValueOnOtherAxis))
                {
                    
                    if(!bAboveAxis)
                    {
                        
                        aRetval.append(rCandidate);
                    }
                }
                else
                {
                    
                    
                    
                    
                    
                    const double fSmallExtension((aCandidateRange.getWidth() + aCandidateRange.getHeight()) * (0.5 * 0.1));
                    const B2DPoint aStart(
                        bParallelToXAxis ? aCandidateRange.getMinX() - fSmallExtension : fValueOnOtherAxis,
                        bParallelToXAxis ? fValueOnOtherAxis : aCandidateRange.getMinY() - fSmallExtension);
                    const B2DPoint aEnd(
                        bParallelToXAxis ? aCandidateRange.getMaxX() + fSmallExtension : fValueOnOtherAxis,
                        bParallelToXAxis ? fValueOnOtherAxis : aCandidateRange.getMaxY() + fSmallExtension);
                    const B2DPolygon aCandidate(addPointsAtCuts(rCandidate, aStart, aEnd));
                    const sal_uInt32 nPointCount(aCandidate.count());
                    const sal_uInt32 nEdgeCount(aCandidate.isClosed() ? nPointCount : nPointCount - 1L);
                    B2DCubicBezier aEdge;
                    B2DPolygon aRun;

                    for(sal_uInt32 a(0L); a < nEdgeCount; a++)
                    {
                        aCandidate.getBezierSegment(a, aEdge);
                        const B2DPoint aTestPoint(aEdge.interpolatePoint(0.5));
                        const bool bInside(bParallelToXAxis ?
                            fTools::moreOrEqual(aTestPoint.getY(), fValueOnOtherAxis) == bAboveAxis :
                            fTools::moreOrEqual(aTestPoint.getX(), fValueOnOtherAxis) == bAboveAxis);

                        if(bInside)
                        {
                            if(!aRun.count() || !aRun.getB2DPoint(aRun.count() - 1).equal(aEdge.getStartPoint()))
                            {
                                aRun.append(aEdge.getStartPoint());
                            }

                            if(aEdge.isBezier())
                            {
                                aRun.appendBezierSegment(aEdge.getControlPointA(), aEdge.getControlPointB(), aEdge.getEndPoint());
                            }
                            else
                            {
                                aRun.append(aEdge.getEndPoint());
                            }
                        }
                        else
                        {
                            if(bStroke && aRun.count())
                            {
                                aRetval.append(aRun);
                                aRun.clear();
                            }
                        }
                    }

                    if(aRun.count())
                    {
                        if(bStroke)
                        {
                            
                            
                            if(aRetval.count())
                            {
                                const B2DPolygon aStartPolygon(aRetval.getB2DPolygon(0));

                                if(aStartPolygon.count() && aStartPolygon.getB2DPoint(0).equal(aRun.getB2DPoint(aRun.count() - 1)))
                                {
                                    
                                    aRun.append(aStartPolygon); aRun.removeDoublePoints();
                                    aRetval.remove(0);
                                }
                            }

                            aRetval.append(aRun);
                        }
                        else
                        {
                            
                            closeWithGeometryChange(aRun);
                            aRetval.append(aRun);
                        }
                    }
                }
            }

            return aRetval;
        }

        B2DPolyPolygon clipPolyPolygonOnParallelAxis(const B2DPolyPolygon& rCandidate, bool bParallelToXAxis, bool bAboveAxis, double fValueOnOtherAxis, bool bStroke)
        {
            const sal_uInt32 nPolygonCount(rCandidate.count());
            B2DPolyPolygon aRetval;

            for(sal_uInt32 a(0L); a < nPolygonCount; a++)
            {
                const B2DPolyPolygon aClippedPolyPolygon(clipPolygonOnParallelAxis(rCandidate.getB2DPolygon(a), bParallelToXAxis, bAboveAxis, fValueOnOtherAxis, bStroke));

                if(aClippedPolyPolygon.count())
                {
                    aRetval.append(aClippedPolyPolygon);
                }
            }

            return aRetval;
        }

        B2DPolyPolygon clipPolygonOnRange(const B2DPolygon& rCandidate, const B2DRange& rRange, bool bInside, bool bStroke)
        {
            const sal_uInt32 nCount(rCandidate.count());
            B2DPolyPolygon aRetval;

            if(!nCount)
            {
                
                return aRetval;
            }

            if(rRange.isEmpty())
            {
                if(bInside)
                {
                    
                    return aRetval;
                }
                else
                {
                    
                    return B2DPolyPolygon(rCandidate);
                }
            }

            const B2DRange aCandidateRange(getRange(rCandidate));

            if(rRange.isInside(aCandidateRange))
            {
                  
                if(bInside)
                {
                    
                    return B2DPolyPolygon(rCandidate);
                }
                else
                {
                    
                    return aRetval;
                }
            }

            if(!bInside)
            {
                
                
                
                
                
                
                
                
                
                const B2DPolygon aClip(createPolygonFromRect(rRange));

                return clipPolygonOnPolyPolygon(rCandidate, B2DPolyPolygon(aClip), bInside, bStroke);
            }

            
            
            aRetval = clipPolygonOnParallelAxis(rCandidate, true, bInside, rRange.getMinY(), bStroke);

            if(aRetval.count())
            {
                
                if(1L == aRetval.count())
                {
                    aRetval = clipPolygonOnParallelAxis(aRetval.getB2DPolygon(0L), false, bInside, rRange.getMinX(), bStroke);
                }
                else
                {
                    aRetval = clipPolyPolygonOnParallelAxis(aRetval, false, bInside, rRange.getMinX(), bStroke);
                }

                if(aRetval.count())
                {
                    
                    if(1L == aRetval.count())
                    {
                        aRetval = clipPolygonOnParallelAxis(aRetval.getB2DPolygon(0L), true, !bInside, rRange.getMaxY(), bStroke);
                    }
                    else
                    {
                        aRetval = clipPolyPolygonOnParallelAxis(aRetval, true, !bInside, rRange.getMaxY(), bStroke);
                    }

                    if(aRetval.count())
                    {
                        
                        if(1L == aRetval.count())
                        {
                            aRetval = clipPolygonOnParallelAxis(aRetval.getB2DPolygon(0L), false, !bInside, rRange.getMaxX(), bStroke);
                        }
                        else
                        {
                            aRetval = clipPolyPolygonOnParallelAxis(aRetval, false, !bInside, rRange.getMaxX(), bStroke);
                        }
                    }
                }
            }

            return aRetval;
        }

        B2DPolyPolygon clipPolyPolygonOnRange(const B2DPolyPolygon& rCandidate, const B2DRange& rRange, bool bInside, bool bStroke)
        {
            const sal_uInt32 nPolygonCount(rCandidate.count());
            B2DPolyPolygon aRetval;

            if(!nPolygonCount)
            {
                
                return aRetval;
            }

            if(rRange.isEmpty())
            {
                if(bInside)
                {
                    
                    return aRetval;
                }
                else
                {
                    
                    return rCandidate;
                }
            }

            if(bInside)
            {
                for(sal_uInt32 a(0L); a < nPolygonCount; a++)
                {
                    const B2DPolyPolygon aClippedPolyPolygon(clipPolygonOnRange(rCandidate.getB2DPolygon(a), rRange, bInside, bStroke));

                    if(aClippedPolyPolygon.count())
                    {
                        aRetval.append(aClippedPolyPolygon);
                    }
                }
            }
            else
            {
                
                
                const B2DPolygon aClip(createPolygonFromRect(rRange));

                return clipPolyPolygonOnPolyPolygon(rCandidate, B2DPolyPolygon(aClip), bInside, bStroke);
            }

            return aRetval;
        }

        

        B2DPolyPolygon clipPolyPolygonOnPolyPolygon(const B2DPolyPolygon& rCandidate, const B2DPolyPolygon& rClip, bool bInside, bool bStroke)
        {
            B2DPolyPolygon aRetval;

            if(rCandidate.count() && rClip.count())
            {
                if(bStroke)
                {
                    
                    
                    
                    for(sal_uInt32 a(0); a < rCandidate.count(); a++)
                    {
                        
                        const B2DPolygon aCandidate(addPointsAtCuts(rCandidate.getB2DPolygon(a), rClip));
                        const sal_uInt32 nPointCount(aCandidate.count());
                        const sal_uInt32 nEdgeCount(aCandidate.isClosed() ? nPointCount : nPointCount - 1L);
                        B2DCubicBezier aEdge;
                        B2DPolygon aRun;

                        for(sal_uInt32 b(0); b < nEdgeCount; b++)
                        {
                            aCandidate.getBezierSegment(b, aEdge);
                            const B2DPoint aTestPoint(aEdge.interpolatePoint(0.5));
                            const bool bIsInside(tools::isInside(rClip, aTestPoint) == bInside);

                            if(bIsInside)
                            {
                                if(!aRun.count())
                                {
                                    aRun.append(aEdge.getStartPoint());
                                }

                                if(aEdge.isBezier())
                                {
                                    aRun.appendBezierSegment(aEdge.getControlPointA(), aEdge.getControlPointB(), aEdge.getEndPoint());
                                }
                                else
                                {
                                    aRun.append(aEdge.getEndPoint());
                                }
                            }
                            else
                            {
                                if(aRun.count())
                                {
                                    aRetval.append(aRun);
                                    aRun.clear();
                                }
                            }
                        }

                        if(aRun.count())
                        {
                            
                            
                            if(aRetval.count())
                            {
                                const B2DPolygon aStartPolygon(aRetval.getB2DPolygon(0));

                                if(aStartPolygon.count() && aStartPolygon.getB2DPoint(0).equal(aRun.getB2DPoint(aRun.count() - 1)))
                                {
                                    
                                    aRun.append(aStartPolygon); aRun.removeDoublePoints();
                                    aRetval.remove(0);
                                }
                            }

                            aRetval.append(aRun);
                        }
                    }
                }
                else
                {
                    
                    B2DPolyPolygon aMergePolyPolygonA(rClip);

                    
                    
                    
                    
                    
                    
                    aMergePolyPolygonA = solveCrossovers(aMergePolyPolygonA);
                    aMergePolyPolygonA = stripNeutralPolygons(aMergePolyPolygonA);
                    aMergePolyPolygonA = correctOrientations(aMergePolyPolygonA);

                    if(!bInside)
                    {
                        
                        
                        aMergePolyPolygonA.flip();
                    }

                    B2DPolyPolygon aMergePolyPolygonB(rCandidate);

                    
                    aMergePolyPolygonB = solveCrossovers(aMergePolyPolygonB);
                    aMergePolyPolygonB = stripNeutralPolygons(aMergePolyPolygonB);
                    aMergePolyPolygonB = correctOrientations(aMergePolyPolygonB);

                    
                    
                    
                    aRetval.append(aMergePolyPolygonA);
                    aRetval.append(aMergePolyPolygonB);
                    aRetval = solveCrossovers(aRetval);

                    
                    
                    
                    
                    
                    
                    aRetval = stripNeutralPolygons(aRetval);
                    aRetval = stripDispensablePolygons(aRetval, bInside);
                }
            }

            return aRetval;
        }

        

        B2DPolyPolygon clipPolygonOnPolyPolygon(const B2DPolygon& rCandidate, const B2DPolyPolygon& rClip, bool bInside, bool bStroke)
        {
            B2DPolyPolygon aRetval;

            if(rCandidate.count() && rClip.count())
            {
                aRetval = clipPolyPolygonOnPolyPolygon(B2DPolyPolygon(rCandidate), rClip, bInside, bStroke);
            }

            return aRetval;
        }

        

        /*
        * let a plane be defined as
        *
        *     v.n+d=0
        *
        * and a ray be defined as
        *
        *     a+(b-a)*t=0
        *
        * substitute and rearranging yields
        *
        *     t = -(a.n+d)/(n.(b-a))
        *
        * if the denominator is zero, the line is either
        * contained in the plane or parallel to the plane.
        * in either case, there is no intersection.
        * if numerator and denominator are both zero, the
        * ray is contained in the plane.
        *
        */
        struct scissor_plane {
            double nx,ny;           
            double d;               
            sal_uInt32 clipmask;    
        };

        /*
        *
        * polygon clipping rules  (straight out of Foley and Van Dam)
        * ===========================================================
        * current   |next       |emit
        * ____________________________________
        * inside    |inside     |next
        * inside    |outside    |intersect with clip plane
        * outside   |outside    |nothing
        * outside   |inside     |intersect with clip plane follwed by next
        *
        */
        sal_uInt32 scissorLineSegment( ::basegfx::B2DPoint           *in_vertex,    
                                       sal_uInt32                     in_count,     
                                       ::basegfx::B2DPoint           *out_vertex,   
                                       scissor_plane                 *pPlane,       
                                       const ::basegfx::B2DRectangle &rR )          
        {
            ::basegfx::B2DPoint *curr;
            ::basegfx::B2DPoint *next;

            sal_uInt32 out_count=0;

            
            for(sal_uInt32 i=0; i<in_count; i++) {

                
                
                curr = &in_vertex[i];
                next = &in_vertex[(i+1)%in_count];

                
                sal_uInt32 clip = pPlane->clipmask & ((getCohenSutherlandClipFlags(*curr,rR)<<4)|getCohenSutherlandClipFlags(*next,rR));

                if(clip==0) { 
                    out_vertex[out_count++] = *next;
                }
                else if((clip&0x0f) && (clip&0xf0)) { 
                }
                else if((clip&0x0f) && (clip&0xf0)==0) { 

                    
                    
                    ::basegfx::B2DPoint dir((*next)-(*curr));

                    double denominator = ( pPlane->nx*dir.getX() +
                                        pPlane->ny*dir.getY() );
                    double numerator = ( pPlane->nx*curr->getX() +
                                        pPlane->ny*curr->getY() +
                                        pPlane->d );
                    double t = -numerator/denominator;

                    
                    ::basegfx::B2DPoint intersection( curr->getX()+t*dir.getX(),
                                                    curr->getY()+t*dir.getY() );

                    out_vertex[out_count++] = intersection;
                }
                else if((clip&0x0f)==0 && (clip&0xf0)) { 

                    
                    
                    ::basegfx::B2DPoint dir((*next)-(*curr));

                    double denominator = ( pPlane->nx*dir.getX() +
                                        pPlane->ny*dir.getY() );
                    double numerator = ( pPlane->nx*curr->getX() +
                                        pPlane->ny*curr->getY() +
                                        pPlane->d );
                    double t = -numerator/denominator;

                    
                    ::basegfx::B2DPoint intersection( curr->getX()+t*dir.getX(),
                                                    curr->getY()+t*dir.getY() );

                    out_vertex[out_count++] = intersection;
                    out_vertex[out_count++] = *next;
                }
            }

            return out_count;
        }

        B2DPolygon clipTriangleListOnRange( const B2DPolygon& rCandidate,
                                            const B2DRange&   rRange )
        {
            B2DPolygon aResult;

            if( !(rCandidate.count()%3) )
            {
                const int scissor_plane_count = 4;

                scissor_plane sp[scissor_plane_count];

                sp[0].nx = +1.0;
                sp[0].ny = +0.0;
                sp[0].d = -(rRange.getMinX());
                sp[0].clipmask = (RectClipFlags::LEFT << 4) | RectClipFlags::LEFT; 
                sp[1].nx = -1.0;
                sp[1].ny = +0.0;
                sp[1].d = +(rRange.getMaxX());
                sp[1].clipmask = (RectClipFlags::RIGHT << 4) | RectClipFlags::RIGHT; 
                sp[2].nx = +0.0;
                sp[2].ny = +1.0;
                sp[2].d = -(rRange.getMinY());
                sp[2].clipmask = (RectClipFlags::TOP << 4) | RectClipFlags::TOP; 
                sp[3].nx = +0.0;
                sp[3].ny = -1.0;
                sp[3].d = +(rRange.getMaxY());
                sp[3].clipmask = (RectClipFlags::BOTTOM << 4) | RectClipFlags::BOTTOM; 

                
                const sal_uInt32 nVertexCount = rCandidate.count();

                if(nVertexCount)
                {
                    
                    
                    
                    //
                    
                    
                    //
                    
                    
                    //
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    //
                    
                    
                    
                    
                    
                    
                    
                    
                    //
                    
                    
                    
                    
                    
                    
                    
                    //
                    
                    
                    //
                    
                    
                    
                    
                    //
                    
                    //
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    //
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    //
                    
                    
                    

                    
                    
                    

                    
                    
                    
                    
                    
                    ::basegfx::B2DPoint stack[3];
                    unsigned int clipflag = 0;

                    for(sal_uInt32 nIndex=0; nIndex<nVertexCount; ++nIndex)
                    {
                        
                        stack[0] = stack[1];
                        stack[1] = stack[2];
                        stack[2] = rCandidate.getB2DPoint(nIndex);

                        
                        clipflag |= unsigned(!(rRange.isInside(stack[2])));

                        if(nIndex > 1)
                        {
                            
                            if(!((nIndex+1)%3))
                            {
                                
                                
                                if(clipflag & 7)
                                {
                                    ::basegfx::B2DPoint buf0[16];
                                    ::basegfx::B2DPoint buf1[16];

                                    sal_uInt32 vertex_count = 3;

                                    
                                    
                                    vertex_count = scissorLineSegment(stack,vertex_count,buf1,&sp[0],rRange);
                                    vertex_count = scissorLineSegment(buf1,vertex_count,buf0,&sp[1],rRange);
                                    vertex_count = scissorLineSegment(buf0,vertex_count,buf1,&sp[2],rRange);
                                    vertex_count = scissorLineSegment(buf1,vertex_count,buf0,&sp[3],rRange);

                                    if(vertex_count >= 3)
                                    {
                                        
                                        ::basegfx::B2DPoint v0(buf0[0]);
                                        ::basegfx::B2DPoint v1(buf0[1]);
                                        for(sal_uInt32 i=2; i<vertex_count; ++i)
                                        {
                                            ::basegfx::B2DPoint v2(buf0[i]);
                                            aResult.append(v0);
                                            aResult.append(v1);
                                            aResult.append(v2);
                                            v1 = v2;
                                        }
                                    }
                                }
                                else
                                {
                                    
                                    for(sal_uInt32 i=0; i<3; ++i)
                                        aResult.append(stack[i]);
                                }
                            }
                        }

                        clipflag <<= 1;
                    }
                }
            }

            return aResult;
        }

        

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
