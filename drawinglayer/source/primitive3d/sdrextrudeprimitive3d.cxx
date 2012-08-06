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

#include <drawinglayer/primitive3d/sdrextrudeprimitive3d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#include <drawinglayer/primitive3d/sdrdecompositiontools3d.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>
#include <drawinglayer/geometry/viewinformation3d.hxx>
#include <drawinglayer/attribute/sdrfillattribute.hxx>
#include <drawinglayer/attribute/sdrlineattribute.hxx>
#include <drawinglayer/attribute/sdrshadowattribute.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        Primitive3DSequence SdrExtrudePrimitive3D::create3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const
        {
            Primitive3DSequence aRetval;

            // get slices
            const Slice3DVector& rSliceVector = getSlices();

            if(!rSliceVector.empty())
            {
                sal_uInt32 a;

                // decide what to create
                const ::com::sun::star::drawing::NormalsKind eNormalsKind(getSdr3DObjectAttribute().getNormalsKind());
                const bool bCreateNormals(::com::sun::star::drawing::NormalsKind_SPECIFIC == eNormalsKind);
                const bool bCreateTextureCoordiantesX(::com::sun::star::drawing::TextureProjectionMode_OBJECTSPECIFIC == getSdr3DObjectAttribute().getTextureProjectionX());
                const bool bCreateTextureCoordiantesY(::com::sun::star::drawing::TextureProjectionMode_OBJECTSPECIFIC == getSdr3DObjectAttribute().getTextureProjectionY());
                basegfx::B2DHomMatrix aTexTransform;

                if(!getSdrLFSAttribute().getFill().isDefault() && (bCreateTextureCoordiantesX || bCreateTextureCoordiantesY))
                {
                    const basegfx::B2DPolygon aFirstPolygon(maCorrectedPolyPolygon.getB2DPolygon(0L));
                    const double fFrontLength(basegfx::tools::getLength(aFirstPolygon));
                    const double fFrontArea(basegfx::tools::getArea(aFirstPolygon));
                    const double fSqrtFrontArea(sqrt(fFrontArea));
                    double fRelativeTextureWidth = basegfx::fTools::equalZero(fSqrtFrontArea) ? 1.0 : fFrontLength / fSqrtFrontArea;
                    fRelativeTextureWidth = (double)((sal_uInt32)(fRelativeTextureWidth - 0.5));

                    if(fRelativeTextureWidth < 1.0)
                    {
                        fRelativeTextureWidth = 1.0;
                    }

                    aTexTransform.translate(-0.5, -0.5);
                    aTexTransform.scale(-1.0, -1.0);
                    aTexTransform.translate(0.5, 0.5);
                    aTexTransform.scale(fRelativeTextureWidth, 1.0);
                }

                // create geometry
                ::std::vector< basegfx::B3DPolyPolygon > aFill;
                extractPlanesFromSlice(aFill, rSliceVector,
                    bCreateNormals, getSmoothHorizontalNormals(), getSmoothNormals(), getSmoothLids(), false,
                    0.5, 0.6, bCreateTextureCoordiantesX || bCreateTextureCoordiantesY, aTexTransform);

                // get full range
                const basegfx::B3DRange aRange(getRangeFrom3DGeometry(aFill));

                // normal creation
                if(!getSdrLFSAttribute().getFill().isDefault())
                {
                    if(::com::sun::star::drawing::NormalsKind_SPHERE == eNormalsKind)
                    {
                        applyNormalsKindSphereTo3DGeometry(aFill, aRange);
                    }
                    else if(::com::sun::star::drawing::NormalsKind_FLAT == eNormalsKind)
                    {
                        applyNormalsKindFlatTo3DGeometry(aFill);
                    }

                    if(getSdr3DObjectAttribute().getNormalsInvert())
                    {
                        applyNormalsInvertTo3DGeometry(aFill);
                    }
                }

                // texture coordinates
                if(!getSdrLFSAttribute().getFill().isDefault())
                {
                    applyTextureTo3DGeometry(
                        getSdr3DObjectAttribute().getTextureProjectionX(),
                        getSdr3DObjectAttribute().getTextureProjectionY(),
                        aFill,
                        aRange,
                        getTextureSize());
                }

                if(!getSdrLFSAttribute().getFill().isDefault())
                {
                    // add fill
                    aRetval = create3DPolyPolygonFillPrimitives(
                        aFill,
                        getTransform(),
                        getTextureSize(),
                        getSdr3DObjectAttribute(),
                        getSdrLFSAttribute().getFill(),
                        getSdrLFSAttribute().getFillFloatTransGradient());
                }
                else
                {
                    // create simplified 3d hit test geometry
                    aRetval = createHiddenGeometryPrimitives3D(
                        aFill,
                        getTransform(),
                        getTextureSize(),
                        getSdr3DObjectAttribute());
                }

                // add line
                if(!getSdrLFSAttribute().getLine().isDefault())
                {
                    if(getSdr3DObjectAttribute().getReducedLineGeometry())
                    {
                        // create geometric outlines with reduced line geometry for chart.
                        const basegfx::B3DPolyPolygon aVerLine(extractVerticalLinesFromSlice(rSliceVector));
                        const sal_uInt32 nCount(aVerLine.count());
                        basegfx::B3DPolyPolygon aReducedLoops;
                        basegfx::B3DPolyPolygon aNewLineGeometry;

                        // sort out doubles (front and back planes when no edge rounding is done). Since
                        // this is a line geometry merged from PolyPolygons, loop over all Polygons
                        for(a = 0; a < nCount; a++)
                        {
                            const sal_uInt32 nReducedCount(aReducedLoops.count());
                            const basegfx::B3DPolygon aCandidate(aVerLine.getB3DPolygon(a));
                            bool bAdd(true);

                            if(nReducedCount)
                            {
                                for(sal_uInt32 b(0); bAdd && b < nReducedCount; b++)
                                {
                                    if(aCandidate == aReducedLoops.getB3DPolygon(b))
                                    {
                                        bAdd = false;
                                    }
                                }
                            }

                            if(bAdd)
                            {
                                aReducedLoops.append(aCandidate);
                            }
                        }

                        // from here work with reduced loops and reduced count without changing them
                        const sal_uInt32 nReducedCount(aReducedLoops.count());

                        if(nReducedCount > 1)
                        {
                            for(sal_uInt32 b(1); b < nReducedCount; b++)
                            {
                                // get loop pair
                                const basegfx::B3DPolygon aCandA(aReducedLoops.getB3DPolygon(b - 1));
                                const basegfx::B3DPolygon aCandB(aReducedLoops.getB3DPolygon(b));

                                // for each loop pair create the connection edges
                                createReducedOutlines(
                                    rViewInformation,
                                    getTransform(),
                                    aCandA,
                                    aCandB,
                                    aNewLineGeometry);
                            }
                        }

                        // add reduced loops themselves
                        aNewLineGeometry.append(aReducedLoops);

                        // to create vertical edges at non-C1/C2 steady loops, use maCorrectedPolyPolygon
                        // directly since the 3D Polygons do not suport this.
                        //
                        // Unfortunately there is no bezier polygon provided by the chart module; one reason is
                        // that the API for extrude wants a 3D polygon geometry (for historical reasons, i guess)
                        // and those have no beziers. Another reason is that he chart module uses self-created
                        // stuff to create the 2D geometry (in ShapeFactory::createPieSegment), but this geometry
                        // does not contain bezier infos, either. The only way which is possible for now is to 'detect'
                        // candidates for vertical edges of pie segments by looking for the angles in the polygon.
                        //
                        // This is all not very well designed ATM. Ideally, the ReducedLineGeometry is responsible
                        // for creating the outer geometry edges (createReducedOutlines), but for special edges
                        // like the vertical ones for pie center and both start/end, the incarnation with the
                        // knowledge about that it needs to create those and IS a pie segment -> in this case,
                        // the chart itself.
                        const sal_uInt32 nPolyCount(maCorrectedPolyPolygon.count());

                        for(sal_uInt32 c(0); c < nPolyCount; c++)
                        {
                            const basegfx::B2DPolygon aCandidate(maCorrectedPolyPolygon.getB2DPolygon(c));
                            const sal_uInt32 nPointCount(aCandidate.count());

                            if(nPointCount > 2)
                            {
                                sal_uInt32 nIndexA(nPointCount);
                                sal_uInt32 nIndexB(nPointCount);
                                sal_uInt32 nIndexC(nPointCount);

                                for(sal_uInt32 d(0); d < nPointCount; d++)
                                {
                                    const sal_uInt32 nPrevInd((d + nPointCount - 1) % nPointCount);
                                    const sal_uInt32 nNextInd((d + 1) % nPointCount);
                                    const basegfx::B2DPoint aPoint(aCandidate.getB2DPoint(d));
                                    const basegfx::B2DVector aPrev(aCandidate.getB2DPoint(nPrevInd) - aPoint);
                                    const basegfx::B2DVector aNext(aCandidate.getB2DPoint(nNextInd) - aPoint);
                                    const double fAngle(aPrev.angle(aNext));

                                    // take each angle which deviates more than 10% from going straight as
                                    // special edge. This will detect the two outer edges of pie segments,
                                    // but not always the center one (think about a near 180 degree pie)
                                    if(F_PI - fabs(fAngle) > F_PI * 0.1)
                                    {
                                        if(nPointCount == nIndexA)
                                        {
                                            nIndexA = d;
                                        }
                                        else if(nPointCount == nIndexB)
                                        {
                                            nIndexB = d;
                                        }
                                        else if(nPointCount == nIndexC)
                                        {
                                            nIndexC = d;
                                            d = nPointCount;
                                        }
                                    }
                                }

                                const bool bIndexAUsed(nIndexA != nPointCount);
                                const bool bIndexBUsed(nIndexB != nPointCount);
                                bool bIndexCUsed(nIndexC != nPointCount);

                                if(bIndexCUsed)
                                {
                                    // already three special edges found, so the center one was already detected
                                    // and does not need to be searched
                                }
                                else if(bIndexAUsed && bIndexBUsed)
                                {
                                    // outer edges detected (they are approx. 90 degrees), but center one not.
                                    // Look with the knowledge that it's in-between the two found ones
                                    if(((nIndexA + 2) % nPointCount) == nIndexB)
                                    {
                                        nIndexC = (nIndexA + 1) % nPointCount;
                                    }
                                    else if(((nIndexA + nPointCount - 2) % nPointCount) == nIndexB)
                                    {
                                        nIndexC = (nIndexA + nPointCount - 1) % nPointCount;
                                    }

                                    bIndexCUsed = (nIndexC != nPointCount);
                                }

                                if(bIndexAUsed)
                                {
                                    const basegfx::B2DPoint aPoint(aCandidate.getB2DPoint(nIndexA));
                                    const basegfx::B3DPoint aStart(aPoint.getX(), aPoint.getY(), 0.0);
                                    const basegfx::B3DPoint aEnd(aPoint.getX(), aPoint.getY(), getDepth());
                                    basegfx::B3DPolygon aToBeAdded;

                                    aToBeAdded.append(aStart);
                                    aToBeAdded.append(aEnd);
                                    aNewLineGeometry.append(aToBeAdded);
                                }

                                if(bIndexBUsed)
                                {
                                    const basegfx::B2DPoint aPoint(aCandidate.getB2DPoint(nIndexB));
                                    const basegfx::B3DPoint aStart(aPoint.getX(), aPoint.getY(), 0.0);
                                    const basegfx::B3DPoint aEnd(aPoint.getX(), aPoint.getY(), getDepth());
                                    basegfx::B3DPolygon aToBeAdded;

                                    aToBeAdded.append(aStart);
                                    aToBeAdded.append(aEnd);
                                    aNewLineGeometry.append(aToBeAdded);
                                }

                                if(bIndexCUsed)
                                {
                                    const basegfx::B2DPoint aPoint(aCandidate.getB2DPoint(nIndexC));
                                    const basegfx::B3DPoint aStart(aPoint.getX(), aPoint.getY(), 0.0);
                                    const basegfx::B3DPoint aEnd(aPoint.getX(), aPoint.getY(), getDepth());
                                    basegfx::B3DPolygon aToBeAdded;

                                    aToBeAdded.append(aStart);
                                    aToBeAdded.append(aEnd);
                                    aNewLineGeometry.append(aToBeAdded);
                                }
                            }
                        }

                        // append loops themselves
                        aNewLineGeometry.append(aReducedLoops);

                        if(aNewLineGeometry.count())
                        {
                            const Primitive3DSequence aLines(create3DPolyPolygonLinePrimitives(
                                aNewLineGeometry, getTransform(), getSdrLFSAttribute().getLine()));
                            appendPrimitive3DSequenceToPrimitive3DSequence(aRetval, aLines);
                        }
                    }
                    else
                    {
                        // extract line geometry from slices
                        const basegfx::B3DPolyPolygon aHorLine(extractHorizontalLinesFromSlice(rSliceVector, false));
                        const basegfx::B3DPolyPolygon aVerLine(extractVerticalLinesFromSlice(rSliceVector));

                        // add horizontal lines
                        const Primitive3DSequence aHorLines(create3DPolyPolygonLinePrimitives(
                            aHorLine, getTransform(), getSdrLFSAttribute().getLine()));
                        appendPrimitive3DSequenceToPrimitive3DSequence(aRetval, aHorLines);

                        // add vertical lines
                        const Primitive3DSequence aVerLines(create3DPolyPolygonLinePrimitives(
                            aVerLine, getTransform(), getSdrLFSAttribute().getLine()));
                        appendPrimitive3DSequenceToPrimitive3DSequence(aRetval, aVerLines);
                    }
                }

                // add shadow
                if(!getSdrLFSAttribute().getShadow().isDefault() && aRetval.hasElements())
                {
                    const Primitive3DSequence aShadow(createShadowPrimitive3D(
                        aRetval, getSdrLFSAttribute().getShadow(), getSdr3DObjectAttribute().getShadow3D()));
                    appendPrimitive3DSequenceToPrimitive3DSequence(aRetval, aShadow);
                }
            }

            return aRetval;
        }

        void SdrExtrudePrimitive3D::impCreateSlices()
        {
            // prepare the polygon. No double points, correct orientations and a correct
            // outmost polygon are needed
            maCorrectedPolyPolygon = getPolyPolygon();
            maCorrectedPolyPolygon.removeDoublePoints();
            maCorrectedPolyPolygon = basegfx::tools::correctOrientations(maCorrectedPolyPolygon);
            maCorrectedPolyPolygon = basegfx::tools::correctOutmostPolygon(maCorrectedPolyPolygon);

            // prepare slices as geometry
            createExtrudeSlices(maSlices, maCorrectedPolyPolygon, getBackScale(), getDiagonal(), getDepth(), getCharacterMode(), getCloseFront(), getCloseBack());
        }

        const Slice3DVector& SdrExtrudePrimitive3D::getSlices() const
        {
            // This can be made dependent of  getSdrLFSAttribute().getFill() and getSdrLFSAttribute().getLine()
            // again when no longer geometry is needed for non-visible 3D objects as it is now for chart
            if(getPolyPolygon().count() && !maSlices.size())
            {
                ::osl::Mutex m_mutex;
                const_cast< SdrExtrudePrimitive3D& >(*this).impCreateSlices();
            }

            return maSlices;
        }

        SdrExtrudePrimitive3D::SdrExtrudePrimitive3D(
            const basegfx::B3DHomMatrix& rTransform,
            const basegfx::B2DVector& rTextureSize,
            const attribute::SdrLineFillShadowAttribute3D& rSdrLFSAttribute,
            const attribute::Sdr3DObjectAttribute& rSdr3DObjectAttribute,
            const basegfx::B2DPolyPolygon& rPolyPolygon,
            double fDepth,
            double fDiagonal,
            double fBackScale,
            bool bSmoothNormals,
            bool bSmoothHorizontalNormals,
            bool bSmoothLids,
            bool bCharacterMode,
            bool bCloseFront,
            bool bCloseBack)
        :   SdrPrimitive3D(rTransform, rTextureSize, rSdrLFSAttribute, rSdr3DObjectAttribute),
            maCorrectedPolyPolygon(),
            maSlices(),
            maPolyPolygon(rPolyPolygon),
            mfDepth(fDepth),
            mfDiagonal(fDiagonal),
            mfBackScale(fBackScale),
            mpLastRLGViewInformation(0),
            mbSmoothNormals(bSmoothNormals),
            mbSmoothHorizontalNormals(bSmoothHorizontalNormals),
            mbSmoothLids(bSmoothLids),
            mbCharacterMode(bCharacterMode),
            mbCloseFront(bCloseFront),
            mbCloseBack(bCloseBack)
        {
            // make sure depth is positive
            if(basegfx::fTools::lessOrEqual(getDepth(), 0.0))
            {
                mfDepth = 0.0;
            }

            // make sure the percentage value getDiagonal() is between 0.0 and 1.0
            if(basegfx::fTools::lessOrEqual(getDiagonal(), 0.0))
            {
                mfDiagonal = 0.0;
            }
            else if(basegfx::fTools::moreOrEqual(getDiagonal(), 1.0))
            {
                mfDiagonal = 1.0;
            }

            // no close front/back when polygon is not closed
            if(getPolyPolygon().count() && !getPolyPolygon().getB2DPolygon(0L).isClosed())
            {
                mbCloseFront = mbCloseBack = false;
            }

            // no edge rounding when not closing
            if(!getCloseFront() && !getCloseBack())
            {
                mfDiagonal = 0.0;
            }
        }

        SdrExtrudePrimitive3D::~SdrExtrudePrimitive3D()
        {
            if(mpLastRLGViewInformation)
            {
                delete mpLastRLGViewInformation;
            }
        }

        bool SdrExtrudePrimitive3D::operator==(const BasePrimitive3D& rPrimitive) const
        {
            if(SdrPrimitive3D::operator==(rPrimitive))
            {
                const SdrExtrudePrimitive3D& rCompare = static_cast< const SdrExtrudePrimitive3D& >(rPrimitive);

                return (getPolyPolygon() == rCompare.getPolyPolygon()
                    && getDepth() == rCompare.getDepth()
                    && getDiagonal() == rCompare.getDiagonal()
                    && getBackScale() == rCompare.getBackScale()
                    && getSmoothNormals() == rCompare.getSmoothNormals()
                    && getSmoothHorizontalNormals() == rCompare.getSmoothHorizontalNormals()
                    && getSmoothLids() == rCompare.getSmoothLids()
                    && getCharacterMode() == rCompare.getCharacterMode()
                    && getCloseFront() == rCompare.getCloseFront()
                    && getCloseBack() == rCompare.getCloseBack());
            }

            return false;
        }

        basegfx::B3DRange SdrExtrudePrimitive3D::getB3DRange(const geometry::ViewInformation3D& /*rViewInformation*/) const
        {
            // use defaut from sdrPrimitive3D which uses transformation expanded by line width/2
            // The parent implementation which uses the ranges of the decomposition would be more
            // corrcet, but for historical reasons it is necessary to do the old method: To get
            // the range of the non-transformed geometry and transform it then. This leads to different
            // ranges where the new method is more correct, but the need to keep the old behaviour
            // has priority here.
            return get3DRangeFromSlices(getSlices());
        }

        Primitive3DSequence SdrExtrudePrimitive3D::get3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const
        {
            if(getSdr3DObjectAttribute().getReducedLineGeometry())
            {
                if(!mpLastRLGViewInformation ||
                    (getBuffered3DDecomposition().hasElements()
                        && *mpLastRLGViewInformation != rViewInformation))
                {
                    // conditions of last local decomposition with reduced lines have changed. Remember
                    // new one and clear current decompositiopn
                    ::osl::Mutex m_mutex;
                    SdrExtrudePrimitive3D* pThat = const_cast< SdrExtrudePrimitive3D* >(this);
                    pThat->setBuffered3DDecomposition(Primitive3DSequence());
                    delete pThat->mpLastRLGViewInformation;
                    pThat->mpLastRLGViewInformation = new geometry::ViewInformation3D(rViewInformation);
                }
            }

            // no test for buffering needed, call parent
            return SdrPrimitive3D::get3DDecomposition(rViewInformation);
        }

        // provide unique ID
        ImplPrimitrive3DIDBlock(SdrExtrudePrimitive3D, PRIMITIVE3D_ID_SDREXTRUDEPRIMITIVE3D)

    } // end of namespace primitive3d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
