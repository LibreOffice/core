/*************************************************************************
 *
 *  $RCSfile: bitmap3.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <stdlib.h>
#define _SV_BITMAP_CXX

#ifdef W31
#include <tools/svwin.h>
#endif
#include <tools/new.hxx>
#ifndef _SV_BMPACC_HXX
#include <bmpacc.hxx>
#endif
#ifndef _SV_IMPOCT_HXX
#include <impoct.hxx>
#endif
#ifndef _SV_OCTREE_HXX
#include <octree.hxx>
#endif
#ifndef _SV_IMPVECT_HXX
#include <impvect.hxx>
#endif
#ifndef _SV_BITMAPEX_HXX
#include <bitmapex.hxx>
#endif
#ifndef _SV_BITMAP_HXX
#include <bitmap.hxx>
#endif

// -----------
// - Defines -
// -----------

#define RGB15( _def_cR, _def_cG, _def_cB )  (((ULONG)(_def_cR)<<10UL)|((ULONG)(_def_cG)<<5UL)|(ULONG)(_def_cB))
#define GAMMA( _def_cVal, _def_InvGamma )   ((BYTE)MinMax(FRound(pow( _def_cVal/255.0,_def_InvGamma)*255.0),0L,255L))

#define CALC_ERRORS                                                             \
                        nTemp   = p1T[nX++] >> 12;                              \
                        nBErr = MinMax( nTemp, 0, 255 );                        \
                        nBErr = nBErr - FloydIndexMap[ nBC = FloydMap[nBErr] ]; \
                        nTemp   = p1T[nX++] >> 12;                              \
                        nGErr = MinMax( nTemp, 0, 255 );                        \
                        nGErr = nGErr - FloydIndexMap[ nGC = FloydMap[nGErr] ]; \
                        nTemp   = p1T[nX] >> 12;                                \
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

// -----------
// - Statics -
// -----------

ULONG nVCLRLut[ 6 ] = { 16, 17, 18, 19, 20, 21 };
ULONG nVCLGLut[ 6 ] = { 0, 6, 12, 18, 24, 30 };
ULONG nVCLBLut[ 6 ] = { 0, 36, 72, 108, 144, 180 };

// ------------------------------------------------------------------------

ULONG nVCLDitherLut[ 256 ] =
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

// ------------------------------------------------------------------------

ULONG nVCLLut[ 256 ] =
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

// ------------------------------------------------------------------------

long FloydMap[256] =
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

// ------------------------------------------------------------------------

long FloydError1[61] =
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

// ------------------------------------------------------------------------

long FloydError3[61] =
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

// ------------------------------------------------------------------------

long FloydError5[61] =
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

// ------------------------------------------------------------------------

long FloydError7[61] =
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

// ------------------------------------------------------------------------

long FloydIndexMap[6] =
{
    -30,  21, 72, 123, 174, 225
};

// --------------------------
// - ImplCreateDitherMatrix -
// --------------------------

void ImplCreateDitherMatrix( BYTE (*pDitherMatrix)[16][16] )
{
    double          fVal = 3.125;
    const double    fVal16 = fVal / 16.;
    long            i, j, k, l;
    USHORT          pMtx[ 16 ][ 16 ];
    USHORT          nMax = 0;
    static BYTE     pMagic[4][4] = { 0, 14,  3, 13,
                                    11,  5,  8,  6,
                                    12,  2, 15,  1,
                                    7,  9,  4, 10 };

    // MagicSquare aufbauen
    for ( i = 0; i < 4; i++ )
       for ( j = 0; j < 4; j++ )
           for ( k = 0; k < 4; k++ )
                for ( l = 0; l < 4; l++ )
                    nMax = Max ( pMtx[ (k<<2) + i][(l<<2 ) + j] =
                    (USHORT) ( 0.5 + pMagic[i][j]*fVal + pMagic[k][l]*fVal16 ), nMax );

    // auf Intervall [0;254] skalieren
    for ( i = 0, fVal = 254. / nMax; i < 16; i++ )
        for( j = 0; j < 16; j++ )
            (*pDitherMatrix)[i][j] = (BYTE) ( fVal * pMtx[i][j] );
}

// ----------
// - Bitmap -
// ----------

BOOL Bitmap::Convert( BmpConversion eConversion )
{
    const USHORT    nBitCount = GetBitCount();
    BOOL            bRet = FALSE;

    switch( eConversion )
    {
        case( BMP_CONVERSION_1BIT_THRESHOLD ):
            bRet = ImplMakeMono( 128 );
        break;

        case( BMP_CONVERSION_1BIT_MATRIX ):
            bRet = ImplMakeMonoDither();
        break;

        case( BMP_CONVERSION_4BIT_GREYS ):
            bRet = ImplMakeGreyscales( 16 );
        break;

        case( BMP_CONVERSION_4BIT_COLORS ):
        {
            if( nBitCount < 4 )
                bRet = ImplConvertUp( 4, NULL );
            else if( nBitCount > 4 )
                bRet = ImplConvertDown( 4, NULL );
            else
                bRet = TRUE;
        }
        break;

        case( BMP_CONVERSION_4BIT_TRANS ):
        {
            Color aTrans( BMP_COL_TRANS );

            if( nBitCount < 4 )
                bRet = ImplConvertUp( 4, &aTrans );
            else
                bRet = ImplConvertDown( 4, &aTrans );
        }
        break;

        case( BMP_CONVERSION_8BIT_GREYS ):
            bRet = ImplMakeGreyscales( 256 );
        break;

        case( BMP_CONVERSION_8BIT_COLORS ):
        {
            if( nBitCount < 8 )
                bRet = ImplConvertUp( 8 );
            else if( nBitCount > 8 )
                bRet = ImplConvertDown( 8 );
            else
                bRet = TRUE;
        }
        break;

        case( BMP_CONVERSION_8BIT_TRANS ):
        {
            Color aTrans( BMP_COL_TRANS );

            if( nBitCount < 8 )
                bRet = ImplConvertUp( 8, &aTrans );
            else
                bRet = ImplConvertDown( 8, &aTrans );
        }
        break;

        case( BMP_CONVERSION_24BIT ):
        {
            if( nBitCount < 24 )
                bRet = ImplConvertUp( 24, FALSE );
            else
                bRet = TRUE;
        }
        break;

        case( BMP_CONVERSION_GHOSTED ):
            bRet = ImplConvertGhosted();
        break;

        default:
            DBG_ERROR( "Bitmap::Convert(): Unsupported conversion" );
        break;
    }

    return bRet;
}

// ------------------------------------------------------------------------

BOOL Bitmap::ImplMakeMono( BYTE cThreshold )
{
    BitmapReadAccess*   pReadAcc = AcquireReadAccess();
    BOOL                bRet = FALSE;

    if( pReadAcc )
    {
        Bitmap              aNewBmp( GetSizePixel(), 1 );
        BitmapWriteAccess*  pWriteAcc = aNewBmp.AcquireWriteAccess();

        if( pWriteAcc )
        {
            const BitmapColor   aBlack( pWriteAcc->GetBestMatchingColor( Color( COL_BLACK ) ) );
            const BitmapColor   aWhite( pWriteAcc->GetBestMatchingColor( Color( COL_WHITE ) ) );
            const long          nWidth = pWriteAcc->Width();
            const long          nHeight = pWriteAcc->Height();

            if( pReadAcc->HasPalette() )
            {
                for( long nY = 0L; nY < nHeight; nY++ )
                {
                    for( long nX = 0L; nX < nWidth; nX++ )
                    {
                        if( pReadAcc->GetPaletteColor( pReadAcc->GetPixel( nY, nX ) ).GetLuminance() >=
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
                for( long nY = 0L; nY < nHeight; nY++ )
                {
                    for( long nX = 0L; nX < nWidth; nX++ )
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

            aNewBmp.ReleaseAccess( pWriteAcc );
            bRet = TRUE;
        }

        ReleaseAccess( pReadAcc );

        if( bRet )
        {
            const MapMode   aMap( maPrefMapMode );
            const Size      aSize( maPrefSize );

            *this = aNewBmp;

            maPrefMapMode = aMap;
            maPrefSize = aSize;
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

BOOL Bitmap::ImplMakeMonoDither()
{
    BitmapReadAccess*   pReadAcc = AcquireReadAccess();
    BOOL                bRet = FALSE;

    if( pReadAcc )
    {
        Bitmap              aNewBmp( GetSizePixel(), 1 );
        BitmapWriteAccess*  pWriteAcc = aNewBmp.AcquireWriteAccess();

        if( pWriteAcc )
        {
            const BitmapColor   aBlack( pWriteAcc->GetBestMatchingColor( Color( COL_BLACK ) ) );
            const BitmapColor   aWhite( pWriteAcc->GetBestMatchingColor( Color( COL_WHITE ) ) );
            const long          nWidth = pWriteAcc->Width();
            const long          nHeight = pWriteAcc->Height();
            BYTE                pDitherMatrix[ 16 ][ 16 ];

            ImplCreateDitherMatrix( &pDitherMatrix );

            if( pReadAcc->HasPalette() )
            {
                for( long nY = 0L; nY < nHeight; nY++ )
                {
                    for( long nX = 0L, nModY = nY % 16; nX < nWidth; nX++ )
                    {
                        if( pReadAcc->GetPaletteColor( pReadAcc->GetPixel( nY, nX ) ).GetLuminance() >
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
                for( long nY = 0L; nY < nHeight; nY++ )
                {
                    for( long nX = 0L, nModY = nY % 16; nX < nWidth; nX++ )
                    {
                        if( pReadAcc->GetPixel( nY, nX ).GetLuminance() >
                            pDitherMatrix[ nModY ][ nX % 16 ]  )
                        {
                            pWriteAcc->SetPixel( nY, nX, aWhite );
                        }
                        else
                            pWriteAcc->SetPixel( nY, nX, aBlack );
                    }
                }
            }

            aNewBmp.ReleaseAccess( pWriteAcc );
            bRet = TRUE;
        }

        ReleaseAccess( pReadAcc );

        if( bRet )
        {
            const MapMode   aMap( maPrefMapMode );
            const Size      aSize( maPrefSize );

            *this = aNewBmp;

            maPrefMapMode = aMap;
            maPrefSize = aSize;
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

BOOL Bitmap::ImplMakeGreyscales( USHORT nGreys )
{
    DBG_ASSERT( nGreys == 16 || nGreys == 256, "Only 16 or 256 greyscales are supported!" )

    BitmapReadAccess*   pReadAcc = AcquireReadAccess();
    BOOL                bRet = FALSE;

    if( pReadAcc )
    {
        const BitmapPalette&    rPal = GetGreyPalette( nGreys );
        ULONG                   nShift = ( ( nGreys == 16 ) ? 4UL : 0UL );
        BOOL                    bPalDiffers = !pReadAcc->HasPalette() || ( rPal.GetEntryCount() != pReadAcc->GetPaletteEntryCount() );

        if( !bPalDiffers )
            bPalDiffers = ( (BitmapPalette&) rPal != pReadAcc->GetPalette() );

        if( bPalDiffers )
        {
            Bitmap              aNewBmp( GetSizePixel(), ( nGreys == 16 ) ? 4 : 8, &rPal );
            BitmapWriteAccess*  pWriteAcc = aNewBmp.AcquireWriteAccess();

            if( pWriteAcc )
            {
                const long  nWidth = pWriteAcc->Width();
                const long  nHeight = pWriteAcc->Height();

                if( pReadAcc->HasPalette() )
                {
                    for( long nY = 0L; nY < nHeight; nY++ )
                    {
                        for( long nX = 0L; nX < nWidth; nX++ )
                        {
                            pWriteAcc->SetPixel( nY, nX,
                                (BYTE) ( pReadAcc->GetPaletteColor(
                                    pReadAcc->GetPixel( nY, nX ) ).GetLuminance() >> nShift ) );
                        }
                    }
                }
                else if( pReadAcc->GetScanlineFormat() == BMP_FORMAT_24BIT_TC_BGR &&
                         pWriteAcc->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL )
                {
                    nShift += 8;

                    for( long nY = 0L; nY < nHeight; nY++ )
                    {
                        Scanline pReadScan = pReadAcc->GetScanline( nY );
                        Scanline pWriteScan = pWriteAcc->GetScanline( nY );

                        for( long nX = 0L; nX < nWidth; nX++ )
                        {
                            const ULONG nB = *pReadScan++;
                            const ULONG nG = *pReadScan++;
                            const ULONG nR = *pReadScan++;

                            *pWriteScan++ = (BYTE) ( ( nB * 28UL + nG * 151UL + nR * 77UL ) >> nShift );
                        }
                    }
                }
                else if( pReadAcc->GetScanlineFormat() == BMP_FORMAT_24BIT_TC_RGB &&
                         pWriteAcc->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL )
                {
                    nShift += 8;

                    for( long nY = 0L; nY < nHeight; nY++ )
                    {
                        Scanline pReadScan = pReadAcc->GetScanline( nY );
                        Scanline pWriteScan = pWriteAcc->GetScanline( nY );

                        for( long nX = 0L; nX < nWidth; nX++ )
                        {
                            const ULONG nR = *pReadScan++;
                            const ULONG nG = *pReadScan++;
                            const ULONG nB = *pReadScan++;

                            *pWriteScan++ = (BYTE) ( ( nB * 28UL + nG * 151UL + nR * 77UL ) >> nShift );
                        }
                    }
                }
                else
                {
                    for( long nY = 0L; nY < nHeight; nY++ )
                        for( long nX = 0L; nX < nWidth; nX++ )
                            pWriteAcc->SetPixel( nY, nX, ( pReadAcc->GetPixel( nY, nX ) ).GetLuminance() >> nShift );
                }

                aNewBmp.ReleaseAccess( pWriteAcc );
                bRet = TRUE;
            }

            ReleaseAccess( pReadAcc );

            if( bRet )
            {
                const MapMode   aMap( maPrefMapMode );
                const Size      aSize( maPrefSize );

                *this = aNewBmp;

                maPrefMapMode = aMap;
                maPrefSize = aSize;
            }
        }
        else
        {
            ReleaseAccess( pReadAcc );
            bRet = TRUE;
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

BOOL Bitmap::ImplConvertUp( USHORT nBitCount, Color* pExtColor )
{
    DBG_ASSERT( nBitCount > GetBitCount(), "New BitCount must be greater!" );

    BitmapReadAccess*   pReadAcc = AcquireReadAccess();
    BOOL                bRet = FALSE;

    if( pReadAcc )
    {
        BitmapPalette       aPal;
        Bitmap              aNewBmp( GetSizePixel(), nBitCount, &aPal );
        BitmapWriteAccess*  pWriteAcc = aNewBmp.AcquireWriteAccess();

        if( pWriteAcc )
        {
            const long  nWidth = pWriteAcc->Width();
            const long  nHeight = pWriteAcc->Height();

            if( pWriteAcc->HasPalette() )
            {
                const USHORT            nOldCount = 1 << GetBitCount();
                const BitmapPalette&    rOldPal = pReadAcc->GetPalette();

                aPal.SetEntryCount( 1 << nBitCount );

                for( USHORT i = 0; i < nOldCount; i++ )
                    aPal[ i ] = rOldPal[ i ];

                if( pExtColor )
                    aPal[ aPal.GetEntryCount() - 1 ] = *pExtColor;

                pWriteAcc->SetPalette( aPal );

                for( long nY = 0L; nY < nHeight; nY++ )
                    for( long nX = 0L; nX < nWidth; nX++ )
                        pWriteAcc->SetPixel( nY, nX, pReadAcc->GetPixel( nY, nX ) );
            }
            else
            {
                if( pReadAcc->HasPalette() )
                {
                    for( long nY = 0L; nY < nHeight; nY++ )
                        for( long nX = 0L; nX < nWidth; nX++ )
                            pWriteAcc->SetPixel( nY, nX, pReadAcc->GetPaletteColor( pReadAcc->GetPixel( nY, nX ) ) );
                }
                else
                {
                    for( long nY = 0L; nY < nHeight; nY++ )
                        for( long nX = 0L; nX < nWidth; nX++ )
                            pWriteAcc->SetPixel( nY, nX, pReadAcc->GetPixel( nY, nX ) );
                }
            }

            aNewBmp.ReleaseAccess( pWriteAcc );
            bRet = TRUE;
        }

        ReleaseAccess( pReadAcc );

        if( bRet )
        {
            const MapMode   aMap( maPrefMapMode );
            const Size      aSize( maPrefSize );

            *this = aNewBmp;

            maPrefMapMode = aMap;
            maPrefSize = aSize;
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

BOOL Bitmap::ImplConvertDown( USHORT nBitCount, Color* pExtColor )
{
    DBG_ASSERT( nBitCount <= GetBitCount(), "New BitCount must be lower ( or equal when pExtColor is set )!" );

    BitmapReadAccess*   pReadAcc = AcquireReadAccess();
    BOOL                bRet = FALSE;

    if( pReadAcc )
    {
        BitmapPalette       aPal;
        Bitmap              aNewBmp( GetSizePixel(), nBitCount, &aPal );
        BitmapWriteAccess*  pWriteAcc = aNewBmp.AcquireWriteAccess();

        if( pWriteAcc )
        {
            const USHORT    nCount = 1 << nBitCount;
            const long      nWidth = pWriteAcc->Width();
            const long      nWidth1 = nWidth - 1L;
            const long      nHeight = pWriteAcc->Height();
            Octree          aOctree( *pReadAcc, pExtColor ? ( nCount - 1 ) : nCount );
            InverseColorMap aColorMap( aPal = aOctree.GetPalette() );
            BitmapColor     aColor;
            ImpErrorQuad    aErrQuad;
            ImpErrorQuad*   pErrQuad1 = new ImpErrorQuad[ nWidth ];
            ImpErrorQuad*   pErrQuad2 = new ImpErrorQuad[ nWidth ];
            ImpErrorQuad*   pQLine1 = pErrQuad1;
            ImpErrorQuad*   pQLine2;
            long            nX, nY;
            long            nYTmp = 0L;
            BYTE            cIndex;
            BOOL            bQ1 = TRUE;

            if( pExtColor )
            {
                aPal.SetEntryCount( aPal.GetEntryCount() + 1 );
                aPal[ aPal.GetEntryCount() - 1 ] = *pExtColor;
            }

            // set Black/White always, if we have enough space
            if( aPal.GetEntryCount() < ( nCount - 1 ) )
            {
                aPal.SetEntryCount( aPal.GetEntryCount() + 2 );
                aPal[ aPal.GetEntryCount() - 2 ] = Color( COL_BLACK );
                aPal[ aPal.GetEntryCount() - 1 ] = Color( COL_WHITE );
            }

            pWriteAcc->SetPalette( aPal );

            for( nY = 0L; nY < Min( nHeight, 2L ); nY++, nYTmp++ )
            {
                for( nX = 0L, pQLine2 = !nY ? pErrQuad1 : pErrQuad2; nX < nWidth; nX++ )
                {
                    if( pReadAcc->HasPalette() )
                        pQLine2[ nX ] = pReadAcc->GetPaletteColor( pReadAcc->GetPixel( nYTmp, nX ) );
                    else
                        pQLine2[ nX ] = pReadAcc->GetPixel( nYTmp, nX );
                }
            }

            for( nY = 0L; nY < nHeight; nY++, nYTmp++ )
            {
                // erstes ZeilenPixel
                cIndex = (BYTE) aColorMap.GetBestPaletteIndex( pQLine1[ 0 ].ImplGetColor() );
                pWriteAcc->SetPixel( nY, 0, cIndex );

                for( nX = 1L; nX < nWidth1; nX++ )
                {
                    cIndex = (BYTE) aColorMap.GetBestPaletteIndex( aColor = pQLine1[ nX ].ImplGetColor() );
                    aErrQuad = ( ImpErrorQuad( aColor ) -= pWriteAcc->GetPaletteColor( cIndex ) );
                    pQLine1[ ++nX ].ImplAddColorError7( aErrQuad );
                    pQLine2[ nX-- ].ImplAddColorError1( aErrQuad );
                    pQLine2[ nX-- ].ImplAddColorError5( aErrQuad );
                    pQLine2[ nX++ ].ImplAddColorError3( aErrQuad );
                    pWriteAcc->SetPixel( nY, nX, cIndex );
                }

                // letztes ZeilenPixel
                cIndex = (BYTE) aColorMap.GetBestPaletteIndex( pQLine1[ nWidth1 ].ImplGetColor() );
                pWriteAcc->SetPixel( nY, nX, cIndex );

                // Zeilenpuffer neu fuellen/kopieren
                pQLine1 = pQLine2;
                pQLine2 = ( bQ1 = !bQ1 ) ? pErrQuad2 : pErrQuad1;

                if( nYTmp < nHeight )
                {
                    for( nX = 0L; nX < nWidth; nX++ )
                    {
                        if( pReadAcc->HasPalette() )
                                pQLine2[ nX ] = pReadAcc->GetPaletteColor( pReadAcc->GetPixel( nYTmp, nX ) );
                        else
                            pQLine2[ nX ] = pReadAcc->GetPixel( nYTmp, nX );
                    }
                }
            }

            // Zeilenpuffer zerstoeren
            delete[] pErrQuad1;
            delete[] pErrQuad2;

            aNewBmp.ReleaseAccess( pWriteAcc );
            bRet = TRUE;
        }

        ReleaseAccess( pReadAcc );

        if( bRet )
        {
            const MapMode   aMap( maPrefMapMode );
            const Size      aSize( maPrefSize );

            *this = aNewBmp;

            maPrefMapMode = aMap;
            maPrefSize = aSize;
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

BOOL Bitmap::ImplConvertGhosted()
{
    Bitmap              aNewBmp;
    BitmapReadAccess*   pR = AcquireReadAccess();
    BOOL                bRet = FALSE;

    if( pR )
    {
        if( pR->HasPalette() )
        {
            BitmapPalette aNewPal( pR->GetPaletteEntryCount() );

            for( long i = 0, nCount = aNewPal.GetEntryCount(); i < nCount; i++ )
            {
                const BitmapColor& rOld = pR->GetPaletteColor( (USHORT) i );
                aNewPal[ (USHORT) i ] = BitmapColor( ( rOld.GetRed() >> 1 ) | 0x80,
                                                     ( rOld.GetGreen() >> 1 ) | 0x80,
                                                     ( rOld.GetBlue() >> 1 ) | 0x80 );
            }

            aNewBmp = Bitmap( GetSizePixel(), GetBitCount(), &aNewPal );
            BitmapWriteAccess* pW = aNewBmp.AcquireWriteAccess();

            if( pW )
            {
                pW->CopyBuffer( *pR );
                aNewBmp.ReleaseAccess( pW );
                bRet = TRUE;
            }
        }
        else
        {
            aNewBmp = Bitmap( GetSizePixel(), 24 );

            BitmapWriteAccess* pW = aNewBmp.AcquireWriteAccess();

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

                aNewBmp.ReleaseAccess( pW );
                bRet = TRUE;
            }
        }

        ReleaseAccess( pR );
    }

    if( bRet )
    {
        const MapMode   aMap( maPrefMapMode );
        const Size      aSize( maPrefSize );

        *this = aNewBmp;

        maPrefMapMode = aMap;
        maPrefSize = aSize;
    }

    return bRet;
}

// ------------------------------------------------------------------------

BOOL Bitmap::Scale( const double& rScaleX, const double& rScaleY, ULONG nScaleFlag )
{
    BOOL bRet;

    if( ( rScaleX != 1.0 ) || ( rScaleY != 1.0 ) )
    {
        if( BMP_SCALE_FAST == nScaleFlag )
            bRet = ImplScaleFast( rScaleX, rScaleY );
        else if( BMP_SCALE_INTERPOLATE == nScaleFlag )
            bRet = ImplScaleInterpolate( rScaleX, rScaleY );
        else
            bRet = FALSE;
    }
    else
        bRet = TRUE;

    return bRet;
}

// ------------------------------------------------------------------------

BOOL Bitmap::Scale( const Size& rNewSize, ULONG nScaleFlag )
{
    const Size  aSize( GetSizePixel() );
    BOOL        bRet;

    if( aSize.Width() && aSize.Height() )
    {
        bRet = Scale( (double) rNewSize.Width() / aSize.Width(),
                      (double) rNewSize.Height() / aSize.Height(),
                      nScaleFlag );
    }
    else
        bRet = TRUE;

    return bRet;
}

// ------------------------------------------------------------------------

BOOL Bitmap::ImplScaleFast( const double& rScaleX, const double& rScaleY )
{
    const Size  aSizePix( GetSizePixel() );
    const long  nNewWidth = FRound( aSizePix.Width() * rScaleX );
    const long  nNewHeight = FRound( aSizePix.Height() * rScaleY );
    BOOL        bRet = FALSE;

    if( nNewWidth && nNewHeight )
    {
        BitmapReadAccess*   pReadAcc = AcquireReadAccess();
        Bitmap              aNewBmp( Size( nNewWidth, nNewHeight ), GetBitCount(), &pReadAcc->GetPalette() );
        BitmapWriteAccess*  pWriteAcc = aNewBmp.AcquireWriteAccess();

        if( pReadAcc && pWriteAcc )
        {
            const long  nScanlineSize = pWriteAcc->GetScanlineSize();
            const long  nNewWidth1 = nNewWidth - 1L;
            const long  nNewHeight1 = nNewHeight - 1L;
            const long  nWidth1 = pReadAcc->Width() - 1L;
            const long  nHeight1 = pReadAcc->Height() - 1L;
            long*       pLutX = new long[ nNewWidth ];
            long*       pLutY = new long[ nNewHeight ];
            long        nX, nY, nMapY, nActY = 0L;

            if( nNewWidth1 && nNewHeight1 )
            {
                for( nX = 0L; nX < nNewWidth; nX++ )
                    pLutX[ nX ] = nX * nWidth1 / nNewWidth1;

                for( nY = 0L; nY < nNewHeight; nY++ )
                    pLutY[ nY ] = nY * nHeight1 / nNewHeight1;

                while( nActY < nNewHeight )
                {
                    nMapY = pLutY[ nActY ];

                    for( nX = 0L; nX < nNewWidth; nX++ )
                        pWriteAcc->SetPixel( nActY, nX, pReadAcc->GetPixel( nMapY , pLutX[ nX ] ) );

                    while( ( nActY < nNewHeight1 ) && ( pLutY[ nActY + 1 ] == nMapY ) )
                    {
                        HMEMCPY( pWriteAcc->GetScanline( nActY + 1L ),
                                 pWriteAcc->GetScanline( nActY ), nScanlineSize );
                        nActY++;
                    }

                    nActY++;
                }

                bRet = TRUE;
            }

            delete[] pLutX;
            delete[] pLutY;
        }

        ReleaseAccess( pReadAcc );
        aNewBmp.ReleaseAccess( pWriteAcc );

        if( bRet )
            ImplAssignWithSize( aNewBmp );
    }

    return bRet;
}

// ------------------------------------------------------------------------

BOOL Bitmap::ImplScaleInterpolate( const double& rScaleX, const double& rScaleY )
{
    const Size  aSizePix( GetSizePixel() );
    const long  nNewWidth = FRound( aSizePix.Width() * rScaleX );
    const long  nNewHeight = FRound( aSizePix.Height() * rScaleY );
    BOOL        bRet = FALSE;

    if( ( nNewWidth > 1L ) && ( nNewHeight > 1L ) )
    {
        BitmapColor         aCol0;
        BitmapColor         aCol1;
        BitmapReadAccess*   pReadAcc = AcquireReadAccess();
        long                nWidth = pReadAcc->Width();
        long                nHeight = pReadAcc->Height();
        Bitmap              aNewBmp( Size( nNewWidth, nHeight ), 24 );
        BitmapWriteAccess*  pWriteAcc = aNewBmp.AcquireWriteAccess();
        long*               pLutInt;
        long*               pLutFrac;
        long                nX, nY;
        long                lXB0, lXB1, lXG0, lXG1, lXR0, lXR1;
        double              fTemp;
        long                nTemp;

        if( pReadAcc && pWriteAcc )
        {
            const long      nNewWidth1 = nNewWidth - 1L;
            const long      nWidth1 = pReadAcc->Width() - 1L;
            const double    fRevScaleX = (double) nWidth1 / nNewWidth1;

            pLutInt = new long[ nNewWidth ];
            pLutFrac = new long[ nNewWidth ];

            for( nX = 0L, nTemp = nWidth - 2L; nX < nNewWidth; nX++ )
            {
                fTemp = nX * fRevScaleX;
                pLutInt[ nX ] = MinMax( (long) fTemp, 0, nTemp );
                fTemp -= pLutInt[ nX ];
                pLutFrac[ nX ] = (long) ( fTemp * 1024. );
            }

            if( pReadAcc->HasPalette() )
            {
                for( nY = 0L; nY < nHeight; nY++ )
                {
                    if( 1 == nWidth )
                    {
                        aCol0 = pReadAcc->GetPaletteColor( pReadAcc->GetPixel( nY, 0 ) );

                        for( nX = 0L; nX < nNewWidth; nX++ )
                            pWriteAcc->SetPixel( nY, nX, aCol0 );
                    }
                    else
                    {
                        for( nX = 0L; nX < nNewWidth; nX++ )
                        {
                            nTemp = pLutInt[ nX ];

                            aCol0 = pReadAcc->GetPaletteColor( pReadAcc->GetPixel( nY, nTemp++ ) );
                            aCol1 = pReadAcc->GetPaletteColor( pReadAcc->GetPixel( nY, nTemp ) );

                            nTemp = pLutFrac[ nX ];

                            lXR1 = aCol1.GetRed() - ( lXR0 = aCol0.GetRed() );
                            lXG1 = aCol1.GetGreen() - ( lXG0 = aCol0.GetGreen() );
                            lXB1 = aCol1.GetBlue() - ( lXB0 = aCol0.GetBlue() );

                            aCol0.SetRed( (BYTE) ( ( lXR1 * nTemp + ( lXR0 << 10 ) ) >> 10 ) );
                            aCol0.SetGreen( (BYTE) ( ( lXG1 * nTemp + ( lXG0 << 10 ) ) >> 10 ) );
                            aCol0.SetBlue( (BYTE) ( ( lXB1 * nTemp + ( lXB0 << 10 ) ) >> 10 ) );

                            pWriteAcc->SetPixel( nY, nX, aCol0 );
                        }
                    }
                }
            }
            else
            {
                for( nY = 0L; nY < nHeight; nY++ )
                {
                    if( 1 == nWidth )
                    {
                        aCol0 = pReadAcc->GetPixel( nY, 0 );

                        for( nX = 0L; nX < nNewWidth; nX++ )
                            pWriteAcc->SetPixel( nY, nX, aCol0 );
                    }
                    else
                    {
                        for( nX = 0L; nX < nNewWidth; nX++ )
                        {
                            nTemp = pLutInt[ nX ];

                            aCol0 = pReadAcc->GetPixel( nY, nTemp++ );
                            aCol1 = pReadAcc->GetPixel( nY, nTemp );

                            nTemp = pLutFrac[ nX ];

                            lXR1 = aCol1.GetRed() - ( lXR0 = aCol0.GetRed() );
                            lXG1 = aCol1.GetGreen() - ( lXG0 = aCol0.GetGreen() );
                            lXB1 = aCol1.GetBlue() - ( lXB0 = aCol0.GetBlue() );

                            aCol0.SetRed( (BYTE) ( ( lXR1 * nTemp + ( lXR0 << 10 ) ) >> 10 ) );
                            aCol0.SetGreen( (BYTE) ( ( lXG1 * nTemp + ( lXG0 << 10 ) ) >> 10 ) );
                            aCol0.SetBlue( (BYTE) ( ( lXB1 * nTemp + ( lXB0 << 10 ) ) >> 10 ) );

                            pWriteAcc->SetPixel( nY, nX, aCol0 );
                        }
                    }
                }
            }

            delete[] pLutInt;
            delete[] pLutFrac;
            bRet = TRUE;
        }

        ReleaseAccess( pReadAcc );
        aNewBmp.ReleaseAccess( pWriteAcc );

        if( bRet )
        {
            bRet = FALSE;
            ImplAssignWithSize( aNewBmp );
            pReadAcc = AcquireReadAccess();
            aNewBmp = Bitmap( Size( nNewWidth, nNewHeight ), 24 );
            pWriteAcc = aNewBmp.AcquireWriteAccess();

            if( pReadAcc && pWriteAcc )
            {
                const long      nNewHeight1 = nNewHeight - 1L;
                const long      nHeight1 = pReadAcc->Height() - 1L;
                const double    fRevScaleY = (double) nHeight1 / nNewHeight1;

                pLutInt = new long[ nNewHeight ];
                pLutFrac = new long[ nNewHeight ];

                for( nY = 0L, nTemp = nHeight - 2L; nY < nNewHeight; nY++ )
                {
                    fTemp = nY * fRevScaleY;
                    pLutInt[ nY ] = MinMax( (long) fTemp, 0, nTemp );
                    fTemp -= pLutInt[ nY ];
                    pLutFrac[ nY ] = (long) ( fTemp * 1024. );
                }

                if( pReadAcc->HasPalette() )
                {
                    for( nX = 0L; nX < nNewWidth; nX++ )
                    {
                        if( 1 == nHeight )
                        {
                            aCol0 = pReadAcc->GetPaletteColor( pReadAcc->GetPixel( 0, nX ) );

                            for( nY = 0L; nY < nNewHeight; nY++ )
                                pWriteAcc->SetPixel( nY, nX, aCol0 );
                        }
                        else
                        {
                            for( nY = 0L; nY < nNewHeight; nY++ )
                            {
                                nTemp = pLutInt[ nY ];

                                aCol0 = pReadAcc->GetPaletteColor( pReadAcc->GetPixel( nTemp++, nX ) );
                                aCol1 = pReadAcc->GetPaletteColor( pReadAcc->GetPixel( nTemp, nX ) );

                                nTemp = pLutFrac[ nY ];

                                lXR1 = aCol1.GetRed() - ( lXR0 = aCol0.GetRed() );
                                lXG1 = aCol1.GetGreen() - ( lXG0 = aCol0.GetGreen() );
                                lXB1 = aCol1.GetBlue() - ( lXB0 = aCol0.GetBlue() );

                                aCol0.SetRed( (BYTE) ( ( lXR1 * nTemp + ( lXR0 << 10 ) ) >> 10 ) );
                                aCol0.SetGreen( (BYTE) ( ( lXG1 * nTemp + ( lXG0 << 10 ) ) >> 10 ) );
                                aCol0.SetBlue( (BYTE) ( ( lXB1 * nTemp + ( lXB0 << 10 ) ) >> 10 ) );

                                pWriteAcc->SetPixel( nY, nX, aCol0 );
                            }
                        }
                    }
                }
                else
                {
                    for( nX = 0L; nX < nNewWidth; nX++ )
                    {
                        if( 1 == nHeight )
                        {
                            aCol0 = pReadAcc->GetPixel( 0, nX );

                            for( nY = 0L; nY < nNewHeight; nY++ )
                                pWriteAcc->SetPixel( nY, nX, aCol0 );
                        }
                        else
                        {
                            for( nY = 0L; nY < nNewHeight; nY++ )
                            {
                                nTemp = pLutInt[ nY ];

                                aCol0 = pReadAcc->GetPixel( nTemp++, nX );
                                aCol1 = pReadAcc->GetPixel( nTemp, nX );

                                nTemp = pLutFrac[ nY ];

                                lXR1 = aCol1.GetRed() - ( lXR0 = aCol0.GetRed() );
                                lXG1 = aCol1.GetGreen() - ( lXG0 = aCol0.GetGreen() );
                                lXB1 = aCol1.GetBlue() - ( lXB0 = aCol0.GetBlue() );

                                aCol0.SetRed( (BYTE) ( ( lXR1 * nTemp + ( lXR0 << 10 ) ) >> 10 ) );
                                aCol0.SetGreen( (BYTE) ( ( lXG1 * nTemp + ( lXG0 << 10 ) ) >> 10 ) );
                                aCol0.SetBlue( (BYTE) ( ( lXB1 * nTemp + ( lXB0 << 10 ) ) >> 10 ) );

                                pWriteAcc->SetPixel( nY, nX, aCol0 );
                            }
                        }
                    }
                }

                delete[] pLutInt;
                delete[] pLutFrac;
                bRet = TRUE;
            }

            ReleaseAccess( pReadAcc );
            aNewBmp.ReleaseAccess( pWriteAcc );

            if( bRet )
                ImplAssignWithSize( aNewBmp );
        }
    }

    if( !bRet )
        bRet = ImplScaleFast( rScaleX, rScaleY );

    return bRet;
}

// ------------------------------------------------------------------------

BOOL Bitmap::Dither( ULONG nDitherFlags, const BitmapPalette* pDitherPal )
{
    DBG_ASSERT( !pDitherPal, "Sorry, using owner defined palettes is not yet supportet..." );

    BOOL bRet = FALSE;

    const Size aSizePix( GetSizePixel() );

    if( aSizePix.Width() == 1 || aSizePix.Height() == 1 )
        bRet = TRUE;
    else if( nDitherFlags & BMP_DITHER_MATRIX )
        bRet = ImplDitherMatrix( pDitherPal );
    else if( nDitherFlags & BMP_DITHER_FLOYD )
        bRet = ImplDitherFloyd( pDitherPal );
    else if( ( nDitherFlags & BMP_DITHER_FLOYD_16 ) && ( GetBitCount() == 24 ) )
        bRet = ImplDitherFloyd16();

    return bRet;
}

//fuer WIN16 Borland
#ifdef WIN
#pragma codeseg BITMAP3_SEG1
#endif

// ------------------------------------------------------------------------

BOOL Bitmap::ImplDitherMatrix( const BitmapPalette* pDitherPal )
{
    BitmapReadAccess*   pReadAcc = AcquireReadAccess();
    Bitmap              aNewBmp( GetSizePixel(), 8 );
    BitmapWriteAccess*  pWriteAcc = aNewBmp.AcquireWriteAccess();
    BOOL                bRet = FALSE;

    if( pReadAcc && pWriteAcc )
    {
        const ULONG nWidth = pReadAcc->Width();
        const ULONG nHeight = pReadAcc->Height();
        BitmapColor aIndex( (BYTE) 0 );

        if( pReadAcc->HasPalette() )
        {
            for( ULONG nY = 0UL; nY < nHeight; nY++ )
            {
                for( ULONG nX = 0UL, nModY = ( nY & 0x0FUL ) << 4UL; nX < nWidth; nX++ )
                {
                    const BitmapColor   aCol( pReadAcc->GetPaletteColor( pReadAcc->GetPixel( nY, nX ) ) );
                    const ULONG         nD = nVCLDitherLut[ nModY + ( nX & 0x0FUL ) ];
                    const ULONG         nR = ( nVCLLut[ aCol.GetRed() ] + nD ) >> 16UL;
                    const ULONG         nG = ( nVCLLut[ aCol.GetGreen() ] + nD ) >> 16UL;
                    const ULONG         nB = ( nVCLLut[ aCol.GetBlue() ] + nD ) >> 16UL;

                    aIndex.SetIndex( (BYTE) ( nVCLRLut[ nR ] + nVCLGLut[ nG ] + nVCLBLut[ nB ] ) );
                    pWriteAcc->SetPixel( nY, nX, aIndex );
                }
            }
        }
        else
        {
            for( ULONG nY = 0UL; nY < nHeight; nY++ )
            {
                for( ULONG nX = 0UL, nModY = ( nY & 0x0FUL ) << 4UL; nX < nWidth; nX++ )
                {
                    const BitmapColor   aCol( pReadAcc->GetPixel( nY, nX ) );
                    const ULONG         nD = nVCLDitherLut[ nModY + ( nX & 0x0FUL ) ];
                    const ULONG         nR = ( nVCLLut[ aCol.GetRed() ] + nD ) >> 16UL;
                    const ULONG         nG = ( nVCLLut[ aCol.GetGreen() ] + nD ) >> 16UL;
                    const ULONG         nB = ( nVCLLut[ aCol.GetBlue() ] + nD ) >> 16UL;

                    aIndex.SetIndex( (BYTE) ( nVCLRLut[ nR ] + nVCLGLut[ nG ] + nVCLBLut[ nB ] ) );
                    pWriteAcc->SetPixel( nY, nX, aIndex );
                }
            }
        }

        bRet = TRUE;
    }

    ReleaseAccess( pReadAcc );
    aNewBmp.ReleaseAccess( pWriteAcc );

    if( bRet )
    {
        const MapMode   aMap( maPrefMapMode );
        const Size      aSize( maPrefSize );

        *this = aNewBmp;

        maPrefMapMode = aMap;
        maPrefSize = aSize;
    }

    return bRet;
}

// ------------------------------------------------------------------------

BOOL Bitmap::ImplDitherFloyd( const BitmapPalette* pDitherPal )
{
    const Size  aSize( GetSizePixel() );
    BOOL        bRet = FALSE;

    if( ( aSize.Width() > 3 ) && ( aSize.Height() > 2 ) )
    {
        BitmapReadAccess*   pReadAcc = AcquireReadAccess();
        Bitmap              aNewBmp( GetSizePixel(), 8 );
        BitmapWriteAccess*  pWriteAcc = aNewBmp.AcquireWriteAccess();

        if( pReadAcc && pWriteAcc )
        {
            BitmapColor aColor;
            long        nWidth = pReadAcc->Width();
            long        nWidth1 = nWidth - 1L;
            long        nHeight = pReadAcc->Height();
            long        nX;
            long        nW = nWidth * 3L;
            long        nW2 = nW - 3L;
            long        nWLen = nW << 2;
            long        nRErr, nGErr, nBErr;
            long        nRC, nGC, nBC;
            long        nTemp;
            long        nZ;
            long*       p1 = new long[ nW ];
            long*       p2 = new long[ nW ];
            long*       p1T = p1;
            long*       p2T = p2;
            long*       pTmp;
            BOOL        bPal = pReadAcc->HasPalette();

            pTmp = p2T;

            if( bPal )
            {
                for( nZ = 0; nZ < nWidth; nZ++ )
                {
                    aColor = pReadAcc->GetPaletteColor( pReadAcc->GetPixel( 0, nZ ) );

                    *pTmp++ = (long) aColor.GetBlue() << 12;
                    *pTmp++ = (long) aColor.GetGreen() << 12;
                    *pTmp++ = (long) aColor.GetRed() << 12;
                }
            }
            else
            {
                for( nZ = 0; nZ < nWidth; nZ++ )
                {
                    aColor = pReadAcc->GetPixel( 0, nZ );

                    *pTmp++ = (long) aColor.GetBlue() << 12;
                    *pTmp++ = (long) aColor.GetGreen() << 12;
                    *pTmp++ = (long) aColor.GetRed() << 12;
                }
            }

            for( long nY = 1, nYAcc = 0L; nY <= nHeight; nY++, nYAcc++ )
            {
                pTmp = p1T;
                p1T = p2T;
                p2T = pTmp;

                if( nY < nHeight )
                {
                    if( bPal )
                    {
                        for( nZ = 0; nZ < nWidth; nZ++ )
                        {
                            aColor = pReadAcc->GetPaletteColor( pReadAcc->GetPixel( nY, nZ ) );

                            *pTmp++ = (long) aColor.GetBlue() << 12;
                            *pTmp++ = (long) aColor.GetGreen() << 12;
                            *pTmp++ = (long) aColor.GetRed() << 12;
                        }
                    }
                    else
                    {
                        for( nZ = 0; nZ < nWidth; nZ++ )
                        {
                            aColor = pReadAcc->GetPixel( nY, nZ );

                            *pTmp++ = (long) aColor.GetBlue() << 12;
                            *pTmp++ = (long) aColor.GetGreen() << 12;
                            *pTmp++ = (long) aColor.GetRed() << 12;
                        }
                    }
                }

                // erstes Pixel gesondert betrachten
                nX = 0;
                CALC_ERRORS;
                CALC_TABLES7;
                nX -= 5;
                CALC_TABLES5;
                pWriteAcc->SetPixel( nYAcc, 0, BitmapColor( (BYTE) ( nVCLBLut[ nBC ] + nVCLGLut[nGC ] + nVCLRLut[nRC ] ) ) );

                // mittlere Pixel ueber Schleife
                for ( long nX = 3L, nXAcc = 1L; nX < nW2; nXAcc++ )
                {
                    CALC_ERRORS;
                    CALC_TABLES7;
                    nX -= 8;
                    CALC_TABLES3;
                    CALC_TABLES5;
                    pWriteAcc->SetPixel( nYAcc, nXAcc, BitmapColor( (BYTE) ( nVCLBLut[ nBC ] + nVCLGLut[nGC ] + nVCLRLut[nRC ] ) ) );
                }

                // letztes Pixel gesondert betrachten
                CALC_ERRORS;
                nX -= 5;
                CALC_TABLES3;
                CALC_TABLES5;
                pWriteAcc->SetPixel( nYAcc, nWidth1, BitmapColor( (BYTE) ( nVCLBLut[ nBC ] + nVCLGLut[nGC ] + nVCLRLut[nRC ] ) ) );
            }

            delete[] p1;
            delete[] p2;
            bRet = TRUE;
        }

        ReleaseAccess( pReadAcc );
        aNewBmp.ReleaseAccess( pWriteAcc );

        if( bRet )
        {
            const MapMode   aMap( maPrefMapMode );
            const Size      aSize( maPrefSize );

            *this = aNewBmp;

            maPrefMapMode = aMap;
            maPrefSize = aSize;
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

BOOL Bitmap::ImplDitherFloyd16()
{
    BitmapReadAccess*   pReadAcc = AcquireReadAccess();
    Bitmap              aNewBmp( GetSizePixel(), 24 );
    BitmapWriteAccess*  pWriteAcc = aNewBmp.AcquireWriteAccess();
    BOOL                bRet = FALSE;

    if( pReadAcc && pWriteAcc )
    {
        const long      nWidth = pWriteAcc->Width();
        const long      nWidth1 = nWidth - 1L;
        const long      nHeight = pWriteAcc->Height();
        BitmapColor     aColor;
        BitmapColor     aBestCol;
        ImpErrorQuad    aErrQuad;
        ImpErrorQuad*   pErrQuad1 = new ImpErrorQuad[ nWidth ];
        ImpErrorQuad*   pErrQuad2 = new ImpErrorQuad[ nWidth ];
        ImpErrorQuad*   pQLine1 = pErrQuad1;
        ImpErrorQuad*   pQLine2;
        long            nX, nY;
        long            nYTmp = 0L;
        BOOL            bQ1 = TRUE;

        for( nY = 0L; nY < Min( nHeight, 2L ); nY++, nYTmp++ )
            for( nX = 0L, pQLine2 = !nY ? pErrQuad1 : pErrQuad2; nX < nWidth; nX++ )
                pQLine2[ nX ] = pReadAcc->GetPixel( nYTmp, nX );

        for( nY = 0L; nY < nHeight; nY++, nYTmp++ )
        {
            // erstes ZeilenPixel
            aBestCol = pQLine1[ 0 ].ImplGetColor();
            aBestCol.SetRed( ( aBestCol.GetRed() & 248 ) | 7 );
            aBestCol.SetGreen( ( aBestCol.GetGreen() & 248 ) | 7 );
            aBestCol.SetBlue( ( aBestCol.GetBlue() & 248 ) | 7 );
            pWriteAcc->SetPixel( nY, 0, aBestCol );

            for( nX = 1L; nX < nWidth1; nX++ )
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

            // letztes ZeilenPixel
            aBestCol = pQLine1[ nWidth1 ].ImplGetColor();
            aBestCol.SetRed( ( aBestCol.GetRed() & 248 ) | 7 );
            aBestCol.SetGreen( ( aBestCol.GetGreen() & 248 ) | 7 );
            aBestCol.SetBlue( ( aBestCol.GetBlue() & 248 ) | 7 );
            pWriteAcc->SetPixel( nY, nX, aBestCol );

            // Zeilenpuffer neu fuellen/kopieren
            pQLine1 = pQLine2;
            pQLine2 = ( bQ1 = !bQ1 ) ? pErrQuad2 : pErrQuad1;

            if( nYTmp < nHeight )
                for( nX = 0L; nX < nWidth; nX++ )
                    pQLine2[ nX ] = pReadAcc->GetPixel( nYTmp, nX );
        }

        // Zeilenpuffer zerstoeren
        delete[] pErrQuad1;
        delete[] pErrQuad2;
        bRet = TRUE;
    }

    ReleaseAccess( pReadAcc );
    aNewBmp.ReleaseAccess( pWriteAcc );

    if( bRet )
    {
        const MapMode   aMap( maPrefMapMode );
        const Size      aSize( maPrefSize );

        *this = aNewBmp;

        maPrefMapMode = aMap;
        maPrefSize = aSize;
    }

    return bRet;
}

// ------------------------------------------------------------------------

BOOL Bitmap::ReduceColors( USHORT nColorCount, BmpReduce eReduce )
{
    BOOL bRet;

    if( GetColorCount() <= (ULONG) nColorCount )
        bRet = TRUE;
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
        bRet = FALSE;

    return bRet;
}

// ------------------------------------------------------------------------

BOOL Bitmap::ImplReduceSimple( USHORT nColorCount )
{
    Bitmap              aNewBmp;
    BitmapReadAccess*   pRAcc = AcquireReadAccess();
    const USHORT        nColCount = Min( nColorCount, (USHORT) 256 );
    USHORT              nBitCount;
    BOOL                bRet = FALSE;

    if( nColCount <= 2 )
        nBitCount = 1;
    else if( nColCount <= 16 )
        nBitCount = 4;
    else
        nBitCount = 8;

    if( pRAcc )
    {
        Octree                  aOct( *pRAcc, nColCount );
        const BitmapPalette&    rPal = aOct.GetPalette();
        BitmapWriteAccess*      pWAcc;

        aNewBmp = Bitmap( GetSizePixel(), nBitCount, &rPal );
        pWAcc = aNewBmp.AcquireWriteAccess();

        if( pWAcc )
        {
            const long nWidth = pRAcc->Width();
            const long nHeight = pRAcc->Height();

            if( pRAcc->HasPalette() )
            {
                for( long nY = 0L; nY < nHeight; nY++ )
                    for( long nX =0L; nX < nWidth; nX++ )
                        pWAcc->SetPixel( nY, nX, (BYTE) aOct.GetBestPaletteIndex( pRAcc->GetPaletteColor( pRAcc->GetPixel( nY, nX ) ) ) );
            }
            else
            {
                for( long nY = 0L; nY < nHeight; nY++ )
                    for( long nX =0L; nX < nWidth; nX++ )
                        pWAcc->SetPixel( nY, nX, (BYTE) aOct.GetBestPaletteIndex( pRAcc->GetPixel( nY, nX ) ) );
            }

            aNewBmp.ReleaseAccess( pWAcc );
            bRet = TRUE;
        }

        ReleaseAccess( pRAcc );
    }

    if( bRet )
    {
        const MapMode   aMap( maPrefMapMode );
        const Size      aSize( maPrefSize );

        *this = aNewBmp;
        maPrefMapMode = aMap;
        maPrefSize = aSize;
    }

    return bRet;
}

// ------------------------------------------------------------------------

struct PopularColorCount
{
    sal_uInt32  mnIndex;
    sal_uInt32  mnCount;
};

// ------------------------------------------------------------------------

extern "C" int __LOADONCALLAPI ImplPopularCmpFnc( const void* p1, const void* p2 )
{
    int nRet;

    if( ( (PopularColorCount*) p1 )->mnCount < ( (PopularColorCount*) p2 )->mnCount )
        nRet = 1;
    else if( ( (PopularColorCount*) p1 )->mnCount == ( (PopularColorCount*) p2 )->mnCount )
        nRet = 0;
    else
        nRet = -1;

    return nRet;
}

// ------------------------------------------------------------------------

BOOL Bitmap::ImplReducePopular( USHORT nColCount )
{
    BitmapReadAccess*   pRAcc = AcquireReadAccess();
    USHORT              nBitCount;
    BOOL                bRet = FALSE;

    if( nColCount > 256 )
        nColCount = 256;

    if( nColCount < 17 )
        nBitCount = 4;
    else
        nBitCount = 8;

    if( pRAcc )
    {
        const sal_uInt32    nValidBits = 4;
        const sal_uInt32    nRightShiftBits = 8 - nValidBits;
        const sal_uInt32    nLeftShiftBits1 = nValidBits;
        const sal_uInt32    nLeftShiftBits2 = nValidBits << 1;
        const sal_uInt32    nColorsPerComponent = 1 << nValidBits;
        const sal_uInt32    nColorOffset = 256 / nColorsPerComponent;
        const sal_uInt32    nTotalColors = nColorsPerComponent * nColorsPerComponent * nColorsPerComponent;
        const long          nWidth = pRAcc->Width();
        const long          nHeight = pRAcc->Height();
        PopularColorCount*  pCountTable = new PopularColorCount[ nTotalColors ];
        long                nX, nY, nR, nG, nB, nIndex;

        rtl_zeroMemory( pCountTable, nTotalColors * sizeof( PopularColorCount ) );

        for( nR = 0, nIndex = 0; nR < 256; nR += nColorOffset )
            for( nG = 0; nG < 256; nG += nColorOffset )
                for( nB = 0; nB < 256; nB += nColorOffset )
                    pCountTable[ nIndex ].mnIndex = nIndex++;

        if( pRAcc->HasPalette() )
        {
            for( nY = 0L; nY < nHeight; nY++ )
            {
                for( nX = 0L; nX < nWidth; nX++ )
                {
                    const BitmapColor& rCol = pRAcc->GetPaletteColor( pRAcc->GetPixel( nY, nX ) );
                    pCountTable[ ( ( ( (sal_uInt32) rCol.GetRed() ) >> nRightShiftBits ) << nLeftShiftBits2 ) |
                                 ( ( ( (sal_uInt32) rCol.GetGreen() ) >> nRightShiftBits ) << nLeftShiftBits1 ) |
                                 ( ( (sal_uInt32) rCol.GetBlue() ) >> nRightShiftBits ) ].mnCount++;
                }
            }
        }
        else
        {
            for( nY = 0L; nY < nHeight; nY++ )
            {
                for( nX = 0L; nX < nWidth; nX++ )
                {
                    const BitmapColor aCol( pRAcc->GetPixel( nY, nX ) );
                    pCountTable[ ( ( ( (sal_uInt32) aCol.GetRed() ) >> nRightShiftBits ) << nLeftShiftBits2 ) |
                                 ( ( ( (sal_uInt32) aCol.GetGreen() ) >> nRightShiftBits ) << nLeftShiftBits1 ) |
                                 ( ( (sal_uInt32) aCol.GetBlue() ) >> nRightShiftBits ) ].mnCount++;
                }
            }
        }

        BitmapPalette aNewPal( nColCount );

        qsort( pCountTable, nTotalColors, sizeof( PopularColorCount ), ImplPopularCmpFnc );

        for( USHORT n = 0; n < nColCount; n++ )
        {
            const PopularColorCount& rPop = pCountTable[ n ];
            aNewPal[ n ] = BitmapColor( (BYTE) ( ( rPop.mnIndex >> nLeftShiftBits2 ) << nRightShiftBits ),
                                        (BYTE) ( ( ( rPop.mnIndex >> nLeftShiftBits1 ) & ( nColorsPerComponent - 1 ) ) << nRightShiftBits ),
                                        (BYTE) ( ( rPop.mnIndex & ( nColorsPerComponent - 1 ) ) << nRightShiftBits ) );
        }

        Bitmap              aNewBmp( GetSizePixel(), nBitCount, &aNewPal );
        BitmapWriteAccess*  pWAcc = aNewBmp.AcquireWriteAccess();

        if( pWAcc )
        {
            BitmapColor aDstCol( (BYTE) 0 );
            BYTE*       pIndexMap = new BYTE[ nTotalColors ];

            for( nR = 0, nIndex = 0; nR < 256; nR += nColorOffset )
                for( nG = 0; nG < 256; nG += nColorOffset )
                    for( nB = 0; nB < 256; nB += nColorOffset )
                        pIndexMap[ nIndex++ ] = aNewPal.GetBestIndex( BitmapColor( (BYTE) nR, (BYTE) nG, (BYTE) nB ) );

            if( pRAcc->HasPalette() )
            {
                for( nY = 0L; nY < nHeight; nY++ )
                {
                    for( nX = 0L; nX < nWidth; nX++ )
                    {
                        const BitmapColor& rCol = pRAcc->GetPaletteColor( pRAcc->GetPixel( nY, nX ) );
                        aDstCol.SetIndex( pIndexMap[ ( ( ( (sal_uInt32) rCol.GetRed() ) >> nRightShiftBits ) << nLeftShiftBits2 ) |
                                                     ( ( ( (sal_uInt32) rCol.GetGreen() ) >> nRightShiftBits ) << nLeftShiftBits1 ) |
                                                     ( ( (sal_uInt32) rCol.GetBlue() ) >> nRightShiftBits ) ] );
                        pWAcc->SetPixel( nY, nX, aDstCol );
                    }
                }
            }
            else
            {
                for( nY = 0L; nY < nHeight; nY++ )
                {
                    for( nX = 0L; nX < nWidth; nX++ )
                    {
                        const BitmapColor aCol( pRAcc->GetPixel( nY, nX ) );
                        aDstCol.SetIndex( pIndexMap[ ( ( ( (sal_uInt32) aCol.GetRed() ) >> nRightShiftBits ) << nLeftShiftBits2 ) |
                                                     ( ( ( (sal_uInt32) aCol.GetGreen() ) >> nRightShiftBits ) << nLeftShiftBits1 ) |
                                                     ( ( (sal_uInt32) aCol.GetBlue() ) >> nRightShiftBits ) ] );
                        pWAcc->SetPixel( nY, nX, aDstCol );
                    }
                }
            }

            delete[] pIndexMap;
            aNewBmp.ReleaseAccess( pWAcc );
            bRet = TRUE;
        }

        delete[] pCountTable;
        ReleaseAccess( pRAcc );

        if( bRet )
        {
            const MapMode   aMap( maPrefMapMode );
            const Size      aSize( maPrefSize );

            *this = aNewBmp;
            maPrefMapMode = aMap;
            maPrefSize = aSize;
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

BOOL Bitmap::ImplReduceMedian( USHORT nColCount )
{
    BitmapReadAccess*   pRAcc = AcquireReadAccess();
    USHORT              nBitCount;
    BOOL                bRet = FALSE;

    if( nColCount < 17 )
        nBitCount = 4;
    else if( nColCount < 257 )
        nBitCount = 8;
    else
    {
        DBG_ERROR( "Bitmap::ImplReduceMedian(): invalid color count!" );
        nBitCount = 8;
        nColCount = 256;
    }

    if( pRAcc )
    {
        Bitmap              aNewBmp( GetSizePixel(), nBitCount );
        BitmapWriteAccess*  pWAcc = aNewBmp.AcquireWriteAccess();

        if( pWAcc )
        {
            const ULONG nSize = 32768UL * sizeof( ULONG );
            HPULONG     pColBuf = (HPULONG) SvMemAlloc( nSize );
            const long  nWidth = pWAcc->Width();
            const long  nHeight = pWAcc->Height();
            long        nIndex = 0L;

            HMEMSET( (HPBYTE) pColBuf, 0, nSize );

            // create Buffer
            if( pRAcc->HasPalette() )
            {
                for( long nY = 0L; nY < nHeight; nY++ )
                {
                    for( long nX = 0L; nX < nWidth; nX++ )
                    {
                        const BitmapColor& rCol = pRAcc->GetPaletteColor( pRAcc->GetPixel( nY, nX ) );
                        pColBuf[ RGB15( rCol.GetRed() >> 3, rCol.GetGreen() >> 3, rCol.GetBlue() >> 3 ) ]++;
                    }
                }
            }
            else
            {
                for( long nY = 0L; nY < nHeight; nY++ )
                {
                    for( long nX = 0L; nX < nWidth; nX++ )
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
            for( long nY = 0L; nY < nHeight; nY++ )
                for( long nX = 0L; nX < nWidth; nX++ )
                    pWAcc->SetPixel( nY, nX, (BYTE) aMap.GetBestPaletteIndex( pRAcc->GetColor( nY, nX ) ) );

            SvMemFree( pColBuf );
            aNewBmp.ReleaseAccess( pWAcc );
            bRet = TRUE;
        }

        ReleaseAccess( pRAcc );

        if( bRet )
        {
            const MapMode   aMap( maPrefMapMode );
            const Size      aSize( maPrefSize );

            *this = aNewBmp;
            maPrefMapMode = aMap;
            maPrefSize = aSize;
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

void Bitmap::ImplMedianCut( ULONG* pColBuf, BitmapPalette& rPal,
                            long nR1, long nR2, long nG1, long nG2, long nB1, long nB2,
                            long nColors, long nPixels, long& rIndex )
{
    if( !nPixels )
        return;

    BitmapColor aCol;
    const long  nRLen = nR2 - nR1;
    const long  nGLen = nG2 - nG1;
    const long  nBLen = nB2 - nB1;
    long        nR, nG, nB;
    HPULONG     pBuf = pColBuf;

    if( !nRLen && !nGLen && !nBLen )
    {
        if( pBuf[ RGB15( nR1, nG1, nB1 ) ] )
        {
            aCol.SetRed( (BYTE) ( nR1 << 3 ) );
            aCol.SetGreen( (BYTE) ( nG1 << 3 ) );
            aCol.SetBlue( (BYTE) ( nB1 << 3 ) );
            rPal[ (USHORT) rIndex++ ] = aCol;
        }
    }
    else
    {
        if( 1 == nColors || 1 == nPixels )
        {
            long nPixSum = 0, nRSum = 0, nGSum = 0, nBSum = 0;

            for( nR = nR1; nR <= nR2; nR++ )
            {
                for( nG = nG1; nG <= nG2; nG++ )
                {
                    for( nB = nB1; nB <= nB2; nB++ )
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

            aCol.SetRed( (BYTE) ( ( nRSum / nPixels ) << 3 ) );
            aCol.SetGreen( (BYTE) ( ( nGSum / nPixels ) << 3 ) );
            aCol.SetBlue( (BYTE) ( ( nBSum / nPixels ) << 3 ) );
            rPal[ (USHORT) rIndex++ ] = aCol;
        }
        else
        {
            const long  nTest = ( nPixels >> 1 );
            long        nPixOld, nPixNew = 0;

            if( nBLen > nGLen && nBLen > nRLen )
            {
                nB = nB1 - 1;

                while( nPixNew < nTest )
                {
                    nB++, nPixOld = nPixNew;
                    for( nR = nR1; nR <= nR2; nR++ )
                        for( nG = nG1; nG <= nG2; nG++ )
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
                nG = nG1 - 1;

                while( nPixNew < nTest )
                {
                    nG++, nPixOld = nPixNew;
                    for( nR = nR1; nR <= nR2; nR++ )
                        for( nB = nB1; nB <= nB2; nB++ )
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
                nR = nR1 - 1;

                while( nPixNew < nTest )
                {
                    nR++, nPixOld = nPixNew;
                    for( nG = nG1; nG <= nG2; nG++ )
                        for( nB = nB1; nB <= nB2; nB++ )
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

// ------------------------------------------------------------------------

BOOL Bitmap::Vectorize( PolyPolygon& rPolyPoly, ULONG nFlags, const Link* pProgress )
{
    return ImplVectorizer().ImplVectorize( *this, rPolyPoly, nFlags, pProgress );
}

// ------------------------------------------------------------------------

BOOL Bitmap::Vectorize( GDIMetaFile& rMtf, BYTE cReduce, ULONG nFlags, const Link* pProgress )
{
    return ImplVectorizer().ImplVectorize( *this, rMtf, cReduce, nFlags, pProgress );
}

// ------------------------------------------------------------------------

BOOL Bitmap::Adjust( short nLuminancePercent, short nContrastPercent,
                     short nChannelRPercent, short nChannelGPercent, short nChannelBPercent,
                     double fGamma, BOOL bInvert )
{
    BOOL bRet = FALSE;

    // nothing to do => return quickly
    if( !nLuminancePercent && !nContrastPercent &&
        !nChannelRPercent && !nChannelGPercent && !nChannelBPercent &&
        ( fGamma == 1.0 ) && !bInvert )
    {
        bRet = TRUE;
    }
    else
    {
        BitmapWriteAccess* pAcc = AcquireWriteAccess();

        if( pAcc )
        {
            BitmapColor     aCol;
            const long      nW = pAcc->Width();
            const long      nH = pAcc->Height();
            BYTE*           cMapR = new BYTE[ 256 ];
            BYTE*           cMapG = new BYTE[ 256 ];
            BYTE*           cMapB = new BYTE[ 256 ];
            long            nX, nY;
            double          fM, fROff, fGOff, fBOff, fOff;

            // calculate slope
            if( nContrastPercent >= 0 )
                fM = 128.0 / ( 128.0 - 1.27 * MinMax( nContrastPercent, 0L, 100L ) );
            else
                fM = ( 128.0 + 1.27 * MinMax( nContrastPercent, -100L, 0L ) ) / 128.0;

            // total offset = luminance offset + contrast offset
            fOff = MinMax( nLuminancePercent, -100L, 100L ) * 2.55 + 128.0 - fM * 128.0;

            // channel offset = channel offset  + total offset
            fROff = nChannelRPercent * 2.55 + fOff;
            fGOff = nChannelGPercent * 2.55 + fOff;
            fBOff = nChannelBPercent * 2.55 + fOff;

            // calculate gamma value
            fGamma = ( fGamma <= 0.0 || fGamma > 10.0 ) ? 1.0 : ( 1.0 / fGamma );
            const BOOL bGamma = ( fGamma != 1.0 );

            // create mapping table
            for( nX = 0L; nX < 256L; nX++ )
            {
                cMapR[ nX ] = (BYTE) MinMax( FRound( nX * fM + fROff ), 0L, 255L );
                cMapG[ nX ] = (BYTE) MinMax( FRound( nX * fM + fGOff ), 0L, 255L );
                cMapB[ nX ] = (BYTE) MinMax( FRound( nX * fM + fBOff ), 0L, 255L );

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

                for( USHORT i = 0, nCount = pAcc->GetPaletteEntryCount(); i < nCount; i++ )
                {
                    const BitmapColor& rCol = pAcc->GetPaletteColor( i );
                    aNewCol.SetRed( cMapR[ rCol.GetRed() ] );
                    aNewCol.SetGreen( cMapG[ rCol.GetGreen() ] );
                    aNewCol.SetBlue( cMapB[ rCol.GetBlue() ] );
                    pAcc->SetPaletteColor( i, aNewCol );
                }
            }
            else if( pAcc->GetScanlineFormat() == BMP_FORMAT_24BIT_TC_BGR )
            {
                for( nY = 0L; nY < nH; nY++ )
                {
                    Scanline pScan = pAcc->GetScanline( nY );

                    for( nX = 0L; nX < nW; nX++ )
                    {
                        *pScan = cMapB[ *pScan ]; pScan++;
                        *pScan = cMapG[ *pScan ]; pScan++;
                        *pScan = cMapR[ *pScan ]; pScan++;
                    }
                }
            }
            else if( pAcc->GetScanlineFormat() == BMP_FORMAT_24BIT_TC_RGB )
            {
                for( nY = 0L; nY < nH; nY++ )
                {
                    Scanline pScan = pAcc->GetScanline( nY );

                    for( nX = 0L; nX < nW; nX++ )
                    {
                        *pScan = cMapR[ *pScan ]; pScan++;
                        *pScan = cMapG[ *pScan ]; pScan++;
                        *pScan = cMapB[ *pScan ]; pScan++;
                    }
                }
            }
            else
            {
                for( nY = 0L; nY < nH; nY++ )
                {
                    for( nX = 0L; nX < nW; nX++ )
                    {
                        aCol = pAcc->GetPixel( nY, nX );
                        aCol.SetRed( cMapR[ aCol.GetRed() ] );
                        aCol.SetGreen( cMapG[ aCol.GetGreen() ] );
                        aCol.SetBlue( cMapB[ aCol.GetBlue() ] );
                        pAcc->SetPixel( nY, nX, aCol );
                    }
                }
            }

            delete[] cMapR;
            delete[] cMapG;
            delete[] cMapB;
            ReleaseAccess( pAcc );
            bRet = TRUE;
        }
    }

    return bRet;
}
