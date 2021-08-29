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

#include <SwapFile.hxx>

#include <comphelper/fileformat.h>
#include <o3tl/make_shared.hxx>
#include <tools/fract.hxx>
#include <tools/vcompat.hxx>
#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/tempfile.hxx>
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

class ImpSwapFile : public vcl::SwapFile
{
private:
    OUString maOriginURL;

public:
    ImpSwapFile(INetURLObject const & rSwapURL, OUString const & rOriginURL)
        : SwapFile(rSwapURL)
        , maOriginURL(rOriginURL)
    {
    }

    OUString const & getOriginURL() const { return maOriginURL; }
};

OUString ImpGraphic::getSwapFileURL() const
{
    if (mpSwapFile)
        return mpSwapFile->getSwapURL().GetMainURL(INetURLObject::DecodeMechanism::NONE);
    return OUString();
}

ImpGraphic::ImpGraphic() :
        meType          ( GraphicType::NONE ),
        mnSizeBytes     ( 0 ),
        mbSwapOut       ( false ),
        mbDummyContext  ( false ),
        maLastUsed (std::chrono::high_resolution_clock::now()),
        mbPrepared      ( false )
{
}

ImpGraphic::ImpGraphic(const ImpGraphic& rImpGraphic)
    : maMetaFile(rImpGraphic.maMetaFile)
    , maBitmapEx(rImpGraphic.maBitmapEx)
    , maSwapInfo(rImpGraphic.maSwapInfo)
    , mpContext(rImpGraphic.mpContext)
    , mpSwapFile(rImpGraphic.mpSwapFile)
    , mpGfxLink(rImpGraphic.mpGfxLink)
    , meType(rImpGraphic.meType)
    , mnSizeBytes(rImpGraphic.mnSizeBytes)
    , mbSwapOut(rImpGraphic.mbSwapOut)
    , mbDummyContext(rImpGraphic.mbDummyContext)
    , maVectorGraphicData(rImpGraphic.maVectorGraphicData)
    , maGraphicExternalLink(rImpGraphic.maGraphicExternalLink)
    , maLastUsed (std::chrono::high_resolution_clock::now())
    , mbPrepared (rImpGraphic.mbPrepared)
{
    if( rImpGraphic.mpAnimation )
    {
        mpAnimation = std::make_unique<Animation>( *rImpGraphic.mpAnimation );
        maBitmapEx = mpAnimation->GetBitmapEx();
    }
}

ImpGraphic::ImpGraphic(ImpGraphic&& rImpGraphic) noexcept
    : maMetaFile(std::move(rImpGraphic.maMetaFile))
    , maBitmapEx(std::move(rImpGraphic.maBitmapEx))
    , maSwapInfo(std::move(rImpGraphic.maSwapInfo))
    , mpAnimation(std::move(rImpGraphic.mpAnimation))
    , mpContext(std::move(rImpGraphic.mpContext))
    , mpSwapFile(std::move(rImpGraphic.mpSwapFile))
    , mpGfxLink(std::move(rImpGraphic.mpGfxLink))
    , meType(rImpGraphic.meType)
    , mnSizeBytes(rImpGraphic.mnSizeBytes)
    , mbSwapOut(rImpGraphic.mbSwapOut)
    , mbDummyContext(rImpGraphic.mbDummyContext)
    , maVectorGraphicData(std::move(rImpGraphic.maVectorGraphicData))
    , maGraphicExternalLink(rImpGraphic.maGraphicExternalLink)
    , maLastUsed (std::chrono::high_resolution_clock::now())
    , mbPrepared (rImpGraphic.mbPrepared)
{
    rImpGraphic.clear();
    rImpGraphic.mbDummyContext = false;
}

ImpGraphic::ImpGraphic(std::shared_ptr<GfxLink> const & rGfxLink, sal_Int32 nPageIndex)
    : mpGfxLink(rGfxLink)
    , meType(GraphicType::Bitmap)
    , mnSizeBytes(0)
    , mbSwapOut(true)
    , mbDummyContext(false)
    , maLastUsed (std::chrono::high_resolution_clock::now())
    , mbPrepared (false)
{
    maSwapInfo.mbIsTransparent = true;
    maSwapInfo.mbIsAlpha = true;
    maSwapInfo.mbIsEPS = false;
    maSwapInfo.mbIsAnimated = false;
    maSwapInfo.mnAnimationLoopCount = 0;
    maSwapInfo.mnPageIndex = nPageIndex;
}

