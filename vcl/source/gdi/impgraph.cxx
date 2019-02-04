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
#include <comphelper/processfactory.hxx>
#include <tools/fract.hxx>
#include <tools/vcompat.hxx>
#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <tools/helpers.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/outdev.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/virdev.hxx>
#include <vcl/gfxlink.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/graph.hxx>
#include <vcl/metaact.hxx>
#include <impgraph.hxx>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/graphic/XPrimitive2D.hpp>
#include <vcl/dibtools.hxx>
#include <memory>
#include <vcl/gdimetafiletools.hxx>

#include <vcl/pdfread.hxx>

#define GRAPHIC_MTFTOBMP_MAXEXT     2048
#define GRAPHIC_STREAMBUFSIZE       8192UL

#define SYS_WINMETAFILE             0x00000003L
#define SYS_WNTMETAFILE             0x00000004L
#define SYS_OS2METAFILE             0x00000005L
#define SYS_MACMETAFILE             0x00000006L

#define GRAPHIC_FORMAT_50           COMPAT_FORMAT( 'G', 'R', 'F', '5' )
#define NATIVE_FORMAT_50            COMPAT_FORMAT( 'N', 'A', 'T', '5' )

const sal_uInt32 nPdfMagic((sal_uInt32('p') << 24) | (sal_uInt32('d') << 16) | (sal_uInt32('f') << 8) | sal_uInt32('0'));

using namespace com::sun::star;

struct ImpSwapFile
{
    INetURLObject aSwapURL;
    OUString maOriginURL;
    ~ImpSwapFile();
};

class ReaderData
{
public:
    Size    maPreviewSize;
};

GraphicReader::GraphicReader()
{
}

GraphicReader::~GraphicReader()
{
}

void GraphicReader::DisablePreviewMode()
{
    if( mpReaderData )
        mpReaderData->maPreviewSize = Size( 0, 0 );
}

void GraphicReader::SetPreviewSize( const Size& rSize )
{
    if( !mpReaderData )
        mpReaderData.reset( new ReaderData );
    mpReaderData->maPreviewSize = rSize;
}

Size GraphicReader::GetPreviewSize() const
{
    Size aSize( 0, 0 );
    if( mpReaderData )
        aSize = mpReaderData->maPreviewSize;
    return aSize;
}

GraphicID::GraphicID(ImpGraphic const & rGraphic)
{
    rGraphic.ensureAvailable();

    mnID1 = static_cast<sal_uLong>(rGraphic.ImplGetType()) << 28;
    mnID2 = mnID3 = mnID4 = 0;

    if (rGraphic.ImplGetType() == GraphicType::Bitmap)
    {
        if (rGraphic.getVectorGraphicData().get())
        {
            const VectorGraphicDataPtr& rVectorGraphicDataPtr = rGraphic.getVectorGraphicData();
            const basegfx::B2DRange& rRange = rVectorGraphicDataPtr->getRange();

            mnID1 |= rVectorGraphicDataPtr->getVectorGraphicDataArrayLength();
            mnID2 = basegfx::fround(rRange.getWidth());
            mnID3 = basegfx::fround(rRange.getHeight());
            mnID4 = vcl_get_checksum(0, rVectorGraphicDataPtr->getVectorGraphicDataArray().getConstArray(), rVectorGraphicDataPtr->getVectorGraphicDataArrayLength());
        }
        else if (rGraphic.ImplIsAnimated())
        {
            const Animation aAnimation(rGraphic.ImplGetAnimation());

            mnID1 |= ( aAnimation.Count() & 0x0fffffff );
            mnID2 = aAnimation.GetDisplaySizePixel().Width();
            mnID3 = aAnimation.GetDisplaySizePixel().Height();
            mnID4 = rGraphic.ImplGetChecksum();
        }
        else
        {
            const BitmapEx aBmpEx(rGraphic.ImplGetBitmapEx(GraphicConversionParameters()));

            mnID1 |= ( ( ( static_cast<sal_uLong>(aBmpEx.GetTransparentType()) << 8 ) | ( aBmpEx.IsAlpha() ? 1 : 0 ) ) & 0x0fffffff );
            mnID2 = aBmpEx.GetSizePixel().Width();
            mnID3 = aBmpEx.GetSizePixel().Height();
            mnID4 = rGraphic.ImplGetChecksum();
        }
    }
    else if (rGraphic.ImplGetType() == GraphicType::GdiMetafile)
    {
        const GDIMetaFile& rMtf = rGraphic.ImplGetGDIMetaFile();

        mnID1 |= ( rMtf.GetActionSize() & 0x0fffffff );
        mnID2 = rMtf.GetPrefSize().Width();
        mnID3 = rMtf.GetPrefSize().Height();
        mnID4 = rGraphic.ImplGetChecksum();
    }
}

