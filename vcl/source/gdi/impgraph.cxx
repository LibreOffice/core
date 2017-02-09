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

#include "sal/config.h"

#include <comphelper/processfactory.hxx>
#include <tools/fract.hxx>
#include <tools/vcompat.hxx>
#include <tools/urlobj.hxx>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <tools/helpers.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/gfxlink.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/graph.hxx>
#include <vcl/metaact.hxx>
#include <impgraph.hxx>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <vcl/dibtools.hxx>
#include <memory>
#include <o3tl/make_unique.hxx>

#define GRAPHIC_MTFTOBMP_MAXEXT     2048
#define GRAPHIC_STREAMBUFSIZE       8192UL

#define SYS_WINMETAFILE             0x00000003L
#define SYS_WNTMETAFILE             0x00000004L
#define SYS_OS2METAFILE             0x00000005L
#define SYS_MACMETAFILE             0x00000006L

#define GRAPHIC_FORMAT_50           static_cast<sal_uInt32>(COMPAT_FORMAT( 'G', 'R', 'F', '5' ))
#define NATIVE_FORMAT_50            static_cast<sal_uInt32>(COMPAT_FORMAT( 'N', 'A', 'T', '5' ))

const sal_uInt32 nPdfMagic((sal_uInt32('p') << 24) | (sal_uInt32('d') << 16) | (sal_uInt32('f') << 8) | sal_uInt32('0'));

using namespace com::sun::star;

