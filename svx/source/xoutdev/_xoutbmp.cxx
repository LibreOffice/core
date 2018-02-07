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

#include <tools/poly.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/virdev.hxx>
#include <vcl/wrkwin.hxx>
#include <svl/solar.hrc>
#include <sfx2/docfile.hxx>
#include <sfx2/app.hxx>
#include <svx/xoutbmp.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/FilterConfigItem.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/cvtgrf.hxx>
#include <sax/tools/converter.hxx>
#include <memory>

#define FORMAT_BMP  "bmp"
#define FORMAT_GIF  "gif"
#define FORMAT_JPG  "jpg"
#define FORMAT_PNG  "png"

using namespace com::sun::star;

GraphicFilter* XOutBitmap::pGrfFilter = nullptr;

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
            AnimationBitmap aAnimBmp( aNewAnim.Get( i ) );

            // mirror the BitmapEx
            aAnimBmp.aBmpEx.Mirror( nMirrorFlags );

            // Adjust the positions inside the whole bitmap
            if( bHMirr )
                aAnimBmp.aPosPix.X() = rGlobalSize.Width() - aAnimBmp.aPosPix.X() -
                                       aAnimBmp.aSizePix.Width();

            if( bVMirr )
                aAnimBmp.aPosPix.Y() = rGlobalSize.Height() - aAnimBmp.aPosPix.Y() -
                                       aAnimBmp.aSizePix.Height();

            aNewAnim.Replace( aAnimBmp, i );
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
            if( rGraphic.IsTransparent() )
            {
                BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

                aBmpEx.Mirror( nMirrorFlags );
                aRetGraphic = aBmpEx;
            }
            else
            {
                Bitmap aBmp( rGraphic.GetBitmap() );

                aBmp.Mirror( nMirrorFlags );
                aRetGraphic = aBmp;
            }
        }
    }
    else
        aRetGraphic = rGraphic;

    return aRetGraphic;
}

