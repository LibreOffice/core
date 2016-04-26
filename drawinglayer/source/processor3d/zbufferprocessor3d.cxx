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
#include <comphelper/threadpool.hxx>
#include <list>

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

class LineSpanJob
{
private:
    friend class ZBufferRasterConverter3D;

    // geometric base data for LineSpan
    sal_uInt32          mnCount;
    basegfx::BPixel*    mpBPixel;
    sal_uInt16*         mpZ;
    drawinglayer::attribute::MaterialAttribute3D    maCurrentMaterial;
    std::shared_ptr< drawinglayer::texture::GeoTexSvx > maGeoTexSvx;
    std::shared_ptr< drawinglayer::texture::GeoTexSvx > maTransparenceGeoTexSvx;
    drawinglayer::attribute::SdrLightingAttribute maSdrLightingAttribute;
    basegfx::BColorModifierStack maBColorModifierStack;

    // interpolators for a single line span
    basegfx::ip_single                                      maIntZ;
    basegfx::ip_triple                                      maIntColor;
    basegfx::ip_triple                                      maIntNormal;
    basegfx::ip_double                                      maIntTexture;
    basegfx::ip_triple                                      maIntInvTexture;

    // bitfield
    // some boolean flags for line span interpolator usages
    bool                                                    mbModifyColor : 1;
    bool                                                    mbUseTex : 1;
    bool                                                    mbHasTexCoor : 1;
    bool                                                    mbHasInvTexCoor : 1;
    bool                                                    mbUseNrm : 1;
    bool                                                    mbUseCol : 1;
    bool                                                    mbModulate : 1;

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