ImpGraphic::ImpGraphic(GraphicExternalLink const & rGraphicExternalLink) :
        meType          ( GraphicType::Default ),
        mnSizeBytes     ( 0 ),
        mbSwapOut       ( false ),
        mbDummyContext  ( false ),
        maGraphicExternalLink(rGraphicExternalLink),
        maLastUsed (std::chrono::high_resolution_clock::now()),
        mbPrepared (false)
{
}

ImpGraphic::ImpGraphic( const BitmapEx& rBitmapEx ) :
        maBitmapEx            ( rBitmapEx ),
        meType          ( !rBitmapEx.IsEmpty() ? GraphicType::Bitmap : GraphicType::NONE ),
        mnSizeBytes     ( 0 ),
        mbSwapOut       ( false ),
        mbDummyContext  ( false ),
        maLastUsed (std::chrono::high_resolution_clock::now()),
        mbPrepared (false)
{
}

ImpGraphic::ImpGraphic(const std::shared_ptr<VectorGraphicData>& rVectorGraphicDataPtr)
:   meType( rVectorGraphicDataPtr ? GraphicType::Bitmap : GraphicType::NONE ),
    mnSizeBytes( 0 ),
    mbSwapOut( false ),
    mbDummyContext  ( false ),
    maVectorGraphicData(rVectorGraphicDataPtr),
    maLastUsed (std::chrono::high_resolution_clock::now()),
    mbPrepared (false)
{
}

ImpGraphic::ImpGraphic( const Animation& rAnimation ) :
        maBitmapEx      ( rAnimation.GetBitmapEx() ),
        mpAnimation     ( std::make_unique<Animation>( rAnimation ) ),
        meType          ( GraphicType::Bitmap ),
        mnSizeBytes     ( 0 ),
        mbSwapOut       ( false ),
        mbDummyContext  ( false ),
        maLastUsed (std::chrono::high_resolution_clock::now()),
        mbPrepared (false)
{
}

ImpGraphic::ImpGraphic( const GDIMetaFile& rMtf ) :
        maMetaFile      ( rMtf ),
        meType          ( GraphicType::GdiMetafile ),
        mnSizeBytes     ( 0 ),
        mbSwapOut       ( false ),
        mbDummyContext  ( false ),
        maLastUsed (std::chrono::high_resolution_clock::now()),
        mbPrepared (false)
{
}

ImpGraphic::~ImpGraphic()
{
    vcl::graphic::Manager::get().unregisterGraphic(this);
}

ImpGraphic& ImpGraphic::operator=( const ImpGraphic& rImpGraphic )
{
    if( &rImpGraphic != this )
    {
        sal_Int64 aOldSizeBytes = mnSizeBytes;

        maMetaFile = rImpGraphic.maMetaFile;
        meType = rImpGraphic.meType;
        mnSizeBytes = rImpGraphic.mnSizeBytes;

        maSwapInfo = rImpGraphic.maSwapInfo;
        mpContext = rImpGraphic.mpContext;
        mbDummyContext = rImpGraphic.mbDummyContext;
        maGraphicExternalLink = rImpGraphic.maGraphicExternalLink;

        mpAnimation.reset();

        if ( rImpGraphic.mpAnimation )
        {
            mpAnimation = std::make_unique<Animation>( *rImpGraphic.mpAnimation );
            maBitmapEx = mpAnimation->GetBitmapEx();
        }
        else
        {
            maBitmapEx = rImpGraphic.maBitmapEx;
        }

        mbSwapOut = rImpGraphic.mbSwapOut;
        mpSwapFile = rImpGraphic.mpSwapFile;
        mbPrepared = rImpGraphic.mbPrepared;

        mpGfxLink = rImpGraphic.mpGfxLink;

        maVectorGraphicData = rImpGraphic.maVectorGraphicData;
        maLastUsed = std::chrono::high_resolution_clock::now();

        vcl::graphic::Manager::get().changeExisting(this, aOldSizeBytes);
    }

    return *this;
}