ErrCode XOutBitmap::WriteGraphic( const Graphic& rGraphic, OUString& rFileName,
                                 const OUString& rFilterName, const XOutFlags nFlags,
                                 const Size* pMtfSize_100TH_MM,
                                 const css::uno::Sequence< css::beans::PropertyValue >* pFilterData )
{
    if( rGraphic.GetType() != GraphicType::NONE )
    {
        INetURLObject   aURL( rFileName );
        Graphic         aGraphic;
        OUString        aExt;
        GraphicFilter&  rFilter = GraphicFilter::GetGraphicFilter();
        ErrCode         nErr = ERRCODE_GRFILTER_FILTERERROR;
        sal_uInt16      nFilter = GRFILTER_FORMAT_NOTFOUND;
        bool            bTransparent = rGraphic.IsTransparent(), bAnimated = rGraphic.IsAnimated();

        DBG_ASSERT( aURL.GetProtocol() != INetProtocol::NotValid, "XOutBitmap::WriteGraphic(...): invalid URL" );

        // calculate correct file name
        if( !( nFlags & XOutFlags::DontExpandFilename ) )
        {
            OUString aName( aURL.getBase() );
            aName += "_";
            aName += aURL.getExtension();
            aName += "_";
            OUString aStr( OUString::number( rGraphic.GetChecksum(), 16 ) );
            if ( aStr[0] == '-' )
                aStr = "m" + aStr.copy(1);
            aName += aStr;
            aURL.setBase( aName );
        }

        // #i121128# use shortcut to write Vector Graphic Data data in original form (if possible)
        const VectorGraphicDataPtr& aVectorGraphicDataPtr(rGraphic.getVectorGraphicData());

        if(aVectorGraphicDataPtr.get()
            && aVectorGraphicDataPtr->getVectorGraphicDataArrayLength())
        {
            const bool bIsSvg(rFilterName.equalsIgnoreAsciiCase("svg") && VectorGraphicDataType::Svg == aVectorGraphicDataPtr->getVectorGraphicDataType());
            const bool bIsWmf(rFilterName.equalsIgnoreAsciiCase("wmf") && VectorGraphicDataType::Wmf == aVectorGraphicDataPtr->getVectorGraphicDataType());
            const bool bIsEmf(rFilterName.equalsIgnoreAsciiCase("emf") && VectorGraphicDataType::Emf == aVectorGraphicDataPtr->getVectorGraphicDataType());

            if (bIsSvg || bIsWmf || bIsEmf)
            {
                if (!(nFlags & XOutFlags::DontAddExtension))
                {
                    aURL.setExtension(rFilterName);
                }

                rFileName = aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE);
                SfxMedium aMedium(aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE), StreamMode::WRITE | StreamMode::SHARE_DENYNONE | StreamMode::TRUNC);
                SvStream* pOStm = aMedium.GetOutStream();

                if (pOStm)
                {
                    pOStm->WriteBytes(aVectorGraphicDataPtr->getVectorGraphicDataArray().getConstArray(), aVectorGraphicDataPtr->getVectorGraphicDataArrayLength());
                    aMedium.Commit();

                    if (!aMedium.GetError())
                    {
                        nErr = ERRCODE_NONE;
                    }
                }
            }
        }

        // Write PDF data in original form if possible.
        if (rGraphic.getPdfData().hasElements() && rFilterName.equalsIgnoreAsciiCase("pdf"))
        {
            if (!(nFlags & XOutFlags::DontAddExtension))
                aURL.setExtension(rFilterName);

            rFileName = aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE);
            SfxMedium aMedium(aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE), StreamMode::WRITE|StreamMode::SHARE_DENYNONE|StreamMode::TRUNC);
            if (SvStream* pOutStream = aMedium.GetOutStream())
            {
                uno::Sequence<sal_Int8> aPdfData = rGraphic.getPdfData();
                pOutStream->WriteBytes(aPdfData.getConstArray(), aPdfData.getLength());
                aMedium.Commit();
                if (!aMedium.GetError())
                    nErr = ERRCODE_NONE;
            }
        }

        if( ERRCODE_NONE != nErr )
        {
            if( ( nFlags & XOutFlags::UseNativeIfPossible ) &&
                !( nFlags & XOutFlags::MirrorHorz ) &&
                !( nFlags & XOutFlags::MirrorVert ) &&
                ( rGraphic.GetType() != GraphicType::GdiMetafile ) && rGraphic.IsLink() )
            {
                // try to write native link
                const GfxLink aGfxLink( rGraphic.GetLink() );

                switch( aGfxLink.GetType() )
                {
                    case GfxLinkType::NativeGif: aExt = FORMAT_GIF; break;

                    // #i15508# added BMP type for better exports (no call/trigger found, prob used in HTML export)
                    case GfxLinkType::NativeBmp: aExt = FORMAT_BMP; break;

                    case GfxLinkType::NativeJpg: aExt = FORMAT_JPG; break;
                    case GfxLinkType::NativePng: aExt = FORMAT_PNG; break;

                    default:
                    break;
                }

                if( !aExt.isEmpty() )
                {
                    if( !(nFlags & XOutFlags::DontAddExtension) )
                        aURL.setExtension( aExt );
                    rFileName = aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );

                    SfxMedium   aMedium(aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE), StreamMode::WRITE | StreamMode::SHARE_DENYNONE | StreamMode::TRUNC);
                    SvStream*   pOStm = aMedium.GetOutStream();

                    if( pOStm && aGfxLink.GetDataSize() && aGfxLink.GetData() )
                    {
                        pOStm->WriteBytes(aGfxLink.GetData(), aGfxLink.GetDataSize());
                        aMedium.Commit();

                        if( !aMedium.GetError() )
                            nErr = ERRCODE_NONE;
                    }
                }
            }
        }

        if( ERRCODE_NONE != nErr )
        {
            OUString  aFilter( rFilterName );
            bool    bWriteTransGrf = ( aFilter.equalsIgnoreAsciiCase( "transgrf" ) ) ||
                                     ( aFilter.equalsIgnoreAsciiCase( "gif" ) ) ||
                                     ( nFlags & XOutFlags::UseGifIfPossible ) ||
                                     ( ( nFlags & XOutFlags::UseGifIfSensible ) && ( bAnimated || bTransparent ) );

            // get filter and extension
            if( bWriteTransGrf )
                aFilter = FORMAT_GIF;

            nFilter = rFilter.GetExportFormatNumberForShortName( aFilter );

            if( GRFILTER_FORMAT_NOTFOUND == nFilter )
            {
                nFilter = rFilter.GetExportFormatNumberForShortName( FORMAT_PNG );

                if( GRFILTER_FORMAT_NOTFOUND == nFilter )
                    nFilter = rFilter.GetExportFormatNumberForShortName( FORMAT_BMP );
            }

            if( GRFILTER_FORMAT_NOTFOUND != nFilter )
            {
                aExt = rFilter.GetExportFormatShortName( nFilter ).toAsciiLowerCase();

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

                                pVDev->SetBackground( Wallpaper( Color( COL_BLACK ) ) );
                                pVDev->Erase();
                                rGraphic.Draw( pVDev.get(), aPt, aSize );

                                const Bitmap aBitmap( pVDev->GetBitmap( aPt, aSize ) );

                                pVDev->SetBackground( aWallpaper );
                                pVDev->Erase();
                                rGraphic.Draw( pVDev.get(), aPt, aSize );

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
                    if( pMtfSize_100TH_MM && ( rGraphic.GetType() != GraphicType::Bitmap ) )
                    {
                        ScopedVclPtrInstance< VirtualDevice > pVDev;
                        const Size aSize(pVDev->LogicToPixel(*pMtfSize_100TH_MM, MapMode(MapUnit::Map100thMM)));

                        if( pVDev->SetOutputSizePixel( aSize ) )
                        {
                            rGraphic.Draw( pVDev.get(), Point(), aSize );
                            aGraphic = pVDev->GetBitmap( Point(), aSize );
                        }
                        else
                            aGraphic = rGraphic.GetBitmap();
                    }
                    else
                        aGraphic = rGraphic.GetBitmap();
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

                if( ( GRFILTER_FORMAT_NOTFOUND != nFilter ) && ( aGraphic.GetType() != GraphicType::NONE ) )
                {
                    if( !(nFlags & XOutFlags::DontAddExtension) )
                        aURL.setExtension( aExt );
                    rFileName = aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );
                    nErr = ExportGraphic( aGraphic, aURL, rFilter, nFilter, pFilterData );
                }
            }
        }

        return nErr;
    }
    else
    {
        return ERRCODE_NONE;
    }
}

