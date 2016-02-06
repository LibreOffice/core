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

#include <drawinglayer/processor3d/zbufferprocessor3d.hxx>
#include <basegfx/raster/bpixelraster.hxx>
#include <vcl/bitmapaccess.hxx>
#include <basegfx/raster/rasterconvert3d.hxx>
#include <basegfx/raster/bzpixelraster.hxx>
#include <drawinglayer/attribute/materialattribute3d.hxx>
#include <drawinglayer/texture/texture.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>
#include <drawinglayer/primitive3d/textureprimitive3d.hxx>
#include <drawinglayer/primitive3d/polygonprimitive3d.hxx>
#include <drawinglayer/primitive3d/polypolygonprimitive3d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <basegfx/polygon/b3dpolygontools.hxx>
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#include <drawinglayer/attribute/sdrlightingattribute3d.hxx>

using namespace com::sun::star;

namespace
{
    BitmapEx BPixelRasterToBitmapEx(const basegfx::BPixelRaster& rRaster, sal_uInt16 mnAntiAlialize)
    {
        BitmapEx aRetval;
        const sal_uInt32 nWidth(mnAntiAlialize ? rRaster.getWidth()/mnAntiAlialize : rRaster.getWidth());
        const sal_uInt32 nHeight(mnAntiAlialize ? rRaster.getHeight()/mnAntiAlialize : rRaster.getHeight());

        if(nWidth && nHeight)
        {
            const Size aDestSize(nWidth, nHeight);
            sal_uInt8 nInitAlpha(255);
            Bitmap aContent(aDestSize, 24);
            AlphaMask aAlpha(aDestSize, &nInitAlpha);
            BitmapWriteAccess* pContent = aContent.AcquireWriteAccess();
            BitmapWriteAccess* pAlpha = aAlpha.AcquireWriteAccess();

            if (pContent && pAlpha)
            {
                if(mnAntiAlialize)
                {
                    const sal_uInt16 nDivisor(mnAntiAlialize * mnAntiAlialize);

                    for(sal_uInt32 y(0L); y < nHeight; y++)
                    {
                        for(sal_uInt32 x(0L); x < nWidth; x++)
                        {
                            sal_uInt16 nRed(0);
                            sal_uInt16 nGreen(0);
                            sal_uInt16 nBlue(0);
                            sal_uInt16 nOpacity(0);
                            sal_uInt32 nIndex(rRaster.getIndexFromXY(x * mnAntiAlialize, y * mnAntiAlialize));

                            for(sal_uInt32 c(0); c < mnAntiAlialize; c++)
                            {
                                for(sal_uInt32 d(0); d < mnAntiAlialize; d++)
                                {
                                    const basegfx::BPixel& rPixel(rRaster.getBPixel(nIndex++));
                                    nRed = nRed + rPixel.getRed();
                                    nGreen = nGreen + rPixel.getGreen();
                                    nBlue = nBlue + rPixel.getBlue();
                                    nOpacity = nOpacity + rPixel.getOpacity();
                                }

                                nIndex += rRaster.getWidth() - mnAntiAlialize;
                            }

                            nOpacity = nOpacity / nDivisor;

                            if(nOpacity)
                            {
                                pContent->SetPixel(y, x, BitmapColor(
                                    (sal_uInt8)(nRed / nDivisor),
                                    (sal_uInt8)(nGreen / nDivisor),
                                    (sal_uInt8)(nBlue / nDivisor)));
                                pAlpha->SetPixel(y, x, BitmapColor(255 - (sal_uInt8)nOpacity));
                            }
                        }
                    }
                }
                else
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
                }
            }

            aAlpha.ReleaseAccess(pAlpha);
            Bitmap::ReleaseAccess(pContent);

            aRetval = BitmapEx(aContent, aAlpha);

            // #i101811# set PrefMapMode and PrefSize at newly created Bitmap
            aRetval.SetPrefMapMode(MAP_PIXEL);
            aRetval.SetPrefSize(Size(nWidth, nHeight));
        }

        return aRetval;
    }
} // end of anonymous namespace

class ZBufferRasterConverter3D : public basegfx::RasterConverter3D
{
private:
    const drawinglayer::processor3d::DefaultProcessor3D&    mrProcessor;
    basegfx::BZPixelRaster&                                 mrBuffer;

