/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: defaultprocessor3d.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: aw $ $Date: 2008-04-16 04:59:59 $
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

#ifndef _SV_BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif

#ifndef _BGFX_RASTER_BZPIXELRASTER_HXX
#include <basegfx/raster/bzpixelraster.hxx>
#endif

#ifndef _BGFX_RASTER_B3DPOLYPOLYGONRASTERCONVERTER_HXX
#include <basegfx/raster/b3dpolypolygonrasterconverter.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_MATERIALATTRIBUTE3D_HXX
#include <drawinglayer/attribute/materialattribute3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_TEXTURE_TEXTURE_HXX
#include <drawinglayer/texture/texture.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRATTRIBUTE3D_HXX
#include <drawinglayer/attribute/sdrattribute3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_TEXTUREPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/textureprimitive3d.hxx>
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYGONTUBEPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/polygontubeprimitive3d.hxx>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b3dpolygontools.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYPOLYGONPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/polypolygonprimitive3d.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B3DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_TRANSFORMPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/transformprimitive3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_PRIMITIVETYPES3D_HXX
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_GEOMETRY_VIEWINFORMATION2D_HXX
#include <drawinglayer/geometry/viewinformation2d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace
{
    BitmapEx BPixelRasterToBitmapEx(const basegfx::BPixelRaster& rRaster)
    {
        BitmapEx aRetval;
        const sal_uInt32 nWidth(rRaster.getWidth());
        const sal_uInt32 nHeight(rRaster.getHeight());

        if(nWidth && nHeight)
        {
            sal_uInt8 nInitAlpha(255);
            Bitmap aContent(Size(nWidth, nHeight), 24);
            AlphaMask aAlpha(Size(nWidth, nHeight), &nInitAlpha);
            BitmapWriteAccess* pContent = aContent.AcquireWriteAccess();
            BitmapWriteAccess* pAlpha = aAlpha.AcquireWriteAccess();

            if(pContent && pAlpha)
            {
                sal_uInt32 nIndex(0L);

                for(sal_uInt32 y(0L); y < nHeight; y++)
                {
                    for(sal_uInt32 x(0L); x < nWidth; x++)
                    {
                        const basegfx::BPixel& rPixel(rRaster.getBPixel(nIndex++));

                        if(rPixel.getOpacity())
                        {
                            pContent->SetPixel(y, x, BitmapColor(rPixel.getRed(), rPixel.getGreen(), rPixel.getBlue()));
                            pAlpha->SetPixel(y, x, BitmapColor(255 - rPixel.getOpacity()));
                        }
                    }
                }

                delete pContent;
                delete pAlpha;
            }

            aRetval = BitmapEx(aContent, aAlpha);
        }

        return aRetval;
    }
} // end of anonymous namespace

//////////////////////////////////////////////////////////////////////////////
// own default 3D rasterconverter based on basic 3d raster implementation
// in basegfx

namespace drawinglayer
{
    namespace
    {
        class BZPolyRaCon : public basegfx::B3DPolyPolygonRasterConverter
        {
        protected:
            basegfx::BZPixelRaster&                     mrBuffer;
            const attribute::MaterialAttribute3D&       mrMaterial;
            const processor3d::DefaultProcessor3D&      mrProcessor;

            // virtual rasterconverter
            virtual void processSpan(const basegfx::B3DScanline& rA, const basegfx::B3DScanline& rB, sal_Int32 nLine, sal_uInt32 nSpanCount);
            virtual void processLine(const basegfx::B3DScanline& rEntry, sal_Int32 nLine);

        public:
            BZPolyRaCon(
                bool bArea,
                basegfx::BZPixelRaster& rBuffer,
                const attribute::MaterialAttribute3D& rMaterial,
                const processor3d::DefaultProcessor3D& rProcessor)
            :   B3DPolyPolygonRasterConverter(bArea),
                mrBuffer(rBuffer),
                mrMaterial(rMaterial),
                mrProcessor(rProcessor)
            {}
        };

