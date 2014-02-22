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

#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <rtl/ustring.hxx>
#include <rtl/math.hxx>
#include <stringconversiontools.hxx>

namespace basegfx
{
    namespace tools
    {
        bool PointIndex::operator<(const PointIndex& rComp) const
        {
            if(rComp.getPolygonIndex() == getPolygonIndex())
            {
                return rComp.getPointIndex() < getPointIndex();
            }

            return rComp.getPolygonIndex() < getPolygonIndex();
        }

        bool importFromSvgD(
            B2DPolyPolygon& o_rPolyPolygon,
            const OUString& rSvgDStatement,
            bool bHandleRelativeNextPointCompatible,
            PointIndexSet* pHelpPointIndexSet)
        {
            o_rPolyPolygon.clear();
            const sal_Int32 nLen(rSvgDStatement.getLength());
            sal_Int32 nPos(0);
            double nLastX( 0.0 );
            double nLastY( 0.0 );
            B2DPolygon aCurrPoly;

            
            ::basegfx::internal::lcl_skipSpaces(nPos, rSvgDStatement, nLen);

            while(nPos < nLen)
            {
                bool bRelative(false);
                const sal_Unicode aCurrChar(rSvgDStatement[nPos]);

                if(o_rPolyPolygon.count() && !aCurrPoly.count() && !('m' == aCurrChar || 'M' == aCurrChar))
                {
                    
                    
                    
                    aCurrPoly.append(B2DPoint(nLastX, nLastY));
                }

                switch(aCurrChar)
                {
                    case 'z' :
                    case 'Z' :
                    {
                        
                        nPos++;
                        ::basegfx::internal::lcl_skipSpaces(nPos, rSvgDStatement, nLen);

                        
                        if(aCurrPoly.count())
                        {
                            if(!bHandleRelativeNextPointCompatible)
                            {
                                
                                
                                
                                nLastX = aCurrPoly.getB2DPoint(0).getX();
                                nLastY = aCurrPoly.getB2DPoint(0).getY();
                            }

                            aCurrPoly.setClosed(true);
                            o_rPolyPolygon.append(aCurrPoly);
                            aCurrPoly.clear();
                        }

                        break;
                    }

                    case 'm' :
                    case 'M' :
                    {
                        
                        if(aCurrPoly.count())
                        {
                            o_rPolyPolygon.append(aCurrPoly);
                            aCurrPoly.clear();
                        }

                        
                    }
                    case 'l' :
                    case 'L' :
                    {
                        if('m' == aCurrChar || 'l' == aCurrChar)
                        {
                            bRelative = true;
                        }

                        
                        nPos++;
                        ::basegfx::internal::lcl_skipSpaces(nPos, rSvgDStatement, nLen);

                        while(nPos < nLen && ::basegfx::internal::lcl_isOnNumberChar(rSvgDStatement, nPos))
                        {
                            double nX, nY;

                            if(!::basegfx::internal::lcl_importDoubleAndSpaces(nX, nPos, rSvgDStatement, nLen)) return false;
                            if(!::basegfx::internal::lcl_importDoubleAndSpaces(nY, nPos, rSvgDStatement, nLen)) return false;

                            if(bRelative)
                            {
                                nX += nLastX;
                                nY += nLastY;
                            }

                            
                            nLastX = nX;
                            nLastY = nY;

                            
                            aCurrPoly.append(B2DPoint(nX, nY));
                        }
                        break;
                    }

                    case 'h' :
                    {
                        bRelative = true;
                        
                    }
                    case 'H' :
                    {
                        nPos++;
                        ::basegfx::internal::lcl_skipSpaces(nPos, rSvgDStatement, nLen);

                        while(nPos < nLen && ::basegfx::internal::lcl_isOnNumberChar(rSvgDStatement, nPos))
                        {
                            double nX, nY(nLastY);

                            if(!::basegfx::internal::lcl_importDoubleAndSpaces(nX, nPos, rSvgDStatement, nLen)) return false;

                            if(bRelative)
                            {
                                nX += nLastX;
                            }

                            
                            nLastX = nX;

                            
                            aCurrPoly.append(B2DPoint(nX, nY));
                        }
                        break;
                    }

                    case 'v' :
                    {
                        bRelative = true;
                        
                    }
                    case 'V' :
                    {
                        nPos++;
                        ::basegfx::internal::lcl_skipSpaces(nPos, rSvgDStatement, nLen);

                        while(nPos < nLen && ::basegfx::internal::lcl_isOnNumberChar(rSvgDStatement, nPos))
                        {
                            double nX(nLastX), nY;

                            if(!::basegfx::internal::lcl_importDoubleAndSpaces(nY, nPos, rSvgDStatement, nLen)) return false;

                            if(bRelative)
                            {
                                nY += nLastY;
                            }

                            
                            nLastY = nY;

                            
                            aCurrPoly.append(B2DPoint(nX, nY));
                        }
                        break;
                    }

                    case 's' :
                    {
                        bRelative = true;
                        
                    }
                    case 'S' :
                    {
                        nPos++;
                        ::basegfx::internal::lcl_skipSpaces(nPos, rSvgDStatement, nLen);

                        while(nPos < nLen && ::basegfx::internal::lcl_isOnNumberChar(rSvgDStatement, nPos))
                        {
                            double nX, nY;
                            double nX2, nY2;

                            if(!::basegfx::internal::lcl_importDoubleAndSpaces(nX2, nPos, rSvgDStatement, nLen)) return false;
                            if(!::basegfx::internal::lcl_importDoubleAndSpaces(nY2, nPos, rSvgDStatement, nLen)) return false;
                            if(!::basegfx::internal::lcl_importDoubleAndSpaces(nX, nPos, rSvgDStatement, nLen)) return false;
                            if(!::basegfx::internal::lcl_importDoubleAndSpaces(nY, nPos, rSvgDStatement, nLen)) return false;

                            if(bRelative)
                            {
                                nX2 += nLastX;
                                nY2 += nLastY;
                                nX += nLastX;
                                nY += nLastY;
                            }

                            
                            if(!aCurrPoly.count())
                            {
                                aCurrPoly.append(B2DPoint(nLastX, nLastY));
                            }

                            
                            
                            B2DPoint aPrevControl(B2DPoint(nLastX, nLastY));
                            const sal_uInt32 nIndex(aCurrPoly.count() - 1);

                            if(aCurrPoly.areControlPointsUsed() && aCurrPoly.isPrevControlPointUsed(nIndex))
                            {
                                const B2DPoint aPrevPoint(aCurrPoly.getB2DPoint(nIndex));
                                const B2DPoint aPrevControlPoint(aCurrPoly.getPrevControlPoint(nIndex));

                                
                                aPrevControl.setX((2.0 * aPrevPoint.getX()) - aPrevControlPoint.getX());
                                aPrevControl.setY((2.0 * aPrevPoint.getY()) - aPrevControlPoint.getY());
                            }

                            
                            aCurrPoly.appendBezierSegment(aPrevControl, B2DPoint(nX2, nY2), B2DPoint(nX, nY));

                            
                            nLastX = nX;
                            nLastY = nY;
                        }
                        break;
                    }

                    case 'c' :
                    {
                        bRelative = true;
                        
                    }
                    case 'C' :
                    {
                        nPos++;
                        ::basegfx::internal::lcl_skipSpaces(nPos, rSvgDStatement, nLen);

                        while(nPos < nLen && ::basegfx::internal::lcl_isOnNumberChar(rSvgDStatement, nPos))
                        {
                            double nX, nY;
                            double nX1, nY1;
                            double nX2, nY2;

                            if(!::basegfx::internal::lcl_importDoubleAndSpaces(nX1, nPos, rSvgDStatement, nLen)) return false;
                            if(!::basegfx::internal::lcl_importDoubleAndSpaces(nY1, nPos, rSvgDStatement, nLen)) return false;
                            if(!::basegfx::internal::lcl_importDoubleAndSpaces(nX2, nPos, rSvgDStatement, nLen)) return false;
                            if(!::basegfx::internal::lcl_importDoubleAndSpaces(nY2, nPos, rSvgDStatement, nLen)) return false;
                            if(!::basegfx::internal::lcl_importDoubleAndSpaces(nX, nPos, rSvgDStatement, nLen)) return false;
                            if(!::basegfx::internal::lcl_importDoubleAndSpaces(nY, nPos, rSvgDStatement, nLen)) return false;

                            if(bRelative)
                            {
                                nX1 += nLastX;
                                nY1 += nLastY;
                                nX2 += nLastX;
                                nY2 += nLastY;
                                nX += nLastX;
                                nY += nLastY;
                            }

                            
                            if(!aCurrPoly.count())
                            {
                                aCurrPoly.append(B2DPoint(nLastX, nLastY));
                            }

                            
                            aCurrPoly.appendBezierSegment(B2DPoint(nX1, nY1), B2DPoint(nX2, nY2), B2DPoint(nX, nY));

                            
                            nLastX = nX;
                            nLastY = nY;
                        }
                        break;
                    }

                    
                    case 'q' :
                    {
                        bRelative = true;
                        
                    }
                    case 'Q' :
                    {
                        nPos++;
                        ::basegfx::internal::lcl_skipSpaces(nPos, rSvgDStatement, nLen);

                        while(nPos < nLen && ::basegfx::internal::lcl_isOnNumberChar(rSvgDStatement, nPos))
                        {
                            double nX, nY;
                            double nX1, nY1;

                            if(!::basegfx::internal::lcl_importDoubleAndSpaces(nX1, nPos, rSvgDStatement, nLen)) return false;
                            if(!::basegfx::internal::lcl_importDoubleAndSpaces(nY1, nPos, rSvgDStatement, nLen)) return false;
                            if(!::basegfx::internal::lcl_importDoubleAndSpaces(nX, nPos, rSvgDStatement, nLen)) return false;
                            if(!::basegfx::internal::lcl_importDoubleAndSpaces(nY, nPos, rSvgDStatement, nLen)) return false;

                            if(bRelative)
                            {
                                nX1 += nLastX;
                                nY1 += nLastY;
                                nX += nLastX;
                                nY += nLastY;
                            }

                            
                            const double nX1Prime((nX1 * 2.0 + nLastX) / 3.0);
                            const double nY1Prime((nY1 * 2.0 + nLastY) / 3.0);
                            const double nX2Prime((nX1 * 2.0 + nX) / 3.0);
                            const double nY2Prime((nY1 * 2.0 + nY) / 3.0);

                            
                            if(!aCurrPoly.count())
                            {
                                aCurrPoly.append(B2DPoint(nLastX, nLastY));
                            }

                            
                            aCurrPoly.appendBezierSegment(B2DPoint(nX1Prime, nY1Prime), B2DPoint(nX2Prime, nY2Prime), B2DPoint(nX, nY));

                            
                            nLastX = nX;
                            nLastY = nY;
                        }
                        break;
                    }

                    
                    case 't' :
                    {
                        bRelative = true;
                        
                    }
                    case 'T' :
                    {
                        nPos++;
                        ::basegfx::internal::lcl_skipSpaces(nPos, rSvgDStatement, nLen);

                        while(nPos < nLen && ::basegfx::internal::lcl_isOnNumberChar(rSvgDStatement, nPos))
                        {
                            double nX, nY;

                            if(!::basegfx::internal::lcl_importDoubleAndSpaces(nX, nPos, rSvgDStatement, nLen)) return false;
                            if(!::basegfx::internal::lcl_importDoubleAndSpaces(nY, nPos, rSvgDStatement, nLen)) return false;

                            if(bRelative)
                            {
                                nX += nLastX;
                                nY += nLastY;
                            }

                            
                            if(!aCurrPoly.count())
                            {
                                aCurrPoly.append(B2DPoint(nLastX, nLastY));
                            }

                            
                            
                            B2DPoint aPrevControl(B2DPoint(nLastX, nLastY));
                            const sal_uInt32 nIndex(aCurrPoly.count() - 1);
                            const B2DPoint aPrevPoint(aCurrPoly.getB2DPoint(nIndex));

                            if(aCurrPoly.areControlPointsUsed() && aCurrPoly.isPrevControlPointUsed(nIndex))
                            {
                                const B2DPoint aPrevControlPoint(aCurrPoly.getPrevControlPoint(nIndex));

                                
                                aPrevControl.setX((2.0 * aPrevPoint.getX()) - aPrevControlPoint.getX());
                                aPrevControl.setY((2.0 * aPrevPoint.getY()) - aPrevControlPoint.getY());
                            }

                            if(!aPrevControl.equal(aPrevPoint))
                            {
                                
                                
                                
                                const B2DPoint aQuadControlPoint(
                                    ((3.0 * aPrevControl.getX()) - aPrevPoint.getX()) / 2.0,
                                    ((3.0 * aPrevControl.getY()) - aPrevPoint.getY()) / 2.0);

                                
                                const double nX2Prime((aQuadControlPoint.getX() * 2.0 + nX) / 3.0);
                                const double nY2Prime((aQuadControlPoint.getY() * 2.0 + nY) / 3.0);

                                
                                aCurrPoly.appendBezierSegment(aPrevControl, B2DPoint(nX2Prime, nY2Prime), B2DPoint(nX, nY));
                            }
                            else
                            {
                                
                                
                                aCurrPoly.append(B2DPoint(nX, nY));
                            }

                            
                            nLastX = nX;
                            nLastY = nY;
                        }
                        break;
                    }

                    case 'a' :
                    {
                        bRelative = true;
                        
                    }
                    case 'A' :
                    {
                        nPos++;
                        ::basegfx::internal::lcl_skipSpaces(nPos, rSvgDStatement, nLen);

                        while(nPos < nLen && ::basegfx::internal::lcl_isOnNumberChar(rSvgDStatement, nPos))
                        {
                            double nX, nY;
                            double fRX, fRY, fPhi;
                            sal_Int32 bLargeArcFlag, bSweepFlag;

                            if(!::basegfx::internal::lcl_importDoubleAndSpaces(fRX, nPos, rSvgDStatement, nLen)) return false;
                            if(!::basegfx::internal::lcl_importDoubleAndSpaces(fRY, nPos, rSvgDStatement, nLen)) return false;
                            if(!::basegfx::internal::lcl_importDoubleAndSpaces(fPhi, nPos, rSvgDStatement, nLen)) return false;
                            if(!::basegfx::internal::lcl_importFlagAndSpaces(bLargeArcFlag, nPos, rSvgDStatement, nLen)) return false;
                            if(!::basegfx::internal::lcl_importFlagAndSpaces(bSweepFlag, nPos, rSvgDStatement, nLen)) return false;
                            if(!::basegfx::internal::lcl_importDoubleAndSpaces(nX, nPos, rSvgDStatement, nLen)) return false;
                            if(!::basegfx::internal::lcl_importDoubleAndSpaces(nY, nPos, rSvgDStatement, nLen)) return false;

                            if(bRelative)
                            {
                                nX += nLastX;
                                nY += nLastY;
                            }

                            if( nX == nLastX && nY == nLastY )
                                continue; 

                            if( fRX == 0.0 || fRY == 0.0 )
                            {
                                
                                aCurrPoly.append(B2DPoint(nX, nY));
                            }
                            else
                            {
                                
                                fRX=fabs(fRX); fRY=fabs(fRY);

                                

                                
                                
                                const B2DPoint p1(nLastX, nLastY);
                                const B2DPoint p2(nX, nY);
                                B2DHomMatrix aTransform(basegfx::tools::createRotateB2DHomMatrix(-fPhi*M_PI/180));

                                const B2DPoint p1_prime( aTransform * B2DPoint(((p1-p2)/2.0)) );

                                
                                
                                
                                
                                
                                
                                B2DPoint aCenter_prime;
                                const double fRadicant(
                                    (fRX*fRX*fRY*fRY - fRX*fRX*p1_prime.getY()*p1_prime.getY() - fRY*fRY*p1_prime.getX()*p1_prime.getX())/
                                    (fRX*fRX*p1_prime.getY()*p1_prime.getY() + fRY*fRY*p1_prime.getX()*p1_prime.getX()));
                                if( fRadicant < 0.0 )
                                {
                                    
                                    
                                    
                                    
                                    
                                    
                                    const double fRatio(fRX/fRY);
                                    const double fRadicant2(
                                        p1_prime.getY()*p1_prime.getY() +
                                        p1_prime.getX()*p1_prime.getX()/(fRatio*fRatio));
                                    if( fRadicant2 < 0.0 )
                                    {
                                        
                                        
                                        
                                        
                                        aCurrPoly.append(B2DPoint(nX, nY));
                                        continue;
                                    }

                                    fRY=sqrt(fRadicant2);
                                    fRX=fRatio*fRY;

                                    
                                }
                                else
                                {
                                    const double fFactor(
                                        (bLargeArcFlag==bSweepFlag ? -1.0 : 1.0) *
                                        sqrt(fRadicant));

                                    
                                    aCenter_prime = B2DPoint(
                                        fFactor*fRX*p1_prime.getY()/fRY,
                                        -fFactor*fRY*p1_prime.getX()/fRX);
                                }

                                
                                
                                

                                
                                
                                
                                const B2DPoint aRadii(fRX,fRY);
                                double fTheta1(
                                    B2DVector(1.0,0.0).angle(
                                        (p1_prime-aCenter_prime)/aRadii));

                                
                                
                                
                                double fTheta2(
                                    B2DVector(1.0,0.0).angle(
                                        (-p1_prime-aCenter_prime)/aRadii));

                                
                                fTheta1 = fmod(2*M_PI+fTheta1,2*M_PI);
                                fTheta2 = fmod(2*M_PI+fTheta2,2*M_PI);

                                
                                
                                
                                
                                if( !bSweepFlag )
                                    std::swap(fTheta1,fTheta2);

                                
                                B2DPolygon aSegment(
                                    tools::createPolygonFromUnitEllipseSegment(
                                        fTheta1, fTheta2 ));

                                
                                aTransform = basegfx::tools::createScaleB2DHomMatrix(fRX, fRY);
                                aTransform.translate(aCenter_prime.getX(),
                                                     aCenter_prime.getY());
                                aTransform.rotate(fPhi*M_PI/180);
                                const B2DPoint aOffset((p1+p2)/2.0);
                                aTransform.translate(aOffset.getX(),
                                                     aOffset.getY());
                                aSegment.transform(aTransform);

                                
                                
                                
                                
                                if( !bSweepFlag )
                                    aSegment.flip();

                                
                                sal_uInt32 nPointIndex(aCurrPoly.count() + 1);
                                aCurrPoly.append(aSegment);

                                
                                
                                if(pHelpPointIndexSet && aCurrPoly.count() > 1)
                                {
                                    const sal_uInt32 nPolyIndex(o_rPolyPolygon.count());

                                    for(;nPointIndex + 1 < aCurrPoly.count(); nPointIndex++)
                                    {
                                        pHelpPointIndexSet->insert(PointIndex(nPolyIndex, nPointIndex));
                                    }
                                }
                            }

                            
                            nLastX = nX;
                            nLastY = nY;
                        }
                        break;
                    }

                    default:
                    {
                        OSL_FAIL("importFromSvgD(): skipping tags in svg:d element (unknown)!");
                        OSL_TRACE("importFromSvgD(): skipping tags in svg:d element (unknown: \"%c\")!", aCurrChar);
                        ++nPos;
                        break;
                    }
                }
            }

            
            if(aCurrPoly.count())
            {
                o_rPolyPolygon.append(aCurrPoly);
            }

            return true;
        }