    double decideColorAndOpacity(basegfx::BColor& rColor)
    {
        // init values with full opacity and material color
        double fOpacity(1.0);
        rColor = maCurrentMaterial.getColor();

        if(mbUseTex)
        {
            basegfx::B2DPoint aTexCoor(0.0, 0.0);
            getTextureCoor(aTexCoor);

            if(maGeoTexSvx.get())
            {
                // calc color in spot. This may also set to invisible already when
                // e.g. bitmap textures have transparent parts
                maGeoTexSvx->modifyBColor(aTexCoor, rColor, fOpacity);
            }

            if(basegfx::fTools::more(fOpacity, 0.0) && maTransparenceGeoTexSvx.get())
            {
                // calc opacity. Object has a 2nd texture, a transparence texture
                maTransparenceGeoTexSvx->modifyOpacity(aTexCoor, fOpacity);
            }
        }

        if(basegfx::fTools::more(fOpacity, 0.0))
        {
            if(maGeoTexSvx.get())
            {
                if(mbUseNrm)
                {
                    // blend texture with phong
                    rColor = maSdrLightingAttribute.solveColorModel(
                        basegfx::B3DVector(maIntNormal.getX().getVal(), maIntNormal.getY().getVal(), maIntNormal.getZ().getVal()),
                        rColor,
                        maCurrentMaterial.getSpecular(),
                        maCurrentMaterial.getEmission(),
                        maCurrentMaterial.getSpecularIntensity());
                }
                else if(mbUseCol)
                {
                    // blend texture with gouraud
                    basegfx::BColor aBlendColor(maIntColor.getX().getVal(), maIntColor.getY().getVal(), maIntColor.getZ().getVal());
                    rColor *= aBlendColor;
                }
                else if(mbModulate)
                {
                    // blend texture with single material color
                    rColor *= maCurrentMaterial.getColor();
                }
            }
            else
            {
                if(mbUseNrm)
                {
                    // modify color with phong
                    rColor = maSdrLightingAttribute.solveColorModel(
                        basegfx::B3DVector(maIntNormal.getX().getVal(), maIntNormal.getY().getVal(), maIntNormal.getZ().getVal()),
                        rColor,
                        maCurrentMaterial.getSpecular(),
                        maCurrentMaterial.getEmission(),
                        maCurrentMaterial.getSpecularIntensity());
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
                rColor = maBColorModifierStack.getModifiedColor(rColor);
            }
        }

        return fOpacity;
    }

public:
    LineSpanJob(
        sal_uInt32 nCount,
        basegfx::BPixel& rBPixel,
        sal_uInt16& rZ,
        const drawinglayer::attribute::MaterialAttribute3D& rCurrentMaterial,
        const std::shared_ptr< drawinglayer::texture::GeoTexSvx >& rGeoTexSvx,
        const std::shared_ptr< drawinglayer::texture::GeoTexSvx >& rTransparenceGeoTexSvx,
        const drawinglayer::attribute::SdrLightingAttribute& rSdrLightingAttribute,
        const basegfx::BColorModifierStack& rBColorModifierStack,
        bool bModulate)
    :   mnCount(nCount),
        mpBPixel(&rBPixel),
        mpZ(&rZ),
        maCurrentMaterial(rCurrentMaterial),
        maGeoTexSvx(rGeoTexSvx),
        maTransparenceGeoTexSvx(rTransparenceGeoTexSvx),
        maSdrLightingAttribute(rSdrLightingAttribute),
        maBColorModifierStack(rBColorModifierStack),
        maIntZ(),
        maIntColor(),
        maIntNormal(),
        maIntTexture(),
        maIntInvTexture(),
        mbModifyColor(false),
        mbUseTex(false),
        mbHasTexCoor(false),
        mbHasInvTexCoor(false),
        mbUseNrm(false),
        mbUseCol(false),
        mbModulate(bModulate)
    {
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

    void execute()
    {
        while(mnCount--)
        {
            // early-test Z values if we need to do anything at all
            const double fNewZ(::std::max(0.0, ::std::min((double)0xffff, maIntZ.getVal())));
            const sal_uInt16 nNewZ(static_cast< sal_uInt16 >(fNewZ));

            if(nNewZ > *mpZ)
            {
                // detect color and opacity for this pixel
                basegfx::BColor aNewColor;
                const sal_uInt16 nOpacity(::std::max((sal_Int16)0, static_cast< sal_Int16 >(decideColorAndOpacity(aNewColor) * 255.0)));

                if(nOpacity > 0)
                {
                    // avoid color overrun
                    aNewColor.clamp();

                    if(nOpacity >= 0x00ff)
                    {
                        // full opacity (not transparent), set z and color
                        *mpZ = nNewZ;
                        *mpBPixel = basegfx::BPixel(aNewColor, 0xff);
                    }
                    else
                    {
                        if(mpBPixel->getOpacity())
                        {
                            // mix new color by using
                            // color' = color * (1 - opacity) + newcolor * opacity
                            const sal_uInt16 nTransparence(0x0100 - nOpacity);
                            mpBPixel->setRed((sal_uInt8)(((mpBPixel->getRed() * nTransparence) + ((sal_uInt16)(255.0 * aNewColor.getRed()) * nOpacity)) >> 8));
                            mpBPixel->setGreen((sal_uInt8)(((mpBPixel->getGreen() * nTransparence) + ((sal_uInt16)(255.0 * aNewColor.getGreen()) * nOpacity)) >> 8));
                            mpBPixel->setBlue((sal_uInt8)(((mpBPixel->getBlue() * nTransparence) + ((sal_uInt16)(255.0 * aNewColor.getBlue()) * nOpacity)) >> 8));

                            if(0xff != mpBPixel->getOpacity())
                            {
                                // both are transparent, mix new opacity by using
                                // opacity = newopacity * (1 - oldopacity) + oldopacity
                                mpBPixel->setOpacity(((sal_uInt8)((nOpacity * (0x0100 - mpBPixel->getOpacity())) >> 8)) + mpBPixel->getOpacity());
                            }
                        }
                        else
                        {
                            // dest is unused, set color
                            *mpBPixel = basegfx::BPixel(aNewColor, (sal_uInt8)nOpacity);
                        }
                    }
                }
            }

            // increments
            incrementLineSpanInterpolators(1.0);
            mpBPixel++;
            mpZ++;
        }
    }
};

class LineSpanExecutor : public comphelper::ThreadTask
{
private:
    osl::Mutex                  m_aMutex;
    std::list< LineSpanJob* >   maJobs;
    bool                        mbFinished;

public:
    explicit LineSpanExecutor()
    :   m_aMutex(),
        maJobs(),
        mbFinished(false)
    {
    }

    void push(LineSpanJob* pNew)
    {
        osl::MutexGuard aGuard(m_aMutex);
        maJobs.push_back(pNew);
    }

    void setFinished()
    {
        osl::MutexGuard aGuard(m_aMutex);
        mbFinished = true;
    }

    virtual void doWork() override
    {
        while(!mbFinished || !maJobs.empty())
        {
            LineSpanJob* pCandidate = 0;

            if(!maJobs.empty())
            {
                osl::MutexGuard aGuard(m_aMutex);
                pCandidate = maJobs.front();
                maJobs.pop_front();
            }

            if(pCandidate)
            {
                pCandidate->execute();
                delete pCandidate;
            }
            else
            {
                TimeValue aDelay(0, 1000);
                osl_waitThread(&aDelay);
            }
        }
    }
};

class ZBufferRasterConverter3D : public basegfx::RasterConverter3D
{
private:
    const drawinglayer::processor3d::DefaultProcessor3D&    mrProcessor;
    basegfx::BZPixelRaster                                  maBuffer;
    drawinglayer::attribute::MaterialAttribute3D            maCurrentMaterial;
    comphelper::ThreadPool&                                 mrShared;
    sal_Int32                                               mnWorkerCount;
    std::vector< LineSpanExecutor* >                        maWorkers;
    bool                                                    mbTasksSetup;

    void setupLineSpanInterpolators(
        const basegfx::RasterConversionLineEntry3D& rA,
        const basegfx::RasterConversionLineEntry3D& rB,
        LineSpanJob& rJob)
    {
        // get inverse XDelta
        const double xInvDelta(1.0 / (rB.getX().getVal() - rA.getX().getVal()));

        // prepare Z-interpolator
        const double fZA(rA.getZ().getVal());
        const double fZB(rB.getZ().getVal());
        rJob.maIntZ = basegfx::ip_single(fZA, (fZB - fZA) * xInvDelta);

        // get bools and init other interpolators on demand accordingly
        rJob.mbModifyColor = mrProcessor.getBColorModifierStack().count();
        rJob.mbHasTexCoor = SCANLINE_EMPTY_INDEX != rA.getTextureIndex() && SCANLINE_EMPTY_INDEX != rB.getTextureIndex();
        rJob.mbHasInvTexCoor = SCANLINE_EMPTY_INDEX != rA.getInverseTextureIndex() && SCANLINE_EMPTY_INDEX != rB.getInverseTextureIndex();
        const bool bTextureActive(mrProcessor.getGeoTexSvx().get() || mrProcessor.getTransparenceGeoTexSvx().get());
        rJob.mbUseTex = bTextureActive && (rJob.mbHasTexCoor || rJob.mbHasInvTexCoor || mrProcessor.getSimpleTextureActive());
        const bool bUseColorTex(rJob.mbUseTex && mrProcessor.getGeoTexSvx().get());
        const bool bNeedNrmOrCol(!bUseColorTex || mrProcessor.getModulate());
        rJob.mbUseNrm = bNeedNrmOrCol && SCANLINE_EMPTY_INDEX != rA.getNormalIndex() && SCANLINE_EMPTY_INDEX != rB.getNormalIndex();
        rJob.mbUseCol = !rJob.mbUseNrm && bNeedNrmOrCol && SCANLINE_EMPTY_INDEX != rA.getColorIndex() && SCANLINE_EMPTY_INDEX != rB.getColorIndex();

        if(rJob.mbUseTex)
        {
            if(rJob.mbHasTexCoor)
            {
                const basegfx::ip_double& rTA(getTextureInterpolators()[rA.getTextureIndex()]);
                const basegfx::ip_double& rTB(getTextureInterpolators()[rB.getTextureIndex()]);
                rJob.maIntTexture = basegfx::ip_double(
                    rTA.getX().getVal(), (rTB.getX().getVal() - rTA.getX().getVal()) * xInvDelta,
                    rTA.getY().getVal(), (rTB.getY().getVal() - rTA.getY().getVal()) * xInvDelta);
            }
            else if(rJob.mbHasInvTexCoor)
            {
                const basegfx::ip_triple& rITA(getInverseTextureInterpolators()[rA.getInverseTextureIndex()]);
                const basegfx::ip_triple& rITB(getInverseTextureInterpolators()[rB.getInverseTextureIndex()]);
                rJob.maIntInvTexture = basegfx::ip_triple(
                    rITA.getX().getVal(), (rITB.getX().getVal() - rITA.getX().getVal()) * xInvDelta,
                    rITA.getY().getVal(), (rITB.getY().getVal() - rITA.getY().getVal()) * xInvDelta,
                    rITA.getZ().getVal(), (rITB.getZ().getVal() - rITA.getZ().getVal()) * xInvDelta);
            }
        }

        if(rJob.mbUseNrm)
        {
            const basegfx::ip_triple& rNA(getNormalInterpolators()[rA.getNormalIndex()]);
            const basegfx::ip_triple& rNB(getNormalInterpolators()[rB.getNormalIndex()]);
            rJob.maIntNormal = basegfx::ip_triple(
                rNA.getX().getVal(), (rNB.getX().getVal() - rNA.getX().getVal()) * xInvDelta,
                rNA.getY().getVal(), (rNB.getY().getVal() - rNA.getY().getVal()) * xInvDelta,
                rNA.getZ().getVal(), (rNB.getZ().getVal() - rNA.getZ().getVal()) * xInvDelta);
        }

        if(rJob.mbUseCol)
        {
            const basegfx::ip_triple& rCA(getColorInterpolators()[rA.getColorIndex()]);
            const basegfx::ip_triple& rCB(getColorInterpolators()[rB.getColorIndex()]);
            rJob.maIntColor = basegfx::ip_triple(
                rCA.getX().getVal(), (rCB.getX().getVal() - rCA.getX().getVal()) * xInvDelta,
                rCA.getY().getVal(), (rCB.getY().getVal() - rCA.getY().getVal()) * xInvDelta,
                rCA.getZ().getVal(), (rCB.getZ().getVal() - rCA.getZ().getVal()) * xInvDelta);
        }
    }

    virtual void processLineSpan(const basegfx::RasterConversionLineEntry3D& rA, const basegfx::RasterConversionLineEntry3D& rB, sal_Int32 nLine, sal_uInt32 nSpanCount) override;

public:
    ZBufferRasterConverter3D(sal_uInt32 nWidth, sal_uInt32 nHeight, const drawinglayer::processor3d::ZBufferProcessor3D& rProcessor)
    :   basegfx::RasterConverter3D(),
        mrProcessor(rProcessor),
        maBuffer(nWidth, nHeight),
        maCurrentMaterial(),
        mrShared(comphelper::ThreadPool::getSharedOptimalPool()),
        mnWorkerCount(mrShared.getWorkerCount()),
        maWorkers(),
        mbTasksSetup(false)
    {
        static bool bAllowParallellExecution(true);

        if(!bAllowParallellExecution)
        {
            // switch off parallell execution
            mnWorkerCount = 0;
        }
        else
        {
            // limit to number of lines for small 3D scenes (remember, this is multiplied
            // with mnAntiAlialize already)
            mnWorkerCount = std::min(mnWorkerCount, static_cast< sal_Int32 >(nHeight));
        }
    }

    virtual ~ZBufferRasterConverter3D() override;

    void setCurrentMaterial(const drawinglayer::attribute::MaterialAttribute3D& rMaterial)
    {
        maCurrentMaterial = rMaterial;
    }

    sal_uInt32 getWidth() const
    {
        return maBuffer.getWidth();
    }

    sal_uInt32 getHeight() const
    {
        return maBuffer.getHeight();
    }

    void setFinishedAndWait()
    {
        if(mbTasksSetup)
        {
            for(auto candidate : maWorkers)
            {
                candidate->setFinished();
            }

            mbTasksSetup = false;
            mrShared.waitUntilEmpty();
        }
    }

    BitmapEx getBitmapEx(sal_uInt16 nAntiAlialize) const
    {
        const_cast< ZBufferRasterConverter3D* >(this)->setFinishedAndWait();

        return BPixelRasterToBitmapEx(maBuffer, nAntiAlialize);
    }
};

ZBufferRasterConverter3D::~ZBufferRasterConverter3D()
{
    setFinishedAndWait();
}

void ZBufferRasterConverter3D::processLineSpan(const basegfx::RasterConversionLineEntry3D& rA, const basegfx::RasterConversionLineEntry3D& rB, sal_Int32 nLine, sal_uInt32 nSpanCount)
{
    if(!(nSpanCount & 0x0001))
    {
        if(nLine >= 0 && nLine < (sal_Int32)maBuffer.getHeight())
        {
            sal_uInt32 nXA(::std::min(maBuffer.getWidth(), (sal_uInt32)::std::max((sal_Int32)0, basegfx::fround(rA.getX().getVal()))));
            const sal_uInt32 nXB(::std::min(maBuffer.getWidth(), (sal_uInt32)::std::max((sal_Int32)0, basegfx::fround(rB.getX().getVal()))));

            if(nXA < nXB)
            {
                // create LineSpanJob
                static sal_uInt32 nCount = 0;
                nCount++;
                const sal_uInt32 nIndex(maBuffer.getIndexFromXY(nXA, static_cast<sal_uInt32>(nLine)));
                LineSpanJob* pLineSpanJob = new LineSpanJob(
                    nXB - nXA,
                    maBuffer.getBPixel(nIndex),
                    maBuffer.getZ(nIndex),
                    maCurrentMaterial,
                    mrProcessor.getGeoTexSvx(),
                    mrProcessor.getTransparenceGeoTexSvx(),
                    mrProcessor.getSdrLightingAttribute(),
                    mrProcessor.getBColorModifierStack(),
                    mrProcessor.getModulate());

                // prepare the span interpolators
                setupLineSpanInterpolators(rA, rB, *pLineSpanJob);

                // bring span interpolators to start condition by incrementing with the possible difference of
                // clamped and non-clamped XStart. Interpolators are setup relying on double precision
                // X-values, so that difference is the correct value to compensate for possible clampings
                pLineSpanJob->incrementLineSpanInterpolators(static_cast<double>(nXA) - rA.getX().getVal());

                if(mnWorkerCount > 0)
                {
                    if(!mbTasksSetup)
                    {
                        for(sal_Int32 a(0); a < mnWorkerCount; a++)
                        {
                            LineSpanExecutor* pNewThread = new LineSpanExecutor();
                            maWorkers.push_back(pNewThread);
                            mrShared.pushTask(pNewThread);
                        }

                        mbTasksSetup = true;
                    }

                    maWorkers[nLine % mnWorkerCount]->push(pLineSpanJob);
                }
                else
                {
                    // render it
                    pLineSpanJob->execute();
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
    std::shared_ptr< drawinglayer::texture::GeoTexSvx > mpGeoTexSvx;
    std::shared_ptr< drawinglayer::texture::GeoTexSvx > mpTransparenceGeoTexSvx;
    drawinglayer::attribute::MaterialAttribute3D        maMaterial;
    basegfx::B3DPolyPolygon                             maPolyPolygon;
    double                                              mfCenterZ;

    // bitfield
    bool                                                mbModulate : 1;
    bool                                                mbFilter : 1;
    bool                                                mbSimpleTextureActive : 1;
    bool                                                mbIsLine : 1;

    RasterPrimitive3D(const RasterPrimitive3D&) = delete;
    RasterPrimitive3D& operator =( const RasterPrimitive3D& ) = delete;

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
            if(mpZBufferRasterConverter3D)
            {
                if(getTransparenceCounter())
                {
                    // transparent output; record for later sorting and painting from
                    // back to front
                    const_cast< ZBufferProcessor3D* >(this)->maRasterPrimitive3Ds.push_back(
                        RasterPrimitive3DSharedPtr(
                            new RasterPrimitive3D(
                                getGeoTexSvx(),
                                getTransparenceGeoTexSvx(),
                                rMaterial,
                                basegfx::B3DPolyPolygon(rHairline),
                                getModulate(),
                                getFilter(),
                                getSimpleTextureActive(),
                                true)));
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

                            mpZBufferRasterConverter3D->rasterconvertB3DPolygon(aSnappedHairline, 0, mpZBufferRasterConverter3D->getHeight(), mnAntiAlialize);
                        }
                        else
                        {
                            mpZBufferRasterConverter3D->rasterconvertB3DPolygon(rHairline, 0, mpZBufferRasterConverter3D->getHeight(), mnAntiAlialize);
                        }
                    }
                    else
                    {
                        mpZBufferRasterConverter3D->rasterconvertB3DPolygon(rHairline, 0, mpZBufferRasterConverter3D->getHeight(), 1);
                    }
                }
            }
        }

        void ZBufferProcessor3D::rasterconvertB3DPolyPolygon(const attribute::MaterialAttribute3D& rMaterial, const basegfx::B3DPolyPolygon& rFill) const
        {
            if(mpZBufferRasterConverter3D)
            {
                if(getTransparenceCounter())
                {
                    // transparent output; record for later sorting and painting from
                    // back to front
                    const_cast< ZBufferProcessor3D* >(this)->maRasterPrimitive3Ds.push_back(
                        RasterPrimitive3DSharedPtr(
                            new RasterPrimitive3D(
                                getGeoTexSvx(),
                                getTransparenceGeoTexSvx(),
                                rMaterial,
                                rFill,
                                getModulate(),
                                getFilter(),
                                getSimpleTextureActive(),
                                false)));
                }
                else
                {
                    mpZBufferRasterConverter3D->setCurrentMaterial(rMaterial);
                    mpZBufferRasterConverter3D->rasterconvertB3DPolyPolygon(rFill, &maInvEyeToView, 0, mpZBufferRasterConverter3D->getHeight());
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
            maInvEyeToView(),
            mpZBufferRasterConverter3D(nullptr),
            mnAntiAlialize(nAntiAlialize),
            maRasterPrimitive3Ds()
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

                // prepare buffer size
                const sal_uInt32 nWidth(mnAntiAlialize ? nRasterWidth * mnAntiAlialize : nRasterWidth);
                const sal_uInt32 nHeight(mnAntiAlialize ? nRasterHeight * mnAntiAlialize : nRasterHeight);

                // prepare maRasterRange
                maRasterRange.reset();
                maRasterRange.expand(basegfx::B2DPoint(0.0, 0.0));
                maRasterRange.expand(basegfx::B2DPoint(nWidth, nHeight));

                // create the raster converter
                mpZBufferRasterConverter3D = new ZBufferRasterConverter3D(nWidth, nHeight, *this);
            }
        }

        ZBufferProcessor3D::~ZBufferProcessor3D()
        {
            if(mpZBufferRasterConverter3D)
            {
                delete mpZBufferRasterConverter3D;
            }
        }

        void ZBufferProcessor3D::finish()
        {
            if(!maRasterPrimitive3Ds.empty())
            {
                // there are transparent rasterprimitives
                // sort them from back to front
                struct RasterPrimitive3DOrder
                {
                    bool operator()(const RasterPrimitive3DSharedPtr& rL, const RasterPrimitive3DSharedPtr& rR)
                    {
                        return *rL < *rR;
                    }
                };

                std::sort(maRasterPrimitive3Ds.begin(), maRasterPrimitive3Ds.end(), RasterPrimitive3DOrder());

                for(auto candidate : maRasterPrimitive3Ds)
                {
                    // paint each one by setting the remembered data and calling
                    // the render method
                    mpGeoTexSvx = candidate->getGeoTexSvx();
                    mpTransparenceGeoTexSvx = candidate->getTransparenceGeoTexSvx();
                    mbModulate = candidate->getModulate();
                    mbFilter = candidate->getFilter();
                    mbSimpleTextureActive = candidate->getSimpleTextureActive();

                    if(candidate->getIsLine())
                    {
                        rasterconvertB3DPolygon(
                            candidate->getMaterial(),
                            candidate->getPolyPolygon().getB3DPolygon(0));
                    }
                    else
                    {
                        rasterconvertB3DPolyPolygon(
                            candidate->getMaterial(),
                            candidate->getPolyPolygon());
                    }
                }

                // clear them to signal the destructor that all is done and
                // to allow asserting there
                maRasterPrimitive3Ds.clear();
            }
        }

        BitmapEx ZBufferProcessor3D::getBitmapEx() const
        {
            if(mpZBufferRasterConverter3D)
            {
                return mpZBufferRasterConverter3D->getBitmapEx(mnAntiAlialize);
            }

            return BitmapEx();
        }
    } // end of namespace processor3d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