        void BZPolyRaCon::processSpan(const basegfx::B3DScanline& rA, const basegfx::B3DScanline& rB, sal_Int32 nLine, sal_uInt32 nSpanCount)
        {
            if(!(nSpanCount & 0x0001))
            {
                if(nLine >= 0L && nLine < (sal_Int32)mrBuffer.getHeight())
                {
                    sal_Int32 nXA((sal_Int32)(rA.getXInterpolator().getVal()));
                    sal_Int32 nXB((sal_Int32)(rB.getXInterpolator().getVal()));
                    OSL_ENSURE(nXB >= nXA,"processSpan: positive run expected (!)");

                    if(nXB > nXA)
                    {
                        // initialize Z interpolator
                        const sal_uInt32 nSpanLength(nXB - nXA);
                        basegfx::BDInterpolator aZ(rA.getZInterpolator().getVal(), rB.getZInterpolator().getVal(), nSpanLength);

                        // prepare some references to used variables
                        const basegfx::BColor& rColor(mrMaterial.getColor());
                        const basegfx::BColor& rSpecular(mrMaterial.getSpecular());
                        const basegfx::BColor& rEmission(mrMaterial.getEmission());
                        const sal_uInt16 nSpecularIntensity(mrMaterial.getSpecularIntensity());

                        // get bools and init other interpolators on demand accordingly
                        const bool bUseTex((mrProcessor.getGeoTexSvx() || mrProcessor.getTransparenceGeoTexSvx()) && rA.getTextureCoordinateIndex() != SCANLINE_EMPTY_INDEX && rB.getTextureCoordinateIndex() != SCANLINE_EMPTY_INDEX);
                        const bool bUseColorTex(bUseTex && mrProcessor.getGeoTexSvx());
                        const bool bNeedOthers(!bUseColorTex || (bUseColorTex && mrProcessor.getModulate()));
                        const bool bUseNrm(bNeedOthers && rA.getNormalIndex() != SCANLINE_EMPTY_INDEX && rB.getNormalIndex() != SCANLINE_EMPTY_INDEX);
                        const bool bUseCol(!bUseNrm && bNeedOthers && rA.getBColorIndex() != SCANLINE_EMPTY_INDEX && rB.getBColorIndex() != SCANLINE_EMPTY_INDEX);
                        const bool bModifyColor(mrProcessor.getBColorModifierStack().count());
                        basegfx::B2DPointInterpolator aTex;
                        basegfx::B3DVectorInterpolator aNrm;
                        basegfx::BColorInterpolator aCol;

                        if(bUseTex)
                        {
                            const basegfx::B2DPointInterpolator& rLA(maGlobalTextureCoordinateInterpolators[rA.getTextureCoordinateIndex()]);
                            const basegfx::B2DPointInterpolator& rLB(maGlobalTextureCoordinateInterpolators[rB.getTextureCoordinateIndex()]);
                            aTex = basegfx::B2DPointInterpolator(rLA.getVal(), rLB.getVal(), rLA.getZVal(), rLB.getZVal(), nSpanLength);
                        }

                        if(bUseNrm)
                        {
                            aNrm = basegfx::B3DVectorInterpolator(
                                maGlobalNormalInterpolators[rA.getNormalIndex()].getVal(),
                                maGlobalNormalInterpolators[rB.getNormalIndex()].getVal(),
                                nSpanLength);
                        }

                        if(bUseCol)
                        {
                            aCol = basegfx::BColorInterpolator(
                                maGlobalBColorInterpolators[rA.getBColorIndex()].getVal(),
                                maGlobalBColorInterpolators[rB.getBColorIndex()].getVal(),
                                nSpanLength);
                        }

                        if(nXA < 0L)
                        {
                            const double fIncrement(-nXA);
                            nXA = 0L;
                            aZ.increment(fIncrement);

                            if(bUseTex)
                            {
                                aTex.increment(fIncrement);
                            }

                            if(bUseNrm)
                            {
                                aNrm.increment(fIncrement);
                            }

                            if(bUseCol)
                            {
                                aCol.increment(fIncrement);
                            }
                        }

                        if(nXB > (sal_Int32)mrBuffer.getWidth())
                        {
                            nXB = mrBuffer.getWidth();
                        }

                        if(nXA < nXB)
                        {
                            sal_uInt32 nScanlineIndex(mrBuffer.getIndexFromXY((sal_uInt32)nXA, (sal_uInt32)nLine));

                            while(nXA < nXB)
                            {
                                // get old and new Z to see if we need to do somethng at all
                                sal_uInt16& rOldZ(mrBuffer.getZ(nScanlineIndex));
                                const sal_uInt16 nNewZ((sal_uInt16)(aZ.getVal()));

                                if(nNewZ > rOldZ)
                                {
                                    // prepare color
                                    basegfx::BColor aNewColor(rColor);
                                    double fOpacity(1.0);
                                    bool bOpacity(true);

                                    if(bUseTex)
                                    {
                                        // get texture coor
                                        const basegfx::B2DPoint aTexCoor(aTex.getVal() / aTex.getZVal());

                                        if(mrProcessor.getGeoTexSvx())
                                        {
                                            // calc color in spot
                                            mrProcessor.getGeoTexSvx()->modifyBColor(aTexCoor, aNewColor, fOpacity);
                                            bOpacity = basegfx::fTools::more(fOpacity, 0.0);
                                        }

                                        if(bOpacity && mrProcessor.getTransparenceGeoTexSvx())
                                        {
                                            // calc opacity
                                            mrProcessor.getTransparenceGeoTexSvx()->modifyOpacity(aTexCoor, fOpacity);
                                            bOpacity = basegfx::fTools::more(fOpacity, 0.0);
                                        }
                                    }

                                    if(bOpacity)
                                    {
                                        if(mrProcessor.getGeoTexSvx())
                                        {
                                            if(bUseNrm)
                                            {
                                                // blend texture with phong
                                                aNewColor = mrProcessor.getSdrLightingAttribute().solveColorModel(aNrm.getVal(), aNewColor, rSpecular, rEmission, nSpecularIntensity);
                                            }
                                            else if(bUseCol)
                                            {
                                                // blend texture with gouraud
                                                aNewColor *= aCol.getVal();
                                            }
                                            else if(mrProcessor.getModulate())
                                            {
                                                // blend texture with single material color
                                                aNewColor *= rColor;
                                            }
                                        }
                                        else
                                        {
                                            if(bUseNrm)
                                            {
                                                // modify color with phong
                                                aNewColor = mrProcessor.getSdrLightingAttribute().solveColorModel(aNrm.getVal(), rColor, rSpecular, rEmission, nSpecularIntensity);
                                            }
                                            else if(bUseCol)
                                            {
                                                // modify color with gouraud
                                                aNewColor = aCol.getVal();
                                            }
                                        }

                                        if(bModifyColor)
                                        {
                                            aNewColor = mrProcessor.getBColorModifierStack().getModifiedColor(aNewColor);
                                        }

                                        if(basegfx::fTools::moreOrEqual(fOpacity, 1.0))
                                        {
                                            // full opacity, set z and color
                                            rOldZ = nNewZ;
                                            mrBuffer.getBPixel(nScanlineIndex) = basegfx::BPixel(aNewColor, 0xff);
                                        }
                                        else
                                        {
                                            basegfx::BPixel& rDest = mrBuffer.getBPixel(nScanlineIndex);

                                            if(rDest.getOpacity())
                                            {
                                                // mix color and existing color
                                                const double fOld(1.0 - fOpacity);
                                                fOpacity *= 255.0;
                                                rDest.setRed((sal_uInt8)(((double)rDest.getRed() * fOld) + (aNewColor.getRed() * fOpacity)));
                                                rDest.setGreen((sal_uInt8)(((double)rDest.getGreen() * fOld) + (aNewColor.getGreen() * fOpacity)));
                                                rDest.setBlue((sal_uInt8)(((double)rDest.getBlue() * fOld) + (aNewColor.getBlue() * fOpacity)));

                                                if((sal_uInt8)255 != rDest.getOpacity())
                                                {
                                                    // mix opacities by adding
                                                    double fNewOpacity(rDest.getOpacity() + fOpacity);

                                                    if(fNewOpacity > 255.0)
                                                    {
                                                        // set full opacity
                                                        rDest.setOpacity(0xff);
                                                    }
                                                    else
                                                    {
                                                        // set new opacity which is still transparent, so set no z
                                                        rDest.setOpacity((sal_uInt8)(fNewOpacity));
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                // set color and opacity
                                                rDest = basegfx::BPixel(aNewColor, (sal_uInt8)(fOpacity * 255.0));
                                            }
                                        }
                                    }
                                }

                                // increments
                                {
                                    nScanlineIndex++;
                                    nXA++;
                                    aZ.increment();

                                    if(bUseTex)
                                    {
                                        aTex.increment();
                                    }

                                    if(bUseNrm)
                                    {
                                        aNrm.increment();
                                    }

                                    if(bUseCol)
                                    {
                                        aCol.increment();
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        void BZPolyRaCon::processLine(const basegfx::B3DScanline& rEntry, sal_Int32 nLine)
        {
            if(nLine >= 0L && nLine < (sal_Int32)mrBuffer.getHeight())
            {
                sal_Int32 nXA((sal_uInt32)(rEntry.getXInterpolator().getVal()));
                sal_Int32 nXB((sal_uInt32)(rEntry.getXInterpolator().getVal() + rEntry.getXInterpolator().getInc()));

                if(nXA == nXB)
                {
                    // only one position, get values and set direct
                    if(nXA >= 0L && nXA < (sal_Int32)mrBuffer.getWidth())
                    {
                        const sal_uInt32 nScanlineIndex(mrBuffer.getIndexFromXY((sal_uInt32)nXA, (sal_uInt32)nLine));
                        sal_uInt16& rOldZ(mrBuffer.getZ(nScanlineIndex));
                        const sal_uInt16 nNewZ((sal_uInt16)(rEntry.getZInterpolator().getVal()) + 0x00ff);

                        if(nNewZ > rOldZ)
                        {
                            rOldZ = nNewZ;
                            mrBuffer.getBPixel(nScanlineIndex) = basegfx::BPixel(mrMaterial.getColor(), 0xff);
                        }
                    }
                }
                else
                {
                    double fZStart(rEntry.getZInterpolator().getVal());
                    double fZStop(fZStart + rEntry.getZInterpolator().getInc());

                    if(nXB < nXA)
                    {
                        ::std::swap(nXB, nXA);
                        ::std::swap(fZStart, fZStop);
                    }

                    const basegfx::BPixel aPixel(mrMaterial.getColor(), 0xff);
                    const sal_uInt32 nSpanLength(nXB - nXA);
                    basegfx::BDInterpolator aZ(fZStart, fZStop, nSpanLength);

                    if(nXA < 0L)
                    {
                        const double fIncrement(-nXA);
                        nXA = 0L;
                        aZ.increment(fIncrement);
                    }

                    if(nXB > (sal_Int32)mrBuffer.getWidth())
                    {
                        nXB = mrBuffer.getWidth();
                    }

                    if(nXA < nXB)
                    {
                        sal_uInt32 nScanlineIndex(mrBuffer.getIndexFromXY((sal_uInt32)nXA, (sal_uInt32)nLine));

                        while(nXA < nXB)
                        {
                            sal_uInt16& rOldZ(mrBuffer.getZ(nScanlineIndex));
                            const sal_uInt16 nNewZ((sal_uInt16)(aZ.getVal()) + 0x00ff);

                            if(nNewZ > rOldZ)
                            {
                                rOldZ = nNewZ;
                                mrBuffer.getBPixel(nScanlineIndex) = aPixel;
                            }

                            nScanlineIndex++;
                            nXA++;
                            aZ.increment();
                        }
                    }
                }
            }
        }

        const ::rtl::OUString& getNamePropertyTime()
        {
            static ::rtl::OUString s_sNamePropertyTime(RTL_CONSTASCII_USTRINGPARAM("Time"));
            return s_sNamePropertyTime;
        }
    } // end of anonymous namespace
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// the DefaultProcessor3D itself

namespace drawinglayer
{
    namespace processor3d
    {
        void DefaultProcessor3D::impRender_GRX3(const primitive3d::GradientTexturePrimitive3D& rPrimitive, bool bTransparence)
        {
            const primitive3d::Primitive3DSequence& rSubSequence = rPrimitive.getChildren();

            if(rSubSequence.hasElements())
            {
                // rescue values
                const bool bOldModulate(mbModulate); mbModulate = rPrimitive.getModulate();
                const bool bOldFilter(mbFilter); mbFilter = rPrimitive.getFilter();
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
                }
                else
                {
                    // no color distance -> same color, use simple texture
                    pNewTex = new texture::GeoTexSvxMono(aStart, 1.0 - aStart.luminance());
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

        void DefaultProcessor3D::impRender_HAX3(const primitive3d::HatchTexturePrimitive3D& rPrimitive)
        {
            const primitive3d::Primitive3DSequence& rSubSequence = rPrimitive.getChildren();

            if(rSubSequence.hasElements())
            {
                // rescue values
                const bool bOldModulate(mbModulate); mbModulate = rPrimitive.getModulate();
                const bool bOldFilter(mbFilter); mbFilter = rPrimitive.getFilter();
                texture::GeoTexSvx* pOldTex = mpGeoTexSvx;

                // calculate logic pixel size in world coordinates
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

        void DefaultProcessor3D::impRender_BMX3(const primitive3d::BitmapTexturePrimitive3D& rPrimitive)
        {
            const primitive3d::Primitive3DSequence& rSubSequence = rPrimitive.getChildren();

            if(rSubSequence.hasElements())
            {
                // rescue values
                const bool bOldModulate(mbModulate); mbModulate = rPrimitive.getModulate();
                const bool bOldFilter(mbFilter); mbFilter = rPrimitive.getFilter();
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

        void DefaultProcessor3D::impRender_MCOL(const primitive3d::ModifiedColorPrimitive3D& rModifiedCandidate)
        {
            const primitive3d::Primitive3DSequence& rSubSequence = rModifiedCandidate.getChildren();

            if(rSubSequence.hasElements())
            {
                maBColorModifierStack.push(rModifiedCandidate.getColorModifier());
                process(rModifiedCandidate.getChildren());
                maBColorModifierStack.pop();
            }
        }

        void DefaultProcessor3D::impRender_POH3(const primitive3d::PolygonHairlinePrimitive3D& rPrimitive)
        {
            basegfx::B3DPolygon aHairline(rPrimitive.getB3DPolygon());

            if(aHairline.count() && mpBZPixelRaster)
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
                    BZPolyRaCon aNewRaCon(false, *mpBZPixelRaster, aMaterial, *this);
                    aNewRaCon.addPolygon(aHairline, maInvEyeToView);
                    aNewRaCon.rasterconvert(0L, mpBZPixelRaster->getHeight());
                }
            }
        }

        void DefaultProcessor3D::impRender_POM3(const primitive3d::PolyPolygonMaterialPrimitive3D& rPrimitive)
        {
            basegfx::B3DPolyPolygon aFill(rPrimitive.getB3DPolyPolygon());
            basegfx::BColor aObjectColor(rPrimitive.getMaterial().getColor());
            bool bPaintIt(aFill.count() && mpBZPixelRaster);

            if(bPaintIt)
            {
                // get rid of texture coordinates if there is no texture
                if(aFill.areTextureCoordinatesUsed() && !mpGeoTexSvx && !mpTransparenceGeoTexSvx)
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
                ::com::sun::star::drawing::ShadeMode aShadeMode(mrSdrSceneAttribute.getShadeMode());
                basegfx::B3DHomMatrix aNormalTransform(maWorldToEye);

                if(mrSdrSceneAttribute.getTwoSidedLighting())
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
                        const basegfx::BColor aColor(mbModulate ? basegfx::BColor(1.0, 1.0, 1.0) : rPrimitive.getMaterial().getColor());
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
                                const basegfx::BColor aSolvedColor(mrSdrLightingAttribute.solveColorModel(aNormal, aColor, rSpecular, rEmission, nSpecularIntensity));
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
                    const basegfx::BColor aColor(mbModulate ? basegfx::BColor(1.0, 1.0, 1.0) : rPrimitive.getMaterial().getColor());
                    const basegfx::BColor& rSpecular(rPrimitive.getMaterial().getSpecular());
                    const basegfx::BColor& rEmission(rPrimitive.getMaterial().getEmission());
                    const sal_uInt16 nSpecularIntensity(rPrimitive.getMaterial().getSpecularIntensity());

                    // solve color model for plane vector and use that color for whole plane
                    aObjectColor = mrSdrLightingAttribute.solveColorModel(aPlaneEyeNormal, aColor, rSpecular, rEmission, nSpecularIntensity);
                }

                if(::com::sun::star::drawing::ShadeMode_DRAFT == aShadeMode)
                {
                    // draft, just use object color which is already set. Delete all other infos
                    aFill.clearNormals();
                    aFill.clearBColors();
                }
            }

            if(bPaintIt)
            {
                // draw it to ZBuffer
                const attribute::MaterialAttribute3D aMaterial(
                    aObjectColor, rPrimitive.getMaterial().getSpecular(),
                    rPrimitive.getMaterial().getEmission(),
                    rPrimitive.getMaterial().getSpecularIntensity());
                BZPolyRaCon aNewRaCon(true, *mpBZPixelRaster, aMaterial, *this);

                for(sal_uInt32 a(0L); a < aFill.count(); a++)
                {
                    aNewRaCon.addPolygon(aFill.getB3DPolygon(a), maInvEyeToView);
                }

                aNewRaCon.rasterconvert(0L, mpBZPixelRaster->getHeight());
            }
        }

        void DefaultProcessor3D::impRender_TRN3(const primitive3d::TransformPrimitive3D& rTransformCandidate)
        {
            // remember current transformations
            basegfx::B3DHomMatrix aLastWorldToView(maWorldToView);
            basegfx::B3DHomMatrix aLastWorldToEye(maWorldToEye);
            basegfx::B3DHomMatrix aLastInvWorldToView(maInvWorldToView);

            // create new transformations
            maWorldToView = maWorldToView * rTransformCandidate.getTransformation();
            maWorldToEye = maWorldToEye * rTransformCandidate.getTransformation();
            maInvWorldToView = maWorldToView;
            maInvWorldToView.invert();

            // let break down
            process(rTransformCandidate.getChildren());

            // restore transformations
            maWorldToView = aLastWorldToView;
            maWorldToEye = aLastWorldToEye;
            maInvWorldToView = aLastInvWorldToView;
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
                            // it is a BasePrimitive3D implementation, use getPrimitiveID() call for switch
                            switch(pBasePrimitive->getPrimitiveID())
                            {
                                case PRIMITIVE3D_ID_GRADIENTTEXTUREPRIMITIVE3D :
                                {
                                    // GradientTexturePrimitive3D
                                    const primitive3d::GradientTexturePrimitive3D& rPrimitive = static_cast< const primitive3d::GradientTexturePrimitive3D& >(*pBasePrimitive);
                                    impRender_GRX3(rPrimitive, false);
                                    break;
                                }
                                case PRIMITIVE3D_ID_HATCHTEXTUREPRIMITIVE3D :
                                {
                                    // HatchTexturePrimitive3D
                                    static bool bDoHatchDecomposition(true);

                                    if(bDoHatchDecomposition)
                                    {
                                        // let break down
                                        process(pBasePrimitive->get3DDecomposition(getTime()));
                                    }
                                    else
                                    {
                                        // hatchTexturePrimitive3D
                                        const primitive3d::HatchTexturePrimitive3D& rPrimitive = static_cast< const primitive3d::HatchTexturePrimitive3D& >(*pBasePrimitive);
                                        impRender_HAX3(rPrimitive);
                                    }
                                    break;
                                }
                                case PRIMITIVE3D_ID_BITMAPTEXTUREPRIMITIVE3D :
                                {
                                    // BitmapTexturePrimitive3D
                                    const primitive3d::BitmapTexturePrimitive3D& rPrimitive = static_cast< const primitive3d::BitmapTexturePrimitive3D& >(*pBasePrimitive);
                                    impRender_BMX3(rPrimitive);
                                    break;
                                }
                                case PRIMITIVE3D_ID_ALPHATEXTUREPRIMITIVE3D :
                                {
                                    // AlphaTexturePrimitive3D
                                    const primitive3d::AlphaTexturePrimitive3D& rPrimitive = static_cast< const primitive3d::AlphaTexturePrimitive3D& >(*pBasePrimitive);

                                    if(mbProcessTransparent)
                                    {
                                        impRender_GRX3(rPrimitive, true);
                                    }
                                    else
                                    {
                                        mbContainsTransparent = true;
                                    }
                                    break;
                                }
                                case PRIMITIVE3D_ID_MODIFIEDCOLORPRIMITIVE3D :
                                {
                                    // ModifiedColorPrimitive3D
                                    // Force output to unified color.
                                    const primitive3d::ModifiedColorPrimitive3D& rPrimitive = static_cast< const primitive3d::ModifiedColorPrimitive3D& >(*pBasePrimitive);
                                    impRender_MCOL(rPrimitive);
                                    break;
                                }
                                case PRIMITIVE3D_ID_POLYGONHAIRLINEPRIMITIVE3D :
                                {
                                    // directdraw of PolygonHairlinePrimitive3D
                                    const primitive3d::PolygonHairlinePrimitive3D& rPrimitive = static_cast< const primitive3d::PolygonHairlinePrimitive3D& >(*pBasePrimitive);

                                    if((bool)mbProcessTransparent == (0L != mpTransparenceGeoTexSvx))
                                    {
                                        impRender_POH3(rPrimitive);
                                    }
                                    break;
                                }
                                case PRIMITIVE3D_ID_POLYPOLYGONMATERIALPRIMITIVE3D :
                                {
                                    // directdraw of PolyPolygonMaterialPrimitive3D
                                    const primitive3d::PolyPolygonMaterialPrimitive3D& rPrimitive = static_cast< const primitive3d::PolyPolygonMaterialPrimitive3D& >(*pBasePrimitive);

                                    if((bool)mbProcessTransparent == (0L != mpTransparenceGeoTexSvx))
                                    {
                                        impRender_POM3(rPrimitive);
                                    }
                                    break;
                                }
                                case PRIMITIVE3D_ID_TRANSFORMPRIMITIVE3D :
                                {
                                    // transform group (TransformPrimitive3D)
                                    impRender_TRN3(static_cast< const primitive3d::TransformPrimitive3D& >(*pBasePrimitive));
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
                                    process(pBasePrimitive->get3DDecomposition(getTime()));
                                    break;
                                }
                            }
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

        void DefaultProcessor3D::processNonTransparent(const primitive3d::Primitive3DSequence& rSource)
        {
            mbProcessTransparent = false;
            mbContainsTransparent = false;
            process(rSource);
        }

        void DefaultProcessor3D::processTransparent(const primitive3d::Primitive3DSequence& rSource)
        {
            if(mbContainsTransparent)
            {
                mbProcessTransparent = true;
                process(rSource);
            }
        }

        DefaultProcessor3D::DefaultProcessor3D(
            const geometry::ViewInformation2D& rViewInformation,
            const geometry::Transformation3D& rTransformation3D,
            const attribute::SdrSceneAttribute& rSdrSceneAttribute,
            const attribute::SdrLightingAttribute& rSdrLightingAttribute,
            double fSizeX,
            double fSizeY,
            const basegfx::B2DRange& rVisiblePart)
        :   BaseProcessor3D(rViewInformation.getViewTime()),
            mrSdrSceneAttribute(rSdrSceneAttribute),
            mrSdrLightingAttribute(rSdrLightingAttribute),
            maDeviceToView(),
            maWorldToEye(),
            maWorldToView(),
            maInvEyeToView(),
            maInvWorldToView(),
            maRasterRange(),
            mpBZPixelRaster(0),
            maBColorModifierStack(),
            mpGeoTexSvx(0),
            mpTransparenceGeoTexSvx(0),
            mbModulate(false),
            mbFilter(false),
            mbProcessTransparent(false),
            mbContainsTransparent(false)
        {
            // generate ViewSizes
            const double fFullViewSizeX((rViewInformation.getViewTransformation() * basegfx::B2DVector(fSizeX, 0.0)).getLength());
            const double fFullViewSizeY((rViewInformation.getViewTransformation() * basegfx::B2DVector(0.0, fSizeY)).getLength());
            const double fViewSizeX(fFullViewSizeX * rVisiblePart.getWidth());
            const double fViewSizeY(fFullViewSizeY * rVisiblePart.getHeight());
            const sal_uInt32 nViewSizeX((sal_uInt32)floor(fViewSizeX));
            const sal_uInt32 nViewSizeY((sal_uInt32)floor(fViewSizeY));

            if(nViewSizeX && nViewSizeY)
            {
                // create view unit buffer
                mpBZPixelRaster = new basegfx::BZPixelRaster(nViewSizeX + 1L, nViewSizeY + 1L);
                OSL_ENSURE(mpBZPixelRaster, "DefaultProcessor3D: Could not allocate basegfx::BZPixelRaster (!)");

                // create DeviceToView
                // outcome is [-1.0 .. 1.0] in X,Y and Z.

                {
                    // step one:
                    //
                    // bring from [-1.0 .. 1.0] in X,Y and Z to [0.0 .. 1.0]. Also
                    // necessary to
                    // - flip Y due to screen orientation
                    // - flip Z due to Z-Buffer orientation from back to front

                    maDeviceToView.scale(0.5, -0.5, -0.5);
                    maDeviceToView.translate(0.5, 0.5, 0.5);
                }

                {
                    // step two:
                    //
                    // bring from [0.0 .. 1.0] in X,Y and Z to view cordinates. also:
                    // - scale Z to [0.0 .. fMaxZDepth]
                    const double fMaxZDepth(double(0x0000ff00));
                    maDeviceToView.translate(-rVisiblePart.getMinX(), -rVisiblePart.getMinY(), 0.0);
                    maDeviceToView.scale(fFullViewSizeX, fFullViewSizeY, fMaxZDepth);
                }

                // create world to eye transformation
                maWorldToEye = rTransformation3D.getOrientation() * rTransformation3D.getTransformation();

                // create EyeToView transformation
                maWorldToView = maDeviceToView * rTransformation3D.getProjection() * maWorldToEye;

                // create inverse EyeToView transformation
                maInvEyeToView = maDeviceToView * rTransformation3D.getProjection();
                maInvEyeToView.invert();

                // create inverse WorldToView transformation
                maInvWorldToView = maWorldToView;
                maInvWorldToView.invert();

                // prepare maRasterRange
                maRasterRange.expand(basegfx::B2DPoint(0.0, 0.0));
                maRasterRange.expand(basegfx::B2DPoint(mpBZPixelRaster->getWidth(), mpBZPixelRaster->getHeight()));
            }
        }

        DefaultProcessor3D::~DefaultProcessor3D()
        {
            if(mpBZPixelRaster)
            {
                delete mpBZPixelRaster;
            }
        }

        BitmapEx DefaultProcessor3D::getBitmapEx() const
        {
            if(mpBZPixelRaster)
            {
                return BPixelRasterToBitmapEx(*mpBZPixelRaster);
            }

            return BitmapEx();
        }
    } // end of namespace processor3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
