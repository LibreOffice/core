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
#include <memory>

#include "impbmp.hxx"
#include "impoctree.hxx"
#include "impvect.hxx"

#include <bitmapscalesuper.hxx>
#include "octree.hxx"
#include "BitmapScaleConvolution.hxx"

#define RGB15( _def_cR, _def_cG, _def_cB )  (((sal_uLong)(_def_cR)<<10UL)|((sal_uLong)(_def_cG)<<5UL)|(sal_uLong)(_def_cB))
#define GAMMA( _def_cVal, _def_InvGamma )   ((sal_uInt8)MinMax(FRound(pow( _def_cVal/255.0,_def_InvGamma)*255.0),0,255))

#define CALC_ERRORS                                                             \
                        nTemp = p1T[nX++] >> 12;                              \
                        nBErr = MinMax( nTemp, 0, 255 );                        \
                        nBErr = nBErr - FloydIndexMap[ nBC = FloydMap[nBErr] ]; \
                        nTemp = p1T[nX++] >> 12;                              \
                        nGErr = MinMax( nTemp, 0, 255 );                        \
                        nGErr = nGErr - FloydIndexMap[ nGC = FloydMap[nGErr] ]; \
                        nTemp = p1T[nX] >> 12;                                \
                        nRErr = MinMax( nTemp, 0, 255 );                        \
                        nRErr = nRErr - FloydIndexMap[ nRC = FloydMap[nRErr] ];

#define CALC_TABLES3                                        \
                        p2T[nX++] += FloydError3[nBErr];    \
                        p2T[nX++] += FloydError3[nGErr];    \
                        p2T[nX++] += FloydError3[nRErr];

#define CALC_TABLES5                                        \
                        p2T[nX++] += FloydError5[nBErr];    \
                        p2T[nX++] += FloydError5[nGErr];    \
                        p2T[nX++] += FloydError5[nRErr];

#define CALC_TABLES7                                        \
                        p1T[++nX] += FloydError7[nBErr];    \
                        p2T[nX++] += FloydError1[nBErr];    \
                        p1T[nX] += FloydError7[nGErr];      \
                        p2T[nX++] += FloydError1[nGErr];    \
                        p1T[nX] += FloydError7[nRErr];      \
                        p2T[nX] += FloydError1[nRErr];

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

const long FloydMap[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5
};

const long FloydError1[61] =
{
    -7680, -7424, -7168, -6912, -6656, -6400, -6144,
    -5888, -5632, -5376, -5120, -4864, -4608, -4352,
    -4096, -3840, -3584, -3328, -3072, -2816, -2560,
    -2304, -2048, -1792, -1536, -1280, -1024, -768,
    -512, -256, 0, 256, 512, 768, 1024, 1280, 1536,
    1792, 2048, 2304, 2560, 2816, 3072, 3328, 3584,
    3840, 4096, 4352, 4608, 4864, 5120, 5376, 5632,
    5888, 6144, 6400, 6656, 6912, 7168, 7424, 7680
};

const long FloydError3[61] =
{
    -23040, -22272, -21504, -20736, -19968, -19200,
    -18432, -17664, -16896, -16128, -15360, -14592,
    -13824, -13056, -12288, -11520, -10752, -9984,
    -9216, -8448, -7680, -6912, -6144, -5376, -4608,
    -3840, -3072, -2304, -1536, -768, 0, 768, 1536,
    2304, 3072, 3840, 4608, 5376, 6144, 6912, 7680,
    8448, 9216, 9984, 10752, 11520, 12288, 13056,
    13824, 14592, 15360, 16128, 16896, 17664, 18432,
    19200, 19968, 20736, 21504, 22272, 23040
};

const long FloydError5[61] =
{
    -38400, -37120, -35840, -34560, -33280, -32000,
    -30720, -29440, -28160, -26880, -25600, -24320,
    -23040, -21760, -20480, -19200, -17920, -16640,
    -15360, -14080, -12800, -11520, -10240, -8960,
    -7680, -6400, -5120, -3840, -2560, -1280,   0,
    1280, 2560, 3840, 5120, 6400, 7680, 8960, 10240,
    11520, 12800, 14080, 15360, 16640, 17920, 19200,
    20480, 21760, 23040, 24320, 25600, 26880, 28160,
    29440, 30720, 32000, 33280, 34560, 35840, 37120,
    38400
};

const long FloydError7[61] =
{
    -53760, -51968, -50176, -48384, -46592, -44800,
    -43008, -41216, -39424, -37632, -35840, -34048,
    -32256, -30464, -28672, -26880, -25088, -23296,
    -21504, -19712, -17920, -16128, -14336, -12544,
    -10752, -8960, -7168, -5376, -3584, -1792,  0,
    1792, 3584, 5376, 7168, 8960, 10752, 12544, 14336,
    16128, 17920, 19712, 21504, 23296, 25088, 26880,
    28672, 30464, 32256, 34048, 35840, 37632, 39424,
    41216, 43008, 44800, 46592, 48384, 50176, 51968,
    53760
};

const long FloydIndexMap[6] =
{
    -30,  21, 72, 123, 174, 225
};

void ImplCreateDitherMatrix( sal_uInt8 (*pDitherMatrix)[16][16] )
{
    const double fVal = 3.125;
    const double fVal16 = fVal / 16.;
    const double fValScale = 254.;
    sal_uInt16 pMtx[ 16 ][ 16 ];
    sal_uInt16 nMax = 0;
    static const sal_uInt8 pMagic[4][4] = { { 0, 14,  3, 13, },
                                     {11,  5,  8,  6, },
                                     {12,  2, 15,  1, },
                                     {7,   9,  4, 10 } };

    // Build MagicSquare
    for ( long i = 0; i < 4; i++ )
       for ( long j = 0; j < 4; j++ )
           for ( long k = 0; k < 4; k++ )
                for ( long l = 0; l < 4; l++ )
                {
                    pMtx[ (k<<2) + i][(l<<2 ) + j ] = (sal_uInt16) ( 0.5 + pMagic[i][j]*fVal + pMagic[k][l]*fVal16 );
                    nMax = std::max ( pMtx[ (k<<2) + i][(l<<2 ) + j], nMax );
                }

    // Scale to interval [0;254]
    double tmp = fValScale / nMax;
    for ( long i = 0; i < 16; i++ )
        for( long j = 0; j < 16; j++ )
            (*pDitherMatrix)[i][j] = (sal_uInt8) ( tmp * pMtx[i][j] );
}