        bool importFromSvgPoints( B2DPolygon&            o_rPoly,
                                  const OUString& rSvgPointsAttribute )
        {
            o_rPoly.clear();
            const sal_Int32 nLen(rSvgPointsAttribute.getLength());
            sal_Int32 nPos(0);
            double nX, nY;

            
            ::basegfx::internal::lcl_skipSpaces(nPos, rSvgPointsAttribute, nLen);

            while(nPos < nLen)
            {
                if(!::basegfx::internal::lcl_importDoubleAndSpaces(nX, nPos, rSvgPointsAttribute, nLen)) return false;
                if(!::basegfx::internal::lcl_importDoubleAndSpaces(nY, nPos, rSvgPointsAttribute, nLen)) return false;

                
                o_rPoly.append(B2DPoint(nX, nY));

                
                ::basegfx::internal::lcl_skipSpaces(nPos, rSvgPointsAttribute, nLen);
            }

            return true;
        }

        OUString exportToSvgPoints( const B2DPolygon& rPoly )
        {
            OSL_ENSURE(!rPoly.areControlPointsUsed(), "exportToSvgPoints: Only non-bezier polygons allowed (!)");
            const sal_uInt32 nPointCount(rPoly.count());
            OUStringBuffer aResult;

            for(sal_uInt32 a(0); a < nPointCount; a++)
            {
                const basegfx::B2DPoint aPoint(rPoly.getB2DPoint(a));

                if(a)
                {
                    aResult.append(' ');
                }

                ::basegfx::internal::lcl_putNumberChar(aResult, aPoint.getX());
                aResult.append(',');
                ::basegfx::internal::lcl_putNumberChar(aResult, aPoint.getY());
            }

            return aResult.makeStringAndClear();
        }

