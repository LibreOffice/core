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

#include <sal/config.h>
#include <sal/log.hxx>

#include <comphelper/fileformat.h>
#include <o3tl/make_shared.hxx>
#include <tools/fract.hxx>
#include <tools/vcompat.hxx>
#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/tempfile.hxx>
#include <utility>
#include <vcl/filter/SvmReader.hxx>
#include <vcl/filter/SvmWriter.hxx>
#include <vcl/outdev.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/virdev.hxx>
#include <vcl/gfxlink.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/graph.hxx>
#include <vcl/metaact.hxx>
#include <impgraph.hxx>
#include <com/sun/star/graphic/XPrimitive2D.hpp>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <vcl/dibtools.hxx>
#include <map>
#include <memory>
#include <vcl/gdimetafiletools.hxx>
#include <vcl/TypeSerializer.hxx>
#include <vcl/pdfread.hxx>
#include <graphic/VectorGraphicLoader.hxx>

#define GRAPHIC_MTFTOBMP_MAXEXT     2048
#define GRAPHIC_STREAMBUFSIZE       8192UL

#define SWAP_FORMAT_ID COMPAT_FORMAT( 'S', 'W', 'A', 'P' )

using namespace com::sun::star;


class ImpSwapFile
{
private:
    utl::TempFileFast maTempFile;
    OUString maOriginURL;

public:
    ImpSwapFile(OUString aOriginURL)
        : maOriginURL(std::move(aOriginURL))
    {
    }

    SvStream* getStream() { return maTempFile.GetStream(StreamMode::READWRITE); }
    OUString const & getOriginURL() const { return maOriginURL; }
};

SvStream* ImpGraphic::getSwapFileStream() const
{
    if (mpSwapFile)
        return mpSwapFile->getStream();
    return nullptr;
}

ImpGraphic::ImpGraphic(bool bDefault)
    : MemoryManaged(false)
    , meType(bDefault ? GraphicType::Default : GraphicType::NONE)
{
}

ImpGraphic::ImpGraphic(const ImpGraphic& rImpGraphic)
    : MemoryManaged(rImpGraphic)
    , maCachedBitmap(rImpGraphic.maCachedBitmap)
    , maMetaFile(rImpGraphic.maMetaFile)
    , mpBitmapContainer(rImpGraphic.mpBitmapContainer)
    , maSwapInfo(rImpGraphic.maSwapInfo)
    , mpSwapFile(rImpGraphic.mpSwapFile)
    , mpGfxLink(rImpGraphic.mpGfxLink)
    , maVectorGraphicData(rImpGraphic.maVectorGraphicData)
    , meType(rImpGraphic.meType)
    , mnSizeBytes(rImpGraphic.mnSizeBytes)
    , mbSwapOut(rImpGraphic.mbSwapOut)
    , mbDummyContext(rImpGraphic.mbDummyContext)
    , maGraphicExternalLink(rImpGraphic.maGraphicExternalLink)
    , mbPrepared(rImpGraphic.mbPrepared)
{
    updateCurrentSizeInBytes(mnSizeBytes);

    // Special case for animations
    if (rImpGraphic.mpAnimationContainer)
    {
        mpAnimationContainer = std::make_shared<AnimationContainer>(rImpGraphic.mpAnimationContainer->maAnimation);
        maCachedBitmap = mpAnimationContainer->maAnimation.GetBitmapEx();
    }
}

ImpGraphic::ImpGraphic(ImpGraphic&& rImpGraphic) noexcept
    : MemoryManaged(rImpGraphic)
    , maCachedBitmap(std::move(rImpGraphic.maCachedBitmap))
    , maMetaFile(std::move(rImpGraphic.maMetaFile))
    , mpBitmapContainer(std::move(rImpGraphic.mpBitmapContainer))
    , maSwapInfo(std::move(rImpGraphic.maSwapInfo))
    , mpAnimationContainer(std::move(rImpGraphic.mpAnimationContainer))
    , mpSwapFile(std::move(rImpGraphic.mpSwapFile))
    , mpGfxLink(std::move(rImpGraphic.mpGfxLink))
    , maVectorGraphicData(std::move(rImpGraphic.maVectorGraphicData))
    , meType(rImpGraphic.meType)
    , mnSizeBytes(rImpGraphic.mnSizeBytes)
    , mbSwapOut(rImpGraphic.mbSwapOut)
    , mbDummyContext(rImpGraphic.mbDummyContext)
    , maGraphicExternalLink(rImpGraphic.maGraphicExternalLink)
    , mbPrepared (rImpGraphic.mbPrepared)
{
    updateCurrentSizeInBytes(mnSizeBytes);

    rImpGraphic.clear();
    rImpGraphic.mbDummyContext = false;
}

ImpGraphic::ImpGraphic(std::shared_ptr<GfxLink> xGfxLink, sal_Int32 nPageIndex)
    : MemoryManaged(true)
    , mpGfxLink(std::move(xGfxLink))
    , meType(GraphicType::Bitmap)
    , mbSwapOut(true)
{
    maSwapInfo.mbIsTransparent = true;
    maSwapInfo.mbIsAlpha = true;
    maSwapInfo.mbIsEPS = false;
    maSwapInfo.mbIsAnimated = false;
    maSwapInfo.mnAnimationLoopCount = 0;
    maSwapInfo.mnPageIndex = nPageIndex;

    ensureCurrentSizeInBytes();
}

ImpGraphic::ImpGraphic(GraphicExternalLink aGraphicExternalLink)
    : MemoryManaged(true)
    , meType(GraphicType::Default)
    , maGraphicExternalLink(std::move(aGraphicExternalLink))
{
    ensureCurrentSizeInBytes();
}

ImpGraphic::ImpGraphic(const BitmapEx& rBitmapEx)
    : MemoryManaged(!rBitmapEx.IsEmpty())
    , mpBitmapContainer(new BitmapContainer(rBitmapEx))
    , meType(rBitmapEx.IsEmpty() ? GraphicType::NONE : GraphicType::Bitmap)
{
    ensureCurrentSizeInBytes();
}

ImpGraphic::ImpGraphic(const std::shared_ptr<VectorGraphicData>& rVectorGraphicDataPtr)
    : MemoryManaged(bool(rVectorGraphicDataPtr))
    , maVectorGraphicData(rVectorGraphicDataPtr)
    , meType(rVectorGraphicDataPtr ? GraphicType::Bitmap : GraphicType::NONE)
{
    ensureCurrentSizeInBytes();
}

ImpGraphic::ImpGraphic(const Animation& rAnimation)
    : MemoryManaged(true)
    , maCachedBitmap(rAnimation.GetBitmapEx())
    , mpAnimationContainer(std::make_shared<AnimationContainer>(rAnimation))
    , meType(GraphicType::Bitmap)
{
    ensureCurrentSizeInBytes();
}

