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

#include <drawinglayer/primitive3d/sdrlatheprimitive3d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#include <drawinglayer/primitive3d/sdrdecompositiontools3d.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>
#include <drawinglayer/geometry/viewinformation3d.hxx>
#include <drawinglayer/attribute/sdrfillattribute.hxx>
#include <drawinglayer/attribute/sdrlineattribute.hxx>
#include <drawinglayer/attribute/sdrshadowattribute.hxx>


using namespace com::sun::star;


namespace drawinglayer
{
    namespace primitive3d
    {
        Primitive3DContainer SdrLathePrimitive3D::create3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const
        {
            Primitive3DContainer aRetval;

            // get slices
            const Slice3DVector& rSliceVector = getSlices();

            if(!rSliceVector.empty())
            {
                const bool bBackScale(!basegfx::fTools::equal(getBackScale(), 1.0));
                const bool bClosedRotation(!bBackScale && getHorizontalSegments() && basegfx::fTools::equal(getRotation(), F_2PI));
                sal_uInt32 a;

                // decide what to create
                const css::drawing::NormalsKind eNormalsKind(getSdr3DObjectAttribute().getNormalsKind());
                const bool bCreateNormals(css::drawing::NormalsKind_SPECIFIC == eNormalsKind);
                const bool bCreateTextureCoordinatesX(css::drawing::TextureProjectionMode_OBJECTSPECIFIC == getSdr3DObjectAttribute().getTextureProjectionX());
                const bool bCreateTextureCoordinatesY(css::drawing::TextureProjectionMode_OBJECTSPECIFIC == getSdr3DObjectAttribute().getTextureProjectionY());
                basegfx::B2DHomMatrix aTexTransform;

                if(!getSdrLFSAttribute().getFill().isDefault()
                    && (bCreateTextureCoordinatesX || bCreateTextureCoordinatesY))
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
                std::vector< basegfx::B3DPolyPolygon > aFill;
                extractPlanesFromSlice(aFill, rSliceVector,
                    bCreateNormals, true/*smoothHorizontalNormals*/, getSmoothNormals(), getSmoothLids(), bClosedRotation,
                    0.85, 0.6, bCreateTextureCoordinatesX || bCreateTextureCoordinatesY, aTexTransform);

                // get full range
                const basegfx::B3DRange aRange(getRangeFrom3DGeometry(aFill));

                // normal creation
                if(!getSdrLFSAttribute().getFill().isDefault())
                {
                    if(css::drawing::NormalsKind_SPHERE == eNormalsKind)
                    {
                        applyNormalsKindSphereTo3DGeometry(aFill, aRange);
                    }
                    else if(css::drawing::NormalsKind_FLAT == eNormalsKind)
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
                            const Primitive3DContainer aLines(create3DPolyPolygonLinePrimitives(
                                aNewLineGeometry, getTransform(), getSdrLFSAttribute().getLine()));
                            aRetval.append(aLines);
                        }
                    }
                    else
                    {
                        // extract line geometry from slices
                        const basegfx::B3DPolyPolygon aHorLine(extractHorizontalLinesFromSlice(rSliceVector, bClosedRotation));
                        const basegfx::B3DPolyPolygon aVerLine(extractVerticalLinesFromSlice(rSliceVector));

                        // add horizontal lines
                        const Primitive3DContainer aHorLines(create3DPolyPolygonLinePrimitives(
                            aHorLine, getTransform(), getSdrLFSAttribute().getLine()));
                        aRetval.append(aHorLines);

                        // add vertical lines
                        const Primitive3DContainer aVerLines(create3DPolyPolygonLinePrimitives(
                            aVerLine, getTransform(), getSdrLFSAttribute().getLine()));
                        aRetval.append(aVerLines);
                    }
                }