ImpGraphic& ImpGraphic::operator=(ImpGraphic&& rImpGraphic)
{
    sal_Int64 aOldSizeBytes = mnSizeBytes;

    maMetaFile = std::move(rImpGraphic.maMetaFile);
    meType = rImpGraphic.meType;
    mnSizeBytes = rImpGraphic.mnSizeBytes;
    maSwapInfo = std::move(rImpGraphic.maSwapInfo);
    mpContext = std::move(rImpGraphic.mpContext);
    mbDummyContext = rImpGraphic.mbDummyContext;
    mpAnimation = std::move(rImpGraphic.mpAnimation);
    maBitmapEx = std::move(rImpGraphic.maBitmapEx);
    mbSwapOut = rImpGraphic.mbSwapOut;
    mpSwapFile = std::move(rImpGraphic.mpSwapFile);
    mpGfxLink = std::move(rImpGraphic.mpGfxLink);
    maVectorGraphicData = std::move(rImpGraphic.maVectorGraphicData);
    maGraphicExternalLink = rImpGraphic.maGraphicExternalLink;
    mbPrepared = rImpGraphic.mbPrepared;

    rImpGraphic.clear();
    rImpGraphic.mbDummyContext = false;
    maLastUsed = std::chrono::high_resolution_clock::now();

    vcl::graphic::Manager::get().changeExisting(this, aOldSizeBytes);

    return *this;
}

bool ImpGraphic::operator==( const ImpGraphic& rImpGraphic ) const
{
    bool bRet = false;

    if( this == &rImpGraphic )
        bRet = true;
    else if (mbPrepared && rImpGraphic.mbPrepared)
    {
        bRet = (*mpGfxLink == *rImpGraphic.mpGfxLink);
    }
    else if (isAvailable() && rImpGraphic.isAvailable())
    {
        switch( meType )
        {
            case GraphicType::NONE:
                bRet = true;
            break;

            case GraphicType::GdiMetafile:
            {
                if( rImpGraphic.maMetaFile == maMetaFile )
                    bRet = true;
            }
            break;

            case GraphicType::Bitmap:
            {
                if(maVectorGraphicData)
                {
                    if(maVectorGraphicData == rImpGraphic.maVectorGraphicData)
                    {
                        // equal instances
                        bRet = true;
                    }
                    else if(rImpGraphic.maVectorGraphicData)
                    {
                        // equal content
                        bRet = (*maVectorGraphicData) == (*rImpGraphic.maVectorGraphicData);
                    }
                }
                else if( mpAnimation )
                {
                    if( rImpGraphic.mpAnimation && ( *rImpGraphic.mpAnimation == *mpAnimation ) )
                        bRet = true;
                }
                else if( !rImpGraphic.mpAnimation && ( rImpGraphic.maBitmapEx == maBitmapEx ) )
                {
                    bRet = true;
                }
            }
            break;

            case GraphicType::Default:
            break;
        }
    }

    return bRet;
}

const std::shared_ptr<VectorGraphicData>& ImpGraphic::getVectorGraphicData() const
{
    ensureAvailable();

    return maVectorGraphicData;
}

void ImpGraphic::createSwapInfo()
{
    if (isSwappedOut())
        return;

    if (!maBitmapEx.IsEmpty())
        maSwapInfo.maSizePixel = maBitmapEx.GetSizePixel();
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
    maBitmapEx.Clear();
    maMetaFile.Clear();
    mpAnimation.reset();
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
    sal_Int64 nOldSize = mnSizeBytes;
    mnSizeBytes = 0;
    vcl::graphic::Manager::get().changeExisting(this, nOldSize);
    maGraphicExternalLink.msURL.clear();
}

void ImpGraphic::setDefaultType()
{
    clear();
    meType = GraphicType::Default;
}

bool ImpGraphic::isSupportedGraphic() const
{
    return( meType != GraphicType::NONE );
}

bool ImpGraphic::isTransparent() const
{
    bool bRet(true);

    if (mbSwapOut)
    {
        bRet = maSwapInfo.mbIsTransparent;
    }
    else if (meType == GraphicType::Bitmap && !maVectorGraphicData)
    {
        bRet = mpAnimation ? mpAnimation->IsTransparent() : maBitmapEx.IsAlpha();
    }

    return bRet;
}

bool ImpGraphic::isAlpha() const
{
    bool bRet(false);

    if (mbSwapOut)
    {
        bRet = maSwapInfo.mbIsAlpha;
    }
    else if (maVectorGraphicData)
    {
        bRet = true;
    }
    else if (meType == GraphicType::Bitmap)
    {
        bRet = (nullptr == mpAnimation && maBitmapEx.IsAlpha());
    }

    return bRet;
}