ImpGraphic::ImpGraphic(const GDIMetaFile& rMetafile)
    : MemoryManaged(true)
    , maMetaFile(rMetafile)
    , meType(GraphicType::GdiMetafile)
{
    ensureCurrentSizeInBytes();
}

ImpGraphic::~ImpGraphic()
{
}

ImpGraphic& ImpGraphic::operator=(const ImpGraphic& rImpGraphic)
{
    if( &rImpGraphic != this )
    {
        maMetaFile = rImpGraphic.maMetaFile;
        meType = rImpGraphic.meType;
        mnSizeBytes = rImpGraphic.mnSizeBytes;
        updateCurrentSizeInBytes(mnSizeBytes);

        maSwapInfo = rImpGraphic.maSwapInfo;
        mbDummyContext = rImpGraphic.mbDummyContext;
        maGraphicExternalLink = rImpGraphic.maGraphicExternalLink;

        mpAnimationContainer.reset();
        if (rImpGraphic.mpAnimationContainer)
        {
            mpAnimationContainer = std::make_shared<AnimationContainer>(*rImpGraphic.mpAnimationContainer);
            maCachedBitmap = mpAnimationContainer->maAnimation.GetBitmapEx();
        }
        else
        {
            maCachedBitmap = rImpGraphic.maCachedBitmap;
        }

        mpBitmapContainer.reset();
        if (rImpGraphic.mpBitmapContainer)
            mpBitmapContainer = rImpGraphic.mpBitmapContainer;

        mbSwapOut = rImpGraphic.mbSwapOut;
        mpSwapFile = rImpGraphic.mpSwapFile;
        mbPrepared = rImpGraphic.mbPrepared;

        mpGfxLink = rImpGraphic.mpGfxLink;

        maVectorGraphicData.reset();
        if (rImpGraphic.maVectorGraphicData)
            maVectorGraphicData = rImpGraphic.maVectorGraphicData;

        resetLastUsed();

        changeExisting(mnSizeBytes);
    }

    return *this;
}

ImpGraphic& ImpGraphic::operator=(ImpGraphic&& rImpGraphic)
{
    maMetaFile = std::move(rImpGraphic.maMetaFile);
    meType = rImpGraphic.meType;
    mnSizeBytes = rImpGraphic.mnSizeBytes;
    maSwapInfo = std::move(rImpGraphic.maSwapInfo);
    mbDummyContext = rImpGraphic.mbDummyContext;
    maCachedBitmap = std::move(rImpGraphic.maCachedBitmap);
    mpAnimationContainer = std::move(rImpGraphic.mpAnimationContainer);
    mpBitmapContainer = std::move(rImpGraphic.mpBitmapContainer);
    mbSwapOut = rImpGraphic.mbSwapOut;
    mpSwapFile = std::move(rImpGraphic.mpSwapFile);
    mpGfxLink = std::move(rImpGraphic.mpGfxLink);
    maVectorGraphicData = std::move(rImpGraphic.maVectorGraphicData);
    maGraphicExternalLink = rImpGraphic.maGraphicExternalLink;
    mbPrepared = rImpGraphic.mbPrepared;

    rImpGraphic.clear();
    rImpGraphic.mbDummyContext = false;
    resetLastUsed();

    changeExisting(mnSizeBytes);

    return *this;
}

bool ImpGraphic::operator==( const ImpGraphic& rOther ) const
{
    if( this == &rOther )
        return true;

    if (mbPrepared && rOther.mbPrepared)
        return (*mpGfxLink == *rOther.mpGfxLink);

    if (!isAvailable() || !rOther.isAvailable())
        return false;

    if ( meType != rOther.meType )
        return false;

    bool bRet = false;
    switch( meType )
    {
        case GraphicType::NONE:
        case GraphicType::Default:
            return true;

        case GraphicType::GdiMetafile:
            return ( rOther.maMetaFile == maMetaFile );

        case GraphicType::Bitmap:
        {
            if (maVectorGraphicData)
            {
                if (maVectorGraphicData == rOther.maVectorGraphicData)
                    // equal instances
                    bRet = true;
                else if (rOther.maVectorGraphicData)
                    // equal content
                    bRet = (*maVectorGraphicData) == (*rOther.maVectorGraphicData);
            }
            else if (mpAnimationContainer && rOther.mpAnimationContainer && (*mpAnimationContainer == *rOther.mpAnimationContainer))
            {
                bRet = true;
            }
            else if (mpBitmapContainer && rOther.mpBitmapContainer && (*mpBitmapContainer == *rOther.mpBitmapContainer))
            {
                bRet = true;
            }
        }
        break;
    }

    return bRet;
}

const std::shared_ptr<VectorGraphicData>& ImpGraphic::getVectorGraphicData() const
{
    ensureAvailable();

    return maVectorGraphicData;
}

void BitmapContainer::createSwapInfo(ImpSwapInfo& rSwapInfo)
{
    rSwapInfo.maSizePixel = maBitmapEx.GetSizePixel();

    rSwapInfo.maPrefMapMode = getPrefMapMode();
    rSwapInfo.maPrefSize = getPrefSize();
    rSwapInfo.mbIsAnimated = false;
    rSwapInfo.mbIsEPS = false;
    rSwapInfo.mbIsTransparent = isAlpha();
    rSwapInfo.mbIsAlpha = isAlpha();
    rSwapInfo.mnAnimationLoopCount = 0;
    rSwapInfo.mnPageIndex = -1;
}

void ImpGraphic::createSwapInfo()
{
    if (isSwappedOut())
        return;

    if (mpBitmapContainer)
    {
        mpBitmapContainer->createSwapInfo(maSwapInfo);
        return;
    }
    else if (!maCachedBitmap.IsEmpty())
        maSwapInfo.maSizePixel = maCachedBitmap.GetSizePixel();
    else
        maSwapInfo.maSizePixel = Size();

    maSwapInfo.maPrefMapMode = getPrefMapMode();
    maSwapInfo.maPrefSize = getPrefSize();
    maSwapInfo.mbIsAnimated = isAnimated();
    maSwapInfo.mbIsEPS = isEPS();
    maSwapInfo.mbIsTransparent = isTransparent();
    maSwapInfo.mbIsAlpha = isAlpha();
    maSwapInfo.mnAnimationLoopCount = getAnimationLoopCount();
    maSwapInfo.mnPageIndex = getPageNumber();
}

void ImpGraphic::clearGraphics()
{
    maCachedBitmap.Clear();
    mpBitmapContainer.reset();
    maMetaFile.Clear();
    mpAnimationContainer.reset();
    maVectorGraphicData.reset();
}

