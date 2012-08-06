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

#include <drawinglayer/primitive3d/sdrlatheprimitive3d.hxx>
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
        Primitive3DSequence SdrLathePrimitive3D::create3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const
        {
            Primitive3DSequence aRetval;

            // get slices
            const Slice3DVector& rSliceVector = getSlices();

            if(!rSliceVector.empty())
            {
                const bool bBackScale(!basegfx::fTools::equal(getBackScale(), 1.0));
                const bool bClosedRotation(!bBackScale && getHorizontalSegments() && basegfx::fTools::equal(getRotation(), F_2PI));
                sal_uInt32 a;

                // decide what to create
                const ::com::sun::star::drawing::NormalsKind eNormalsKind(getSdr3DObjectAttribute().getNormalsKind());
                const bool bCreateNormals(::com::sun::star::drawing::NormalsKind_SPECIFIC == eNormalsKind);
                const bool bCreateTextureCoordiantesX(::com::sun::star::drawing::TextureProjectionMode_OBJECTSPECIFIC == getSdr3DObjectAttribute().getTextureProjectionX());
                const bool bCreateTextureCoordiantesY(::com::sun::star::drawing::TextureProjectionMode_OBJECTSPECIFIC == getSdr3DObjectAttribute().getTextureProjectionY());
                basegfx::B2DHomMatrix aTexTransform;

                if(!getSdrLFSAttribute().getFill().isDefault()
                    && (bCreateTextureCoordiantesX || bCreateTextureCoordiantesY))
                {
                    aTexTransform.set(0, 0, 0.0);
                    aTexTransform.set(0, 1, 1.0);
                    aTexTransform.set(1, 0, 1.0);
                    aTexTransform.set(1, 1, 0.0);

                    aTexTransform.translate(0.0, -0.5);
                    aTexTransform.scale(1.0, -1.0);
                    aTexTransform.translate(0.0, 0.5);
                }

                // create geometry
                ::std::vector< basegfx::B3DPolyPolygon > aFill;
                extractPlanesFromSlice(aFill, rSliceVector,
                    bCreateNormals, getSmoothHorizontalNormals(), getSmoothNormals(), getSmoothLids(), bClosedRotation,
                    0.85, 0.6, bCreateTextureCoordiantesX || bCreateTextureCoordiantesY, aTexTransform);

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
                        // create geometric outlines with reduced line geometry for chart
                        const basegfx::B3DPolyPolygon aHorLine(extractHorizontalLinesFromSlice(rSliceVector, bClosedRotation));
                        const sal_uInt32 nCount(aHorLine.count());
                        basegfx::B3DPolyPolygon aNewLineGeometry;

                        for(a = 1; a < nCount; a++)
                        {
                            // for each loop pair create the connection edges
                            createReducedOutlines(
                                rViewInformation,
                                getTransform(),
                                aHorLine.getB3DPolygon(a - 1),
                                aHorLine.getB3DPolygon(a),
                                aNewLineGeometry);
                        }

                        for(a = 0; a < nCount; a++)
                        {
                            // filter hor lines for empty loops (those who have their defining point on the Y-Axis)
                            basegfx::B3DPolygon aCandidate(aHorLine.getB3DPolygon(a));
                            aCandidate.removeDoublePoints();

                            if(aCandidate.count())
                            {
                                aNewLineGeometry.append(aCandidate);
                            }
                        }

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
                        const basegfx::B3DPolyPolygon aHorLine(extractHorizontalLinesFromSlice(rSliceVector, bClosedRotation));
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
                if(!getSdrLFSAttribute().getShadow().isDefault()
                    && aRetval.hasElements())
                {
                    const Primitive3DSequence aShadow(createShadowPrimitive3D(
                        aRetval, getSdrLFSAttribute().getShadow(), getSdr3DObjectAttribute().getShadow3D()));
                    appendPrimitive3DSequenceToPrimitive3DSequence(aRetval, aShadow);
                }
            }

            return aRetval;
        }

        void SdrLathePrimitive3D::impCreateSlices()
        {
            // prepare the polygon. No double points, correct orientations and a correct
            // outmost polygon are needed
            maCorrectedPolyPolygon = basegfx::tools::adaptiveSubdivideByAngle(getPolyPolygon());
            maCorrectedPolyPolygon.removeDoublePoints();
            maCorrectedPolyPolygon = basegfx::tools::correctOrientations(maCorrectedPolyPolygon);
            maCorrectedPolyPolygon = basegfx::tools::correctOutmostPolygon(maCorrectedPolyPolygon);

            // check edge count of first sub-polygon. If different, reSegment polyPolygon. This ensures
            // that for polyPolygons, the subPolys 1..n only get reSegmented when polygon 0L is different
            // at all (and not always)
            const basegfx::B2DPolygon aSubCandidate(maCorrectedPolyPolygon.getB2DPolygon(0));
            const sal_uInt32 nSubEdgeCount(aSubCandidate.isClosed() ? aSubCandidate.count() : (aSubCandidate.count() ? aSubCandidate.count() - 1L : 0L));

            if(nSubEdgeCount != getVerticalSegments())
            {
                maCorrectedPolyPolygon = basegfx::tools::reSegmentPolyPolygon(maCorrectedPolyPolygon, getVerticalSegments());
            }

            // prepare slices as geometry
            createLatheSlices(maSlices, maCorrectedPolyPolygon, getBackScale(), getDiagonal(), getRotation(), getHorizontalSegments(), getCharacterMode(), getCloseFront(), getCloseBack());
        }

        const Slice3DVector& SdrLathePrimitive3D::getSlices() const
        {
            // This can be made dependent of  getSdrLFSAttribute().getFill() and getSdrLFSAttribute().getLine()
            // again when no longer geometry is needed for non-visible 3D objects as it is now for chart
            if(getPolyPolygon().count() && !maSlices.size())
            {
                ::osl::Mutex m_mutex;
                const_cast< SdrLathePrimitive3D& >(*this).impCreateSlices();
            }

            return maSlices;
        }

        SdrLathePrimitive3D::SdrLathePrimitive3D(
            const basegfx::B3DHomMatrix& rTransform,
            const basegfx::B2DVector& rTextureSize,
            const attribute::SdrLineFillShadowAttribute3D& rSdrLFSAttribute,
            const attribute::Sdr3DObjectAttribute& rSdr3DObjectAttribute,
            const basegfx::B2DPolyPolygon& rPolyPolygon,
            sal_uInt32 nHorizontalSegments,
            sal_uInt32 nVerticalSegments,
            double fDiagonal,
            double fBackScale,
            double fRotation,
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
            mnHorizontalSegments(nHorizontalSegments),
            mnVerticalSegments(nVerticalSegments),
            mfDiagonal(fDiagonal),
            mfBackScale(fBackScale),
            mfRotation(fRotation),
            mpLastRLGViewInformation(0),
            mbSmoothNormals(bSmoothNormals),
            mbSmoothHorizontalNormals(bSmoothHorizontalNormals),
            mbSmoothLids(bSmoothLids),
            mbCharacterMode(bCharacterMode),
            mbCloseFront(bCloseFront),
            mbCloseBack(bCloseBack)
        {
            // make sure Rotation is positive
            if(basegfx::fTools::lessOrEqual(getRotation(), 0.0))
            {
                mfRotation = 0.0;
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

        SdrLathePrimitive3D::~SdrLathePrimitive3D()
        {
            if(mpLastRLGViewInformation)
            {
                delete mpLastRLGViewInformation;
            }
        }

        bool SdrLathePrimitive3D::operator==(const BasePrimitive3D& rPrimitive) const
        {
            if(SdrPrimitive3D::operator==(rPrimitive))
            {
                const SdrLathePrimitive3D& rCompare = static_cast< const SdrLathePrimitive3D& >(rPrimitive);

                return (getPolyPolygon() == rCompare.getPolyPolygon()
                    && getHorizontalSegments() == rCompare.getHorizontalSegments()
                    && getVerticalSegments() == rCompare.getVerticalSegments()
                    && getDiagonal() == rCompare.getDiagonal()
                    && getBackScale() == rCompare.getBackScale()
                    && getRotation() == rCompare.getRotation()
                    && getSmoothNormals() == rCompare.getSmoothNormals()
                    && getSmoothHorizontalNormals() == rCompare.getSmoothHorizontalNormals()
                    && getSmoothLids() == rCompare.getSmoothLids()
                    && getCharacterMode() == rCompare.getCharacterMode()
                    && getCloseFront() == rCompare.getCloseFront()
                    && getCloseBack() == rCompare.getCloseBack());
            }

            return false;
        }

        basegfx::B3DRange SdrLathePrimitive3D::getB3DRange(const geometry::ViewInformation3D& /*rViewInformation*/) const
        {
            // use defaut from sdrPrimitive3D which uses transformation expanded by line width/2
            // The parent implementation which uses the ranges of the decomposition would be more
            // corrcet, but for historical reasons it is necessary to do the old method: To get
            // the range of the non-transformed geometry and transform it then. This leads to different
            // ranges where the new method is more correct, but the need to keep the old behaviour
            // has priority here.
            return get3DRangeFromSlices(getSlices());
        }

        Primitive3DSequence SdrLathePrimitive3D::get3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const
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
                    SdrLathePrimitive3D* pThat = const_cast< SdrLathePrimitive3D* >(this);
                    pThat->setBuffered3DDecomposition(Primitive3DSequence());
                    delete pThat->mpLastRLGViewInformation;
                    pThat->mpLastRLGViewInformation = new geometry::ViewInformation3D(rViewInformation);
                }
            }

            // no test for buffering needed, call parent
            return SdrPrimitive3D::get3DDecomposition(rViewInformation);
        }

        // provide unique ID
        ImplPrimitrive3DIDBlock(SdrLathePrimitive3D, PRIMITIVE3D_ID_SDRLATHEPRIMITIVE3D)

    } // end of namespace primitive3d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
