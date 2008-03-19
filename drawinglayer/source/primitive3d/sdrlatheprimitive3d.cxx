/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrlatheprimitive3d.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: aw $ $Date: 2008-03-19 04:35:22 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRLATHEPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/sdrlatheprimitive3d.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B3DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRDECOMPOSITIONTOOLS3D_HXX
#include <drawinglayer/primitive3d/sdrdecompositiontools3d.hxx>
#endif

#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_PRIMITIVETYPES3D_HXX
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        Primitive3DSequence SdrLathePrimitive3D::createLocalDecomposition(double /*fTime*/) const
        {
            Primitive3DSequence aRetval;

            // get slices
            const Slice3DVector& rSliceVector = getSlices();

            if(rSliceVector.size())
            {
                const bool bBackScale(!basegfx::fTools::equal(getBackScale(), 1.0));
                const bool bClosedRotation(!bBackScale && getHorizontalSegments() && basegfx::fTools::equal(getRotation(), F_2PI));

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
                        bCreateNormals, getSmoothHorizontalNormals(), getSmoothNormals(), getSmoothLids(), bClosedRotation,
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
                        aTexMatrix.scale(getTextureSize().getX(), getTextureSize().getY());

                        for(a = 0L; a < aFill.size(); a++)
                        {
                            aFill[a].transformTextureCoordiantes(aTexMatrix);
                        }
                    }

                    // create single PolyPolygonFill primitives
                    aRetval = create3DPolyPolygonFillPrimitives(
                        aFill, getTransform(), getTextureSize(),
                        getSdr3DObjectAttribute(), *getSdrLFSAttribute().getFill(),
                        getSdrLFSAttribute().getFillFloatTransGradient());
                }

                // add line
                if(getSdrLFSAttribute().getLine())
                {
                    basegfx::B3DPolyPolygon aLine;
                    extractLinesFromSlice(aLine, rSliceVector, bClosedRotation,
                        true,
                        !getSdr3DObjectAttribute().getReducedLineGeometry());
                    const Primitive3DSequence aLines(create3DPolyPolygonLinePrimitives(aLine, getTransform(), *getSdrLFSAttribute().getLine()));
                    appendPrimitive3DSequenceToPrimitive3DSequence(aRetval, aLines);
                }

                // add shadow
                if(getSdrLFSAttribute().getShadow() && aRetval.hasElements())
                {
                    const Primitive3DSequence aShadow(createShadowPrimitive3D(aRetval, *getSdrLFSAttribute().getShadow(), getSdr3DObjectAttribute().getShadow3D()));
                    appendPrimitive3DSequenceToPrimitive3DSequence(aRetval, aShadow);
                }
            }

            return aRetval;
        }

        void SdrLathePrimitive3D::impCreateSlices()
        {
            // prepare the polygon
            basegfx::B2DPolyPolygon aCandidate(basegfx::tools::adaptiveSubdivideByAngle(getPolyPolygon()));
            aCandidate.removeDoublePoints();
            aCandidate = basegfx::tools::correctOrientations(aCandidate);
            aCandidate = basegfx::tools::correctOutmostPolygon(aCandidate);

            // check edge count of first sub-polygon. If different, reSegment polyPolygon. This ensures
            // that for polyPolygons, the subPolys 1..n only get reSegmented when polygon 0L is different
            // at all (and not always)
            const basegfx::B2DPolygon aSubCandidate(aCandidate.getB2DPolygon(0L));
            const sal_uInt32 nSubEdgeCount(aSubCandidate.isClosed() ? aSubCandidate.count() : (aSubCandidate.count() ? aSubCandidate.count() - 1L : 0L));

            if(nSubEdgeCount != getVerticalSegments())
            {
                aCandidate = basegfx::tools::reSegmentPolyPolygon(aCandidate, getVerticalSegments());
            }

            // prepare slices as geometry
            createLatheSlices(maSlices, aCandidate, getBackScale(), getDiagonal(), getRotation(), getHorizontalSegments(), getCharacterMode(), getCloseFront(), getCloseBack());
        }

        const Slice3DVector& SdrLathePrimitive3D::getSlices() const
        {
            if(getPolyPolygon().count() && !maSlices.size() && (getSdrLFSAttribute().getFill() || getSdrLFSAttribute().getLine()))
            {
                ::osl::Mutex m_mutex;
                const_cast< SdrLathePrimitive3D& >(*this).impCreateSlices();
            }

            return maSlices;
        }

        SdrLathePrimitive3D::SdrLathePrimitive3D(
            const basegfx::B3DHomMatrix& rTransform,
            const basegfx::B2DVector& rTextureSize,
            const attribute::SdrLineFillShadowAttribute& rSdrLFSAttribute,
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

        basegfx::B3DRange SdrLathePrimitive3D::getB3DRange(double /*fTime*/) const
        {
            // use defaut from sdrPrimitive3D which uses transformation expanded by line width/2
            // The parent implementation which uses the ranges of the decomposition would be more
            // corrcet, but for historical reasons it is necessary to do the old method: To get
            // the range of the non-transformed geometry and transform it then. This leads to different
            // ranges where the new method is more correct, but the need to keep the old behaviour
            // has priority here.
            return get3DRangeFromSlices(getSlices());
        }

        // provide unique ID
        ImplPrimitrive3DIDBlock(SdrLathePrimitive3D, PRIMITIVE3D_ID_SDRLATHEPRIMITIVE3D)

    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