void ImpGraphic::setPrepared(bool bAnimated, const Size* pSizeHint)
{
    mbPrepared = true;
    mbSwapOut = true;
    meType = GraphicType::Bitmap;

    SvMemoryStream aMemoryStream(const_cast<sal_uInt8*>(mpGfxLink->GetData()), mpGfxLink->GetDataSize(), StreamMode::READ | StreamMode::WRITE);

    if (pSizeHint)
    {
        maSwapInfo.maPrefSize = *pSizeHint;
        maSwapInfo.maPrefMapMode = MapMode(MapUnit::Map100thMM);
    }

    GraphicDescriptor aDescriptor(aMemoryStream, nullptr);
    if (aDescriptor.Detect(true))
    {
        if (!pSizeHint)
        {
            // If we have logic size, work with that, as later pixel -> logic
            // conversion will work with the output device DPI, not the graphic
            // DPI.
            Size aLogSize = aDescriptor.GetSize_100TH_MM();
            if (aDescriptor.GetPreferredLogSize() && aDescriptor.GetPreferredMapMode())
            {
                maSwapInfo.maPrefSize = *aDescriptor.GetPreferredLogSize();
                maSwapInfo.maPrefMapMode = *aDescriptor.GetPreferredMapMode();
            }
            else if (aLogSize.getWidth() && aLogSize.getHeight())
            {
                maSwapInfo.maPrefSize = aLogSize;
                maSwapInfo.maPrefMapMode = MapMode(MapUnit::Map100thMM);
            }
            else
            {
                maSwapInfo.maPrefSize = aDescriptor.GetSizePixel();
                maSwapInfo.maPrefMapMode = MapMode(MapUnit::MapPixel);
            }
        }

        maSwapInfo.maSizePixel = aDescriptor.GetSizePixel();
        maSwapInfo.mbIsTransparent = aDescriptor.IsTransparent();
        maSwapInfo.mbIsAlpha = aDescriptor.IsAlpha();
    } else {
        maSwapInfo.mbIsTransparent = false;
        maSwapInfo.mbIsAlpha = false;
    }

    maSwapInfo.mnAnimationLoopCount = 0;
    maSwapInfo.mbIsEPS = false;
    maSwapInfo.mbIsAnimated = bAnimated;

    if (maVectorGraphicData)
        maSwapInfo.mnPageIndex = maVectorGraphicData->getPageIndex();
}

void ImpGraphic::clear()
{
    mpSwapFile.reset();
    mbSwapOut = false;
    mbPrepared = false;

    // cleanup
    clearGraphics();
    meType = GraphicType::NONE;
    mnSizeBytes = 0;

    changeExisting(mnSizeBytes);
    maGraphicExternalLink.msURL.clear();
}

bool ImpGraphic::isSupportedGraphic() const
{
    return meType != GraphicType::NONE;
}

bool ImpGraphic::isTransparent() const
{
    if (mbSwapOut)
    {
        return maSwapInfo.mbIsTransparent;
    }
    else if (meType == GraphicType::Bitmap)
    {
        if (maVectorGraphicData)
            return true;
        else if (mpBitmapContainer)
            return mpBitmapContainer->isAlpha();
        else if (mpAnimationContainer)
            return mpAnimationContainer->isTransparent();
    }

    return true;
}

bool ImpGraphic::isAlpha() const
{
    if (mbSwapOut)
        return maSwapInfo.mbIsAlpha;

    if (meType == GraphicType::Bitmap)
    {
        if (maVectorGraphicData)
            return true;
        else if (mpBitmapContainer)
            return mpBitmapContainer->isAlpha();
    }
    return false;
}

bool ImpGraphic::isAnimated() const
{
    return mbSwapOut ? maSwapInfo.mbIsAnimated : mpAnimationContainer != nullptr;
}

bool ImpGraphic::isEPS() const
{
    if (mbSwapOut)
        return maSwapInfo.mbIsEPS;

    return( ( meType == GraphicType::GdiMetafile ) &&
            ( maMetaFile.GetActionSize() > 0 ) &&
            ( maMetaFile.GetAction( 0 )->GetType() == MetaActionType::EPS ) );
}

bool ImpGraphic::isAvailable() const
{
    return !mbPrepared && !mbSwapOut;
}

bool ImpGraphic::makeAvailable()
{
    return ensureAvailable();
}

void ImpGraphic::updateBitmapFromVectorGraphic(const Size& pixelSize) const
{
    assert (maVectorGraphicData);
    auto* pThisRW = const_cast<ImpGraphic*>(this);
    // use maBitmapEx as local buffer for rendered vector image
    if (pixelSize.Width() && pixelSize.Height())
    {
        if (maCachedBitmap.IsEmpty() || maCachedBitmap.GetSizePixel() != pixelSize)
            pThisRW->maCachedBitmap = maVectorGraphicData->getBitmap(pixelSize);
    }
    else // maVectorGraphicData caches the replacement, so updating unconditionally is cheap
    {
        pThisRW->maCachedBitmap = maVectorGraphicData->getReplacement();
    }

    if (maExPrefSize.getWidth() && maExPrefSize.getHeight())
        pThisRW->maCachedBitmap.SetPrefSize(maExPrefSize);
}