struct ImpSwapFile
{
    INetURLObject   aSwapURL;
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

ImpGraphic::ImpGraphic() :
        meType          ( GraphicType::NONE ),
        mnSizeBytes     ( 0UL ),
        mbSwapOut       ( false ),
        mbDummyContext  ( false )
{
}

ImpGraphic::ImpGraphic(const ImpGraphic& rImpGraphic)
    : maMetaFile(rImpGraphic.maMetaFile)
    , maEx(rImpGraphic.maEx)
    , maSwapInfo(rImpGraphic.maSwapInfo)
    , mpContext(rImpGraphic.mpContext)
    , mpSwapFile(rImpGraphic.mpSwapFile)
    , meType(rImpGraphic.meType)
    , mnSizeBytes(rImpGraphic.mnSizeBytes)
    , mbSwapOut(rImpGraphic.mbSwapOut)
    , mbDummyContext(rImpGraphic.mbDummyContext)
    , maSvgData(rImpGraphic.maSvgData)
    , maPdfData(rImpGraphic.maPdfData)
{
    if( rImpGraphic.mpGfxLink )
        mpGfxLink = o3tl::make_unique<GfxLink>( *rImpGraphic.mpGfxLink );

    if( rImpGraphic.mpAnimation )
    {
        mpAnimation = o3tl::make_unique<Animation>( *rImpGraphic.mpAnimation );
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
    , maSvgData(std::move(rImpGraphic.maSvgData))
    , maPdfData(std::move(rImpGraphic.maPdfData))
{
    rImpGraphic.ImplClear();
    rImpGraphic.mbDummyContext = false;
}

ImpGraphic::ImpGraphic( const Bitmap& rBitmap ) :
        maEx            ( rBitmap ),
        meType          ( !rBitmap.IsEmpty() ? GraphicType::Bitmap : GraphicType::NONE ),
        mnSizeBytes     ( 0UL ),
        mbSwapOut       ( false ),
        mbDummyContext  ( false )
{
}

ImpGraphic::ImpGraphic( const BitmapEx& rBitmapEx ) :
        maEx            ( rBitmapEx ),
        meType          ( !rBitmapEx.IsEmpty() ? GraphicType::Bitmap : GraphicType::NONE ),
        mnSizeBytes     ( 0UL ),
        mbSwapOut       ( false ),
        mbDummyContext  ( false )
{
}

ImpGraphic::ImpGraphic(const SvgDataPtr& rSvgDataPtr)
:   meType( rSvgDataPtr.get() ? GraphicType::Bitmap : GraphicType::NONE ),
    mnSizeBytes( 0UL ),
    mbSwapOut( false ),
    mbDummyContext  ( false ),
    maSvgData(rSvgDataPtr)
{
}

ImpGraphic::ImpGraphic( const Animation& rAnimation ) :
        maEx            ( rAnimation.GetBitmapEx() ),
        mpAnimation     ( o3tl::make_unique<Animation>( rAnimation ) ),
        meType          ( GraphicType::Bitmap ),
        mnSizeBytes     ( 0UL ),
        mbSwapOut       ( false ),
        mbDummyContext  ( false )
{
}

ImpGraphic::ImpGraphic( const GDIMetaFile& rMtf ) :
        maMetaFile      ( rMtf ),
        meType          ( GraphicType::GdiMetafile ),
        mnSizeBytes     ( 0UL ),
        mbSwapOut       ( false ),
        mbDummyContext  ( false )
{
}

ImpGraphic::~ImpGraphic()
{
}

ImpGraphic& ImpGraphic::operator=( const ImpGraphic& rImpGraphic )
{
    if( &rImpGraphic != this )
    {
        maMetaFile = rImpGraphic.maMetaFile;
        meType = rImpGraphic.meType;
        mnSizeBytes = rImpGraphic.mnSizeBytes;

        maSwapInfo = rImpGraphic.maSwapInfo;
        mpContext = rImpGraphic.mpContext;
        mbDummyContext = rImpGraphic.mbDummyContext;

        mpAnimation.reset();

        if ( rImpGraphic.mpAnimation )
        {
            mpAnimation = o3tl::make_unique<Animation>( *rImpGraphic.mpAnimation );
            maEx = mpAnimation->GetBitmapEx();
        }
        else
        {
            maEx = rImpGraphic.maEx;
        }

        mbSwapOut = rImpGraphic.mbSwapOut;
        mpSwapFile = rImpGraphic.mpSwapFile;

        mpGfxLink.reset();

        if( rImpGraphic.mpGfxLink )
            mpGfxLink = o3tl::make_unique<GfxLink>( *rImpGraphic.mpGfxLink );

        maSvgData = rImpGraphic.maSvgData;
        maPdfData = rImpGraphic.maPdfData;
    }

    return *this;
}

ImpGraphic& ImpGraphic::operator=(ImpGraphic&& rImpGraphic)
{
    maMetaFile = std::move(rImpGraphic.maMetaFile);
    meType = rImpGraphic.meType;
    mnSizeBytes = rImpGraphic.mnSizeBytes;
    maSwapInfo = std::move(rImpGraphic.maSwapInfo);
    mpContext = std::move(rImpGraphic.mpContext);
    mbDummyContext = rImpGraphic.mbDummyContext;
    mpAnimation = std::move(rImpGraphic.mpAnimation);
    maEx = std::move(rImpGraphic.maEx);
    mbSwapOut = rImpGraphic.mbSwapOut;
    mpSwapFile = std::move(rImpGraphic.mpSwapFile);
    mpGfxLink = std::move(rImpGraphic.mpGfxLink);
    maSvgData = std::move(rImpGraphic.maSvgData);
    maPdfData = std::move(rImpGraphic.maPdfData);

    rImpGraphic.ImplClear();
    rImpGraphic.mbDummyContext = false;

    return *this;
}

bool ImpGraphic::operator==( const ImpGraphic& rImpGraphic ) const
{
    bool bRet = false;

    if( this == &rImpGraphic )
        bRet = true;
    else if( !ImplIsSwapOut() && ( rImpGraphic.meType == meType ) )
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
                if(maSvgData.get())
                {
                    if(maSvgData == rImpGraphic.maSvgData)
                    {
                        bRet = true;
                    }
                    else if(rImpGraphic.maSvgData)
                    {
                        if(maSvgData->getSvgDataArrayLength() == rImpGraphic.maSvgData->getSvgDataArrayLength())
                        {
                            if(0 == memcmp(
                                maSvgData->getSvgDataArray().getConstArray(),
                                rImpGraphic.maSvgData->getSvgDataArray().getConstArray(),
                                maSvgData->getSvgDataArrayLength()))
                            {
                                bRet = true;
                            }
                        }
                    }
                }
                else if (maPdfData.hasElements())
                {
                    bRet = maPdfData == rImpGraphic.maPdfData;
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

void ImpGraphic::ImplCreateSwapInfo()
{
    if (!ImplIsSwapOut())
    {
        maSwapInfo.maPrefMapMode = ImplGetPrefMapMode();
        maSwapInfo.maPrefSize = ImplGetPrefSize();
    }
}

void ImpGraphic::ImplClearGraphics()
{
    maEx.Clear();
    maMetaFile.Clear();
    mpAnimation.reset();
    mpGfxLink.reset();
    maSvgData.reset();
    maPdfData = uno::Sequence<sal_Int8>();
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

void ImpGraphic::ImplClear()
{
    mpSwapFile.reset();
    mbSwapOut = false;

    // cleanup
    ImplClearGraphics();
    meType = GraphicType::NONE;
    mnSizeBytes = 0;
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

    if( meType == GraphicType::Bitmap && !maSvgData.get())
    {
        bRet = ( mpAnimation ? mpAnimation->IsTransparent() : maEx.IsTransparent() );
    }

    return bRet;
}

bool ImpGraphic::ImplIsAlpha() const
{
    bool bRet(false);

    if(maSvgData.get())
    {
        bRet = true;
    }
    else if( meType == GraphicType::Bitmap )
    {
        bRet = ( nullptr == mpAnimation ) && maEx.IsAlpha();
    }

    return bRet;
}

bool ImpGraphic::ImplIsAnimated() const
{
    return( mpAnimation != nullptr );
}

bool ImpGraphic::ImplIsEPS() const
{
    return( ( meType == GraphicType::GdiMetafile ) &&
            ( maMetaFile.GetActionSize() > 0 ) &&
            ( maMetaFile.GetAction( 0 )->GetType() == MetaActionType::EPS ) );
}

Bitmap ImpGraphic::ImplGetBitmap(const GraphicConversionParameters& rParameters) const
{
    Bitmap aRetBmp;

    if( meType == GraphicType::Bitmap )
    {
        if(maSvgData.get() && maEx.IsEmpty())
        {
            // use maEx as local buffer for rendered svg
            const_cast< ImpGraphic* >(this)->maEx = maSvgData->getReplacement();
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
                double fWH((double)aDrawSize.Width() / (double)aDrawSize.Height());

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
                Rectangle aHairlineRect;
                const Rectangle aRect(maMetaFile.GetBoundRect(*aVDev.get(), &aHairlineRect));

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
                const_cast< ImpGraphic* >(this)->maEx = aVDev->GetBitmap( Point(), aVDev->GetOutputSizePixel() );
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

    if( meType == GraphicType::Bitmap )
    {
        if(maSvgData.get() && maEx.IsEmpty())
        {
            // use maEx as local buffer for rendered svg
            const_cast< ImpGraphic* >(this)->maEx = maSvgData->getReplacement();
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

    if( mpAnimation )
        aAnimation = *mpAnimation;

    return aAnimation;
}

const GDIMetaFile& ImpGraphic::ImplGetGDIMetaFile() const
{
    if (GraphicType::Bitmap == meType && !maMetaFile.GetActionSize())
    {
        // #i119735#
        // Use the local maMetaFile as container for a metafile-representation
        // of the bitmap graphic. This will be done only once, thus be buffered.
        // I checked all usages of maMetaFile, it is only used when type is not
        // GraphicType::Bitmap. In operator= it will get copied, thus buffering will
        // survive copying (change this if not wanted)
        ImpGraphic* pThat = const_cast< ImpGraphic* >(this);

        if(maSvgData.get() && !maEx)
        {
            // use maEx as local buffer for rendered svg
            pThat->maEx = maSvgData->getReplacement();
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

Size ImpGraphic::ImplGetPrefSize() const
{
    Size aSize;

    if( ImplIsSwapOut() )
        aSize = maSwapInfo.maPrefSize;
    else
    {
        switch( meType )
        {
            case GraphicType::NONE:
            case GraphicType::Default:
            break;

            case GraphicType::Bitmap:
            {
                if(maSvgData.get() && maEx.IsEmpty())
                {
                    // svg not yet buffered in maEx, return size derived from range
                    const basegfx::B2DRange& rRange = maSvgData->getRange();

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
    switch( meType )
    {
        case GraphicType::NONE:
        case GraphicType::Default:
        break;

        case GraphicType::Bitmap:
        {
            //UUUU used when importing a writer FlyFrame with SVG as graphic, added conversion
            // to allow setting the PrefSize at the BitmapEx to hold it
            if(maSvgData.get() && maEx.IsEmpty())
            {
                // use maEx as local buffer for rendered svg
                maEx = maSvgData->getReplacement();
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

    if( ImplIsSwapOut() )
        aMapMode = maSwapInfo.maPrefMapMode;
    else
    {
        switch( meType )
        {
            case GraphicType::NONE:
            case GraphicType::Default:
            break;

            case GraphicType::Bitmap:
            {
                if(maSvgData.get() && maEx.IsEmpty())
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
    switch( meType )
    {
        case GraphicType::NONE:
        case GraphicType::Default:
        break;

        case GraphicType::Bitmap:
        {
            if(maSvgData.get())
            {
                // ignore for Svg. If this is really used (except the grfcache)
                // it can be extended by using maEx as buffer for maSvgData->getReplacement()
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
        if( meType == GraphicType::Bitmap )
        {
            if(maSvgData.get())
            {
                std::pair<SvgData::State, size_t> tmp(maSvgData->getSizeBytes());
                if (SvgData::State::UNPARSED == tmp.first)
                {
                    return tmp.second; // don't cache it until SVG is parsed
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
    if( ImplIsSupportedGraphic() && !ImplIsSwapOut() )
    {
        switch( meType )
        {
            case GraphicType::Default:
            break;

            case GraphicType::Bitmap:
            {
                if(maSvgData.get() && !maEx)
                {
                    // use maEx as local buffer for rendered svg
                    const_cast< ImpGraphic* >(this)->maEx = maSvgData->getReplacement();
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
    if( ImplIsSupportedGraphic() && !ImplIsSwapOut() )
    {
        switch( meType )
        {
            case GraphicType::Default:
            break;

            case GraphicType::Bitmap:
            {
                if(maSvgData.get() && maEx.IsEmpty())
                {
                    // use maEx as local buffer for rendered svg
                    const_cast< ImpGraphic* >(this)->maEx = maSvgData->getReplacement();
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
    if( ImplIsSupportedGraphic() && !ImplIsSwapOut() && mpAnimation )
        mpAnimation->Start( pOutDev, rDestPt, rDestSize, nExtraData, pFirstFrameOutDev );
}

void ImpGraphic::ImplStopAnimation( OutputDevice* pOutDev, long nExtraData )
{
    if( ImplIsSupportedGraphic() && !ImplIsSwapOut() && mpAnimation )
        mpAnimation->Stop( pOutDev, nExtraData );
}

void ImpGraphic::ImplSetAnimationNotifyHdl( const Link<Animation*,void>& rLink )
{
    if( mpAnimation )
        mpAnimation->SetNotifyHdl( rLink );
}

Link<Animation*,void> ImpGraphic::ImplGetAnimationNotifyHdl() const
{
    Link<Animation*,void> aLink;

    if( mpAnimation )
        aLink = mpAnimation->GetNotifyHdl();

    return aLink;
}

sal_uLong ImpGraphic::ImplGetAnimationLoopCount() const
{
    return( mpAnimation ? mpAnimation->GetLoopCount() : 0UL );
}


void ImpGraphic::ImplSetContext( const std::shared_ptr<GraphicReader>& pReader )
{
    mpContext = pReader;
    mbDummyContext = false;
}

bool ImpGraphic::ImplReadEmbedded( SvStream& rIStm )
{
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
        aMapMode = MapMode( (MapUnit) nMapMode, Point( nOffsX, nOffsY ),
                            Fraction( nScaleNumX, nScaleDenomX ),
                            Fraction( nScaleNumY, nScaleDenomY ) );
    }

    meType = (GraphicType) nType;

    if( meType != GraphicType::NONE )
    {
        if( meType == GraphicType::Bitmap )
        {
            if(maSvgData.get() && maEx.IsEmpty())
            {
                // use maEx as local buffer for rendered svg
                maEx = maSvgData->getReplacement();
            }

            maEx.aBitmapSize = aSize;

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
            bRet = ( rIStm.GetError() == 0UL );
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
                bRet = ( rIStm.GetError() == 0UL );
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

            rOStm.WriteInt32( (sal_Int32)meType );

            // data size is updated later
            nDataFieldPos = rOStm.Tell();
            rOStm.WriteInt32( 0 );

            WritePair( rOStm, aSize );
            WriteMapMode( rOStm, aMapMode );
        }
        else
        {
            // write old style (<=4.0) header
            rOStm.WriteInt32( (sal_Int32)meType );

            // data size is updated later
            nDataFieldPos = rOStm.Tell();
            rOStm.WriteInt32( 0 );
            rOStm.WriteInt32( aSize.Width() );
            rOStm.WriteInt32( aSize.Height() );
            rOStm.WriteInt32( (sal_uInt16)aMapMode.GetMapUnit() );
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
                xOStm.reset(::utl::UcbStreamHelper::CreateStream( aTmpURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::READWRITE | StreamMode::SHARE_DENYWRITE ));
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
                    mpSwapFile = o3tl::make_unique<ImpSwapFile>();
                    mpSwapFile->aSwapURL = aTmpURL;
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

    return bRet;
}

void ImpGraphic::ImplSwapOutAsLink()
{
    ImplCreateSwapInfo();
    ImplClearGraphics();
    mbSwapOut = true;
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

bool ImpGraphic::ImplSwapIn()
{
    bool bRet = false;

    if( ImplIsSwapOut() )
    {
        OUString aSwapURL;

        if( mpSwapFile )
            aSwapURL = mpSwapFile->aSwapURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );

        if( !aSwapURL.isEmpty() )
        {
            std::unique_ptr<SvStream> xIStm;
            try
            {
                xIStm.reset(::utl::UcbStreamHelper::CreateStream( aSwapURL, StreamMode::READWRITE | StreamMode::SHARE_DENYWRITE ));
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

                mpSwapFile.reset();
            }
        }
    }

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

void ImpGraphic::ImplSetLink( const GfxLink& rGfxLink )
{
    mpGfxLink = o3tl::make_unique<GfxLink>( rGfxLink );

    if( mpGfxLink->IsNative() )
        mpGfxLink->SwapOut();
}

GfxLink ImpGraphic::ImplGetLink()
{
    return( mpGfxLink ? *mpGfxLink : GfxLink() );
}

bool ImpGraphic::ImplIsLink() const
{
    return ( bool(mpGfxLink) );
}

BitmapChecksum ImpGraphic::ImplGetChecksum() const
{
    BitmapChecksum nRet = 0;

    if( ImplIsSupportedGraphic() && !ImplIsSwapOut() )
    {
        switch( meType )
        {
            case GraphicType::Default:
            break;

            case GraphicType::Bitmap:
            {
                if(maSvgData.get() && maEx.IsEmpty())
                {
                    // use maEx as local buffer for rendered svg
                    const_cast< ImpGraphic* >(this)->maEx = maSvgData->getReplacement();
                }

                if( mpAnimation )
                {
                    nRet = mpAnimation->GetChecksum();
                }
                else
                {
                    nRet = maEx.GetChecksum();
                }
            }
            break;

            default:
                nRet = maMetaFile.GetChecksum();
                if (maPdfData.hasElements())
                    // Include the PDF data in the checksum, so a metafile with
                    // and without PDF data is considered to be different.
                    nRet = vcl_get_checksum(nRet, maPdfData.getConstArray(), maPdfData.getLength());
            break;
        }
    }

    return nRet;
}

bool ImpGraphic::ImplExportNative( SvStream& rOStm ) const
{
    bool bResult = false;

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
    if (rIStm.GetError() || rIStm.IsEof())
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
        aGraphic.SetLink( GfxLink() );

        if( !rIStm.GetError() && aLink.LoadNative( aGraphic ) )
        {
            // set link only, if no other link was set
            const bool bSetLink = ( !rImpGraphic.mpGfxLink );

            // assign graphic
            rImpGraphic = *aGraphic.ImplGetImpGraphic();

            if( aLink.IsPrefMapModeValid() )
                rImpGraphic.ImplSetPrefMapMode( aLink.GetPrefMapMode() );

            if( aLink.IsPrefSizeValid() )
                rImpGraphic.ImplSetPrefSize( aLink.GetPrefSize() );

            if( bSetLink )
                rImpGraphic.ImplSetLink( aLink );
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
            rImpGraphic.mpAnimation = o3tl::make_unique<Animation>();
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
            sal_uInt32 nMagic;
            rIStm.Seek(nStmPos1);
            rIStm.ResetError();
            rIStm.ReadUInt32( nMagic );

            if (nSvgMagic == nMagic)
            {
                sal_uInt32 nSvgDataArrayLength(0);
                rIStm.ReadUInt32(nSvgDataArrayLength);

                if (nSvgDataArrayLength)
                {
                    SvgDataArray aNewData(nSvgDataArrayLength);

                    rIStm.ReadBytes(aNewData.getArray(), nSvgDataArrayLength);
                    OUString aPath = rIStm.ReadUniOrByteString(rIStm.GetStreamCharSet());

                    if (!rIStm.GetError())
                    {
                        SvgDataPtr aSvgDataPtr(
                            new SvgData(
                                aNewData,
                                OUString(aPath)));

                        rImpGraphic = aSvgDataPtr;
                    }
                }
            }
            else if (nMagic == nPdfMagic)
            {
                // Stream in PDF data.
                sal_uInt32 nPdfDataLength = 0;
                rIStm.ReadUInt32(nPdfDataLength);

                if (nPdfDataLength)
                {
                    uno::Sequence<sal_Int8> aPdfData(nPdfDataLength);
                    rIStm.ReadBytes(aPdfData.getArray(), nPdfDataLength);
                    if (!rIStm.GetError())
                        rImpGraphic.maPdfData = aPdfData;
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

    if (rImpGraphic.ImplIsSwapOut())
    {
        rOStm.SetError( SVSTREAM_GENERALERROR );
        return;
    }

    if( ( rOStm.GetVersion() >= SOFFICE_FILEFORMAT_50 ) &&
        ( rOStm.GetCompressMode() & SvStreamCompressFlags::NATIVE ) &&
        rImpGraphic.mpGfxLink && rImpGraphic.mpGfxLink->IsNative() &&
        !rImpGraphic.maPdfData.hasElements())
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
                if(rImpGraphic.getSvgData().get())
                {
                    // stream out Svg defining data (length, byte array and evtl. path)
                    // this is used e.g. in swapping out graphic data and in transporting it over UNO API
                    // as sequence of bytes, but AFAIK not written anywhere to any kind of file, so it should be
                    // no problem to extend it; only used at runtime
                    const sal_uInt32 nSvgMagic((sal_uInt32('s') << 24) | (sal_uInt32('v') << 16) | (sal_uInt32('g') << 8) | sal_uInt32('0'));

                    rOStm.WriteUInt32( nSvgMagic );
                    rOStm.WriteUInt32( rImpGraphic.getSvgData()->getSvgDataArrayLength() );
                    rOStm.WriteBytes(rImpGraphic.getSvgData()->getSvgDataArray().getConstArray(),
                        rImpGraphic.getSvgData()->getSvgDataArrayLength());
                    rOStm.WriteUniOrByteString(rImpGraphic.getSvgData()->getPath(),
                                               rOStm.GetStreamCharSet());
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
                if (rImpGraphic.maPdfData.hasElements())
                {
                    // Stream out PDF data.
                    rOStm.WriteUInt32(nPdfMagic);
                    rOStm.WriteUInt32(rImpGraphic.maPdfData.getLength());
                    rOStm.WriteBytes(rImpGraphic.maPdfData.getConstArray(), rImpGraphic.maPdfData.getLength());
                }
                if( rImpGraphic.ImplIsSupportedGraphic() )
                    WriteGDIMetaFile( rOStm, rImpGraphic.maMetaFile );
            }
            break;
        }

        rOStm.SetEndian( nOldFormat );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