bool ImpGraphic::isAnimated() const
{
    return mbSwapOut ? maSwapInfo.mbIsAnimated : mpAnimation != nullptr;
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

BitmapEx ImpGraphic::getVectorGraphicReplacement() const
{
    BitmapEx aRet = maVectorGraphicData->getReplacement();

    if (maExPrefSize.getWidth() && maExPrefSize.getHeight())
    {
        aRet.SetPrefSize(maExPrefSize);
    }

    return aRet;
}

Bitmap ImpGraphic::getBitmap(const GraphicConversionParameters& rParameters) const
{
    Bitmap aRetBmp;

    ensureAvailable();

    if( meType == GraphicType::Bitmap )
    {
        if(maVectorGraphicData && maBitmapEx.IsEmpty())
        {
            // use maBitmapEx as local buffer for rendered svg
            const_cast< ImpGraphic* >(this)->maBitmapEx = getVectorGraphicReplacement();
        }

        const BitmapEx& rRetBmpEx = ( mpAnimation ? mpAnimation->GetBitmapEx() : maBitmapEx );

        aRetBmp = rRetBmpEx.GetBitmap( COL_WHITE );

        if(rParameters.getSizePixel().Width() || rParameters.getSizePixel().Height())
            aRetBmp.Scale(rParameters.getSizePixel());
    }
    else if( ( meType != GraphicType::Default ) && isSupportedGraphic() )
    {
        if(maBitmapEx.IsEmpty())
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
                    aDrawSize.setWidth(basegfx::fround(GRAPHIC_MTFTOBMP_MAXEXT * fWH));
                    aDrawSize.setHeight(GRAPHIC_MTFTOBMP_MAXEXT);
                }
                else
                {
                    aDrawSize.setWidth(GRAPHIC_MTFTOBMP_MAXEXT);
                    aDrawSize.setHeight(basegfx::fround(GRAPHIC_MTFTOBMP_MAXEXT / fWH));
                }
            }

            // calculate pixel size. Normally, it's the same as aDrawSize, but may
            // need to be extended when hairlines are on the right or bottom edge
            Size aPixelSize(aDrawSize);

            if(GraphicType::GdiMetafile == getType())
            {
                // get hairline and full bound rect
                tools::Rectangle aHairlineRect;
                const tools::Rectangle aRect(maMetaFile.GetBoundRect(*aVDev, &aHairlineRect));

                if(!aRect.IsEmpty() && !aHairlineRect.IsEmpty())
                {
                    // expand if needed to allow bottom and right hairlines to be added
                    if(aRect.Right() == aHairlineRect.Right())
                    {
                        aPixelSize.setWidth(aPixelSize.getWidth() + 1);
                    }

                    if(aRect.Bottom() == aHairlineRect.Bottom())
                    {
                        aPixelSize.setHeight(aPixelSize.getHeight() + 1);
                    }
                }
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
                const_cast< ImpGraphic* >(this)->maBitmapEx = aVDev->GetBitmapEx( Point(), aVDev->GetOutputSizePixel() );
            }
        }

        aRetBmp = maBitmapEx.GetBitmap();
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
    BitmapEx aRetBmpEx;

    ensureAvailable();

    if( meType == GraphicType::Bitmap )
    {
        if(maVectorGraphicData && maBitmapEx.IsEmpty())
        {
            // use maBitmapEx as local buffer for rendered svg
            const_cast< ImpGraphic* >(this)->maBitmapEx = getVectorGraphicReplacement();
        }

        aRetBmpEx = ( mpAnimation ? mpAnimation->GetBitmapEx() : maBitmapEx );

        if(rParameters.getSizePixel().Width() || rParameters.getSizePixel().Height())
        {
            aRetBmpEx.Scale(
                rParameters.getSizePixel(),
                BmpScaleFlag::Fast);
        }
    }
    else if( ( meType != GraphicType::Default ) && isSupportedGraphic() )
    {
        if(maBitmapEx.IsEmpty())
        {
            const ImpGraphic aMonoMask( maMetaFile.GetMonochromeMtf( COL_BLACK ) );

            // use maBitmapEx as local buffer for rendered metafile
            const_cast< ImpGraphic* >(this)->maBitmapEx = BitmapEx(getBitmap(rParameters), aMonoMask.getBitmap(rParameters));
        }

        aRetBmpEx = maBitmapEx;
    }

    return aRetBmpEx;
}