Bitmap ImpGraphic::getBitmap(const GraphicConversionParameters& rParameters) const
{
    Bitmap aRetBmp;

    ensureAvailable();

    if (meType == GraphicType::Bitmap)
    {
        if (!mpAnimationContainer && maVectorGraphicData)
            updateBitmapFromVectorGraphic(rParameters.getSizePixel());

        const BitmapEx& rRetBmpEx = mpAnimationContainer
            ? mpAnimationContainer->maAnimation.GetBitmapEx()
            : (mpBitmapContainer ? mpBitmapContainer->maBitmapEx : maCachedBitmap);

        aRetBmp = rRetBmpEx.GetBitmap(COL_WHITE);

        if (rParameters.getSizePixel().Width() || rParameters.getSizePixel().Height())
            aRetBmp.Scale(rParameters.getSizePixel());
    }
    else if( ( meType != GraphicType::Default ) && isSupportedGraphic() )
    {
        if (maCachedBitmap.IsEmpty())
        {
            // calculate size
            ScopedVclPtrInstance< VirtualDevice > aVDev;
            Size aDrawSize(aVDev->LogicToPixel(maMetaFile.GetPrefSize(), maMetaFile.GetPrefMapMode()));

            if(rParameters.getSizePixel().Width() && rParameters.getSizePixel().Height())
            {
                // apply given size if exists
                aDrawSize = rParameters.getSizePixel();
            }

            if(aDrawSize.Width() && aDrawSize.Height() && !rParameters.getUnlimitedSize()
                && (aDrawSize.Width() > GRAPHIC_MTFTOBMP_MAXEXT || aDrawSize.Height() > GRAPHIC_MTFTOBMP_MAXEXT))
            {
                // limit bitmap size to a maximum of GRAPHIC_MTFTOBMP_MAXEXT x GRAPHIC_MTFTOBMP_MAXEXT
                double fWH(static_cast<double>(aDrawSize.Width()) / static_cast<double>(aDrawSize.Height()));

                if(fWH <= 1.0)
                {
                    aDrawSize.setWidth(basegfx::fround<tools::Long>(GRAPHIC_MTFTOBMP_MAXEXT * fWH));
                    aDrawSize.setHeight(GRAPHIC_MTFTOBMP_MAXEXT);
                }
                else
                {
                    aDrawSize.setWidth(GRAPHIC_MTFTOBMP_MAXEXT);
                    aDrawSize.setHeight(basegfx::fround<tools::Long>(GRAPHIC_MTFTOBMP_MAXEXT / fWH));
                }
            }

            // calculate pixel size. Normally, it's the same as aDrawSize, but may
            // need to be extended when hairlines are on the right or bottom edge
            Size aPixelSize(aDrawSize);

            if(GraphicType::GdiMetafile == getType())
            {
                // tdf#126319 Removed correction based on hairline-at-the-extremes of
                // the metafile. The task shows that this is no longer sufficient since
                // less hairlines get used in general - what is good, but breaks that
                // old fix. Anyways, hairlines are a left-over from non-AA times
                // when it was not possible to paint lines taller than one pixel.
                // This might need to be corrected further using primitives and
                // the possibility to get better-quality ranges for correction. For
                // now, always add that one pixel.
                aPixelSize.setWidth(aPixelSize.getWidth() + 1);
                aPixelSize.setHeight(aPixelSize.getHeight() + 1);
            }

            if(aVDev->SetOutputSizePixel(aPixelSize))
            {
                if(rParameters.getAntiAliase())
                {
                    aVDev->SetAntialiasing(aVDev->GetAntialiasing() | AntialiasingFlags::Enable);
                }

                if(rParameters.getSnapHorVerLines())
                {
                    aVDev->SetAntialiasing(aVDev->GetAntialiasing() | AntialiasingFlags::PixelSnapHairline);
                }

                draw(*aVDev, Point(), aDrawSize);

                // use maBitmapEx as local buffer for rendered metafile
                const_cast<ImpGraphic*>(this)->maCachedBitmap = aVDev->GetBitmapEx( Point(), aVDev->GetOutputSizePixel() );
            }
        }

        aRetBmp = maCachedBitmap.GetBitmap();
    }

    if( !aRetBmp.IsEmpty() )
    {
        aRetBmp.SetPrefMapMode(getPrefMapMode());
        aRetBmp.SetPrefSize(getPrefSize());
    }

    return aRetBmp;
}

BitmapEx ImpGraphic::getBitmapEx(const GraphicConversionParameters& rParameters) const
{
    ensureAvailable();

    BitmapEx aBitmapEx;

    if (meType == GraphicType::Bitmap)
    {
        if (maVectorGraphicData)
            updateBitmapFromVectorGraphic(rParameters.getSizePixel());

        aBitmapEx = mpAnimationContainer
                        ? mpAnimationContainer->maAnimation.GetBitmapEx()
                        : (mpBitmapContainer ? mpBitmapContainer->maBitmapEx : maCachedBitmap);

        if (rParameters.getSizePixel().Width() || rParameters.getSizePixel().Height())
            aBitmapEx.Scale(rParameters.getSizePixel(), BmpScaleFlag::Fast);
    }
    else if (meType != GraphicType::Default && isSupportedGraphic())
    {
        if (maCachedBitmap.IsEmpty())
        {
            const ImpGraphic aMonoMask( maMetaFile.GetMonochromeMtf( COL_BLACK ) );

            // use maBitmapEx as local buffer for rendered metafile
            const_cast<ImpGraphic*>(this)->maCachedBitmap = BitmapEx(getBitmap(rParameters), aMonoMask.getBitmap(rParameters));
        }

        aBitmapEx = maCachedBitmap;
    }

    return aBitmapEx;
}

Animation ImpGraphic::getAnimation() const
{
    Animation aAnimation;

    ensureAvailable();

    if (mpAnimationContainer)
        aAnimation = mpAnimationContainer->maAnimation;

    return aAnimation;
}

const BitmapEx& ImpGraphic::getBitmapExRef() const
{
    ensureAvailable();

    if (mpBitmapContainer)
        return mpBitmapContainer->getBitmapExRef();
    else
        return maCachedBitmap;
}

const GDIMetaFile& ImpGraphic::getGDIMetaFile() const
{
    ensureAvailable();
    if (!maMetaFile.GetActionSize()
        && maVectorGraphicData
        && (VectorGraphicDataType::Emf == maVectorGraphicData->getType()
            || VectorGraphicDataType::Wmf == maVectorGraphicData->getType()))
    {
        // If we have a Emf/Wmf VectorGraphic object, we
        // need a way to get the Metafile data out of the primitive
        // representation. Use a strict virtual hook (MetafileAccessor)
        // to access the MetafilePrimitive2D directly. Also see comments in
        // XEmfParser about this.
        const std::deque< css::uno::Reference< css::graphic::XPrimitive2D > > aSequence(maVectorGraphicData->getPrimitive2DSequence());

        if (1 == aSequence.size())
        {
            // try to cast to MetafileAccessor implementation
            const css::uno::Reference< css::graphic::XPrimitive2D >& xReference(aSequence[0]);
            auto pUnoPrimitive = static_cast< const drawinglayer::primitive2d::UnoPrimitive2D* >(xReference.get());
            if (pUnoPrimitive)
            {
                const MetafileAccessor* pMetafileAccessor = dynamic_cast< const MetafileAccessor* >(pUnoPrimitive->getBasePrimitive2D().get());

                if (pMetafileAccessor)
                {
                    // it is a MetafileAccessor implementation, get Metafile
                    pMetafileAccessor->accessMetafile(const_cast< ImpGraphic* >(this)->maMetaFile);
                }
            }
        }
    }

    if (GraphicType::Bitmap == meType && !maMetaFile.GetActionSize())
    {
        if (maVectorGraphicData)
            updateBitmapFromVectorGraphic();

        // #i119735#
        // Use the local maMetaFile as container for a metafile-representation
        // of the bitmap graphic. This will be done only once, thus be buffered.
        // I checked all usages of maMetaFile, it is only used when type is not
        // GraphicType::Bitmap. In operator= it will get copied, thus buffering will
        // survive copying (change this if not wanted)
        ImpGraphic* pThat = const_cast< ImpGraphic* >(this);

        BitmapEx aBitmapEx = mpBitmapContainer ? mpBitmapContainer->maBitmapEx : maCachedBitmap;

        // #123983# directly create a metafile with the same PrefSize and PrefMapMode
        // the bitmap has, this will be an always correct metafile
        if (aBitmapEx.IsAlpha())
        {
            pThat->maMetaFile.AddAction(new MetaBmpExScaleAction(Point(), aBitmapEx.GetPrefSize(), aBitmapEx));
        }
        else
        {
            pThat->maMetaFile.AddAction(new MetaBmpScaleAction(Point(), aBitmapEx.GetPrefSize(), aBitmapEx.GetBitmap()));
        }

        pThat->maMetaFile.Stop();
        pThat->maMetaFile.WindStart();
        pThat->maMetaFile.SetPrefSize(aBitmapEx.GetPrefSize());
        pThat->maMetaFile.SetPrefMapMode(aBitmapEx.GetPrefMapMode());
    }

    return maMetaFile;
}