OString GraphicID::getIDString() const
{
    static const char aHexData[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

    OStringBuffer aHexStr;
    sal_Int32 nShift, nIndex = 0;
    aHexStr.setLength(24 + (2 * BITMAP_CHECKSUM_SIZE));

    for( nShift = 28; nShift >= 0; nShift -= 4 )
        aHexStr[nIndex++] = aHexData[ ( mnID1 >> static_cast<sal_uInt32>(nShift) ) & 0xf ];

    for( nShift = 28; nShift >= 0; nShift -= 4 )
        aHexStr[nIndex++] = aHexData[ ( mnID2 >> static_cast<sal_uInt32>(nShift) ) & 0xf ];

    for( nShift = 28; nShift >= 0; nShift -= 4 )
        aHexStr[nIndex++] = aHexData[ ( mnID3 >> static_cast<sal_uInt32>(nShift) ) & 0xf ];

    for( nShift = ( 8 * BITMAP_CHECKSUM_SIZE ) - 4; nShift >= 0; nShift -= 4 )
        aHexStr[nIndex++] = aHexData[ ( mnID4 >> static_cast<sal_uInt32>(nShift) ) & 0xf ];

    return aHexStr.makeStringAndClear();
}

ImpGraphic::ImpGraphic() :
        meType          ( GraphicType::NONE ),
        mnSizeBytes     ( 0 ),
        mbSwapOut       ( false ),
        mbDummyContext  ( false ),
        maLastUsed (std::chrono::high_resolution_clock::now()),
        mbPrepared      ( false ),
        mnPageNumber(-1)
{
}

ImpGraphic::ImpGraphic(const ImpGraphic& rImpGraphic)
    : maMetaFile(rImpGraphic.maMetaFile)
    , maEx(rImpGraphic.maEx)
    , maSwapInfo(rImpGraphic.maSwapInfo)
    , mpContext(rImpGraphic.mpContext)
    , mpSwapFile(rImpGraphic.mpSwapFile)
    , mpGfxLink(rImpGraphic.mpGfxLink)
    , meType(rImpGraphic.meType)
    , mnSizeBytes(rImpGraphic.mnSizeBytes)
    , mbSwapOut(rImpGraphic.mbSwapOut)
    , mbDummyContext(rImpGraphic.mbDummyContext)
    , maVectorGraphicData(rImpGraphic.maVectorGraphicData)
    , mpPdfData(rImpGraphic.mpPdfData)
    , maGraphicExternalLink(rImpGraphic.maGraphicExternalLink)
    , maLastUsed (std::chrono::high_resolution_clock::now())
    , mbPrepared (rImpGraphic.mbPrepared)
    , mnPageNumber(rImpGraphic.mnPageNumber)
{
    if( rImpGraphic.mpAnimation )
    {
        mpAnimation = std::make_unique<Animation>( *rImpGraphic.mpAnimation );
        maEx = mpAnimation->GetBitmapEx();
    }
}

ImpGraphic::ImpGraphic(ImpGraphic&& rImpGraphic)
    : maMetaFile(std::move(rImpGraphic.maMetaFile))
    , maEx(std::move(rImpGraphic.maEx))
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
    , mpPdfData(std::move(rImpGraphic.mpPdfData))
    , maGraphicExternalLink(rImpGraphic.maGraphicExternalLink)
    , maLastUsed (std::chrono::high_resolution_clock::now())
    , mbPrepared (rImpGraphic.mbPrepared)
    , mnPageNumber(rImpGraphic.mnPageNumber)
{
    rImpGraphic.ImplClear();
    rImpGraphic.mbDummyContext = false;
}

ImpGraphic::ImpGraphic(GraphicExternalLink const & rGraphicExternalLink) :
        meType          ( GraphicType::Default ),
        mnSizeBytes     ( 0 ),
        mbSwapOut       ( false ),
        mbDummyContext  ( false ),
        maGraphicExternalLink(rGraphicExternalLink),
        maLastUsed (std::chrono::high_resolution_clock::now()),
        mbPrepared (false),
        mnPageNumber(-1)
{
}

ImpGraphic::ImpGraphic( const Bitmap& rBitmap ) :
        maEx            ( rBitmap ),
        meType          ( !rBitmap.IsEmpty() ? GraphicType::Bitmap : GraphicType::NONE ),
        mnSizeBytes     ( 0 ),
        mbSwapOut       ( false ),
        mbDummyContext  ( false ),
        maLastUsed (std::chrono::high_resolution_clock::now()),
        mbPrepared (false),
        mnPageNumber(-1)
{
}

ImpGraphic::ImpGraphic( const BitmapEx& rBitmapEx ) :
        maEx            ( rBitmapEx ),
        meType          ( !rBitmapEx.IsEmpty() ? GraphicType::Bitmap : GraphicType::NONE ),
        mnSizeBytes     ( 0 ),
        mbSwapOut       ( false ),
        mbDummyContext  ( false ),
        maLastUsed (std::chrono::high_resolution_clock::now()),
        mbPrepared (false),
        mnPageNumber(-1)
{
}

ImpGraphic::ImpGraphic(const VectorGraphicDataPtr& rVectorGraphicDataPtr)
:   meType( rVectorGraphicDataPtr.get() ? GraphicType::Bitmap : GraphicType::NONE ),
    mnSizeBytes( 0 ),
    mbSwapOut( false ),
    mbDummyContext  ( false ),
    maVectorGraphicData(rVectorGraphicDataPtr),
    maLastUsed (std::chrono::high_resolution_clock::now()),
    mbPrepared (false),
    mnPageNumber(-1)
{
}

ImpGraphic::ImpGraphic( const Animation& rAnimation ) :
        maEx            ( rAnimation.GetBitmapEx() ),
        mpAnimation     ( std::make_unique<Animation>( rAnimation ) ),
        meType          ( GraphicType::Bitmap ),
        mnSizeBytes     ( 0 ),
        mbSwapOut       ( false ),
        mbDummyContext  ( false ),
        maLastUsed (std::chrono::high_resolution_clock::now()),
        mbPrepared (false),
        mnPageNumber(-1)
{
}

ImpGraphic::ImpGraphic( const GDIMetaFile& rMtf ) :
        maMetaFile      ( rMtf ),
        meType          ( GraphicType::GdiMetafile ),
        mnSizeBytes     ( 0 ),
        mbSwapOut       ( false ),
        mbDummyContext  ( false ),
        maLastUsed (std::chrono::high_resolution_clock::now()),
        mbPrepared (false),
        mnPageNumber(-1)
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
        mnPageNumber = rImpGraphic.mnPageNumber;
        maGraphicExternalLink = rImpGraphic.maGraphicExternalLink;

        mpAnimation.reset();

        if ( rImpGraphic.mpAnimation )
        {
            mpAnimation = std::make_unique<Animation>( *rImpGraphic.mpAnimation );
            maEx = mpAnimation->GetBitmapEx();
        }
        else
        {
            maEx = rImpGraphic.maEx;
        }

        mbSwapOut = rImpGraphic.mbSwapOut;
        mpSwapFile = rImpGraphic.mpSwapFile;
        mbPrepared = rImpGraphic.mbPrepared;

        mpGfxLink = rImpGraphic.mpGfxLink;

        maVectorGraphicData = rImpGraphic.maVectorGraphicData;
        mpPdfData = rImpGraphic.mpPdfData;
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
    mnPageNumber = rImpGraphic.mnPageNumber;
    mpAnimation = std::move(rImpGraphic.mpAnimation);
    maEx = std::move(rImpGraphic.maEx);
    mbSwapOut = rImpGraphic.mbSwapOut;
    mpSwapFile = std::move(rImpGraphic.mpSwapFile);
    mpGfxLink = std::move(rImpGraphic.mpGfxLink);
    maVectorGraphicData = std::move(rImpGraphic.maVectorGraphicData);
    mpPdfData = std::move(rImpGraphic.mpPdfData);
    maGraphicExternalLink = rImpGraphic.maGraphicExternalLink;
    mbPrepared = rImpGraphic.mbPrepared;

    rImpGraphic.ImplClear();
    rImpGraphic.mbDummyContext = false;
    maLastUsed = std::chrono::high_resolution_clock::now();

    vcl::graphic::Manager::get().changeExisting(this, aOldSizeBytes);

    rImpGraphic.mnPageNumber = -1;

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
                if(maVectorGraphicData.get())
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
                else if (mpPdfData && mpPdfData->hasElements())
                {
                    bRet = (rImpGraphic.mpPdfData && *mpPdfData == *rImpGraphic.mpPdfData);
                }
                else if( mpAnimation )
                {
                    if( rImpGraphic.mpAnimation && ( *rImpGraphic.mpAnimation == *mpAnimation ) )
                        bRet = true;
                }
                else if( !rImpGraphic.mpAnimation && ( rImpGraphic.maEx == maEx ) )
                {
                    bRet = true;
                }
            }
            break;

            default:
            break;
        }
    }

    return bRet;
}

const VectorGraphicDataPtr& ImpGraphic::getVectorGraphicData() const
{
    ensureAvailable();

    return maVectorGraphicData;
}

void ImpGraphic::setPdfData(const std::shared_ptr<uno::Sequence<sal_Int8>>& rPdfData)
{
    ensureAvailable();

    mpPdfData = rPdfData;
}

const std::shared_ptr<uno::Sequence<sal_Int8>>& ImpGraphic::getPdfData() const
{
    ensureAvailable();

    return mpPdfData;
}

void ImpGraphic::ImplCreateSwapInfo()
{
    if (!ImplIsSwapOut())
    {
        maSwapInfo.maPrefMapMode = ImplGetPrefMapMode();
        maSwapInfo.maPrefSize = ImplGetPrefSize();
        maSwapInfo.mbIsAnimated = ImplIsAnimated();
        maSwapInfo.mbIsEPS = ImplIsEPS();
        maSwapInfo.mbIsTransparent = ImplIsTransparent();
        maSwapInfo.mbIsAlpha = ImplIsAlpha();
        maSwapInfo.mnAnimationLoopCount = ImplGetAnimationLoopCount();
    }
}

