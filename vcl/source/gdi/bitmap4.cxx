/*************************************************************************
 *
 *  $RCSfile: bitmap4.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2000-11-21 13:30:46 $
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
#define _SV_BITMAP_CXX

#include <stdlib.h>
#include <vos/macros.hxx>
#include <tools/new.hxx>
#ifndef _SV_BMPACC_HXX
#include <bmpacc.hxx>
#endif
#ifndef _SV_BITMAP_HXX
#include <bitmap.hxx>
#endif

// -----------
// - Defines -
// -----------

#define S2(a,b)             { register long t; if( ( t = b - a ) < 0 ) { a += t; b -= t; } }
#define MN3(a,b,c)          S2(a,b); S2(a,c);
#define MX3(a,b,c)          S2(b,c); S2(a,c);
#define MNMX3(a,b,c)        MX3(a,b,c); S2(a,b);
#define MNMX4(a,b,c,d)      S2(a,b); S2(c,d); S2(a,c); S2(b,d);
#define MNMX5(a,b,c,d,e)    S2(a,b); S2(c,d); MN3(a,c,e); MX3(b,d,e);
#define MNMX6(a,b,c,d,e,f)  S2(a,d); S2(b,e); S2(c,f); MN3(a,b,c); MX3(d,e,f);

// ----------
// - Bitmap -
// ----------

BOOL Bitmap::Filter( BmpFilter eFilter, const BmpFilterParam* pFilterParam, const Link* pProgress )
{
    const USHORT    nBitCount = GetBitCount();
    BOOL            bRet = FALSE;

    switch( eFilter )
    {
        case( BMP_FILTER_SMOOTH ):
        {
            const long pSmoothMatrix[] = { 1, 2, 1, 2, 5, 2, 1, 2, 1 };
            bRet = ImplConvolute3( &pSmoothMatrix[ 0 ], 17, pFilterParam, pProgress );
        }
        break;

        case( BMP_FILTER_SHARPEN ):
        {
            const long pSharpenMatrix[] = { -1, -1,  -1, -1, 16, -1, -1, -1,  -1 };
            bRet = ImplConvolute3( &pSharpenMatrix[ 0 ], 8, pFilterParam, pProgress );
        }
        break;

        case( BMP_FILTER_REMOVENOISE ):
            bRet = ImplMedianFilter( pFilterParam, pProgress );
        break;

        case( BMP_FILTER_SOBEL_GREY ):
            bRet = ImplSobelGrey( pFilterParam, pProgress );
        break;

        case( BMP_FILTER_SOLARIZE ):
            bRet = ImplSolarize( pFilterParam, pProgress );
        break;

        case( BMP_FILTER_SEPIA ):
            bRet = ImplSepia( pFilterParam, pProgress );
        break;

        case( BMP_FILTER_MOSAIC ):
            bRet = ImplMosaic( pFilterParam, pProgress );
        break;

        case( BMP_FILTER_EMBOSS_GREY ):
            bRet = ImplEmbossGrey( pFilterParam, pProgress );
        break;

        case( BMP_FILTER_POPART ):
            bRet = ImplPopArt( pFilterParam, pProgress );
        break;

        default:
            DBG_ERROR( "Bitmap::Convert(): Unsupported filter" );
        break;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL Bitmap::ImplConvolute3( const long* pMatrix, long nDivisor,
                             const BmpFilterParam* pFilterParam, const Link* pProgress )
{
    BitmapReadAccess*   pReadAcc = AcquireReadAccess();
    BOOL                bRet = FALSE;

    if( pReadAcc )
    {
        Bitmap              aNewBmp( GetSizePixel(), 24 );
        BitmapWriteAccess*  pWriteAcc = aNewBmp.AcquireWriteAccess();

        if( pWriteAcc )
        {
            const long      nWidth = pWriteAcc->Width(), nWidth2 = nWidth + 2;
            const long      nHeight = pWriteAcc->Height(), nHeight2 = nHeight + 2;
            long*           pColm = new long[ nWidth2 ];
            long*           pRows = new long[ nHeight2 ];
            BitmapColor*    pColRow1 = (BitmapColor*) new BYTE[ sizeof( BitmapColor ) * nWidth2 ];
            BitmapColor*    pColRow2 = (BitmapColor*) new BYTE[ sizeof( BitmapColor ) * nWidth2 ];
            BitmapColor*    pColRow3 = (BitmapColor*) new BYTE[ sizeof( BitmapColor ) * nWidth2 ];
            BitmapColor*    pRowTmp1 = pColRow1;
            BitmapColor*    pRowTmp2 = pColRow2;
            BitmapColor*    pRowTmp3 = pColRow3;
            BitmapColor*    pColor;
            long            nY, nX, i, nSumR, nSumG, nSumB, nMatrixVal, nTmp;
            long            (*pKoeff)[ 256 ] = new long[ 9 ][ 256 ];
            long*           pTmp;

            // create LUT of products of matrix value and possible color component values
            for( nY = 0; nY < 9; nY++ )
                for( nX = nTmp = 0, nMatrixVal = pMatrix[ nY ]; nX < 256; nX++, nTmp += nMatrixVal )
                    pKoeff[ nY ][ nX ] = nTmp;

            // create column LUT
            for( i = 0; i < nWidth2; i++ )
                pColm[ i ] = ( i > 0 ) ? ( i - 1 ) : 0;

            pColm[ nWidth + 1 ] = pColm[ nWidth ];

            // create row LUT
            for( i = 0; i < nHeight2; i++ )
                pRows[ i ] = ( i > 0 ) ? ( i - 1 ) : 0;

            pRows[ nHeight + 1 ] = pRows[ nHeight ];

            // read first three rows of bitmap color
            for( i = 0; i < nWidth2; i++ )
            {
                pColRow1[ i ] = pReadAcc->GetColor( pRows[ 0 ], pColm[ i ] );
                pColRow2[ i ] = pReadAcc->GetColor( pRows[ 1 ], pColm[ i ] );
                pColRow3[ i ] = pReadAcc->GetColor( pRows[ 2 ], pColm[ i ] );
            }

            // do convolution
            for( nY = 0; nY < nHeight; )
            {
                for( nX = 0; nX < nWidth; nX++ )
                {
                    // first row
                    nSumR = ( pTmp = pKoeff[ 0 ] )[ ( pColor = pRowTmp1 + nX )->GetRed() ];
                    nSumG = pTmp[ pColor->GetGreen() ];
                    nSumB = pTmp[ pColor->GetBlue() ];

                    nSumR += ( pTmp = pKoeff[ 1 ] )[ ( ++pColor )->GetRed() ];
                    nSumG += pTmp[ pColor->GetGreen() ];
                    nSumB += pTmp[ pColor->GetBlue() ];

                    nSumR += ( pTmp = pKoeff[ 2 ] )[ ( ++pColor )->GetRed() ];
                    nSumG += pTmp[ pColor->GetGreen() ];
                    nSumB += pTmp[ pColor->GetBlue() ];

                    // second row
                    nSumR += ( pTmp = pKoeff[ 3 ] )[ ( pColor = pRowTmp2 + nX )->GetRed() ];
                    nSumG += pTmp[ pColor->GetGreen() ];
                    nSumB += pTmp[ pColor->GetBlue() ];

                    nSumR += ( pTmp = pKoeff[ 4 ] )[ ( ++pColor )->GetRed() ];
                    nSumG += pTmp[ pColor->GetGreen() ];
                    nSumB += pTmp[ pColor->GetBlue() ];

                    nSumR += ( pTmp = pKoeff[ 5 ] )[ ( ++pColor )->GetRed() ];
                    nSumG += pTmp[ pColor->GetGreen() ];
                    nSumB += pTmp[ pColor->GetBlue() ];

                    // third row
                    nSumR += ( pTmp = pKoeff[ 6 ] )[ ( pColor = pRowTmp3 + nX )->GetRed() ];
                    nSumG += pTmp[ pColor->GetGreen() ];
                    nSumB += pTmp[ pColor->GetBlue() ];

                    nSumR += ( pTmp = pKoeff[ 7 ] )[ ( ++pColor )->GetRed() ];
                    nSumG += pTmp[ pColor->GetGreen() ];
                    nSumB += pTmp[ pColor->GetBlue() ];

                    nSumR += ( pTmp = pKoeff[ 8 ] )[ ( ++pColor )->GetRed() ];
                    nSumG += pTmp[ pColor->GetGreen() ];
                    nSumB += pTmp[ pColor->GetBlue() ];

                    // calculate destination color
                    pWriteAcc->SetPixel( nY, nX, BitmapColor( (BYTE) MinMax( nSumR / nDivisor, 0, 255 ),
                                                              (BYTE) MinMax( nSumG / nDivisor, 0, 255 ),
                                                              (BYTE) MinMax( nSumB / nDivisor, 0, 255 ) ) );
                }

                if( ++nY < nHeight )
                {
                    if( pRowTmp1 == pColRow1 )
                        pRowTmp1 = pColRow2, pRowTmp2 = pColRow3, pRowTmp3 = pColRow1;
                    else if( pRowTmp1 == pColRow2 )
                        pRowTmp1 = pColRow3, pRowTmp2 = pColRow1, pRowTmp3 = pColRow2;
                    else
                        pRowTmp1 = pColRow1, pRowTmp2 = pColRow2, pRowTmp3 = pColRow3;

                    for( i = 0; i < nWidth2; i++ )
                        pRowTmp3[ i ] = pReadAcc->GetColor( pRows[ nY + 2 ], pColm[ i ] );
                }
            }

            delete[] pKoeff;
            delete[] (BYTE*) pColRow1;
            delete[] (BYTE*) pColRow2;
            delete[] (BYTE*) pColRow3;
            delete[] pColm;
            delete[] pRows;

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

// -----------------------------------------------------------------------------

BOOL Bitmap::ImplMedianFilter( const BmpFilterParam* pFilterParam, const Link* pProgress )
{
    BitmapReadAccess*   pReadAcc = AcquireReadAccess();
    BOOL                bRet = FALSE;

    if( pReadAcc )
    {
        Bitmap              aNewBmp( GetSizePixel(), 24 );
        BitmapWriteAccess*  pWriteAcc = aNewBmp.AcquireWriteAccess();

        if( pWriteAcc )
        {
            const long      nWidth = pWriteAcc->Width(), nWidth2 = nWidth + 2;
            const long      nHeight = pWriteAcc->Height(), nHeight2 = nHeight + 2;
            long*           pColm = new long[ nWidth2 ];
            long*           pRows = new long[ nHeight2 ];
            BitmapColor*    pColRow1 = (BitmapColor*) new BYTE[ sizeof( BitmapColor ) * nWidth2 ];
            BitmapColor*    pColRow2 = (BitmapColor*) new BYTE[ sizeof( BitmapColor ) * nWidth2 ];
            BitmapColor*    pColRow3 = (BitmapColor*) new BYTE[ sizeof( BitmapColor ) * nWidth2 ];
            BitmapColor*    pRowTmp1 = pColRow1;
            BitmapColor*    pRowTmp2 = pColRow2;
            BitmapColor*    pRowTmp3 = pColRow3;
            BitmapColor*    pColor;
            long            nY, nX, i;
            long            nR1, nR2, nR3, nR4, nR5, nR6, nR7, nR8, nR9;
            long            nG1, nG2, nG3, nG4, nG5, nG6, nG7, nG8, nG9;
            long            nB1, nB2, nB3, nB4, nB5, nB6, nB7, nB8, nB9;

            // create column LUT
            for( i = 0; i < nWidth2; i++ )
                pColm[ i ] = ( i > 0 ) ? ( i - 1 ) : 0;

            pColm[ nWidth + 1 ] = pColm[ nWidth ];

            // create row LUT
            for( i = 0; i < nHeight2; i++ )
                pRows[ i ] = ( i > 0 ) ? ( i - 1 ) : 0;

            pRows[ nHeight + 1 ] = pRows[ nHeight ];

            // read first three rows of bitmap color
            for( i = 0; i < nWidth2; i++ )
            {
                pColRow1[ i ] = pReadAcc->GetColor( pRows[ 0 ], pColm[ i ] );
                pColRow2[ i ] = pReadAcc->GetColor( pRows[ 1 ], pColm[ i ] );
                pColRow3[ i ] = pReadAcc->GetColor( pRows[ 2 ], pColm[ i ] );
            }

            // do median filtering
            for( nY = 0; nY < nHeight; )
            {
                for( nX = 0; nX < nWidth; nX++ )
                {
                    nR1 = ( pColor = pRowTmp1 + nX )->GetRed(), nG1 = pColor->GetGreen(), nB1 = pColor->GetBlue();
                    nR2 = ( ++pColor )->GetRed(), nG2 = pColor->GetGreen(), nB2 = pColor->GetBlue();
                    nR3 = ( ++pColor )->GetRed(), nG3 = pColor->GetGreen(), nB3 = pColor->GetBlue();

                    nR4 = ( pColor = pRowTmp2 + nX )->GetRed(), nG4 = pColor->GetGreen(), nB4 = pColor->GetBlue();
                    nR5 = ( ++pColor )->GetRed(), nG5 = pColor->GetGreen(), nB5 = pColor->GetBlue();
                    nR6 = ( ++pColor )->GetRed(), nG6 = pColor->GetGreen(), nB6 = pColor->GetBlue();

                    nR7 = ( pColor = pRowTmp3 + nX )->GetRed(), nG7 = pColor->GetGreen(), nB7 = pColor->GetBlue();
                    nR8 = ( ++pColor )->GetRed(), nG8 = pColor->GetGreen(), nB8 = pColor->GetBlue();
                    nR9 = ( ++pColor )->GetRed(), nG9 = pColor->GetGreen(), nB9 = pColor->GetBlue();

                    MNMX6( nR1, nR2, nR3, nR4, nR5, nR6 );
                    MNMX5( nR7, nR2, nR3, nR4, nR5 );
                    MNMX4( nR8, nR2, nR3, nR4 );
                    MNMX3( nR9, nR2, nR3 );

                    MNMX6( nG1, nG2, nG3, nG4, nG5, nG6 );
                    MNMX5( nG7, nG2, nG3, nG4, nG5 );
                    MNMX4( nG8, nG2, nG3, nG4 );
                    MNMX3( nG9, nG2, nG3 );

                    MNMX6( nB1, nB2, nB3, nB4, nB5, nB6 );
                    MNMX5( nB7, nB2, nB3, nB4, nB5 );
                    MNMX4( nB8, nB2, nB3, nB4 );
                    MNMX3( nB9, nB2, nB3 );

                    // set destination color
                    pWriteAcc->SetPixel( nY, nX, BitmapColor( (BYTE) nR2, (BYTE) nG2, (BYTE) nB2 ) );
                }

                if( ++nY < nHeight )
                {
                    if( pRowTmp1 == pColRow1 )
                        pRowTmp1 = pColRow2, pRowTmp2 = pColRow3, pRowTmp3 = pColRow1;
                    else if( pRowTmp1 == pColRow2 )
                        pRowTmp1 = pColRow3, pRowTmp2 = pColRow1, pRowTmp3 = pColRow2;
                    else
                        pRowTmp1 = pColRow1, pRowTmp2 = pColRow2, pRowTmp3 = pColRow3;

                    for( i = 0; i < nWidth2; i++ )
                        pRowTmp3[ i ] = pReadAcc->GetColor( pRows[ nY + 2 ], pColm[ i ] );
                }
            }

            delete[] (BYTE*) pColRow1;
            delete[] (BYTE*) pColRow2;
            delete[] (BYTE*) pColRow3;
            delete[] pColm;
            delete[] pRows;

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

// -----------------------------------------------------------------------------

BOOL Bitmap::ImplSobelGrey( const BmpFilterParam* pFilterParam, const Link* pProgress )
{
    BOOL bRet = ImplMakeGreyscales( 256 );

    if( bRet )
    {
        bRet = FALSE;

        BitmapReadAccess* pReadAcc = AcquireReadAccess();

        if( pReadAcc )
        {
            Bitmap              aNewBmp( GetSizePixel(), 8, &pReadAcc->GetPalette() );
            BitmapWriteAccess*  pWriteAcc = aNewBmp.AcquireWriteAccess();

            if( pWriteAcc )
            {
                BitmapColor aGrey( (BYTE) 0 );
                const long  nWidth = pWriteAcc->Width();
                const long  nHeight = pWriteAcc->Height();
                const long  nMask111 = -1, nMask121 =  0, nMask131 =  1;
                const long  nMask211 = -2, nMask221 =  0, nMask231 =  2;
                const long  nMask311 = -1, nMask321 =  0, nMask331 =  1;
                const long  nMask112 =  1, nMask122 =  2, nMask132 =  1;
                const long  nMask212 =  0, nMask222 =  0, nMask232 =  0;
                const long  nMask312 = -1, nMask322 = -2, nMask332 = -1;
                long        nGrey11, nGrey12, nGrey13;
                long        nGrey21, nGrey22, nGrey23;
                long        nGrey31, nGrey32, nGrey33;
                long*       pHMap = new long[ nWidth + 2 ];
                long*       pVMap = new long[ nHeight + 2 ];
                long        nX, nY, nSum1, nSum2;

                // fill mapping tables
                pHMap[ 0 ] = 0;
                for( nX = 1; nX <= nWidth; nX++ )
                    pHMap[ nX ] = nX - 1;
                pHMap[ nWidth + 1 ] = nWidth - 1;

                pVMap[ 0 ] = 0;
                for( nY = 1; nY <= nHeight; nY++ )
                    pVMap[ nY ] = nY - 1;
                pVMap[ nHeight + 1 ] = nHeight - 1;

                for( nY = 0; nY < nHeight ; nY++ )
                {
                    nGrey11 = pReadAcc->GetPixel( pVMap[ nY ], pHMap[ 0 ] ).GetIndex();
                    nGrey12 = pReadAcc->GetPixel( pVMap[ nY ], pHMap[ 1 ] ).GetIndex();
                    nGrey13 = pReadAcc->GetPixel( pVMap[ nY ], pHMap[ 2 ] ).GetIndex();
                    nGrey21 = pReadAcc->GetPixel( pVMap[ nY + 1 ], pHMap[ 0 ] ).GetIndex();
                    nGrey22 = pReadAcc->GetPixel( pVMap[ nY + 1 ], pHMap[ 1 ] ).GetIndex();
                    nGrey23 = pReadAcc->GetPixel( pVMap[ nY + 1 ], pHMap[ 2 ] ).GetIndex();
                    nGrey31 = pReadAcc->GetPixel( pVMap[ nY + 2 ], pHMap[ 0 ] ).GetIndex();
                    nGrey32 = pReadAcc->GetPixel( pVMap[ nY + 2 ], pHMap[ 1 ] ).GetIndex();
                    nGrey33 = pReadAcc->GetPixel( pVMap[ nY + 2 ], pHMap[ 2 ] ).GetIndex();

                    for( nX = 0; nX < nWidth; nX++ )
                    {
                        nSum1 = nSum2 = 0;

                        nSum1 += nMask111 * nGrey11;
                        nSum2 += nMask112 * nGrey11;

                        nSum1 += nMask121 * nGrey12;
                        nSum2 += nMask122 * nGrey12;

                        nSum1 += nMask131 * nGrey13;
                        nSum2 += nMask132 * nGrey13;

                        nSum1 += nMask211 * nGrey21;
                        nSum2 += nMask212 * nGrey21;

                        nSum1 += nMask221 * nGrey22;
                        nSum2 += nMask222 * nGrey22;

                        nSum1 += nMask231 * nGrey23;
                        nSum2 += nMask232 * nGrey23;

                        nSum1 += nMask311 * nGrey31;
                        nSum2 += nMask312 * nGrey31;

                        nSum1 += nMask321 * nGrey32;
                        nSum2 += nMask322 * nGrey32;

                        nSum1 += nMask331 * nGrey33;
                        nSum2 += nMask332 * nGrey33;

                        nSum1 = (long) sqrt( ( nSum1 * nSum1 + nSum2 * nSum2 ) );
                        aGrey.SetIndex( ~(BYTE) VOS_BOUND( nSum1, 0, 255 ) );
                        pWriteAcc->SetPixel( nY, nX, aGrey );

                        if( nX < ( nWidth - 1 ) )
                        {
                            const long nNextX = pHMap[ nX + 3 ];

                            nGrey11 = nGrey12; nGrey12 = nGrey13; nGrey13 = pReadAcc->GetPixel( pVMap[ nY ], nNextX ).GetIndex();
                            nGrey21 = nGrey22; nGrey22 = nGrey23; nGrey23 = pReadAcc->GetPixel( pVMap[ nY + 1 ], nNextX ).GetIndex();
                            nGrey31 = nGrey32; nGrey32 = nGrey33; nGrey33 = pReadAcc->GetPixel( pVMap[ nY + 2 ], nNextX ).GetIndex();
                        }
                    }
                }

                delete[] pHMap;
                delete[] pVMap;
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
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL Bitmap::ImplEmbossGrey( const BmpFilterParam* pFilterParam, const Link* pProgress )
{
    BOOL bRet = ImplMakeGreyscales( 256 );

    if( bRet )
    {
        bRet = FALSE;

        BitmapReadAccess* pReadAcc = AcquireReadAccess();

        if( pReadAcc )
        {
            Bitmap              aNewBmp( GetSizePixel(), 8, &pReadAcc->GetPalette() );
            BitmapWriteAccess*  pWriteAcc = aNewBmp.AcquireWriteAccess();

            if( pWriteAcc )
            {
                BitmapColor aGrey( (BYTE) 0 );
                const long  nWidth = pWriteAcc->Width();
                const long  nHeight = pWriteAcc->Height();
                long        nGrey11, nGrey12, nGrey13;
                long        nGrey21, nGrey22, nGrey23;
                long        nGrey31, nGrey32, nGrey33;
                double      fAzim = ( ( pFilterParam && pFilterParam->meFilter == BMP_FILTER_EMBOSS_GREY ) ?
                                      ( pFilterParam->maEmbossAngles.mnAzimuthAngle100 * 0.01 ) : 0.0 ) * F_PI180;
                double      fElev = ( ( pFilterParam && pFilterParam->meFilter == BMP_FILTER_EMBOSS_GREY ) ?
                                      ( pFilterParam->maEmbossAngles.mnElevationAngle100 * 0.01 ) : 90.0 ) * F_PI180;
                long*       pHMap = new long[ nWidth + 2 ];
                long*       pVMap = new long[ nHeight + 2 ];
                long        nX, nY, nNx, nNy, nDotL;
                const long  nLx = FRound( cos( fAzim ) * cos( fElev ) * 255.0 );
                const long  nLy = FRound( sin( fAzim ) * cos( fElev ) * 255.0 );
                const long  nLz = FRound( sin( fElev ) * 255.0 );
                const long  nZ2 = ( ( 6 * 255 ) / 4 ) * ( ( 6 * 255 ) / 4 );
                const long  nNzLz = ( ( 6 * 255 ) / 4 ) * nLz;
                const BYTE  cLz = (BYTE) VOS_BOUND( nLz, 0, 255 );

                // fill mapping tables
                pHMap[ 0 ] = 0;
                for( nX = 1; nX <= nWidth; nX++ )
                    pHMap[ nX ] = nX - 1;
                pHMap[ nWidth + 1 ] = nWidth - 1;

                pVMap[ 0 ] = 0;
                for( nY = 1; nY <= nHeight; nY++ )
                    pVMap[ nY ] = nY - 1;
                pVMap[ nHeight + 1 ] = nHeight - 1;

                for( nY = 0; nY < nHeight ; nY++ )
                {
                    nGrey11 = pReadAcc->GetPixel( pVMap[ nY ], pHMap[ 0 ] ).GetIndex();
                    nGrey12 = pReadAcc->GetPixel( pVMap[ nY ], pHMap[ 1 ] ).GetIndex();
                    nGrey13 = pReadAcc->GetPixel( pVMap[ nY ], pHMap[ 2 ] ).GetIndex();
                    nGrey21 = pReadAcc->GetPixel( pVMap[ nY + 1 ], pHMap[ 0 ] ).GetIndex();
                    nGrey22 = pReadAcc->GetPixel( pVMap[ nY + 1 ], pHMap[ 1 ] ).GetIndex();
                    nGrey23 = pReadAcc->GetPixel( pVMap[ nY + 1 ], pHMap[ 2 ] ).GetIndex();
                    nGrey31 = pReadAcc->GetPixel( pVMap[ nY + 2 ], pHMap[ 0 ] ).GetIndex();
                    nGrey32 = pReadAcc->GetPixel( pVMap[ nY + 2 ], pHMap[ 1 ] ).GetIndex();
                    nGrey33 = pReadAcc->GetPixel( pVMap[ nY + 2 ], pHMap[ 2 ] ).GetIndex();

                    for( nX = 0; nX < nWidth; nX++ )
                    {
                        nNx = nGrey11 + nGrey21 + nGrey31 - nGrey13 - nGrey23 - nGrey33;
                        nNy = nGrey31 + nGrey32 + nGrey33 - nGrey11 - nGrey12 - nGrey13;

                        if( !nNx && !nNy )
                            aGrey.SetIndex( cLz );
                        else if( ( nDotL = nNx * nLx + nNy * nLy +nNzLz ) < 0 )
                            aGrey.SetIndex( 0 );
                        else
                        {
                            const double fGrey = nDotL / sqrt( nNx * nNx + nNy * nNy + nZ2 );
                            aGrey.SetIndex( (BYTE) VOS_BOUND( fGrey, 0, 255 ) );
                        }

                        pWriteAcc->SetPixel( nY, nX, aGrey );

                        if( nX < ( nWidth - 1 ) )
                        {
                            const long nNextX = pHMap[ nX + 3 ];

                            nGrey11 = nGrey12; nGrey12 = nGrey13; nGrey13 = pReadAcc->GetPixel( pVMap[ nY ], nNextX ).GetIndex();
                            nGrey21 = nGrey22; nGrey22 = nGrey23; nGrey23 = pReadAcc->GetPixel( pVMap[ nY + 1 ], nNextX ).GetIndex();
                            nGrey31 = nGrey32; nGrey32 = nGrey33; nGrey33 = pReadAcc->GetPixel( pVMap[ nY + 2 ], nNextX ).GetIndex();
                        }
                    }
                }

                delete[] pHMap;
                delete[] pVMap;
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
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL Bitmap::ImplSolarize( const BmpFilterParam* pFilterParam, const Link* pProgress )
{
    BOOL                bRet = FALSE;
    BitmapWriteAccess*  pWriteAcc = AcquireWriteAccess();

    if( pWriteAcc )
    {
        const BYTE cThreshold = ( pFilterParam && pFilterParam->meFilter == BMP_FILTER_SOLARIZE ) ?
                                pFilterParam->mcSolarGreyThreshold : 128;

        if( pWriteAcc->HasPalette() )
        {
            const BitmapPalette& rPal = pWriteAcc->GetPalette();

            for( USHORT i = 0, nCount = rPal.GetEntryCount(); i < nCount; i++ )
            {
                if( rPal[ i ].GetLuminance() >= cThreshold )
                {
                    BitmapColor aCol( rPal[ i ] );
                    pWriteAcc->SetPaletteColor( i, aCol.Invert() );
                }
            }
        }
        else
        {
            BitmapColor aCol;
            const long  nWidth = pWriteAcc->Width();
            const long  nHeight = pWriteAcc->Height();

            for( long nY = 0; nY < nHeight ; nY++ )
            {
                for( long nX = 0; nX < nWidth; nX++ )
                {
                    aCol = pWriteAcc->GetPixel( nY, nX );

                    if( aCol.GetLuminance() >= cThreshold )
                        pWriteAcc->SetPixel( nY, nX, aCol.Invert() );
                }
            }
        }

        ReleaseAccess( pWriteAcc );
        bRet = TRUE;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL Bitmap::ImplSepia( const BmpFilterParam* pFilterParam, const Link* pProgress )
{
    BitmapReadAccess*   pReadAcc = AcquireReadAccess();
    BOOL                bRet = FALSE;

    if( pReadAcc )
    {
        long            nSepiaPercent = ( pFilterParam && pFilterParam->meFilter == BMP_FILTER_SEPIA ) ?
                                        pFilterParam->mcSolarGreyThreshold : 10;
        const long      nSepia = 10000 - 100 * VOS_BOUND( nSepiaPercent, 0, 100 );
        BitmapPalette   aSepiaPal( 256 );

        DBG_ASSERT( nSepiaPercent <= 100, "Bitmap::ImplSepia(): sepia value out of range; defaulting to 100%" );

        for( long i = 0; i < 256; i++ )
        {
            BitmapColor&    rCol = aSepiaPal[ i ];
            const BYTE      cSepiaValue = (BYTE) ( ( nSepia * i ) / 10000 );

            rCol.SetRed( (BYTE) i );
            rCol.SetGreen( cSepiaValue );
            rCol.SetBlue( cSepiaValue );
        }

        Bitmap              aNewBmp( GetSizePixel(), 8, &aSepiaPal );
        BitmapWriteAccess*  pWriteAcc = aNewBmp.AcquireWriteAccess();

        if( pWriteAcc )
        {
            BitmapColor aCol( (BYTE) 0 );
            const long  nWidth = pWriteAcc->Width();
            const long  nHeight = pWriteAcc->Height();

            if( pReadAcc->HasPalette() )
            {
                for( long nY = 0; nY < nHeight ; nY++ )
                {
                    const USHORT    nPalCount = pReadAcc->GetPaletteEntryCount();
                    BYTE*           pIndexMap = new BYTE[ nPalCount ];

                    for( USHORT i = 0; i < nPalCount; i++ )
                        pIndexMap[ i ] = pReadAcc->GetPaletteColor( i ).GetLuminance();

                    for( long nX = 0; nX < nWidth; nX++ )
                    {
                        aCol.SetIndex( pIndexMap[ pReadAcc->GetPixel( nY, nX ).GetIndex() ] );
                        pWriteAcc->SetPixel( nY, nX, aCol );
                    }

                    delete[] pIndexMap;
                }
            }
            else
            {
                for( long nY = 0; nY < nHeight ; nY++ )
                {
                    for( long nX = 0; nX < nWidth; nX++ )
                    {
                        aCol.SetIndex( pReadAcc->GetPixel( nY, nX ).GetLuminance() );
                        pWriteAcc->SetPixel( nY, nX, aCol );
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

// -----------------------------------------------------------------------------

BOOL Bitmap::ImplMosaic( const BmpFilterParam* pFilterParam, const Link* pProgress )
{
    ULONG               nTileWidth = ( pFilterParam && pFilterParam->meFilter == BMP_FILTER_MOSAIC ) ?
                                     pFilterParam->maMosaicTileSize.mnTileWidth : 4;
    ULONG               nTileHeight = ( pFilterParam && pFilterParam->meFilter == BMP_FILTER_MOSAIC ) ?
                                      pFilterParam->maMosaicTileSize.mnTileHeight : 4;
    BOOL                bRet = FALSE;

    if( !nTileWidth )
        nTileWidth = 1;

    if( !nTileHeight )
        nTileHeight = 1;

    if( nTileWidth > 1 || nTileHeight > 1 )
    {
        Bitmap*             pNewBmp;
        BitmapReadAccess*   pReadAcc;
        BitmapWriteAccess*  pWriteAcc;

        if( GetBitCount() > 8 )
        {
            pNewBmp = NULL;
            pReadAcc = pWriteAcc = AcquireWriteAccess();
        }
        else
        {
            pNewBmp = new Bitmap( GetSizePixel(), 24 );
            pReadAcc = AcquireReadAccess();
            pWriteAcc = pNewBmp->AcquireWriteAccess();
        }

        if( pReadAcc && pWriteAcc )
        {
            BitmapColor aCol;
            long        nWidth = pReadAcc->Width();
            long        nHeight = pReadAcc->Height();
            long        nX, nY, nX1, nX2, nY1, nY2, nSumR, nSumG, nSumB;
            double      fArea_1;

            nY1 = 0; nY2 = nTileHeight - 1;

            if( nY2 >= nHeight )
                nY2 = nHeight - 1;

            do
            {
                nX1 = 0; nX2 = nTileWidth - 1;

                if( nX2 >= nWidth )
                    nX2 = nWidth - 1;

                fArea_1 = 1.0 / ( ( nX2 - nX1 + 1 ) * ( nY2 - nY1 + 1 ) );

                if( !pNewBmp )
                {
                    do
                    {
                        for( nY = nY1, nSumR = nSumG = nSumB = 0; nY <= nY2; nY++ )
                        {
                            for( nX = nX1; nX <= nX2; nX++ )
                            {
                                aCol = pReadAcc->GetPixel( nY, nX );
                                nSumR += aCol.GetRed();
                                nSumG += aCol.GetGreen();
                                nSumB += aCol.GetBlue();
                            }
                        }

                        aCol.SetRed( (BYTE) ( nSumR * fArea_1 ) );
                        aCol.SetGreen( (BYTE) ( nSumG * fArea_1 ) );
                        aCol.SetBlue( (BYTE) ( nSumB * fArea_1 ) );

                        for( nY = nY1; nY <= nY2; nY++ )
                            for( nX = nX1; nX <= nX2; nX++ )
                                pWriteAcc->SetPixel( nY, nX, aCol );

                        nX1 += nTileWidth; nX2 += nTileWidth;

                        if( nX2 >= nWidth )
                        {
                            nX2 = nWidth - 1;
                            fArea_1 = 1.0 / ( ( nX2 - nX1 + 1 ) * ( nY2 - nY1 + 1 ) );
                        }
                    }
                    while( nX1 < nWidth );
                }
                else
                {
                    do
                    {
                        for( nY = nY1, nSumR = nSumG = nSumB = 0; nY <= nY2; nY++ )
                        {
                            for( nX = nX1; nX <= nX2; nX++ )
                            {
                                const BitmapColor& rCol = pReadAcc->GetPaletteColor( (BYTE) pReadAcc->GetPixel( nY, nX ) );
                                nSumR += rCol.GetRed();
                                nSumG += rCol.GetGreen();
                                nSumB += rCol.GetBlue();
                            }
                        }

                        aCol.SetRed( (BYTE) ( nSumR * fArea_1 ) );
                        aCol.SetGreen( (BYTE) ( nSumG * fArea_1 ) );
                        aCol.SetBlue( (BYTE) ( nSumB * fArea_1 ) );

                        for( nY = nY1; nY <= nY2; nY++ )
                            for( nX = nX1; nX <= nX2; nX++ )
                                pWriteAcc->SetPixel( nY, nX, aCol );

                        nX1 += nTileWidth; nX2 += nTileWidth;

                        if( nX2 >= nWidth )
                        {
                            nX2 = nWidth - 1;
                            fArea_1 = 1.0 / ( ( nX2 - nX1 + 1 ) * ( nY2 - nY1 + 1 ) );
                        }
                    }
                    while( nX1 < nWidth );
                }

                nY1 += nTileHeight; nY2 += nTileHeight;

                if( nY2 >= nHeight )
                    nY2 = nHeight - 1;
            }
            while( nY1 < nHeight );

            bRet = TRUE;
        }

        ReleaseAccess( pReadAcc );

        if( pNewBmp )
        {
            pNewBmp->ReleaseAccess( pWriteAcc );

            if( bRet )
            {
                const MapMode   aMap( maPrefMapMode );
                const Size      aSize( maPrefSize );

                *this = *pNewBmp;

                maPrefMapMode = aMap;
                maPrefSize = aSize;
            }

            delete pNewBmp;
        }
    }
    else
        bRet = TRUE;

    return bRet;
}

// -----------------------------------------------------------------------------

struct PopArtEntry
{
    sal_uInt32  mnIndex;
    sal_uInt32  mnCount;
};

// ------------------------------------------------------------------------

extern "C" int __LOADONCALLAPI ImplPopArtCmpFnc( const void* p1, const void* p2 )
{
    int nRet;

    if( ( (PopArtEntry*) p1 )->mnCount < ( (PopArtEntry*) p2 )->mnCount )
        nRet = 1;
    else if( ( (PopArtEntry*) p1 )->mnCount == ( (PopArtEntry*) p2 )->mnCount )
        nRet = 0;
    else
        nRet = -1;

    return nRet;
}

// ------------------------------------------------------------------------

BOOL Bitmap::ImplPopArt( const BmpFilterParam* pFilterParam, const Link* pProgress )
{
    BOOL bRet = ( GetBitCount() > 8 ) ? Convert( BMP_CONVERSION_8BIT_COLORS ) : TRUE;

    if( bRet )
    {
        bRet = FALSE;

        BitmapWriteAccess* pWriteAcc = AcquireWriteAccess();

        if( pWriteAcc )
        {
            const long      nWidth = pWriteAcc->Width();
            const long      nHeight = pWriteAcc->Height();
            const ULONG     nEntryCount = 1 << pWriteAcc->GetBitCount();
            ULONG           n;
            PopArtEntry*    pPopArtTable = new PopArtEntry[ nEntryCount ];

            for( n = 0; n < nEntryCount; n++ )
            {
                PopArtEntry& rEntry = pPopArtTable[ n ];
                rEntry.mnIndex = (sal_uInt16) n;
                rEntry.mnCount = 0;
            }

            // get pixel count for each palette entry
            for( long nY = 0; nY < nHeight ; nY++ )
                for( long nX = 0; nX < nWidth; nX++ )
                    pPopArtTable[ pWriteAcc->GetPixel( nY, nX ).GetIndex() ].mnCount++;

            // sort table
            qsort( pPopArtTable, nEntryCount, sizeof( PopArtEntry ), ImplPopArtCmpFnc );

            // get last used entry
            ULONG nFirstEntry, nLastEntry;

            for( n = 0; n < nEntryCount; n++ )
                if( pPopArtTable[ n ].mnCount )
                    nLastEntry = n;

            // rotate palette (one entry)
            const BitmapColor aFirstCol( pWriteAcc->GetPaletteColor( pPopArtTable[ 0 ].mnIndex ) );
            for( nFirstEntry = 0; nFirstEntry < nLastEntry; nFirstEntry++ )
            {
                pWriteAcc->SetPaletteColor( pPopArtTable[ nFirstEntry ].mnIndex,
                                            pWriteAcc->GetPaletteColor( pPopArtTable[ nFirstEntry + 1 ].mnIndex ) );
            }
            pWriteAcc->SetPaletteColor( pPopArtTable[ nLastEntry ].mnIndex, aFirstCol );

            // cleanup
            delete[] pPopArtTable;
            ReleaseAccess( pWriteAcc );
            bRet = TRUE;
        }
    }

    return bRet;
}
