/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: polygonprimitive2d.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: aw $ $Date: 2008-03-05 09:15:43 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PRIMITIVETYPES2D_HXX
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
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
            return basegfx::tools::getRange(getB2DPolygon());
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
        Primitive2DSequence PolygonMarkerPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            if(getDashLength() > 0.0)
            {
                // apply dashing; get line and gap snippets
                ::std::vector< double > aDash;
                basegfx::B2DPolyPolygon aDashedPolyPolyA;
                basegfx::B2DPolyPolygon aDashedPolyPolyB;

                aDash.push_back(getDashLength());
                aDash.push_back(getDashLength());
                basegfx::tools::applyLineDashing(getB2DPolygon(), aDash, &aDashedPolyPolyA, &aDashedPolyPolyB, 2.0 * getDashLength());

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
            return basegfx::tools::getRange(getB2DPolygon());
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
        Primitive2DSequence PolygonStrokePrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            if(getB2DPolygon().count())
            {
                basegfx::B2DPolyPolygon aHairLinePolyPolygon;

                if(0.0 == getStrokeAttribute().getFullDotDashLen())
                {
                    // no line dashing, just copy
                    aHairLinePolyPolygon.append(getB2DPolygon());
                }
                else
                {
                    // apply LineStyle
                    basegfx::tools::applyLineDashing(getB2DPolygon(), getStrokeAttribute().getDotDashArray(), &aHairLinePolyPolygon, 0, getStrokeAttribute().getFullDotDashLen());
                }

                const sal_uInt32 nCount(aHairLinePolyPolygon.count());

                if(getLineAttribute().getWidth())
                {
                    // create fat line data
                    const double fHalfLineWidth(getLineAttribute().getWidth() / 2.0);
                    const basegfx::B2DLineJoin aLineJoin(getLineAttribute().getLineJoin());
                    basegfx::B2DPolyPolygon aAreaPolyPolygon;

                    for(sal_uInt32 a(0L); a < nCount; a++)
                    {
                        // AW: New version of createAreaGeometry; now creates bezier polygons
                        aAreaPolyPolygon.append(basegfx::tools::createAreaGeometry(
                            aHairLinePolyPolygon.getB2DPolygon(a), fHalfLineWidth, aLineJoin));
                        //const basegfx::B2DPolyPolygon aNewPolyPolygon(basegfx::tools::createAreaGeometryForPolygon(
                        //  aHairLinePolyPolygon.getB2DPolygon(a), fHalfLineWidth, aLineJoin, fMiterMinimumAngle));
                        //aAreaPolyPolygon.append(aNewPolyPolygon);
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
                    const Primitive2DReference xRef(new PolyPolygonHairlinePrimitive2D(aHairLinePolyPolygon, getLineAttribute().getColor()));
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
        :   BasePrimitive2D(),
            maPolygon(rPolygon),
            maLineAttribute(rLineAttribute),
            maStrokeAttribute(rStrokeAttribute)
        {
        }

        PolygonStrokePrimitive2D::PolygonStrokePrimitive2D(
            const basegfx::B2DPolygon& rPolygon,
            const attribute::LineAttribute& rLineAttribute)
        :   BasePrimitive2D(),
            maPolygon(rPolygon),
            maLineAttribute(rLineAttribute),
            maStrokeAttribute()
        {
        }

        bool PolygonStrokePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const PolygonStrokePrimitive2D& rCompare = (PolygonStrokePrimitive2D&)rPrimitive;

                return (getB2DPolygon() == rCompare.getB2DPolygon()
                    && getLineAttribute() == rCompare.getLineAttribute()
                    && getStrokeAttribute() == rCompare.getStrokeAttribute());
            }

            return false;
        }

        basegfx::B2DRange PolygonStrokePrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // get range of it (subdivided)
            basegfx::B2DRange aRetval(basegfx::tools::getRange(getB2DPolygon()));

            // if width, grow by line width
            if(getLineAttribute().getWidth())
            {
                aRetval.grow(getLineAttribute().getWidth() / 2.0);
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
        Primitive2DSequence PolygonWavePrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
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
            if(!basegfx::fTools::equalZero(getWaveHeight()))
            {
                aRetval.grow(getWaveHeight());
            }

            // if line width, grow by it
            if(!basegfx::fTools::equalZero(getLineAttribute().getWidth()))
            {
                aRetval.grow(getLineAttribute().getWidth());
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
        Primitive2DSequence PolygonStrokeArrowPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // copy local polygon, it may be changed
            basegfx::B2DPolygon aLocalPolygon(getB2DPolygon());
            basegfx::B2DPolyPolygon aArrowA;
            basegfx::B2DPolyPolygon aArrowB;

            if(!aLocalPolygon.isClosed())
            {
                // apply arrows
                const double fPolyLength(basegfx::tools::getLength(aLocalPolygon));
                double fStart(0.0);
                double fEnd(0.0);

                if(getStart().isActive())
                {
                    // create start arrow primitive and consume
                    aArrowA = basegfx::tools::createAreaGeometryForLineStartEnd(
                        aLocalPolygon, getStart().getB2DPolyPolygon(), true, getStart().getWidth(),
                        fPolyLength, getStart().isCentered() ? 0.5 : 0.0, &fStart);

                    // create some overlapping
                    fStart *= 0.8;
                }

                if(getEnd().isActive())
                {
                    // create end arrow primitive and consume
                    aArrowB = basegfx::tools::createAreaGeometryForLineStartEnd(
                        aLocalPolygon, getEnd().getB2DPolyPolygon(), false, getEnd().getWidth(),
                        fPolyLength, getEnd().isCentered() ? 0.5 : 0.0, &fEnd);

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
            const Primitive2DReference xRefShaft(new PolygonStrokePrimitive2D(aLocalPolygon, getLineAttribute(), getStrokeAttribute()));
            aRetval[nInd++] = xRefShaft;

            if(aArrowA.count())
            {
                const Primitive2DReference xRefA(new PolyPolygonColorPrimitive2D(aArrowA, getLineAttribute().getColor()));
                aRetval[nInd++] = xRefA;
            }

            if(aArrowB.count())
            {
                const Primitive2DReference xRefB(new PolyPolygonColorPrimitive2D(aArrowB, getLineAttribute().getColor()));
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

        PolygonStrokeArrowPrimitive2D::PolygonStrokeArrowPrimitive2D(
            const basegfx::B2DPolygon& rPolygon,
            const attribute::LineAttribute& rLineAttribute,
            const attribute::LineStartEndAttribute& rStart,
            const attribute::LineStartEndAttribute& rEnd)
        :   PolygonStrokePrimitive2D(rPolygon, rLineAttribute),
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
        ImplPrimitrive2DIDBlock(PolygonStrokeArrowPrimitive2D, PRIMITIVE2D_ID_POLYGONSTROKEARROWPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
