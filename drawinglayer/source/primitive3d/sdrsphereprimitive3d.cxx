/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrsphereprimitive3d.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: aw $ $Date: 2008-03-05 09:15:44 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRSPHEREPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/sdrsphereprimitive3d.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B3DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYGON_HXX
#include <basegfx/polygon/b3dpolygon.hxx>
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
        Primitive3DSequence SdrSpherePrimitive3D::createLocalDecomposition(double /*fTime*/) const
        {
            Primitive3DSequence aRetval;
            const basegfx::B3DRange aUnitRange(0.0, 0.0, 0.0, 1.0, 1.0, 1.0);

            // add fill
            if(getSdrLFSAttribute().getFill())
            {
                const bool bCreateNormals(::com::sun::star::drawing::NormalsKind_SPECIFIC == getSdr3DObjectAttribute().getNormalsKind()
                    || ::com::sun::star::drawing::NormalsKind_SPHERE == getSdr3DObjectAttribute().getNormalsKind());

                // create unit geometry
                basegfx::B3DPolyPolygon aFill(basegfx::tools::createSphereFillPolyPolygonFromB3DRange(aUnitRange,
                    getHorizontalSegments(), getVerticalSegments(), bCreateNormals));

                // normal inversion
                if(bCreateNormals && getSdr3DObjectAttribute().getNormalsInvert() && aFill.areNormalsUsed())
                {
                    // invert normals
                    aFill = basegfx::tools::invertNormals(aFill);
                }

                // texture coordinates
                {
                    // handle texture coordinates X
                    const bool bParallelX(::com::sun::star::drawing::TextureProjectionMode_PARALLEL == getSdr3DObjectAttribute().getTextureProjectionX());
                    const bool bObjectSpecificX(::com::sun::star::drawing::TextureProjectionMode_OBJECTSPECIFIC == getSdr3DObjectAttribute().getTextureProjectionX());
                    const bool bSphereX(::com::sun::star::drawing::TextureProjectionMode_SPHERE == getSdr3DObjectAttribute().getTextureProjectionX());

                    // handle texture coordinates Y
                    const bool bParallelY(::com::sun::star::drawing::TextureProjectionMode_PARALLEL == getSdr3DObjectAttribute().getTextureProjectionY());
                    const bool bObjectSpecificY(::com::sun::star::drawing::TextureProjectionMode_OBJECTSPECIFIC == getSdr3DObjectAttribute().getTextureProjectionY());
                    const bool bSphereY(::com::sun::star::drawing::TextureProjectionMode_SPHERE == getSdr3DObjectAttribute().getTextureProjectionY());

                    if(bParallelX || bParallelY)
                    {
                        // apply parallel texture coordinates in X and/or Y
                        const basegfx::B3DRange aRange(basegfx::tools::getRange(aFill));
                        aFill = basegfx::tools::applyDefaultTextureCoordinatesParallel(aFill, aRange, bParallelX, bParallelY);
                    }

                    if(bSphereX || bObjectSpecificX || bSphereY || bObjectSpecificY)
                    {
                        double fRelativeAngle(0.0);

                        if(bObjectSpecificX)
                        {
                            // Since the texture coordinates are (for historical reasons)
                            // different from forced to sphere texture coordinates,
                            // create a old version from it by rotating to old state before applying
                            // the texture coordinates to emulate old behaviour
                            fRelativeAngle = F_2PI * ((double)((getHorizontalSegments() >> 1L)  - 1L) / (double)getHorizontalSegments());
                            basegfx::B3DHomMatrix aRot;
                            aRot.rotate(0.0, fRelativeAngle, 0.0);
                            aFill.transform(aRot);
                        }

                        // apply spherical texture coordinates in X and/or Y
                        const basegfx::B3DRange aRange(basegfx::tools::getRange(aFill));
                        const basegfx::B3DPoint aCenter(aRange.getCenter());
                        aFill = basegfx::tools::applyDefaultTextureCoordinatesSphere(aFill, aCenter,
                            bSphereX || bObjectSpecificX, bSphereY || bObjectSpecificY);

                        if(bObjectSpecificX)
                        {
                            // rotate back again
                            basegfx::B3DHomMatrix aRot;
                            aRot.rotate(0.0, -fRelativeAngle, 0.0);
                            aFill.transform(aRot);
                        }
                    }

                    // transform texture coordinates to texture size
                    basegfx::B2DHomMatrix aTexMatrix;
                    aTexMatrix.scale(getTextureSize().getX(), getTextureSize().getY());
                    aFill.transformTextureCoordiantes(aTexMatrix);
                }

                // build vector of PolyPolygons
                ::std::vector< basegfx::B3DPolyPolygon > a3DPolyPolygonVector;

                for(sal_uInt32 a(0L); a < aFill.count(); a++)
                {
                    a3DPolyPolygonVector.push_back(basegfx::B3DPolyPolygon(aFill.getB3DPolygon(a)));
                }

                // create single PolyPolygonFill primitives
                aRetval = create3DPolyPolygonFillPrimitives(
                    a3DPolyPolygonVector, getTransform(), getTextureSize(),
                    getSdr3DObjectAttribute(), *getSdrLFSAttribute().getFill(),
                    getSdrLFSAttribute().getFillFloatTransGradient());
            }

            // add line
            if(getSdrLFSAttribute().getLine())
            {
                basegfx::B3DPolyPolygon aSphere(basegfx::tools::createSpherePolyPolygonFromB3DRange(aUnitRange, getHorizontalSegments(), getVerticalSegments()));
                const Primitive3DSequence aLines(create3DPolyPolygonLinePrimitives(aSphere, getTransform(), *getSdrLFSAttribute().getLine()));
                appendPrimitive3DSequenceToPrimitive3DSequence(aRetval, aLines);
            }

            // add shadow
            if(getSdrLFSAttribute().getShadow() && aRetval.hasElements())
            {
                const Primitive3DSequence aShadow(createShadowPrimitive3D(aRetval, *getSdrLFSAttribute().getShadow(), getSdr3DObjectAttribute().getShadow3D()));
                appendPrimitive3DSequenceToPrimitive3DSequence(aRetval, aShadow);
            }

            return aRetval;
        }

        SdrSpherePrimitive3D::SdrSpherePrimitive3D(
            const basegfx::B3DHomMatrix& rTransform,
            const basegfx::B2DVector& rTextureSize,
            const attribute::SdrLineFillShadowAttribute& rSdrLFSAttribute,
            const attribute::Sdr3DObjectAttribute& rSdr3DObjectAttribute,
            sal_uInt32 nHorizontalSegments,
            sal_uInt32 nVerticalSegments)
        :   SdrPrimitive3D(rTransform, rTextureSize, rSdrLFSAttribute, rSdr3DObjectAttribute),
            mnHorizontalSegments(nHorizontalSegments),
            mnVerticalSegments(nVerticalSegments)
        {
        }

        bool SdrSpherePrimitive3D::operator==(const BasePrimitive3D& rPrimitive) const
        {
            if(SdrPrimitive3D::operator==(rPrimitive))
            {
                const SdrSpherePrimitive3D& rCompare = static_cast< const SdrSpherePrimitive3D& >(rPrimitive);

                return (getHorizontalSegments() == rCompare.getHorizontalSegments()
                    && getVerticalSegments() == rCompare.getVerticalSegments());
            }

            return false;
        }

        basegfx::B3DRange SdrSpherePrimitive3D::getB3DRange(double /*fTime*/) const
        {
            // use defaut from sdrPrimitive3D which uses transformation expanded by line width/2
            // The parent implementation which uses the ranges of the decomposition would be more
            // corrcet, but for historical reasons it is necessary to do the old method: To get
            // the range of the non-transformed geometry and transform it then. This leads to different
            // ranges where the new method is more correct, but the need to keep the old behaviour
            // has priority here.
            return getStandard3DRange();
        }

        // provide unique ID
        ImplPrimitrive3DIDBlock(SdrSpherePrimitive3D, PRIMITIVE3D_ID_SDRSPHEREPRIMITIVE3D)

    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
