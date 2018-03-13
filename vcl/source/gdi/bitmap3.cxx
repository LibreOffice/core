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
#include <stdlib.h>

#include <vcl/bitmapaccess.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/bitmap.hxx>
#include <config_features.h>
#if HAVE_FEATURE_OPENGL
#include <vcl/opengl/OpenGLHelper.hxx>
#endif

#include <impoctree.hxx>
#include <BitmapScaleSuperFilter.hxx>
#include <BitmapScaleConvolutionFilter.hxx>
#include <BitmapFastScaleFilter.hxx>
#include <BitmapInterpolateScaleFilter.hxx>
#include <bitmapwriteaccess.hxx>
#include <octree.hxx>
#include <svdata.hxx>
#include <salinst.hxx>
#include <salbmp.hxx>

#include "impvect.hxx"

#include <memory>

#define GAMMA( _def_cVal, _def_InvGamma )   (static_cast<sal_uInt8>(MinMax(FRound(pow( _def_cVal/255.0,_def_InvGamma)*255.0),0,255)))

bool Bitmap::Convert( BmpConversion eConversion )
{
    // try to convert in backend
    if (mxSalBmp)
    {
        // avoid large chunk of obsolete and hopefully rarely used conversions.
        if (eConversion == BmpConversion::N8BitGreys)
        {
            std::shared_ptr<SalBitmap> xImpBmp(ImplGetSVData()->mpDefInst->CreateSalBitmap());
            // frequently used conversion for creating alpha masks
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
            bRet = MakeMonochrome(128);
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

        case BmpConversion::Ghosted:
            bRet = ImplConvertGhosted();
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
                const long nWidth = pWriteAcc->Width();
                const long nHeight = pWriteAcc->Height();

                if( pReadAcc->HasPalette() )
                {
                    for( long nY = 0; nY < nHeight; nY++ )
                    {
                        Scanline pScanline = pWriteAcc->GetScanline(nY);
                        Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                        for( long nX = 0; nX < nWidth; nX++ )
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

                    for( long nY = 0; nY < nHeight; nY++ )
                    {
                        Scanline pReadScan = pReadAcc->GetScanline( nY );
                        Scanline pWriteScan = pWriteAcc->GetScanline( nY );

                        for( long nX = 0; nX < nWidth; nX++ )
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

                    for( long nY = 0; nY < nHeight; nY++ )
                    {
                        Scanline pReadScan = pReadAcc->GetScanline( nY );
                        Scanline pWriteScan = pWriteAcc->GetScanline( nY );

                        for( long nX = 0; nX < nWidth; nX++ )
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
                    for( long nY = 0; nY < nHeight; nY++ )
                    {
                        Scanline pScanline = pWriteAcc->GetScanline(nY);
                        Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                        for( long nX = 0; nX < nWidth; nX++ )
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
            const long nWidth = pWriteAcc->Width();
            const long nHeight = pWriteAcc->Height();

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

                for (long nY = 0; nY < nHeight; nY++)
                {
                    Scanline pScanline = pWriteAcc->GetScanline(nY);
                    Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                    for (long nX = 0; nX < nWidth; nX++)
                    {
                        pWriteAcc->SetPixelOnData(pScanline, nX, pReadAcc->GetPixelFromData(pScanlineRead, nX));
                    }
                }
            }
            else
            {
                if (pReadAcc->HasPalette())
                {
                    for (long nY = 0; nY < nHeight; nY++)
                    {
                        Scanline pScanline = pWriteAcc->GetScanline(nY);
                        Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                        for (long nX = 0; nX < nWidth; nX++)
                        {
                            pWriteAcc->SetPixelOnData(pScanline, nX, pReadAcc->GetPaletteColor(pReadAcc->GetIndexFromData(pScanlineRead, nX)));
                        }
                    }
                }
                else
                {
                    for (long nY = 0; nY < nHeight; nY++)
                    {
                        Scanline pScanline = pWriteAcc->GetScanline(nY);
                        Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                        for (long nX = 0; nX < nWidth; nX++)
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
            const long nWidth = pWriteAcc->Width();
            const long nWidth1 = nWidth - 1;
            const long nHeight = pWriteAcc->Height();
            Octree aOctree(*pReadAcc, pExtColor ? (nCount - 1) : nCount);
            aPalette = aOctree.GetPalette();
            InverseColorMap aColorMap(aPalette);
            BitmapColor aColor;
            ImpErrorQuad aErrQuad;
            std::vector<ImpErrorQuad> aErrQuad1(nWidth);
            std::vector<ImpErrorQuad> aErrQuad2(nWidth);
            ImpErrorQuad* pQLine1 = aErrQuad1.data();
            ImpErrorQuad* pQLine2 = nullptr;
            long nYTmp = 0;
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

            for (long nY = 0; nY < std::min(nHeight, 2L); nY++, nYTmp++)
            {
                pQLine2 = !nY ? aErrQuad1.data() : aErrQuad2.data();
                Scanline pScanlineRead = pReadAcc->GetScanline(nYTmp);
                for (long nX = 0; nX < nWidth; nX++)
                {
                    if (pReadAcc->HasPalette())
                        pQLine2[nX] = pReadAcc->GetPaletteColor(pReadAcc->GetIndexFromData(pScanlineRead, nX));
                    else
                        pQLine2[nX] = pReadAcc->GetPixelFromData(pScanlineRead, nX);
                }
            }

            assert(pQLine2 || nHeight == 0);

            for (long nY = 0; nY < nHeight; nY++, nYTmp++)
            {
                // first pixel in the line
                cIndex = static_cast<sal_uInt8>(aColorMap.GetBestPaletteIndex(pQLine1[0].ImplGetColor()));
                Scanline pScanline = pWriteAcc->GetScanline(nY);
                pWriteAcc->SetPixelOnData(pScanline, 0, BitmapColor(cIndex));

                long nX;
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

bool Bitmap::ImplConvertGhosted()
{
    Bitmap aNewBmp;
    ScopedReadAccess pR(*this);
    bool bRet = false;

    if( pR )
    {
        if( pR->HasPalette() )
        {
            BitmapPalette aNewPal( pR->GetPaletteEntryCount() );

            for( long i = 0, nCount = aNewPal.GetEntryCount(); i < nCount; i++ )
            {
                const BitmapColor& rOld = pR->GetPaletteColor( static_cast<sal_uInt16>(i) );
                aNewPal[ static_cast<sal_uInt16>(i) ] = BitmapColor( ( rOld.GetRed() >> 1 ) | 0x80,
                                                     ( rOld.GetGreen() >> 1 ) | 0x80,
                                                     ( rOld.GetBlue() >> 1 ) | 0x80 );
            }

            aNewBmp = Bitmap( GetSizePixel(), GetBitCount(), &aNewPal );
            BitmapScopedWriteAccess pW(aNewBmp);

            if( pW )
            {
                pW->CopyBuffer( *pR );
                bRet = true;
            }
        }
        else
        {
            aNewBmp = Bitmap( GetSizePixel(), 24 );

            BitmapScopedWriteAccess pW(aNewBmp);

            if( pW )
            {
                const long nWidth = pR->Width(), nHeight = pR->Height();

                for( long nY = 0; nY < nHeight; nY++ )
                {
                    Scanline pScanline = pW->GetScanline(nY);
                    Scanline pScanlineRead = pR->GetScanline(nY);
                    for( long nX = 0; nX < nWidth; nX++ )
                    {
                        const BitmapColor aOld( pR->GetPixelFromData( pScanlineRead, nX ) );
                        pW->SetPixelOnData( pScanline, nX, BitmapColor( ( aOld.GetRed() >> 1 ) | 0x80,
                                                                        ( aOld.GetGreen() >> 1 ) | 0x80,
                                                                        ( aOld.GetBlue() >> 1 ) | 0x80 ) );

                    }
                }

                bRet = true;
            }
        }

        pR.reset();
    }

    if( bRet )
    {
        const MapMode aMap( maPrefMapMode );
        const Size aSize( maPrefSize );

        *this = aNewBmp;

        maPrefMapMode = aMap;
        maPrefSize = aSize;
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
        case BmpScaleFlag::Fast:
            bRetval = BitmapFilter::Filter(aBmpEx, BitmapFastScaleFilter(rScaleX, rScaleY));
            break;

        case BmpScaleFlag::Interpolate:
            bRetval = BitmapFilter::Filter(aBmpEx, BitmapInterpolateScaleFilter(rScaleX, rScaleY));
            break;

        case BmpScaleFlag::Default:
            if (GetSizePixel().Width() < 2 || GetSizePixel().Height() < 2)
                bRetval = BitmapFilter::Filter(aBmpEx, BitmapFastScaleFilter(rScaleX, rScaleY));
            else
                bRetval = BitmapFilter::Filter(aBmpEx, BitmapScaleSuperFilter(rScaleX, rScaleY));
            break;

        case BmpScaleFlag::Lanczos:
        case BmpScaleFlag::BestQuality:
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
        *this = aBmpEx.GetBitmapRef();

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
#if HAVE_FEATURE_OPENGL
    return OpenGLHelper::isVCLOpenGLEnabled();
#else
    return false;
#endif
}

void Bitmap::AdaptBitCount(Bitmap& rNew) const
{
    // aNew is the result of some operation; adapt it's BitCount to the original (this)
    if(GetBitCount() != rNew.GetBitCount())
    {
        switch(GetBitCount())
        {
            case 1:
            {
                rNew.Convert(BmpConversion::N1BitThreshold);
                break;
            }
            case 4:
            {
                if(HasGreyPalette())
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
                if(HasGreyPalette())
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
            default:
            {
                OSL_ENSURE(false, "BitDepth adaption failed (!)");
                break;
            }
        }
    }
}

bool Bitmap::ReduceColors( sal_uInt16 nColorCount, BmpReduce eReduce )
{
    bool bRet;

    if( GetColorCount() <= static_cast<sal_uLong>(nColorCount) )
        bRet = true;
    else if( nColorCount )
    {
        if( BMP_REDUCE_SIMPLE == eReduce )
            bRet = ImplReduceSimple( nColorCount );
        else if( BMP_REDUCE_POPULAR == eReduce )
            bRet = ImplReducePopular( nColorCount );
        else
            bRet = ImplReduceMedian( nColorCount );
    }
    else
        bRet = false;

    return bRet;
}

bool Bitmap::ImplReduceSimple( sal_uInt16 nColorCount )
{
    Bitmap aNewBmp;
    ScopedReadAccess pRAcc(*this);
    const sal_uInt16 nColCount = std::min( nColorCount, sal_uInt16(256) );
    sal_uInt16 nBitCount;
    bool bRet = false;

    if( nColCount <= 2 )
        nBitCount = 1;
    else if( nColCount <= 16 )
        nBitCount = 4;
    else
        nBitCount = 8;

    if( pRAcc )
    {
        Octree aOct( *pRAcc, nColCount );
        const BitmapPalette& rPal = aOct.GetPalette();

        aNewBmp = Bitmap( GetSizePixel(), nBitCount, &rPal );
        BitmapScopedWriteAccess pWAcc(aNewBmp);

        if( pWAcc )
        {
            const long nWidth = pRAcc->Width();
            const long nHeight = pRAcc->Height();

            if( pRAcc->HasPalette() )
            {
                for( long nY = 0; nY < nHeight; nY++ )
                {
                    Scanline pScanline = pWAcc->GetScanline(nY);
                    Scanline pScanlineRead = pRAcc->GetScanline(nY);
                    for( long nX =0; nX < nWidth; nX++ )
                    {
                        auto c = pRAcc->GetPaletteColor( pRAcc->GetIndexFromData( pScanlineRead, nX ) );
                        pWAcc->SetPixelOnData( pScanline, nX, BitmapColor(static_cast<sal_uInt8>(aOct.GetBestPaletteIndex( c ))) );
                    }
                }
            }
            else
            {
                for( long nY = 0; nY < nHeight; nY++ )
                {
                    Scanline pScanline = pWAcc->GetScanline(nY);
                    Scanline pScanlineRead = pRAcc->GetScanline(nY);
                    for( long nX =0; nX < nWidth; nX++ )
                    {
                        auto c = pRAcc->GetPixelFromData( pScanlineRead, nX );
                        pWAcc->SetPixelOnData( pScanline, nX, BitmapColor(static_cast<sal_uInt8>(aOct.GetBestPaletteIndex( c ))) );
                    }
                }
            }

            pWAcc.reset();
            bRet = true;
        }

        pRAcc.reset();
    }

    if( bRet )
    {
        const MapMode aMap( maPrefMapMode );
        const Size aSize( maPrefSize );

        *this = aNewBmp;
        maPrefMapMode = aMap;
        maPrefSize = aSize;
    }

    return bRet;
}

struct PopularColorCount
{
    sal_uInt32 mnIndex;
    sal_uInt32 mnCount;
};

extern "C" int ImplPopularCmpFnc( const void* p1, const void* p2 )
{
    int nRet;

    if( static_cast<PopularColorCount const *>(p1)->mnCount < static_cast<PopularColorCount const *>(p2)->mnCount )
        nRet = 1;
    else if( static_cast<PopularColorCount const *>(p1)->mnCount == static_cast<PopularColorCount const *>(p2)->mnCount )
        nRet = 0;
    else
        nRet = -1;

    return nRet;
}

bool Bitmap::ImplReducePopular( sal_uInt16 nColCount )
{
    ScopedReadAccess pRAcc(*this);
    sal_uInt16 nBitCount;
    bool bRet = false;

    if( nColCount > 256 )
        nColCount = 256;

    if( nColCount < 17 )
        nBitCount = 4;
    else
        nBitCount = 8;

    if( pRAcc )
    {
        const sal_uInt32 nValidBits = 4;
        const sal_uInt32 nRightShiftBits = 8 - nValidBits;
        const sal_uInt32 nLeftShiftBits1 = nValidBits;
        const sal_uInt32 nLeftShiftBits2 = nValidBits << 1;
        const sal_uInt32 nColorsPerComponent = 1 << nValidBits;
        const sal_uInt32 nColorOffset = 256 / nColorsPerComponent;
        const sal_uInt32 nTotalColors = nColorsPerComponent * nColorsPerComponent * nColorsPerComponent;
        const long nWidth = pRAcc->Width();
        const long nHeight = pRAcc->Height();
        std::unique_ptr<PopularColorCount[]> pCountTable(new PopularColorCount[ nTotalColors ]);

        memset( pCountTable.get(), 0, nTotalColors * sizeof( PopularColorCount ) );

        for( long nR = 0, nIndex = 0; nR < 256; nR += nColorOffset )
        {
            for( long nG = 0; nG < 256; nG += nColorOffset )
            {
                for( long nB = 0; nB < 256; nB += nColorOffset )
                {
                    pCountTable[ nIndex ].mnIndex = nIndex;
                    nIndex++;
                }
            }
        }

        if( pRAcc->HasPalette() )
        {
            for( long nY = 0; nY < nHeight; nY++ )
            {
                Scanline pScanlineRead = pRAcc->GetScanline(nY);
                for( long nX = 0; nX < nWidth; nX++ )
                {
                    const BitmapColor& rCol = pRAcc->GetPaletteColor( pRAcc->GetIndexFromData( pScanlineRead, nX ) );
                    pCountTable[ ( ( static_cast<sal_uInt32>(rCol.GetRed()) >> nRightShiftBits ) << nLeftShiftBits2 ) |
                                 ( ( static_cast<sal_uInt32>(rCol.GetGreen()) >> nRightShiftBits ) << nLeftShiftBits1 ) |
                                 ( static_cast<sal_uInt32>(rCol.GetBlue()) >> nRightShiftBits ) ].mnCount++;
                }
            }
        }
        else
        {
            for( long nY = 0; nY < nHeight; nY++ )
            {
                Scanline pScanlineRead = pRAcc->GetScanline(nY);
                for( long nX = 0; nX < nWidth; nX++ )
                {
                    const BitmapColor aCol( pRAcc->GetPixelFromData( pScanlineRead, nX ) );
                    pCountTable[ ( ( static_cast<sal_uInt32>(aCol.GetRed()) >> nRightShiftBits ) << nLeftShiftBits2 ) |
                                 ( ( static_cast<sal_uInt32>(aCol.GetGreen()) >> nRightShiftBits ) << nLeftShiftBits1 ) |
                                 ( static_cast<sal_uInt32>(aCol.GetBlue()) >> nRightShiftBits ) ].mnCount++;
                }
            }
        }

        BitmapPalette aNewPal( nColCount );

        qsort( pCountTable.get(), nTotalColors, sizeof( PopularColorCount ), ImplPopularCmpFnc );

        for( sal_uInt16 n = 0; n < nColCount; n++ )
        {
            const PopularColorCount& rPop = pCountTable[ n ];
            aNewPal[ n ] = BitmapColor( static_cast<sal_uInt8>( ( rPop.mnIndex >> nLeftShiftBits2 ) << nRightShiftBits ),
                                        static_cast<sal_uInt8>( ( ( rPop.mnIndex >> nLeftShiftBits1 ) & ( nColorsPerComponent - 1 ) ) << nRightShiftBits ),
                                        static_cast<sal_uInt8>( ( rPop.mnIndex & ( nColorsPerComponent - 1 ) ) << nRightShiftBits ) );
        }

        Bitmap aNewBmp( GetSizePixel(), nBitCount, &aNewPal );
        BitmapScopedWriteAccess pWAcc(aNewBmp);

        if( pWAcc )
        {
            BitmapColor aDstCol( sal_uInt8(0) );
            std::unique_ptr<sal_uInt8[]> pIndexMap(new sal_uInt8[ nTotalColors ]);

            for( long nR = 0, nIndex = 0; nR < 256; nR += nColorOffset )
                for( long nG = 0; nG < 256; nG += nColorOffset )
                    for( long nB = 0; nB < 256; nB += nColorOffset )
                        pIndexMap[ nIndex++ ] = static_cast<sal_uInt8>(aNewPal.GetBestIndex( BitmapColor( static_cast<sal_uInt8>(nR), static_cast<sal_uInt8>(nG), static_cast<sal_uInt8>(nB) ) ));

            if( pRAcc->HasPalette() )
            {
                for( long nY = 0; nY < nHeight; nY++ )
                {
                    Scanline pScanline = pWAcc->GetScanline(nY);
                    Scanline pScanlineRead = pRAcc->GetScanline(nY);
                    for( long nX = 0; nX < nWidth; nX++ )
                    {
                        const BitmapColor& rCol = pRAcc->GetPaletteColor( pRAcc->GetIndexFromData( pScanlineRead, nX ) );
                        aDstCol.SetIndex( pIndexMap[ ( ( static_cast<sal_uInt32>(rCol.GetRed()) >> nRightShiftBits ) << nLeftShiftBits2 ) |
                                                     ( ( static_cast<sal_uInt32>(rCol.GetGreen()) >> nRightShiftBits ) << nLeftShiftBits1 ) |
                                                     ( static_cast<sal_uInt32>(rCol.GetBlue()) >> nRightShiftBits ) ] );
                        pWAcc->SetPixelOnData( pScanline, nX, aDstCol );
                    }
                }
            }
            else
            {
                for( long nY = 0; nY < nHeight; nY++ )
                {
                    Scanline pScanline = pWAcc->GetScanline(nY);
                    Scanline pScanlineRead = pRAcc->GetScanline(nY);
                    for( long nX = 0; nX < nWidth; nX++ )
                    {
                        const BitmapColor aCol( pRAcc->GetPixelFromData( pScanlineRead, nX ) );
                        aDstCol.SetIndex( pIndexMap[ ( ( static_cast<sal_uInt32>(aCol.GetRed()) >> nRightShiftBits ) << nLeftShiftBits2 ) |
                                                     ( ( static_cast<sal_uInt32>(aCol.GetGreen()) >> nRightShiftBits ) << nLeftShiftBits1 ) |
                                                     ( static_cast<sal_uInt32>(aCol.GetBlue()) >> nRightShiftBits ) ] );
                        pWAcc->SetPixelOnData( pScanline, nX, aDstCol );
                    }
                }
            }

            pWAcc.reset();
            bRet = true;
        }

        pCountTable.reset();
        pRAcc.reset();

        if( bRet )
        {
            const MapMode aMap( maPrefMapMode );
            const Size aSize( maPrefSize );

            *this = aNewBmp;
            maPrefMapMode = aMap;
            maPrefSize = aSize;
        }
    }

    return bRet;
}

bool Bitmap::ImplReduceMedian( sal_uInt16 nColCount )
{
    ScopedReadAccess pRAcc(*this);
    sal_uInt16 nBitCount;
    bool bRet = false;

    if( nColCount < 17 )
        nBitCount = 4;
    else if( nColCount < 257 )
        nBitCount = 8;
    else
    {
        OSL_FAIL( "Bitmap::ImplReduceMedian(): invalid color count!" );
        nBitCount = 8;
        nColCount = 256;
    }

    if( pRAcc )
    {
        Bitmap aNewBmp( GetSizePixel(), nBitCount );
        BitmapScopedWriteAccess pWAcc(aNewBmp);

        if( pWAcc )
        {
            const sal_uLong nSize = 32768 * sizeof( sal_uLong );
            sal_uLong* pColBuf = static_cast<sal_uLong*>(rtl_allocateMemory( nSize ));
            const long nWidth = pWAcc->Width();
            const long nHeight = pWAcc->Height();
            long nIndex = 0;

            memset( pColBuf, 0, nSize );

            // create Buffer
            if( pRAcc->HasPalette() )
            {
                for( long nY = 0; nY < nHeight; nY++ )
                {
                    Scanline pScanlineRead = pRAcc->GetScanline(nY);
                    for( long nX = 0; nX < nWidth; nX++ )
                    {
                        const BitmapColor& rCol = pRAcc->GetPaletteColor( pRAcc->GetIndexFromData( pScanlineRead, nX ) );
                        pColBuf[ RGB15( rCol.GetRed() >> 3, rCol.GetGreen() >> 3, rCol.GetBlue() >> 3 ) ]++;
                    }
                }
            }
            else
            {
                for( long nY = 0; nY < nHeight; nY++ )
                {
                    Scanline pScanlineRead = pRAcc->GetScanline(nY);
                    for( long nX = 0; nX < nWidth; nX++ )
                    {
                        const BitmapColor aCol( pRAcc->GetPixelFromData( pScanlineRead, nX ) );
                        pColBuf[ RGB15( aCol.GetRed() >> 3, aCol.GetGreen() >> 3, aCol.GetBlue() >> 3 ) ]++;
                    }
                }
            }

            // create palette via median cut
            BitmapPalette aPal( pWAcc->GetPaletteEntryCount() );
            ImplMedianCut( pColBuf, aPal, 0, 31, 0, 31, 0, 31,
                           nColCount, nWidth * nHeight, nIndex );

            // do mapping of colors to palette
            InverseColorMap aMap( aPal );
            pWAcc->SetPalette( aPal );
            for( long nY = 0; nY < nHeight; nY++ )
            {
                Scanline pScanline = pWAcc->GetScanline(nY);
                for( long nX = 0; nX < nWidth; nX++ )
                    pWAcc->SetPixelOnData( pScanline, nX, BitmapColor(static_cast<sal_uInt8>( aMap.GetBestPaletteIndex( pRAcc->GetColor( nY, nX ) ))) );
            }

            rtl_freeMemory( pColBuf );
            pWAcc.reset();
            bRet = true;
        }

        pRAcc.reset();

        if( bRet )
        {
            const MapMode aMap( maPrefMapMode );
            const Size aSize( maPrefSize );

            *this = aNewBmp;
            maPrefMapMode = aMap;
            maPrefSize = aSize;
        }
    }

    return bRet;
}

void Bitmap::ImplMedianCut( sal_uLong* pColBuf, BitmapPalette& rPal,
                            long nR1, long nR2, long nG1, long nG2, long nB1, long nB2,
                            long nColors, long nPixels, long& rIndex )
{
    if( !nPixels )
        return;

    BitmapColor aCol;
    const long nRLen = nR2 - nR1;
    const long nGLen = nG2 - nG1;
    const long nBLen = nB2 - nB1;
    sal_uLong* pBuf = pColBuf;

    if( !nRLen && !nGLen && !nBLen )
    {
        if( pBuf[ RGB15( nR1, nG1, nB1 ) ] )
        {
            aCol.SetRed( static_cast<sal_uInt8>( nR1 << 3 ) );
            aCol.SetGreen( static_cast<sal_uInt8>( nG1 << 3 ) );
            aCol.SetBlue( static_cast<sal_uInt8>( nB1 << 3 ) );
            rPal[ static_cast<sal_uInt16>(rIndex++) ] = aCol;
        }
    }
    else
    {
        if( 1 == nColors || 1 == nPixels )
        {
            long nPixSum = 0, nRSum = 0, nGSum = 0, nBSum = 0;

            for( long nR = nR1; nR <= nR2; nR++ )
            {
                for( long nG = nG1; nG <= nG2; nG++ )
                {
                    for( long nB = nB1; nB <= nB2; nB++ )
                    {
                        nPixSum = pBuf[ RGB15( nR, nG, nB ) ];

                        if( nPixSum )
                        {
                            nRSum += nR * nPixSum;
                            nGSum += nG * nPixSum;
                            nBSum += nB * nPixSum;
                        }
                    }
                }
            }

            aCol.SetRed( static_cast<sal_uInt8>( ( nRSum / nPixels ) << 3 ) );
            aCol.SetGreen( static_cast<sal_uInt8>( ( nGSum / nPixels ) << 3 ) );
            aCol.SetBlue( static_cast<sal_uInt8>( ( nBSum / nPixels ) << 3 ) );
            rPal[ static_cast<sal_uInt16>(rIndex++) ] = aCol;
        }
        else
        {
            const long nTest = ( nPixels >> 1 );
            long nPixOld = 0;
            long nPixNew = 0;

            if( nBLen > nGLen && nBLen > nRLen )
            {
                long nB = nB1 - 1;

                while( nPixNew < nTest )
                {
                    nB++;
                    nPixOld = nPixNew;
                    for( long nR = nR1; nR <= nR2; nR++ )
                        for( long nG = nG1; nG <= nG2; nG++ )
                            nPixNew += pBuf[ RGB15( nR, nG, nB ) ];
                }

                if( nB < nB2 )
                {
                    ImplMedianCut( pBuf, rPal, nR1, nR2, nG1, nG2, nB1, nB, nColors >> 1, nPixNew, rIndex );
                    ImplMedianCut( pBuf, rPal, nR1, nR2, nG1, nG2, nB + 1, nB2, nColors >> 1, nPixels - nPixNew, rIndex );
                }
                else
                {
                    ImplMedianCut( pBuf, rPal, nR1, nR2, nG1, nG2, nB1, nB - 1, nColors >> 1, nPixOld, rIndex );
                    ImplMedianCut( pBuf, rPal, nR1, nR2, nG1, nG2, nB, nB2, nColors >> 1, nPixels - nPixOld, rIndex );
                }
            }
            else if( nGLen > nRLen )
            {
                long nG = nG1 - 1;

                while( nPixNew < nTest )
                {
                    nG++;
                    nPixOld = nPixNew;
                    for( long nR = nR1; nR <= nR2; nR++ )
                        for( long nB = nB1; nB <= nB2; nB++ )
                            nPixNew += pBuf[ RGB15( nR, nG, nB ) ];
                }

                if( nG < nG2 )
                {
                    ImplMedianCut( pBuf, rPal, nR1, nR2, nG1, nG, nB1, nB2, nColors >> 1, nPixNew, rIndex );
                    ImplMedianCut( pBuf, rPal, nR1, nR2, nG + 1, nG2, nB1, nB2, nColors >> 1, nPixels - nPixNew, rIndex );
                }
                else
                {
                    ImplMedianCut( pBuf, rPal, nR1, nR2, nG1, nG - 1, nB1, nB2, nColors >> 1, nPixOld, rIndex );
                    ImplMedianCut( pBuf, rPal, nR1, nR2, nG, nG2, nB1, nB2, nColors >> 1, nPixels - nPixOld, rIndex );
                }
            }
            else
            {
                long nR = nR1 - 1;

                while( nPixNew < nTest )
                {
                    nR++;
                    nPixOld = nPixNew;
                    for( long nG = nG1; nG <= nG2; nG++ )
                        for( long nB = nB1; nB <= nB2; nB++ )
                            nPixNew += pBuf[ RGB15( nR, nG, nB ) ];
                }

                if( nR < nR2 )
                {
                    ImplMedianCut( pBuf, rPal, nR1, nR, nG1, nG2, nB1, nB2, nColors >> 1, nPixNew, rIndex );
                    ImplMedianCut( pBuf, rPal, nR1 + 1, nR2, nG1, nG2, nB1, nB2, nColors >> 1, nPixels - nPixNew, rIndex );
                }
                else
                {
                    ImplMedianCut( pBuf, rPal, nR1, nR - 1, nG1, nG2, nB1, nB2, nColors >> 1, nPixOld, rIndex );
                    ImplMedianCut( pBuf, rPal, nR, nR2, nG1, nG2, nB1, nB2, nColors >> 1, nPixels - nPixOld, rIndex );
                }
            }
        }
    }
}

void Bitmap::Vectorize( GDIMetaFile& rMtf, sal_uInt8 cReduce, const Link<long,void>* pProgress )
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
            const long nW = pAcc->Width();
            const long nH = pAcc->Height();
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
            for( long nX = 0; nX < 256; nX++ )
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
                for( long nY = 0; nY < nH; nY++ )
                {
                    Scanline pScan = pAcc->GetScanline( nY );

                    for( long nX = 0; nX < nW; nX++ )
                    {
                        *pScan = cMapB[ *pScan ]; pScan++;
                        *pScan = cMapG[ *pScan ]; pScan++;
                        *pScan = cMapR[ *pScan ]; pScan++;
                    }
                }
            }
            else if( pAcc->GetScanlineFormat() == ScanlineFormat::N24BitTcRgb )
            {
                for( long nY = 0; nY < nH; nY++ )
                {
                    Scanline pScan = pAcc->GetScanline( nY );

                    for( long nX = 0; nX < nW; nX++ )
                    {
                        *pScan = cMapR[ *pScan ]; pScan++;
                        *pScan = cMapG[ *pScan ]; pScan++;
                        *pScan = cMapB[ *pScan ]; pScan++;
                    }
                }
            }
            else
            {
                for( long nY = 0; nY < nH; nY++ )
                {
                    Scanline pScanline = pAcc->GetScanline(nY);
                    for( long nX = 0; nX < nW; nX++ )
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