void ImpGraphic::ImplClearGraphics()
{
    maEx.Clear();
    maMetaFile.Clear();
    mpAnimation.reset();
    mpGfxLink.reset();
    maVectorGraphicData.reset();
    mpPdfData.reset();
}

ImpSwapFile::~ImpSwapFile()
{
    try
    {
        ::ucbhelper::Content aCnt( aSwapURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ),
            css::uno::Reference< css::ucb::XCommandEnvironment >(),
            comphelper::getProcessComponentContext() );

        aCnt.executeCommand( "delete", css::uno::makeAny( true ) );
    }
    catch( const css::ucb::ContentCreationException& )
    {
    }
    catch( const css::uno::RuntimeException& )
    {
    }
    catch( const css::ucb::CommandAbortedException& )
    {
    }
    catch( const css::uno::Exception& )
    {
    }
}

void ImpGraphic::ImplSetPrepared(bool bAnimated)
{
    mbPrepared = true;
    mbSwapOut = true;
    meType = GraphicType::Bitmap;

    SvMemoryStream aMemoryStream(const_cast<sal_uInt8*>(mpGfxLink->GetData()), mpGfxLink->GetDataSize(), StreamMode::READ | StreamMode::WRITE);

    GraphicDescriptor aDescriptor(aMemoryStream, nullptr);
    if (aDescriptor.Detect(true))
    {
        // If we have logic size, work with that, as later pixel -> logic
        // conversion will work with the output device DPI, not the graphic
        // DPI.
        Size aLogSize = aDescriptor.GetSize_100TH_MM();
        if (aLogSize.getWidth() && aLogSize.getHeight())
        {
            maSwapInfo.maPrefSize = aLogSize;
            maSwapInfo.maPrefMapMode = MapMode(MapUnit::Map100thMM);
        }
        else
        {
            maSwapInfo.maPrefSize = aDescriptor.GetSizePixel();
            maSwapInfo.maPrefMapMode = MapMode(MapUnit::MapPixel);
        }

        maSwapInfo.maSizePixel = aDescriptor.GetSizePixel();
    }
    maSwapInfo.mnAnimationLoopCount = 0;
    maSwapInfo.mbIsEPS = false;
    maSwapInfo.mbIsTransparent = false;
    maSwapInfo.mbIsAlpha = false;
    maSwapInfo.mbIsAnimated = bAnimated;
}

void ImpGraphic::ImplClear()
{
    mpSwapFile.reset();
    mbSwapOut = false;
    mbPrepared = false;

    // cleanup
    ImplClearGraphics();
    meType = GraphicType::NONE;
    sal_Int64 nOldSize = mnSizeBytes;
    mnSizeBytes = 0;
    vcl::graphic::Manager::get().changeExisting(this, nOldSize);
    maGraphicExternalLink.msURL.clear();
}

void ImpGraphic::ImplSetDefaultType()
{
    ImplClear();
    meType = GraphicType::Default;
}

bool ImpGraphic::ImplIsSupportedGraphic() const
{
    return( meType != GraphicType::NONE );
}

bool ImpGraphic::ImplIsTransparent() const
{
    bool bRet(true);

    if (mbSwapOut)
    {
        bRet = maSwapInfo.mbIsTransparent;
    }
    else if (meType == GraphicType::Bitmap && !maVectorGraphicData.get())
    {
        bRet = mpAnimation ? mpAnimation->IsTransparent() : maEx.IsTransparent();
    }

    return bRet;
}

bool ImpGraphic::ImplIsAlpha() const
{
    bool bRet(false);

    if (mbSwapOut)
    {
        bRet = maSwapInfo.mbIsAlpha;
    }
    else if (maVectorGraphicData.get())
    {
        bRet = true;
    }
    else if (meType == GraphicType::Bitmap)
    {
        bRet = (nullptr == mpAnimation && maEx.IsAlpha());
    }

    return bRet;
}

bool ImpGraphic::ImplIsAnimated() const
{
    return mbSwapOut ? maSwapInfo.mbIsAnimated : mpAnimation != nullptr;
}

bool ImpGraphic::ImplIsEPS() const
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

Bitmap ImpGraphic::ImplGetBitmap(const GraphicConversionParameters& rParameters) const
{
    Bitmap aRetBmp;

    ensureAvailable();

    if( meType == GraphicType::Bitmap )
    {
        if(maVectorGraphicData.get() && maEx.IsEmpty())
        {
            // use maEx as local buffer for rendered svg
            const_cast< ImpGraphic* >(this)->maEx = maVectorGraphicData->getReplacement();
        }

        const BitmapEx& rRetBmpEx = ( mpAnimation ? mpAnimation->GetBitmapEx() : maEx );
        const Color     aReplaceColor( COL_WHITE );

        aRetBmp = rRetBmpEx.GetBitmap( &aReplaceColor );

        if(rParameters.getSizePixel().Width() || rParameters.getSizePixel().Height())
            aRetBmp.Scale(rParameters.getSizePixel());
    }
    else if( ( meType != GraphicType::Default ) && ImplIsSupportedGraphic() )
    {
        if(maEx.IsEmpty())
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

            if(GraphicType::GdiMetafile == ImplGetType())
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
                    aVDev->SetAntialiasing(aVDev->GetAntialiasing() | AntialiasingFlags::EnableB2dDraw);
                }

                if(rParameters.getSnapHorVerLines())
                {
                    aVDev->SetAntialiasing(aVDev->GetAntialiasing() | AntialiasingFlags::PixelSnapHairline);
                }

                ImplDraw( aVDev.get(), Point(), aDrawSize );

                // use maEx as local buffer for rendered metafile
                const_cast< ImpGraphic* >(this)->maEx = aVDev->GetBitmapEx( Point(), aVDev->GetOutputSizePixel() );
            }
        }

        aRetBmp = maEx.GetBitmap();
    }

    if( !!aRetBmp )
    {
        aRetBmp.SetPrefMapMode( ImplGetPrefMapMode() );
        aRetBmp.SetPrefSize( ImplGetPrefSize() );
    }

    return aRetBmp;
}

BitmapEx ImpGraphic::ImplGetBitmapEx(const GraphicConversionParameters& rParameters) const
{
    BitmapEx aRetBmpEx;

    ensureAvailable();

    if( meType == GraphicType::Bitmap )
    {
        if(maVectorGraphicData.get() && maEx.IsEmpty())
        {
            // use maEx as local buffer for rendered svg
            const_cast< ImpGraphic* >(this)->maEx = maVectorGraphicData->getReplacement();
        }

        aRetBmpEx = ( mpAnimation ? mpAnimation->GetBitmapEx() : maEx );

        if(rParameters.getSizePixel().Width() || rParameters.getSizePixel().Height())
        {
            aRetBmpEx.Scale(
                rParameters.getSizePixel(),
                BmpScaleFlag::Fast);
        }
    }
    else if( ( meType != GraphicType::Default ) && ImplIsSupportedGraphic() )
    {
        if(maEx.IsEmpty())
        {
            const ImpGraphic aMonoMask( maMetaFile.GetMonochromeMtf( COL_BLACK ) );

            // use maEx as local buffer for rendered metafile
            const_cast< ImpGraphic* >(this)->maEx = BitmapEx(ImplGetBitmap(rParameters), aMonoMask.ImplGetBitmap(rParameters));
        }

        aRetBmpEx = maEx;
    }

    return aRetBmpEx;
}