        OUString exportToSvgD(
            const B2DPolyPolygon& rPolyPolygon,
            bool bUseRelativeCoordinates,
            bool bDetectQuadraticBeziers,
            bool bHandleRelativeNextPointCompatible)
        {
            const sal_uInt32 nCount(rPolyPolygon.count());
            OUStringBuffer aResult;
            B2DPoint aCurrentSVGPosition(0.0, 0.0); 

            for(sal_uInt32 i(0); i < nCount; i++)
            {
                const B2DPolygon aPolygon(rPolyPolygon.getB2DPolygon(i));
                const sal_uInt32 nPointCount(aPolygon.count());

                if(nPointCount)
                {
                    const bool bPolyUsesControlPoints(aPolygon.areControlPointsUsed());
                    const sal_uInt32 nEdgeCount(aPolygon.isClosed() ? nPointCount : nPointCount - 1);
                    sal_Unicode aLastSVGCommand(' '); 
                    B2DPoint aLeft, aRight; 

                    
                    B2DPoint aEdgeStart(aPolygon.getB2DPoint(0));
                    bool bUseRelativeCoordinatesForFirstPoint(bUseRelativeCoordinates);

                    if(bHandleRelativeNextPointCompatible)
                    {
                        
                        
                        
                        bUseRelativeCoordinatesForFirstPoint = false;
                    }

                    
                    aResult.append(::basegfx::internal::lcl_getCommand('M', 'm', bUseRelativeCoordinatesForFirstPoint));
                    ::basegfx::internal::lcl_putNumberCharWithSpace(aResult, aEdgeStart.getX(), aCurrentSVGPosition.getX(), bUseRelativeCoordinatesForFirstPoint);
                    ::basegfx::internal::lcl_putNumberCharWithSpace(aResult, aEdgeStart.getY(), aCurrentSVGPosition.getY(), bUseRelativeCoordinatesForFirstPoint);
                    aLastSVGCommand =  ::basegfx::internal::lcl_getCommand('L', 'l', bUseRelativeCoordinatesForFirstPoint);
                    aCurrentSVGPosition = aEdgeStart;

                    for(sal_uInt32 nIndex(0); nIndex < nEdgeCount; nIndex++)
                    {
                        
                        const sal_uInt32 nNextIndex((nIndex + 1) % nPointCount);
                        const B2DPoint aEdgeEnd(aPolygon.getB2DPoint(nNextIndex));

                        
                        const bool bEdgeIsBezier(bPolyUsesControlPoints
                            && (aPolygon.isNextControlPointUsed(nIndex) || aPolygon.isPrevControlPointUsed(nNextIndex)));

                        if(bEdgeIsBezier)
                        {
                            
                            const B2DPoint aControlEdgeStart(aPolygon.getNextControlPoint(nIndex));
                            const B2DPoint aControlEdgeEnd(aPolygon.getPrevControlPoint(nNextIndex));
                            bool bIsQuadraticBezier(false);

                            
                            
                            
                            
                            //
                            
                            
                            
                            //
                            
                            
                            const bool bSymmetricAtEdgeStart(
                                0 != nIndex
                                && CONTINUITY_C2 == aPolygon.getContinuityInPoint(nIndex));

                            if(bDetectQuadraticBeziers)
                            {
                                
                                
                                
                                
                                //
                                
                                
                                aLeft = B2DPoint((3.0 * aControlEdgeStart - aEdgeStart) / 2.0);
                                aRight= B2DPoint((3.0 * aControlEdgeEnd - aEdgeEnd) / 2.0);
                                bIsQuadraticBezier = aLeft.equal(aRight);
                            }

                            if(bIsQuadraticBezier)
                            {
                                
                                if(bSymmetricAtEdgeStart)
                                {
                                    const sal_Unicode aCommand(::basegfx::internal::lcl_getCommand('T', 't', bUseRelativeCoordinates));

                                    if(aLastSVGCommand != aCommand)
                                    {
                                        aResult.append(aCommand);
                                        aLastSVGCommand = aCommand;
                                    }

                                    ::basegfx::internal::lcl_putNumberCharWithSpace(aResult, aEdgeEnd.getX(), aCurrentSVGPosition.getX(), bUseRelativeCoordinates);
                                    ::basegfx::internal::lcl_putNumberCharWithSpace(aResult, aEdgeEnd.getY(), aCurrentSVGPosition.getY(), bUseRelativeCoordinates);
                                    aLastSVGCommand = aCommand;
                                    aCurrentSVGPosition = aEdgeEnd;
                                }
                                else
                                {
                                    const sal_Unicode aCommand(::basegfx::internal::lcl_getCommand('Q', 'q', bUseRelativeCoordinates));

                                    if(aLastSVGCommand != aCommand)
                                    {
                                        aResult.append(aCommand);
                                        aLastSVGCommand = aCommand;
                                    }

                                    ::basegfx::internal::lcl_putNumberCharWithSpace(aResult, aLeft.getX(), aCurrentSVGPosition.getX(), bUseRelativeCoordinates);
                                    ::basegfx::internal::lcl_putNumberCharWithSpace(aResult, aLeft.getY(), aCurrentSVGPosition.getY(), bUseRelativeCoordinates);
                                    ::basegfx::internal::lcl_putNumberCharWithSpace(aResult, aEdgeEnd.getX(), aCurrentSVGPosition.getX(), bUseRelativeCoordinates);
                                    ::basegfx::internal::lcl_putNumberCharWithSpace(aResult, aEdgeEnd.getY(), aCurrentSVGPosition.getY(), bUseRelativeCoordinates);
                                    aLastSVGCommand = aCommand;
                                    aCurrentSVGPosition = aEdgeEnd;
                                }
                            }
                            else
                            {
                                
                                if(bSymmetricAtEdgeStart)
                                {
                                    const sal_Unicode aCommand(::basegfx::internal::lcl_getCommand('S', 's', bUseRelativeCoordinates));

                                    if(aLastSVGCommand != aCommand)
                                    {
                                        aResult.append(aCommand);
                                        aLastSVGCommand = aCommand;
                                    }

                                    ::basegfx::internal::lcl_putNumberCharWithSpace(aResult, aControlEdgeEnd.getX(), aCurrentSVGPosition.getX(), bUseRelativeCoordinates);
                                    ::basegfx::internal::lcl_putNumberCharWithSpace(aResult, aControlEdgeEnd.getY(), aCurrentSVGPosition.getY(), bUseRelativeCoordinates);
                                    ::basegfx::internal::lcl_putNumberCharWithSpace(aResult, aEdgeEnd.getX(), aCurrentSVGPosition.getX(), bUseRelativeCoordinates);
                                    ::basegfx::internal::lcl_putNumberCharWithSpace(aResult, aEdgeEnd.getY(), aCurrentSVGPosition.getY(), bUseRelativeCoordinates);
                                    aLastSVGCommand = aCommand;
                                    aCurrentSVGPosition = aEdgeEnd;
                                }
                                else
                                {
                                    const sal_Unicode aCommand(::basegfx::internal::lcl_getCommand('C', 'c', bUseRelativeCoordinates));

                                    if(aLastSVGCommand != aCommand)
                                    {
                                        aResult.append(aCommand);
                                        aLastSVGCommand = aCommand;
                                    }

                                    ::basegfx::internal::lcl_putNumberCharWithSpace(aResult, aControlEdgeStart.getX(), aCurrentSVGPosition.getX(), bUseRelativeCoordinates);
                                    ::basegfx::internal::lcl_putNumberCharWithSpace(aResult, aControlEdgeStart.getY(), aCurrentSVGPosition.getY(), bUseRelativeCoordinates);
                                    ::basegfx::internal::lcl_putNumberCharWithSpace(aResult, aControlEdgeEnd.getX(), aCurrentSVGPosition.getX(), bUseRelativeCoordinates);
                                    ::basegfx::internal::lcl_putNumberCharWithSpace(aResult, aControlEdgeEnd.getY(), aCurrentSVGPosition.getY(), bUseRelativeCoordinates);
                                    ::basegfx::internal::lcl_putNumberCharWithSpace(aResult, aEdgeEnd.getX(), aCurrentSVGPosition.getX(), bUseRelativeCoordinates);
                                    ::basegfx::internal::lcl_putNumberCharWithSpace(aResult, aEdgeEnd.getY(), aCurrentSVGPosition.getY(), bUseRelativeCoordinates);
                                    aLastSVGCommand = aCommand;
                                    aCurrentSVGPosition = aEdgeEnd;
                                }
                            }
                        }
                        else
                        {
                            
                            if(0 == nNextIndex)
                            {
                                
                                
                            }
                            else
                            {
                                const bool bXEqual(aEdgeStart.getX() == aEdgeEnd.getX());
                                const bool bYEqual(aEdgeStart.getY() == aEdgeEnd.getY());

                                if(bXEqual && bYEqual)
                                {
                                    
                                }
                                else if(bXEqual)
                                {
                                    
                                    const sal_Unicode aCommand(::basegfx::internal::lcl_getCommand('V', 'v', bUseRelativeCoordinates));

                                    if(aLastSVGCommand != aCommand)
                                    {
                                        aResult.append(aCommand);
                                        aLastSVGCommand = aCommand;
                                    }

                                    ::basegfx::internal::lcl_putNumberCharWithSpace(aResult, aEdgeEnd.getY(), aCurrentSVGPosition.getY(), bUseRelativeCoordinates);
                                    aCurrentSVGPosition = aEdgeEnd;
                                }
                                else if(bYEqual)
                                {
                                    
                                    const sal_Unicode aCommand(::basegfx::internal::lcl_getCommand('H', 'h', bUseRelativeCoordinates));

                                    if(aLastSVGCommand != aCommand)
                                    {
                                        aResult.append(aCommand);
                                        aLastSVGCommand = aCommand;
                                    }

                                    ::basegfx::internal::lcl_putNumberCharWithSpace(aResult, aEdgeEnd.getX(), aCurrentSVGPosition.getX(), bUseRelativeCoordinates);
                                    aCurrentSVGPosition = aEdgeEnd;
                                }
                                else
                                {
                                    
                                    const sal_Unicode aCommand(::basegfx::internal::lcl_getCommand('L', 'l', bUseRelativeCoordinates));

                                    if(aLastSVGCommand != aCommand)
                                    {
                                        aResult.append(aCommand);
                                        aLastSVGCommand = aCommand;
                                    }

                                    ::basegfx::internal::lcl_putNumberCharWithSpace(aResult, aEdgeEnd.getX(), aCurrentSVGPosition.getX(), bUseRelativeCoordinates);
                                    ::basegfx::internal::lcl_putNumberCharWithSpace(aResult, aEdgeEnd.getY(), aCurrentSVGPosition.getY(), bUseRelativeCoordinates);
                                    aCurrentSVGPosition = aEdgeEnd;
                                }
                            }
                        }

                        
                        aEdgeStart = aEdgeEnd;
                    }

                    
                    if(aPolygon.isClosed())
                    {
                        aResult.append(::basegfx::internal::lcl_getCommand('Z', 'z', bUseRelativeCoordinates));
                    }

                    if(!bHandleRelativeNextPointCompatible)
                    {
                        
                        
                        aCurrentSVGPosition = aPolygon.getB2DPoint(0);
                    }
                }
            }

            return aResult.makeStringAndClear();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