    // interpolators for a single line span
    basegfx::ip_single                                      maIntZ;
    basegfx::ip_triple                                      maIntColor;
    basegfx::ip_triple                                      maIntNormal;
    basegfx::ip_double                                      maIntTexture;
    basegfx::ip_triple                                      maIntInvTexture;

    // current material to use for ratsreconversion
    const drawinglayer::attribute::MaterialAttribute3D*     mpCurrentMaterial;

    // bitfield
    // some boolean flags for line span interpolator usages
    bool                                                    mbModifyColor : 1;
    bool                                                    mbUseTex : 1;
    bool                                                    mbHasTexCoor : 1;
    bool                                                    mbHasInvTexCoor : 1;
    bool                                                    mbUseNrm : 1;
    bool                                                    mbUseCol : 1;

    void getTextureCoor(basegfx::B2DPoint& rTarget) const
    {
        if(mbHasTexCoor)
        {
            rTarget.setX(maIntTexture.getX().getVal());
            rTarget.setY(maIntTexture.getY().getVal());
        }
        else if(mbHasInvTexCoor)
        {
            const double fZFactor(maIntInvTexture.getZ().getVal());
            const double fInvZFactor(basegfx::fTools::equalZero(fZFactor) ? 1.0 : 1.0 / fZFactor);
            rTarget.setX(maIntInvTexture.getX().getVal() * fInvZFactor);
            rTarget.setY(maIntInvTexture.getY().getVal() * fInvZFactor);
        }
    }

    void incrementLineSpanInterpolators(double fStep)
    {
        maIntZ.increment(fStep);

        if(mbUseTex)
        {
            if(mbHasTexCoor)
            {
                maIntTexture.increment(fStep);
            }
            else if(mbHasInvTexCoor)
            {
                maIntInvTexture.increment(fStep);
            }
        }

        if(mbUseNrm)
        {
            maIntNormal.increment(fStep);
        }

        if(mbUseCol)
        {
            maIntColor.increment(fStep);
        }
    }

    double decideColorAndOpacity(basegfx::BColor& rColor)
    {
        // init values with full opacity and material color
        OSL_ENSURE(nullptr != mpCurrentMaterial, "CurrentMaterial not set (!)");
        double fOpacity(1.0);
        rColor = mpCurrentMaterial->getColor();

        if(mbUseTex)
        {
            basegfx::B2DPoint aTexCoor(0.0, 0.0);
            getTextureCoor(aTexCoor);

            if(mrProcessor.getGeoTexSvx().get())
            {
                // calc color in spot. This may also set to invisible already when
                // e.g. bitmap textures have transparent parts
                mrProcessor.getGeoTexSvx()->modifyBColor(aTexCoor, rColor, fOpacity);
            }

            if(basegfx::fTools::more(fOpacity, 0.0) && mrProcessor.getTransparenceGeoTexSvx().get())
            {
                // calc opacity. Object has a 2nd texture, a transparence texture
                mrProcessor.getTransparenceGeoTexSvx()->modifyOpacity(aTexCoor, fOpacity);
            }
        }

        if(basegfx::fTools::more(fOpacity, 0.0))
        {
            if(mrProcessor.getGeoTexSvx().get())
            {
                if(mbUseNrm)
                {
                    // blend texture with phong
                    rColor = mrProcessor.getSdrLightingAttribute().solveColorModel(
                        basegfx::B3DVector(maIntNormal.getX().getVal(), maIntNormal.getY().getVal(), maIntNormal.getZ().getVal()),
                        rColor,
                        mpCurrentMaterial->getSpecular(),
                        mpCurrentMaterial->getEmission(),
                        mpCurrentMaterial->getSpecularIntensity());
                }
                else if(mbUseCol)
                {
                    // blend texture with gouraud
                    basegfx::BColor aBlendColor(maIntColor.getX().getVal(), maIntColor.getY().getVal(), maIntColor.getZ().getVal());
                    rColor *= aBlendColor;
                }
                else if(mrProcessor.getModulate())
                {
                    // blend texture with single material color
                    rColor *= mpCurrentMaterial->getColor();
                }
            }
            else
            {
                if(mbUseNrm)
                {
                    // modify color with phong
                    rColor = mrProcessor.getSdrLightingAttribute().solveColorModel(
                        basegfx::B3DVector(maIntNormal.getX().getVal(), maIntNormal.getY().getVal(), maIntNormal.getZ().getVal()),
                        rColor,
                        mpCurrentMaterial->getSpecular(),
                        mpCurrentMaterial->getEmission(),
                        mpCurrentMaterial->getSpecularIntensity());
                }
                else if(mbUseCol)
                {
                    // modify color with gouraud
                    rColor.setRed(maIntColor.getX().getVal());
                    rColor.setGreen(maIntColor.getY().getVal());
                    rColor.setBlue(maIntColor.getZ().getVal());
                }
            }

            if(mbModifyColor)
            {
                rColor = mrProcessor.getBColorModifierStack().getModifiedColor(rColor);
            }
        }

        return fOpacity;
    }

