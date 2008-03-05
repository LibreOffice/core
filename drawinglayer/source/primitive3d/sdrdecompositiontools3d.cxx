/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrdecompositiontools3d.cxx,v $
 *
 *  $Revision: 1.6 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRDECOMPOSITIONTOOLS3D_HXX
#include <drawinglayer/primitive3d/sdrdecompositiontools3d.hxx>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYGON_HXX
#include <basegfx/polygon/b3dpolygon.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_STROKEATTRIBUTE_HXX
#include <drawinglayer/attribute/strokeattribute.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRATTRIBUTE_HXX
#include <drawinglayer/attribute/sdrattribute.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_BASEPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/baseprimitive3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYGONPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/polygonprimitive3d.hxx>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYPOLYGON_HXX
#include <basegfx/polygon/b3dpolypolygon.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYPOLYGONPRIMITIVE_HXX
#include <drawinglayer/primitive3d/polypolygonprimitive3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRATTRIBUTE_HXX
#include <drawinglayer/attribute/sdrattribute.hxx>
#endif

#ifndef _VCL_VCLENUM_HXX
#include <vcl/vclenum.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLATTRIBUTE_HXX
#include <drawinglayer/attribute/fillattribute.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLBITMAPATTRIBUTE_HXX
#include <drawinglayer/attribute/fillbitmapattribute.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRFILLBITMAPATTRIBUTE_HXX
#include <drawinglayer/attribute/sdrfillbitmapattribute.hxx>
#endif

#ifndef _SV_BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B3DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_TEXTUREPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/textureprimitive3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_MODIFIEDCOLORPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/modifiedcolorprimitive3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_HATCHTEXTUREPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/hatchtextureprimitive3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SHADOWPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/shadowprimitive3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRATTRIBUTE3D_HXX
#include <drawinglayer/attribute/sdrattribute3d.hxx>
#endif