Size ImpGraphic::getSizePixel() const
{
    Size aSize;

    if (isSwappedOut())
        aSize = maSwapInfo.maSizePixel;
    else
        aSize = getBitmapEx(GraphicConversionParameters()).GetSizePixel();

    return aSize;
}

Size ImpGraphic::getPrefSize() const
{
    Size aSize;

    if (isSwappedOut())
    {
        aSize = maSwapInfo.maPrefSize;
    }
    else
    {
        switch (meType)
        {
            case GraphicType::Bitmap:
            {
                if (maVectorGraphicData && maCachedBitmap.IsEmpty())
                {
                    if (!maExPrefSize.getWidth() || !maExPrefSize.getHeight())
                    {
                        // svg not yet buffered in maBitmapEx, return size derived from range
                        const basegfx::B2DRange& rRange = maVectorGraphicData->getRange();

#ifdef MACOSX
                        // tdf#157680 scale down estimated size of embedded PDF
                        // For some unknown reason, the embedded PDF sizes
                        // are 20x larger than expected. This only occurs on
                        // macOS so possibly there is some special conversion
                        // from MapUnit::MapPoint to MapUnit::MapTwip elsewhere
                        // in the code.
                        if (maVectorGraphicData->getType() == VectorGraphicDataType::Pdf)
                            aSize = Size(basegfx::fround(rRange.getWidth() / 20.0f), basegfx::fround(rRange.getHeight() / 20.0f));
                        else
#endif
                            aSize = Size(basegfx::fround<tools::Long>(rRange.getWidth()), basegfx::fround<tools::Long>(rRange.getHeight()));
                    }
                    else
                    {
                        aSize = maExPrefSize;
                    }
                }
                else if (mpAnimationContainer || maVectorGraphicData)
                {
                    aSize = maCachedBitmap.GetPrefSize();

                    if (!aSize.Width() || !aSize.Height())
                        aSize = maCachedBitmap.GetSizePixel();
                }
                else if (mpBitmapContainer)
                {
                    aSize = mpBitmapContainer->getPrefSize();
                }
            }
            break;

            case GraphicType::GdiMetafile:
            {
                aSize = maMetaFile.GetPrefSize();
            }
            break;

            case GraphicType::NONE:
            case GraphicType::Default:
                break;
        }
    }

    return aSize;
}

void ImpGraphic::setValuesForPrefSize(const Size& rPrefSize)
{
    switch (meType)
    {
        case GraphicType::Bitmap:
        {
            // used when importing a writer FlyFrame with SVG as graphic, added conversion
            // to allow setting the PrefSize at the BitmapEx to hold it
            if (maVectorGraphicData)
            {
                maExPrefSize = rPrefSize;
                maCachedBitmap.SetPrefSize(rPrefSize);
            }
            // #108077# Push through pref size to animation object,
            // will be lost on copy otherwise
            else if (mpAnimationContainer)
            {
                const_cast<BitmapEx&>(mpAnimationContainer->maAnimation.GetBitmapEx()).SetPrefSize(rPrefSize);
                maCachedBitmap.SetPrefSize(rPrefSize);
            }
            else if (mpBitmapContainer)
            {
                mpBitmapContainer->maBitmapEx.SetPrefSize(rPrefSize);
            }
        }
        break;

        case GraphicType::GdiMetafile:
        {
            if (isSupportedGraphic())
                maMetaFile.SetPrefSize(rPrefSize);
        }
        break;

        case GraphicType::NONE:
        case GraphicType::Default:
            break;
    }
}

void ImpGraphic::setPrefSize(const Size& rPrefSize)
{
    ensureAvailable();
    setValuesForPrefSize(rPrefSize);
}

MapMode ImpGraphic::getPrefMapMode() const
{
    MapMode aMapMode;

    if (isSwappedOut())
    {
        aMapMode = maSwapInfo.maPrefMapMode;
    }
    else
    {
        switch (meType)
        {
            case GraphicType::Bitmap:
            {
                if (maVectorGraphicData && maCachedBitmap.IsEmpty())
                {
                    // svg not yet buffered in maBitmapEx, return default PrefMapMode
                    aMapMode = MapMode(MapUnit::Map100thMM);
                }
                else if (mpBitmapContainer)
                {
                    aMapMode = mpBitmapContainer->getPrefMapMode();
                }
                else
                {
                    const Size aSize = maCachedBitmap.GetPrefSize();
                    if (aSize.Width() && aSize.Height())
                        aMapMode = maCachedBitmap.GetPrefMapMode();
                }
            }
            break;

            case GraphicType::GdiMetafile:
            {
                return maMetaFile.GetPrefMapMode();
            }
            break;

            case GraphicType::NONE:
            case GraphicType::Default:
                break;
        }
    }

    return aMapMode;
}

void ImpGraphic::setValuesForPrefMapMod(const MapMode& rPrefMapMode)
{
    switch (meType)
    {
        case GraphicType::Bitmap:
        {
            if (maVectorGraphicData)
            {
                // ignore for Vector Graphic Data. If this is really used (except the grfcache)
                // it can be extended by using maBitmapEx as buffer for updateBitmapFromVectorGraphic()
            }

            // #108077# Push through pref mapmode to animation object,
            // will be lost on copy otherwise
            else if (mpAnimationContainer)
            {
                const_cast<BitmapEx&>(mpAnimationContainer->maAnimation.GetBitmapEx()).SetPrefMapMode(rPrefMapMode);
                maCachedBitmap.SetPrefMapMode(rPrefMapMode);
            }
            else if (mpBitmapContainer)
            {
                mpBitmapContainer->maBitmapEx.SetPrefMapMode(rPrefMapMode);
            }
        }
        break;

        case GraphicType::GdiMetafile:
        {
            maMetaFile.SetPrefMapMode(rPrefMapMode);
        }
        break;

        case GraphicType::NONE:
        case GraphicType::Default:
            break;
    }
}

void ImpGraphic::setPrefMapMode(const MapMode& rPrefMapMode)
{
    ensureAvailable();
    setValuesForPrefMapMod(rPrefMapMode);
}

void ImpGraphic::ensureCurrentSizeInBytes()
{
    if (isAvailable())
        changeExisting(getSizeBytes());
    else
        changeExisting(0);
}

