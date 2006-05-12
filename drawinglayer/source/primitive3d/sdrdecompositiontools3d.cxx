/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrdecompositiontools3d.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2006-05-12 11:49:08 $
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

#ifndef _DRAWINGLAYER_PRIMITIVE_STROKEATTRIBUTE_HXX
#include <drawinglayer/primitive/strokeattribute.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE_SDRATTRIBUTE_HXX
#include <drawinglayer/primitive/sdrattribute.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE_PRIMITIVE_HXX
#include <drawinglayer/primitive/primitive.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE3D_POLYGONPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/polygonprimitive3d.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE_PRIMITIVELIST_HXX
#include <drawinglayer/primitive/primitivelist.hxx>
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

#ifndef _DRAWINGLAYER_PRIMITIVE3D_SDRATTRIBUTE3D_HXX
#include <drawinglayer/primitive3d/sdrattribute3d.hxx>
#endif

#ifndef _VCL_VCLENUM_HXX
#include <vcl/vclenum.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE_FILLATTRIBUTE_HXX
#include <drawinglayer/primitive/fillattribute.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE_FILLBITMAPATTRIBUTE_HXX
#include <drawinglayer/primitive/fillbitmapattribute.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE_SDRFILLBITMAPATTRIBUTE_HXX
#include <drawinglayer/primitive/sdrfillbitmapattribute.hxx>
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

