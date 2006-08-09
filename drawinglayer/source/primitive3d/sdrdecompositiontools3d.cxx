/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrdecompositiontools3d.cxx,v $
 *
 *  $Revision: 1.3 $
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

#ifndef _DRAWINGLAYER_PRIMITIVE3D_SDRDECOMPOSITIONTOOLS3D_HXX
#include <drawinglayer/primitive3d/sdrdecompositiontools3d.hxx>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYGON_HXX
#include <basegfx/polygon/b3dpolygon.hxx>
#endif

#ifndef _DRAWINGLAYER_ATTRIBUTE_STROKEATTRIBUTE_HXX
#include <drawinglayer/attribute/strokeattribute.hxx>
#endif

#ifndef _DRAWINGLAYER_ATTRIBUTE_SDRATTRIBUTE_HXX
#include <drawinglayer/attribute/sdrattribute.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE3D_PRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/primitive3d.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE3D_POLYGONPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/polygonprimitive3d.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE_SIMPLETRANSPARENCEPRIMITIVE_HXX
#include <drawinglayer/primitive/simpletransparenceprimitive.hxx>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYPOLYGON_HXX
#include <basegfx/polygon/b3dpolypolygon.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE3D_POLYPOLYGONPRIMITIVE_HXX
#include <drawinglayer/primitive3d/polypolygonprimitive3d.hxx>
#endif

#ifndef _DRAWINGLAYER_ATTRIBUTE_SDRATTRIBUTE_HXX
#include <drawinglayer/attribute/sdrattribute.hxx>
#endif

#ifndef _VCL_VCLENUM_HXX
#include <vcl/vclenum.hxx>
#endif

#ifndef _DRAWINGLAYER_ATTRIBUTE_FILLATTRIBUTE_HXX
#include <drawinglayer/attribute/fillattribute.hxx>
#endif

#ifndef _DRAWINGLAYER_ATTRIBUTE_FILLBITMAPATTRIBUTE_HXX
#include <drawinglayer/attribute/fillbitmapattribute.hxx>
#endif

#ifndef _DRAWINGLAYER_ATTRIBUTE_SDRFILLBITMAPATTRIBUTE_HXX
#include <drawinglayer/attribute/sdrfillbitmapattribute.hxx>
#endif

#ifndef _SV_BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B3DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE3D_TEXTUREPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/textureprimitive3d.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE3D_MODIFIEDCOLORPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/modifiedcolorprimitive3d.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE3D_HATCHTEXTUREPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/hatchtextureprimitive3d.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE3D_SHADOWPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/shadowprimitive3d.hxx>
#endif