Animation ImpGraphic::ImplGetAnimation() const
{
    Animation aAnimation;

    ensureAvailable();
    if( mpAnimation )
        aAnimation = *mpAnimation;

    return aAnimation;
}

const BitmapEx& ImpGraphic::ImplGetBitmapExRef() const
{
    ensureAvailable();
    return maEx;
}

const GDIMetaFile& ImpGraphic::ImplGetGDIMetaFile() const
{
    ensureAvailable();
    if (!maMetaFile.GetActionSize()
        && maVectorGraphicData.get()
        && (VectorGraphicDataType::Emf == maVectorGraphicData->getVectorGraphicDataType()
            || VectorGraphicDataType::Wmf == maVectorGraphicData->getVectorGraphicDataType()))
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

        if(maVectorGraphicData.get() && !maEx)
        {
            // use maEx as local buffer for rendered svg
            pThat->maEx = maVectorGraphicData->getReplacement();
        }

        // #123983# directly create a metafile with the same PrefSize and PrefMapMode
        // the bitmap has, this will be an always correct metafile
        if(maEx.IsTransparent())
        {
            pThat->maMetaFile.AddAction(new MetaBmpExScaleAction(Point(), maEx.GetPrefSize(), maEx));
        }
        else
        {
            pThat->maMetaFile.AddAction(new MetaBmpScaleAction(Point(), maEx.GetPrefSize(), maEx.GetBitmap()));
        }

        pThat->maMetaFile.Stop();
        pThat->maMetaFile.WindStart();
        pThat->maMetaFile.SetPrefSize(maEx.GetPrefSize());
        pThat->maMetaFile.SetPrefMapMode(maEx.GetPrefMapMode());
    }

    return maMetaFile;
}

Size ImpGraphic::ImplGetSizePixel() const
{
    Size aSize;

    if (ImplIsSwapOut())
        aSize = maSwapInfo.maSizePixel;
    else
        aSize = ImplGetBitmapEx(GraphicConversionParameters()).GetSizePixel();

    return aSize;
}

Size ImpGraphic::ImplGetPrefSize() const
{
    Size aSize;

    if (ImplIsSwapOut())
    {
        aSize = maSwapInfo.maPrefSize;
    }
    else
    {
        switch( meType )
        {
            case GraphicType::NONE:
            case GraphicType::Default:
            break;

            case GraphicType::Bitmap:
            {
                if(maVectorGraphicData.get() && maEx.IsEmpty())
                {
                    // svg not yet buffered in maEx, return size derived from range
                    const basegfx::B2DRange& rRange = maVectorGraphicData->getRange();

                    aSize = Size(basegfx::fround(rRange.getWidth()), basegfx::fround(rRange.getHeight()));
                }
                else
                {
                    aSize = maEx.GetPrefSize();

                    if( !aSize.Width() || !aSize.Height() )
                    {
                        aSize = maEx.GetSizePixel();
                    }
                }
            }
            break;

            default:
            {
                if( ImplIsSupportedGraphic() )
                  aSize = maMetaFile.GetPrefSize();
            }
            break;
        }
    }

    return aSize;
}

void ImpGraphic::ImplSetPrefSize( const Size& rPrefSize )
{
    ensureAvailable();

    switch( meType )
    {
        case GraphicType::NONE:
        case GraphicType::Default:
        break;

        case GraphicType::Bitmap:
        {
            // used when importing a writer FlyFrame with SVG as graphic, added conversion
            // to allow setting the PrefSize at the BitmapEx to hold it
            if(maVectorGraphicData.get() && maEx.IsEmpty())
            {
                // use maEx as local buffer for rendered svg
                maEx = maVectorGraphicData->getReplacement();
            }

            // #108077# Push through pref size to animation object,
            // will be lost on copy otherwise
            if( ImplIsAnimated() )
            {
                const_cast< BitmapEx& >(mpAnimation->GetBitmapEx()).SetPrefSize( rPrefSize );
            }

            maEx.SetPrefSize( rPrefSize );
        }
        break;

        default:
        {
            if( ImplIsSupportedGraphic() )
                maMetaFile.SetPrefSize( rPrefSize );
        }
        break;
    }
}

MapMode ImpGraphic::ImplGetPrefMapMode() const
{
    MapMode aMapMode;

    if (ImplIsSwapOut())
    {
        aMapMode = maSwapInfo.maPrefMapMode;
    }
    else
    {
        switch( meType )
        {
            case GraphicType::NONE:
            case GraphicType::Default:
            break;

            case GraphicType::Bitmap:
            {
                if(maVectorGraphicData.get() && maEx.IsEmpty())
                {
                    // svg not yet buffered in maEx, return default PrefMapMode
                    aMapMode = MapMode(MapUnit::Map100thMM);
                }
                else
                {
                    const Size aSize( maEx.GetPrefSize() );

                    if ( aSize.Width() && aSize.Height() )
                        aMapMode = maEx.GetPrefMapMode();
                }
            }
            break;

            default:
            {
                if( ImplIsSupportedGraphic() )
                    return maMetaFile.GetPrefMapMode();
            }
            break;
        }
    }

    return aMapMode;
}

void ImpGraphic::ImplSetPrefMapMode( const MapMode& rPrefMapMode )
{
    ensureAvailable();

    switch( meType )
    {
        case GraphicType::NONE:
        case GraphicType::Default:
        break;

        case GraphicType::Bitmap:
        {
            if(maVectorGraphicData.get())
            {
                // ignore for Vector Graphic Data. If this is really used (except the grfcache)
                // it can be extended by using maEx as buffer for maVectorGraphicData->getReplacement()
            }
            else
            {
                // #108077# Push through pref mapmode to animation object,
                // will be lost on copy otherwise
                if( ImplIsAnimated() )
                {
                    const_cast< BitmapEx& >(mpAnimation->GetBitmapEx()).SetPrefMapMode( rPrefMapMode );
                }

                maEx.SetPrefMapMode( rPrefMapMode );
            }
        }
        break;

        default:
        {
            if( ImplIsSupportedGraphic() )
                maMetaFile.SetPrefMapMode( rPrefMapMode );
        }
        break;
    }
}

sal_uLong ImpGraphic::ImplGetSizeBytes() const
{
    if( 0 == mnSizeBytes )
    {
        if (mbPrepared)
            ensureAvailable();

        if( meType == GraphicType::Bitmap )
        {
            if(maVectorGraphicData.get())
            {
                std::pair<VectorGraphicData::State, size_t> tmp(maVectorGraphicData->getSizeBytes());
                if (VectorGraphicData::State::UNPARSED == tmp.first)
                {
                    return tmp.second; // don't cache it until Vector Graphic Data is parsed
                }
                mnSizeBytes = tmp.second;
            }
            else
            {
                mnSizeBytes = mpAnimation ? mpAnimation->GetSizeBytes() : maEx.GetSizeBytes();
            }
        }
        else if( meType == GraphicType::GdiMetafile )
        {
            mnSizeBytes = maMetaFile.GetSizeBytes();
        }
    }

    return mnSizeBytes;
}

void ImpGraphic::ImplDraw( OutputDevice* pOutDev, const Point& rDestPt ) const
{
    ensureAvailable();
    if( ImplIsSupportedGraphic() && !ImplIsSwapOut() )
    {
        switch( meType )
        {
            case GraphicType::Default:
            break;

            case GraphicType::Bitmap:
            {
                if(maVectorGraphicData.get() && !maEx)
                {
                    // use maEx as local buffer for rendered svg
                    const_cast< ImpGraphic* >(this)->maEx = maVectorGraphicData->getReplacement();
                }

                if ( mpAnimation )
                {
                    mpAnimation->Draw( pOutDev, rDestPt );
                }
                else
                {
                    maEx.Draw( pOutDev, rDestPt );
                }
            }
            break;

            default:
                ImplDraw( pOutDev, rDestPt, maMetaFile.GetPrefSize() );
            break;
        }
    }
}