Animation ImpGraphic::getAnimation() const
{
    Animation aAnimation;

    ensureAvailable();
    if( mpAnimation )
        aAnimation = *mpAnimation;

    return aAnimation;
}

const BitmapEx& ImpGraphic::getBitmapExRef() const
{
    ensureAvailable();
    return maBitmapEx;
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
            const css::uno::Reference< css::graphic::XPrimitive2D > xReference(aSequence[0]);
            const MetafileAccessor* pMetafileAccessor = dynamic_cast< const MetafileAccessor* >(xReference.get());

            if (pMetafileAccessor)
            {
                // it is a MetafileAccessor implementation, get Metafile
                pMetafileAccessor->accessMetafile(const_cast< ImpGraphic* >(this)->maMetaFile);
            }
        }
    }

    if (GraphicType::Bitmap == meType && !maMetaFile.GetActionSize())
    {
        // #i119735#
        // Use the local maMetaFile as container for a metafile-representation
        // of the bitmap graphic. This will be done only once, thus be buffered.
        // I checked all usages of maMetaFile, it is only used when type is not
        // GraphicType::Bitmap. In operator= it will get copied, thus buffering will
        // survive copying (change this if not wanted)
        ImpGraphic* pThat = const_cast< ImpGraphic* >(this);

        if(maVectorGraphicData && maBitmapEx.IsEmpty())
        {
            // use maBitmapEx as local buffer for rendered svg
            pThat->maBitmapEx = getVectorGraphicReplacement();
        }

        // #123983# directly create a metafile with the same PrefSize and PrefMapMode
        // the bitmap has, this will be an always correct metafile
        if(maBitmapEx.IsAlpha())
        {
            pThat->maMetaFile.AddAction(new MetaBmpExScaleAction(Point(), maBitmapEx.GetPrefSize(), maBitmapEx));
        }
        else
        {
            pThat->maMetaFile.AddAction(new MetaBmpScaleAction(Point(), maBitmapEx.GetPrefSize(), maBitmapEx.GetBitmap()));
        }

        pThat->maMetaFile.Stop();
        pThat->maMetaFile.WindStart();
        pThat->maMetaFile.SetPrefSize(maBitmapEx.GetPrefSize());
        pThat->maMetaFile.SetPrefMapMode(maBitmapEx.GetPrefMapMode());
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
                if (maVectorGraphicData && maBitmapEx.IsEmpty())
                {
                    if (!maExPrefSize.getWidth() || !maExPrefSize.getHeight())
                    {
                        // svg not yet buffered in maBitmapEx, return size derived from range
                        const basegfx::B2DRange& rRange = maVectorGraphicData->getRange();

                        aSize = Size(basegfx::fround(rRange.getWidth()), basegfx::fround(rRange.getHeight()));
                    }
                    else
                    {
                        aSize = maExPrefSize;
                    }
                }
                else
                {
                    aSize = maBitmapEx.GetPrefSize();

                    if( !aSize.Width() || !aSize.Height() )
                    {
                        aSize = maBitmapEx.GetSizePixel();
                    }
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
            if (maVectorGraphicData && maBitmapEx.IsEmpty())
            {
                maExPrefSize = rPrefSize;
            }

            // #108077# Push through pref size to animation object,
            // will be lost on copy otherwise
            if (isAnimated())
            {
                const_cast< BitmapEx& >(mpAnimation->GetBitmapEx()).SetPrefSize(rPrefSize);
            }

            if (!maExPrefSize.getWidth() || !maExPrefSize.getHeight())
            {
                maBitmapEx.SetPrefSize(rPrefSize);
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
                if (maVectorGraphicData && maBitmapEx.IsEmpty())
                {
                    // svg not yet buffered in maBitmapEx, return default PrefMapMode
                    aMapMode = MapMode(MapUnit::Map100thMM);
                }
                else
                {
                    const Size aSize(maBitmapEx.GetPrefSize());

                    if (aSize.Width() && aSize.Height())
                        aMapMode = maBitmapEx.GetPrefMapMode();
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
                // it can be extended by using maBitmapEx as buffer for getVectorGraphicReplacement()
            }
            else
            {
                // #108077# Push through pref mapmode to animation object,
                // will be lost on copy otherwise
                if (isAnimated())
                {
                    const_cast<BitmapEx&>(mpAnimation->GetBitmapEx()).SetPrefMapMode(rPrefMapMode);
                }

                maBitmapEx.SetPrefMapMode(rPrefMapMode);
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

sal_uInt32 ImpGraphic::getSizeBytes() const
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
                mnSizeBytes = mpAnimation ? mpAnimation->GetSizeBytes() : maBitmapEx.GetSizeBytes();
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

void ImpGraphic::draw(OutputDevice& rOutDev, const Point& rDestPt) const
{
    ensureAvailable();

    if (isSwappedOut())
        return;

    switch (meType)
    {
        case GraphicType::Bitmap:
        {
            if (maVectorGraphicData && maBitmapEx.IsEmpty())
            {
                // use maBitmapEx as local buffer for rendered svg
                const_cast<ImpGraphic*>(this)->maBitmapEx = getVectorGraphicReplacement();
            }

            if (mpAnimation)
            {
                mpAnimation->Draw(rOutDev, rDestPt);
            }
            else
            {
                maBitmapEx.Draw(&rOutDev, rDestPt);
            }
        }
        break;

        case GraphicType::GdiMetafile:
        {
            draw(rOutDev, rDestPt, maMetaFile.GetPrefSize());
        }
        break;

        case GraphicType::Default:
        case GraphicType::NONE:
            break;
    }
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
            if (maVectorGraphicData && maBitmapEx.IsEmpty())
            {
                // use maBitmapEx as local buffer for rendered svg
                const_cast<ImpGraphic*>(this)->maBitmapEx = getVectorGraphicReplacement();
            }

            if (mpAnimation)
            {
                mpAnimation->Draw(rOutDev, rDestPt, rDestSize);
            }
            else
            {
                maBitmapEx.Draw(&rOutDev, rDestPt, rDestSize);
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
                                const Size& rDestSize, tools::Long nExtraData,
                                OutputDevice* pFirstFrameOutDev )
{
    ensureAvailable();

    if( isSupportedGraphic() && !isSwappedOut() && mpAnimation )
        mpAnimation->Start(rOutDev, rDestPt, rDestSize, nExtraData, pFirstFrameOutDev);
}

void ImpGraphic::stopAnimation( const OutputDevice* pOutDev, tools::Long nExtraData )
{
    ensureAvailable();

    if( isSupportedGraphic() && !isSwappedOut() && mpAnimation )
        mpAnimation->Stop( pOutDev, nExtraData );
}

void ImpGraphic::setAnimationNotifyHdl( const Link<Animation*,void>& rLink )
{
    ensureAvailable();

    if( mpAnimation )
        mpAnimation->SetNotifyHdl( rLink );
}

Link<Animation*,void> ImpGraphic::getAnimationNotifyHdl() const
{
    Link<Animation*,void> aLink;

    ensureAvailable();

    if( mpAnimation )
        aLink = mpAnimation->GetNotifyHdl();

    return aLink;
}

sal_uInt32 ImpGraphic::getAnimationLoopCount() const
{
    if (mbSwapOut)
        return maSwapInfo.mnAnimationLoopCount;

    return mpAnimation ? mpAnimation->GetLoopCount() : 0;
}

void ImpGraphic::setContext( const std::shared_ptr<GraphicReader>& pReader )
{
    mpContext = pReader;
    mbDummyContext = false;
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

                rStream.WriteBytes(
                    maVectorGraphicData->getBinaryDataContainer().getData(),
                    maVectorGraphicData->getBinaryDataContainer().getSize());
            }
            else if (isAnimated())
            {
                rStream.WriteInt32(sal_Int32(GraphicContentType::Animation));
                WriteAnimation(rStream, *mpAnimation);
            }
            else
            {
                rStream.WriteInt32(sal_Int32(GraphicContentType::Bitmap));
                WriteDIBBitmapEx(maBitmapEx, rStream);
            }
        }
        break;

        case GraphicType::NONE:
        case GraphicType::Default:
            break;
    }

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
    const sal_uLong nDataStart = rStream.Tell();

    swapOutGraphic(rStream);

    if (!rStream.GetError())
    {
        // Write the written length th the header
        const sal_uLong nCurrentPosition = rStream.Tell();
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

    sal_Int64 nByteSize = getSizeBytes();

    // We have GfxLink so we have the source available
    if (mpGfxLink && mpGfxLink->IsNative())
    {
        createSwapInfo();

        clearGraphics();

        // reset the swap file
        mpSwapFile.reset();

        // mark as swapped out
        mbSwapOut = true;

        bResult = true;
    }
    else
    {
        // Create a temp filename for the swap file
        utl::TempFile aTempFile;
        const INetURLObject aTempFileURL(aTempFile.GetURL());

        // Create a swap file
        auto pSwapFile = o3tl::make_shared<ImpSwapFile>(aTempFileURL, getOriginURL());

        // Open a stream to write the swap file to
        {
            std::unique_ptr<SvStream> xOutputStream = pSwapFile->openOutputStream();

            if (!xOutputStream)
                return false;

            // Write to stream
            xOutputStream->SetVersion(SOFFICE_FILEFORMAT_50);
            xOutputStream->SetCompressMode(SvStreamCompressFlags::NATIVE);
            xOutputStream->SetBufferSize(GRAPHIC_STREAMBUFSIZE);

            if (!xOutputStream->GetError() && swapOutContent(*xOutputStream))
            {
                xOutputStream->Flush();
                bResult = !xOutputStream->GetError();
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
        vcl::graphic::Manager::get().swappedOut(this, nByteSize);
    }

    return bResult;
}

bool ImpGraphic::ensureAvailable() const
{
    auto pThis = const_cast<ImpGraphic*>(this);

    bool bResult = true;

    if (isSwappedOut())
        bResult = pThis->swapIn();

    pThis->maLastUsed = std::chrono::high_resolution_clock::now();
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
        maGraphicExternalLink = aLink;
    }
    else
    {
        // Move over only graphic content
        mpAnimation.reset();
        if (pGraphic->mpAnimation)
        {
            mpAnimation = std::make_unique<Animation>(*pGraphic->mpAnimation);
            maBitmapEx = mpAnimation->GetBitmapEx();
        }
        else
        {
            maBitmapEx = pGraphic->maBitmapEx;
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
        if (!mpGfxLink->LoadNative(aGraphic))
            return false;

        updateFromLoadedGraphic(aGraphic.ImplGetImpGraphic());

        maLastUsed = std::chrono::high_resolution_clock::now();
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

        maLastUsed = std::chrono::high_resolution_clock::now();
        bReturn = true;
    }
    else
    {
        OUString aSwapURL;

        if (mpSwapFile)
            aSwapURL = mpSwapFile->getSwapURL().GetMainURL(INetURLObject::DecodeMechanism::NONE);

        if (!aSwapURL.isEmpty())
        {
            std::unique_ptr<SvStream> xStream;
            try
            {
                xStream = ::utl::UcbStreamHelper::CreateStream(aSwapURL, StreamMode::READWRITE | StreamMode::SHARE_DENYWRITE);
            }
            catch( const css::uno::Exception& )
            {
            }

            if (xStream)
            {
                xStream->SetVersion(SOFFICE_FILEFORMAT_50);
                xStream->SetCompressMode(SvStreamCompressFlags::NATIVE);
                xStream->SetBufferSize(GRAPHIC_STREAMBUFSIZE);

                bReturn = swapInFromStream(*xStream);

                xStream.reset();

                restoreFromSwapInfo();

                if (mpSwapFile)
                    setOriginURL(mpSwapFile->getOriginURL());

                mpSwapFile.reset();
            }
        }
    }

    if (bReturn)
    {
        vcl::graphic::Manager::get().swappedIn(this, getSizeBytes());
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
                    maBitmapEx = aBitmapEx;
                    bReturn = true;
                }
            }
            break;

            case GraphicContentType::Animation:
            {
                auto pAnimation = std::make_unique<Animation>();
                ReadAnimation(rStream, *pAnimation);
                if (!rStream.GetError())
                {
                    mpAnimation = std::move(pAnimation);
                    maBitmapEx = mpAnimation->GetBitmapEx();
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
                        auto rData = std::make_unique<std::vector<sal_uInt8>>(nVectorGraphicDataSize);
                        rStream.ReadBytes(rData->data(), nVectorGraphicDataSize);
                        BinaryDataContainer aDataContainer(std::move(rData));

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
                            maVectorGraphicData = aVectorGraphicDataPtr;
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
            else if (mpAnimation)
                mnChecksum = mpAnimation->GetChecksum();
            else
                mnChecksum = maBitmapEx.GetChecksum();
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
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
