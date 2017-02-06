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

#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <com/sun/star/drawing/FlagSequence.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>

#include <basegfx/tools/unotools.hxx>
#include <comphelper/sequence.hxx>

using namespace ::com::sun::star;

namespace basegfx
{
namespace unotools
{

    B2DPolyPolygon polyPolygonBezierToB2DPolyPolygon(const drawing::PolyPolygonBezierCoords& rSourcePolyPolygon)
    {
        const sal_Int32 nOuterSequenceCount(rSourcePolyPolygon.Coordinates.getLength());
        B2DPolyPolygon aNewPolyPolygon;

        if(rSourcePolyPolygon.Flags.getLength() != nOuterSequenceCount)
            throw lang::IllegalArgumentException();

        // get pointers to inner sequence
        const drawing::PointSequence* pInnerSequence = rSourcePolyPolygon.Coordinates.getConstArray();
        const drawing::FlagSequence* pInnerSequenceFlags = rSourcePolyPolygon.Flags.getConstArray();

        for(sal_Int32 a(0); a < nOuterSequenceCount; a++)
        {
            const sal_Int32 nInnerSequenceCount(pInnerSequence->getLength());

            if (!nInnerSequenceCount)
                throw lang::IllegalArgumentException();

            if (pInnerSequenceFlags->getLength() != nInnerSequenceCount)
                throw lang::IllegalArgumentException();

            // prepare new polygon
            basegfx::B2DPolygon aNewPolygon;
            const awt::Point* pArray = pInnerSequence->getConstArray();
            const drawing::PolygonFlags* pArrayFlags = pInnerSequenceFlags->getConstArray();

            // get first point and flag
            basegfx::B2DPoint aNewCoordinatePair(pArray->X, pArray->Y); pArray++;
            drawing::PolygonFlags ePolyFlag(*pArrayFlags); pArrayFlags++;
            basegfx::B2DPoint aControlA;
            basegfx::B2DPoint aControlB;

            // first point is not allowed to be a control point
            if(drawing::PolygonFlags_CONTROL == ePolyFlag)
                throw lang::IllegalArgumentException();

            // add first point as start point
            aNewPolygon.append(aNewCoordinatePair);
            for(sal_Int32 b(1); b < nInnerSequenceCount;)
            {
                // prepare loop
                bool bControlA(false);
                bool bControlB(false);

                // get next point and flag
                aNewCoordinatePair = basegfx::B2DPoint(pArray->X, pArray->Y);
                ePolyFlag = *pArrayFlags;
                pArray++; pArrayFlags++; b++;

                if(b < nInnerSequenceCount && drawing::PolygonFlags_CONTROL == ePolyFlag)
                {
                    aControlA = aNewCoordinatePair;
                    bControlA = true;

                    // get next point and flag
                    aNewCoordinatePair = basegfx::B2DPoint(pArray->X, pArray->Y);
                    ePolyFlag = *pArrayFlags;
                    pArray++; pArrayFlags++; b++;
                }

                if(b < nInnerSequenceCount && drawing::PolygonFlags_CONTROL == ePolyFlag)
                {
                    aControlB = aNewCoordinatePair;
                    bControlB = true;

                    // get next point and flag
                    aNewCoordinatePair = basegfx::B2DPoint(pArray->X, pArray->Y);
                    ePolyFlag = *pArrayFlags;
                    pArray++; pArrayFlags++; b++;
                }

                // two or no control points are consumed, another one would be an error.
                // It's also an error if only one control point was read
                if(drawing::PolygonFlags_CONTROL == ePolyFlag || bControlA != bControlB)
                    throw lang::IllegalArgumentException();

                // the previous writes used the B2DPolyPoygon -> tools::PolyPolygon converter
                // which did not create minimal PolyPolygons, but created all control points
                // as null vectors (identical points). Because of the former P(CA)(CB)-norm of
                // B2DPolygon and it's unused sign of being the zero-vector and CA and CB being
                // relative to P, an empty edge was exported as P == CA == CB. Luckily, the new
                // export format can be read without errors by the old OOo-versions, so we need only
                // to correct here at read and do not need to export a wrong but compatible version
                // for the future.
                if(bControlA
                   && aControlA.equal(aControlB)
                   && aControlA.equal(aNewPolygon.getB2DPoint(aNewPolygon.count() - 1)))
                {
                    bControlA = bControlB = false;
                }

                if(bControlA)
                {
                    // add bezier edge
                    aNewPolygon.appendBezierSegment(aControlA, aControlB, aNewCoordinatePair);
                }
                else
                {
                    // add edge
                    aNewPolygon.append(aNewCoordinatePair);
                }
            }

            // next sequence
            pInnerSequence++;
            pInnerSequenceFlags++;

            // #i72807# API import uses old line start/end-equal definition for closed,
            // so we need to correct this to closed state here
            basegfx::tools::checkClosed(aNewPolygon);

            // add new subpolygon
            aNewPolyPolygon.append(aNewPolygon);
        }

        return aNewPolyPolygon;
    }

