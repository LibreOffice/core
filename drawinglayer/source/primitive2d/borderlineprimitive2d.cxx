/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: borderlineprimitive2d.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:20 $
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
        Primitive2DSequence BorderLinePrimitive2D::createLocal2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
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
        :   BufDecPrimitive2D(),
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

        bool BorderLinePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufDecPrimitive2D::operator==(rPrimitive))
            {
                const BorderLinePrimitive2D& rCompare = (BorderLinePrimitive2D&)rPrimitive;

                return (getStart() == rCompare.getStart()
                    && getEnd() == rCompare.getEnd()
                    && getLeftWidth() == rCompare.getLeftWidth()
                    && getDistance() == rCompare.getDistance()
                    && getRightWidth() == rCompare.getRightWidth()
                    && getExtendInnerStart() == rCompare.getExtendInnerStart()
                    && getExtendInnerEnd() == rCompare.getExtendInnerEnd()
                    && getExtendOuterStart() == rCompare.getExtendOuterStart()
                    && getExtendOuterEnd() == rCompare.getExtendOuterEnd()
                    && getCreateInside() == rCompare.getCreateInside()
                    && getCreateOutside() == rCompare.getCreateOutside()
                    && getRGBColor() == rCompare.getRGBColor());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(BorderLinePrimitive2D, PRIMITIVE2D_ID_BORDERLINEPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