bool XOutBitmap::GraphicToBase64(const Graphic& rGraphic, OUString& rOUString)
{
    SvMemoryStream aOStm;
    OUString aMimeType;
    GfxLink aLink = rGraphic.GetLink();
    ConvertDataFormat aCvtType;
    switch(  aLink.GetType() )
    {
        case GfxLinkType::NativeJpg:
            aCvtType = ConvertDataFormat::JPG;
            aMimeType = "image/jpeg";
            break;
        case GfxLinkType::NativePng:
            aCvtType = ConvertDataFormat::PNG;
            aMimeType = "image/png";
            break;
        case GfxLinkType::NativeSvg:
            aCvtType = ConvertDataFormat::SVG;
            aMimeType = "image/svg+xml";
            break;
        default:
            // save everything else (including gif) into png
            aCvtType = ConvertDataFormat::PNG;
            aMimeType = "image/png";
            break;
    }
    ErrCode nErr = GraphicConverter::Export(aOStm,rGraphic,aCvtType);
    if ( nErr )
    {
        SAL_WARN("svx", "XOutBitmap::GraphicToBase64() invalid Graphic? error: " << nErr );
        return false;
    }
    aOStm.Seek(STREAM_SEEK_TO_END);
    css::uno::Sequence<sal_Int8> aOStmSeq( static_cast<sal_Int8 const *>(aOStm.GetData()),aOStm.Tell() );
    OUStringBuffer aStrBuffer;
    ::sax::Converter::encodeBase64(aStrBuffer,aOStmSeq);
    OUString aEncodedBase64Image = aStrBuffer.makeStringAndClear();
    if( aLink.GetType() == GfxLinkType::NativeSvg )
    {
      sal_Int32 ite(8);
      sal_Int32 nBufferLength(aOStmSeq.getLength());
      const sal_Int8* pBuffer = aOStmSeq.getConstArray();
      css::uno::Sequence<sal_Int8> newTempSeq = aOStmSeq;        // creates new Sequence to remove front 8 bits of garbage and encodes in base64
      sal_Int8 *pOutBuffer = newTempSeq.getArray();
      while(ite < nBufferLength)
      {
        *pOutBuffer++ = pBuffer[ite];
        ite++;
      }
      ::sax::Converter::encodeBase64(aStrBuffer, newTempSeq);
      aEncodedBase64Image = aStrBuffer.makeStringAndClear();
      sal_Int32 SVGFixLength = aEncodedBase64Image.getLength();
      aEncodedBase64Image = aEncodedBase64Image.replaceAt(SVGFixLength - 12, SVGFixLength, "") + "PC9zdmc+Cg=="; // removes rear 12 bits of garbage and adds svg closing tag in base64
    }
    rOUString = aMimeType + ";base64," + aEncodedBase64Image;
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
        pGrfFilter = &rFilter;

        nRet = rFilter.ExportGraphic( rGraphic, rURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), *pOStm, nFormat, pFilterData );

        pGrfFilter = nullptr;
        aMedium.Commit();

        if( aMedium.GetError() && ( ERRCODE_NONE == nRet  ) )
            nRet = ERRCODE_GRFILTER_IOERROR;
    }

    return nRet;
}