sal_uLong ImpGraphic::getSizeBytes() const
{
    if (mnSizeBytes > 0)
        return mnSizeBytes;

    if (mbPrepared)
        ensureAvailable();

    switch (meType)
    {
        case GraphicType::Bitmap:
        {
            if (maVectorGraphicData)
            {
                std::pair<VectorGraphicData::State, size_t> aPair(maVectorGraphicData->getSizeBytes());
                if (VectorGraphicData::State::UNPARSED == aPair.first)
                {
                    return aPair.second; // don't cache it until Vector Graphic Data is parsed
                }
                mnSizeBytes = aPair.second;
            }
            else
            {
                if (mpAnimationContainer)
                    mnSizeBytes = mpAnimationContainer->getSizeBytes();
                else if (mpBitmapContainer)
                    mnSizeBytes = mpBitmapContainer->getSizeBytes();
            }
        }
        break;

        case GraphicType::GdiMetafile:
        {
            mnSizeBytes = maMetaFile.GetSizeBytes();
        }
        break;

        case GraphicType::NONE:
        case GraphicType::Default:
            break;
    }

    return mnSizeBytes;
}

void ImpGraphic::draw(OutputDevice& rOutDev,
                      const Point& rDestPt, const Size& rDestSize) const
{
    ensureAvailable();

    if (isSwappedOut())
        return;

    switch (meType)
    {
        case GraphicType::Bitmap:
        {
            if (mpAnimationContainer)
            {
                mpAnimationContainer->maAnimation.Draw(rOutDev, rDestPt, rDestSize);
            }
            else
            {
                if (maVectorGraphicData)
                    updateBitmapFromVectorGraphic(rOutDev.LogicToPixel(rDestSize));

                getBitmapExRef().Draw(&rOutDev, rDestPt, rDestSize);
            }
        }
        break;

        case GraphicType::GdiMetafile:
        {
            const_cast<ImpGraphic*>(this)->maMetaFile.WindStart();
            const_cast<ImpGraphic*>(this)->maMetaFile.Play(rOutDev, rDestPt, rDestSize);
            const_cast<ImpGraphic*>(this)->maMetaFile.WindStart();
        }
        break;

        case GraphicType::Default:
        case GraphicType::NONE:
            break;
    }
}

void ImpGraphic::startAnimation(OutputDevice& rOutDev, const Point& rDestPt,
                                const Size& rDestSize, tools::Long nRendererId,
                                OutputDevice* pFirstFrameOutDev )
{
    ensureAvailable();

    if (isSupportedGraphic() && !isSwappedOut() && mpAnimationContainer)
        mpAnimationContainer->maAnimation.Start(rOutDev, rDestPt, rDestSize, nRendererId, pFirstFrameOutDev);
}

void ImpGraphic::stopAnimation( const OutputDevice* pOutDev, tools::Long nRendererId )
{
    ensureAvailable();

    if (isSupportedGraphic() && !isSwappedOut() && mpAnimationContainer)
        mpAnimationContainer->maAnimation.Stop( pOutDev, nRendererId );
}

void ImpGraphic::setAnimationNotifyHdl( const Link<Animation*,void>& rLink )
{
    ensureAvailable();

    if (mpAnimationContainer)
        mpAnimationContainer->maAnimation.SetNotifyHdl( rLink );
}

Link<Animation*,void> ImpGraphic::getAnimationNotifyHdl() const
{
    Link<Animation*,void> aLink;

    ensureAvailable();

    if (mpAnimationContainer)
        aLink = mpAnimationContainer->maAnimation.GetNotifyHdl();

    return aLink;
}

sal_uInt32 ImpGraphic::getAnimationLoopCount() const
{
    if (mbSwapOut)
        return maSwapInfo.mnAnimationLoopCount;

    return mpAnimationContainer ? mpAnimationContainer->maAnimation.GetLoopCount() : 0;
}

bool ImpGraphic::swapInContent(SvStream& rStream)
{
    bool bRet = false;

    sal_uInt32 nId;
    sal_Int32 nType;
    sal_Int32 nLength;

    rStream.ReadUInt32(nId);

    // check version
    if (SWAP_FORMAT_ID != nId)
    {
        SAL_WARN("vcl", "Incompatible swap file!");
        return false;
    }

    rStream.ReadInt32(nType);
    rStream.ReadInt32(nLength);

    meType = static_cast<GraphicType>(nType);

    if (meType == GraphicType::NONE  || meType == GraphicType::Default)
    {
        return true;
    }
    else
    {
        bRet = swapInGraphic(rStream);
    }

    return bRet;
}

bool ImpGraphic::swapOutGraphic(SvStream& rStream)
{
    if (rStream.GetError())
        return false;

    ensureAvailable();

    if (isSwappedOut())
    {
        rStream.SetError(SVSTREAM_GENERALERROR);
        return false;
    }

    switch (meType)
    {
        case GraphicType::GdiMetafile:
        {
            if(!rStream.GetError())
            {
                SvmWriter aWriter(rStream);
                aWriter.Write(maMetaFile);
            }
        }
        break;

        case GraphicType::Bitmap:
        {
            if (maVectorGraphicData)
            {
                rStream.WriteInt32(sal_Int32(GraphicContentType::Vector));
                // stream out Vector Graphic defining data (length, byte array and evtl. path)
                // this is used e.g. in swapping out graphic data and in transporting it over UNO API
                // as sequence of bytes, but AFAIK not written anywhere to any kind of file, so it should be
                // no problem to extend it; only used at runtime
                switch (maVectorGraphicData->getType())
                {
                    case VectorGraphicDataType::Wmf:
                    {
                        rStream.WriteUInt32(constWmfMagic);
                        break;
                    }
                    case VectorGraphicDataType::Emf:
                    {
                        rStream.WriteUInt32(constEmfMagic);
                        break;
                    }
                    case VectorGraphicDataType::Svg:
                    {
                        rStream.WriteUInt32(constSvgMagic);
                        break;
                    }
                    case VectorGraphicDataType::Pdf:
                    {
                        rStream.WriteUInt32(constPdfMagic);
                        break;
                    }
                }

                rStream.WriteUInt32(maVectorGraphicData->getBinaryDataContainer().getSize());
                maVectorGraphicData->getBinaryDataContainer().writeToStream(rStream);
            }
            else if (mpAnimationContainer)
            {
                rStream.WriteInt32(sal_Int32(GraphicContentType::Animation));
                WriteAnimation(rStream, mpAnimationContainer->maAnimation);
            }
            else if (mpBitmapContainer)
            {
                rStream.WriteInt32(sal_Int32(GraphicContentType::Bitmap));
                WriteDIBBitmapEx(mpBitmapContainer->maBitmapEx, rStream);
            }
        }
        break;

        case GraphicType::NONE:
        case GraphicType::Default:
            break;
    }

    if (mpGfxLink)
        mpGfxLink->getDataContainer().swapOut();

    return true;
}

