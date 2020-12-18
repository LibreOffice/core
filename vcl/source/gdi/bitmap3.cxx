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

#include <math.h>

#include <vcl/bitmapaccess.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/bitmap.hxx>
#include <config_features.h>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <tools/helpers.hxx>
#if HAVE_FEATURE_SKIA
#include <vcl/skia/SkiaHelper.hxx>
#endif
#include <vcl/BitmapMonochromeFilter.hxx>

#include <BitmapScaleSuperFilter.hxx>
#include <BitmapScaleConvolutionFilter.hxx>
#include <BitmapFastScaleFilter.hxx>
#include <BitmapInterpolateScaleFilter.hxx>
#include <bitmapwriteaccess.hxx>
#include <bitmap/impoctree.hxx>
#include <bitmap/Octree.hxx>
#include <svdata.hxx>
#include <salinst.hxx>
#include <salbmp.hxx>

#include "impvect.hxx"

#include <memory>

bool Bitmap::Convert( BmpConversion eConversion )
{
    // try to convert in backend
    if (mxSalBmp)
    {
        // avoid large chunk of obsolete and hopefully rarely used conversions.
        if (eConversion == BmpConversion::N8BitNoConversion)
        {
            std::shared_ptr<SalBitmap> xImpBmp(ImplGetSVData()->mpDefInst->CreateSalBitmap());
            // frequently used conversion for creating alpha masks
            if (xImpBmp->Create(*mxSalBmp) && xImpBmp->InterpretAs8Bit())
            {
                ImplSetSalBitmap(xImpBmp);
                SAL_INFO( "vcl.opengl", "Ref count: " << mxSalBmp.use_count() );
                return true;
            }
        }
        if (eConversion == BmpConversion::N8BitGreys)
        {
            std::shared_ptr<SalBitmap> xImpBmp(ImplGetSVData()->mpDefInst->CreateSalBitmap());
            if (xImpBmp->Create(*mxSalBmp) && xImpBmp->ConvertToGreyscale())
            {
                ImplSetSalBitmap(xImpBmp);
                SAL_INFO( "vcl.opengl", "Ref count: " << mxSalBmp.use_count() );
                return true;
            }
        }
    }

    const sal_uInt16 nBitCount = GetBitCount ();
    bool bRet = false;

    switch( eConversion )
    {
        case BmpConversion::N1BitThreshold:
        {
            BitmapEx aBmpEx(*this);
            bRet = BitmapFilter::Filter(aBmpEx, BitmapMonochromeFilter(128));
            *this = aBmpEx.GetBitmap();
        }
        break;

        case BmpConversion::N4BitGreys:
            bRet = ImplMakeGreyscales( 16 );
        break;

        case BmpConversion::N4BitColors:
        {
            if( nBitCount < 4 )
                bRet = ImplConvertUp( 4 );
            else if( nBitCount > 4 )
                bRet = ImplConvertDown( 4 );
            else
                bRet = true;
        }
        break;

        case BmpConversion::N8BitGreys:
        case BmpConversion::N8BitNoConversion:
            bRet = ImplMakeGreyscales( 256 );
        break;

        case BmpConversion::N8BitColors:
        {
            if( nBitCount < 8 )
                bRet = ImplConvertUp( 8 );
            else if( nBitCount > 8 )
                bRet = ImplConvertDown( 8 );
            else
                bRet = true;
        }
        break;

        case BmpConversion::N8BitTrans:
        {
            Color aTrans( BMP_COL_TRANS );

            if( nBitCount < 8 )
                bRet = ImplConvertUp( 8, &aTrans );
            else
                bRet = ImplConvertDown( 8, &aTrans );
        }
        break;

        case BmpConversion::N24Bit:
        {
            if( nBitCount < 24 )
                bRet = ImplConvertUp( 24 );
            else
                bRet = true;
        }
        break;

        case BmpConversion::N32Bit:
        {
            if( nBitCount < 32 )
                bRet = ImplConvertUp( 32 );
            else
                bRet = true;
        }
        break;

        default:
            OSL_FAIL( "Bitmap::Convert(): Unsupported conversion" );
        break;
    }

    return bRet;
}

