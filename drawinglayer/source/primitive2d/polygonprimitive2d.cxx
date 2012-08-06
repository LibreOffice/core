/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>

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

        basegfx::B2DRange PolygonHairlinePrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            // this is a hairline, thus the line width is view-dependent. Get range of polygon
            // as base size
            basegfx::B2DRange aRetval(getB2DPolygon().getB2DRange());

            if(!aRetval.isEmpty())
            {
                // Calculate view-dependent hairline width
                const basegfx::B2DVector aDiscreteSize(rViewInformation.getInverseObjectToViewTransformation() * basegfx::B2DVector(1.0, 0.0));
                const double fDiscreteHalfLineWidth(aDiscreteSize.getLength() * 0.5);

                if(basegfx::fTools::more(fDiscreteHalfLineWidth, 0.0))
                {
                    aRetval.grow(fDiscreteHalfLineWidth);
                }
            }

            // return range
            return aRetval;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(PolygonHairlinePrimitive2D, PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence PolygonMarkerPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            // calculate logic DashLength
            const basegfx::B2DVector aDashVector(rViewInformation.getInverseObjectToViewTransformation() * basegfx::B2DVector(getDiscreteDashLength(), 0.0));
            const double fLogicDashLength(aDashVector.getX());

            if(fLogicDashLength > 0.0 && !getRGBColorA().equal(getRGBColorB()))
            {
                // apply dashing; get line and gap snippets
                ::std::vector< double > aDash;
                basegfx::B2DPolyPolygon aDashedPolyPolyA;
                basegfx::B2DPolyPolygon aDashedPolyPolyB;

                aDash.push_back(fLogicDashLength);
                aDash.push_back(fLogicDashLength);
                basegfx::tools::applyLineDashing(getB2DPolygon(), aDash, &aDashedPolyPolyA, &aDashedPolyPolyB, 2.0 * fLogicDashLength);

                // prepare return value
                Primitive2DSequence aRetval(2);

                aRetval[0] = Primitive2DReference(new PolyPolygonHairlinePrimitive2D(aDashedPolyPolyA, getRGBColorA()));
                aRetval[1] = Primitive2DReference(new PolyPolygonHairlinePrimitive2D(aDashedPolyPolyB, getRGBColorB()));

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
            double fDiscreteDashLength)
        :   BufferedDecompositionPrimitive2D(),
            maPolygon(rPolygon),
            maRGBColorA(rRGBColorA),
            maRGBColorB(rRGBColorB),
            mfDiscreteDashLength(fDiscreteDashLength),
            maLastInverseObjectToViewTransformation()
        {
        }

        bool PolygonMarkerPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const PolygonMarkerPrimitive2D& rCompare = (PolygonMarkerPrimitive2D&)rPrimitive;

                return (getB2DPolygon() == rCompare.getB2DPolygon()
                    && getRGBColorA() == rCompare.getRGBColorA()
                    && getRGBColorB() == rCompare.getRGBColorB()
                    && getDiscreteDashLength() == rCompare.getDiscreteDashLength());
            }

            return false;
        }

        basegfx::B2DRange PolygonMarkerPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            // this is a hairline, thus the line width is view-dependent. Get range of polygon
            // as base size
            basegfx::B2DRange aRetval(getB2DPolygon().getB2DRange());

            if(!aRetval.isEmpty())
            {
                // Calculate view-dependent hairline width
                const basegfx::B2DVector aDiscreteSize(rViewInformation.getInverseObjectToViewTransformation() * basegfx::B2DVector(1.0, 0.0));
                const double fDiscreteHalfLineWidth(aDiscreteSize.getLength() * 0.5);

                if(basegfx::fTools::more(fDiscreteHalfLineWidth, 0.0))
                {
                    aRetval.grow(fDiscreteHalfLineWidth);
                }
            }

            // return range
            return aRetval;
        }

        Primitive2DSequence PolygonMarkerPrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            bool bNeedNewDecomposition(false);

            if(getBuffered2DDecomposition().hasElements())
            {
                if(rViewInformation.getInverseObjectToViewTransformation() != maLastInverseObjectToViewTransformation)
                {
                    bNeedNewDecomposition = true;
                }
            }

            if(bNeedNewDecomposition)
            {
                // conditions of last local decomposition have changed, delete
                const_cast< PolygonMarkerPrimitive2D* >(this)->setBuffered2DDecomposition(Primitive2DSequence());
            }

            if(!getBuffered2DDecomposition().hasElements())
            {
                // remember last used InverseObjectToViewTransformation
                PolygonMarkerPrimitive2D* pThat = const_cast< PolygonMarkerPrimitive2D* >(this);
                pThat->maLastInverseObjectToViewTransformation = rViewInformation.getInverseObjectToViewTransformation();
            }

            // use parent implementation
            return BufferedDecompositionPrimitive2D::get2DDecomposition(rViewInformation);
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(PolygonMarkerPrimitive2D, PRIMITIVE2D_ID_POLYGONMARKERPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence PolygonStrokePrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            if(getB2DPolygon().count())
            {
                // #i102241# try to simplify before usage
                const basegfx::B2DPolygon aB2DPolygon(basegfx::tools::simplifyCurveSegments(getB2DPolygon()));
                basegfx::B2DPolyPolygon aHairLinePolyPolygon;

                if(getStrokeAttribute().isDefault() || 0.0 == getStrokeAttribute().getFullDotDashLen())
                {
                    // no line dashing, just copy
                    aHairLinePolyPolygon.append(aB2DPolygon);
                }
                else
                {
                    // apply LineStyle
                    basegfx::tools::applyLineDashing(
                        aB2DPolygon, getStrokeAttribute().getDotDashArray(),
                        &aHairLinePolyPolygon, 0, getStrokeAttribute().getFullDotDashLen());
                }

                const sal_uInt32 nCount(aHairLinePolyPolygon.count());

                if(!getLineAttribute().isDefault() && getLineAttribute().getWidth())
                {
                    // create fat line data
                    const double fHalfLineWidth(getLineAttribute().getWidth() / 2.0);
                    const basegfx::B2DLineJoin aLineJoin(getLineAttribute().getLineJoin());
                    basegfx::B2DPolyPolygon aAreaPolyPolygon;

                    for(sal_uInt32 a(0L); a < nCount; a++)
                    {
                        // New version of createAreaGeometry; now creates bezier polygons
                        aAreaPolyPolygon.append(basegfx::tools::createAreaGeometry(
                            aHairLinePolyPolygon.getB2DPolygon(a), fHalfLineWidth, aLineJoin));
                    }

                    // prepare return value
                    Primitive2DSequence aRetval(aAreaPolyPolygon.count());

                    // create primitive
                    for(sal_uInt32 b(0L); b < aAreaPolyPolygon.count(); b++)
                    {
                        // put into single polyPolygon primitives to make clear that this is NOT meant
                        // to be painted as a single PolyPolygon (XORed as fill rule). Alternatively, a
                        // melting process may be used here one day.
                        const basegfx::B2DPolyPolygon aNewPolyPolygon(aAreaPolyPolygon.getB2DPolygon(b));
                        static bool bTestByUsingRandomColor(false);
                        const basegfx::BColor aColor(bTestByUsingRandomColor
                            ? basegfx::BColor(rand() / 32767.0, rand() / 32767.0, rand() / 32767.0)
                            : getLineAttribute().getColor());
                        const Primitive2DReference xRef(new PolyPolygonColorPrimitive2D(aNewPolyPolygon, aColor));
                        aRetval[b] = xRef;
                    }

                    return aRetval;
                }
                else
                {
                    // prepare return value
                    const Primitive2DReference xRef(
                        new PolyPolygonHairlinePrimitive2D(
                            aHairLinePolyPolygon,
                            getLineAttribute().getColor()));

                    return Primitive2DSequence(&xRef, 1);
                }
            }
            else
            {
                return Primitive2DSequence();
            }
        }

        PolygonStrokePrimitive2D::PolygonStrokePrimitive2D(
            const basegfx::B2DPolygon& rPolygon,
            const attribute::LineAttribute& rLineAttribute,
            const attribute::StrokeAttribute& rStrokeAttribute)
        :   BufferedDecompositionPrimitive2D(),
            maPolygon(rPolygon),
            maLineAttribute(rLineAttribute),
            maStrokeAttribute(rStrokeAttribute)
        {
        }

        PolygonStrokePrimitive2D::PolygonStrokePrimitive2D(
            const basegfx::B2DPolygon& rPolygon,
            const attribute::LineAttribute& rLineAttribute)
        :   BufferedDecompositionPrimitive2D(),
            maPolygon(rPolygon),
            maLineAttribute(rLineAttribute),
            maStrokeAttribute()
        {
        }

        bool PolygonStrokePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const PolygonStrokePrimitive2D& rCompare = (PolygonStrokePrimitive2D&)rPrimitive;

                return (getB2DPolygon() == rCompare.getB2DPolygon()
                    && getLineAttribute() == rCompare.getLineAttribute()
                    && getStrokeAttribute() == rCompare.getStrokeAttribute());
            }

            return false;
        }

        basegfx::B2DRange PolygonStrokePrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            basegfx::B2DRange aRetval;

            if(getLineAttribute().getWidth())
            {
                if(basegfx::B2DLINEJOIN_MITER == getLineAttribute().getLineJoin())
                {
                    // if line is mitered, use parent call since mitered line
                    // geometry may use more space than the geometry grown by half line width
                    aRetval = BufferedDecompositionPrimitive2D::getB2DRange(rViewInformation);
                }
                else
                {
                    // for all other B2DLINEJOIN_* get the range from the base geometry
                    // and expand by half the line width
                    aRetval = getB2DPolygon().getB2DRange();
                    aRetval.grow(getLineAttribute().getWidth() * 0.5);
                }
            }
            else
            {
                // this is a hairline, thus the line width is view-dependent. Get range of polygon
                // as base size
                aRetval = getB2DPolygon().getB2DRange();

                if(!aRetval.isEmpty())
                {
                    // Calculate view-dependent hairline width
                    const basegfx::B2DVector aDiscreteSize(rViewInformation.getInverseObjectToViewTransformation() * basegfx::B2DVector(1.0, 0.0));
                    const double fDiscreteHalfLineWidth(aDiscreteSize.getLength() * 0.5);

                    if(basegfx::fTools::more(fDiscreteHalfLineWidth, 0.0))
                    {
                        aRetval.grow(fDiscreteHalfLineWidth);
                    }
                }
            }

            return aRetval;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(PolygonStrokePrimitive2D, PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence PolygonWavePrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            Primitive2DSequence aRetval;

            if(getB2DPolygon().count())
            {
                const bool bHasWidth(!basegfx::fTools::equalZero(getWaveWidth()));
                const bool bHasHeight(!basegfx::fTools::equalZero(getWaveHeight()));

                if(bHasWidth && bHasHeight)
                {
                    // create waveline curve
                    const basegfx::B2DPolygon aWaveline(basegfx::tools::createWaveline(getB2DPolygon(), getWaveWidth(), getWaveHeight()));
                    const Primitive2DReference xRef(new PolygonStrokePrimitive2D(aWaveline, getLineAttribute(), getStrokeAttribute()));
                    aRetval = Primitive2DSequence(&xRef, 1);
                }
                else
                {
                    // flat waveline, decompose to simple line primitive
                    const Primitive2DReference xRef(new PolygonStrokePrimitive2D(getB2DPolygon(), getLineAttribute(), getStrokeAttribute()));
                    aRetval = Primitive2DSequence(&xRef, 1);
                }
            }

            return aRetval;
        }

        PolygonWavePrimitive2D::PolygonWavePrimitive2D(
            const basegfx::B2DPolygon& rPolygon,
            const attribute::LineAttribute& rLineAttribute,
            const attribute::StrokeAttribute& rStrokeAttribute,
            double fWaveWidth,
            double fWaveHeight)
        :   PolygonStrokePrimitive2D(rPolygon, rLineAttribute, rStrokeAttribute),
            mfWaveWidth(fWaveWidth),
            mfWaveHeight(fWaveHeight)
        {
            if(mfWaveWidth < 0.0)
            {
                mfWaveWidth = 0.0;
            }

            if(mfWaveHeight < 0.0)
            {
                mfWaveHeight = 0.0;
            }
        }

        PolygonWavePrimitive2D::PolygonWavePrimitive2D(
            const basegfx::B2DPolygon& rPolygon,
            const attribute::LineAttribute& rLineAttribute,
            double fWaveWidth,
            double fWaveHeight)
        :   PolygonStrokePrimitive2D(rPolygon, rLineAttribute),
            mfWaveWidth(fWaveWidth),
            mfWaveHeight(fWaveHeight)
        {
            if(mfWaveWidth < 0.0)
            {
                mfWaveWidth = 0.0;
            }

            if(mfWaveHeight < 0.0)
            {
                mfWaveHeight = 0.0;
            }
        }

        bool PolygonWavePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(PolygonStrokePrimitive2D::operator==(rPrimitive))
            {
                const PolygonWavePrimitive2D& rCompare = (PolygonWavePrimitive2D&)rPrimitive;

                return (getWaveWidth() == rCompare.getWaveWidth()
                    && getWaveHeight() == rCompare.getWaveHeight());
            }

            return false;
        }

        basegfx::B2DRange PolygonWavePrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            // get range of parent
            basegfx::B2DRange aRetval(PolygonStrokePrimitive2D::getB2DRange(rViewInformation));

            // if WaveHeight, grow by it
            if(basegfx::fTools::more(getWaveHeight(), 0.0))
            {
                aRetval.grow(getWaveHeight());
            }

            // if line width, grow by it
            if(basegfx::fTools::more(getLineAttribute().getWidth(), 0.0))
            {
                aRetval.grow(getLineAttribute().getWidth() * 0.5);
            }

            return aRetval;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(PolygonWavePrimitive2D, PRIMITIVE2D_ID_POLYGONWAVEPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence PolygonStrokeArrowPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // copy local polygon, it may be changed
            basegfx::B2DPolygon aLocalPolygon(getB2DPolygon());
            aLocalPolygon.removeDoublePoints();
            basegfx::B2DPolyPolygon aArrowA;
            basegfx::B2DPolyPolygon aArrowB;

            if(!aLocalPolygon.isClosed() && aLocalPolygon.count() > 1)
            {
                // apply arrows
                const double fPolyLength(basegfx::tools::getLength(aLocalPolygon));
                double fStart(0.0);
                double fEnd(0.0);
                double fStartOverlap(0.0);
                double fEndOverlap(0.0);

                if(!getStart().isDefault() && getStart().isActive())
                {
                    // create start arrow primitive and consume
                    aArrowA = basegfx::tools::createAreaGeometryForLineStartEnd(
                        aLocalPolygon, getStart().getB2DPolyPolygon(), true, getStart().getWidth(),
                        fPolyLength, getStart().isCentered() ? 0.5 : 0.0, &fStart);

                    // create some overlapping, compromise between straight and peaked markers
                    // for marker width 0.3cm and marker line width 0.02cm
                    fStartOverlap = getStart().getWidth() / 15.0;
                }

                if(!getEnd().isDefault() && getEnd().isActive())
                {
                    // create end arrow primitive and consume
                    aArrowB = basegfx::tools::createAreaGeometryForLineStartEnd(
                        aLocalPolygon, getEnd().getB2DPolyPolygon(), false, getEnd().getWidth(),
                        fPolyLength, getEnd().isCentered() ? 0.5 : 0.0, &fEnd);

                    // create some overlapping
                    fEndOverlap = getEnd().getWidth() / 15.0;
                }

                if(0.0 != fStart || 0.0 != fEnd)
                {
                    // build new poly, consume something from old poly
                    aLocalPolygon = basegfx::tools::getSnippetAbsolute(aLocalPolygon, fStart-fStartOverlap, fPolyLength - fEnd + fEndOverlap, fPolyLength);
                }
            }

            // prepare return value
            Primitive2DSequence aRetval(1L + (aArrowA.count() ? 1L : 0L) + (aArrowB.count() ? 1L : 0L));
            sal_uInt32 nInd(0L);

            // add shaft
            const Primitive2DReference xRefShaft(new
                PolygonStrokePrimitive2D(
                    aLocalPolygon, getLineAttribute(), getStrokeAttribute()));
            aRetval[nInd++] = xRefShaft;

            if(aArrowA.count())
            {
                const Primitive2DReference xRefA(
                    new PolyPolygonColorPrimitive2D(
                        aArrowA, getLineAttribute().getColor()));
                aRetval[nInd++] = xRefA;
            }

            if(aArrowB.count())
            {
                const Primitive2DReference xRefB(
                    new PolyPolygonColorPrimitive2D(
                        aArrowB, getLineAttribute().getColor()));
                aRetval[nInd++] = xRefB;
            }

            return aRetval;
        }

        PolygonStrokeArrowPrimitive2D::PolygonStrokeArrowPrimitive2D(
            const basegfx::B2DPolygon& rPolygon,
            const attribute::LineAttribute& rLineAttribute,
            const attribute::StrokeAttribute& rStrokeAttribute,
            const attribute::LineStartEndAttribute& rStart,
            const attribute::LineStartEndAttribute& rEnd)
        :   PolygonStrokePrimitive2D(rPolygon, rLineAttribute, rStrokeAttribute),
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
                return BufferedDecompositionPrimitive2D::getB2DRange(rViewInformation);
            }
            else
            {
                // get range from parent
                return PolygonStrokePrimitive2D::getB2DRange(rViewInformation);
            }
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(PolygonStrokeArrowPrimitive2D, PRIMITIVE2D_ID_POLYGONSTROKEARROWPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