bool ImpGraphic::swapOutContent(SvStream& rStream)
{
    ensureAvailable();

    bool bRet = false;

    if (meType == GraphicType::NONE || meType == GraphicType::Default || isSwappedOut())
        return false;

    sal_uLong nDataFieldPos;

    // Write the SWAP ID
    rStream.WriteUInt32(SWAP_FORMAT_ID);

    rStream.WriteInt32(static_cast<sal_Int32>(meType));

    // data size is updated later
    nDataFieldPos = rStream.Tell();
    rStream.WriteInt32(0);

    // write data block
    const sal_uInt64 nDataStart = rStream.Tell();

    swapOutGraphic(rStream);

    if (!rStream.GetError())
    {
        // Write the written length th the header
        const sal_uInt64 nCurrentPosition = rStream.Tell();
        rStream.Seek(nDataFieldPos);
        rStream.WriteInt32(nCurrentPosition - nDataStart);
        rStream.Seek(nCurrentPosition);
        bRet = true;
    }

    return bRet;
}

bool ImpGraphic::swapOut()
{
    if (isSwappedOut())
        return false;

    bool bResult = false;

    // We have GfxLink so we have the source available
    if (mpGfxLink && mpGfxLink->IsNative())
    {
        createSwapInfo();

        clearGraphics();

        // reset the swap file
        mpSwapFile.reset();

        mpGfxLink->getDataContainer().swapOut();

        // mark as swapped out
        mbSwapOut = true;

        bResult = true;
    }
    else
    {
        // Create a swap file
        auto pSwapFile = o3tl::make_shared<ImpSwapFile>(getOriginURL());

        // Open a stream to write the swap file to
        {
            SvStream* pOutputStream = pSwapFile->getStream();

            if (!pOutputStream)
                return false;

            // Write to stream
            pOutputStream->SetVersion(SOFFICE_FILEFORMAT_50);
            pOutputStream->SetCompressMode(SvStreamCompressFlags::NATIVE);
            pOutputStream->SetBufferSize(GRAPHIC_STREAMBUFSIZE);

            if (!pOutputStream->GetError() && swapOutContent(*pOutputStream))
            {
                pOutputStream->FlushBuffer();
                bResult = !pOutputStream->GetError();
            }
        }

        // Check if writing was successful
        if (bResult)
        {
            // We have swapped out, so can clean memory and prepare swap info
            createSwapInfo();
            clearGraphics();

            mpSwapFile = std::move(pSwapFile);
            mbSwapOut = true;
        }
    }

    if (bResult)
    {
        // Signal to manager that we have swapped out
        swappedOut(0);
    }

    return bResult;
}

bool ImpGraphic::ensureAvailable() const
{
    bool bResult = true;

    if (isSwappedOut())
    {
        auto pThis = const_cast<ImpGraphic*>(this);
        pThis->registerIntoManager();

        bResult = pThis->swapIn();
    }

    resetLastUsed();
    return bResult;
}

void ImpGraphic::updateFromLoadedGraphic(const ImpGraphic* pGraphic)
{
    if (mbPrepared)
    {
        GraphicExternalLink aLink = maGraphicExternalLink;
        Size aPrefSize = maSwapInfo.maPrefSize;
        MapMode aPrefMapMode = maSwapInfo.maPrefMapMode;
        *this = *pGraphic;
        if (aPrefSize.getWidth() && aPrefSize.getHeight() && aPrefMapMode == getPrefMapMode())
        {
            // Use custom preferred size if it was set when the graphic was still unloaded.
            // Only set the size in case the unloaded and loaded unit matches.
            setPrefSize(aPrefSize);
        }
        maGraphicExternalLink = std::move(aLink);
    }
    else
    {
        // Move over only graphic content
        mpAnimationContainer.reset();

        if (pGraphic->mpAnimationContainer)
        {
            mpAnimationContainer = std::make_shared<AnimationContainer>(*pGraphic->mpAnimationContainer);
            maCachedBitmap = mpAnimationContainer->maAnimation.GetBitmapEx();
        }
        else if (pGraphic->mpBitmapContainer)
        {
            mpBitmapContainer = pGraphic->mpBitmapContainer;
        }
        else
        {
            maCachedBitmap = pGraphic->maCachedBitmap;
        }

        maMetaFile = pGraphic->maMetaFile;
        maVectorGraphicData = pGraphic->maVectorGraphicData;

        // Set to 0, to force recalculation
        mnSizeBytes = 0;
        mnChecksum = 0;

        restoreFromSwapInfo();

        mbSwapOut = false;
    }
}

void ImpGraphic::dumpState(rtl::OStringBuffer &rState)
{
    if (meType == GraphicType::NONE && mnSizeBytes == 0)
        return; // uninteresting.

    rState.append("\n\t");

    if (mbSwapOut)
        rState.append("swapped\t");
    else
        rState.append("loaded\t");

    rState.append(static_cast<sal_Int32>(meType));
    rState.append("\tsize:\t");
    rState.append(static_cast<sal_Int64>(mnSizeBytes));
    rState.append("\tgfxl:\t");
    rState.append(static_cast<sal_Int64>(mpGfxLink ? mpGfxLink->getSizeBytes() : -1));
    rState.append("\t");
    rState.append(static_cast<sal_Int32>(maSwapInfo.maSizePixel.Width()));
    rState.append("x");
    rState.append(static_cast<sal_Int32>(maSwapInfo.maSizePixel.Height()));
    rState.append("\t");
    rState.append(static_cast<sal_Int32>(maExPrefSize.Width()));
    rState.append("x");
    rState.append(static_cast<sal_Int32>(maExPrefSize.Height()));
}

void ImpGraphic::restoreFromSwapInfo()
{
    setValuesForPrefMapMod(maSwapInfo.maPrefMapMode);
    setValuesForPrefSize(maSwapInfo.maPrefSize);

    if (maVectorGraphicData)
    {
        maVectorGraphicData->setPageIndex(maSwapInfo.mnPageIndex);
    }
}

namespace
{

std::optional<VectorGraphicDataType> lclConvertToVectorGraphicType(GfxLink const & rLink)
{
    switch(rLink.GetType())
    {
        case GfxLinkType::NativePdf:
            return VectorGraphicDataType::Pdf;

        case GfxLinkType::NativeWmf:
            if (rLink.IsEMF())
                return VectorGraphicDataType::Emf;
            else
                return VectorGraphicDataType::Wmf;

        case GfxLinkType::NativeSvg:
            return VectorGraphicDataType::Svg;

        default:
            break;
    }
    return std::optional<VectorGraphicDataType>();
}

} // end namespace