bool Bitmap::ImplMakeGreyscales( sal_uInt16 nGreys )
{
    SAL_WARN_IF( nGreys != 16 && nGreys != 256, "vcl", "Only 16 or 256 greyscales are supported!" );

    ScopedReadAccess pReadAcc(*this);
    bool bRet = false;

    if( pReadAcc )
    {
        const BitmapPalette& rPal = GetGreyPalette( nGreys );
        sal_uLong nShift = ( ( nGreys == 16 ) ? 4UL : 0UL );
        bool bPalDiffers = !pReadAcc->HasPalette() || ( rPal.GetEntryCount() != pReadAcc->GetPaletteEntryCount() );

        if( !bPalDiffers )
            bPalDiffers = ( rPal != pReadAcc->GetPalette() );

        if( bPalDiffers )
        {
            Bitmap aNewBmp( GetSizePixel(), ( nGreys == 16 ) ? 4 : 8, &rPal );
            BitmapScopedWriteAccess pWriteAcc(aNewBmp);

            if( pWriteAcc )
            {
                const tools::Long nWidth = pWriteAcc->Width();
                const tools::Long nHeight = pWriteAcc->Height();

                if( pReadAcc->HasPalette() )
                {
                    for( tools::Long nY = 0; nY < nHeight; nY++ )
                    {
                        Scanline pScanline = pWriteAcc->GetScanline(nY);
                        Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                        for( tools::Long nX = 0; nX < nWidth; nX++ )
                        {
                            const sal_uInt8 cIndex = pReadAcc->GetIndexFromData( pScanlineRead, nX );
                            pWriteAcc->SetPixelOnData( pScanline, nX,
                                BitmapColor(pReadAcc->GetPaletteColor( cIndex ).GetLuminance() >> nShift) );
                        }
                    }
                }
                else if( pReadAcc->GetScanlineFormat() == ScanlineFormat::N24BitTcBgr &&
                         pWriteAcc->GetScanlineFormat() == ScanlineFormat::N8BitPal )
                {
                    nShift += 8;

                    for( tools::Long nY = 0; nY < nHeight; nY++ )
                    {
                        Scanline pReadScan = pReadAcc->GetScanline( nY );
                        Scanline pWriteScan = pWriteAcc->GetScanline( nY );

                        for( tools::Long nX = 0; nX < nWidth; nX++ )
                        {
                            const sal_uLong nB = *pReadScan++;
                            const sal_uLong nG = *pReadScan++;
                            const sal_uLong nR = *pReadScan++;

                            *pWriteScan++ = static_cast<sal_uInt8>( ( nB * 28UL + nG * 151UL + nR * 77UL ) >> nShift );
                        }
                    }
                }
                else if( pReadAcc->GetScanlineFormat() == ScanlineFormat::N24BitTcRgb &&
                         pWriteAcc->GetScanlineFormat() == ScanlineFormat::N8BitPal )
                {
                    nShift += 8;

                    for( tools::Long nY = 0; nY < nHeight; nY++ )
                    {
                        Scanline pReadScan = pReadAcc->GetScanline( nY );
                        Scanline pWriteScan = pWriteAcc->GetScanline( nY );

                        for( tools::Long nX = 0; nX < nWidth; nX++ )
                        {
                            const sal_uLong nR = *pReadScan++;
                            const sal_uLong nG = *pReadScan++;
                            const sal_uLong nB = *pReadScan++;

                            *pWriteScan++ = static_cast<sal_uInt8>( ( nB * 28UL + nG * 151UL + nR * 77UL ) >> nShift );
                        }
                    }
                }
                else
                {
                    for( tools::Long nY = 0; nY < nHeight; nY++ )
                    {
                        Scanline pScanline = pWriteAcc->GetScanline(nY);
                        Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                        for( tools::Long nX = 0; nX < nWidth; nX++ )
                            pWriteAcc->SetPixelOnData( pScanline, nX, BitmapColor(pReadAcc->GetPixelFromData( pScanlineRead, nX ).GetLuminance() >> nShift) );
                    }
                }

                pWriteAcc.reset();
                bRet = true;
            }

            pReadAcc.reset();

            if( bRet )
            {
                const MapMode aMap( maPrefMapMode );
                const Size aSize( maPrefSize );

                *this = aNewBmp;

                maPrefMapMode = aMap;
                maPrefSize = aSize;
            }
        }
        else
        {
            pReadAcc.reset();
            bRet = true;
        }
    }

    return bRet;
}