void ImpGraphic::ImplDraw( OutputDevice* pOutDev,
                           const Point& rDestPt, const Size& rDestSize ) const
{
    ensureAvailable();
    if( ImplIsSupportedGraphic() && !ImplIsSwapOut() )
    {
        switch( meType )
        {
            case GraphicType::Default:
            break;

            case GraphicType::Bitmap:
            {
                if(maVectorGraphicData.get() && maEx.IsEmpty())
                {
                    // use maEx as local buffer for rendered svg
                    const_cast< ImpGraphic* >(this)->maEx = maVectorGraphicData->getReplacement();
                }

                if( mpAnimation )
                {
                    mpAnimation->Draw( pOutDev, rDestPt, rDestSize );
                }
                else
                {
                    maEx.Draw( pOutDev, rDestPt, rDestSize );
                }
            }
            break;

            default:
            {
                const_cast<ImpGraphic*>(this)->maMetaFile.WindStart();
                const_cast<ImpGraphic*>(this)->maMetaFile.Play( pOutDev, rDestPt, rDestSize );
                const_cast<ImpGraphic*>(this)->maMetaFile.WindStart();
            }
            break;
        }
    }
}

void ImpGraphic::ImplStartAnimation( OutputDevice* pOutDev, const Point& rDestPt,
                                     const Size& rDestSize, long nExtraData,
                                     OutputDevice* pFirstFrameOutDev )
{
    ensureAvailable();

    if( ImplIsSupportedGraphic() && !ImplIsSwapOut() && mpAnimation )
        mpAnimation->Start( pOutDev, rDestPt, rDestSize, nExtraData, pFirstFrameOutDev );
}

void ImpGraphic::ImplStopAnimation( OutputDevice* pOutDev, long nExtraData )
{
    ensureAvailable();

    if( ImplIsSupportedGraphic() && !ImplIsSwapOut() && mpAnimation )
        mpAnimation->Stop( pOutDev, nExtraData );
}

void ImpGraphic::ImplSetAnimationNotifyHdl( const Link<Animation*,void>& rLink )
{
    ensureAvailable();

    if( mpAnimation )
        mpAnimation->SetNotifyHdl( rLink );
}

Link<Animation*,void> ImpGraphic::ImplGetAnimationNotifyHdl() const
{
    Link<Animation*,void> aLink;

    ensureAvailable();

    if( mpAnimation )
        aLink = mpAnimation->GetNotifyHdl();

    return aLink;
}

sal_uInt32 ImpGraphic::ImplGetAnimationLoopCount() const
{
    if (mbSwapOut)
        return maSwapInfo.mnAnimationLoopCount;

    return mpAnimation ? mpAnimation->GetLoopCount() : 0;
}

void ImpGraphic::ImplSetContext( const std::shared_ptr<GraphicReader>& pReader )
{
    mpContext = pReader;
    mbDummyContext = false;
}

bool ImpGraphic::ImplReadEmbedded( SvStream& rIStm )
{
    ensureAvailable();

    MapMode         aMapMode;
    Size            aSize;
    sal_uInt32      nId;
    sal_Int32       nType;
    const SvStreamEndian nOldFormat = rIStm.GetEndian();
    bool            bRet = false;

    rIStm.SetEndian( SvStreamEndian::LITTLE );
    rIStm.ReadUInt32( nId );

    // check version
    if( GRAPHIC_FORMAT_50 == nId )
    {
        // read new style header
        std::unique_ptr<VersionCompat> pCompat( new VersionCompat( rIStm, StreamMode::READ ) );

        rIStm.ReadInt32( nType );
        sal_Int32 nLen;
        rIStm.ReadInt32( nLen );
        ReadPair( rIStm, aSize );
        ReadMapMode( rIStm, aMapMode );
    }
    else
    {
        // read old style header
        sal_Int32 nWidth, nHeight;
        sal_Int32 nMapMode, nScaleNumX, nScaleDenomX;
        sal_Int32 nScaleNumY, nScaleDenomY, nOffsX, nOffsY;

        rIStm.SeekRel( -4 );

        sal_Int32 nLen;
        rIStm.ReadInt32( nType ).ReadInt32( nLen ).ReadInt32( nWidth ).ReadInt32( nHeight );
        rIStm.ReadInt32( nMapMode ).ReadInt32( nScaleNumX ).ReadInt32( nScaleDenomX ).ReadInt32( nScaleNumY );
        rIStm.ReadInt32( nScaleDenomY ).ReadInt32( nOffsX ).ReadInt32( nOffsY );

        // swapped
        if( nType > 100 )
        {
            nType = OSL_SWAPDWORD( nType );
            nWidth = OSL_SWAPDWORD( nWidth );
            nHeight = OSL_SWAPDWORD( nHeight );
            nMapMode = OSL_SWAPDWORD( nMapMode );
            nScaleNumX = OSL_SWAPDWORD( nScaleNumX );
            nScaleDenomX = OSL_SWAPDWORD( nScaleDenomX );
            nScaleNumY = OSL_SWAPDWORD( nScaleNumY );
            nScaleDenomY = OSL_SWAPDWORD( nScaleDenomY );
            nOffsX = OSL_SWAPDWORD( nOffsX );
            nOffsY = OSL_SWAPDWORD( nOffsY );
        }

        aSize = Size( nWidth, nHeight );
        aMapMode = MapMode( static_cast<MapUnit>(nMapMode), Point( nOffsX, nOffsY ),
                            Fraction( nScaleNumX, nScaleDenomX ),
                            Fraction( nScaleNumY, nScaleDenomY ) );
    }

    meType = static_cast<GraphicType>(nType);

    if( meType != GraphicType::NONE )
    {
        if( meType == GraphicType::Bitmap )
        {
            if(maVectorGraphicData.get() && maEx.IsEmpty())
            {
                // use maEx as local buffer for rendered svg
                maEx = maVectorGraphicData->getReplacement();
            }

            maEx.SetSizePixel(aSize);

            if( aMapMode != MapMode() )
            {
                maEx.SetPrefMapMode( aMapMode );
                maEx.SetPrefSize( aSize );
            }
        }
        else
        {
            maMetaFile.SetPrefMapMode( aMapMode );
            maMetaFile.SetPrefSize( aSize );
        }

        if( meType == GraphicType::Bitmap || meType == GraphicType::GdiMetafile )
        {
            ReadImpGraphic( rIStm, *this );
            bRet = rIStm.GetError() == ERRCODE_NONE;
        }
        else if( sal::static_int_cast<sal_uLong>(meType) >= SYS_WINMETAFILE
                 && sal::static_int_cast<sal_uLong>(meType) <= SYS_MACMETAFILE )
        {
            Graphic           aSysGraphic;
            ConvertDataFormat nCvtType;

            switch( sal::static_int_cast<sal_uLong>(meType) )
            {
                case SYS_WINMETAFILE:
                case SYS_WNTMETAFILE: nCvtType = ConvertDataFormat::WMF; break;
                case SYS_OS2METAFILE: nCvtType = ConvertDataFormat::MET; break;
                case SYS_MACMETAFILE: nCvtType = ConvertDataFormat::PCT; break;

                default:
                    nCvtType = ConvertDataFormat::Unknown;
                break;
            }

            if( nType && GraphicConverter::Import( rIStm, aSysGraphic, nCvtType ) == ERRCODE_NONE )
            {
                *this = ImpGraphic( aSysGraphic.GetGDIMetaFile() );
                bRet = rIStm.GetError() == ERRCODE_NONE;
            }
            else
                meType = GraphicType::Default;
        }

        if( bRet )
        {
            ImplSetPrefMapMode( aMapMode );
            ImplSetPrefSize( aSize );
        }
    }
    else
        bRet = true;

    rIStm.SetEndian( nOldFormat );

    return bRet;
}

