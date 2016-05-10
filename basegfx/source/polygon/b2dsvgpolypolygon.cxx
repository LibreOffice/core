/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <osl/diagnose.h>
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

            // skip initial whitespace
            basegfx::internal::skipSpaces(nPos, rSvgDStatement, nLen);

            while(nPos < nLen)
            {
                bool bRelative(false);
                const sal_Unicode aCurrChar(rSvgDStatement[nPos]);

                if(o_rPolyPolygon.count() && !aCurrPoly.count() && !('m' == aCurrChar || 'M' == aCurrChar))
                {
                    // we have a new sub-polygon starting, but without a 'moveto' command.
                    // this requires to add the current point as start point to the polygon
                    // (see SVG1.1 8.3.3 The "closepath" command)
                    aCurrPoly.append(B2DPoint(nLastX, nLastY));
                }

                switch(aCurrChar)
                {
                    case 'z' :
                    case 'Z' :
                    {
                        // consume CurrChar and whitespace
                        nPos++;
                        basegfx::internal::skipSpaces(nPos, rSvgDStatement, nLen);

                        // create closed polygon and reset import values
                        if(aCurrPoly.count())
                        {
                            if(!bHandleRelativeNextPointCompatible)
                            {
                                // SVG defines that "the next subpath starts at the
                                // same initial point as the current subpath", so set the
                                // current point if we do not need to be compatible
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
                        // create non-closed polygon and reset import values
                        if(aCurrPoly.count())
                        {
                            o_rPolyPolygon.append(aCurrPoly);
                            aCurrPoly.clear();
                        }
                        SAL_FALLTHROUGH; // to add coordinate data as 1st point of new polygon
                    }
                    case 'l' :
                    case 'L' :
                    {
                        if('m' == aCurrChar || 'l' == aCurrChar)
                        {
                            bRelative = true;
                        }

                        // consume CurrChar and whitespace
                        nPos++;
                        basegfx::internal::skipSpaces(nPos, rSvgDStatement, nLen);

                        while(nPos < nLen && basegfx::internal::isOnNumberChar(rSvgDStatement, nPos))
                        {
                            double nX, nY;

                            if(!basegfx::internal::importDoubleAndSpaces(nX, nPos, rSvgDStatement, nLen)) return false;
                            if(!basegfx::internal::importDoubleAndSpaces(nY, nPos, rSvgDStatement, nLen)) return false;

                            if(bRelative)
                            {
                                nX += nLastX;
                                nY += nLastY;
                            }

                            // set last position
                            nLastX = nX;
                            nLastY = nY;

                            // add point
                            aCurrPoly.append(B2DPoint(nX, nY));
                        }
                        break;
                    }

                    case 'h' :
                    {
                        bRelative = true;
                        SAL_FALLTHROUGH;
                    }
                    case 'H' :
                    {
                        nPos++;
                        basegfx::internal::skipSpaces(nPos, rSvgDStatement, nLen);

                        while(nPos < nLen && basegfx::internal::isOnNumberChar(rSvgDStatement, nPos))
                        {
                            double nX, nY(nLastY);

                            if(!basegfx::internal::importDoubleAndSpaces(nX, nPos, rSvgDStatement, nLen)) return false;

                            if(bRelative)
                            {
                                nX += nLastX;
                            }

                            // set last position
                            nLastX = nX;

                            // add point
                            aCurrPoly.append(B2DPoint(nX, nY));
                        }
                        break;
                    }

                    case 'v' :
                    {
                        bRelative = true;
                        SAL_FALLTHROUGH;
                    }
                    case 'V' :
                    {
                        nPos++;
                        basegfx::internal::skipSpaces(nPos, rSvgDStatement, nLen);

                        while(nPos < nLen && basegfx::internal::isOnNumberChar(rSvgDStatement, nPos))
                        {
                            double nX(nLastX), nY;

                            if(!basegfx::internal::importDoubleAndSpaces(nY, nPos, rSvgDStatement, nLen)) return false;

                            if(bRelative)
                            {
                                nY += nLastY;
                            }

                            // set last position
                            nLastY = nY;

                            // add point
                            aCurrPoly.append(B2DPoint(nX, nY));
                        }
                        break;
                    }

                    case 's' :
                    {
                        bRelative = true;
                        SAL_FALLTHROUGH;
                    }
                    case 'S' :
                    {
                        nPos++;
                        basegfx::internal::skipSpaces(nPos, rSvgDStatement, nLen);

                        while(nPos < nLen && basegfx::internal::isOnNumberChar(rSvgDStatement, nPos))
                        {
                            double nX, nY;
                            double nX2, nY2;

                            if(!basegfx::internal::importDoubleAndSpaces(nX2, nPos, rSvgDStatement, nLen)) return false;
                            if(!basegfx::internal::importDoubleAndSpaces(nY2, nPos, rSvgDStatement, nLen)) return false;
                            if(!basegfx::internal::importDoubleAndSpaces(nX, nPos, rSvgDStatement, nLen)) return false;
                            if(!basegfx::internal::importDoubleAndSpaces(nY, nPos, rSvgDStatement, nLen)) return false;

                            if(bRelative)
                            {
                                nX2 += nLastX;
                                nY2 += nLastY;
                                nX += nLastX;
                                nY += nLastY;
                            }

                            // ensure existence of start point
                            if(!aCurrPoly.count())
                            {
                                aCurrPoly.append(B2DPoint(nLastX, nLastY));
                            }

                            // get first control point. It's the reflection of the PrevControlPoint
                            // of the last point. If not existent, use current point (see SVG)
                            B2DPoint aPrevControl(B2DPoint(nLastX, nLastY));
                            const sal_uInt32 nIndex(aCurrPoly.count() - 1);

                            if(aCurrPoly.areControlPointsUsed() && aCurrPoly.isPrevControlPointUsed(nIndex))
                            {
                                const B2DPoint aPrevPoint(aCurrPoly.getB2DPoint(nIndex));
                                const B2DPoint aPrevControlPoint(aCurrPoly.getPrevControlPoint(nIndex));

                                // use mirrored previous control point
                                aPrevControl.setX((2.0 * aPrevPoint.getX()) - aPrevControlPoint.getX());
                                aPrevControl.setY((2.0 * aPrevPoint.getY()) - aPrevControlPoint.getY());
                            }

                            // append curved edge
                            aCurrPoly.appendBezierSegment(aPrevControl, B2DPoint(nX2, nY2), B2DPoint(nX, nY));

                            // set last position
                            nLastX = nX;
                            nLastY = nY;
                        }
                        break;
                    }

                    case 'c' :
                    {
                        bRelative = true;
                        SAL_FALLTHROUGH;
                    }
                    case 'C' :
                    {
                        nPos++;
                        basegfx::internal::skipSpaces(nPos, rSvgDStatement, nLen);

                        while(nPos < nLen && basegfx::internal::isOnNumberChar(rSvgDStatement, nPos))
                        {
                            double nX, nY;
                            double nX1, nY1;
                            double nX2, nY2;

                            if(!basegfx::internal::importDoubleAndSpaces(nX1, nPos, rSvgDStatement, nLen)) return false;
                            if(!basegfx::internal::importDoubleAndSpaces(nY1, nPos, rSvgDStatement, nLen)) return false;
                            if(!basegfx::internal::importDoubleAndSpaces(nX2, nPos, rSvgDStatement, nLen)) return false;
                            if(!basegfx::internal::importDoubleAndSpaces(nY2, nPos, rSvgDStatement, nLen)) return false;
                            if(!basegfx::internal::importDoubleAndSpaces(nX, nPos, rSvgDStatement, nLen)) return false;
                            if(!basegfx::internal::importDoubleAndSpaces(nY, nPos, rSvgDStatement, nLen)) return false;

                            if(bRelative)
                            {
                                nX1 += nLastX;
                                nY1 += nLastY;
                                nX2 += nLastX;
                                nY2 += nLastY;
                                nX += nLastX;
                                nY += nLastY;
                            }

                            // ensure existence of start point
                            if(!aCurrPoly.count())
                            {
                                aCurrPoly.append(B2DPoint(nLastX, nLastY));
                            }

                            // append curved edge
                            aCurrPoly.appendBezierSegment(B2DPoint(nX1, nY1), B2DPoint(nX2, nY2), B2DPoint(nX, nY));

                            // set last position
                            nLastX = nX;
                            nLastY = nY;
                        }
                        break;
                    }

                    // #100617# quadratic beziers are imported as cubic ones
                    case 'q' :
                    {
                        bRelative = true;
                        SAL_FALLTHROUGH;
                    }
                    case 'Q' :
                    {
                        nPos++;
                        basegfx::internal::skipSpaces(nPos, rSvgDStatement, nLen);

                        while(nPos < nLen && basegfx::internal::isOnNumberChar(rSvgDStatement, nPos))
                        {
                            double nX, nY;
                            double nX1, nY1;

                            if(!basegfx::internal::importDoubleAndSpaces(nX1, nPos, rSvgDStatement, nLen)) return false;
                            if(!basegfx::internal::importDoubleAndSpaces(nY1, nPos, rSvgDStatement, nLen)) return false;
                            if(!basegfx::internal::importDoubleAndSpaces(nX, nPos, rSvgDStatement, nLen)) return false;
                            if(!basegfx::internal::importDoubleAndSpaces(nY, nPos, rSvgDStatement, nLen)) return false;

                            if(bRelative)
                            {
                                nX1 += nLastX;
                                nY1 += nLastY;
                                nX += nLastX;
                                nY += nLastY;
                            }

                            // calculate the cubic bezier coefficients from the quadratic ones
                            const double nX1Prime((nX1 * 2.0 + nLastX) / 3.0);
                            const double nY1Prime((nY1 * 2.0 + nLastY) / 3.0);
                            const double nX2Prime((nX1 * 2.0 + nX) / 3.0);
                            const double nY2Prime((nY1 * 2.0 + nY) / 3.0);

                            // ensure existence of start point
                            if(!aCurrPoly.count())
                            {
                                aCurrPoly.append(B2DPoint(nLastX, nLastY));
                            }

                            // append curved edge
                            aCurrPoly.appendBezierSegment(B2DPoint(nX1Prime, nY1Prime), B2DPoint(nX2Prime, nY2Prime), B2DPoint(nX, nY));

                            // set last position
                            nLastX = nX;
                            nLastY = nY;
                        }
                        break;
                    }

                    // #100617# relative quadratic beziers are imported as cubic
                    case 't' :
                    {
                        bRelative = true;
                        SAL_FALLTHROUGH;
                    }
                    case 'T' :
                    {
                        nPos++;
                        basegfx::internal::skipSpaces(nPos, rSvgDStatement, nLen);

                        while(nPos < nLen && basegfx::internal::isOnNumberChar(rSvgDStatement, nPos))
                        {
                            double nX, nY;

                            if(!basegfx::internal::importDoubleAndSpaces(nX, nPos, rSvgDStatement, nLen)) return false;
                            if(!basegfx::internal::importDoubleAndSpaces(nY, nPos, rSvgDStatement, nLen)) return false;

                            if(bRelative)
                            {
                                nX += nLastX;
                                nY += nLastY;
                            }

                            // ensure existence of start point
                            if(!aCurrPoly.count())
                            {
                                aCurrPoly.append(B2DPoint(nLastX, nLastY));
                            }

                            // get first control point. It's the reflection of the PrevControlPoint
                            // of the last point. If not existent, use current point (see SVG)
                            B2DPoint aPrevControl(B2DPoint(nLastX, nLastY));
                            const sal_uInt32 nIndex(aCurrPoly.count() - 1);
                            const B2DPoint aPrevPoint(aCurrPoly.getB2DPoint(nIndex));

                            if(aCurrPoly.areControlPointsUsed() && aCurrPoly.isPrevControlPointUsed(nIndex))
                            {
                                const B2DPoint aPrevControlPoint(aCurrPoly.getPrevControlPoint(nIndex));

                                // use mirrored previous control point
                                aPrevControl.setX((2.0 * aPrevPoint.getX()) - aPrevControlPoint.getX());
                                aPrevControl.setY((2.0 * aPrevPoint.getY()) - aPrevControlPoint.getY());
                            }

                            if(!aPrevControl.equal(aPrevPoint))
                            {
                                // there is a prev control point, and we have the already mirrored one
                                // in aPrevControl. We also need the quadratic control point for this
                                // new quadratic segment to calculate the 2nd cubic control point
                                const B2DPoint aQuadControlPoint(
                                    ((3.0 * aPrevControl.getX()) - aPrevPoint.getX()) / 2.0,
                                    ((3.0 * aPrevControl.getY()) - aPrevPoint.getY()) / 2.0);

                                // calculate the cubic bezier coefficients from the quadratic ones.
                                const double nX2Prime((aQuadControlPoint.getX() * 2.0 + nX) / 3.0);
                                const double nY2Prime((aQuadControlPoint.getY() * 2.0 + nY) / 3.0);

                                // append curved edge, use mirrored cubic control point directly
                                aCurrPoly.appendBezierSegment(aPrevControl, B2DPoint(nX2Prime, nY2Prime), B2DPoint(nX, nY));
                            }
                            else
                            {
                                // when no previous control, SVG says to use current point -> straight line.
                                // Just add end point
                                aCurrPoly.append(B2DPoint(nX, nY));
                            }

                            // set last position
                            nLastX = nX;
                            nLastY = nY;
                        }
                        break;
                    }

                    case 'a' :
                    {
                        bRelative = true;
                        SAL_FALLTHROUGH;
                    }
                    case 'A' :
                    {
                        nPos++;
                        basegfx::internal::skipSpaces(nPos, rSvgDStatement, nLen);

                        while(nPos < nLen && basegfx::internal::isOnNumberChar(rSvgDStatement, nPos))
                        {
                            double nX, nY;
                            double fRX, fRY, fPhi;
                            sal_Int32 bLargeArcFlag, bSweepFlag;

                            if(!basegfx::internal::importDoubleAndSpaces(fRX, nPos, rSvgDStatement, nLen)) return false;
                            if(!basegfx::internal::importDoubleAndSpaces(fRY, nPos, rSvgDStatement, nLen)) return false;
                            if(!basegfx::internal::importDoubleAndSpaces(fPhi, nPos, rSvgDStatement, nLen)) return false;
                            if(!basegfx::internal::importFlagAndSpaces(bLargeArcFlag, nPos, rSvgDStatement, nLen)) return false;
                            if(!basegfx::internal::importFlagAndSpaces(bSweepFlag, nPos, rSvgDStatement, nLen)) return false;
                            if(!basegfx::internal::importDoubleAndSpaces(nX, nPos, rSvgDStatement, nLen)) return false;
                            if(!basegfx::internal::importDoubleAndSpaces(nY, nPos, rSvgDStatement, nLen)) return false;

                            if(bRelative)
                            {
                                nX += nLastX;
                                nY += nLastY;
                            }

                            if( nX == nLastX && nY == nLastY )
                                continue; // start==end -> skip according to SVG spec

                            if( fRX == 0.0 || fRY == 0.0 )
                            {
                                // straight line segment according to SVG spec
                                aCurrPoly.append(B2DPoint(nX, nY));
                            }
                            else
                            {
                                // normalize according to SVG spec
                                fRX=fabs(fRX); fRY=fabs(fRY);

                                // from the SVG spec, appendix F.6.4

                                // |x1'|   |cos phi   sin phi|  |(x1 - x2)/2|
                                // |y1'| = |-sin phi  cos phi|  |(y1 - y2)/2|
                                const B2DPoint p1(nLastX, nLastY);
                                const B2DPoint p2(nX, nY);
                                B2DHomMatrix aTransform(basegfx::tools::createRotateB2DHomMatrix(-fPhi*M_PI/180));

                                const B2DPoint p1_prime( aTransform * B2DPoint(((p1-p2)/2.0)) );

                                //           ______________________________________       rx y1'
                                // |cx'|  + /  rx^2 ry^2 - rx^2 y1'^2 - ry^2 x1^2           ry
                                // |cy'| =-/       rx^2y1'^2 + ry^2 x1'^2               - ry x1'
                                //                                                          rx
                                // chose + if f_A != f_S
                                // chose - if f_A  = f_S
                                B2DPoint aCenter_prime;
                                const double fRadicant(
                                    (fRX*fRX*fRY*fRY - fRX*fRX*p1_prime.getY()*p1_prime.getY() - fRY*fRY*p1_prime.getX()*p1_prime.getX())/
                                    (fRX*fRX*p1_prime.getY()*p1_prime.getY() + fRY*fRY*p1_prime.getX()*p1_prime.getX()));
                                if( fRadicant < 0.0 )
                                {
                                    // no solution - according to SVG
                                    // spec, scale up ellipse
                                    // uniformly such that it passes
                                    // through end points (denominator
                                    // of radicant solved for fRY,
                                    // with s=fRX/fRY)
                                    const double fRatio(fRX/fRY);
                                    const double fRadicant2(
                                        p1_prime.getY()*p1_prime.getY() +
                                        p1_prime.getX()*p1_prime.getX()/(fRatio*fRatio));
                                    if( fRadicant2 < 0.0 )
                                    {
                                        // only trivial solution, one
                                        // of the axes 0 -> straight
                                        // line segment according to
                                        // SVG spec
                                        aCurrPoly.append(B2DPoint(nX, nY));
                                        continue;
                                    }

                                    fRY=sqrt(fRadicant2);
                                    fRX=fRatio*fRY;

                                    // keep center_prime forced to (0,0)
                                }
                                else
                                {
                                    const double fFactor(
                                        (bLargeArcFlag==bSweepFlag ? -1.0 : 1.0) *
                                        sqrt(fRadicant));

                                    // actually calculate center_prime
                                    aCenter_prime = B2DPoint(
                                        fFactor*fRX*p1_prime.getY()/fRY,
                                        -fFactor*fRY*p1_prime.getX()/fRX);
                                }

                                //              +           u - v
                                // angle(u,v) =  arccos( ------------ )     (take the sign of (ux vy - uy vx))
                                //              -        ||u|| ||v||

                                //                  1    | (x1' - cx')/rx |
                                // theta1 = angle((   ), |                | )
                                //                  0    | (y1' - cy')/ry |
                                const B2DPoint aRadii(fRX,fRY);
                                double fTheta1(
                                    B2DVector(1.0,0.0).angle(
                                        (p1_prime-aCenter_prime)/aRadii));

                                //                 |1|    |  (-x1' - cx')/rx |
                                // theta2 = angle( | | ,  |                  | )
                                //                 |0|    |  (-y1' - cy')/ry |
                                double fTheta2(
                                    B2DVector(1.0,0.0).angle(
                                        (-p1_prime-aCenter_prime)/aRadii));

                                // map both angles to [0,2pi)
                                fTheta1 = fmod(2*M_PI+fTheta1,2*M_PI);
                                fTheta2 = fmod(2*M_PI+fTheta2,2*M_PI);

                                // make sure the large arc is taken
                                // (since
                                // createPolygonFromEllipseSegment()
                                // normalizes to e.g. cw arc)
                                if( !bSweepFlag )
                                    std::swap(fTheta1,fTheta2);

                                // finally, create bezier polygon from this
                                B2DPolygon aSegment(
                                    tools::createPolygonFromUnitEllipseSegment(
                                        fTheta1, fTheta2 ));

                                // transform ellipse by rotation & move to final center
                                aTransform = basegfx::tools::createScaleB2DHomMatrix(fRX, fRY);
                                aTransform.translate(aCenter_prime.getX(),
                                                     aCenter_prime.getY());
                                aTransform.rotate(fPhi*M_PI/180);
                                const B2DPoint aOffset((p1+p2)/2.0);
                                aTransform.translate(aOffset.getX(),
                                                     aOffset.getY());
                                aSegment.transform(aTransform);

                                // createPolygonFromEllipseSegment()
                                // always creates arcs that are
                                // positively oriented - flip polygon
                                // if we swapped angles above
                                if( !bSweepFlag )
                                    aSegment.flip();

                                // remember PointIndex of evtl. added pure helper points
                                sal_uInt32 nPointIndex(aCurrPoly.count() + 1);
                                aCurrPoly.append(aSegment);

                                // if asked for, mark pure helper points by adding them to the index list of
                                // helper points
                                if(pHelpPointIndexSet && aCurrPoly.count() > 1)
                                {
                                    const sal_uInt32 nPolyIndex(o_rPolyPolygon.count());

                                    for(;nPointIndex + 1 < aCurrPoly.count(); nPointIndex++)
                                    {
                                        pHelpPointIndexSet->insert(PointIndex(nPolyIndex, nPointIndex));
                                    }
                                }
                            }

                            // set last position
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

            // if there is polygon data, create non-closed polygon
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

            // skip initial whitespace
            basegfx::internal::skipSpaces(nPos, rSvgPointsAttribute, nLen);

            while(nPos < nLen)
            {
                if(!basegfx::internal::importDoubleAndSpaces(nX, nPos, rSvgPointsAttribute, nLen)) return false;
                if(!basegfx::internal::importDoubleAndSpaces(nY, nPos, rSvgPointsAttribute, nLen)) return false;

                // add point
                o_rPoly.append(B2DPoint(nX, nY));

                // skip to next number, or finish
                basegfx::internal::skipSpaces(nPos, rSvgPointsAttribute, nLen);
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

                aResult.append(aPoint.getX());
                aResult.append(',');
                aResult.append(aPoint.getY());
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
            B2DPoint aCurrentSVGPosition(0.0, 0.0); // SVG assumes (0,0) as the initial current point

            for(sal_uInt32 i(0); i < nCount; i++)
            {
                const B2DPolygon aPolygon(rPolyPolygon.getB2DPolygon(i));
                const sal_uInt32 nPointCount(aPolygon.count());

                if(nPointCount)
                {
                    const bool bPolyUsesControlPoints(aPolygon.areControlPointsUsed());
                    const sal_uInt32 nEdgeCount(aPolygon.isClosed() ? nPointCount : nPointCount - 1);
                    sal_Unicode aLastSVGCommand(' '); // last SVG command char
                    B2DPoint aLeft, aRight; // for quadratic bezier test

                    // handle polygon start point
                    B2DPoint aEdgeStart(aPolygon.getB2DPoint(0));
                    bool bUseRelativeCoordinatesForFirstPoint(bUseRelativeCoordinates);

                    if(bHandleRelativeNextPointCompatible)
                    {
                        // To get around the error that the start point for the next polygon is the
                        // start point of the current one (and not the last as it was handled up to now)
                        // do force to write an absolute 'M' command as start for the next polygon
                        bUseRelativeCoordinatesForFirstPoint = false;
                    }

                    // Write 'moveto' and the 1st coordinates, set aLastSVGCommand to 'lineto'
                    aResult.append(basegfx::internal::getCommand('M', 'm', bUseRelativeCoordinatesForFirstPoint));
                    basegfx::internal::putNumberCharWithSpace(aResult, aEdgeStart.getX(), aCurrentSVGPosition.getX(), bUseRelativeCoordinatesForFirstPoint);
                    basegfx::internal::putNumberCharWithSpace(aResult, aEdgeStart.getY(), aCurrentSVGPosition.getY(), bUseRelativeCoordinatesForFirstPoint);
                    aLastSVGCommand =  basegfx::internal::getCommand('L', 'l', bUseRelativeCoordinatesForFirstPoint);
                    aCurrentSVGPosition = aEdgeStart;

                    for(sal_uInt32 nIndex(0); nIndex < nEdgeCount; nIndex++)
                    {
                        // prepare access to next point
                        const sal_uInt32 nNextIndex((nIndex + 1) % nPointCount);
                        const B2DPoint aEdgeEnd(aPolygon.getB2DPoint(nNextIndex));

                        // handle edge from (aEdgeStart, aEdgeEnd) using indices (nIndex, nNextIndex)
                        const bool bEdgeIsBezier(bPolyUsesControlPoints
                            && (aPolygon.isNextControlPointUsed(nIndex) || aPolygon.isPrevControlPointUsed(nNextIndex)));

                        if(bEdgeIsBezier)
                        {
                            // handle bezier edge
                            const B2DPoint aControlEdgeStart(aPolygon.getNextControlPoint(nIndex));
                            const B2DPoint aControlEdgeEnd(aPolygon.getPrevControlPoint(nNextIndex));
                            bool bIsQuadraticBezier(false);

                            // check continuity at current edge's start point. For SVG, do NOT use an
                            // existing continuity since no 'S' or 's' statement should be written. At
                            // import, that 'previous' control vector is not available. SVG documentation
                            // says for interpretation:

                            // "(If there is no previous command or if the previous command was
                            // not an C, c, S or s, assume the first control point is coincident
                            // with the current point.)"

                            // That's what is done from our import, so avoid exporting it as first statement
                            // is necessary.
                            const bool bSymmetricAtEdgeStart(
                                0 != nIndex
                                && B2VectorContinuity::C2 == aPolygon.getContinuityInPoint(nIndex));

                            if(bDetectQuadraticBeziers)
                            {
                                // check for quadratic beziers - that's
                                // the case if both control points are in
                                // the same place when they are prolonged
                                // to the common quadratic control point

                                // Left: P = (3P1 - P0) / 2
                                // Right: P = (3P2 - P3) / 2
                                aLeft = B2DPoint((3.0 * aControlEdgeStart - aEdgeStart) / 2.0);
                                aRight= B2DPoint((3.0 * aControlEdgeEnd - aEdgeEnd) / 2.0);
                                bIsQuadraticBezier = aLeft.equal(aRight);
                            }

                            if(bIsQuadraticBezier)
                            {
                                // approximately equal, export as quadratic bezier
                                if(bSymmetricAtEdgeStart)
                                {
                                    const sal_Unicode aCommand(basegfx::internal::getCommand('T', 't', bUseRelativeCoordinates));

                                    if(aLastSVGCommand != aCommand)
                                    {
                                        aResult.append(aCommand);
                                    }

                                    basegfx::internal::putNumberCharWithSpace(aResult, aEdgeEnd.getX(), aCurrentSVGPosition.getX(), bUseRelativeCoordinates);
                                    basegfx::internal::putNumberCharWithSpace(aResult, aEdgeEnd.getY(), aCurrentSVGPosition.getY(), bUseRelativeCoordinates);
                                    aLastSVGCommand = aCommand;
                                    aCurrentSVGPosition = aEdgeEnd;
                                }
                                else
                                {
                                    const sal_Unicode aCommand(basegfx::internal::getCommand('Q', 'q', bUseRelativeCoordinates));

                                    if(aLastSVGCommand != aCommand)
                                    {
                                        aResult.append(aCommand);
                                    }

                                    basegfx::internal::putNumberCharWithSpace(aResult, aLeft.getX(), aCurrentSVGPosition.getX(), bUseRelativeCoordinates);
                                    basegfx::internal::putNumberCharWithSpace(aResult, aLeft.getY(), aCurrentSVGPosition.getY(), bUseRelativeCoordinates);
                                    basegfx::internal::putNumberCharWithSpace(aResult, aEdgeEnd.getX(), aCurrentSVGPosition.getX(), bUseRelativeCoordinates);
                                    basegfx::internal::putNumberCharWithSpace(aResult, aEdgeEnd.getY(), aCurrentSVGPosition.getY(), bUseRelativeCoordinates);
                                    aLastSVGCommand = aCommand;
                                    aCurrentSVGPosition = aEdgeEnd;
                                }
                            }
                            else
                            {
                                // export as cubic bezier
                                if(bSymmetricAtEdgeStart)
                                {
                                    const sal_Unicode aCommand(basegfx::internal::getCommand('S', 's', bUseRelativeCoordinates));

                                    if(aLastSVGCommand != aCommand)
                                    {
                                        aResult.append(aCommand);
                                    }

                                    basegfx::internal::putNumberCharWithSpace(aResult, aControlEdgeEnd.getX(), aCurrentSVGPosition.getX(), bUseRelativeCoordinates);
                                    basegfx::internal::putNumberCharWithSpace(aResult, aControlEdgeEnd.getY(), aCurrentSVGPosition.getY(), bUseRelativeCoordinates);
                                    basegfx::internal::putNumberCharWithSpace(aResult, aEdgeEnd.getX(), aCurrentSVGPosition.getX(), bUseRelativeCoordinates);
                                    basegfx::internal::putNumberCharWithSpace(aResult, aEdgeEnd.getY(), aCurrentSVGPosition.getY(), bUseRelativeCoordinates);
                                    aLastSVGCommand = aCommand;
                                    aCurrentSVGPosition = aEdgeEnd;
                                }
                                else
                                {
                                    const sal_Unicode aCommand(basegfx::internal::getCommand('C', 'c', bUseRelativeCoordinates));

                                    if(aLastSVGCommand != aCommand)
                                    {
                                        aResult.append(aCommand);
                                    }

                                    basegfx::internal::putNumberCharWithSpace(aResult, aControlEdgeStart.getX(), aCurrentSVGPosition.getX(), bUseRelativeCoordinates);
                                    basegfx::internal::putNumberCharWithSpace(aResult, aControlEdgeStart.getY(), aCurrentSVGPosition.getY(), bUseRelativeCoordinates);
                                    basegfx::internal::putNumberCharWithSpace(aResult, aControlEdgeEnd.getX(), aCurrentSVGPosition.getX(), bUseRelativeCoordinates);
                                    basegfx::internal::putNumberCharWithSpace(aResult, aControlEdgeEnd.getY(), aCurrentSVGPosition.getY(), bUseRelativeCoordinates);
                                    basegfx::internal::putNumberCharWithSpace(aResult, aEdgeEnd.getX(), aCurrentSVGPosition.getX(), bUseRelativeCoordinates);
                                    basegfx::internal::putNumberCharWithSpace(aResult, aEdgeEnd.getY(), aCurrentSVGPosition.getY(), bUseRelativeCoordinates);
                                    aLastSVGCommand = aCommand;
                                    aCurrentSVGPosition = aEdgeEnd;
                                }
                            }
                        }
                        else
                        {
                            // straight edge
                            if(0 == nNextIndex)
                            {
                                // it's a closed polygon's last edge and it's not a bezier edge, so there is
                                // no need to write it
                            }
                            else
                            {
                                const bool bXEqual(aEdgeStart.getX() == aEdgeEnd.getX());
                                const bool bYEqual(aEdgeStart.getY() == aEdgeEnd.getY());

                                if(bXEqual && bYEqual)
                                {
                                    // point is a double point; do not export at all
                                }
                                else if(bXEqual)
                                {
                                    // export as vertical line
                                    const sal_Unicode aCommand(basegfx::internal::getCommand('V', 'v', bUseRelativeCoordinates));

                                    if(aLastSVGCommand != aCommand)
                                    {
                                        aResult.append(aCommand);
                                        aLastSVGCommand = aCommand;
                                    }

                                    basegfx::internal::putNumberCharWithSpace(aResult, aEdgeEnd.getY(), aCurrentSVGPosition.getY(), bUseRelativeCoordinates);
                                    aCurrentSVGPosition = aEdgeEnd;
                                }
                                else if(bYEqual)
                                {
                                    // export as horizontal line
                                    const sal_Unicode aCommand(basegfx::internal::getCommand('H', 'h', bUseRelativeCoordinates));

                                    if(aLastSVGCommand != aCommand)
                                    {
                                        aResult.append(aCommand);
                                        aLastSVGCommand = aCommand;
                                    }

                                    basegfx::internal::putNumberCharWithSpace(aResult, aEdgeEnd.getX(), aCurrentSVGPosition.getX(), bUseRelativeCoordinates);
                                    aCurrentSVGPosition = aEdgeEnd;
                                }
                                else
                                {
                                    // export as line
                                    const sal_Unicode aCommand(basegfx::internal::getCommand('L', 'l', bUseRelativeCoordinates));

                                    if(aLastSVGCommand != aCommand)
                                    {
                                        aResult.append(aCommand);
                                        aLastSVGCommand = aCommand;
                                    }

                                    basegfx::internal::putNumberCharWithSpace(aResult, aEdgeEnd.getX(), aCurrentSVGPosition.getX(), bUseRelativeCoordinates);
                                    basegfx::internal::putNumberCharWithSpace(aResult, aEdgeEnd.getY(), aCurrentSVGPosition.getY(), bUseRelativeCoordinates);
                                    aCurrentSVGPosition = aEdgeEnd;
                                }
                            }
                        }

                        // prepare edge start for next loop step
                        aEdgeStart = aEdgeEnd;
                    }

                    // close path if closed poly (Z and z are equivalent here, but looks nicer when case is matched)
                    if(aPolygon.isClosed())
                    {
                        aResult.append(basegfx::internal::getCommand('Z', 'z', bUseRelativeCoordinates));
                    }

                    if(!bHandleRelativeNextPointCompatible)
                    {
                        // SVG defines that "the next subpath starts at the same initial point as the current subpath",
                        // so set aCurrentSVGPosition to the 1st point of the current, now ended and written path
                        aCurrentSVGPosition = aPolygon.getB2DPoint(0);
                    }
                }
            }

            return aResult.makeStringAndClear();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