bool Bitmap::Convert( BmpConversion eConversion )
{
    // try to convert in backend
    if (mxImpBmp)
    {
        std::shared_ptr<ImpBitmap> xImpBmp(new ImpBitmap);
        if (xImpBmp->ImplCreate(*mxImpBmp) && xImpBmp->ImplConvert(eConversion))
        {
            ImplSetImpBitmap(xImpBmp);
            SAL_INFO( "vcl.opengl", "Ref count: " << mxImpBmp.use_count() );
            return true;
        }
    }

    const sal_uInt16 nBitCount = GetBitCount ();
    bool bRet = false;

    switch( eConversion )
    {
        case BmpConversion::N1BitThreshold:
            bRet = ImplMakeMono( 128 );
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

bool Bitmap::ImplMakeMono( sal_uInt8 cThreshold )
{
    ScopedReadAccess pReadAcc(*this);
    bool bRet = false;

    if( pReadAcc )
    {
        Bitmap aNewBmp( GetSizePixel(), 1 );
        ScopedWriteAccess pWriteAcc(aNewBmp);

        if( pWriteAcc )
        {
            const BitmapColor aBlack( pWriteAcc->GetBestMatchingColor( Color( COL_BLACK ) ) );
            const BitmapColor aWhite( pWriteAcc->GetBestMatchingColor( Color( COL_WHITE ) ) );
            const long nWidth = pWriteAcc->Width();
            const long nHeight = pWriteAcc->Height();

            if( pReadAcc->HasPalette() )
            {
                for( long nY = 0; nY < nHeight; nY++ )
                {
                    for( long nX = 0; nX < nWidth; nX++ )
                    {
                        const sal_uInt8 cIndex = pReadAcc->GetPixelIndex( nY, nX );
                        if( pReadAcc->GetPaletteColor( cIndex ).GetLuminance() >=
                            cThreshold )
                        {
                            pWriteAcc->SetPixel( nY, nX, aWhite );
                        }
                        else
                            pWriteAcc->SetPixel( nY, nX, aBlack );
                    }
                }
            }
            else
            {
                for( long nY = 0; nY < nHeight; nY++ )
                {
                    for( long nX = 0; nX < nWidth; nX++ )
                    {
                        if( pReadAcc->GetPixel( nY, nX ).GetLuminance() >=
                            cThreshold )
                        {
                            pWriteAcc->SetPixel( nY, nX, aWhite );
                        }
                        else
                            pWriteAcc->SetPixel( nY, nX, aBlack );
                    }
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

    return bRet;
}

bool Bitmap::ImplMakeMonoDither()
{
    ScopedReadAccess pReadAcc(*this);
    bool bRet = false;

    if( pReadAcc )
    {
        Bitmap aNewBmp( GetSizePixel(), 1 );
        ScopedWriteAccess pWriteAcc(aNewBmp);

        if( pWriteAcc )
        {
            const BitmapColor aBlack( pWriteAcc->GetBestMatchingColor( Color( COL_BLACK ) ) );
            const BitmapColor aWhite( pWriteAcc->GetBestMatchingColor( Color( COL_WHITE ) ) );
            const long nWidth = pWriteAcc->Width();
            const long nHeight = pWriteAcc->Height();
            sal_uInt8 pDitherMatrix[ 16 ][ 16 ];

            ImplCreateDitherMatrix( &pDitherMatrix );

            if( pReadAcc->HasPalette() )
            {
                for( long nY = 0; nY < nHeight; nY++ )
                {
                    for( long nX = 0, nModY = nY % 16; nX < nWidth; nX++ )
                    {
                        const sal_uInt8 cIndex = pReadAcc->GetPixelIndex( nY, nX );
                        if( pReadAcc->GetPaletteColor( cIndex ).GetLuminance() >
                            pDitherMatrix[ nModY ][ nX % 16 ] )
                        {
                            pWriteAcc->SetPixel( nY, nX, aWhite );
                        }
                        else
                            pWriteAcc->SetPixel( nY, nX, aBlack );
                    }
                }
            }
            else
            {
                for( long nY = 0; nY < nHeight; nY++ )
                {
                    for( long nX = 0, nModY = nY % 16; nX < nWidth; nX++ )
                    {
                        if( pReadAcc->GetPixel( nY, nX ).GetLuminance() >
                            pDitherMatrix[ nModY ][ nX % 16 ] )
                        {
                            pWriteAcc->SetPixel( nY, nX, aWhite );
                        }
                        else
                            pWriteAcc->SetPixel( nY, nX, aBlack );
                    }
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
            bPalDiffers = ( (BitmapPalette&) rPal != pReadAcc->GetPalette() );

        if( bPalDiffers )
        {
            Bitmap aNewBmp( GetSizePixel(), ( nGreys == 16 ) ? 4 : 8, &rPal );
            ScopedWriteAccess pWriteAcc(aNewBmp);

            if( pWriteAcc )
            {
                const long nWidth = pWriteAcc->Width();
                const long nHeight = pWriteAcc->Height();

                if( pReadAcc->HasPalette() )
                {
                    for( long nY = 0; nY < nHeight; nY++ )
                    {
                        for( long nX = 0; nX < nWidth; nX++ )
                        {
                            const sal_uInt8 cIndex = pReadAcc->GetPixelIndex( nY, nX );
                            pWriteAcc->SetPixelIndex( nY, nX,
                                (pReadAcc->GetPaletteColor( cIndex ).GetLuminance() >> nShift) );
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

                            *pWriteScan++ = (sal_uInt8) ( ( nB * 28UL + nG * 151UL + nR * 77UL ) >> nShift );
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

                            *pWriteScan++ = (sal_uInt8) ( ( nB * 28UL + nG * 151UL + nR * 77UL ) >> nShift );
                        }
                    }
                }
                else
                {
                    for( long nY = 0; nY < nHeight; nY++ )
                        for( long nX = 0; nX < nWidth; nX++ )
                            pWriteAcc->SetPixelIndex( nY, nX, (pReadAcc->GetPixel( nY, nX ) ).GetLuminance() >> nShift );
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

bool Bitmap::ImplConvertUp(sal_uInt16 nBitCount, Color* pExtColor)
{
    SAL_WARN_IF( nBitCount <= GetBitCount(), "vcl", "New BitCount must be greater!" );

    Bitmap::ScopedReadAccess pReadAcc(*this);
    bool bRet = false;

    if (pReadAcc)
    {
        BitmapPalette aPalette;
        Bitmap aNewBmp(GetSizePixel(), nBitCount, pReadAcc->HasPalette() ? &pReadAcc->GetPalette() : &aPalette);
        Bitmap::ScopedWriteAccess pWriteAcc(aNewBmp);

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
                    for (long nX = 0; nX < nWidth; nX++)
                    {
                        pWriteAcc->SetPixel(nY, nX, pReadAcc->GetPixel(nY, nX));
                    }
                }
            }
            else
            {
                if (pReadAcc->HasPalette())
                {
                    for (long nY = 0; nY < nHeight; nY++)
                    {
                        for (long nX = 0; nX < nWidth; nX++)
                        {
                            pWriteAcc->SetPixel(nY, nX, pReadAcc->GetPaletteColor(pReadAcc->GetPixelIndex(nY, nX)));
                        }
                    }
                }
                else
                {
                    for (long nY = 0; nY < nHeight; nY++)
                    {
                        for (long nX = 0; nX < nWidth; nX++)
                        {
                            pWriteAcc->SetPixel(nY, nX, pReadAcc->GetPixel(nY, nX));
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

bool Bitmap::ImplConvertDown(sal_uInt16 nBitCount, Color* pExtColor)
{
    SAL_WARN_IF(nBitCount > GetBitCount(), "vcl", "New BitCount must be lower ( or equal when pExtColor is set )!");

    Bitmap::ScopedReadAccess pReadAcc(*this);
    bool bRet = false;

    if (pReadAcc)
    {
        BitmapPalette aPalette;
        Bitmap aNewBmp(GetSizePixel(), nBitCount, &aPalette);
        Bitmap::ScopedWriteAccess pWriteAcc(aNewBmp);

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
                aPalette[aPalette.GetEntryCount() - 2] = Color(COL_BLACK);
                aPalette[aPalette.GetEntryCount() - 1] = Color(COL_WHITE);
            }

            pWriteAcc->SetPalette(aPalette);

            for (long nY = 0; nY < std::min(nHeight, 2L); nY++, nYTmp++)
            {
                pQLine2 = !nY ? aErrQuad1.data() : aErrQuad2.data();
                for (long nX = 0; nX < nWidth; nX++)
                {
                    if (pReadAcc->HasPalette())
                        pQLine2[nX] = pReadAcc->GetPaletteColor(pReadAcc->GetPixelIndex(nYTmp, nX));
                    else
                        pQLine2[nX] = pReadAcc->GetPixel(nYTmp, nX);
                }
            }

            for (long nY = 0; nY < nHeight; nY++, nYTmp++)
            {
                // first pixel in the line
                cIndex = (sal_uInt8) aColorMap.GetBestPaletteIndex(pQLine1[0].ImplGetColor());
                pWriteAcc->SetPixelIndex(nY, 0, cIndex);

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
                    pWriteAcc->SetPixelIndex(nY, nX, cIndex);
                }

                // Last RowPixel
                if (nX < nWidth)
                {
                    cIndex = static_cast<sal_uInt8>(aColorMap.GetBestPaletteIndex(pQLine1[nWidth1].ImplGetColor()));
                    pWriteAcc->SetPixelIndex(nY, nX, cIndex);
                }

                // Refill/copy row buffer
                pQLine1 = pQLine2;
                bQ1 = !bQ1;
                pQLine2 = bQ1 ? aErrQuad2.data() : aErrQuad1.data();

                if (nYTmp < nHeight)
                {
                    for (nX = 0; nX < nWidth; nX++)
                    {
                        if (pReadAcc->HasPalette())
                            pQLine2[nX] = pReadAcc->GetPaletteColor(pReadAcc->GetPixelIndex(nYTmp, nX));
                        else
                            pQLine2[nX] = pReadAcc->GetPixel(nYTmp, nX);
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
                const BitmapColor& rOld = pR->GetPaletteColor( (sal_uInt16) i );
                aNewPal[ (sal_uInt16) i ] = BitmapColor( ( rOld.GetRed() >> 1 ) | 0x80,
                                                     ( rOld.GetGreen() >> 1 ) | 0x80,
                                                     ( rOld.GetBlue() >> 1 ) | 0x80 );
            }

            aNewBmp = Bitmap( GetSizePixel(), GetBitCount(), &aNewPal );
            ScopedWriteAccess pW(aNewBmp);

            if( pW )
            {
                pW->CopyBuffer( *pR );
                bRet = true;
            }
        }
        else
        {
            aNewBmp = Bitmap( GetSizePixel(), 24 );

            ScopedWriteAccess pW(aNewBmp);

            if( pW )
            {
                const long nWidth = pR->Width(), nHeight = pR->Height();

                for( long nY = 0; nY < nHeight; nY++ )
                {
                    for( long nX = 0; nX < nWidth; nX++ )
                    {
                        const BitmapColor aOld( pR->GetPixel( nY, nX ) );
                        pW->SetPixel( nY, nX, BitmapColor( ( aOld.GetRed() >> 1 ) | 0x80,
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

    if (mxImpBmp)
    {
        // implementation specific scaling
        std::shared_ptr<ImpBitmap> xImpBmp(new ImpBitmap);
        if (xImpBmp->ImplCreate(*mxImpBmp) && xImpBmp->ImplScale(rScaleX, rScaleY, nScaleFlag))
        {
            ImplSetImpBitmap(xImpBmp);
            SAL_INFO( "vcl.opengl", "Ref count: " << mxImpBmp.use_count() );
            maPrefMapMode = MapMode( MapUnit::MapPixel );
            maPrefSize = xImpBmp->ImplGetSize();
            return true;
        }
    }

    //fdo#33455
    //
    //If we start with a 1 bit image, then after scaling it in any mode except
    //BmpScaleFlag::Fast we have a 24bit image which is perfectly correct, but we
    //are going to down-shift it to mono again and Bitmap::ImplMakeMono just
    //has "Bitmap aNewBmp( GetSizePixel(), 1 );" to create a 1 bit bitmap which
    //will default to black/white and the colors mapped to which ever is closer
    //to black/white
    //
    //So the easiest thing to do to retain the colors of 1 bit bitmaps is to
    //just use the fast scale rather than attempting to count unique colors in
    //the other converters and pass all the info down through
    //Bitmap::ImplMakeMono
    if (nStartCount == 1)
        nScaleFlag = BmpScaleFlag::Fast;

    bool bRetval(false);

    switch(nScaleFlag)
    {
        case BmpScaleFlag::Fast :
        {
            bRetval = ImplScaleFast( rScaleX, rScaleY );
            break;
        }
        case BmpScaleFlag::Interpolate :
        {
            bRetval = ImplScaleInterpolate( rScaleX, rScaleY );
            break;
        }
        case BmpScaleFlag::Default:
        {
            if (GetSizePixel().Width() < 2 || GetSizePixel().Height() < 2)
            {
                // fallback to ImplScaleFast
                bRetval = ImplScaleFast( rScaleX, rScaleY );
            }
            else
            {
                BitmapScaleSuper aScaleSuper(rScaleX, rScaleY);
                bRetval = aScaleSuper.filter(*this);
            }
            break;
        }
        case BmpScaleFlag::Lanczos :
        case BmpScaleFlag::BestQuality:
        {
            vcl::BitmapScaleConvolution aScaleConvolution(rScaleX, rScaleY, vcl::ConvolutionKernelType::Lanczos3);
            bRetval = aScaleConvolution.filter(*this);
            break;
        }
        case BmpScaleFlag::BiCubic :
        {
            vcl::BitmapScaleConvolution aScaleConvolution(rScaleX, rScaleY, vcl::ConvolutionKernelType::BiCubic);
            bRetval = aScaleConvolution.filter(*this);
            break;
        }
        case BmpScaleFlag::BiLinear :
        {
            vcl::BitmapScaleConvolution aScaleConvolution(rScaleX, rScaleY, vcl::ConvolutionKernelType::BiLinear);
            bRetval = aScaleConvolution.filter(*this);
            break;
        }
    }

    OSL_ENSURE(!bRetval || nStartCount == GetBitCount(), "Bitmap::Scale has changed the ColorDepth, this should *not* happen (!)");
    return bRetval;
}

bool Bitmap::Scale( const Size& rNewSize, BmpScaleFlag nScaleFlag )
{
    const Size aSize( GetSizePixel() );
    bool bRet;

    if( aSize.Width() && aSize.Height() )
    {
        bRet = Scale( (double) rNewSize.Width() / aSize.Width(),
                      (double) rNewSize.Height() / aSize.Height(),
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
    ImplAdaptBitCount(rNew);
}

void Bitmap::ImplAdaptBitCount(Bitmap& rNew) const
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

bool Bitmap::ImplScaleFast( const double& rScaleX, const double& rScaleY )
{
    const Size aSizePix( GetSizePixel() );
    const long nNewWidth = FRound( aSizePix.Width() * rScaleX );
    const long nNewHeight = FRound( aSizePix.Height() * rScaleY );
    bool bRet = false;

    if( nNewWidth && nNewHeight )
    {
        ScopedReadAccess pReadAcc(*this);

        if(pReadAcc)
        {
            Bitmap aNewBmp( Size( nNewWidth, nNewHeight ), GetBitCount(), &pReadAcc->GetPalette() );
            ScopedWriteAccess pWriteAcc(aNewBmp);

            if( pWriteAcc )
            {
                const long nScanlineSize = pWriteAcc->GetScanlineSize();
                const long nNewWidth1 = nNewWidth - 1;
                const long nNewHeight1 = nNewHeight - 1;

                if( nNewWidth1 && nNewHeight1 )
                {
                    const double nWidth = pReadAcc->Width();
                    const double nHeight = pReadAcc->Height();
                    std::unique_ptr<long[]> pLutX(new long[ nNewWidth ]);
                    std::unique_ptr<long[]> pLutY(new long[ nNewHeight ]);

                    for( long nX = 0; nX < nNewWidth; nX++ )
                        pLutX[ nX ] = long(nX * nWidth / nNewWidth);

                    for( long nY = 0; nY < nNewHeight; nY++ )
                        pLutY[ nY ] = long(nY * nHeight / nNewHeight);

                    long nActY = 0;
                    while( nActY < nNewHeight )
                    {
                        long nMapY = pLutY[ nActY ];

                        for( long nX = 0; nX < nNewWidth; nX++ )
                            pWriteAcc->SetPixel( nActY, nX, pReadAcc->GetPixel( nMapY , pLutX[ nX ] ) );

                        while( ( nActY < nNewHeight1 ) && ( pLutY[ nActY + 1 ] == nMapY ) )
                        {
                            memcpy( pWriteAcc->GetScanline( nActY + 1 ),
                                    pWriteAcc->GetScanline( nActY ), nScanlineSize );
                            nActY++;
                        }
                        nActY++;
                    }

                    bRet = true;
                }

                pWriteAcc.reset();
            }
            pReadAcc.reset();

            if( bRet )
                ImplAssignWithSize( aNewBmp );
        }
    }

    return bRet;
}

bool Bitmap::ImplScaleInterpolate( const double& rScaleX, const double& rScaleY )
{
    const Size aSizePix( GetSizePixel() );
    const long nNewWidth = FRound( aSizePix.Width() * rScaleX );
    const long nNewHeight = FRound( aSizePix.Height() * rScaleY );
    bool bRet = false;

    if( ( nNewWidth > 1 ) && ( nNewHeight > 1 ) )
    {
        ScopedReadAccess pReadAcc(*this);
        if( pReadAcc )
        {
            long nWidth = pReadAcc->Width();
            long nHeight = pReadAcc->Height();
            Bitmap aNewBmp( Size( nNewWidth, nHeight ), 24 );
            ScopedWriteAccess pWriteAcc(aNewBmp);

            if( pWriteAcc )
            {
                const long nNewWidth1 = nNewWidth - 1;
                const long nWidth1 = pReadAcc->Width() - 1;
                const double fRevScaleX = (double) nWidth1 / nNewWidth1;

                std::unique_ptr<long[]> pLutInt(new long[ nNewWidth ]);
                std::unique_ptr<long[]> pLutFrac(new long[ nNewWidth ]);

                for( long nX = 0, nTemp = nWidth - 2; nX < nNewWidth; nX++ )
                {
                    double fTemp = nX * fRevScaleX;
                    pLutInt[ nX ] = MinMax( (long) fTemp, 0, nTemp );
                    fTemp -= pLutInt[ nX ];
                    pLutFrac[ nX ] = (long) ( fTemp * 1024. );
                }

                for( long nY = 0; nY < nHeight; nY++ )
                {
                    if( 1 == nWidth )
                    {
                        BitmapColor aCol0;
                        if( pReadAcc->HasPalette() )
                        {
                            aCol0 = pReadAcc->GetPaletteColor( pReadAcc->GetPixelIndex( nY, 0 ) );
                        }
                        else
                        {
                            aCol0 = pReadAcc->GetPixel( nY, 0 );
                        }

                        for( long nX = 0; nX < nNewWidth; nX++ )
                        {
                            pWriteAcc->SetPixel( nY, nX, aCol0 );
                        }
                    }
                    else
                    {
                        for( long nX = 0; nX < nNewWidth; nX++ )
                        {
                            long nTemp = pLutInt[ nX ];

                            BitmapColor aCol0, aCol1;
                            if( pReadAcc->HasPalette() )
                            {
                                aCol0 = pReadAcc->GetPaletteColor( pReadAcc->GetPixelIndex( nY, nTemp++ ) );
                                aCol1 = pReadAcc->GetPaletteColor( pReadAcc->GetPixelIndex( nY, nTemp ) );
                            }
                            else
                            {
                                aCol0 = pReadAcc->GetPixel( nY, nTemp++ );
                                aCol1 = pReadAcc->GetPixel( nY, nTemp );
                            }

                            nTemp = pLutFrac[ nX ];

                            long lXR0 = aCol0.GetRed();
                            long lXG0 = aCol0.GetGreen();
                            long lXB0 = aCol0.GetBlue();
                            long lXR1 = aCol1.GetRed() - lXR0;
                            long lXG1 = aCol1.GetGreen() - lXG0;
                            long lXB1 = aCol1.GetBlue() - lXB0;

                            aCol0.SetRed( (sal_uInt8) ( ( lXR1 * nTemp + ( lXR0 << 10 ) ) >> 10 ) );
                            aCol0.SetGreen( (sal_uInt8) ( ( lXG1 * nTemp + ( lXG0 << 10 ) ) >> 10 ) );
                            aCol0.SetBlue( (sal_uInt8) ( ( lXB1 * nTemp + ( lXB0 << 10 ) ) >> 10 ) );

                            pWriteAcc->SetPixel( nY, nX, aCol0 );
                        }
                    }
                }

                bRet = true;
            }

            pReadAcc.reset();
            pWriteAcc.reset();

            if( bRet )
            {
                bRet = false;
                const Bitmap aOriginal(*this);
                *this = aNewBmp;
                aNewBmp = Bitmap( Size( nNewWidth, nNewHeight ), 24 );
                pReadAcc = ScopedReadAccess(*this);
                pWriteAcc = ScopedWriteAccess(aNewBmp);

                if( pReadAcc && pWriteAcc )
                {
                    const long nNewHeight1 = nNewHeight - 1;
                    const long nHeight1 = pReadAcc->Height() - 1;
                    const double fRevScaleY = (double) nHeight1 / nNewHeight1;

                    std::unique_ptr<long[]> pLutInt(new long[ nNewHeight ]);
                    std::unique_ptr<long[]> pLutFrac(new long[ nNewHeight ]);

                    for( long nY = 0, nTemp = nHeight - 2; nY < nNewHeight; nY++ )
                    {
                        double fTemp = nY * fRevScaleY;
                        pLutInt[ nY ] = MinMax( (long) fTemp, 0, nTemp );
                        fTemp -= pLutInt[ nY ];
                        pLutFrac[ nY ] = (long) ( fTemp * 1024. );
                    }

                    // after 1st step, bitmap *is* 24bit format (see above)
                    OSL_ENSURE(!pReadAcc->HasPalette(), "OOps, somehow ImplScaleInterpolate in-between format has palette, should not happen (!)");

                    for( long nX = 0; nX < nNewWidth; nX++ )
                    {
                        if( 1 == nHeight )
                        {
                            BitmapColor aCol0 = pReadAcc->GetPixel( 0, nX );

                            for( long nY = 0; nY < nNewHeight; nY++ )
                            {
                                pWriteAcc->SetPixel( nY, nX, aCol0 );
                            }
                        }
                        else
                        {
                            for( long nY = 0; nY < nNewHeight; nY++ )
                            {
                                long nTemp = pLutInt[ nY ];

                                BitmapColor aCol0 = pReadAcc->GetPixel( nTemp++, nX );
                                BitmapColor aCol1 = pReadAcc->GetPixel( nTemp, nX );

                                nTemp = pLutFrac[ nY ];

                                long lXR0 = aCol0.GetRed();
                                long lXG0 = aCol0.GetGreen();
                                long lXB0 = aCol0.GetBlue();
                                long lXR1 = aCol1.GetRed() - lXR0;
                                long lXG1 = aCol1.GetGreen() - lXG0;
                                long lXB1 = aCol1.GetBlue() - lXB0;

                                aCol0.SetRed( (sal_uInt8) ( ( lXR1 * nTemp + ( lXR0 << 10 ) ) >> 10 ) );
                                aCol0.SetGreen( (sal_uInt8) ( ( lXG1 * nTemp + ( lXG0 << 10 ) ) >> 10 ) );
                                aCol0.SetBlue( (sal_uInt8) ( ( lXB1 * nTemp + ( lXB0 << 10 ) ) >> 10 ) );

                                pWriteAcc->SetPixel( nY, nX, aCol0 );
                            }
                        }
                    }

                    bRet = true;
                }

                pReadAcc.reset();
                pWriteAcc.reset();

                if( bRet )
                {
                    aOriginal.ImplAdaptBitCount(aNewBmp);
                    *this = aNewBmp;
                }
            }
        }
    }

    if( !bRet )
    {
        bRet = ImplScaleFast( rScaleX, rScaleY );
    }

    return bRet;
}

bool Bitmap::Dither( BmpDitherFlags nDitherFlags )
{
    bool bRet = false;

    const Size aSizePix( GetSizePixel() );

    if( aSizePix.Width() == 1 || aSizePix.Height() == 1 )
        bRet = true;
    else if( nDitherFlags & BmpDitherFlags::Matrix )
        bRet = ImplDitherMatrix();
    else if( nDitherFlags & BmpDitherFlags::Floyd )
        bRet = ImplDitherFloyd();
    else if( ( nDitherFlags & BmpDitherFlags::Floyd16 ) && ( GetBitCount() == 24 ) )
        bRet = ImplDitherFloyd16();

    return bRet;
}

bool Bitmap::ImplDitherMatrix()
{
    ScopedReadAccess pReadAcc(*this);
    Bitmap aNewBmp( GetSizePixel(), 8 );
    ScopedWriteAccess pWriteAcc(aNewBmp);
    bool bRet = false;

    if( pReadAcc && pWriteAcc )
    {
        const sal_uLong nWidth = pReadAcc->Width();
        const sal_uLong nHeight = pReadAcc->Height();
        BitmapColor aIndex( (sal_uInt8) 0 );

        if( pReadAcc->HasPalette() )
        {
            for( sal_uLong nY = 0UL; nY < nHeight; nY++ )
            {
                for( sal_uLong nX = 0UL, nModY = ( nY & 0x0FUL ) << 4UL; nX < nWidth; nX++ )
                {
                    const BitmapColor aCol( pReadAcc->GetPaletteColor( pReadAcc->GetPixelIndex( nY, nX ) ) );
                    const sal_uLong nD = nVCLDitherLut[ nModY + ( nX & 0x0FUL ) ];
                    const sal_uLong nR = ( nVCLLut[ aCol.GetRed() ] + nD ) >> 16UL;
                    const sal_uLong nG = ( nVCLLut[ aCol.GetGreen() ] + nD ) >> 16UL;
                    const sal_uLong nB = ( nVCLLut[ aCol.GetBlue() ] + nD ) >> 16UL;

                    aIndex.SetIndex( (sal_uInt8) ( nVCLRLut[ nR ] + nVCLGLut[ nG ] + nVCLBLut[ nB ] ) );
                    pWriteAcc->SetPixel( nY, nX, aIndex );
                }
            }
        }
        else
        {
            for( sal_uLong nY = 0UL; nY < nHeight; nY++ )
            {
                for( sal_uLong nX = 0UL, nModY = ( nY & 0x0FUL ) << 4UL; nX < nWidth; nX++ )
                {
                    const BitmapColor aCol( pReadAcc->GetPixel( nY, nX ) );
                    const sal_uLong nD = nVCLDitherLut[ nModY + ( nX & 0x0FUL ) ];
                    const sal_uLong nR = ( nVCLLut[ aCol.GetRed() ] + nD ) >> 16UL;
                    const sal_uLong nG = ( nVCLLut[ aCol.GetGreen() ] + nD ) >> 16UL;
                    const sal_uLong nB = ( nVCLLut[ aCol.GetBlue() ] + nD ) >> 16UL;

                    aIndex.SetIndex( (sal_uInt8) ( nVCLRLut[ nR ] + nVCLGLut[ nG ] + nVCLBLut[ nB ] ) );
                    pWriteAcc->SetPixel( nY, nX, aIndex );
                }
            }
        }

        bRet = true;
    }

    pReadAcc.reset();
    pWriteAcc.reset();

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

bool Bitmap::ImplDitherFloyd()
{
    const Size aSize( GetSizePixel() );
    bool bRet = false;

    if( ( aSize.Width() > 3 ) && ( aSize.Height() > 2 ) )
    {
        ScopedReadAccess pReadAcc(*this);
        Bitmap aNewBmp( GetSizePixel(), 8 );
        ScopedWriteAccess pWriteAcc(aNewBmp);

        if( pReadAcc && pWriteAcc )
        {
            BitmapColor aColor;
            long nWidth = pReadAcc->Width();
            long nWidth1 = nWidth - 1;
            long nHeight = pReadAcc->Height();
            long nX;
            long nW = nWidth * 3;
            long nW2 = nW - 3;
            long nRErr, nGErr, nBErr;
            long nRC, nGC, nBC;
            std::unique_ptr<long[]> p1(new long[ nW ]);
            std::unique_ptr<long[]> p2(new long[ nW ]);
            long* p1T = p1.get();
            long* p2T = p2.get();
            long* pTmp;
            bool bPal = pReadAcc->HasPalette();

            pTmp = p2T;

            if( bPal )
            {
                for( long nZ = 0; nZ < nWidth; nZ++ )
                {
                    aColor = pReadAcc->GetPaletteColor( pReadAcc->GetPixelIndex( 0, nZ ) );

                    *pTmp++ = (long) aColor.GetBlue() << 12;
                    *pTmp++ = (long) aColor.GetGreen() << 12;
                    *pTmp++ = (long) aColor.GetRed() << 12;
                }
            }
            else
            {
                for( long nZ = 0; nZ < nWidth; nZ++ )
                {
                    aColor = pReadAcc->GetPixel( 0, nZ );

                    *pTmp++ = (long) aColor.GetBlue() << 12;
                    *pTmp++ = (long) aColor.GetGreen() << 12;
                    *pTmp++ = (long) aColor.GetRed() << 12;
                }
            }

            for( long nY = 1, nYAcc = 0; nY <= nHeight; nY++, nYAcc++ )
            {
                pTmp = p1T;
                p1T = p2T;
                p2T = pTmp;

                if( nY < nHeight )
                {
                    if( bPal )
                    {
                        for( long nZ = 0; nZ < nWidth; nZ++ )
                        {
                            aColor = pReadAcc->GetPaletteColor( pReadAcc->GetPixelIndex( nY, nZ ) );

                            *pTmp++ = (long) aColor.GetBlue() << 12;
                            *pTmp++ = (long) aColor.GetGreen() << 12;
                            *pTmp++ = (long) aColor.GetRed() << 12;
                        }
                    }
                    else
                    {
                        for( long nZ = 0; nZ < nWidth; nZ++ )
                        {
                            aColor = pReadAcc->GetPixel( nY, nZ );

                            *pTmp++ = (long) aColor.GetBlue() << 12;
                            *pTmp++ = (long) aColor.GetGreen() << 12;
                            *pTmp++ = (long) aColor.GetRed() << 12;
                        }
                    }
                }

                // Examine first Pixel separately
                nX = 0;
                long nTemp;
                CALC_ERRORS;
                CALC_TABLES7;
                nX -= 5;
                CALC_TABLES5;
                pWriteAcc->SetPixelIndex( nYAcc, 0, static_cast<sal_uInt8>(nVCLBLut[ nBC ] + nVCLGLut[nGC ] + nVCLRLut[nRC ]) );

                // Get middle Pixels using a loop
                long nXAcc;
                for ( nX = 3, nXAcc = 1; nX < nW2; nXAcc++ )
                {
                    CALC_ERRORS;
                    CALC_TABLES7;
                    nX -= 8;
                    CALC_TABLES3;
                    CALC_TABLES5;
                    pWriteAcc->SetPixelIndex( nYAcc, nXAcc, static_cast<sal_uInt8>(nVCLBLut[ nBC ] + nVCLGLut[nGC ] + nVCLRLut[nRC ]) );
                }

                // Treat last Pixel separately
                CALC_ERRORS;
                nX -= 5;
                CALC_TABLES3;
                CALC_TABLES5;
                pWriteAcc->SetPixelIndex( nYAcc, nWidth1, static_cast<sal_uInt8>(nVCLBLut[ nBC ] + nVCLGLut[nGC ] + nVCLRLut[nRC ]) );
            }

            bRet = true;
        }

        pReadAcc.reset();
        pWriteAcc.reset();

        if( bRet )
        {
            const MapMode aMap( maPrefMapMode );
            const Size aPrefSize( maPrefSize );

            *this = aNewBmp;

            maPrefMapMode = aMap;
            maPrefSize = aPrefSize;
        }
    }

    return bRet;
}

bool Bitmap::ImplDitherFloyd16()
{
    ScopedReadAccess pReadAcc(*this);
    Bitmap aNewBmp( GetSizePixel(), 24 );
    ScopedWriteAccess pWriteAcc(aNewBmp);
    bool bRet = false;

    if( pReadAcc && pWriteAcc )
    {
        const long nWidth = pWriteAcc->Width();
        const long nWidth1 = nWidth - 1;
        const long nHeight = pWriteAcc->Height();
        BitmapColor aColor;
        BitmapColor aBestCol;
        ImpErrorQuad aErrQuad;
        std::unique_ptr<ImpErrorQuad[]> pErrQuad1(new ImpErrorQuad[ nWidth ]);
        std::unique_ptr<ImpErrorQuad[]> pErrQuad2(new ImpErrorQuad[ nWidth ]);
        ImpErrorQuad* pQLine1 = pErrQuad1.get();
        ImpErrorQuad* pQLine2 = nullptr;
        long nYTmp = 0;
        bool bQ1 = true;

        for( long nY = 0; nY < std::min( nHeight, 2L ); nY++, nYTmp++ )
        {
            pQLine2 = !nY ? pErrQuad1.get() : pErrQuad2.get();
            for( long nX = 0; nX < nWidth; nX++ )
                pQLine2[ nX ] = pReadAcc->GetPixel( nYTmp, nX );
        }

        for( long nY = 0; nY < nHeight; nY++, nYTmp++ )
        {
            // First RowPixel
            aBestCol = pQLine1[ 0 ].ImplGetColor();
            aBestCol.SetRed( ( aBestCol.GetRed() & 248 ) | 7 );
            aBestCol.SetGreen( ( aBestCol.GetGreen() & 248 ) | 7 );
            aBestCol.SetBlue( ( aBestCol.GetBlue() & 248 ) | 7 );
            pWriteAcc->SetPixel( nY, 0, aBestCol );

            long nX;
            for( nX = 1; nX < nWidth1; nX++ )
            {
                aColor = pQLine1[ nX ].ImplGetColor();
                aBestCol.SetRed( ( aColor.GetRed() & 248 ) | 7 );
                aBestCol.SetGreen( ( aColor.GetGreen() & 248 ) | 7 );
                aBestCol.SetBlue( ( aColor.GetBlue() & 248 ) | 7 );
                aErrQuad = ( ImpErrorQuad( aColor ) -= aBestCol );
                pQLine1[ ++nX ].ImplAddColorError7( aErrQuad );
                pQLine2[ nX-- ].ImplAddColorError1( aErrQuad );
                pQLine2[ nX-- ].ImplAddColorError5( aErrQuad );
                pQLine2[ nX++ ].ImplAddColorError3( aErrQuad );
                pWriteAcc->SetPixel( nY, nX, aBestCol );
            }

            // Last RowPixel
            aBestCol = pQLine1[ nWidth1 ].ImplGetColor();
            aBestCol.SetRed( ( aBestCol.GetRed() & 248 ) | 7 );
            aBestCol.SetGreen( ( aBestCol.GetGreen() & 248 ) | 7 );
            aBestCol.SetBlue( ( aBestCol.GetBlue() & 248 ) | 7 );
            pWriteAcc->SetPixel( nY, nX, aBestCol );

            // Refill/copy row buffer
            pQLine1 = pQLine2;
            bQ1 = !bQ1;
            pQLine2 = bQ1 ? pErrQuad2.get() : pErrQuad1.get();

            if( nYTmp < nHeight )
                for( nX = 0; nX < nWidth; nX++ )
                    pQLine2[ nX ] = pReadAcc->GetPixel( nYTmp, nX );
        }

        bRet = true;
    }

    pReadAcc.reset();
    pWriteAcc.reset();

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

bool Bitmap::ReduceColors( sal_uInt16 nColorCount, BmpReduce eReduce )
{
    bool bRet;

    if( GetColorCount() <= (sal_uLong) nColorCount )
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
    const sal_uInt16 nColCount = std::min( nColorCount, (sal_uInt16) 256 );
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
        ScopedWriteAccess pWAcc(aNewBmp);

        if( pWAcc )
        {
            const long nWidth = pRAcc->Width();
            const long nHeight = pRAcc->Height();

            if( pRAcc->HasPalette() )
            {
                for( long nY = 0; nY < nHeight; nY++ )
                    for( long nX =0; nX < nWidth; nX++ )
                        pWAcc->SetPixelIndex( nY, nX, static_cast<sal_uInt8>(aOct.GetBestPaletteIndex( pRAcc->GetPaletteColor( pRAcc->GetPixelIndex( nY, nX ) ))) );
            }
            else
            {
                for( long nY = 0; nY < nHeight; nY++ )
                    for( long nX =0; nX < nWidth; nX++ )
                        pWAcc->SetPixelIndex( nY, nX, static_cast<sal_uInt8>(aOct.GetBestPaletteIndex( pRAcc->GetPixel( nY, nX ) )) );
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

extern "C" int SAL_CALL ImplPopularCmpFnc( const void* p1, const void* p2 )
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
                for( long nX = 0; nX < nWidth; nX++ )
                {
                    const BitmapColor& rCol = pRAcc->GetPaletteColor( pRAcc->GetPixelIndex( nY, nX ) );
                    pCountTable[ ( ( ( (sal_uInt32) rCol.GetRed() ) >> nRightShiftBits ) << nLeftShiftBits2 ) |
                                 ( ( ( (sal_uInt32) rCol.GetGreen() ) >> nRightShiftBits ) << nLeftShiftBits1 ) |
                                 ( ( (sal_uInt32) rCol.GetBlue() ) >> nRightShiftBits ) ].mnCount++;
                }
            }
        }
        else
        {
            for( long nY = 0; nY < nHeight; nY++ )
            {
                for( long nX = 0; nX < nWidth; nX++ )
                {
                    const BitmapColor aCol( pRAcc->GetPixel( nY, nX ) );
                    pCountTable[ ( ( ( (sal_uInt32) aCol.GetRed() ) >> nRightShiftBits ) << nLeftShiftBits2 ) |
                                 ( ( ( (sal_uInt32) aCol.GetGreen() ) >> nRightShiftBits ) << nLeftShiftBits1 ) |
                                 ( ( (sal_uInt32) aCol.GetBlue() ) >> nRightShiftBits ) ].mnCount++;
                }
            }
        }

        BitmapPalette aNewPal( nColCount );

        qsort( pCountTable.get(), nTotalColors, sizeof( PopularColorCount ), ImplPopularCmpFnc );

        for( sal_uInt16 n = 0; n < nColCount; n++ )
        {
            const PopularColorCount& rPop = pCountTable[ n ];
            aNewPal[ n ] = BitmapColor( (sal_uInt8) ( ( rPop.mnIndex >> nLeftShiftBits2 ) << nRightShiftBits ),
                                        (sal_uInt8) ( ( ( rPop.mnIndex >> nLeftShiftBits1 ) & ( nColorsPerComponent - 1 ) ) << nRightShiftBits ),
                                        (sal_uInt8) ( ( rPop.mnIndex & ( nColorsPerComponent - 1 ) ) << nRightShiftBits ) );
        }

        Bitmap aNewBmp( GetSizePixel(), nBitCount, &aNewPal );
        ScopedWriteAccess pWAcc(aNewBmp);

        if( pWAcc )
        {
            BitmapColor aDstCol( (sal_uInt8) 0 );
            std::unique_ptr<sal_uInt8[]> pIndexMap(new sal_uInt8[ nTotalColors ]);

            for( long nR = 0, nIndex = 0; nR < 256; nR += nColorOffset )
                for( long nG = 0; nG < 256; nG += nColorOffset )
                    for( long nB = 0; nB < 256; nB += nColorOffset )
                        pIndexMap[ nIndex++ ] = (sal_uInt8) aNewPal.GetBestIndex( BitmapColor( (sal_uInt8) nR, (sal_uInt8) nG, (sal_uInt8) nB ) );

            if( pRAcc->HasPalette() )
            {
                for( long nY = 0; nY < nHeight; nY++ )
                {
                    for( long nX = 0; nX < nWidth; nX++ )
                    {
                        const BitmapColor& rCol = pRAcc->GetPaletteColor( pRAcc->GetPixelIndex( nY, nX ) );
                        aDstCol.SetIndex( pIndexMap[ ( ( ( (sal_uInt32) rCol.GetRed() ) >> nRightShiftBits ) << nLeftShiftBits2 ) |
                                                     ( ( ( (sal_uInt32) rCol.GetGreen() ) >> nRightShiftBits ) << nLeftShiftBits1 ) |
                                                     ( ( (sal_uInt32) rCol.GetBlue() ) >> nRightShiftBits ) ] );
                        pWAcc->SetPixel( nY, nX, aDstCol );
                    }
                }
            }
            else
            {
                for( long nY = 0; nY < nHeight; nY++ )
                {
                    for( long nX = 0; nX < nWidth; nX++ )
                    {
                        const BitmapColor aCol( pRAcc->GetPixel( nY, nX ) );
                        aDstCol.SetIndex( pIndexMap[ ( ( ( (sal_uInt32) aCol.GetRed() ) >> nRightShiftBits ) << nLeftShiftBits2 ) |
                                                     ( ( ( (sal_uInt32) aCol.GetGreen() ) >> nRightShiftBits ) << nLeftShiftBits1 ) |
                                                     ( ( (sal_uInt32) aCol.GetBlue() ) >> nRightShiftBits ) ] );
                        pWAcc->SetPixel( nY, nX, aDstCol );
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
        ScopedWriteAccess pWAcc(aNewBmp);

        if( pWAcc )
        {
            const sal_uLong nSize = 32768UL * sizeof( sal_uLong );
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
                    for( long nX = 0; nX < nWidth; nX++ )
                    {
                        const BitmapColor& rCol = pRAcc->GetPaletteColor( pRAcc->GetPixelIndex( nY, nX ) );
                        pColBuf[ RGB15( rCol.GetRed() >> 3, rCol.GetGreen() >> 3, rCol.GetBlue() >> 3 ) ]++;
                    }
                }
            }
            else
            {
                for( long nY = 0; nY < nHeight; nY++ )
                {
                    for( long nX = 0; nX < nWidth; nX++ )
                    {
                        const BitmapColor aCol( pRAcc->GetPixel( nY, nX ) );
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
                for( long nX = 0; nX < nWidth; nX++ )
                    pWAcc->SetPixelIndex( nY, nX, static_cast<sal_uInt8>( aMap.GetBestPaletteIndex( pRAcc->GetColor( nY, nX ) )) );

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
            aCol.SetRed( (sal_uInt8) ( nR1 << 3 ) );
            aCol.SetGreen( (sal_uInt8) ( nG1 << 3 ) );
            aCol.SetBlue( (sal_uInt8) ( nB1 << 3 ) );
            rPal[ (sal_uInt16) rIndex++ ] = aCol;
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

            aCol.SetRed( (sal_uInt8) ( ( nRSum / nPixels ) << 3 ) );
            aCol.SetGreen( (sal_uInt8) ( ( nGSum / nPixels ) << 3 ) );
            aCol.SetBlue( (sal_uInt8) ( ( nBSum / nPixels ) << 3 ) );
            rPal[ (sal_uInt16) rIndex++ ] = aCol;
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

bool Bitmap::Vectorize( GDIMetaFile& rMtf, sal_uInt8 cReduce, const Link<long,void>* pProgress )
{
    return ImplVectorizer::ImplVectorize( *this, rMtf, cReduce, pProgress );
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
        ScopedWriteAccess pAcc(*this);

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
                    cMapR[ nX ] = (sal_uInt8) MinMax( FRound( nX * fM + fROff ), 0, 255 );
                    cMapG[ nX ] = (sal_uInt8) MinMax( FRound( nX * fM + fGOff ), 0, 255 );
                    cMapB[ nX ] = (sal_uInt8) MinMax( FRound( nX * fM + fBOff ), 0, 255 );
                }
                else
                {
                    // LO simply uses (in a somewhat optimized form) "newcolor = (oldcolor-128)*contrast+brightness+128"
                    // as the formula, i.e. contrast first, brightness afterwards. MSOffice, for whatever weird reason,
                    // use neither first, but apparently it applies half of brightness before contrast and half afterwards.
                    cMapR[ nX ] = (sal_uInt8) MinMax( FRound( (nX+fROff/2-128) * fM + 128 + fROff/2 ), 0, 255 );
                    cMapG[ nX ] = (sal_uInt8) MinMax( FRound( (nX+fGOff/2-128) * fM + 128 + fGOff/2 ), 0, 255 );
                    cMapB[ nX ] = (sal_uInt8) MinMax( FRound( (nX+fBOff/2-128) * fM + 128 + fBOff/2 ), 0, 255 );
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
                    for( long nX = 0; nX < nW; nX++ )
                    {
                        aCol = pAcc->GetPixel( nY, nX );
                        aCol.SetRed( cMapR[ aCol.GetRed() ] );
                        aCol.SetGreen( cMapG[ aCol.GetGreen() ] );
                        aCol.SetBlue( cMapB[ aCol.GetBlue() ] );
                        pAcc->SetPixel( nY, nX, aCol );
                    }
                }
            }

            pAcc.reset();
            bRet = true;
        }
    }

    return bRet;
}

bool Bitmap::ImplConvolutionPass(Bitmap& aNewBitmap, BitmapReadAccess* pReadAcc, int aNumberOfContributions, double* pWeights, int* pPixels, int* pCount)
{
    if (!pReadAcc)
        return false;

    ScopedWriteAccess pWriteAcc(aNewBitmap);
    if (!pWriteAcc)
        return false;

    const int nHeight = GetSizePixel().Height();
    assert(GetSizePixel().Height() == aNewBitmap.GetSizePixel().Width());
    const int nWidth = GetSizePixel().Width();
    assert(GetSizePixel().Width() == aNewBitmap.GetSizePixel().Height());

    BitmapColor aColor;
    double aValueRed, aValueGreen, aValueBlue;
    double aSum, aWeight;
    int aBaseIndex, aIndex;

    for (int nSourceY = 0; nSourceY < nHeight; ++nSourceY)
    {
        for (int nSourceX = 0; nSourceX < nWidth; ++nSourceX)
        {
            aBaseIndex = nSourceX * aNumberOfContributions;
            aSum = aValueRed = aValueGreen = aValueBlue = 0.0;

            for (int j = 0; j < pCount[nSourceX]; ++j)
            {
                aIndex = aBaseIndex + j;
                aSum += aWeight = pWeights[ aIndex ];

                aColor = pReadAcc->GetColor(nSourceY, pPixels[aIndex]);

                aValueRed += aWeight * aColor.GetRed();
                aValueGreen += aWeight * aColor.GetGreen();
                aValueBlue += aWeight * aColor.GetBlue();
            }

            BitmapColor aResultColor(
                (sal_uInt8) MinMax( aValueRed / aSum, 0, 255 ),
                (sal_uInt8) MinMax( aValueGreen / aSum, 0, 255 ),
                (sal_uInt8) MinMax( aValueBlue / aSum, 0, 255 ) );

            int nDestX = nSourceY;
            int nDestY = nSourceX;

            pWriteAcc->SetPixel(nDestY, nDestX, aResultColor);
        }
    }
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
