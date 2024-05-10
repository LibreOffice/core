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

#include <comphelper/base64.hxx>
#include <comphelper/graphicmimetype.hxx>
#include <tools/debug.hxx>
#include <vcl/virdev.hxx>
#include <sfx2/docfile.hxx>
#include <svx/xoutbmp.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/cvtgrf.hxx>
#include <memory>

#include <com/sun/star/beans/XPropertySet.hpp>

constexpr OUStringLiteral FORMAT_SVG = u"svg";
constexpr OUStringLiteral FORMAT_WMF = u"wmf";
constexpr OUString FORMAT_EMF = u"emf"_ustr;
constexpr OUStringLiteral FORMAT_PDF = u"pdf";

constexpr OUString FORMAT_BMP = u"bmp"_ustr;
constexpr OUString FORMAT_GIF = u"gif"_ustr;
constexpr OUStringLiteral FORMAT_JPG = u"jpg";
constexpr OUString FORMAT_PNG = u"png"_ustr;
constexpr OUStringLiteral FORMAT_TIF = u"tif";
constexpr OUStringLiteral FORMAT_WEBP = u"webp";

using namespace com::sun::star;

Animation XOutBitmap::MirrorAnimation( const Animation& rAnimation, bool bHMirr, bool bVMirr )
{
    Animation aNewAnim( rAnimation );

    if( bHMirr || bVMirr )
    {
        const Size& rGlobalSize = aNewAnim.GetDisplaySizePixel();
        BmpMirrorFlags nMirrorFlags = BmpMirrorFlags::NONE;

        if( bHMirr )
            nMirrorFlags |= BmpMirrorFlags::Horizontal;

        if( bVMirr )
            nMirrorFlags |= BmpMirrorFlags::Vertical;

        for( sal_uInt16 i = 0, nCount = aNewAnim.Count(); i < nCount; i++ )
        {
            AnimationFrame aAnimationFrame( aNewAnim.Get( i ) );

            // mirror the BitmapEx
            aAnimationFrame.maBitmapEx.Mirror( nMirrorFlags );

            // Adjust the positions inside the whole bitmap
            if( bHMirr )
                aAnimationFrame.maPositionPixel.setX(rGlobalSize.Width() - aAnimationFrame.maPositionPixel.X() -
                                       aAnimationFrame.maSizePixel.Width());

            if( bVMirr )
                aAnimationFrame.maPositionPixel.setY(rGlobalSize.Height() - aAnimationFrame.maPositionPixel.Y() -
                                       aAnimationFrame.maSizePixel.Height());

            aNewAnim.Replace(aAnimationFrame, i);
        }
    }

    return aNewAnim;
}

Graphic XOutBitmap::MirrorGraphic( const Graphic& rGraphic, const BmpMirrorFlags nMirrorFlags )
{
    Graphic aRetGraphic;

    if( nMirrorFlags != BmpMirrorFlags::NONE )
    {
        if( rGraphic.IsAnimated() )
        {
            aRetGraphic = MirrorAnimation( rGraphic.GetAnimation(),
                                           bool( nMirrorFlags & BmpMirrorFlags::Horizontal ),
                                           bool( nMirrorFlags & BmpMirrorFlags::Vertical ) );
        }
        else
        {
            BitmapEx aBmp( rGraphic.GetBitmapEx() );
            aBmp.Mirror( nMirrorFlags );
            aRetGraphic = aBmp;
        }
    }
    else
        aRetGraphic = rGraphic;

    return aRetGraphic;
}

static OUString match(std::u16string_view filter, const OUString& expected, bool matchEmpty = true)
{
    return (matchEmpty && filter.empty()) || expected.equalsIgnoreAsciiCase(filter) ? expected
                                                                                    : OUString();
}

static OUString isKnownVectorFormat(const Graphic& rGraphic, std::u16string_view rFilter)
{
    const auto& pData(rGraphic.getVectorGraphicData());
    if (!pData || pData->getBinaryDataContainer().getSize() == 0)
        return {};

    // Does the filter name match the original format?
    switch (pData->getType())
    {
        case VectorGraphicDataType::Svg:
            return match(rFilter, FORMAT_SVG, false);
        case VectorGraphicDataType::Wmf:
            return match(rFilter, FORMAT_WMF, false);
        case VectorGraphicDataType::Emf:
            return match(rFilter, FORMAT_EMF, false);
        case VectorGraphicDataType::Pdf:
            return match(rFilter, FORMAT_PDF, false);
    }

    if (rGraphic.GetGfxLink().IsEMF())
        return match(rFilter, FORMAT_EMF, false);

    return {};
}

