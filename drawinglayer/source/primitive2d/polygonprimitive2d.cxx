/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: polygonprimitive2d.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2006-11-07 15:49:09 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#endif

#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYPOLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        PolygonHairlinePrimitive2D::PolygonHairlinePrimitive2D(
            const basegfx::B2DPolygon& rPolygon,
            const basegfx::BColor& rBColor)
        :   BasePrimitive2D(),
            maPolygon(rPolygon),
            maBColor(rBColor)
        {
        }

        bool PolygonHairlinePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const PolygonHairlinePrimitive2D& rCompare = (PolygonHairlinePrimitive2D&)rPrimitive;

                return (getB2DPolygon() == rCompare.getB2DPolygon()
                    && getBColor() == rCompare.getBColor());
            }

            return false;
        }

        basegfx::B2DRange PolygonHairlinePrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // return range
            return basegfx::tools::getRange(basegfx::tools::adaptiveSubdivideByAngle(getB2DPolygon()));
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(PolygonHairlinePrimitive2D, '2','P','H','a')

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence PolygonMarkerPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            if(getDashLength() > 0.0)
            {
                ::std::vector< double > aDashA;
                ::std::vector< double > aDashB;

                aDashA.push_back(getDashLength());
                aDashA.push_back(getDashLength());

                aDashB.push_back(0.0);
                aDashB.push_back(getDashLength());
                aDashB.push_back(getDashLength());
                aDashB.push_back(0.0);

                const basegfx::B2DPolyPolygon aDashedPolyPolyA(basegfx::tools::applyLineDashing(getB2DPolygon(), aDashA, 2.0 * getDashLength()));
                const basegfx::B2DPolyPolygon aDashedPolyPolyB(basegfx::tools::applyLineDashing(getB2DPolygon(), aDashB, 2.0 * getDashLength()));

                // prepare return value
                Primitive2DSequence aRetval(aDashedPolyPolyA.count() + aDashedPolyPolyB.count());
                sal_uInt32 a(0L), b(0L);

                for(; a < aDashedPolyPolyA.count(); a++)
                {
                    const Primitive2DReference xRef(new PolygonHairlinePrimitive2D(aDashedPolyPolyA.getB2DPolygon(a), getRGBColorA()));
                    aRetval[a] = xRef;
                }

                for(; b < aDashedPolyPolyB.count(); b++)
                {
                    const Primitive2DReference xRef(new PolygonHairlinePrimitive2D(aDashedPolyPolyB.getB2DPolygon(b), getRGBColorB()));
                    aRetval[a + b] = xRef;
                }

                return aRetval;
            }
            else
            {
                const Primitive2DReference xRef(new PolygonHairlinePrimitive2D(getB2DPolygon(), getRGBColorA()));
                return Primitive2DSequence(&xRef, 1L);
            }
        }

        PolygonMarkerPrimitive2D::PolygonMarkerPrimitive2D(
            const basegfx::B2DPolygon& rPolygon,
            const basegfx::BColor& rRGBColorA,
            const basegfx::BColor& rRGBColorB,
            double fDashLength)
        :   BasePrimitive2D(),
            maPolygon(rPolygon),
            maRGBColorA(rRGBColorA),
            maRGBColorB(rRGBColorB),
            mfDashLength(fDashLength)
        {
        }

        bool PolygonMarkerPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const PolygonMarkerPrimitive2D& rCompare = (PolygonMarkerPrimitive2D&)rPrimitive;

                return (getB2DPolygon() == rCompare.getB2DPolygon()
                    && getRGBColorA() == rCompare.getRGBColorA()
                    && getRGBColorB() == rCompare.getRGBColorB()
                    && getDashLength() == rCompare.getDashLength());
            }

            return false;
        }

        basegfx::B2DRange PolygonMarkerPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // return range
            return basegfx::tools::getRange(basegfx::tools::adaptiveSubdivideByAngle(getB2DPolygon()));
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(PolygonMarkerPrimitive2D, '2','P','M','a')

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence PolygonStrokePrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            if(getB2DPolygon().count())
            {
                basegfx::B2DPolyPolygon aHairLinePolyPolygon;

                if(0.0 == getStrokeAttribute().getFullDotDashLen())
                {
                    aHairLinePolyPolygon.append(getB2DPolygon());
                }
                else
                {
                    // apply LineStyle
                    const basegfx::B2DPolygon aHairLinePolygon(basegfx::tools::adaptiveSubdivideByAngle(getB2DPolygon()));
                    aHairLinePolyPolygon = basegfx::tools::applyLineDashing(aHairLinePolygon, getStrokeAttribute().getDotDashArray(), getStrokeAttribute().getFullDotDashLen());

                    // merge LineStyle polygons to bigger parts
                    aHairLinePolyPolygon = basegfx::tools::mergeDashedLines(aHairLinePolyPolygon);
                }

                if(getStrokeAttribute().getWidth())
                {
                    // create fat line data
                    aHairLinePolyPolygon = basegfx::tools::adaptiveSubdivideByAngle(aHairLinePolyPolygon);
                    const double fHalfLineWidth(getStrokeAttribute().getWidth() / 2.0);
                    const double fDegreeStepWidth(10.0 * F_PI180);
                    const double fMiterMinimumAngle(15.0 * F_PI180);
                    const basegfx::tools::B2DLineJoin aLineJoin(getStrokeAttribute().getLineJoin());
                    basegfx::B2DPolyPolygon aAreaPolyPolygon;

                    for(sal_uInt32 a(0L); a < aHairLinePolyPolygon.count(); a++)
                    {
                        const basegfx::B2DPolyPolygon aNewPolyPolygon(basegfx::tools::createAreaGeometryForPolygon(
                            aHairLinePolyPolygon.getB2DPolygon(a), fHalfLineWidth, aLineJoin, fDegreeStepWidth, fMiterMinimumAngle));
                        aAreaPolyPolygon.append(aNewPolyPolygon);
                    }

                    // prepare return value
                    Primitive2DSequence aRetval(aAreaPolyPolygon.count());

                    // create primitive
                    for(sal_uInt32 b(0L); b < aAreaPolyPolygon.count(); b++)
                    {
                        // put into single polyPolygon primitives to make clear thta this is NOT meant
                        // to be painted XORed as fill rule. Alternatively, a melting process may be used
                        // here one day.
                        const basegfx::B2DPolyPolygon aNewPolyPolygon(aAreaPolyPolygon.getB2DPolygon(b));
                        const Primitive2DReference xRef(new PolyPolygonColorPrimitive2D(aNewPolyPolygon, getStrokeAttribute().getColor()));
                        aRetval[b] = xRef;
                    }

                    return aRetval;
                }
                else
                {
                    // prepare return value
                    Primitive2DSequence aRetval(aHairLinePolyPolygon.count());

                    // create hair line data for all sub polygons
                    for(sal_uInt32 a(0L); a < aHairLinePolyPolygon.count(); a++)
                    {
                        const basegfx::B2DPolygon aCandidate = aHairLinePolyPolygon.getB2DPolygon(a);
                        const Primitive2DReference xRef(new PolygonHairlinePrimitive2D(aCandidate, getStrokeAttribute().getColor()));
                        aRetval[a] = xRef;
                    }

                    return aRetval;
                }
            }
            else
            {
                return Primitive2DSequence();
            }
        }

        PolygonStrokePrimitive2D::PolygonStrokePrimitive2D(
            const basegfx::B2DPolygon& rPolygon,
            const attribute::StrokeAttribute& rStrokeAttribute)
        :   BasePrimitive2D(),
            maPolygon(rPolygon),
            maStrokeAttribute(rStrokeAttribute)
        {
        }

        bool PolygonStrokePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const PolygonStrokePrimitive2D& rCompare = (PolygonStrokePrimitive2D&)rPrimitive;

                return (getB2DPolygon() == rCompare.getB2DPolygon()
                    && getStrokeAttribute() == rCompare.getStrokeAttribute());
            }

            return false;
        }

        basegfx::B2DRange PolygonStrokePrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // get range of it (subdivided)
            basegfx::B2DRange aRetval(basegfx::tools::getRange(basegfx::tools::adaptiveSubdivideByAngle(getB2DPolygon())));

            // if width, grow by line width
            if(getStrokeAttribute().getWidth())
            {
                aRetval.grow(getStrokeAttribute().getWidth() / 2.0);
            }

            return aRetval;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(PolygonStrokePrimitive2D, '2','P','S','t')

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence PolygonStrokeArrowPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // copy local polygon, it may be changed
            basegfx::B2DPolygon aLocalPolygon(getB2DPolygon());
            basegfx::B2DPolyPolygon aArrowA;
            basegfx::B2DPolyPolygon aArrowB;

            if(!aLocalPolygon.isClosed())
            {
                // apply arrows. To do that, make sure it's not a curve (necessary at the moment, maybe optimized later)
                aLocalPolygon = basegfx::tools::adaptiveSubdivideByAngle(aLocalPolygon);
                const double fPolyLength(basegfx::tools::getLength(aLocalPolygon));
                double fStart(0.0);
                double fEnd(0.0);

                if(getStart().isActive())
                {
                    // create start arrow primitive and consume
                    aArrowA = basegfx::tools::createAreaGeometryForLineStartEnd(
                        aLocalPolygon, getStart().getB2DPolyPolygon(), true, getStart().getWidth(), getStart().isCentered() ? 0.5 : 0.0, &fStart);

                    // create some overlapping
                    fStart *= 0.8;
                }

                if(getEnd().isActive())
                {
                    // create end arrow primitive and consume
                    aArrowB = basegfx::tools::createAreaGeometryForLineStartEnd(
                        aLocalPolygon, getEnd().getB2DPolyPolygon(), false, getEnd().getWidth(), getEnd().isCentered() ? 0.5 : 0.0, &fEnd);

                    // create some overlapping
                    fEnd *= 0.8;
                }

                if(0.0 != fStart || 0.0 != fEnd)
                {
                    // build new poly, consume something from old poly
                    aLocalPolygon = basegfx::tools::getSnippetAbsolute(aLocalPolygon, fStart, fPolyLength - fEnd, fPolyLength);
                }
            }

            // prepare return value
            Primitive2DSequence aRetval(1L + (aArrowA.count() ? 1L : 0L) + (aArrowB.count() ? 1L : 0L));
            sal_uInt32 nInd(0L);

            // add shaft
            const Primitive2DReference xRefShaft(new PolygonStrokePrimitive2D(aLocalPolygon, getStrokeAttribute()));
            aRetval[nInd++] = xRefShaft;

            if(aArrowA.count())
            {
                const Primitive2DReference xRefA(new PolyPolygonColorPrimitive2D(aArrowA, getStrokeAttribute().getColor()));
                aRetval[nInd++] = xRefA;
            }

            if(aArrowB.count())
            {
                const Primitive2DReference xRefB(new PolyPolygonColorPrimitive2D(aArrowB, getStrokeAttribute().getColor()));
                aRetval[nInd++] = xRefB;
            }

            return aRetval;
        }

        PolygonStrokeArrowPrimitive2D::PolygonStrokeArrowPrimitive2D(
            const basegfx::B2DPolygon& rPolygon,
            const attribute::StrokeAttribute& rStrokeAttribute,
            const attribute::StrokeArrowAttribute& rStart,
            const attribute::StrokeArrowAttribute& rEnd)
        :   PolygonStrokePrimitive2D(rPolygon, rStrokeAttribute),
            maStart(rStart),
            maEnd(rEnd)
        {
        }

        bool PolygonStrokeArrowPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(PolygonStrokePrimitive2D::operator==(rPrimitive))
            {
                const PolygonStrokeArrowPrimitive2D& rCompare = (PolygonStrokeArrowPrimitive2D&)rPrimitive;

                return (getStart() == rCompare.getStart()
                    && getEnd() == rCompare.getEnd());
            }

            return false;
        }

        basegfx::B2DRange PolygonStrokeArrowPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            basegfx::B2DRange aRetval;

            if(getStart().isActive() || getEnd().isActive())
            {
                // use decomposition when line start/end is used
                return BasePrimitive2D::getB2DRange(rViewInformation);
            }
            else
            {
                // get range from parent
                return PolygonStrokePrimitive2D::getB2DRange(rViewInformation);
            }
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(PolygonStrokeArrowPrimitive2D, '2','P','S','A')

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