#ifndef _BGFX_RANGE_B2DRANGE_HXX
#include <basegfx/range/b2drange.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        Primitive3DSequence create3DPolyPolygonLinePrimitives(
            const basegfx::B3DPolyPolygon& rUnitPolyPolygon,
            const basegfx::B3DHomMatrix& rObjectTransform,
            const attribute::SdrLineAttribute& rLine)
        {
            // prepare fully scaled polyPolygon
            basegfx::B3DPolyPolygon aScaledPolyPolygon(rUnitPolyPolygon);
            aScaledPolyPolygon.transform(rObjectTransform);

            // create line and stroke attribute
            const attribute::LineAttribute aLineAttribute(rLine.getColor(), rLine.getWidth(), rLine.getJoin());
            const attribute::StrokeAttribute aStrokeAttribute(rLine.getDotDashArray(), rLine.getFullDotDashLen());

            // create primitives
            Primitive3DSequence aRetval(aScaledPolyPolygon.count());

            for(sal_uInt32 a(0L); a < aScaledPolyPolygon.count(); a++)
            {
                const Primitive3DReference xRef(new PolygonStrokePrimitive3D(aScaledPolyPolygon.getB3DPolygon(a), aLineAttribute, aStrokeAttribute));
                aRetval[a] = xRef;
            }

            if(0.0 != rLine.getTransparence())
            {
                // create UnifiedAlphaTexturePrimitive3D, add created primitives and exchange
                const Primitive3DReference xRef(new UnifiedAlphaTexturePrimitive3D(rLine.getTransparence(), aRetval));
                aRetval = Primitive3DSequence(&xRef, 1L);
            }

            return aRetval;
        }

        Primitive3DSequence create3DPolyPolygonFillPrimitives(
            const ::std::vector< basegfx::B3DPolyPolygon >& r3DPolyPolygonVector,
            const basegfx::B3DHomMatrix& rObjectTransform,
            const basegfx::B2DVector& rTextureSize,
            const attribute::Sdr3DObjectAttribute& aSdr3DObjectAttribute,
            const attribute::SdrFillAttribute& rFill,
            const attribute::FillGradientAttribute* pFillGradient)
        {
            Primitive3DSequence aRetval;

            if(r3DPolyPolygonVector.size())
            {
                // create list of simple fill primitives
                aRetval.realloc(r3DPolyPolygonVector.size());

                for(sal_uInt32 a(0L); a < r3DPolyPolygonVector.size(); a++)
                {
                    // get scaled PolyPolygon
                    basegfx::B3DPolyPolygon aScaledPolyPolygon(r3DPolyPolygonVector[a]);
                    aScaledPolyPolygon.transform(rObjectTransform);

                    if(aScaledPolyPolygon.areNormalsUsed())
                    {
                        aScaledPolyPolygon.transformNormals(rObjectTransform);
                    }

                    const Primitive3DReference xRef(new PolyPolygonMaterialPrimitive3D(
                        aScaledPolyPolygon,
                        aSdr3DObjectAttribute.getMaterial(),
                        aSdr3DObjectAttribute.getDoubleSided()));
                    aRetval[a] = xRef;
                }

                // look for and evtl. build texture sub-group primitive
                if(rFill.isGradient() || rFill.isHatch() || rFill.isBitmap())
                {
                    bool bModulate(::com::sun::star::drawing::TextureMode_MODULATE == aSdr3DObjectAttribute.getTextureMode());
                    bool bFilter(aSdr3DObjectAttribute.getTextureFilter());
                    BasePrimitive3D* pNewTexturePrimitive3D = 0L;

                    if(rFill.isGradient())
                    {
                        // create gradientTexture3D with sublist, add to local aRetval
                        pNewTexturePrimitive3D = new GradientTexturePrimitive3D(*rFill.getGradient(), aRetval, rTextureSize, bModulate, bFilter);
                    }
                    else if(rFill.isHatch())
                    {
                        // create hatchTexture3D with sublist, add to local aRetval
                        pNewTexturePrimitive3D = new HatchTexturePrimitive3D(*rFill.getHatch(), aRetval, rTextureSize, bModulate, bFilter);
                    }
                    else // if(rFill.isBitmap())
                    {
                        // create bitmapTexture3D with sublist, add to local aRetval
                        basegfx::B2DRange aTexRange(0.0, 0.0, rTextureSize.getX(), rTextureSize.getY());
                        pNewTexturePrimitive3D = new BitmapTexturePrimitive3D(rFill.getBitmap()->getFillBitmapAttribute(aTexRange), aRetval, rTextureSize, bModulate, bFilter);
                    }

                    // exchange aRetval content with texture group
                    const Primitive3DReference xRef(pNewTexturePrimitive3D);
                    aRetval = Primitive3DSequence(&xRef, 1L);

                    if(::com::sun::star::drawing::TextureKind2_LUMINANCE == aSdr3DObjectAttribute.getTextureKind())
                    {
                        // use modified color primitive to force textures to gray
                        const basegfx::BColorModifier aBColorModifier(basegfx::BColor(), 0.0, basegfx::BCOLORMODIFYMODE_GRAY);
                        const Primitive3DReference xRef2(new ModifiedColorPrimitive3D(aRetval, aBColorModifier));
                        aRetval = Primitive3DSequence(&xRef2, 1L);
                    }
                }

                if(0.0 != rFill.getTransparence())
                {
                    // create UnifiedAlphaTexturePrimitive3D with sublist and exchange
                    const Primitive3DReference xRef(new UnifiedAlphaTexturePrimitive3D(rFill.getTransparence(), aRetval));
                    aRetval = Primitive3DSequence(&xRef, 1L);
                }
                else if(pFillGradient)
                {
                    // create AlphaTexturePrimitive3D with sublist and exchange
                    const Primitive3DReference xRef(new AlphaTexturePrimitive3D(*pFillGradient, aRetval, rTextureSize));
                    aRetval = Primitive3DSequence(&xRef, 1L);
                }
            }

            return aRetval;
        }

        Primitive3DSequence createShadowPrimitive3D(
            const Primitive3DSequence& rSource,
            const attribute::SdrShadowAttribute& rShadow,
            bool bShadow3D)
        {
            // create Shadow primitives. Uses already created primitives
            if(rSource.hasElements() && !basegfx::fTools::moreOrEqual(rShadow.getTransparence(), 1.0))
            {
                // prepare new list for shadow geometry
                basegfx::B2DHomMatrix aShadowOffset;
                aShadowOffset.set(0, 2, rShadow.getOffset().getX());
                aShadowOffset.set(1, 2, rShadow.getOffset().getY());

                // create shadow primitive and add primitives
                const Primitive3DReference xRef(new ShadowPrimitive3D(aShadowOffset, rShadow.getColor(), rShadow.getTransparence(), bShadow3D, rSource));
                return Primitive3DSequence(&xRef, 1L);
            }
            else
            {
                return Primitive3DSequence();
            }
        }
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