    void setupLineSpanInterpolators(const basegfx::RasterConversionLineEntry3D& rA, const basegfx::RasterConversionLineEntry3D& rB)
    {
        // get inverse XDelta
        const double xInvDelta(1.0 / (rB.getX().getVal() - rA.getX().getVal()));

        // prepare Z-interpolator
        const double fZA(rA.getZ().getVal());
        const double fZB(rB.getZ().getVal());
        maIntZ = basegfx::ip_single(fZA, (fZB - fZA) * xInvDelta);

        // get bools and init other interpolators on demand accordingly
        mbModifyColor = mrProcessor.getBColorModifierStack().count();
        mbHasTexCoor = SCANLINE_EMPTY_INDEX != rA.getTextureIndex() && SCANLINE_EMPTY_INDEX != rB.getTextureIndex();
        mbHasInvTexCoor = SCANLINE_EMPTY_INDEX != rA.getInverseTextureIndex() && SCANLINE_EMPTY_INDEX != rB.getInverseTextureIndex();
        const bool bTextureActive(mrProcessor.getGeoTexSvx().get() || mrProcessor.getTransparenceGeoTexSvx().get());
        mbUseTex = bTextureActive && (mbHasTexCoor || mbHasInvTexCoor || mrProcessor.getSimpleTextureActive());
        const bool bUseColorTex(mbUseTex && mrProcessor.getGeoTexSvx().get());
        const bool bNeedNrmOrCol(!bUseColorTex || mrProcessor.getModulate());
        mbUseNrm = bNeedNrmOrCol && SCANLINE_EMPTY_INDEX != rA.getNormalIndex() && SCANLINE_EMPTY_INDEX != rB.getNormalIndex();
        mbUseCol = !mbUseNrm && bNeedNrmOrCol && SCANLINE_EMPTY_INDEX != rA.getColorIndex() && SCANLINE_EMPTY_INDEX != rB.getColorIndex();

        if(mbUseTex)
        {
            if(mbHasTexCoor)
            {
                const basegfx::ip_double& rTA(getTextureInterpolators()[rA.getTextureIndex()]);
                const basegfx::ip_double& rTB(getTextureInterpolators()[rB.getTextureIndex()]);
                maIntTexture = basegfx::ip_double(
                    rTA.getX().getVal(), (rTB.getX().getVal() - rTA.getX().getVal()) * xInvDelta,
                    rTA.getY().getVal(), (rTB.getY().getVal() - rTA.getY().getVal()) * xInvDelta);
            }
            else if(mbHasInvTexCoor)
            {
                const basegfx::ip_triple& rITA(getInverseTextureInterpolators()[rA.getInverseTextureIndex()]);
                const basegfx::ip_triple& rITB(getInverseTextureInterpolators()[rB.getInverseTextureIndex()]);
                maIntInvTexture = basegfx::ip_triple(
                    rITA.getX().getVal(), (rITB.getX().getVal() - rITA.getX().getVal()) * xInvDelta,
                    rITA.getY().getVal(), (rITB.getY().getVal() - rITA.getY().getVal()) * xInvDelta,
                    rITA.getZ().getVal(), (rITB.getZ().getVal() - rITA.getZ().getVal()) * xInvDelta);
            }
        }

        if(mbUseNrm)
        {
            const basegfx::ip_triple& rNA(getNormalInterpolators()[rA.getNormalIndex()]);
            const basegfx::ip_triple& rNB(getNormalInterpolators()[rB.getNormalIndex()]);
            maIntNormal = basegfx::ip_triple(
                rNA.getX().getVal(), (rNB.getX().getVal() - rNA.getX().getVal()) * xInvDelta,
                rNA.getY().getVal(), (rNB.getY().getVal() - rNA.getY().getVal()) * xInvDelta,
                rNA.getZ().getVal(), (rNB.getZ().getVal() - rNA.getZ().getVal()) * xInvDelta);
        }

        if(mbUseCol)
        {
            const basegfx::ip_triple& rCA(getColorInterpolators()[rA.getColorIndex()]);
            const basegfx::ip_triple& rCB(getColorInterpolators()[rB.getColorIndex()]);
            maIntColor = basegfx::ip_triple(
                rCA.getX().getVal(), (rCB.getX().getVal() - rCA.getX().getVal()) * xInvDelta,
                rCA.getY().getVal(), (rCB.getY().getVal() - rCA.getY().getVal()) * xInvDelta,
                rCA.getZ().getVal(), (rCB.getZ().getVal() - rCA.getZ().getVal()) * xInvDelta);
        }
    }