bool Bitmap::ImplConvertUp(sal_uInt16 nBitCount, Color const * pExtColor)
{
    SAL_WARN_IF( nBitCount <= GetBitCount(), "vcl", "New BitCount must be greater!" );

    Bitmap::ScopedReadAccess pReadAcc(*this);
    bool bRet = false;

    if (pReadAcc)
    {
        BitmapPalette aPalette;
        Bitmap aNewBmp(GetSizePixel(), nBitCount, pReadAcc->HasPalette() ? &pReadAcc->GetPalette() : &aPalette);
        BitmapScopedWriteAccess pWriteAcc(aNewBmp);

        if (pWriteAcc)
        {
            const tools::Long nWidth = pWriteAcc->Width();
            const tools::Long nHeight = pWriteAcc->Height();

            if (pWriteAcc->HasPalette())
            {
                const BitmapPalette& rOldPalette = pReadAcc->GetPalette();
                const sal_uInt16 nOldCount = rOldPalette.GetEntryCount();
                assert(nOldCount <= (1 << GetBitCount()));

                aPalette.SetEntryCount(1 << nBitCount);

                for (sal_uInt16 i = 0; i < nOldCount; i++)
                    aPalette[i] = rOldPalette[i];

                if (pExtColor)
                    aPalette[aPalette.GetEntryCount() - 1] = *pExtColor;

                pWriteAcc->SetPalette(aPalette);

                for (tools::Long nY = 0; nY < nHeight; nY++)
                {
                    Scanline pScanline = pWriteAcc->GetScanline(nY);
                    Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                    for (tools::Long nX = 0; nX < nWidth; nX++)
                    {
                        pWriteAcc->SetPixelOnData(pScanline, nX, pReadAcc->GetPixelFromData(pScanlineRead, nX));
                    }
                }
            }
            else
            {
                if (pReadAcc->HasPalette())
                {
                    for (tools::Long nY = 0; nY < nHeight; nY++)
                    {
                        Scanline pScanline = pWriteAcc->GetScanline(nY);
                        Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                        for (tools::Long nX = 0; nX < nWidth; nX++)
                        {
                            pWriteAcc->SetPixelOnData(pScanline, nX, pReadAcc->GetPaletteColor(pReadAcc->GetIndexFromData(pScanlineRead, nX)));
                        }
                    }
                }
                else
                {
                    for (tools::Long nY = 0; nY < nHeight; nY++)
                    {
                        Scanline pScanline = pWriteAcc->GetScanline(nY);
                        Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                        for (tools::Long nX = 0; nX < nWidth; nX++)
                        {
                            pWriteAcc->SetPixelOnData(pScanline, nX, pReadAcc->GetPixelFromData(pScanlineRead, nX));
                        }
                    }
                }
            }
            bRet = true;
        }

        if (bRet)
        {
            const MapMode aMap(maPrefMapMode);
            const Size aSize(maPrefSize);

            *this = aNewBmp;

            maPrefMapMode = aMap;
            maPrefSize = aSize;
        }
    }

    return bRet;
}