#ifndef _DRAWINGLAYER_PRIMITIVE_MODIFIEDCOLORPRIMITIVE_HXX
#include <drawinglayer/primitive/modifiedcolorprimitive.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE3D_HATCHTEXTUREPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/hatchtextureprimitive3d.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE3D_SHADOWPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/shadowprimitive3d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive
    {
        void add3DPolyPolygonLinePrimitive(
            const ::basegfx::B3DPolyPolygon& rUnitPolyPolygon,
            const ::basegfx::B3DHomMatrix& rObjectTransform,
            primitiveList& rTarget,
            const sdrLineAttribute& rLine)
        {
            // prepare fully scaled polyPolygon
            ::basegfx::B3DPolyPolygon aScaledPolyPolygon(rUnitPolyPolygon);
            aScaledPolyPolygon.transform(rObjectTransform);

            // create stroke attribute
            const strokeAttribute aStrokeAttribute(rLine.getColor(), rLine.getWidth(), rLine.getJoin(), rLine.getDotDashArray(), rLine.getFullDotDashLen());

            // create primitives
            primitiveList aNewPrimitiveList;

            for(sal_uInt32 a(0L); a < aScaledPolyPolygon.count(); a++)
            {
                basePrimitive* pNewLinePrimitive = new polygonStrokePrimitive3D(aScaledPolyPolygon.getB3DPolygon(a), aStrokeAttribute);
                aNewPrimitiveList.append(referencedPrimitive(*pNewLinePrimitive));
            }

            if(0.0 != rLine.getTransparence())
            {
                // create simpleTransparencePrimitive, add created fill primitives
                basePrimitive* pNewSimpleTransparenceTexturePrimitive3D = new simpleTransparenceTexturePrimitive3D(rLine.getTransparence(), aNewPrimitiveList);
                rTarget.append(referencedPrimitive(*pNewSimpleTransparenceTexturePrimitive3D));
            }
            else
            {
                // add to decomposition
                rTarget.append(aNewPrimitiveList);
            }
        }

        void add3DPolyPolygonFillPrimitive(
            const ::std::vector< ::basegfx::B3DPolyPolygon >& r3DPolyPolygonVector,
            const ::basegfx::B3DHomMatrix& rObjectTransform,
            const ::basegfx::B2DVector& rTextureSize,
            primitiveList& rTarget,
            const sdr3DObjectAttribute& aSdr3DObjectAttribute,
            const sdrFillAttribute& rFill,
            const fillGradientAttribute* pFillGradient)
        {
            if(r3DPolyPolygonVector.size())
            {
                // create list of simple fill primitives
                primitiveList aNewPrimitiveList;

                for(sal_uInt32 a(0L); a < r3DPolyPolygonVector.size(); a++)
                {
                    // get scaled PolyPolygon
                    ::basegfx::B3DPolyPolygon aScaledPolyPolygon(r3DPolyPolygonVector[a]);
                    aScaledPolyPolygon.transform(rObjectTransform);

                    if(aScaledPolyPolygon.areNormalsUsed())
                    {
                        aScaledPolyPolygon.transformNormals(rObjectTransform);
                    }

                    basePrimitive* pNewFillPrimitive = new polyPolygonMaterialPrimitive3D(
                        aScaledPolyPolygon,
                        aSdr3DObjectAttribute.getMaterial(),
                        aSdr3DObjectAttribute.getDoubleSided());
                    aNewPrimitiveList.append(referencedPrimitive(*pNewFillPrimitive));
                }

                // look for and evtl. build texture sub-group primitive
                if(rFill.isGradient() || rFill.isHatch() || rFill.isBitmap())
                {
                    bool bModulate(::com::sun::star::drawing::TextureMode_MODULATE == aSdr3DObjectAttribute.getTextureMode());
                    bool bFilter(aSdr3DObjectAttribute.getTextureFilter());
                    basePrimitive* pNewTexturePrimitive3D = 0L;

                    if(rFill.isGradient())
                    {
                        // create gradientTexture3D with sublist, add to local aNewPrimitiveList
                        pNewTexturePrimitive3D = new gradientTexturePrimitive3D(*rFill.getGradient(), aNewPrimitiveList, rTextureSize, bModulate, bFilter);
                    }
                    else if(rFill.isHatch())
                    {
                        // create hatchTexture3D with sublist, add to local aNewPrimitiveList
                        pNewTexturePrimitive3D = new hatchTexturePrimitive3D(*rFill.getHatch(), aNewPrimitiveList, rTextureSize, bModulate, bFilter);
                    }
                    else // if(rFill.isBitmap())
                    {
                        // create bitmapTexture3D with sublist, add to local aNewPrimitiveList
                        ::basegfx::B2DRange aTexRange(0.0, 0.0, rTextureSize.getX(), rTextureSize.getY());
                        pNewTexturePrimitive3D = new bitmapTexturePrimitive3D(rFill.getBitmap()->getFillBitmapAttribute(aTexRange), aNewPrimitiveList, rTextureSize, bModulate, bFilter);
                    }

                    // exchange aNewPrimitiveList content with texture group
                    aNewPrimitiveList.clear();
                    aNewPrimitiveList.append(referencedPrimitive(*pNewTexturePrimitive3D));

                    if(::com::sun::star::drawing::TextureKind2_LUMINANCE == aSdr3DObjectAttribute.getTextureKind())
                    {
                        // use modified color primitive to force textures to gray
                        basePrimitive* pModifiedColor = new modifiedColorPrimitive(::basegfx::BColor(), aNewPrimitiveList, 0.0, COLORMODIFY_GRAY);
                        aNewPrimitiveList.clear();
                        aNewPrimitiveList.append(referencedPrimitive(*pModifiedColor));
                    }
                }

                if(0.0 != rFill.getTransparence())
                {
                    // create simpleTransparenceTexturePrimitive3D with sublist and append
                    basePrimitive* pNewSimpleTransparenceTexturePrimitive3D = new simpleTransparenceTexturePrimitive3D(rFill.getTransparence(), aNewPrimitiveList);
                    rTarget.append(referencedPrimitive(*pNewSimpleTransparenceTexturePrimitive3D));
                }
                else if(pFillGradient)
                {
                    // create transparenceTexture3D with sublist and append
                    basePrimitive* pNewTransparenceTexturePrimitive3D = new transparenceTexturePrimitive3D(*pFillGradient, aNewPrimitiveList, rTextureSize);
                    rTarget.append(referencedPrimitive(*pNewTransparenceTexturePrimitive3D));
                }
                else
                {
                    // append list
                    rTarget.append(aNewPrimitiveList);
                }
            }
        }

        void addShadowPrimitive3D(
            primitiveList& rTarget,
            const sdrShadowAttribute& rShadow,
            bool bShadow3D)
        {
            // create Shadow primitives. Need to be added in front, should use already created primitives
            if(rTarget.count() && !::basegfx::fTools::moreOrEqual(rShadow.getTransparence(), 1.0))
            {
                // prepare new list for shadow geometry
                primitiveList aNewList;

                // prepare shadow offset
                ::basegfx::B2DHomMatrix aShadowOffset;
                aShadowOffset.set(0, 2, rShadow.getOffset().getX());
                aShadowOffset.set(1, 2, rShadow.getOffset().getY());

                // create shadow primitive and add primitives
                primitiveList aShadowPrimitiveList(rTarget);
                shadowPrimitive3D* pNewShadow3D = new shadowPrimitive3D(aShadowOffset, rShadow.getColor(), rShadow.getTransparence(), bShadow3D, aShadowPrimitiveList);
                rTarget.clear();
                rTarget.append(referencedPrimitive(*pNewShadow3D));
            }
        }
    } // end of namespace primitive
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