bool ImpGraphic::ImplWriteEmbedded( SvStream& rOStm )
{
    bool bRet = false;

    ensureAvailable();

    if( ( meType != GraphicType::NONE ) && ( meType != GraphicType::Default ) && !ImplIsSwapOut() )
    {
        const MapMode   aMapMode( ImplGetPrefMapMode() );
        const Size      aSize( ImplGetPrefSize() );
        const SvStreamEndian nOldFormat = rOStm.GetEndian();
        sal_uLong           nDataFieldPos;

        rOStm.SetEndian( SvStreamEndian::LITTLE );

        // write correct version ( old style/new style header )
        if( rOStm.GetVersion() >= SOFFICE_FILEFORMAT_50 )
        {
            // write ID for new format (5.0)
            rOStm.WriteUInt32( GRAPHIC_FORMAT_50 );

            // write new style header
            std::unique_ptr<VersionCompat> pCompat( new VersionCompat( rOStm, StreamMode::WRITE, 1 ) );

            rOStm.WriteInt32( static_cast<sal_Int32>(meType) );

            // data size is updated later
            nDataFieldPos = rOStm.Tell();
            rOStm.WriteInt32( 0 );

            WritePair( rOStm, aSize );
            WriteMapMode( rOStm, aMapMode );
        }
        else
        {
            // write old style (<=4.0) header
            rOStm.WriteInt32( static_cast<sal_Int32>(meType) );

            // data size is updated later
            nDataFieldPos = rOStm.Tell();
            rOStm.WriteInt32( 0 );
            rOStm.WriteInt32( aSize.Width() );
            rOStm.WriteInt32( aSize.Height() );
            rOStm.WriteInt32( static_cast<sal_uInt16>(aMapMode.GetMapUnit()) );
            rOStm.WriteInt32( aMapMode.GetScaleX().GetNumerator() );
            rOStm.WriteInt32( aMapMode.GetScaleX().GetDenominator() );
            rOStm.WriteInt32( aMapMode.GetScaleY().GetNumerator() );
            rOStm.WriteInt32( aMapMode.GetScaleY().GetDenominator() );
            rOStm.WriteInt32( aMapMode.GetOrigin().X() );
            rOStm.WriteInt32( aMapMode.GetOrigin().Y() );
        }

        // write data block
        if( !rOStm.GetError() )
        {
            const sal_uLong nDataStart = rOStm.Tell();

            if( ImplIsSupportedGraphic() )
                WriteImpGraphic( rOStm, *this );

            if( !rOStm.GetError() )
            {
                const sal_uLong nStmPos2 = rOStm.Tell();
                rOStm.Seek( nDataFieldPos );
                rOStm.WriteInt32( nStmPos2 - nDataStart );
                rOStm.Seek( nStmPos2 );
                bRet = true;
            }
        }

        rOStm.SetEndian( nOldFormat );
    }

    return bRet;
}

bool ImpGraphic::ImplSwapOut()
{
    bool bRet = false;

    if( !ImplIsSwapOut() )
    {
        ::utl::TempFile     aTempFile;
        const INetURLObject aTmpURL( aTempFile.GetURL() );

        if( !aTmpURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ).isEmpty() )
        {
            std::unique_ptr<SvStream> xOStm;
            try
            {
                xOStm = ::utl::UcbStreamHelper::CreateStream( aTmpURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::READWRITE | StreamMode::SHARE_DENYWRITE );
            }
            catch( const css::uno::Exception& )
            {
            }
            if( xOStm )
            {
                xOStm->SetVersion( SOFFICE_FILEFORMAT_50 );
                xOStm->SetCompressMode( SvStreamCompressFlags::NATIVE );

                bRet = ImplSwapOut( xOStm.get() );
                if( bRet )
                {
                    mpSwapFile = std::make_unique<ImpSwapFile>();
                    mpSwapFile->aSwapURL = aTmpURL;
                    mpSwapFile->maOriginURL = getOriginURL();
                }
                else
                {
                    xOStm.reset();

                    try
                    {
                        ::ucbhelper::Content aCnt( aTmpURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ),
                                            css::uno::Reference< css::ucb::XCommandEnvironment >(),
                                            comphelper::getProcessComponentContext() );

                        aCnt.executeCommand( "delete", css::uno::makeAny( true ) );
                    }
                    catch( const css::ucb::ContentCreationException& )
                    {
                    }
                    catch( const css::uno::RuntimeException& )
                    {
                    }
                    catch( const css::ucb::CommandAbortedException& )
                    {
                    }
                    catch( const css::uno::Exception& )
                    {
                    }
                }
            }
        }
    }

    if (bRet)
        vcl::graphic::Manager::get().swappedOut(this);
    return bRet;
}

bool ImpGraphic::ImplSwapOut( SvStream* xOStm )
{
    bool bRet = false;

    if( xOStm )
    {
        xOStm->SetBufferSize( GRAPHIC_STREAMBUFSIZE );

        if( !xOStm->GetError() && ImplWriteEmbedded( *xOStm ) )
        {
            xOStm->Flush();

            if( !xOStm->GetError() )
            {
                ImplCreateSwapInfo();
                ImplClearGraphics();
                bRet = mbSwapOut = true;
            }
        }
    }
    else
    {
        SAL_WARN("vcl.gdi", "Graphic SwapOut: No stream for swap out!");
    }

    return bRet;
}

bool ImpGraphic::ensureAvailable() const
{
    auto pThis = const_cast<ImpGraphic*>(this);

    if (ImplIsSwapOut())
        return pThis->ImplSwapIn();

    pThis->maLastUsed = std::chrono::high_resolution_clock::now();
    return true;
}

bool ImpGraphic::loadPrepared()
{
    Graphic aGraphic;
    if (mpGfxLink->LoadNative(aGraphic))
    {
        GraphicExternalLink aLink = maGraphicExternalLink;
        *this = *aGraphic.ImplGetImpGraphic();
        maGraphicExternalLink = aLink;

        return true;
    }
    return false;
}

bool ImpGraphic::ImplSwapIn()
{
    bool bRet = false;

    if (!ImplIsSwapOut())
        return bRet;

    if (mbPrepared)
    {
        bRet = loadPrepared();
    }
    else
    {
        OUString aSwapURL;

        if( mpSwapFile )
            aSwapURL = mpSwapFile->aSwapURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );

        if( !aSwapURL.isEmpty() )
        {
            std::unique_ptr<SvStream> xIStm;
            try
            {
                xIStm = ::utl::UcbStreamHelper::CreateStream( aSwapURL, StreamMode::READWRITE | StreamMode::SHARE_DENYWRITE );
            }
            catch( const css::uno::Exception& )
            {
            }

            if( xIStm )
            {
                xIStm->SetVersion( SOFFICE_FILEFORMAT_50 );
                xIStm->SetCompressMode( SvStreamCompressFlags::NATIVE );

                bRet = ImplSwapIn( xIStm.get() );
                xIStm.reset();
                if (mpSwapFile)
                    setOriginURL(mpSwapFile->maOriginURL);
                mpSwapFile.reset();
            }
        }
    }

    if (bRet)
        vcl::graphic::Manager::get().swappedIn(this);

    return bRet;
}