bool Bitmap::ImplConvertDown(sal_uInt16 nBitCount, Color const * pExtColor)
{
    SAL_WARN_IF(nBitCount > GetBitCount(), "vcl", "New BitCount must be lower ( or equal when pExtColor is set )!");

    Bitmap::ScopedReadAccess pReadAcc(*this);
    bool bRet = false;

    if (pReadAcc)
    {
        BitmapPalette aPalette;
        Bitmap aNewBmp(GetSizePixel(), nBitCount, &aPalette);
        BitmapScopedWriteAccess pWriteAcc(aNewBmp);

        if (pWriteAcc)
        {
            const sal_uInt16 nCount = 1 << nBitCount;
            const tools::Long nWidth = pWriteAcc->Width();
            const tools::Long nWidth1 = nWidth - 1;
            const tools::Long nHeight = pWriteAcc->Height();
            Octree aOctree(*pReadAcc, pExtColor ? (nCount - 1) : nCount);
            aPalette = aOctree.GetPalette();
            InverseColorMap aColorMap(aPalette);
            BitmapColor aColor;
            ImpErrorQuad aErrQuad;
            std::vector<ImpErrorQuad> aErrQuad1(nWidth);
            std::vector<ImpErrorQuad> aErrQuad2(nWidth);
            ImpErrorQuad* pQLine1 = aErrQuad1.data();
            ImpErrorQuad* pQLine2 = nullptr;
            tools::Long nYTmp = 0;
            sal_uInt8 cIndex;
            bool bQ1 = true;

            if (pExtColor)
            {
                aPalette.SetEntryCount(aPalette.GetEntryCount() + 1);
                aPalette[aPalette.GetEntryCount() - 1] = *pExtColor;
            }

            // set Black/White always, if we have enough space
            if (aPalette.GetEntryCount() < (nCount - 1))
            {
                aPalette.SetEntryCount(aPalette.GetEntryCount() + 2);
                aPalette[aPalette.GetEntryCount() - 2] = COL_BLACK;
                aPalette[aPalette.GetEntryCount() - 1] = COL_WHITE;
            }

            pWriteAcc->SetPalette(aPalette);

            for (tools::Long nY = 0; nY < std::min(nHeight, tools::Long(2)); nY++, nYTmp++)
            {
                pQLine2 = !nY ? aErrQuad1.data() : aErrQuad2.data();
                Scanline pScanlineRead = pReadAcc->GetScanline(nYTmp);
                for (tools::Long nX = 0; nX < nWidth; nX++)
                {
                    if (pReadAcc->HasPalette())
                        pQLine2[nX] = pReadAcc->GetPaletteColor(pReadAcc->GetIndexFromData(pScanlineRead, nX));
                    else
                        pQLine2[nX] = pReadAcc->GetPixelFromData(pScanlineRead, nX);
                }
            }

            assert(pQLine2 || nHeight == 0);

            for (tools::Long nY = 0; nY < nHeight; nY++, nYTmp++)
            {
                // first pixel in the line
                cIndex = static_cast<sal_uInt8>(aColorMap.GetBestPaletteIndex(pQLine1[0].ImplGetColor()));
                Scanline pScanline = pWriteAcc->GetScanline(nY);
                pWriteAcc->SetPixelOnData(pScanline, 0, BitmapColor(cIndex));

                tools::Long nX;
                for (nX = 1; nX < nWidth1; nX++)
                {
                    aColor = pQLine1[nX].ImplGetColor();
                    cIndex = static_cast<sal_uInt8>(aColorMap.GetBestPaletteIndex(aColor));
                    aErrQuad = (ImpErrorQuad(aColor) -= pWriteAcc->GetPaletteColor(cIndex));
                    pQLine1[++nX].ImplAddColorError7(aErrQuad);
                    pQLine2[nX--].ImplAddColorError1(aErrQuad);
                    pQLine2[nX--].ImplAddColorError5(aErrQuad);
                    pQLine2[nX++].ImplAddColorError3(aErrQuad);
                    pWriteAcc->SetPixelOnData(pScanline, nX, BitmapColor(cIndex));
                }

                // Last RowPixel
                if (nX < nWidth)
                {
                    cIndex = static_cast<sal_uInt8>(aColorMap.GetBestPaletteIndex(pQLine1[nWidth1].ImplGetColor()));
                    pWriteAcc->SetPixelOnData(pScanline, nX, BitmapColor(cIndex));
                }

                // Refill/copy row buffer
                pQLine1 = pQLine2;
                bQ1 = !bQ1;
                pQLine2 = bQ1 ? aErrQuad2.data() : aErrQuad1.data();

                if (nYTmp < nHeight)
                {
                    Scanline pScanlineRead = pReadAcc->GetScanline(nYTmp);
                    for (nX = 0; nX < nWidth; nX++)
                    {
                        if (pReadAcc->HasPalette())
                            pQLine2[nX] = pReadAcc->GetPaletteColor(pReadAcc->GetIndexFromData(pScanlineRead, nX));
                        else
                            pQLine2[nX] = pReadAcc->GetPixelFromData(pScanlineRead, nX);
                    }
                }
            }

            bRet = true;
        }
        pWriteAcc.reset();

        if(bRet)
        {
            const MapMode aMap(maPrefMapMode);
            const Size aSize(maPrefSize);

            *this = aNewBmp;

            maPrefMapMode = aMap;
            maPrefSize = aSize;
        }
    }

    return bRet;
}

