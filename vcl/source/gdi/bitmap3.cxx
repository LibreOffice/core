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

#include "floyd.hxx"
#include "impvect.hxx"

#include <memory>

const extern sal_uLong nVCLRLut[ 6 ] = { 16, 17, 18, 19, 20, 21 };
const extern sal_uLong nVCLGLut[ 6 ] = { 0, 6, 12, 18, 24, 30 };
const extern sal_uLong nVCLBLut[ 6 ] = { 0, 36, 72, 108, 144, 180 };

const extern sal_uLong nVCLDitherLut[ 256 ] =
{
       0, 49152, 12288, 61440,  3072, 52224, 15360, 64512,   768, 49920, 13056,
   62208,  3840, 52992, 16128, 65280, 32768, 16384, 45056, 28672, 35840, 19456,
   48128, 31744, 33536, 17152, 45824, 29440, 36608, 20224, 48896, 32512, 8192,
   57344,  4096, 53248, 11264, 60416,  7168, 56320,  8960, 58112,  4864, 54016,
   12032, 61184,  7936, 57088, 40960, 24576, 36864, 20480, 44032, 27648, 39936,
   23552, 41728, 25344, 37632, 21248, 44800, 28416, 40704, 24320, 2048, 51200,
   14336, 63488,  1024, 50176, 13312, 62464,  2816, 51968, 15104, 64256,  1792,
   50944, 14080, 63232, 34816, 18432, 47104, 30720, 33792, 17408, 46080, 29696,
   35584, 19200, 47872, 31488, 34560, 18176, 46848, 30464, 10240, 59392,  6144,
   55296,  9216, 58368,  5120, 54272, 11008, 60160,  6912, 56064,  9984, 59136,
    5888, 55040, 43008, 26624, 38912, 22528, 41984, 25600, 37888, 21504, 43776,
   27392, 39680, 23296, 42752, 26368, 38656, 22272,   512, 49664, 12800, 61952,
    3584, 52736, 15872, 65024,   256, 49408, 12544, 61696,  3328, 52480, 15616,
   64768, 33280, 16896, 45568, 29184, 36352, 19968, 48640, 32256, 33024, 16640,
   45312, 28928, 36096, 19712, 48384, 32000,  8704, 57856,  4608, 53760, 11776,
   60928,  7680, 56832,  8448, 57600,  4352, 53504, 11520, 60672,  7424, 56576,
   41472, 25088, 37376, 20992, 44544, 28160, 40448, 24064, 41216, 24832, 37120,
   20736, 44288, 27904, 40192, 23808,  2560, 51712, 14848, 64000,  1536, 50688,
   13824, 62976,  2304, 51456, 14592, 63744,  1280, 50432, 13568, 62720, 35328,
   18944, 47616, 31232, 34304, 17920, 46592, 30208, 35072, 18688, 47360, 30976,
   34048, 17664, 46336, 29952, 10752, 59904,  6656, 55808,  9728, 58880,  5632,
   54784, 10496, 59648,  6400, 55552,  9472, 58624,  5376, 54528, 43520, 27136,
   39424, 23040, 42496, 26112, 38400, 22016, 43264, 26880, 39168, 22784, 42240,
   25856, 38144, 21760
};

const extern sal_uLong nVCLLut[ 256 ] =
{
         0,  1286,  2572,  3858,  5144,  6430,  7716,  9002,
     10288, 11574, 12860, 14146, 15432, 16718, 18004, 19290,
     20576, 21862, 23148, 24434, 25720, 27006, 28292, 29578,
     30864, 32150, 33436, 34722, 36008, 37294, 38580, 39866,
     41152, 42438, 43724, 45010, 46296, 47582, 48868, 50154,
     51440, 52726, 54012, 55298, 56584, 57870, 59156, 60442,
     61728, 63014, 64300, 65586, 66872, 68158, 69444, 70730,
     72016, 73302, 74588, 75874, 77160, 78446, 79732, 81018,
     82304, 83590, 84876, 86162, 87448, 88734, 90020, 91306,
     92592, 93878, 95164, 96450, 97736, 99022,100308,101594,
    102880,104166,105452,106738,108024,109310,110596,111882,
    113168,114454,115740,117026,118312,119598,120884,122170,
    123456,124742,126028,127314,128600,129886,131172,132458,
    133744,135030,136316,137602,138888,140174,141460,142746,
    144032,145318,146604,147890,149176,150462,151748,153034,
    154320,155606,156892,158178,159464,160750,162036,163322,
    164608,165894,167180,168466,169752,171038,172324,173610,
    174896,176182,177468,178754,180040,181326,182612,183898,
    185184,186470,187756,189042,190328,191614,192900,194186,
    195472,196758,198044,199330,200616,201902,203188,204474,
    205760,207046,208332,209618,210904,212190,213476,214762,
    216048,217334,218620,219906,221192,222478,223764,225050,
    226336,227622,228908,230194,231480,232766,234052,235338,
    236624,237910,239196,240482,241768,243054,244340,245626,
    246912,248198,249484,250770,252056,253342,254628,255914,
    257200,258486,259772,261058,262344,263630,264916,266202,
    267488,268774,270060,271346,272632,273918,275204,276490,
    277776,279062,280348,281634,282920,284206,285492,286778,
    288064,289350,290636,291922,293208,294494,295780,297066,
    298352,299638,300924,302210,303496,304782,306068,307354,
    308640,309926,311212,312498,313784,315070,316356,317642,
    318928,320214,321500,322786,324072,325358,326644,327930
};

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

                sal_uLong nShift = ((nGreys == 16) ? 4UL : 0UL);

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