static OUString isKnownRasterFormat(const GfxLink& rLink, std::u16string_view rFilter)
{
    // tdf#60684: use native format if possible but it must correspond to filter name
    // or no specific format has been required
    // without this, you may save for example file with png extension but jpg content
    switch (rLink.GetType())
    {
        case GfxLinkType::NativeGif:
            return match(rFilter, FORMAT_GIF);

        // #i15508# added BMP type for better exports (no call/trigger found, prob used in HTML export)
        case GfxLinkType::NativeBmp:
            return match(rFilter, FORMAT_BMP);

        case GfxLinkType::NativeJpg:
            return match(rFilter, FORMAT_JPG);
        case GfxLinkType::NativePng:
            return match(rFilter, FORMAT_PNG);
        case GfxLinkType::NativeTif:
            return match(rFilter, FORMAT_TIF);
        case GfxLinkType::NativeWebp:
            return match(rFilter, FORMAT_WEBP);
        default:
            return {};
    }
}

ErrCode XOutBitmap::WriteGraphic( const Graphic& rGraphic, OUString& rFileName,
                                 const OUString& rFilterName, const XOutFlags nFlags,
                                 const Size* pMtfSize_100TH_MM,
                                 const css::uno::Sequence< css::beans::PropertyValue >* pFilterData,
                                 OUString* pMediaType )
{
    if( rGraphic.GetType() == GraphicType::NONE )
        return ERRCODE_NONE;

    INetURLObject   aURL( rFileName );
    GraphicFilter&  rFilter = GraphicFilter::GetGraphicFilter();

    DBG_ASSERT( aURL.GetProtocol() != INetProtocol::NotValid, "XOutBitmap::WriteGraphic(...): invalid URL" );

    // calculate correct file name
    if( !( nFlags & XOutFlags::DontExpandFilename ) )
    {
        OUString aStr( OUString::number( rGraphic.GetChecksum(), 16 ) );
        if ( aStr[0] == '-' )
            aStr = OUString::Concat("m") + aStr.subView(1);
        OUString aName = aURL.getBase() + "_" + aURL.getExtension() + "_" + aStr;
        aURL.setBase( aName );
    }

    // #i121128# use shortcut to write Vector Graphic Data data in original form (if possible)
    if (OUString aExt = isKnownVectorFormat(rGraphic, rFilterName); !aExt.isEmpty())
    {
        if (!(nFlags & XOutFlags::DontAddExtension))
            aURL.setExtension(aExt);

        rFileName = aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE);
        if (pMediaType)
            if (auto xGraphic = rGraphic.GetXGraphic().query<css::beans::XPropertySet>())
                xGraphic->getPropertyValue(u"MimeType"_ustr) >>= *pMediaType;

        SfxMedium aMedium(aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE), StreamMode::WRITE | StreamMode::SHARE_DENYNONE | StreamMode::TRUNC);
        SvStream* pOStm = aMedium.GetOutStream();

        if (pOStm)
        {
            rGraphic.getVectorGraphicData()->getBinaryDataContainer().writeToStream(*pOStm);
            aMedium.Commit();

            if (!aMedium.GetErrorIgnoreWarning())
                return ERRCODE_NONE;
        }
    }

    if( ( nFlags & XOutFlags::UseNativeIfPossible ) &&
        !( nFlags & XOutFlags::MirrorHorz ) &&
        !( nFlags & XOutFlags::MirrorVert ) &&
        ( rGraphic.GetType() != GraphicType::GdiMetafile ) && rGraphic.IsGfxLink() )
    {
        // try to write native link
        const GfxLink aGfxLink( rGraphic.GetGfxLink() );
        if (OUString aExt = isKnownRasterFormat(aGfxLink, rFilterName); !aExt.isEmpty())
        {
            if( !(nFlags & XOutFlags::DontAddExtension) )
                aURL.setExtension( aExt );
            rFileName = aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );
            if (pMediaType)
                if (auto xGraphic = rGraphic.GetXGraphic().query<css::beans::XPropertySet>())
                    xGraphic->getPropertyValue(u"MimeType"_ustr) >>= *pMediaType;

            SfxMedium   aMedium(aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE), StreamMode::WRITE | StreamMode::SHARE_DENYNONE | StreamMode::TRUNC);
            SvStream*   pOStm = aMedium.GetOutStream();

            if( pOStm && aGfxLink.GetDataSize() && aGfxLink.GetData() )
            {
                pOStm->WriteBytes(aGfxLink.GetData(), aGfxLink.GetDataSize());
                aMedium.Commit();

                if( !aMedium.GetErrorIgnoreWarning() )
                    return ERRCODE_NONE;
            }
        }
    }

    OUString  aFilter( rFilterName );
    bool bTransparent = rGraphic.IsTransparent(), bAnimated = rGraphic.IsAnimated();
    bool    bWriteTransGrf = ( aFilter.equalsIgnoreAsciiCase( "transgrf" ) ) ||
                                ( aFilter.equalsIgnoreAsciiCase( "gif" ) ) ||
                                ( nFlags & XOutFlags::UseGifIfPossible ) ||
                                ( ( nFlags & XOutFlags::UseGifIfSensible ) && ( bAnimated || bTransparent ) );

    // get filter and extension
    if( bWriteTransGrf )
        aFilter = FORMAT_GIF;

    sal_uInt16 nFilter = rFilter.GetExportFormatNumberForShortName( aFilter );

    if( GRFILTER_FORMAT_NOTFOUND == nFilter )
    {
        nFilter = rFilter.GetExportFormatNumberForShortName( FORMAT_PNG );

        if( GRFILTER_FORMAT_NOTFOUND == nFilter )
            nFilter = rFilter.GetExportFormatNumberForShortName( FORMAT_BMP );
    }

    if( GRFILTER_FORMAT_NOTFOUND != nFilter )
    {
        Graphic aGraphic;
        OUString aExt = rFilter.GetExportFormatShortName( nFilter ).toAsciiLowerCase();

        if( bWriteTransGrf )
        {
            if( bAnimated  )
                aGraphic = rGraphic;
            else
            {
                if( pMtfSize_100TH_MM && ( rGraphic.GetType() != GraphicType::Bitmap ) )
                {
                    ScopedVclPtrInstance< VirtualDevice > pVDev;
                    const Size aSize(pVDev->LogicToPixel(*pMtfSize_100TH_MM, MapMode(MapUnit::Map100thMM)));

                    if( pVDev->SetOutputSizePixel( aSize ) )
                    {
                        const Wallpaper aWallpaper( pVDev->GetBackground() );
                        const Point     aPt;

                        pVDev->SetBackground( Wallpaper( COL_BLACK ) );
                        pVDev->Erase();
                        rGraphic.Draw(*pVDev, aPt, aSize);

                        const Bitmap aBitmap( pVDev->GetBitmap( aPt, aSize ) );

                        pVDev->SetBackground( aWallpaper );
                        pVDev->Erase();
                        rGraphic.Draw(*pVDev, aPt, aSize);

                        pVDev->SetRasterOp( RasterOp::Xor );
                        pVDev->DrawBitmap( aPt, aSize, aBitmap );
                        aGraphic = BitmapEx( aBitmap, pVDev->GetBitmap( aPt, aSize ) );
                    }
                    else
                        aGraphic = rGraphic.GetBitmapEx();
                }
                else
                    aGraphic = rGraphic.GetBitmapEx();
            }
        }
        else
        {
            if (bAnimated)
                aGraphic = rGraphic;
            else if( pMtfSize_100TH_MM && ( rGraphic.GetType() != GraphicType::Bitmap ) )
            {
                ScopedVclPtrInstance< VirtualDevice > pVDev;
                const Size aSize(pVDev->LogicToPixel(*pMtfSize_100TH_MM, MapMode(MapUnit::Map100thMM)));

                if( pVDev->SetOutputSizePixel( aSize ) )
                {
                    rGraphic.Draw(*pVDev, Point(), aSize);
                    aGraphic = BitmapEx(pVDev->GetBitmap(Point(), aSize));
                }
                else
                    aGraphic = rGraphic.GetBitmapEx();
            }
            else
                aGraphic = rGraphic.GetBitmapEx();
        }

        // mirror?
        if( ( nFlags & XOutFlags::MirrorHorz ) || ( nFlags & XOutFlags::MirrorVert ) )
        {
            BmpMirrorFlags nBmpMirrorFlags = BmpMirrorFlags::NONE;
            if( nFlags & XOutFlags::MirrorHorz )
                nBmpMirrorFlags |= BmpMirrorFlags::Horizontal;
            if( nFlags & XOutFlags::MirrorVert )
                nBmpMirrorFlags |= BmpMirrorFlags::Vertical;
            aGraphic = MirrorGraphic( aGraphic, nBmpMirrorFlags );
        }

        if (aGraphic.GetType() != GraphicType::NONE)
        {
            if( !(nFlags & XOutFlags::DontAddExtension) )
                aURL.setExtension( aExt );
            rFileName = aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );
            if (pMediaType)
                *pMediaType = rFilter.GetExportFormatMediaType(nFilter);
            return ExportGraphic( aGraphic, aURL, rFilter, nFilter, pFilterData );
        }
    }

    return ERRCODE_GRFILTER_FILTERERROR;
}

