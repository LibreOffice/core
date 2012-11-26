/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/primitive2d/borderlineprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <numeric>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence BorderLinePrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            Primitive2DSequence xRetval;

            if(!getStart().equal(getEnd()) && (getCreateInside() || getCreateOutside()))
            {
                if(isInsideUsed())
                {
                    // get data and vectors
                    const double fWidth(getWidth());
                    basegfx::B2DVector aVector(getEnd() - getStart());
                    aVector.normalize();
                    const basegfx::B2DVector aPerpendicular(basegfx::getPerpendicular(aVector));

                    if(isOutsideUsed())
                    {
                        // both used, double line definition. Create left and right offset
                        xRetval.realloc(getCreateInside() && getCreateOutside() ? 2 : 1);
                        sal_uInt32 nInsert(0);

                        if(getCreateInside())
                        {
                            // create geometry for left
                            const basegfx::B2DVector aLeftOff(aPerpendicular * (0.5 * (getCorrectedLeftWidth() - fWidth)));
                            const basegfx::B2DPoint aTmpStart(getStart() + aLeftOff - (getExtendInnerStart() * aVector));
                            const basegfx::B2DPoint aTmpEnd(getEnd() + aLeftOff + (getExtendInnerEnd() * aVector));
                            basegfx::B2DPolygon aLeft;

                            if(leftIsHairline())
                            {
                                // create hairline primitive
                                aLeft.append(aTmpStart);
                                aLeft.append(aTmpEnd);

                                xRetval[nInsert++] = Primitive2DReference(new PolygonHairlinePrimitive2D(
                                    aLeft,
                                    getRGBColor()));
                            }
                            else
                            {
                                // create filled polygon primitive. Already tried to create thick lines
                                // with the correct LineWidth, but this leads to problems when no AA
                                // is available and fat line special case reductions between 0.5 < x < 2.5 line widths
                                // are executed due to the FilledPolygon-do-not-paint-their-bottom-and-right-lines.
                                const basegfx::B2DVector aLineWidthOffset((getCorrectedLeftWidth() * 0.5) * aPerpendicular);

                                aLeft.append(aTmpStart + aLineWidthOffset);
                                aLeft.append(aTmpEnd + aLineWidthOffset);
                                aLeft.append(aTmpEnd - aLineWidthOffset);
                                aLeft.append(aTmpStart - aLineWidthOffset);
                                aLeft.setClosed(true);

                                xRetval[nInsert++] = Primitive2DReference(new PolyPolygonColorPrimitive2D(
                                    basegfx::B2DPolyPolygon(aLeft), getRGBColor()));
                            }
                        }

                        if(getCreateOutside())
                        {
                            // create geometry for right
                            const basegfx::B2DVector aRightOff(aPerpendicular * (0.5 * (fWidth - getCorrectedRightWidth())));
                            const basegfx::B2DPoint aTmpStart(getStart() + aRightOff - (getExtendOuterStart() * aVector));
                            const basegfx::B2DPoint aTmpEnd(getEnd() + aRightOff + (getExtendOuterEnd() * aVector));
                            basegfx::B2DPolygon aRight;

                            if(rightIsHairline())
                            {
                                // create hairline primitive
                                aRight.append(aTmpStart);
                                aRight.append(aTmpEnd);

                                xRetval[nInsert++] = Primitive2DReference(new PolygonHairlinePrimitive2D(
                                    aRight,
                                    getRGBColor()));
                            }
                            else
                            {
                                // create filled polygon primitive
                                const basegfx::B2DVector aLineWidthOffset((getCorrectedRightWidth() * 0.5) * aPerpendicular);

                                aRight.append(aTmpStart + aLineWidthOffset);
                                aRight.append(aTmpEnd + aLineWidthOffset);
                                aRight.append(aTmpEnd - aLineWidthOffset);
                                aRight.append(aTmpStart - aLineWidthOffset);
                                aRight.setClosed(true);

                                xRetval[nInsert++] = Primitive2DReference(new PolyPolygonColorPrimitive2D(
                                    basegfx::B2DPolyPolygon(aRight), getRGBColor()));
                            }
                        }
                    }
                    else
                    {
                        // single line, create geometry
                        basegfx::B2DPolygon aPolygon;
                        const double fMaxExtStart(::std::max(getExtendInnerStart(), getExtendOuterStart()));
                        const double fMaxExtEnd(::std::max(getExtendInnerEnd(), getExtendOuterEnd()));
                        const basegfx::B2DPoint aTmpStart(getStart() - (fMaxExtStart * aVector));
                        const basegfx::B2DPoint aTmpEnd(getEnd() + (fMaxExtEnd * aVector));
                        xRetval.realloc(1);

                        if(leftIsHairline())
                        {
                            // create hairline primitive
                            aPolygon.append(aTmpStart);
                            aPolygon.append(aTmpEnd);

                            xRetval[0] = Primitive2DReference(new PolygonHairlinePrimitive2D(
                                aPolygon,
                                getRGBColor()));
                        }
                        else
                        {
                            // create filled polygon primitive
                            const basegfx::B2DVector aLineWidthOffset((getCorrectedLeftWidth() * 0.5) * aPerpendicular);

                            aPolygon.append(aTmpStart + aLineWidthOffset);
                            aPolygon.append(aTmpEnd + aLineWidthOffset);
                            aPolygon.append(aTmpEnd - aLineWidthOffset);
                            aPolygon.append(aTmpStart - aLineWidthOffset);
                            aPolygon.setClosed(true);

                            xRetval[0] = Primitive2DReference(new PolyPolygonColorPrimitive2D(
                                basegfx::B2DPolyPolygon(aPolygon), getRGBColor()));
                        }
                    }
                }
            }

            return xRetval;
        }

        BorderLinePrimitive2D::BorderLinePrimitive2D(
            const basegfx::B2DPoint& rStart,
            const basegfx::B2DPoint& rEnd,
            double fLeftWidth,
            double fDistance,
            double fRightWidth,
            double fExtendInnerStart,
            double fExtendInnerEnd,
            double fExtendOuterStart,
            double fExtendOuterEnd,
            bool bCreateInside,
            bool bCreateOutside,
            const basegfx::BColor& rRGBColor)
        :   BufferedDecompositionPrimitive2D(),
            maStart(rStart),
            maEnd(rEnd),
            mfLeftWidth(fLeftWidth),
            mfDistance(fDistance),
            mfRightWidth(fRightWidth),
            mfExtendInnerStart(fExtendInnerStart),
            mfExtendInnerEnd(fExtendInnerEnd),
            mfExtendOuterStart(fExtendOuterStart),
            mfExtendOuterEnd(fExtendOuterEnd),
            maRGBColor(rRGBColor),
            mbCreateInside(bCreateInside),
            mbCreateOutside(bCreateOutside)
        {
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(BorderLinePrimitive2D, PRIMITIVE2D_ID_BORDERLINEPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