    virtual void processLineSpan(const basegfx::RasterConversionLineEntry3D& rA, const basegfx::RasterConversionLineEntry3D& rB, sal_Int32 nLine, sal_uInt32 nSpanCount) override;

public:
    ZBufferRasterConverter3D(basegfx::BZPixelRaster& rBuffer, const drawinglayer::processor3d::ZBufferProcessor3D& rProcessor)
    :   basegfx::RasterConverter3D(),
        mrProcessor(rProcessor),
        mrBuffer(rBuffer),
        maIntZ(),
        maIntColor(),
        maIntNormal(),
        maIntTexture(),
        maIntInvTexture(),
        mpCurrentMaterial(nullptr),
        mbModifyColor(false),
        mbUseTex(false),
        mbHasTexCoor(false),
        mbHasInvTexCoor(false),
        mbUseNrm(false),
        mbUseCol(false)
    {}

    void setCurrentMaterial(const drawinglayer::attribute::MaterialAttribute3D& rMaterial)
    {
        mpCurrentMaterial = &rMaterial;
    }
};

void ZBufferRasterConverter3D::processLineSpan(const basegfx::RasterConversionLineEntry3D& rA, const basegfx::RasterConversionLineEntry3D& rB, sal_Int32 nLine, sal_uInt32 nSpanCount)
{
    if(!(nSpanCount & 0x0001))
    {
        if(nLine >= 0 && nLine < (sal_Int32)mrBuffer.getHeight())
        {
            sal_uInt32 nXA(::std::min(mrBuffer.getWidth(), (sal_uInt32)::std::max((sal_Int32)0, basegfx::fround(rA.getX().getVal()))));
            const sal_uInt32 nXB(::std::min(mrBuffer.getWidth(), (sal_uInt32)::std::max((sal_Int32)0, basegfx::fround(rB.getX().getVal()))));

            if(nXA < nXB)
            {
                // prepare the span interpolators
                setupLineSpanInterpolators(rA, rB);

                // bring span interpolators to start condition by incrementing with the possible difference of
                // clamped and non-clamped XStart. Interpolators are setup relying on double precision
                // X-values, so that difference is the correct value to compensate for possible clampings
                incrementLineSpanInterpolators(static_cast<double>(nXA) - rA.getX().getVal());

                // prepare scanline index
                sal_uInt32 nScanlineIndex(mrBuffer.getIndexFromXY(nXA, static_cast<sal_uInt32>(nLine)));
                basegfx::BColor aNewColor;

                while(nXA < nXB)
                {
                    // early-test Z values if we need to do anything at all
                    const double fNewZ(::std::max(0.0, ::std::min((double)0xffff, maIntZ.getVal())));
                    const sal_uInt16 nNewZ(static_cast< sal_uInt16 >(fNewZ));
                    sal_uInt16& rOldZ(mrBuffer.getZ(nScanlineIndex));

                    if(nNewZ > rOldZ)
                    {
                        // detect color and opacity for this pixel
                        const sal_uInt16 nOpacity(::std::max((sal_Int16)0, static_cast< sal_Int16 >(decideColorAndOpacity(aNewColor) * 255.0)));

                        if(nOpacity > 0)
                        {
                            // avoid color overrun
                            aNewColor.clamp();

                            if(nOpacity >= 0x00ff)
                            {
                                // full opacity (not transparent), set z and color
                                rOldZ = nNewZ;
                                mrBuffer.getBPixel(nScanlineIndex) = basegfx::BPixel(aNewColor, 0xff);
                            }
                            else
                            {
                                basegfx::BPixel& rDest = mrBuffer.getBPixel(nScanlineIndex);

                                if(rDest.getOpacity())
                                {
                                    // mix new color by using
                                    // color' = color * (1 - opacity) + newcolor * opacity
                                    const sal_uInt16 nTransparence(0x0100 - nOpacity);
                                    rDest.setRed((sal_uInt8)(((rDest.getRed() * nTransparence) + ((sal_uInt16)(255.0 * aNewColor.getRed()) * nOpacity)) >> 8));
                                    rDest.setGreen((sal_uInt8)(((rDest.getGreen() * nTransparence) + ((sal_uInt16)(255.0 * aNewColor.getGreen()) * nOpacity)) >> 8));
                                    rDest.setBlue((sal_uInt8)(((rDest.getBlue() * nTransparence) + ((sal_uInt16)(255.0 * aNewColor.getBlue()) * nOpacity)) >> 8));

                                    if(0xff != rDest.getOpacity())
                                    {
                                        // both are transparent, mix new opacity by using
                                        // opacity = newopacity * (1 - oldopacity) + oldopacity
                                        rDest.setOpacity(((sal_uInt8)((nOpacity * (0x0100 - rDest.getOpacity())) >> 8)) + rDest.getOpacity());
                                    }
                                }
                                else
                                {
                                    // dest is unused, set color
                                    rDest = basegfx::BPixel(aNewColor, (sal_uInt8)nOpacity);
                                }
                            }
                        }
                    }

                    // increments
                    nScanlineIndex++;
                    nXA++;
                    incrementLineSpanInterpolators(1.0);
                }
            }
        }
    }
}