Bitmap XOutBitmap::DetectEdges( const Bitmap& rBmp, const sal_uInt8 cThreshold )
{
    const Size  aSize( rBmp.GetSizePixel() );
    Bitmap      aRetBmp;

    if( ( aSize.Width() > 2 ) && ( aSize.Height() > 2 ) )
    {
        Bitmap aWorkBmp( rBmp );

        if( aWorkBmp.Convert( BmpConversion::N8BitGreys ) )
        {
            bool bRet = false;

            Bitmap              aDstBmp( aSize, 1 );
            Bitmap::ScopedReadAccess pReadAcc(aWorkBmp);
            Bitmap::ScopedWriteAccess pWriteAcc(aDstBmp);

            if( pReadAcc && pWriteAcc )
            {
                const long          nWidth = aSize.Width();
                const long          nWidth2 = nWidth - 2;
                const long          nHeight = aSize.Height();
                const long          nHeight2 = nHeight - 2;
                const long          lThres2 = static_cast<long>(cThreshold) * cThreshold;
                const sal_uInt8 nWhitePalIdx(static_cast< sal_uInt8 >(pWriteAcc->GetBestPaletteIndex(Color(COL_WHITE))));
                const sal_uInt8 nBlackPalIdx(static_cast< sal_uInt8 >(pWriteAcc->GetBestPaletteIndex(Color(COL_BLACK))));
                long                nSum1;
                long                nSum2;
                long                lGray;

                // initialize border with white pixels
                pWriteAcc->SetLineColor( Color( COL_WHITE) );
                pWriteAcc->DrawLine( Point(), Point( nWidth - 1, 0L ) );
                pWriteAcc->DrawLine( Point( nWidth - 1, 0L ), Point( nWidth - 1, nHeight - 1 ) );
                pWriteAcc->DrawLine( Point( nWidth - 1, nHeight - 1 ), Point( 0L, nHeight - 1 ) );
                pWriteAcc->DrawLine( Point( 0, nHeight - 1 ), Point() );

                for( long nY = 0, nY1 = 1, nY2 = 2; nY < nHeight2; nY++, nY1++, nY2++ )
                {
                    Scanline pScanline = pWriteAcc->GetScanline( nY1 );
                    Scanline pScanlineRead = pReadAcc->GetScanline( nY );
                    Scanline pScanlineRead1 = pReadAcc->GetScanline( nY1 );
                    Scanline pScanlineRead2 = pReadAcc->GetScanline( nY2 );
                    for( long nX = 0, nXDst = 1, nXTmp; nX < nWidth2; nX++, nXDst++ )
                    {
                        nXTmp = nX;

                        nSum1 = -( nSum2 = lGray = pReadAcc->GetIndexFromData( pScanlineRead, nXTmp++ ) );
                        nSum2 += static_cast<long>(pReadAcc->GetIndexFromData( pScanlineRead, nXTmp++ )) << 1;
                        nSum1 += ( lGray = pReadAcc->GetIndexFromData( pScanlineRead, nXTmp ) );
                        nSum2 += lGray;

                        nSum1 += static_cast<long>(pReadAcc->GetIndexFromData( pScanlineRead1, nXTmp )) << 1;
                        nSum1 -= static_cast<long>(pReadAcc->GetIndexFromData( pScanlineRead1, nXTmp -= 2 )) << 1;

                        nSum1 += ( lGray = -static_cast<long>(pReadAcc->GetIndexFromData( pScanlineRead2, nXTmp++ )) );
                        nSum2 += lGray;
                        nSum2 -= static_cast<long>(pReadAcc->GetIndexFromData( pScanlineRead2, nXTmp++ )) << 1;
                        nSum1 += ( lGray = static_cast<long>(pReadAcc->GetIndexFromData( pScanlineRead2, nXTmp )) );
                        nSum2 -= lGray;

                        if( ( nSum1 * nSum1 + nSum2 * nSum2 ) < lThres2 )
                            pWriteAcc->SetPixelOnData( pScanline, nXDst, BitmapColor(nWhitePalIdx) );
                        else
                            pWriteAcc->SetPixelOnData( pScanline, nXDst, BitmapColor(nBlackPalIdx) );
                    }
                }

                bRet = true;
            }

            pReadAcc.reset();
            pWriteAcc.reset();

            if( bRet )
                aRetBmp = aDstBmp;
        }
    }

    if( !aRetBmp )
        aRetBmp = rBmp;
    else
    {
        aRetBmp.SetPrefMapMode( rBmp.GetPrefMapMode() );
        aRetBmp.SetPrefSize( rBmp.GetPrefSize() );
    }

    return aRetBmp;
}

