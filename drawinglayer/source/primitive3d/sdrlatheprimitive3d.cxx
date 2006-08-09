/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrlatheprimitive3d.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: aw $ $Date: 2006-08-09 16:51:15 $
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

#ifndef _DRAWINGLAYER_PRIMITIVE3D_SDRLATHEPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/sdrlatheprimitive3d.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE3D_SDREXTRUDELATHETOOLS3D_HXX
#include <drawinglayer/primitive3d/sdrextrudelathetools3d.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE3D_SDRDECOMPOSITIONTOOLS3D_HXX
#include <drawinglayer/primitive3d/sdrdecompositiontools3d.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B3DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        void sdrLathePrimitive3D::decompose(primitiveVector3D& rTarget)
        {
            // get slices
            const sliceVector& rSliceVector = getSlices();

            if(rSliceVector.size())
            {
                const bool bBackScale(!basegfx::fTools::equal(mfBackScale, 1.0));
                const bool bClosedRotation(!bBackScale && mnHorizontalSegments && basegfx::fTools::equal(mfRotation, F_2PI));

                // add fill
                if(getSdrLFSAttribute().getFill())
                {
                    sal_uInt32 a;
                    basegfx::B3DRange aRange;

                    // decide what to create
                    const bool bCreateNormals(::com::sun::star::drawing::NormalsKind_SPECIFIC == getSdr3DObjectAttribute().getNormalsKind());
                    const bool bCreateTextureCoordiantesX(::com::sun::star::drawing::TextureProjectionMode_OBJECTSPECIFIC == getSdr3DObjectAttribute().getTextureProjectionX());
                    const bool bCreateTextureCoordiantesY(::com::sun::star::drawing::TextureProjectionMode_OBJECTSPECIFIC == getSdr3DObjectAttribute().getTextureProjectionY());
                    basegfx::B2DHomMatrix aTexTransform;

                    if(bCreateTextureCoordiantesX || bCreateTextureCoordiantesY)
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
                        bCreateNormals, mbSmoothHorizontalNormals, mbSmoothNormals, mbSmoothLids, bClosedRotation,
                        0.85, 0.6, bCreateTextureCoordiantesX || bCreateTextureCoordiantesY, aTexTransform);

                    // get full range
                    for(a = 0L; a < aFill.size(); a++)
                    {
                        aRange.expand(basegfx::tools::getRange(aFill[a]));
                    }

                    // normal creation
                    {
                        if(::com::sun::star::drawing::NormalsKind_SPHERE == getSdr3DObjectAttribute().getNormalsKind())
                        {
                            // create sphere normals
                            const basegfx::B3DPoint aCenter(aRange.getCenter());

                            for(a = 0L; a < aFill.size(); a++)
                            {
                                aFill[a] = basegfx::tools::applyDefaultNormalsSphere(aFill[a], aCenter);
                            }
                        }
                        else if(::com::sun::star::drawing::NormalsKind_FLAT == getSdr3DObjectAttribute().getNormalsKind())
                        {
                            for(a = 0L; a < aFill.size(); a++)
                            {
                                aFill[a].clearNormals();
                            }
                        }

                        if(getSdr3DObjectAttribute().getNormalsInvert())
                        {
                            // invert normals
                            for(a = 0L; a < aFill.size(); a++)
                            {
                                aFill[a] = basegfx::tools::invertNormals(aFill[a]);
                            }
                        }
                    }

                    // texture coordinates
                    {
                        // handle texture coordinates X
                        const bool bParallelX(::com::sun::star::drawing::TextureProjectionMode_PARALLEL == getSdr3DObjectAttribute().getTextureProjectionX());
                        const bool bSphereX(!bParallelX && (::com::sun::star::drawing::TextureProjectionMode_SPHERE == getSdr3DObjectAttribute().getTextureProjectionX()));

                        // handle texture coordinates Y
                        const bool bParallelY(::com::sun::star::drawing::TextureProjectionMode_PARALLEL == getSdr3DObjectAttribute().getTextureProjectionY());
                        const bool bSphereY(!bParallelY && (::com::sun::star::drawing::TextureProjectionMode_SPHERE == getSdr3DObjectAttribute().getTextureProjectionY()));

                        if(bParallelX || bParallelY)
                        {
                            // apply parallel texture coordinates in X and/or Y

                            for(a = 0L; a < aFill.size(); a++)
                            {
                                aFill[a] = basegfx::tools::applyDefaultTextureCoordinatesParallel(aFill[a], aRange, bParallelX, bParallelY);
                            }
                        }

                        if(bSphereX || bSphereY)
                        {
                            // apply spherical texture coordinates in X and/or Y
                            const basegfx::B3DPoint aCenter(aRange.getCenter());

                            for(a = 0L; a < aFill.size(); a++)
                            {
                                aFill[a] = basegfx::tools::applyDefaultTextureCoordinatesSphere(aFill[a], aCenter, bSphereX, bSphereY);
                            }
                        }

                        // transform texture coordinates to texture size
                        basegfx::B2DHomMatrix aTexMatrix;
                        aTexMatrix.scale(maTextureSize.getX(), maTextureSize.getY());

                        for(a = 0L; a < aFill.size(); a++)
                        {
                            aFill[a].transformTextureCoordiantes(aTexMatrix);
                        }
                    }

                    // create single PolyPolygonFill primitives
                    add3DPolyPolygonFillPrimitive(
                        aFill, maTransform, maTextureSize, rTarget,
                        getSdr3DObjectAttribute(), *getSdrLFSAttribute().getFill(),
                        getSdrLFSAttribute().getFillFloatTransGradient());
                }

                // add line
                if(getSdrLFSAttribute().getLine())
                {
                    basegfx::B3DPolyPolygon aLine;
                    extractLinesFromSlice(aLine, rSliceVector, bClosedRotation);
                    add3DPolyPolygonLinePrimitive(aLine, maTransform, rTarget, *maSdrLFSAttribute.getLine());
                }

                // add shadow
                if(getSdrLFSAttribute().getShadow())
                {
                    addShadowPrimitive3D(rTarget, *getSdrLFSAttribute().getShadow(), getSdr3DObjectAttribute().getShadow3D());
                }
            }
        }

        void sdrLathePrimitive3D::impCreateSlices()
        {
            // prepare the polygon
            basegfx::B2DPolyPolygon aCandidate(basegfx::tools::adaptiveSubdivideByDistance(maPolyPolygon));
            aCandidate.removeDoublePoints();
            aCandidate = basegfx::tools::correctOrientations(aCandidate);
            aCandidate = basegfx::tools::correctOutmostPolygon(aCandidate);

            // check edge count of first sub-polygon. If different, reSegment polyPolygon. This ensures
            // that for polyPolygons, the subPolys 1..n only get reSegmented when polygon 0L is different
            // at all (and not always)
            const basegfx::B2DPolygon aSubCandidate(aCandidate.getB2DPolygon(0L));
            const sal_uInt32 nSubEdgeCount(aSubCandidate.isClosed() ? aSubCandidate.count() : (aSubCandidate.count() ? aSubCandidate.count() - 1L : 0L));

            if(nSubEdgeCount != mnVerticalSegments)
            {
                aCandidate = basegfx::tools::reSegmentPolyPolygon(aCandidate, mnVerticalSegments);
            }

            // prepare slices as geometry
            createLatheSlices(maSlices, aCandidate, mfBackScale, mfDiagonal, mfRotation, mnHorizontalSegments, mbCharacterMode, mbCloseFront, mbCloseBack);
        }

        const sliceVector& sdrLathePrimitive3D::getSlices() const
        {
            if(maPolyPolygon.count() && !maSlices.size() && (getSdrLFSAttribute().getFill() || getSdrLFSAttribute().getLine()))
            {
                ::osl::Mutex m_mutex;
                const_cast< sdrLathePrimitive3D& >(*this).impCreateSlices();
            }

            return maSlices;
        }

        sdrLathePrimitive3D::sdrLathePrimitive3D(
            const basegfx::B3DHomMatrix& rTransform,
            const basegfx::B2DVector& rTextureSize,
            const attribute::sdrLineFillShadowAttribute& rSdrLFSAttribute,
            const attribute::sdr3DObjectAttribute& rSdr3DObjectAttribute,
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
        :   sdrPrimitive3D(rTransform, rTextureSize, rSdrLFSAttribute, rSdr3DObjectAttribute),
            maPolyPolygon(rPolyPolygon),
            mnHorizontalSegments(nHorizontalSegments),
            mnVerticalSegments(nVerticalSegments),
            mfDiagonal(fDiagonal),
            mfBackScale(fBackScale),
            mfRotation(fRotation),
            mbSmoothNormals(bSmoothNormals),
            mbSmoothHorizontalNormals(bSmoothHorizontalNormals),
            mbSmoothLids(bSmoothLids),
            mbCharacterMode(bCharacterMode),
            mbCloseFront(bCloseFront),
            mbCloseBack(bCloseBack)
        {
            // make sure Rotation is positive
            if(basegfx::fTools::lessOrEqual(mfRotation, 0.0))
            {
                mfRotation = 0.0;
            }

            // make sure the percentage value mfDiagonal is between 0.0 and 1.0
            if(basegfx::fTools::lessOrEqual(mfDiagonal, 0.0))
            {
                mfDiagonal = 0.0;
            }
            else if(basegfx::fTools::moreOrEqual(mfDiagonal, 1.0))
            {
                mfDiagonal = 1.0;
            }

            // no close front/back when polygon is not closed
            if(maPolyPolygon.count() && !maPolyPolygon.getB2DPolygon(0L).isClosed())
            {
                mbCloseFront = mbCloseBack = false;
            }

            // no edge rounding when not closing
            if(!mbCloseFront && !mbCloseBack)
            {
                mfDiagonal = 0.0;
            }
        }

        sdrLathePrimitive3D::~sdrLathePrimitive3D()
        {
        }

        bool sdrLathePrimitive3D::operator==(const basePrimitive3D& rPrimitive) const
        {
            if(sdrPrimitive3D::operator==(rPrimitive))
            {
                const sdrLathePrimitive3D& rCompare = static_cast< const sdrLathePrimitive3D& >(rPrimitive);
                return (maPolyPolygon == rCompare.maPolyPolygon
                    && mnHorizontalSegments == rCompare.mnHorizontalSegments
                    && mnVerticalSegments == rCompare.mnVerticalSegments
                    && mfDiagonal == rCompare.mfDiagonal
                    && mfBackScale == rCompare.mfBackScale
                    && mfRotation == rCompare.mfRotation
                    && mbSmoothNormals == rCompare.mbSmoothNormals
                    && mbSmoothHorizontalNormals == rCompare.mbSmoothHorizontalNormals
                    && mbSmoothLids == rCompare.mbSmoothLids
                    && mbCharacterMode == rCompare.mbCharacterMode
                    && mbCloseFront == rCompare.mbCloseFront
                    && mbCloseBack == rCompare.mbCloseBack);
            }

            return false;
        }

        PrimitiveID sdrLathePrimitive3D::getID() const
        {
            return CreatePrimitiveID('S', 'L', 'A', '3');
        }

        basegfx::B3DRange sdrLathePrimitive3D::get3DRange() const
        {
            // use defaut from sdrPrimitive3D which uses transformation expanded by line width/2
            // The parent implementation which uses the ranges of the decomposition would be more
            // corrcet, but for historical reasons it is necessary to do the old method: To get
            // the range of the non-transformed geometry and transform it then. This leads to different
            // ranges where the new method is more correct, but the need to keep the old behaviour
            // has priority here.
            return get3DRangeFromSlices(getSlices());
        }
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