// helper class to buffer output for transparent rasterprimitives (filled areas
// and lines) until the end of processing. To ensure correct transparent
// visualisation, ZBuffers require to not set Z and to mix with the transparent
// color. If transparent rasterprimitives overlap, it gets necessary to
// paint transparent rasterprimitives from back to front to ensure that the
// mixing happens from back to front. For that purpose, transparent
// rasterprimitives are held in this class during the processing run, remember
// all data and will be rendered

class RasterPrimitive3D
{
private:
    std::shared_ptr< drawinglayer::texture::GeoTexSvx >     mpGeoTexSvx;
    std::shared_ptr< drawinglayer::texture::GeoTexSvx >     mpTransparenceGeoTexSvx;
    drawinglayer::attribute::MaterialAttribute3D              maMaterial;
    basegfx::B3DPolyPolygon                                   maPolyPolygon;
    double                                                    mfCenterZ;

    // bitfield
    bool                                                      mbModulate : 1;
    bool                                                      mbFilter : 1;
    bool                                                      mbSimpleTextureActive : 1;
    bool                                                      mbIsLine : 1;

public:
    RasterPrimitive3D(
        const std::shared_ptr< drawinglayer::texture::GeoTexSvx >& pGeoTexSvx,
        const std::shared_ptr< drawinglayer::texture::GeoTexSvx >& pTransparenceGeoTexSvx,
        const drawinglayer::attribute::MaterialAttribute3D& rMaterial,
        const basegfx::B3DPolyPolygon& rPolyPolygon,
        bool bModulate,
        bool bFilter,
        bool bSimpleTextureActive,
        bool bIsLine)
    :   mpGeoTexSvx(pGeoTexSvx),
        mpTransparenceGeoTexSvx(pTransparenceGeoTexSvx),
        maMaterial(rMaterial),
        maPolyPolygon(rPolyPolygon),
        mfCenterZ(basegfx::tools::getRange(rPolyPolygon).getCenter().getZ()),
        mbModulate(bModulate),
        mbFilter(bFilter),
        mbSimpleTextureActive(bSimpleTextureActive),
        mbIsLine(bIsLine)
    {
    }