tools::Polygon XOutBitmap::GetContour( const Bitmap& rBmp, const XOutFlags nFlags,
                                        const sal_uInt8 cEdgeDetectThreshold,
                                        const tools::Rectangle* pWorkRectPixel )
{
    Bitmap      aWorkBmp;
    tools::Polygon aRetPoly;
    Point       aTmpPoint;
    tools::Rectangle   aWorkRect( aTmpPoint, rBmp.GetSizePixel() );

    if( pWorkRectPixel )
        aWorkRect.Intersection( *pWorkRectPixel );

    aWorkRect.Justify();

    if( ( aWorkRect.GetWidth() > 4 ) && ( aWorkRect.GetHeight() > 4 ) )
    {
        // if the flag is set, we need to detect edges
        if( nFlags & XOutFlags::ContourEdgeDetect )
            aWorkBmp = DetectEdges( rBmp, cEdgeDetectThreshold );
        else
            aWorkBmp = rBmp;

        BitmapReadAccess* pAcc = aWorkBmp.AcquireReadAccess();

        const long nWidth = pAcc ? pAcc->Width() : 0;
        const long nHeight = pAcc ? pAcc->Height() : 0;

        if (pAcc && nWidth && nHeight)
        {
            const Size&         rPrefSize = aWorkBmp.GetPrefSize();
            const double        fFactorX = static_cast<double>(rPrefSize.Width()) / nWidth;
            const double        fFactorY = static_cast<double>(rPrefSize.Height()) / nHeight;
            const long          nStartX1 = aWorkRect.Left() + 1;
            const long          nEndX1 = aWorkRect.Right();
            const long          nStartX2 = nEndX1 - 1;
            const long          nStartY1 = aWorkRect.Top() + 1;
            const long          nEndY1 = aWorkRect.Bottom();
            const long          nStartY2 = nEndY1 - 1;
            std::unique_ptr<Point[]> pPoints1;
            std::unique_ptr<Point[]> pPoints2;
            long                nX, nY;
            sal_uInt16              nPolyPos = 0;
            const BitmapColor   aBlack = pAcc->GetBestMatchingColor( Color( COL_BLACK ) );

            if( nFlags & XOutFlags::ContourVert )
            {
                pPoints1.reset(new Point[ nWidth ]);
                pPoints2.reset(new Point[ nWidth ]);

                for( nX = nStartX1; nX < nEndX1; nX++ )
                {
                    nY = nStartY1;

                    // scan row from left to right
                    while( nY < nEndY1 )
                    {
                        Scanline pScanline = pAcc->GetScanline( nY );
                        if( aBlack == pAcc->GetPixelFromData( pScanline, nX ) )
                        {
                            pPoints1[ nPolyPos ] = Point( nX, nY );
                            nY = nStartY2;

                            // this loop always breaks eventually as there is at least one pixel
                            while( true )
                            {
                                if( aBlack == pAcc->GetPixelFromData( pScanline, nX ) )
                                {
                                    pPoints2[ nPolyPos ] = Point( nX, nY );
                                    break;
                                }

                                nY--;
                            }

                            nPolyPos++;
                            break;
                        }

                        nY++;
                    }
                }
            }
            else
            {
                pPoints1.reset(new Point[ nHeight ]);
                pPoints2.reset(new Point[ nHeight ]);

                for ( nY = nStartY1; nY < nEndY1; nY++ )
                {
                    nX = nStartX1;
                    Scanline pScanline = pAcc->GetScanline( nY );

                    // scan row from left to right
                    while( nX < nEndX1 )
                    {
                        if( aBlack == pAcc->GetPixelFromData( pScanline, nX ) )
                        {
                            pPoints1[ nPolyPos ] = Point( nX, nY );
                            nX = nStartX2;

                            // this loop always breaks eventually as there is at least one pixel
                            while( true )
                            {
                                if( aBlack == pAcc->GetPixelFromData( pScanline, nX ) )
                                {
                                    pPoints2[ nPolyPos ] = Point( nX, nY );
                                    break;
                                }

                                nX--;
                            }

                            nPolyPos++;
                            break;
                        }

                        nX++;
                    }
                }
            }

            const sal_uInt16 nNewSize1 = nPolyPos << 1;

            aRetPoly = tools::Polygon( nPolyPos, pPoints1.get() );
            aRetPoly.SetSize( nNewSize1 + 1 );
            aRetPoly[ nNewSize1 ] = aRetPoly[ 0 ];

            for( sal_uInt16 j = nPolyPos; nPolyPos < nNewSize1; )
                aRetPoly[ nPolyPos++ ] = pPoints2[ --j ];

            if( ( fFactorX != 0. ) && ( fFactorY != 0. ) )
                aRetPoly.Scale( fFactorX, fFactorY );
        }

        Bitmap::ReleaseAccess(pAcc);
    }

    return aRetPoly;
}

bool DitherBitmap( Bitmap& rBitmap )
{
    bool bRet = false;

    if( ( rBitmap.GetBitCount() >= 8 ) && ( Application::GetDefaultDevice()->GetColorCount() < 257 ) )
        bRet = rBitmap.Dither( BmpDitherFlags::Floyd );
    else
        bRet = false;

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
