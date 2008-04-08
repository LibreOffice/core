/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: borderlineprimitive2d.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2008-04-08 05:58:18 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BORDERLINEPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/borderlineprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PRIMITIVETYPES2D_HXX
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYPOLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#endif

#include <numeric>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence BorderLinePrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            Primitive2DSequence xRetval;

            if(!getStart().equal(getEnd()) && (getCreateInside() || getCreateOutside()))
            {
                const bool bInsideUsed(!basegfx::fTools::equalZero(getLeftWidth()));

                if(bInsideUsed)
                {
                    // get data and vectors
                    const bool bOutsideUsed(!basegfx::fTools::equalZero(getRightWidth()));
                    const double fWidth(getWidth());
                    basegfx::B2DVector aVector(getEnd() - getStart());
                    aVector.normalize();
                    const basegfx::B2DVector aPerpendicular(basegfx::getPerpendicular(aVector));

                    if(bOutsideUsed)
                    {
                        // both used, double line definition. Create left and right offset
                        xRetval.realloc(getCreateInside() && getCreateOutside() ? 2 : 1);
                        sal_uInt32 nInsert(0);

                        if(getCreateInside())
                        {
                            // create geometry for left
                            const basegfx::B2DVector aLeftOff(aPerpendicular * (0.5 * (getLeftWidth() - fWidth)));
                            basegfx::B2DPolygon aLeft;
                            aLeft.append(getStart() + aLeftOff - (getExtendInnerStart() * aVector));
                            aLeft.append(getEnd() + aLeftOff + (getExtendInnerEnd() * aVector));

                            if(basegfx::fTools::equal(1.0, getLeftWidth()))
                            {
                                xRetval[nInsert++] = Primitive2DReference(new PolygonHairlinePrimitive2D(
                                    aLeft,
                                    getRGBColor()));
                            }
                            else
                            {
                                xRetval[nInsert++] = Primitive2DReference(new PolygonStrokePrimitive2D(
                                    aLeft,
                                    attribute::LineAttribute(getRGBColor(), getLeftWidth()),
                                    attribute::StrokeAttribute()));
                            }
                        }

                        if(getCreateOutside())
                        {
                            // create geometry for right
                            const basegfx::B2DVector aRightOff(aPerpendicular * (0.5 * (fWidth - getRightWidth())));
                            basegfx::B2DPolygon aRight;
                            aRight.append(getStart() + aRightOff - (getExtendOuterStart() * aVector));
                            aRight.append(getEnd() + aRightOff + (getExtendOuterEnd() * aVector));

                            if(basegfx::fTools::equal(1.0, getRightWidth()))
                            {
                                xRetval[nInsert++] = Primitive2DReference(new PolygonHairlinePrimitive2D(
                                    aRight,
                                    getRGBColor()));
                            }
                            else
                            {
                                xRetval[nInsert++] = Primitive2DReference(new PolygonStrokePrimitive2D(
                                    aRight,
                                    attribute::LineAttribute(getRGBColor(), getRightWidth()),
                                    attribute::StrokeAttribute()));
                            }
                        }
                    }
                    else
                    {
                        // single line, create geometry
                        basegfx::B2DPolygon aPolygon;
                        const double fMaxExtStart(::std::max(getExtendInnerStart(), getExtendOuterStart()));
                        const double fMaxExtEnd(::std::max(getExtendInnerEnd(), getExtendOuterEnd()));
                        aPolygon.append(getStart() - (fMaxExtStart * aVector));
                        aPolygon.append(getEnd() + (fMaxExtEnd * aVector));
                        xRetval.realloc(1);

                        if(basegfx::fTools::equal(1.0, getLeftWidth()))
                        {
                            xRetval[0] = Primitive2DReference(new PolygonHairlinePrimitive2D(
                                aPolygon,
                                getRGBColor()));
                        }
                        else
                        {
                            xRetval[0] = Primitive2DReference(new PolygonStrokePrimitive2D(
                                aPolygon,
                                attribute::LineAttribute(getRGBColor(), getLeftWidth()),
                                attribute::StrokeAttribute()));
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
        :   BasePrimitive2D(),
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
            if(BasePrimitive2D::operator==(rPrimitive))
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