bool Bitmap::Scale( const double& rScaleX, const double& rScaleY, BmpScaleFlag nScaleFlag )
{
    if(basegfx::fTools::equalZero(rScaleX) || basegfx::fTools::equalZero(rScaleY))
    {
        // no scale
        return true;
    }

    if(basegfx::fTools::equal(rScaleX, 1.0) && basegfx::fTools::equal(rScaleY, 1.0))
    {
        // no scale
        return true;
    }

    const sal_uInt16 nStartCount(GetBitCount());

    if (mxSalBmp && mxSalBmp->ScalingSupported())
    {
        // implementation specific scaling
        std::shared_ptr<SalBitmap> xImpBmp(ImplGetSVData()->mpDefInst->CreateSalBitmap());
        if (xImpBmp->Create(*mxSalBmp) && xImpBmp->Scale(rScaleX, rScaleY, nScaleFlag))
        {
            ImplSetSalBitmap(xImpBmp);
            SAL_INFO( "vcl.opengl", "Ref count: " << mxSalBmp.use_count() );
            maPrefMapMode = MapMode( MapUnit::MapPixel );
            maPrefSize = xImpBmp->GetSize();
            return true;
        }
    }

    // fdo#33455
    //
    // If we start with a 1 bit image, then after scaling it in any mode except
    // BmpScaleFlag::Fast we have a 24bit image which is perfectly correct, but we
    // are going to down-shift it to mono again and Bitmap::MakeMonochrome just
    // has "Bitmap aNewBmp( GetSizePixel(), 1 );" to create a 1 bit bitmap which
    // will default to black/white and the colors mapped to which ever is closer
    // to black/white
    //
    // So the easiest thing to do to retain the colors of 1 bit bitmaps is to
    // just use the fast scale rather than attempting to count unique colors in
    // the other converters and pass all the info down through
    // Bitmap::MakeMonochrome
    if (nStartCount == 1)
        nScaleFlag = BmpScaleFlag::Fast;

    BitmapEx aBmpEx(*this);
    bool bRetval(false);

    switch(nScaleFlag)
    {
        case BmpScaleFlag::Default:
            if (GetSizePixel().Width() < 2 || GetSizePixel().Height() < 2)
                bRetval = BitmapFilter::Filter(aBmpEx, BitmapFastScaleFilter(rScaleX, rScaleY));
            else
                bRetval = BitmapFilter::Filter(aBmpEx, BitmapScaleSuperFilter(rScaleX, rScaleY));
            break;

        case BmpScaleFlag::Fast:
        case BmpScaleFlag::NearestNeighbor:
            bRetval = BitmapFilter::Filter(aBmpEx, BitmapFastScaleFilter(rScaleX, rScaleY));
            break;

        case BmpScaleFlag::Interpolate:
            bRetval = BitmapFilter::Filter(aBmpEx, BitmapInterpolateScaleFilter(rScaleX, rScaleY));
            break;

        case BmpScaleFlag::Super:
            bRetval = BitmapFilter::Filter(aBmpEx, BitmapScaleSuperFilter(rScaleX, rScaleY));
            break;
        case BmpScaleFlag::BestQuality:
        case BmpScaleFlag::Lanczos:
            bRetval = BitmapFilter::Filter(aBmpEx, vcl::BitmapScaleLanczos3Filter(rScaleX, rScaleY));
            break;

        case BmpScaleFlag::BiCubic:
            bRetval = BitmapFilter::Filter(aBmpEx, vcl::BitmapScaleBicubicFilter(rScaleX, rScaleY));
            break;

        case BmpScaleFlag::BiLinear:
            bRetval = BitmapFilter::Filter(aBmpEx, vcl::BitmapScaleBilinearFilter(rScaleX, rScaleY));
            break;
    }

    if (bRetval)
        *this = aBmpEx.GetBitmap();

    OSL_ENSURE(!bRetval || nStartCount == GetBitCount(), "Bitmap::Scale has changed the ColorDepth, this should *not* happen (!)");
    return bRetval;
}

bool Bitmap::Scale( const Size& rNewSize, BmpScaleFlag nScaleFlag )
{
    const Size aSize( GetSizePixel() );
    bool bRet;

    if( aSize.Width() && aSize.Height() )
    {
        bRet = Scale( static_cast<double>(rNewSize.Width()) / aSize.Width(),
                      static_cast<double>(rNewSize.Height()) / aSize.Height(),
                      nScaleFlag );
    }
    else
        bRet = true;

    return bRet;
}