    void b2DPolyPolygonToPolyPolygonBezier( const basegfx::B2DPolyPolygon& rPolyPoly,
                                            drawing::PolyPolygonBezierCoords& rRetval )
    {
        rRetval.Coordinates.realloc(rPolyPoly.count());
        rRetval.Flags.realloc(rPolyPoly.count());

        drawing::PointSequence* pOuterSequence = rRetval.Coordinates.getArray();
        drawing::FlagSequence*  pOuterFlags = rRetval.Flags.getArray();

        for(sal_uInt32 a=0;a<rPolyPoly.count();a++)
        {
            const B2DPolygon& rPoly = rPolyPoly.getB2DPolygon(a);
            sal_uInt32 nCount(rPoly.count());
            const bool bClosed(rPoly.isClosed());

            // calculate input vertex count
            const sal_uInt32 nLoopCount(bClosed ? nCount : (nCount ? nCount - 1 : 0 ));

            std::vector<awt::Point> aPoints; aPoints.reserve(nLoopCount);
            std::vector<drawing::PolygonFlags> aFlags; aFlags.reserve(nLoopCount);

            if( nCount )
            {
                // prepare insert index and current point
                basegfx::B2DCubicBezier aBezier;
                aBezier.setStartPoint(rPoly.getB2DPoint(0));

                for(sal_uInt32 b(0); b<nLoopCount; b++)
                {
                    // add current point (always) and remember StartPointIndex for evtl. later corrections
                    const awt::Point aStartPoint(fround(aBezier.getStartPoint().getX()),
                                                 fround(aBezier.getStartPoint().getY()));
                    const sal_uInt32 nStartPointIndex(aPoints.size());
                    aPoints.push_back(aStartPoint);
                    aFlags.push_back(drawing::PolygonFlags_NORMAL);

                    // prepare next segment
                    const sal_uInt32 nNextIndex((b + 1) % nCount);
                    aBezier.setEndPoint(rPoly.getB2DPoint(nNextIndex));
                    aBezier.setControlPointA(rPoly.getNextControlPoint(b));
                    aBezier.setControlPointB(rPoly.getPrevControlPoint(nNextIndex));

                    if(aBezier.isBezier())
                    {
                        // if one is used, add always two control points due to the old schema
                        aPoints.push_back( awt::Point(fround(aBezier.getControlPointA().getX()),
                                                      fround(aBezier.getControlPointA().getY())) );
                        aFlags.push_back(drawing::PolygonFlags_CONTROL);

                        aPoints.push_back( awt::Point(fround(aBezier.getControlPointB().getX()),
                                                      fround(aBezier.getControlPointB().getY())) );
                        aFlags.push_back(drawing::PolygonFlags_CONTROL);
                    }

                    // test continuity with previous control point to set flag value
                    if(aBezier.getControlPointA() != aBezier.getStartPoint() && (bClosed || b))
                    {
                        const basegfx::B2VectorContinuity eCont(rPoly.getContinuityInPoint(b));

                        if(basegfx::B2VectorContinuity::C1 == eCont)
                        {
                            aFlags[nStartPointIndex] = drawing::PolygonFlags_SMOOTH;
                        }
                        else if(basegfx::B2VectorContinuity::C2 == eCont)
                        {
                            aFlags[nStartPointIndex] = drawing::PolygonFlags_SYMMETRIC;
                        }
                    }

                    // prepare next polygon step
                    aBezier.setStartPoint(aBezier.getEndPoint());
                }

                if(bClosed)
                {
                    // add first point again as closing point due to old definition
                    aPoints.push_back( aPoints[0] );
                    aFlags.push_back(drawing::PolygonFlags_NORMAL);
                }
                else
                {
                    // add last point as closing point
                    const basegfx::B2DPoint aClosingPoint(rPoly.getB2DPoint(nCount - 1));
                    const awt::Point aEnd(fround(aClosingPoint.getX()),
                                          fround(aClosingPoint.getY()));
                    aPoints.push_back(aEnd);
                    aFlags.push_back(drawing::PolygonFlags_NORMAL);
                }
            }

            *pOuterSequence++ = comphelper::containerToSequence(aPoints);
            *pOuterFlags++ = comphelper::containerToSequence(aFlags);
        }
    }

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