bool XOutBitmap::GraphicToBase64(const Graphic& rGraphic, OUString& rOUString, bool bAddPrefix,
                                 ConvertDataFormat aTargetFormat)
{
    SvMemoryStream aOStm;
    GfxLink aLink = rGraphic.GetGfxLink();

    if (aTargetFormat == ConvertDataFormat::Unknown)
    {
        switch (aLink.GetType())
        {
            case GfxLinkType::NativeJpg:
                aTargetFormat = ConvertDataFormat::JPG;
                break;
            case GfxLinkType::NativePng:
                aTargetFormat = ConvertDataFormat::PNG;
                break;
            case GfxLinkType::NativeSvg:
                aTargetFormat = ConvertDataFormat::SVG;
                break;
            default:
                // save everything else (including gif) into png
                aTargetFormat = ConvertDataFormat::PNG;
                break;
        }
    }

    ErrCode nErr = GraphicConverter::Export(aOStm,rGraphic,aTargetFormat);
    if ( nErr )
    {
        SAL_WARN("svx", "XOutBitmap::GraphicToBase64() invalid Graphic? error: " << nErr );
        return false;
    }
    css::uno::Sequence<sal_Int8> aOStmSeq( static_cast<sal_Int8 const *>(aOStm.GetData()),aOStm.TellEnd() );
    OUStringBuffer aStrBuffer;
    ::comphelper::Base64::encode(aStrBuffer,aOStmSeq);
    rOUString = aStrBuffer.makeStringAndClear();

    if (bAddPrefix)
    {
        OUString aMimeType
            = comphelper::GraphicMimeTypeHelper::GetMimeTypeForConvertDataFormat(aTargetFormat);
        rOUString = aMimeType + ";base64," + rOUString;
    }

    return true;
}

ErrCode XOutBitmap::ExportGraphic( const Graphic& rGraphic, const INetURLObject& rURL,
                                  GraphicFilter& rFilter, const sal_uInt16 nFormat,
                                  const css::uno::Sequence< css::beans::PropertyValue >* pFilterData )
{
    DBG_ASSERT( rURL.GetProtocol() != INetProtocol::NotValid, "XOutBitmap::ExportGraphic(...): invalid URL" );

    SfxMedium   aMedium( rURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::WRITE | StreamMode::SHARE_DENYNONE | StreamMode::TRUNC );
    SvStream*   pOStm = aMedium.GetOutStream();
    ErrCode     nRet = ERRCODE_GRFILTER_IOERROR;

    if( pOStm )
    {
        nRet = rFilter.ExportGraphic( rGraphic, rURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), *pOStm, nFormat, pFilterData );

        aMedium.Commit();

        if( aMedium.GetErrorIgnoreWarning() && ( ERRCODE_NONE == nRet  ) )
            nRet = ERRCODE_GRFILTER_IOERROR;
    }

    return nRet;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