    RasterPrimitive3D& operator=(const RasterPrimitive3D& rComp)
    {
        mpGeoTexSvx = rComp.mpGeoTexSvx;
        mpTransparenceGeoTexSvx = rComp.mpTransparenceGeoTexSvx;
        maMaterial = rComp.maMaterial;
        maPolyPolygon = rComp.maPolyPolygon;
        mfCenterZ = rComp.mfCenterZ;
        mbModulate = rComp.mbModulate;
        mbFilter = rComp.mbFilter;
        mbSimpleTextureActive = rComp.mbSimpleTextureActive;
        mbIsLine = rComp.mbIsLine;

        return *this;
    }

    bool operator<(const RasterPrimitive3D& rComp) const
    {
        return mfCenterZ < rComp.mfCenterZ;
    }

    const std::shared_ptr< drawinglayer::texture::GeoTexSvx >& getGeoTexSvx() const { return mpGeoTexSvx; }
    const std::shared_ptr< drawinglayer::texture::GeoTexSvx >& getTransparenceGeoTexSvx() const { return mpTransparenceGeoTexSvx; }
    const drawinglayer::attribute::MaterialAttribute3D& getMaterial() const { return maMaterial; }
    const basegfx::B3DPolyPolygon& getPolyPolygon() const { return maPolyPolygon; }
    bool getModulate() const { return mbModulate; }
    bool getFilter() const { return mbFilter; }
    bool getSimpleTextureActive() const { return mbSimpleTextureActive; }
    bool getIsLine() const { return mbIsLine; }
};

namespace drawinglayer
{
    namespace processor3d
    {
        void ZBufferProcessor3D::rasterconvertB3DPolygon(const attribute::MaterialAttribute3D& rMaterial, const basegfx::B3DPolygon& rHairline) const
        {
            if(mpBZPixelRaster)
            {
                if(getTransparenceCounter())
                {
                    // transparent output; record for later sorting and painting from
                    // back to front
                    if(!mpRasterPrimitive3Ds)
                    {
                        const_cast< ZBufferProcessor3D* >(this)->mpRasterPrimitive3Ds = new std::vector< RasterPrimitive3D >;
                    }

                    mpRasterPrimitive3Ds->push_back(RasterPrimitive3D(
                        getGeoTexSvx(),
                        getTransparenceGeoTexSvx(),
                        rMaterial,
                        basegfx::B3DPolyPolygon(rHairline),
                        getModulate(),
                        getFilter(),
                        getSimpleTextureActive(),
                        true));
                }
                else
                {
                    // do rasterconversion
                    mpZBufferRasterConverter3D->setCurrentMaterial(rMaterial);

                    if(mnAntiAlialize > 1)
                    {
                        const bool bForceLineSnap(getOptionsDrawinglayer().IsAntiAliasing() && getOptionsDrawinglayer().IsSnapHorVerLinesToDiscrete());

                        if(bForceLineSnap)
                        {
                            basegfx::B3DHomMatrix aTransform;
                            basegfx::B3DPolygon aSnappedHairline(rHairline);
                            const double fScaleDown(1.0 / mnAntiAlialize);
                            const double fScaleUp(mnAntiAlialize);

                            // take oversampling out
                            aTransform.scale(fScaleDown, fScaleDown, 1.0);
                            aSnappedHairline.transform(aTransform);

                            // snap to integer
                            aSnappedHairline = basegfx::tools::snapPointsOfHorizontalOrVerticalEdges(aSnappedHairline);

                            // add oversampling again
                            aTransform.identity();
                            aTransform.scale(fScaleUp, fScaleUp, 1.0);

                            aSnappedHairline.transform(aTransform);

                            mpZBufferRasterConverter3D->rasterconvertB3DPolygon(aSnappedHairline, 0, mpBZPixelRaster->getHeight(), mnAntiAlialize);
                        }
                        else
                        {
                            mpZBufferRasterConverter3D->rasterconvertB3DPolygon(rHairline, 0, mpBZPixelRaster->getHeight(), mnAntiAlialize);
                        }
                    }
                    else
                    {
                        mpZBufferRasterConverter3D->rasterconvertB3DPolygon(rHairline, 0, mpBZPixelRaster->getHeight(), 1);
                    }
                }
            }
        }