bool ImpGraphic::ImplSwapIn( SvStream* xIStm )
{
    bool bRet = false;

    if( xIStm )
    {
        xIStm->SetBufferSize( GRAPHIC_STREAMBUFSIZE );

        if( !xIStm->GetError() )
        {
            //keep the swap file alive, because its quite possibly the backing storage
            //for xIStm
            std::shared_ptr<ImpSwapFile> xSwapFile(std::move(mpSwapFile));
            assert(!mpSwapFile);

            std::shared_ptr<GraphicReader> xContext(std::move(mpContext));
            assert(!mpContext);

            bool bDummyContext = mbDummyContext;
            mbDummyContext = false;

            bRet = ImplReadEmbedded( *xIStm );

            //restore ownership of the swap file and context
            mpSwapFile = std::move(xSwapFile);
            mpContext = std::move(xContext);
            mbDummyContext = bDummyContext;

            if (!bRet)
            {
                //throw away swapfile, etc.
                ImplClear();
            }

            mbSwapOut = false;
        }
    }

    return bRet;
}

void ImpGraphic::ImplSetLink(const std::shared_ptr<GfxLink>& rGfxLink)
{
    ensureAvailable();

    mpGfxLink = rGfxLink;

    if (mpGfxLink && mpGfxLink->IsNative())
        mpGfxLink->SwapOut();
}

GfxLink ImpGraphic::ImplGetLink()
{
    ensureAvailable();

    return( mpGfxLink ? *mpGfxLink : GfxLink() );
}

bool ImpGraphic::ImplIsLink() const
{
    return ( bool(mpGfxLink) );
}

BitmapChecksum ImpGraphic::ImplGetChecksum() const
{
    BitmapChecksum nRet = 0;

    ensureAvailable();

    if( ImplIsSupportedGraphic() && !ImplIsSwapOut() )
    {
        switch( meType )
        {
            case GraphicType::Default:
            break;

            case GraphicType::Bitmap:
            {
                if(maVectorGraphicData.get() && maEx.IsEmpty())
                {
                    // use maEx as local buffer for rendered svg
                    const_cast< ImpGraphic* >(this)->maEx = maVectorGraphicData->getReplacement();
                }

                if( mpAnimation )
                {
                    nRet = mpAnimation->GetChecksum();
                }
                else
                {
                    nRet = maEx.GetChecksum();
                }

                if (mpPdfData && mpPdfData->hasElements())
                    // Include the PDF data in the checksum, so a metafile with
                    // and without PDF data is considered to be different.
                    nRet = vcl_get_checksum(nRet, mpPdfData->getConstArray(), mpPdfData->getLength());
            }
            break;

            default:
                nRet = maMetaFile.GetChecksum();
            break;
        }
    }

    return nRet;
}

bool ImpGraphic::ImplExportNative( SvStream& rOStm ) const
{
    bool bResult = false;

    ensureAvailable();

    if( !rOStm.GetError() )
    {
        if( !ImplIsSwapOut() )
        {
            if( mpGfxLink && mpGfxLink->IsNative() )
                bResult = mpGfxLink->ExportNative( rOStm );
            else
            {
                WriteImpGraphic( rOStm, *this );
                bResult = ( rOStm.GetError() == ERRCODE_NONE );
            }
        }
        else
             rOStm.SetError( SVSTREAM_GENERALERROR );
    }

    return bResult;
}


void ReadImpGraphic( SvStream& rIStm, ImpGraphic& rImpGraphic )
{
    if (rIStm.GetError())
        return;

    const sal_uLong nStmPos1 = rIStm.Tell();
    sal_uInt32 nTmp;

    rImpGraphic.ImplClear();

    // read Id
    rIStm.ReadUInt32( nTmp );

    // if there is no more data, avoid further expensive
    // reading which will create VDevs and other stuff, just to
    // read nothing. CAUTION: Eof is only true AFTER reading another
    // byte, a speciality of SvMemoryStream (!)
    if (!rIStm.good())
        return;

    if (NATIVE_FORMAT_50 == nTmp)
    {
        Graphic         aGraphic;
        GfxLink         aLink;

        // read compat info
        std::unique_ptr<VersionCompat> pCompat(new VersionCompat( rIStm, StreamMode::READ ));
        pCompat.reset(); // destructor writes stuff into the header

        ReadGfxLink( rIStm, aLink );

        // set dummy link to avoid creation of additional link after filtering;
        // we set a default link to avoid unnecessary swapping of native data
        aGraphic.SetGfxLink(std::make_shared<GfxLink>());

        if( !rIStm.GetError() && aLink.LoadNative( aGraphic ) )
        {
            // set link only, if no other link was set
            const bool bSetLink = !rImpGraphic.mpGfxLink;

            // assign graphic
            rImpGraphic = *aGraphic.ImplGetImpGraphic();

            if( aLink.IsPrefMapModeValid() )
                rImpGraphic.ImplSetPrefMapMode( aLink.GetPrefMapMode() );

            if( aLink.IsPrefSizeValid() )
                rImpGraphic.ImplSetPrefSize( aLink.GetPrefSize() );

            if( bSetLink )
                rImpGraphic.ImplSetLink(std::make_shared<GfxLink>(aLink));
        }
        else
        {
            rIStm.Seek( nStmPos1 );
            rIStm.SetError( ERRCODE_IO_WRONGFORMAT );
        }
        return;
    }

    BitmapEx        aBmpEx;
    const SvStreamEndian nOldFormat = rIStm.GetEndian();

    rIStm.SeekRel( -4 );
    rIStm.SetEndian( SvStreamEndian::LITTLE );
    ReadDIBBitmapEx(aBmpEx, rIStm);

    if( !rIStm.GetError() )
    {
        sal_uInt32  nMagic1(0), nMagic2(0);
        sal_uLong   nActPos = rIStm.Tell();

        rIStm.ReadUInt32( nMagic1 ).ReadUInt32( nMagic2 );
        rIStm.Seek( nActPos );

        rImpGraphic = ImpGraphic( aBmpEx );

        if( !rIStm.GetError() && ( 0x5344414e == nMagic1 ) && ( 0x494d4931 == nMagic2 ) )
        {
            rImpGraphic.mpAnimation = std::make_unique<Animation>();
            ReadAnimation( rIStm, *rImpGraphic.mpAnimation );

            // #108077# manually set loaded BmpEx to Animation
            // (which skips loading its BmpEx if already done)
            rImpGraphic.mpAnimation->SetBitmapEx(aBmpEx);
        }
        else
            rIStm.ResetError();
    }
    else
    {
        GDIMetaFile aMtf;

        rIStm.Seek( nStmPos1 );
        rIStm.ResetError();
        ReadGDIMetaFile( rIStm, aMtf );

        if( !rIStm.GetError() )
        {
            rImpGraphic = aMtf;
        }
        else
        {
            ErrCode nOrigError = rIStm.GetErrorCode();
            // try to stream in Svg defining data (length, byte array and evtl. path)
            // See below (operator<<) for more information
            const sal_uInt32 nSvgMagic((sal_uInt32('s') << 24) | (sal_uInt32('v') << 16) | (sal_uInt32('g') << 8) | sal_uInt32('0'));
            const sal_uInt32 nWmfMagic((sal_uInt32('w') << 24) | (sal_uInt32('m') << 16) | (sal_uInt32('f') << 8) | sal_uInt32('0'));
            const sal_uInt32 nEmfMagic((sal_uInt32('e') << 24) | (sal_uInt32('m') << 16) | (sal_uInt32('f') << 8) | sal_uInt32('0'));
            sal_uInt32 nMagic;
            rIStm.Seek(nStmPos1);
            rIStm.ResetError();
            rIStm.ReadUInt32( nMagic );

            if (nSvgMagic == nMagic || nWmfMagic == nMagic || nEmfMagic == nMagic)
            {
                sal_uInt32 nVectorGraphicDataArrayLength(0);
                rIStm.ReadUInt32(nVectorGraphicDataArrayLength);

                if (nVectorGraphicDataArrayLength)
                {
                    VectorGraphicDataArray aNewData(nVectorGraphicDataArrayLength);

                    rIStm.ReadBytes(aNewData.getArray(), nVectorGraphicDataArrayLength);
                    OUString aPath = rIStm.ReadUniOrByteString(rIStm.GetStreamCharSet());

                    if (!rIStm.GetError())
                    {
                        VectorGraphicDataType aDataType(VectorGraphicDataType::Svg);

                        if (nWmfMagic == nMagic)
                        {
                            aDataType = VectorGraphicDataType::Wmf;
                        }
                        else if (nEmfMagic == nMagic)
                        {
                            aDataType = VectorGraphicDataType::Emf;
                        }

                        VectorGraphicDataPtr aVectorGraphicDataPtr(new VectorGraphicData(aNewData, aPath, aDataType));
                        rImpGraphic = aVectorGraphicDataPtr;
                    }
                }
            }
            else if (nMagic == nPdfMagic)
            {
                // Stream in PDF data.
                sal_uInt32 nPdfDataLength = 0;
                rIStm.ReadUInt32(nPdfDataLength);
                Bitmap aBitmap;

                if (nPdfDataLength && !rIStm.GetError())
                {
                    if (!rImpGraphic.mpPdfData)
                        rImpGraphic.mpPdfData.reset(new uno::Sequence<sal_Int8>());

                    if (vcl::ImportPDF(rIStm, aBitmap, rImpGraphic.mnPageNumber,
                                       *rImpGraphic.mpPdfData,
                                       rIStm.Tell(), nPdfDataLength))
                    {
                        rImpGraphic.maEx = aBitmap;
                        rImpGraphic.meType = GraphicType::Bitmap;
                    }
                }
            }
            else
            {
                rIStm.SetError(nOrigError);
            }

            rIStm.Seek(nStmPos1);
        }
    }

    rIStm.SetEndian( nOldFormat );
}