bool Bitmap::Dither()
{
    const Size aSize( GetSizePixel() );

    if( aSize.Width() == 1 || aSize.Height() == 1 )
        return true;

    if( ( aSize.Width() > 3 ) && ( aSize.Height() > 2 ) )
    {
        ScopedReadAccess pReadAcc(*this);
        Bitmap aNewBmp( GetSizePixel(), 8 );
        BitmapScopedWriteAccess pWriteAcc(aNewBmp);

        if( pReadAcc && pWriteAcc )
        {
            BitmapColor aColor;
            tools::Long nWidth = pReadAcc->Width();
            tools::Long nWidth1 = nWidth - 1;
            tools::Long nHeight = pReadAcc->Height();
            tools::Long nW = nWidth * 3;
            tools::Long nW2 = nW - 3;
            std::unique_ptr<tools::Long[]> p1(new tools::Long[ nW ]);
            std::unique_ptr<tools::Long[]> p2(new tools::Long[ nW ]);
            tools::Long* p1T = p1.get();
            tools::Long* p2T = p2.get();
            tools::Long* pTmp;

            bool bPal = pReadAcc->HasPalette();
            pTmp = p2T;

            if( bPal )
            {
                Scanline pScanlineRead = pReadAcc->GetScanline(0);
                for( tools::Long nZ = 0; nZ < nWidth; nZ++ )
                {
                    aColor = pReadAcc->GetPaletteColor( pReadAcc->GetIndexFromData( pScanlineRead, nZ ) );

                    *pTmp++ = static_cast<tools::Long>(aColor.GetBlue()) << 12;
                    *pTmp++ = static_cast<tools::Long>(aColor.GetGreen()) << 12;
                    *pTmp++ = static_cast<tools::Long>(aColor.GetRed()) << 12;
                }
            }
            else
            {
                Scanline pScanlineRead = pReadAcc->GetScanline(0);
                for( tools::Long nZ = 0; nZ < nWidth; nZ++ )
                {
                    aColor = pReadAcc->GetPixelFromData( pScanlineRead, nZ );

                    *pTmp++ = static_cast<tools::Long>(aColor.GetBlue()) << 12;
                    *pTmp++ = static_cast<tools::Long>(aColor.GetGreen()) << 12;
                    *pTmp++ = static_cast<tools::Long>(aColor.GetRed()) << 12;
                }
            }

            tools::Long nRErr, nGErr, nBErr;
            tools::Long nRC, nGC, nBC;

            for( tools::Long nY = 1, nYAcc = 0; nY <= nHeight; nY++, nYAcc++ )
            {
                pTmp = p1T;
                p1T = p2T;
                p2T = pTmp;

                if( nY < nHeight )
                {
                    if( bPal )
                    {
                        Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                        for( tools::Long nZ = 0; nZ < nWidth; nZ++ )
                        {
                            aColor = pReadAcc->GetPaletteColor( pReadAcc->GetIndexFromData( pScanlineRead, nZ ) );

                            *pTmp++ = static_cast<tools::Long>(aColor.GetBlue()) << 12;
                            *pTmp++ = static_cast<tools::Long>(aColor.GetGreen()) << 12;
                            *pTmp++ = static_cast<tools::Long>(aColor.GetRed()) << 12;
                        }
                    }
                    else
                    {
                        Scanline pScanlineRead = pReadAcc->GetScanline(nY);
                        for( tools::Long nZ = 0; nZ < nWidth; nZ++ )
                        {
                            aColor = pReadAcc->GetPixelFromData( pScanlineRead, nZ );

                            *pTmp++ = static_cast<tools::Long>(aColor.GetBlue()) << 12;
                            *pTmp++ = static_cast<tools::Long>(aColor.GetGreen()) << 12;
                            *pTmp++ = static_cast<tools::Long>(aColor.GetRed()) << 12;
                        }
                    }
                }

                // Examine first Pixel separately
                tools::Long nX = 0;
                tools::Long nTemp;
                CALC_ERRORS;
                CALC_TABLES7;
                nX -= 5;
                CALC_TABLES5;
                Scanline pScanline = pWriteAcc->GetScanline(nYAcc);
                pWriteAcc->SetPixelOnData( pScanline, 0, BitmapColor(static_cast<sal_uInt8>(nVCLBLut[ nBC ] + nVCLGLut[nGC ] + nVCLRLut[nRC ])) );

                // Get middle Pixels using a loop
                tools::Long nXAcc;
                for ( nX = 3, nXAcc = 1; nX < nW2; nXAcc++ )
                {
                    CALC_ERRORS;
                    CALC_TABLES7;
                    nX -= 8;
                    CALC_TABLES3;
                    CALC_TABLES5;
                    pWriteAcc->SetPixelOnData( pScanline, nXAcc, BitmapColor(static_cast<sal_uInt8>(nVCLBLut[ nBC ] + nVCLGLut[nGC ] + nVCLRLut[nRC ])) );
                }

                // Treat last Pixel separately
                CALC_ERRORS;
                nX -= 5;
                CALC_TABLES3;
                CALC_TABLES5;
                pWriteAcc->SetPixelOnData( pScanline, nWidth1, BitmapColor(static_cast<sal_uInt8>(nVCLBLut[ nBC ] + nVCLGLut[nGC ] + nVCLRLut[nRC ])) );
            }

            pReadAcc.reset();
            pWriteAcc.reset();

            const MapMode aMap( maPrefMapMode );
            const Size aPrefSize( maPrefSize );

            *this = aNewBmp;

            maPrefMapMode = aMap;
            maPrefSize = aPrefSize;

            return true;
        }

        pReadAcc.reset();
        pWriteAcc.reset();
    }

    return false;
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
