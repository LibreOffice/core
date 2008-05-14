/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: defaultprocessor3d.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-14 09:21:54 $
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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR3D_DEFAULTPROCESSOR3D_HXX
#include <drawinglayer/processor3d/defaultprocessor3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_TEXTUREPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/textureprimitive3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_TEXTURE_TEXTURE_HXX
#include <drawinglayer/texture/texture.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_TEXTURE_TEXTURE3D_HXX
#include <drawinglayer/texture/texture3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_HATCHTEXTUREPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/hatchtextureprimitive3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_MODIFIEDCOLORPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/modifiedcolorprimitive3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYGONPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/polygonprimitive3d.hxx>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b3dpolygontools.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_MATERIALATTRIBUTE3D_HXX
#include <drawinglayer/attribute/materialattribute3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYPOLYGONPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/polypolygonprimitive3d.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B3DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_SHADEMODE_HPP_
#include <com/sun/star/drawing/ShadeMode.hpp>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRATTRIBUTE3D_HXX
#include <drawinglayer/attribute/sdrattribute3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_TRANSFORMPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/transformprimitive3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_PRIMITIVETYPES3D_HXX
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor3d
    {
        void DefaultProcessor3D::impRenderGradientTexturePrimitive3D(const primitive3d::GradientTexturePrimitive3D& rPrimitive, bool bTransparence)
        {
            const primitive3d::Primitive3DSequence& rSubSequence = rPrimitive.getChildren();

            if(rSubSequence.hasElements())
            {
                // rescue values
                const bool bOldModulate(getModulate()); mbModulate = rPrimitive.getModulate();
                const bool bOldFilter(getFilter()); mbFilter = rPrimitive.getFilter();
                const bool bOldSimpleTextureActive(mbSimpleTextureActive);
                texture::GeoTexSvx* pOldTex = (bTransparence) ? mpTransparenceGeoTexSvx : mpGeoTexSvx;

                // create texture
                const attribute::FillGradientAttribute& rFillGradient = rPrimitive.getGradient();
                const basegfx::B2DRange aOutlineRange(0.0, 0.0, rPrimitive.getTextureSize().getX(), rPrimitive.getTextureSize().getY());
                const attribute::GradientStyle aGradientStyle(rFillGradient.getStyle());
                sal_uInt32 nSteps(rFillGradient.getSteps());
                const basegfx::BColor aStart(rFillGradient.getStartColor());
                const basegfx::BColor aEnd(rFillGradient.getEndColor());
                const sal_uInt32 nMaxSteps(sal_uInt32((aStart.getMaximumDistance(aEnd) * 127.5) + 0.5));
                texture::GeoTexSvx* pNewTex = 0L;

                if(nMaxSteps)
                {
                    // there IS a color distance
                    if(nSteps == 0L)
                    {
                        nSteps = nMaxSteps;
                    }

                    if(nSteps < 2L)
                    {
                        nSteps = 2L;
                    }

                    if(nSteps > nMaxSteps)
                    {
                        nSteps = nMaxSteps;
                    }

                    switch(aGradientStyle)
                    {
                        case attribute::GRADIENTSTYLE_LINEAR:
                        {
                            pNewTex = new texture::GeoTexSvxGradientLinear(aOutlineRange, aStart, aEnd, nSteps, rFillGradient.getBorder(), -rFillGradient.getAngle());
                            break;
                        }
                        case attribute::GRADIENTSTYLE_AXIAL:
                        {
                            pNewTex = new texture::GeoTexSvxGradientAxial(aOutlineRange, aStart, aEnd, nSteps, rFillGradient.getBorder(), -rFillGradient.getAngle());
                            break;
                        }
                        case attribute::GRADIENTSTYLE_RADIAL:
                        {
                            pNewTex = new texture::GeoTexSvxGradientRadial(aOutlineRange, aStart, aEnd, nSteps, rFillGradient.getBorder(), rFillGradient.getOffsetX(), rFillGradient.getOffsetY());
                            break;
                        }
                        case attribute::GRADIENTSTYLE_ELLIPTICAL:
                        {
                            pNewTex = new texture::GeoTexSvxGradientElliptical(aOutlineRange, aStart, aEnd, nSteps, rFillGradient.getBorder(), rFillGradient.getOffsetX(), rFillGradient.getOffsetY(), -rFillGradient.getAngle());
                            break;
                        }
                        case attribute::GRADIENTSTYLE_SQUARE:
                        {
                            pNewTex = new texture::GeoTexSvxGradientSquare(aOutlineRange, aStart, aEnd, nSteps, rFillGradient.getBorder(), rFillGradient.getOffsetX(), rFillGradient.getOffsetY(), -rFillGradient.getAngle());
                            break;
                        }
                        case attribute::GRADIENTSTYLE_RECT:
                        {
                            pNewTex = new texture::GeoTexSvxGradientRect(aOutlineRange, aStart, aEnd, nSteps, rFillGradient.getBorder(), rFillGradient.getOffsetX(), rFillGradient.getOffsetY(), -rFillGradient.getAngle());
                            break;
                        }
                    }

                    mbSimpleTextureActive = false;
                }
                else
                {
                    // no color distance -> same color, use simple texture
                    pNewTex = new texture::GeoTexSvxMono(aStart, 1.0 - aStart.luminance());
                    mbSimpleTextureActive = true;
                }

                // set created texture
                if(bTransparence)
                {
                    mpTransparenceGeoTexSvx = pNewTex;
                }
                else
                {
                    mpGeoTexSvx = pNewTex;
                }

                // process sub-list
                process(rSubSequence);

                // delete texture
                delete pNewTex;

                // restore values
                mbModulate = bOldModulate;
                mbFilter = bOldFilter;
                mbSimpleTextureActive = bOldSimpleTextureActive;

                if(bTransparence)
                {
                    mpTransparenceGeoTexSvx = pOldTex;
                }
                else
                {
                    mpGeoTexSvx = pOldTex;
                }
            }
        }

        void DefaultProcessor3D::impRenderHatchTexturePrimitive3D(const primitive3d::HatchTexturePrimitive3D& rPrimitive)
        {
            const primitive3d::Primitive3DSequence& rSubSequence = rPrimitive.getChildren();

            if(rSubSequence.hasElements())
            {
                // rescue values
                const bool bOldModulate(getModulate()); mbModulate = rPrimitive.getModulate();
                const bool bOldFilter(getFilter()); mbFilter = rPrimitive.getFilter();
                texture::GeoTexSvx* pOldTex = mpGeoTexSvx;

                // calculate logic pixel size in world coordinates. Check if InvWorldToView
                // is valid
                if(maInvWorldToView.isIdentity())
                {
                    maInvWorldToView = maWorldToView;
                    maInvWorldToView.invert();
                }

                const basegfx::B3DPoint aZero(maInvWorldToView * basegfx::B3DPoint(0.0, 0.0, 0.0));
                const basegfx::B3DPoint aOne(maInvWorldToView * basegfx::B3DPoint(1.0, 1.0, 1.0));
                const basegfx::B3DVector aLogicPixelSizeWorld(aOne - aZero);
                double fLogicPixelSizeWorld(fabs(aLogicPixelSizeWorld.getX()));

                if(fabs(aLogicPixelSizeWorld.getY()) > fLogicPixelSizeWorld)
                {
                    fLogicPixelSizeWorld = fabs(aLogicPixelSizeWorld.getY());
                }

                if(fabs(aLogicPixelSizeWorld.getZ()) > fLogicPixelSizeWorld)
                {
                    fLogicPixelSizeWorld = fabs(aLogicPixelSizeWorld.getZ());
                }

                // calculate logic pixel size in texture coordinates
                const double fLogicTexSizeX(fLogicPixelSizeWorld / rPrimitive.getTextureSize().getX());
                const double fLogicTexSizeY(fLogicPixelSizeWorld / rPrimitive.getTextureSize().getY());
                const double fLogicTexSize(fLogicTexSizeX > fLogicTexSizeY ? fLogicTexSizeX : fLogicTexSizeY);

                // create texture and set
                texture::GeoTexSvxMultiHatch* pNewTex = new texture::GeoTexSvxMultiHatch(rPrimitive, fLogicTexSize);
                mpGeoTexSvx = pNewTex;

                // process sub-list
                process(rSubSequence);

                // delete texture
                delete mpGeoTexSvx;

                // restore values
                mbModulate = bOldModulate;
                mbFilter = bOldFilter;
                mpGeoTexSvx = pOldTex;
            }
        }

        void DefaultProcessor3D::impRenderBitmapTexturePrimitive3D(const primitive3d::BitmapTexturePrimitive3D& rPrimitive)
        {
            const primitive3d::Primitive3DSequence& rSubSequence = rPrimitive.getChildren();

            if(rSubSequence.hasElements())
            {
                // rescue values
                const bool bOldModulate(getModulate()); mbModulate = rPrimitive.getModulate();
                const bool bOldFilter(getFilter()); mbFilter = rPrimitive.getFilter();
                texture::GeoTexSvx* pOldTex = mpGeoTexSvx;

                // create texture
                const attribute::FillBitmapAttribute& rFillBitmapAttribute = rPrimitive.getBitmap();

                if(rFillBitmapAttribute.getTiling())
                {
                    mpGeoTexSvx = new texture::GeoTexSvxBitmapTiled(
                        rFillBitmapAttribute.getBitmap(),
                        rFillBitmapAttribute.getTopLeft() * rPrimitive.getTextureSize(),
                        rFillBitmapAttribute.getSize() * rPrimitive.getTextureSize());
                }
                else
                {
                    mpGeoTexSvx = new texture::GeoTexSvxBitmap(
                        rFillBitmapAttribute.getBitmap(),
                        rFillBitmapAttribute.getTopLeft() * rPrimitive.getTextureSize(),
                        rFillBitmapAttribute.getSize() * rPrimitive.getTextureSize());
                }

                // process sub-list
                process(rSubSequence);

                // delete texture
                delete mpGeoTexSvx;

                // restore values
                mbModulate = bOldModulate;
                mbFilter = bOldFilter;
                mpGeoTexSvx = pOldTex;
            }
        }

        void DefaultProcessor3D::impRenderModifiedColorPrimitive3D(const primitive3d::ModifiedColorPrimitive3D& rModifiedCandidate)
        {
            const primitive3d::Primitive3DSequence& rSubSequence = rModifiedCandidate.getChildren();

            if(rSubSequence.hasElements())
            {
                // put modifier on stack
                maBColorModifierStack.push(rModifiedCandidate.getColorModifier());

                // process sub-list
                process(rModifiedCandidate.getChildren());

                // remove modifier from stack
                maBColorModifierStack.pop();
            }
        }

        void DefaultProcessor3D::impRenderPolygonHairlinePrimitive3D(const primitive3d::PolygonHairlinePrimitive3D& rPrimitive)
        {
            basegfx::B3DPolygon aHairline(rPrimitive.getB3DPolygon());

            if(aHairline.count())
            {
                // hairlines need no extra data, clear it
                aHairline.clearTextureCoordinates();
                aHairline.clearNormals();
                aHairline.clearBColors();

                // transform to device coordinates (-1.0 .. 1.0) and check for visibility
                aHairline.transform(maWorldToView);
                const basegfx::B3DRange a3DRange(basegfx::tools::getRange(aHairline));
                const basegfx::B2DRange a2DRange(a3DRange.getMinX(), a3DRange.getMinY(), a3DRange.getMaxX(), a3DRange.getMaxY());

                if(a2DRange.overlaps(maRasterRange))
                {
                    const attribute::MaterialAttribute3D aMaterial(rPrimitive.getBColor());

                    rasterconvertB3DPolygon(aMaterial, aHairline);
                }
            }
        }

        void DefaultProcessor3D::impRenderPolyPolygonMaterialPrimitive3D(const primitive3d::PolyPolygonMaterialPrimitive3D& rPrimitive)
        {
            basegfx::B3DPolyPolygon aFill(rPrimitive.getB3DPolyPolygon());
            basegfx::BColor aObjectColor(rPrimitive.getMaterial().getColor());
            bool bPaintIt(aFill.count());

            if(bPaintIt)
            {
                // get rid of texture coordinates if there is no texture
                if(aFill.areTextureCoordinatesUsed() && !getGeoTexSvx() && !getTransparenceGeoTexSvx())
                {
                    aFill.clearTextureCoordinates();
                }

                // transform to device coordinates (-1.0 .. 1.0) and check for visibility
                aFill.transform(maWorldToView);
                const basegfx::B3DRange a3DRange(basegfx::tools::getRange(aFill));
                const basegfx::B2DRange a2DRange(a3DRange.getMinX(), a3DRange.getMinY(), a3DRange.getMaxX(), a3DRange.getMaxY());

                bPaintIt = a2DRange.overlaps(maRasterRange);
            }

            // check if it shall be painted regarding hiding of normals (backface culling)
            if(bPaintIt && !rPrimitive.getDoubleSided())
            {
                // get plane normal of polygon in view coordinates (with ZBuffer values),
                // left-handed coordinate system
                const basegfx::B3DVector aPlaneNormal(aFill.getB3DPolygon(0L).getNormal());

                if(aPlaneNormal.getZ() > 0.0)
                {
                    bPaintIt = false;
                }
            }

            if(bPaintIt)
            {
                ::com::sun::star::drawing::ShadeMode aShadeMode(getSdrSceneAttribute().getShadeMode());
                basegfx::B3DHomMatrix aNormalTransform(maWorldToEye);

                if(getSdrSceneAttribute().getTwoSidedLighting())
                {
                    // get plane normal of polygon in view coordinates (with ZBuffer values),
                    // left-handed coordinate system
                    const basegfx::B3DVector aPlaneNormal(aFill.getB3DPolygon(0L).getNormal());

                    if(aPlaneNormal.getZ() > 0.0)
                    {
                        // mirror normals
                        aNormalTransform.scale(-1.0, -1.0, -1.0);
                    }
                }

                if(::com::sun::star::drawing::ShadeMode_PHONG == aShadeMode)
                {
                    // phong shading
                    if(aFill.areNormalsUsed())
                    {
                        // transform normals to eye coor
                        aFill.transformNormals(aNormalTransform);
                    }
                    else
                    {
                        // fallback to gouraud when no normals available
                        aShadeMode = ::com::sun::star::drawing::ShadeMode_SMOOTH;
                    }
                }

                if(::com::sun::star::drawing::ShadeMode_SMOOTH == aShadeMode)
                {
                    // gouraud shading
                    if(aFill.areNormalsUsed())
                    {
                        // transform normals to eye coor
                        aFill.transformNormals(aNormalTransform);

                        // prepare color model parameters, evtl. use blend color
                        const basegfx::BColor aColor(getModulate() ? basegfx::BColor(1.0, 1.0, 1.0) : rPrimitive.getMaterial().getColor());
                        const basegfx::BColor& rSpecular(rPrimitive.getMaterial().getSpecular());
                        const basegfx::BColor& rEmission(rPrimitive.getMaterial().getEmission());
                        const sal_uInt16 nSpecularIntensity(rPrimitive.getMaterial().getSpecularIntensity());

                        // solve color model for each normal vector, set colors at points. Clear normals.
                        for(sal_uInt32 a(0L); a < aFill.count(); a++)
                        {
                            basegfx::B3DPolygon aPartFill(aFill.getB3DPolygon(a));

                            for(sal_uInt32 b(0L); b < aPartFill.count(); b++)
                            {
                                // solve color model. Transform normal to eye coor
                                const basegfx::B3DVector aNormal(aPartFill.getNormal(b));
                                const basegfx::BColor aSolvedColor(getSdrLightingAttribute().solveColorModel(aNormal, aColor, rSpecular, rEmission, nSpecularIntensity));
                                aPartFill.setBColor(b, aSolvedColor);
                            }

                            // clear normals on this part polygon and write it back
                            aPartFill.clearNormals();
                            aFill.setB3DPolygon(a, aPartFill);
                        }
                    }
                    else
                    {
                        // fallback to flat when no normals available
                        aShadeMode = ::com::sun::star::drawing::ShadeMode_FLAT;
                    }
                }

                if(::com::sun::star::drawing::ShadeMode_FLAT == aShadeMode)
                {
                    // flat shading. Clear normals and colors
                    aFill.clearNormals();
                    aFill.clearBColors();

                    // get plane vector in eye coordinates
                    const basegfx::B3DVector aPlaneEyeNormal(aNormalTransform * rPrimitive.getB3DPolyPolygon().getB3DPolygon(0L).getNormal());

                    // prepare color model parameters, evtl. use blend color
                    const basegfx::BColor aColor(getModulate() ? basegfx::BColor(1.0, 1.0, 1.0) : rPrimitive.getMaterial().getColor());
                    const basegfx::BColor& rSpecular(rPrimitive.getMaterial().getSpecular());
                    const basegfx::BColor& rEmission(rPrimitive.getMaterial().getEmission());
                    const sal_uInt16 nSpecularIntensity(rPrimitive.getMaterial().getSpecularIntensity());

                    // solve color model for plane vector and use that color for whole plane
                    aObjectColor = getSdrLightingAttribute().solveColorModel(aPlaneEyeNormal, aColor, rSpecular, rEmission, nSpecularIntensity);
                }

                if(::com::sun::star::drawing::ShadeMode_DRAFT == aShadeMode)
                {
                    // draft, just use object color which is already set. Delete all other infos
                    aFill.clearNormals();
                    aFill.clearBColors();
                }

                // draw it to ZBuffer
                const attribute::MaterialAttribute3D aMaterial(
                    aObjectColor, rPrimitive.getMaterial().getSpecular(),
                    rPrimitive.getMaterial().getEmission(),
                    rPrimitive.getMaterial().getSpecularIntensity());

                rasterconvertB3DPolyPolygon(aMaterial, aFill);
            }
        }

        void DefaultProcessor3D::impRenderTransformPrimitive3D(const primitive3d::TransformPrimitive3D& rTransformCandidate)
        {
            // remember current transformations
            const basegfx::B3DHomMatrix aLastWorldToView(maWorldToView);
            const basegfx::B3DHomMatrix aLastWorldToEye(maWorldToEye);
            const basegfx::B3DHomMatrix aLastInvWorldToView(maInvWorldToView);

            // create new transformations
            maWorldToView = maWorldToView * rTransformCandidate.getTransformation();
            maWorldToEye = maWorldToEye * rTransformCandidate.getTransformation();
            maInvWorldToView.identity();

            // let break down
            process(rTransformCandidate.getChildren());

            // restore transformations
            maWorldToView = aLastWorldToView;
            maWorldToEye = aLastWorldToEye;
            maInvWorldToView = aLastInvWorldToView;
        }

        void DefaultProcessor3D::processBasePrimitive3D(const primitive3d::BasePrimitive3D& rBasePrimitive)
        {
            // it is a BasePrimitive3D implementation, use getPrimitiveID() call for switch
            switch(rBasePrimitive.getPrimitiveID())
            {
                case PRIMITIVE3D_ID_GRADIENTTEXTUREPRIMITIVE3D :
                {
                    // GradientTexturePrimitive3D
                    const primitive3d::GradientTexturePrimitive3D& rPrimitive = static_cast< const primitive3d::GradientTexturePrimitive3D& >(rBasePrimitive);
                    impRenderGradientTexturePrimitive3D(rPrimitive, false);
                    break;
                }
                case PRIMITIVE3D_ID_HATCHTEXTUREPRIMITIVE3D :
                {
                    // HatchTexturePrimitive3D
                    static bool bDoHatchDecomposition(false);

                    if(bDoHatchDecomposition)
                    {
                        // let break down
                        process(rBasePrimitive.get3DDecomposition(getTime()));
                    }
                    else
                    {
                        // hatchTexturePrimitive3D
                        const primitive3d::HatchTexturePrimitive3D& rPrimitive = static_cast< const primitive3d::HatchTexturePrimitive3D& >(rBasePrimitive);
                        impRenderHatchTexturePrimitive3D(rPrimitive);
                    }
                    break;
                }
                case PRIMITIVE3D_ID_BITMAPTEXTUREPRIMITIVE3D :
                {
                    // BitmapTexturePrimitive3D
                    const primitive3d::BitmapTexturePrimitive3D& rPrimitive = static_cast< const primitive3d::BitmapTexturePrimitive3D& >(rBasePrimitive);
                    impRenderBitmapTexturePrimitive3D(rPrimitive);
                    break;
                }
                case PRIMITIVE3D_ID_ALPHATEXTUREPRIMITIVE3D :
                {
                    // AlphaTexturePrimitive3D
                    const primitive3d::AlphaTexturePrimitive3D& rPrimitive = static_cast< const primitive3d::AlphaTexturePrimitive3D& >(rBasePrimitive);
                    impRenderGradientTexturePrimitive3D(rPrimitive, true);
                    break;
                }
                case PRIMITIVE3D_ID_MODIFIEDCOLORPRIMITIVE3D :
                {
                    // ModifiedColorPrimitive3D
                    // Force output to unified color.
                    const primitive3d::ModifiedColorPrimitive3D& rPrimitive = static_cast< const primitive3d::ModifiedColorPrimitive3D& >(rBasePrimitive);
                    impRenderModifiedColorPrimitive3D(rPrimitive);
                    break;
                }
                case PRIMITIVE3D_ID_POLYGONHAIRLINEPRIMITIVE3D :
                {
                    // directdraw of PolygonHairlinePrimitive3D
                    const primitive3d::PolygonHairlinePrimitive3D& rPrimitive = static_cast< const primitive3d::PolygonHairlinePrimitive3D& >(rBasePrimitive);
                    impRenderPolygonHairlinePrimitive3D(rPrimitive);
                    break;
                }
                case PRIMITIVE3D_ID_POLYPOLYGONMATERIALPRIMITIVE3D :
                {
                    // directdraw of PolyPolygonMaterialPrimitive3D
                    const primitive3d::PolyPolygonMaterialPrimitive3D& rPrimitive = static_cast< const primitive3d::PolyPolygonMaterialPrimitive3D& >(rBasePrimitive);
                    impRenderPolyPolygonMaterialPrimitive3D(rPrimitive);
                    break;
                }
                case PRIMITIVE3D_ID_TRANSFORMPRIMITIVE3D :
                {
                    // transform group (TransformPrimitive3D)
                    impRenderTransformPrimitive3D(static_cast< const primitive3d::TransformPrimitive3D& >(rBasePrimitive));
                    break;
                }
                case PRIMITIVE3D_ID_SDRLABELPRIMITIVE3D :
                {
                    // SdrLabelPrimitive3D. Accept, but ignore. Is handled by the scenePrimitive decompose
                    // method which creates 2d text objects at the 3d-projection-dependent positions.
                    break;
                }
                default:
                {
                    // process recursively
                    process(rBasePrimitive.get3DDecomposition(getTime()));
                    break;
                }
            }
        }

        void DefaultProcessor3D::process(const primitive3d::Primitive3DSequence& rSource)
        {
            if(rSource.hasElements())
            {
                const sal_Int32 nCount(rSource.getLength());

                for(sal_Int32 a(0L); a < nCount; a++)
                {
                    // get reference
                    const primitive3d::Primitive3DReference xReference(rSource[a]);

                    if(xReference.is())
                    {
                        // try to cast to BasePrimitive3D implementation
                        const primitive3d::BasePrimitive3D* pBasePrimitive = dynamic_cast< const primitive3d::BasePrimitive3D* >(xReference.get());

                        if(pBasePrimitive)
                        {
                            processBasePrimitive3D(*pBasePrimitive);
                        }
                        else
                        {
                            // unknown implementation, use UNO API call instead and process recursively
                            const uno::Sequence< beans::PropertyValue > xViewParameters(primitive3d::TimeToViewParameters(getTime()));
                            process(xReference->getDecomposition(xViewParameters));
                        }
                    }
                }
            }
        }

        DefaultProcessor3D::DefaultProcessor3D(
            double fTime,
            const attribute::SdrSceneAttribute& rSdrSceneAttribute,
            const attribute::SdrLightingAttribute& rSdrLightingAttribute)
        :   BaseProcessor3D(fTime),
            mrSdrSceneAttribute(rSdrSceneAttribute),
            mrSdrLightingAttribute(rSdrLightingAttribute),
            maWorldToEye(),
            maWorldToView(),
            maRasterRange(),
            maBColorModifierStack(),
            mpGeoTexSvx(0),
            mpTransparenceGeoTexSvx(0),
            mnRasterWidth(0),
            mnRasterHeight(0),
            mbModulate(false),
            mbFilter(false),
            mbSimpleTextureActive(false)
        {
            // a derivation has to set maWorldToEye, maWorldToView
            // maRasterRange, mnRasterWidth and mnRasterHeight. Those values are
            // used in the basic render methods
        }

        DefaultProcessor3D::~DefaultProcessor3D()
        {
        }
    } // end of namespace processor3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