void WriteImpGraphic(SvStream& rOStm, const ImpGraphic& rImpGraphic)
{
    if (rOStm.GetError())
        return;

    rImpGraphic.ensureAvailable();

    if (rImpGraphic.ImplIsSwapOut())
    {
        rOStm.SetError( SVSTREAM_GENERALERROR );
        return;
    }

    if( ( rOStm.GetVersion() >= SOFFICE_FILEFORMAT_50 ) &&
        ( rOStm.GetCompressMode() & SvStreamCompressFlags::NATIVE ) &&
        rImpGraphic.mpGfxLink && rImpGraphic.mpGfxLink->IsNative() &&
        !rImpGraphic.hasPdfData())
    {
        // native format
        rOStm.WriteUInt32( NATIVE_FORMAT_50 );

        // write compat info
        std::unique_ptr<VersionCompat> pCompat(new VersionCompat( rOStm, StreamMode::WRITE, 1 ));
        pCompat.reset(); // destructor writes stuff into the header

        rImpGraphic.mpGfxLink->SetPrefMapMode( rImpGraphic.ImplGetPrefMapMode() );
        rImpGraphic.mpGfxLink->SetPrefSize( rImpGraphic.ImplGetPrefSize() );
        WriteGfxLink( rOStm, *rImpGraphic.mpGfxLink );
    }
    else
    {
        // own format
        const SvStreamEndian nOldFormat = rOStm.GetEndian();
        rOStm.SetEndian( SvStreamEndian::LITTLE );

        switch( rImpGraphic.ImplGetType() )
        {
            case GraphicType::NONE:
            case GraphicType::Default:
            break;

            case GraphicType::Bitmap:
            {
                if(rImpGraphic.getVectorGraphicData().get())
                {
                    // stream out Vector Graphic defining data (length, byte array and evtl. path)
                    // this is used e.g. in swapping out graphic data and in transporting it over UNO API
                    // as sequence of bytes, but AFAIK not written anywhere to any kind of file, so it should be
                    // no problem to extend it; only used at runtime
                    switch (rImpGraphic.getVectorGraphicData()->getVectorGraphicDataType())
                    {
                        case VectorGraphicDataType::Wmf:
                        {
                            const sal_uInt32 nWmfMagic((sal_uInt32('w') << 24) | (sal_uInt32('m') << 16) | (sal_uInt32('f') << 8) | sal_uInt32('0'));
                            rOStm.WriteUInt32(nWmfMagic);
                            break;
                        }
                        case VectorGraphicDataType::Emf:
                        {
                            const sal_uInt32 nEmfMagic((sal_uInt32('e') << 24) | (sal_uInt32('m') << 16) | (sal_uInt32('f') << 8) | sal_uInt32('0'));
                            rOStm.WriteUInt32(nEmfMagic);
                            break;
                        }
                        default: // case VectorGraphicDataType::Svg:
                        {
                            const sal_uInt32 nSvgMagic((sal_uInt32('s') << 24) | (sal_uInt32('v') << 16) | (sal_uInt32('g') << 8) | sal_uInt32('0'));
                            rOStm.WriteUInt32(nSvgMagic);
                            break;
                        }
                    }

                    rOStm.WriteUInt32( rImpGraphic.getVectorGraphicData()->getVectorGraphicDataArrayLength() );
                    rOStm.WriteBytes(rImpGraphic.getVectorGraphicData()->getVectorGraphicDataArray().getConstArray(),
                        rImpGraphic.getVectorGraphicData()->getVectorGraphicDataArrayLength());
                    rOStm.WriteUniOrByteString(rImpGraphic.getVectorGraphicData()->getPath(),
                                               rOStm.GetStreamCharSet());
                }
                else if (rImpGraphic.hasPdfData())
                {
                    // Stream out PDF data.
                    rOStm.WriteUInt32(nPdfMagic);
                    rOStm.WriteUInt32(rImpGraphic.mpPdfData->getLength());
                    rOStm.WriteBytes(rImpGraphic.mpPdfData->getConstArray(), rImpGraphic.mpPdfData->getLength());
                }
                else if( rImpGraphic.ImplIsAnimated())
                {
                    WriteAnimation( rOStm, *rImpGraphic.mpAnimation );
                }
                else
                {
                    WriteDIBBitmapEx(rImpGraphic.maEx, rOStm);
                }
            }
            break;

            default:
            {
                if( rImpGraphic.ImplIsSupportedGraphic() )
                    WriteGDIMetaFile( rOStm, rImpGraphic.maMetaFile );
            }
            break;
        }

        rOStm.SetEndian( nOldFormat );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