bool Bitmap::HasFastScale()
{
#if HAVE_FEATURE_SKIA
    if( SkiaHelper::isVCLSkiaEnabled() && SkiaHelper::renderMethodToUse() != SkiaHelper::RenderRaster)
        return true;
#endif
    return false;
}

void Bitmap::AdaptBitCount(Bitmap& rNew) const
{
    // aNew is the result of some operation; adapt it's BitCount to the original (this)
    if(GetBitCount() == rNew.GetBitCount())
        return;

    switch(GetBitCount())
    {
        case 1:
        {
            rNew.Convert(BmpConversion::N1BitThreshold);
            break;
        }
        case 4:
        {
            if(HasGreyPaletteAny())
            {
                rNew.Convert(BmpConversion::N4BitGreys);
            }
            else
            {
                rNew.Convert(BmpConversion::N4BitColors);
            }
            break;
        }
        case 8:
        {
            if(HasGreyPaletteAny())
            {
                rNew.Convert(BmpConversion::N8BitGreys);
            }
            else
            {
                rNew.Convert(BmpConversion::N8BitColors);
            }
            break;
        }
        case 24:
        {
            rNew.Convert(BmpConversion::N24Bit);
            break;
        }
        case 32:
        {
            rNew.Convert(BmpConversion::N32Bit);
            break;
        }
        default:
        {
            SAL_WARN("vcl", "BitDepth adaptation failed, from " << rNew.GetBitCount() << " to " << GetBitCount());
            break;
        }
    }
}

void Bitmap::Vectorize( GDIMetaFile& rMtf, sal_uInt8 cReduce, const Link<tools::Long,void>* pProgress )
{
    ImplVectorizer::ImplVectorize( *this, rMtf, cReduce, pProgress );
}