#ifndef _DRAWINGLAYER_ATTRIBUTE_SDRATTRIBUTE3D_HXX
#include <drawinglayer/attribute/sdrattribute3d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        void add3DPolyPolygonLinePrimitive(
            const basegfx::B3DPolyPolygon& rUnitPolyPolygon,
            const basegfx::B3DHomMatrix& rObjectTransform,
            primitiveVector3D& rTarget,
            const attribute::sdrLineAttribute& rLine)
        {
            // prepare fully scaled polyPolygon
            basegfx::B3DPolyPolygon aScaledPolyPolygon(rUnitPolyPolygon);
            aScaledPolyPolygon.transform(rObjectTransform);

            // create stroke attribute
            const attribute::strokeAttribute aStrokeAttribute(rLine.getColor(), rLine.getWidth(), rLine.getJoin(), rLine.getDotDashArray(), rLine.getFullDotDashLen());

            // create primitives
            primitiveVector3D aNewPrimitiveVector;

            for(sal_uInt32 a(0L); a < aScaledPolyPolygon.count(); a++)
            {
                basePrimitive3D* pNewLinePrimitive = new polygonStrokePrimitive3D(aScaledPolyPolygon.getB3DPolygon(a), aStrokeAttribute);
                aNewPrimitiveVector.push_back(referencedPrimitive3D(*pNewLinePrimitive));
            }

            if(0.0 != rLine.getTransparence())
            {
                // create simpleTransparencePrimitive, add created fill primitives
                basePrimitive3D* pNewSimpleTransparenceTexturePrimitive3D = new simpleTransparenceTexturePrimitive3D(rLine.getTransparence(), aNewPrimitiveVector);
                rTarget.push_back(referencedPrimitive3D(*pNewSimpleTransparenceTexturePrimitive3D));
            }
            else
            {
                // add to decomposition
                rTarget.insert(rTarget.end(), aNewPrimitiveVector.begin(), aNewPrimitiveVector.end());
            }
        }

        void add3DPolyPolygonFillPrimitive(
            const ::std::vector< basegfx::B3DPolyPolygon >& r3DPolyPolygonVector,
            const basegfx::B3DHomMatrix& rObjectTransform,
            const basegfx::B2DVector& rTextureSize,
            primitiveVector3D& rTarget,
            const attribute::sdr3DObjectAttribute& aSdr3DObjectAttribute,
            const attribute::sdrFillAttribute& rFill,
            const attribute::fillGradientAttribute* pFillGradient)
        {
            if(r3DPolyPolygonVector.size())
            {
                // create list of simple fill primitives
                primitiveVector3D aNewPrimitiveVector;

                for(sal_uInt32 a(0L); a < r3DPolyPolygonVector.size(); a++)
                {
                    // get scaled PolyPolygon
                    basegfx::B3DPolyPolygon aScaledPolyPolygon(r3DPolyPolygonVector[a]);
                    aScaledPolyPolygon.transform(rObjectTransform);

                    if(aScaledPolyPolygon.areNormalsUsed())
                    {
                        aScaledPolyPolygon.transformNormals(rObjectTransform);
                    }

                    basePrimitive3D* pNewFillPrimitive = new polyPolygonMaterialPrimitive3D(
                        aScaledPolyPolygon,
                        aSdr3DObjectAttribute.getMaterial(),
                        aSdr3DObjectAttribute.getDoubleSided());
                    aNewPrimitiveVector.push_back(referencedPrimitive3D(*pNewFillPrimitive));
                }

                // look for and evtl. build texture sub-group primitive
                if(rFill.isGradient() || rFill.isHatch() || rFill.isBitmap())
                {
                    bool bModulate(::com::sun::star::drawing::TextureMode_MODULATE == aSdr3DObjectAttribute.getTextureMode());
                    bool bFilter(aSdr3DObjectAttribute.getTextureFilter());
                    basePrimitive3D* pNewTexturePrimitive3D = 0L;

                    if(rFill.isGradient())
                    {
                        // create gradientTexture3D with sublist, add to local aNewPrimitiveVector
                        pNewTexturePrimitive3D = new gradientTexturePrimitive3D(*rFill.getGradient(), aNewPrimitiveVector, rTextureSize, bModulate, bFilter);
                    }
                    else if(rFill.isHatch())
                    {
                        // create hatchTexture3D with sublist, add to local aNewPrimitiveVector
                        pNewTexturePrimitive3D = new hatchTexturePrimitive3D(*rFill.getHatch(), aNewPrimitiveVector, rTextureSize, bModulate, bFilter);
                    }
                    else // if(rFill.isBitmap())
                    {
                        // create bitmapTexture3D with sublist, add to local aNewPrimitiveVector
                        basegfx::B2DRange aTexRange(0.0, 0.0, rTextureSize.getX(), rTextureSize.getY());
                        pNewTexturePrimitive3D = new bitmapTexturePrimitive3D(rFill.getBitmap()->getFillBitmapAttribute(aTexRange), aNewPrimitiveVector, rTextureSize, bModulate, bFilter);
                    }

                    // exchange aNewPrimitiveVector content with texture group
                    aNewPrimitiveVector.clear();
                    aNewPrimitiveVector.push_back(referencedPrimitive3D(*pNewTexturePrimitive3D));

                    if(::com::sun::star::drawing::TextureKind2_LUMINANCE == aSdr3DObjectAttribute.getTextureKind())
                    {
                        // use modified color primitive to force textures to gray
                        const basegfx::BColorModifier aBColorModifier(basegfx::BColor(), 0.0, basegfx::BCOLORMODIFYMODE_GRAY);
                        basePrimitive3D* pModifiedColor = new modifiedColorPrimitive3D(aNewPrimitiveVector, aBColorModifier);
                        aNewPrimitiveVector.clear();
                        aNewPrimitiveVector.push_back(referencedPrimitive3D(*pModifiedColor));
                    }
                }

                if(0.0 != rFill.getTransparence())
                {
                    // create simpleTransparenceTexturePrimitive3D with sublist and append
                    basePrimitive3D* pNewSimpleTransparenceTexturePrimitive3D = new simpleTransparenceTexturePrimitive3D(rFill.getTransparence(), aNewPrimitiveVector);
                    rTarget.push_back(referencedPrimitive3D(*pNewSimpleTransparenceTexturePrimitive3D));
                }
                else if(pFillGradient)
                {
                    // create transparenceTexture3D with sublist and append
                    basePrimitive3D* pNewTransparenceTexturePrimitive3D = new transparenceTexturePrimitive3D(*pFillGradient, aNewPrimitiveVector, rTextureSize);
                    rTarget.push_back(referencedPrimitive3D(*pNewTransparenceTexturePrimitive3D));
                }
                else
                {
                    // append list
                    rTarget.insert(rTarget.end(), aNewPrimitiveVector.begin(), aNewPrimitiveVector.end());
                }
            }
        }

        void addShadowPrimitive3D(
            primitiveVector3D& rTarget,
            const attribute::sdrShadowAttribute& rShadow,
            bool bShadow3D)
        {
            // create Shadow primitives. Need to be added in front, should use already created primitives
            if(rTarget.size() && !basegfx::fTools::moreOrEqual(rShadow.getTransparence(), 1.0))
            {
                // prepare new list for shadow geometry
                primitiveVector3D aNewList;

                // prepare shadow offset
                basegfx::B2DHomMatrix   aShadowOffset;
                aShadowOffset.set(0, 2, rShadow.getOffset().getX());
                aShadowOffset.set(1, 2, rShadow.getOffset().getY());

                // create shadow primitive and add primitives
                shadowPrimitive3D* pNewShadow3D = new shadowPrimitive3D(aShadowOffset, rShadow.getColor(), rShadow.getTransparence(), bShadow3D, rTarget);
                rTarget.clear();
                rTarget.push_back(referencedPrimitive3D(*pNewShadow3D));
            }
        }
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