                // add shadow
                if(!getSdrLFSAttribute().getShadow().isDefault()
                    && !aRetval.empty())
                {
                    const Primitive3DContainer aShadow(createShadowPrimitive3D(
                        aRetval, getSdrLFSAttribute().getShadow(), getSdr3DObjectAttribute().getShadow3D()));
                    aRetval.append(aShadow);
                }
            }

            return aRetval;
        }

        void SdrLathePrimitive3D::impCreateSlices()
        {
            // prepare the polygon. No double points, correct orientations and a correct
            // outmost polygon are needed
            // Also important: subdivide here to ensure equal point count for all slices (!)
            maCorrectedPolyPolygon = basegfx::utils::adaptiveSubdivideByAngle(getPolyPolygon());
            maCorrectedPolyPolygon.removeDoublePoints();
            maCorrectedPolyPolygon = basegfx::utils::correctOrientations(maCorrectedPolyPolygon);
            maCorrectedPolyPolygon = basegfx::utils::correctOutmostPolygon(maCorrectedPolyPolygon);

            // check edge count of first sub-polygon. If different, reSegment polyPolygon. This ensures
            // that for polyPolygons, the subPolys 1..n only get reSegmented when polygon 0 is different
            // at all (and not always)
            const basegfx::B2DPolygon aSubCandidate(maCorrectedPolyPolygon.getB2DPolygon(0));
            const sal_uInt32 nSubEdgeCount(aSubCandidate.isClosed() ? aSubCandidate.count() : (aSubCandidate.count() ? aSubCandidate.count() - 1 : 0));

            if(nSubEdgeCount != getVerticalSegments())
            {
                maCorrectedPolyPolygon = basegfx::utils::reSegmentPolyPolygon(maCorrectedPolyPolygon, getVerticalSegments());
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
                ::osl::MutexGuard aGuard( m_aMutex );

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
            mpLastRLGViewInformation(nullptr),
            mbSmoothNormals(bSmoothNormals),
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
            if(getPolyPolygon().count() && !getPolyPolygon().getB2DPolygon(0).isClosed())
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
                    && getSmoothLids() == rCompare.getSmoothLids()
                    && getCharacterMode() == rCompare.getCharacterMode()
                    && getCloseFront() == rCompare.getCloseFront()
                    && getCloseBack() == rCompare.getCloseBack());
            }

            return false;
        }

        basegfx::B3DRange SdrLathePrimitive3D::getB3DRange(const geometry::ViewInformation3D& /*rViewInformation*/) const
        {
            // use default from sdrPrimitive3D which uses transformation expanded by line width/2
            // The parent implementation which uses the ranges of the decomposition would be more
            // correct, but for historical reasons it is necessary to do the old method: To get
            // the range of the non-transformed geometry and transform it then. This leads to different
            // ranges where the new method is more correct, but the need to keep the old behaviour
            // has priority here.
            return get3DRangeFromSlices(getSlices());
        }

        Primitive3DContainer SdrLathePrimitive3D::get3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const
        {
            if(getSdr3DObjectAttribute().getReducedLineGeometry())
            {
                if(!mpLastRLGViewInformation ||
                    (!getBuffered3DDecomposition().empty()
                        && *mpLastRLGViewInformation != rViewInformation))
                {
                    ::osl::MutexGuard aGuard( m_aMutex );

                    // conditions of last local decomposition with reduced lines have changed. Remember
                    // new one and clear current decompositiopn
                    SdrLathePrimitive3D* pThat = const_cast< SdrLathePrimitive3D* >(this);
                    pThat->setBuffered3DDecomposition(Primitive3DContainer());
                    pThat->mpLastRLGViewInformation.reset( new geometry::ViewInformation3D(rViewInformation) );
                }
            }

            // no test for buffering needed, call parent
            return SdrPrimitive3D::get3DDecomposition(rViewInformation);
        }

        // provide unique ID
        ImplPrimitive3DIDBlock(SdrLathePrimitive3D, PRIMITIVE3D_ID_SDRLATHEPRIMITIVE3D)

    } // end of namespace primitive3d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