        void ZBufferProcessor3D::rasterconvertB3DPolyPolygon(const attribute::MaterialAttribute3D& rMaterial, const basegfx::B3DPolyPolygon& rFill) const
        {
            if(mpBZPixelRaster)
            {
                if(getTransparenceCounter())
                {
                    // transparent output; record for later sorting and painting from
                    // back to front
                    if(!mpRasterPrimitive3Ds)
                    {
                        const_cast< ZBufferProcessor3D* >(this)->mpRasterPrimitive3Ds = new std::vector< RasterPrimitive3D >;
                    }

                    mpRasterPrimitive3Ds->push_back(RasterPrimitive3D(
                        getGeoTexSvx(),
                        getTransparenceGeoTexSvx(),
                        rMaterial,
                        rFill,
                        getModulate(),
                        getFilter(),
                        getSimpleTextureActive(),
                        false));
                }
                else
                {
                    mpZBufferRasterConverter3D->setCurrentMaterial(rMaterial);
                    mpZBufferRasterConverter3D->rasterconvertB3DPolyPolygon(rFill, &maInvEyeToView, 0, mpBZPixelRaster->getHeight());
                }
            }
        }

        ZBufferProcessor3D::ZBufferProcessor3D(
            const geometry::ViewInformation3D& rViewInformation3D,
            const geometry::ViewInformation2D& rViewInformation2D,
            const attribute::SdrSceneAttribute& rSdrSceneAttribute,
            const attribute::SdrLightingAttribute& rSdrLightingAttribute,
            double fSizeX,
            double fSizeY,
            const basegfx::B2DRange& rVisiblePart,
            sal_uInt16 nAntiAlialize)
        :   DefaultProcessor3D(rViewInformation3D, rSdrSceneAttribute, rSdrLightingAttribute),
            mpBZPixelRaster(nullptr),
            maInvEyeToView(),
            mpZBufferRasterConverter3D(nullptr),
            mnAntiAlialize(nAntiAlialize),
            mpRasterPrimitive3Ds(nullptr)
        {
            // generate ViewSizes
            const double fFullViewSizeX((rViewInformation2D.getObjectToViewTransformation() * basegfx::B2DVector(fSizeX, 0.0)).getLength());
            const double fFullViewSizeY((rViewInformation2D.getObjectToViewTransformation() * basegfx::B2DVector(0.0, fSizeY)).getLength());
            const double fViewSizeX(fFullViewSizeX * rVisiblePart.getWidth());
            const double fViewSizeY(fFullViewSizeY * rVisiblePart.getHeight());

            // generate RasterWidth and RasterHeight
            const sal_uInt32 nRasterWidth((sal_uInt32)basegfx::fround(fViewSizeX) + 1);
            const sal_uInt32 nRasterHeight((sal_uInt32)basegfx::fround(fViewSizeY) + 1);

            if(nRasterWidth && nRasterHeight)
            {
                // create view unit buffer
                mpBZPixelRaster = new basegfx::BZPixelRaster(
                    mnAntiAlialize ? nRasterWidth * mnAntiAlialize : nRasterWidth,
                    mnAntiAlialize ? nRasterHeight * mnAntiAlialize : nRasterHeight);
                OSL_ENSURE(mpBZPixelRaster, "ZBufferProcessor3D: Could not allocate basegfx::BZPixelRaster (!)");

                // create DeviceToView for Z-Buffer renderer since Z is handled
                // different from standard 3D transformations (Z is mirrored). Also
                // the transformation includes the step from unit device coordinates
                // to discrete units ([-1.0 .. 1.0] -> [minDiscrete .. maxDiscrete]

                basegfx::B3DHomMatrix aDeviceToView;

                {
                    // step one:
                    //
                    // bring from [-1.0 .. 1.0] in X,Y and Z to [0.0 .. 1.0]. Also
                    // necessary to
                    // - flip Y due to screen orientation
                    // - flip Z due to Z-Buffer orientation from back to front

                    aDeviceToView.scale(0.5, -0.5, -0.5);
                    aDeviceToView.translate(0.5, 0.5, 0.5);
                }

                {
                    // step two:
                    //
                    // bring from [0.0 .. 1.0] in X,Y and Z to view coordinates
                    //
                    // #i102611#
                    // also: scale Z to [1.5 .. 65534.5]. Normally, a range of [0.0 .. 65535.0]
                    // could be used, but a 'unused' value is needed, so '0' is used what reduces
                    // the range to [1.0 .. 65535.0]. It has also shown that small numerical errors
                    // (smaller as basegfx::fTools::mfSmallValue, which is 0.000000001) happen.
                    // Instead of checking those by basegfx::fTools methods which would cost
                    // runtime, just add another 0.5 tolerance to the start and end of the Z-Buffer
                    // range, thus resulting in [1.5 .. 65534.5]
                    const double fMaxZDepth(65533.0);
                    aDeviceToView.translate(-rVisiblePart.getMinX(), -rVisiblePart.getMinY(), 0.0);

                    if(mnAntiAlialize)
                        aDeviceToView.scale(fFullViewSizeX * mnAntiAlialize, fFullViewSizeY * mnAntiAlialize, fMaxZDepth);
                    else
                        aDeviceToView.scale(fFullViewSizeX, fFullViewSizeY, fMaxZDepth);

                    aDeviceToView.translate(0.0, 0.0, 1.5);
                }

                // update local ViewInformation3D with own DeviceToView
                const geometry::ViewInformation3D aNewViewInformation3D(
                    getViewInformation3D().getObjectTransformation(),
                    getViewInformation3D().getOrientation(),
                    getViewInformation3D().getProjection(),
                    aDeviceToView,
                    getViewInformation3D().getViewTime(),
                    getViewInformation3D().getExtendedInformationSequence());
                updateViewInformation(aNewViewInformation3D);

                // prepare inverse EyeToView transformation. This can be done in constructor
                // since changes in object transformations when processing TransformPrimitive3Ds
                // do not influence this prepared partial transformation
                maInvEyeToView = getViewInformation3D().getDeviceToView() * getViewInformation3D().getProjection();
                maInvEyeToView.invert();

                // prepare maRasterRange
                maRasterRange.reset();
                maRasterRange.expand(basegfx::B2DPoint(0.0, 0.0));
                maRasterRange.expand(basegfx::B2DPoint(mpBZPixelRaster->getWidth(), mpBZPixelRaster->getHeight()));

                // create the raster converter
                mpZBufferRasterConverter3D = new ZBufferRasterConverter3D(*mpBZPixelRaster, *this);
            }
        }