bool ImpGraphic::swapIn()
{
    if (!isSwappedOut())
        return false;

    bool bReturn = false;

    if (mbPrepared)
    {
        Graphic aGraphic;
        if (!mpGfxLink->LoadNative(aGraphic, getPageNumber()))
            return false;

        updateFromLoadedGraphic(aGraphic.ImplGetImpGraphic());

        resetLastUsed();
        bReturn = true;
    }
    else if (mpGfxLink && mpGfxLink->IsNative())
    {
        std::optional<VectorGraphicDataType> oType = lclConvertToVectorGraphicType(*mpGfxLink);
        if (oType)
        {
            maVectorGraphicData = vcl::loadVectorGraphic(mpGfxLink->getDataContainer(), *oType);

            // Set to 0, to force recalculation
            mnSizeBytes = 0;
            mnChecksum = 0;

            restoreFromSwapInfo();

            mbSwapOut = false;
        }
        else
        {
            Graphic aGraphic;
            if (!mpGfxLink->LoadNative(aGraphic))
                return false;

            ImpGraphic* pImpGraphic = aGraphic.ImplGetImpGraphic();
            if (meType != pImpGraphic->meType)
                return false;

            updateFromLoadedGraphic(pImpGraphic);
        }

        resetLastUsed();
        bReturn = true;
    }
    else
    {
        SvStream* pStream = nullptr;

        if (mpSwapFile)
            pStream = mpSwapFile->getStream();

        if (pStream)
        {
            pStream->SetVersion(SOFFICE_FILEFORMAT_50);
            pStream->SetCompressMode(SvStreamCompressFlags::NATIVE);
            pStream->SetBufferSize(GRAPHIC_STREAMBUFSIZE);
            pStream->Seek(STREAM_SEEK_TO_BEGIN);

            bReturn = swapInFromStream(*pStream);

            restoreFromSwapInfo();

            setOriginURL(mpSwapFile->getOriginURL());

            mpSwapFile.reset();
        }
    }

    if (bReturn)
    {
        swappedIn(getSizeBytes());
    }

    return bReturn;
}

bool ImpGraphic::swapInFromStream(SvStream& rStream)
{
    bool bRet = false;

    if (rStream.GetError())
        return false;

    clearGraphics();
    mnSizeBytes = 0;
    mnChecksum = 0;

    bRet = swapInContent(rStream);

    if (!bRet)
    {
        //throw away swapfile, etc.
        clear();
    }

    mbSwapOut = false;

    return bRet;
}

bool ImpGraphic::swapInGraphic(SvStream& rStream)
{
    bool bReturn = false;

    if (rStream.GetError())
        return bReturn;

    if (meType == GraphicType::Bitmap)
    {
        sal_Int32 nContentType = -1;
        rStream.ReadInt32(nContentType);
        if (nContentType < 0)
            return false;

        auto eContentType = static_cast<GraphicContentType>(nContentType);

        switch (eContentType)
        {
            case GraphicContentType::Bitmap:
            {
                BitmapEx aBitmapEx;
                ReadDIBBitmapEx(aBitmapEx, rStream);
                if (!rStream.GetError())
                {
                    mpBitmapContainer = std::make_shared<BitmapContainer>(aBitmapEx);
                    bReturn = true;
                }
            }
            break;

            case GraphicContentType::Animation:
            {
                Animation aAnimation;
                ReadAnimation(rStream, aAnimation);
                if (!rStream.GetError())
                {
                    mpAnimationContainer = std::make_shared<AnimationContainer>(aAnimation);
                    maCachedBitmap = mpAnimationContainer->maAnimation.GetBitmapEx();
                    bReturn = true;
                }
            }
            break;

            case GraphicContentType::Vector:
            {
                // try to stream in Svg defining data (length, byte array and evtl. path)
                // See below (operator<<) for more information
                sal_uInt32 nMagic;
                rStream.ReadUInt32(nMagic);

                if (constSvgMagic == nMagic || constWmfMagic == nMagic || constEmfMagic == nMagic || constPdfMagic == nMagic)
                {
                    sal_uInt32 nVectorGraphicDataSize(0);
                    rStream.ReadUInt32(nVectorGraphicDataSize);

                    if (nVectorGraphicDataSize)
                    {
                        BinaryDataContainer aDataContainer(rStream, nVectorGraphicDataSize);

                        if (rStream.GetError())
                            return false;

                        VectorGraphicDataType aDataType;

                        switch (nMagic)
                        {
                            case constSvgMagic:
                                aDataType = VectorGraphicDataType::Svg;
                                break;
                            case constWmfMagic:
                                aDataType = VectorGraphicDataType::Wmf;
                                break;
                            case constEmfMagic:
                                aDataType = VectorGraphicDataType::Emf;
                                break;
                            case constPdfMagic:
                                aDataType = VectorGraphicDataType::Pdf;
                                break;
                            default:
                                return false;
                        }

                        auto aVectorGraphicDataPtr = std::make_shared<VectorGraphicData>(aDataContainer, aDataType);

                        if (!rStream.GetError())
                        {
                            maVectorGraphicData = std::move(aVectorGraphicDataPtr);
                            bReturn = true;
                        }
                    }
                }
            }
            break;
        }
    }
    else if (meType == GraphicType::GdiMetafile)
    {
        GDIMetaFile aMetaFile;
        SvmReader aReader(rStream);
        aReader.Read(aMetaFile);
        if (!rStream.GetError())
        {
            maMetaFile = aMetaFile;
            bReturn = true;
        }
    }
    return bReturn;
}

void ImpGraphic::setGfxLink(const std::shared_ptr<GfxLink>& rGfxLink)
{
    ensureAvailable();

    mpGfxLink = rGfxLink;
}

const std::shared_ptr<GfxLink> & ImpGraphic::getSharedGfxLink() const
{
    return mpGfxLink;
}

GfxLink ImpGraphic::getGfxLink() const
{
    ensureAvailable();

    return( mpGfxLink ? *mpGfxLink : GfxLink() );
}

bool ImpGraphic::isGfxLink() const
{
    return ( bool(mpGfxLink) );
}

BitmapChecksum ImpGraphic::getChecksum() const
{
    if (mnChecksum != 0)
        return mnChecksum;

    ensureAvailable();

    switch (meType)
    {
        case GraphicType::NONE:
        case GraphicType::Default:
            break;

        case GraphicType::Bitmap:
        {
            if (maVectorGraphicData)
                mnChecksum = maVectorGraphicData->GetChecksum();
            else if (mpAnimationContainer)
                mnChecksum = mpAnimationContainer->maAnimation.GetChecksum();
            else if (mpBitmapContainer)
                mnChecksum = mpBitmapContainer->maBitmapEx.GetChecksum();
        }
        break;

        case GraphicType::GdiMetafile:
        {
            mnChecksum = SvmWriter::GetChecksum(maMetaFile);
        }
        break;
    }
    return mnChecksum;
}

sal_Int32 ImpGraphic::getPageNumber() const
{
    if (isSwappedOut())
        return maSwapInfo.mnPageIndex;

    if (maVectorGraphicData)
        return maVectorGraphicData->getPageIndex();
    return -1;
}

bool ImpGraphic::canReduceMemory() const
{
    return !isSwappedOut();
}

bool ImpGraphic::reduceMemory()
{
    return swapOut();
}

std::chrono::high_resolution_clock::time_point ImpGraphic::getLastUsed() const
{
    return maLastUsed;
}

void ImpGraphic::resetLastUsed() const
{
    maLastUsed = std::chrono::high_resolution_clock::now();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