bool Bitmap::Adjust( short nLuminancePercent, short nContrastPercent,
                     short nChannelRPercent, short nChannelGPercent, short nChannelBPercent,
                     double fGamma, bool bInvert, bool msoBrightness )
{
    bool bRet = false;

    // nothing to do => return quickly
    if( !nLuminancePercent && !nContrastPercent &&
        !nChannelRPercent && !nChannelGPercent && !nChannelBPercent &&
        ( fGamma == 1.0 ) && !bInvert )
    {
        bRet = true;
    }
    else
    {
        BitmapScopedWriteAccess pAcc(*this);

        if( pAcc )
        {
            BitmapColor aCol;
            const tools::Long nW = pAcc->Width();
            const tools::Long nH = pAcc->Height();
            std::unique_ptr<sal_uInt8[]> cMapR(new sal_uInt8[ 256 ]);
            std::unique_ptr<sal_uInt8[]> cMapG(new sal_uInt8[ 256 ]);
            std::unique_ptr<sal_uInt8[]> cMapB(new sal_uInt8[ 256 ]);
            double fM, fROff, fGOff, fBOff, fOff;

            // calculate slope
            if( nContrastPercent >= 0 )
                fM = 128.0 / ( 128.0 - 1.27 * MinMax( nContrastPercent, 0, 100 ) );
            else
                fM = ( 128.0 + 1.27 * MinMax( nContrastPercent, -100, 0 ) ) / 128.0;

            if(!msoBrightness)
                // total offset = luminance offset + contrast offset
                fOff = MinMax( nLuminancePercent, -100, 100 ) * 2.55 + 128.0 - fM * 128.0;
            else
                fOff = MinMax( nLuminancePercent, -100, 100 ) * 2.55;

            // channel offset = channel offset + total offset
            fROff = nChannelRPercent * 2.55 + fOff;
            fGOff = nChannelGPercent * 2.55 + fOff;
            fBOff = nChannelBPercent * 2.55 + fOff;

            // calculate gamma value
            fGamma = ( fGamma <= 0.0 || fGamma > 10.0 ) ? 1.0 : ( 1.0 / fGamma );
            const bool bGamma = ( fGamma != 1.0 );

            // create mapping table
            for( tools::Long nX = 0; nX < 256; nX++ )
            {
                if(!msoBrightness)
                {
                    cMapR[ nX ] = static_cast<sal_uInt8>(MinMax( FRound( nX * fM + fROff ), 0, 255 ));
                    cMapG[ nX ] = static_cast<sal_uInt8>(MinMax( FRound( nX * fM + fGOff ), 0, 255 ));
                    cMapB[ nX ] = static_cast<sal_uInt8>(MinMax( FRound( nX * fM + fBOff ), 0, 255 ));
                }
                else
                {
                    // LO simply uses (in a somewhat optimized form) "newcolor = (oldcolor-128)*contrast+brightness+128"
                    // as the formula, i.e. contrast first, brightness afterwards. MSOffice, for whatever weird reason,
                    // use neither first, but apparently it applies half of brightness before contrast and half afterwards.
                    cMapR[ nX ] = static_cast<sal_uInt8>(MinMax( FRound( (nX+fROff/2-128) * fM + 128 + fROff/2 ), 0, 255 ));
                    cMapG[ nX ] = static_cast<sal_uInt8>(MinMax( FRound( (nX+fGOff/2-128) * fM + 128 + fGOff/2 ), 0, 255 ));
                    cMapB[ nX ] = static_cast<sal_uInt8>(MinMax( FRound( (nX+fBOff/2-128) * fM + 128 + fBOff/2 ), 0, 255 ));
                }
                if( bGamma )
                {
                    cMapR[ nX ] = GAMMA( cMapR[ nX ], fGamma );
                    cMapG[ nX ] = GAMMA( cMapG[ nX ], fGamma );
                    cMapB[ nX ] = GAMMA( cMapB[ nX ], fGamma );
                }

                if( bInvert )
                {
                    cMapR[ nX ] = ~cMapR[ nX ];
                    cMapG[ nX ] = ~cMapG[ nX ];
                    cMapB[ nX ] = ~cMapB[ nX ];
                }
            }

            // do modifying
            if( pAcc->HasPalette() )
            {
                BitmapColor aNewCol;

                for( sal_uInt16 i = 0, nCount = pAcc->GetPaletteEntryCount(); i < nCount; i++ )
                {
                    const BitmapColor& rCol = pAcc->GetPaletteColor( i );
                    aNewCol.SetRed( cMapR[ rCol.GetRed() ] );
                    aNewCol.SetGreen( cMapG[ rCol.GetGreen() ] );
                    aNewCol.SetBlue( cMapB[ rCol.GetBlue() ] );
                    pAcc->SetPaletteColor( i, aNewCol );
                }
            }
            else if( pAcc->GetScanlineFormat() == ScanlineFormat::N24BitTcBgr )
            {
                for( tools::Long nY = 0; nY < nH; nY++ )
                {
                    Scanline pScan = pAcc->GetScanline( nY );

                    for( tools::Long nX = 0; nX < nW; nX++ )
                    {
                        *pScan = cMapB[ *pScan ]; pScan++;
                        *pScan = cMapG[ *pScan ]; pScan++;
                        *pScan = cMapR[ *pScan ]; pScan++;
                    }
                }
            }
            else if( pAcc->GetScanlineFormat() == ScanlineFormat::N24BitTcRgb )
            {
                for( tools::Long nY = 0; nY < nH; nY++ )
                {
                    Scanline pScan = pAcc->GetScanline( nY );

                    for( tools::Long nX = 0; nX < nW; nX++ )
                    {
                        *pScan = cMapR[ *pScan ]; pScan++;
                        *pScan = cMapG[ *pScan ]; pScan++;
                        *pScan = cMapB[ *pScan ]; pScan++;
                    }
                }
            }
            else
            {
                for( tools::Long nY = 0; nY < nH; nY++ )
                {
                    Scanline pScanline = pAcc->GetScanline(nY);
                    for( tools::Long nX = 0; nX < nW; nX++ )
                    {
                        aCol = pAcc->GetPixelFromData( pScanline, nX );
                        aCol.SetRed( cMapR[ aCol.GetRed() ] );
                        aCol.SetGreen( cMapG[ aCol.GetGreen() ] );
                        aCol.SetBlue( cMapB[ aCol.GetBlue() ] );
                        pAcc->SetPixelOnData( pScanline, nX, aCol );
                    }
                }
            }

            pAcc.reset();
            bRet = true;
        }
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