        ZBufferProcessor3D::~ZBufferProcessor3D()
        {
            if(mpBZPixelRaster)
            {
                delete mpZBufferRasterConverter3D;
                delete mpBZPixelRaster;
            }

            if(mpRasterPrimitive3Ds)
            {
                OSL_FAIL("ZBufferProcessor3D: destructed, but there are unrendered transparent geometries. Use ZBufferProcessor3D::finish() to render these (!)");
                delete mpRasterPrimitive3Ds;
            }
        }

        void ZBufferProcessor3D::finish()
        {
            if(mpRasterPrimitive3Ds)
            {
                // there are transparent rasterprimitives
                const sal_uInt32 nSize(mpRasterPrimitive3Ds->size());

                if(nSize > 1)
                {
                    // sort them from back to front
                    std::sort(mpRasterPrimitive3Ds->begin(), mpRasterPrimitive3Ds->end());
                }

                for(sal_uInt32 a(0); a < nSize; a++)
                {
                    // paint each one by setting the remembered data and calling
                    // the render method
                    const RasterPrimitive3D& rCandidate = (*mpRasterPrimitive3Ds)[a];

                    mpGeoTexSvx = rCandidate.getGeoTexSvx();
                    mpTransparenceGeoTexSvx = rCandidate.getTransparenceGeoTexSvx();
                    mbModulate = rCandidate.getModulate();
                    mbFilter = rCandidate.getFilter();
                    mbSimpleTextureActive = rCandidate.getSimpleTextureActive();

                    if(rCandidate.getIsLine())
                    {
                        rasterconvertB3DPolygon(
                            rCandidate.getMaterial(),
                            rCandidate.getPolyPolygon().getB3DPolygon(0));
                    }
                    else
                    {
                        rasterconvertB3DPolyPolygon(
                            rCandidate.getMaterial(),
                            rCandidate.getPolyPolygon());
                    }
                }

                // delete them to signal the destructor that all is done and
                // to allow asserting there
                delete mpRasterPrimitive3Ds;
                mpRasterPrimitive3Ds = nullptr;
            }
        }

        BitmapEx ZBufferProcessor3D::getBitmapEx() const
        {
            if(mpBZPixelRaster)
            {
                return BPixelRasterToBitmapEx(*mpBZPixelRaster, mnAntiAlialize);
            }

            return BitmapEx();
        }
    } // end of namespace processor3d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
