/*************************************************************************
 *
 *  $RCSfile: ipsd.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:16 $
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

#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#include <svtools/fltcall.hxx>

//============================ PSDReader ==================================

#define PSD_BITMAP          0
#define PSD_GRAYSCALE       1
#define PSD_INDEXED         2
#define PSD_RGB             3
#define PSD_CMYK            4
#define PSD_MULTICHANNEL    7
#define PSD_DUOTONE         8
#define PSD_LAB             9

typedef struct
{
    UINT32  nSignature;
    UINT16  nVersion;
    UINT32  nPad1;
    UINT16  nPad2;
    UINT16  nChannels;
    UINT32  nRows;
    UINT32  nColumns;
    UINT16  nDepth;
    UINT16  nMode;

} PSDFileHeader;

class PSDReader {

private:

    PFilterCallback     pCallback;
    void*               pCallerData;

    SvStream*           mpPSD;          // Die einzulesende PSD-Datei
    PSDFileHeader*      mpFileHeader;

    BOOL                mbStatus;
    BOOL                mbTransparent;
    Bitmap              maBmp;
    Bitmap              maMaskBmp;
    BitmapReadAccess*   mpReadAcc;
    BitmapWriteAccess*  mpWriteAcc;
    BitmapWriteAccess*  mpMaskWriteAcc;
    USHORT              mnDestBitDepth;
    BOOL                mbCompression;  // RLE decoding
    BYTE*               mpPalette;

    BOOL                ImplCallback( USHORT nPercent );
    BOOL                ImplReadBody();
    BOOL                ImplReadHeader();

public:
                        PSDReader();
                        ~PSDReader();
    BOOL                ReadPSD( SvStream & rPSD, Graphic & rGraphic, PFilterCallback pcallback, void * pcallerdata );
};

//=================== Methoden von PSDReader ==============================

PSDReader::PSDReader() :
    mpWriteAcc      ( NULL ),
    mpMaskWriteAcc  ( NULL ),
    mpReadAcc       ( NULL ),
    mpFileHeader    ( NULL ),
    mpPalette       ( NULL ),
    mbStatus        ( TRUE ),
    mbTransparent   ( FALSE )
{
}

PSDReader::~PSDReader()
{
    delete mpPalette;
    delete mpFileHeader;
}

// ------------------------------------------------------------------------

BOOL PSDReader::ImplCallback( USHORT nPercent )
{
    if ( pCallback != NULL )
    {
        if ( ( (*pCallback)( pCallerData, nPercent ) ) == TRUE )
        {
            mpPSD->SetError( SVSTREAM_FILEFORMAT_ERROR );
            return TRUE;
        }
    }
    return FALSE;
}

// ------------------------------------------------------------------------

BOOL PSDReader::ReadPSD( SvStream & rPSD, Graphic & rGraphic, PFilterCallback pcallback, void * pcallerdata)
{
    if ( rPSD.GetError() )
        return FALSE;

    pCallback = pcallback;
    pCallerData = pcallerdata;

    mpPSD = &rPSD;
    mpPSD->SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );

    // Kopf einlesen:

    if ( ImplReadHeader() == FALSE )
        return FALSE;

    maBmp = Bitmap( Size( mpFileHeader->nColumns, mpFileHeader->nRows ), mnDestBitDepth );
    if ( ( mpWriteAcc = maBmp.AcquireWriteAccess() ) == NULL )
        mbStatus = FALSE;
    if ( ( mpReadAcc = maBmp.AcquireReadAccess() ) == NULL )
        mbStatus = FALSE;
    if ( mbTransparent && mbStatus )
    {
        maMaskBmp = Bitmap( Size( mpFileHeader->nColumns, mpFileHeader->nRows ), 1 );
        if ( ( mpMaskWriteAcc = maMaskBmp.AcquireWriteAccess() ) == NULL )
            mbStatus = FALSE;
    }
    if ( mpPalette && mbStatus )
    {
        mpWriteAcc->SetPaletteEntryCount( 256 );
        for ( USHORT i = 0; i < 256; i++ )
        {
            mpWriteAcc->SetPaletteColor( i, Color( mpPalette[ i ], mpPalette[ i + 256 ], mpPalette[ i + 512 ] ) );
        }
    }
    // Bitmap-Daten einlesen
    if ( mbStatus && ImplReadBody() )
    {
        if ( mbTransparent )
            rGraphic = Graphic( BitmapEx( maBmp, maMaskBmp ) );
        else
            rGraphic = maBmp;
    }
    else
        mbStatus = FALSE;
    if ( mpWriteAcc )
        maBmp.ReleaseAccess( mpWriteAcc );
    if ( mpReadAcc )
        maBmp.ReleaseAccess( mpReadAcc );
    if ( mpMaskWriteAcc )
        maMaskBmp.ReleaseAccess( mpMaskWriteAcc );
    return mbStatus;
}

// ------------------------------------------------------------------------

BOOL PSDReader::ImplReadHeader()
{
    UINT16  nCompression;
    UINT32  nColorLength, nResourceLength, nLayerMaskLength;

    mpFileHeader = new PSDFileHeader;

    if ( !mpFileHeader )
        return FALSE;

    *mpPSD >> mpFileHeader->nSignature >> mpFileHeader->nVersion >> mpFileHeader->nPad1 >>
        mpFileHeader->nPad2 >> mpFileHeader->nChannels >> mpFileHeader->nRows >>
            mpFileHeader->nColumns >> mpFileHeader->nDepth >> mpFileHeader->nMode;

    if ( ( mpFileHeader->nSignature != 0x38425053 ) || ( mpFileHeader->nVersion != 1 ) )
        return FALSE;

    if ( mpFileHeader->nRows == 0 || mpFileHeader->nColumns == 0 )
        return FALSE;

    if ( ( mpFileHeader->nRows > 30000 ) || ( mpFileHeader->nColumns > 30000 ) )
        return FALSE;

    UINT16 nDepth = mpFileHeader->nDepth;
    if (!( ( nDepth == 1 ) || ( nDepth == 8 ) || ( nDepth == 16 ) ) )
        return FALSE;

    mnDestBitDepth = ( nDepth == 16 ) ? 8 : nDepth;

    *mpPSD >> nColorLength;
    if ( mpFileHeader->nMode == PSD_CMYK )
    {
        switch ( mpFileHeader->nChannels )
        {
            case 5 :
                mbTransparent = TRUE;
            case 4 :
                mnDestBitDepth = 24;
            break;
            default :
                return FALSE;
        }
    }
    else switch ( mpFileHeader->nChannels )
    {
        case 2 :
            mbTransparent = TRUE;
        case 1 :
            break;
        case 4 :
            mbTransparent = TRUE;
        case 3 :
            mnDestBitDepth = 24;
            break;
        default:
            return FALSE;
    }

    switch ( mpFileHeader->nMode )
    {
        case PSD_BITMAP :
        {
            if ( nColorLength || ( nDepth != 1 ) )
                return FALSE;
        }
        break;

        case PSD_INDEXED :
        {
            if ( nColorLength != 768 )      // we need the color map
                return FALSE;
            mpPalette = new BYTE[ 768 ];
            if ( mpPalette == NULL )
                return FALSE;
            mpPSD->Read( mpPalette, 768 );
        }
        break;

        case PSD_DUOTONE :                  // we'll handle the doutone color like a normal grayscale picture
            mpPSD->SeekRel( nColorLength );
            nColorLength = 0;
        case PSD_GRAYSCALE :
        {
            if ( nColorLength )
                return FALSE;
            mpPalette = new BYTE[ 768 ];
            if ( mpPalette == NULL )
                return FALSE;
            for ( USHORT i = 0; i < 256; i++ )
            {
                mpPalette[ i ] = mpPalette[ i + 256 ] = mpPalette[ i + 512 ] = (BYTE)i;
            }
        }
        break;

        case PSD_CMYK :
        case PSD_RGB :
        case PSD_MULTICHANNEL :
        case PSD_LAB :
        {
            if ( nColorLength )     // color table is not supported by the other graphic modes
                return FALSE;
        }
        break;

        default:
        {
            return FALSE;
        }
        break;
    }

    *mpPSD >> nResourceLength;
    mpPSD->SeekRel( nResourceLength );

    *mpPSD >> nLayerMaskLength;
    mpPSD->SeekRel( nLayerMaskLength );

    *mpPSD >> nCompression;
    if ( nCompression == 0 )
    {
        mbCompression = FALSE;
    }
    else if ( nCompression == 1 )
    {
        mpPSD->SeekRel( ( mpFileHeader->nRows * mpFileHeader->nChannels ) << 1 );
        mbCompression = TRUE;
    }
    else
        return FALSE;

    return TRUE;
}

// ------------------------------------------------------------------------

BOOL PSDReader::ImplReadBody()
{
    ULONG       nX, nY;
    char        nRunCount = 0;
    char        nBitCount = -1;
    BYTE        nDat, nDummy, nRed, nGreen, nBlue;
    BitmapColor aBitmapColor;
    nX = nY = 0;

    switch ( mnDestBitDepth )
    {
        case 1 :
        {
            while ( nY < mpFileHeader->nRows )
            {
                if ( nBitCount == -1 )
                {
                    if ( mbCompression )    // else nRunCount = 0 -> so we use only single raw packets
                        *mpPSD >> nRunCount;
                }
                if ( nRunCount & 0x80 )     // a run length packet
                {
                    if ( nBitCount == -1 )  // bits left in nDat ?
                    {
                        *mpPSD >> nDat;
                        nDat ^= 0xff;
                        nBitCount = 7;
                    }
                    for ( USHORT i = 0; i < ( -nRunCount + 1 ); i++ )
                    {
                        mpWriteAcc->SetPixel( nY, nX, (BYTE)nDat >> nBitCount-- );
                        if ( ++nX == mpFileHeader->nColumns )
                        {
                            nX = 0;
                            nY++;
                            nBitCount = -1;
                        }
                    }
                }
                else                        // a raw packet
                {
                    for ( USHORT i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                    {
                        if ( nBitCount == -1 )  // bits left in nDat ?
                        {
                            *mpPSD >> nDat;
                            nDat ^= 0xff;
                            nBitCount = 7;
                        }
                        mpWriteAcc->SetPixel( nY, nX, (BYTE)nDat >> nBitCount-- );
                        if ( ++nX == mpFileHeader->nColumns )
                        {
                            nX = 0;
                            nY++;
                            nBitCount = -1;
                        }
                    }
                }
            }
        }
        break;

        case 8 :
        {
            while ( nY < mpFileHeader->nRows )
            {
                if ( mbCompression )        // else nRunCount = 0 -> so we use only single raw packets
                    *mpPSD >> nRunCount;

                if ( nRunCount & 0x80 )     // a run length packet
                {
                    *mpPSD >> nDat;
                    if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                        *mpPSD >> nDummy;
                    for ( USHORT i = 0; i < ( -nRunCount + 1 ); i++ )
                    {
                        mpWriteAcc->SetPixel( nY, nX, (BYTE)nDat );
                        if ( ++nX == mpFileHeader->nColumns )
                        {
                            nX = 0;
                            nY++;
                        }
                    }
                }
                else                        // a raw packet
                {
                    for ( USHORT i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                    {
                        *mpPSD >> nDat;
                        if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                            *mpPSD >> nDummy;
                        mpWriteAcc->SetPixel( nY, nX, (BYTE)nDat );
                        if ( ++nX == mpFileHeader->nColumns )
                        {
                            nX = 0;
                            nY++;
                        }
                    }
                }
            }
        }
        break;

        case 24 :
        {

            // the psd format is in plain order (RRRR GGGG BBBB) so we have to set each pixel three times
            // maybe the format is CCCC MMMM YYYY KKKK

            while ( nY < mpFileHeader->nRows )
            {
                if ( mbCompression )        // else nRunCount = 0 -> so we use only single raw packets
                    *mpPSD >> nRunCount;

                if ( nRunCount & 0x80 )     // a run length packet
                {
                    *mpPSD >> nRed;
                    if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                        *mpPSD >> nDummy;
                    for ( USHORT i = 0; i < ( -nRunCount + 1 ); i++ )
                    {
                        mpWriteAcc->SetPixel( nY, nX, BitmapColor( nRed, (BYTE)0, (BYTE)0 ) );
                        if ( ++nX == mpFileHeader->nColumns )
                        {
                            nX = 0;
                            nY++;
                        }
                    }
                }
                else                        // a raw packet
                {
                    for ( USHORT i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                    {
                        *mpPSD >> nRed;
                        if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                            *mpPSD >> nDummy;
                        mpWriteAcc->SetPixel( nY, nX, BitmapColor( nRed, (BYTE)0, (BYTE)0 ) );
                        if ( ++nX == mpFileHeader->nColumns )
                        {
                            nX = 0;
                            nY++;
                        }
                    }
                }
            }
            nY = 0;
            while ( nY < mpFileHeader->nRows )
            {
                if ( mbCompression )
                    *mpPSD >> nRunCount;
                if ( nRunCount & 0x80 )     // a run length packet
                {
                    *mpPSD >> nGreen;
                    if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                        *mpPSD >> nDummy;
                    for ( USHORT i = 0; i < ( -nRunCount + 1 ); i++ )
                    {
                        aBitmapColor = mpReadAcc->GetPixel( nY, nX );
                        mpWriteAcc->SetPixel( nY, nX, BitmapColor( aBitmapColor.GetRed(), nGreen, aBitmapColor.GetBlue() ) );
                        if ( ++nX == mpFileHeader->nColumns )
                        {
                            nX = 0;
                            nY++;
                        }
                    }
                }
                else                        // a raw packet
                {
                    for ( USHORT i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                    {
                        *mpPSD >> nGreen;
                        if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                            *mpPSD >> nDummy;
                        aBitmapColor = mpReadAcc->GetPixel( nY, nX );
                        mpWriteAcc->SetPixel( nY, nX, BitmapColor( aBitmapColor.GetRed(), nGreen, aBitmapColor.GetBlue() ) );
                        if ( ++nX == mpFileHeader->nColumns )
                        {
                            nX = 0;
                            nY++;
                        }
                    }
                }
            }
            nY = 0;
            while ( nY < mpFileHeader->nRows )
            {
                if ( mbCompression )
                    *mpPSD >> nRunCount;
                if ( nRunCount & 0x80 )     // a run length packet
                {
                    *mpPSD >> nBlue;
                    if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                        *mpPSD >> nDummy;
                    for ( USHORT i = 0; i < ( -nRunCount + 1 ); i++ )
                    {
                        aBitmapColor = mpReadAcc->GetPixel( nY, nX );
                        mpWriteAcc->SetPixel( nY, nX, BitmapColor( aBitmapColor.GetRed(), aBitmapColor.GetGreen(), nBlue ) );
                        if ( ++nX == mpFileHeader->nColumns )
                        {
                            nX = 0;
                            nY++;
                        }
                    }
                }
                else                        // a raw packet
                {
                    for ( USHORT i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                    {
                        *mpPSD >> nBlue;
                        if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                            *mpPSD >> nDummy;
                        aBitmapColor = mpReadAcc->GetPixel( nY, nX );
                        mpWriteAcc->SetPixel( nY, nX, BitmapColor( aBitmapColor.GetRed(), aBitmapColor.GetGreen(), nBlue ) );
                        if ( ++nX == mpFileHeader->nColumns )
                        {
                            nX = 0;
                            nY++;
                        }
                    }
                }
            }
            if ( mpFileHeader->nMode == PSD_CMYK )
            {
                UINT32  nBlack, nBlackMax = 0;
                BYTE*   pBlack = new BYTE[ mpFileHeader->nRows * mpFileHeader->nColumns ];
                nY = 0;
                while ( nY < mpFileHeader->nRows )
                {
                    if ( mbCompression )        // else nRunCount = 0 -> so we use only single raw packets
                        *mpPSD >> nRunCount;

                    if ( nRunCount & 0x80 )     // a run length packet
                    {
                        *mpPSD >> nDat;

                        if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                            *mpPSD >> nDummy;

                        for ( USHORT i = 0; i < ( -nRunCount + 1 ); i++ )
                        {
                            nBlack = (BYTE)mpReadAcc->GetPixel( nY, nX ).GetRed() + nDat;
                            if ( nBlack > nBlackMax )
                                nBlackMax = nBlack;
                            nBlack = (BYTE)mpReadAcc->GetPixel( nY, nX ).GetGreen() + nDat;
                            if ( nBlack > nBlackMax )
                                nBlackMax = nBlack;
                            nBlack = (BYTE)mpReadAcc->GetPixel( nY, nX ).GetBlue() + nDat;
                            if ( nBlack > nBlackMax )
                                nBlackMax = nBlack;
                            pBlack[ nX + nY * mpFileHeader->nColumns ] = nDat ^ 0xff;
                            if ( ++nX == mpFileHeader->nColumns )
                            {
                                nX = 0;
                                nY++;
                            }
                        }
                    }
                    else                        // a raw packet
                    {
                        for ( USHORT i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                        {
                            *mpPSD >> nDat;

                            if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                                *mpPSD >> nDummy;
                            nBlack = (BYTE)mpReadAcc->GetPixel( nY, nX ).GetRed() + nDat;
                            if ( nBlack > nBlackMax )
                                nBlackMax = nBlack;
                            nBlack = (BYTE)mpReadAcc->GetPixel( nY, nX ).GetGreen() + nDat;
                            if ( nBlack > nBlackMax )
                                nBlackMax = nBlack;
                            nBlack = (BYTE)mpReadAcc->GetPixel( nY, nX ).GetBlue() + nDat;
                            if ( nBlack > nBlackMax )
                                nBlackMax = nBlack;
                            pBlack[ nX + nY * mpFileHeader->nColumns ] = nDat ^ 0xff;
                            if ( ++nX == mpFileHeader->nColumns )
                            {
                                nX = 0;
                                nY++;
                            }
                        }
                    }
                }

                for ( nY = 0; nY < mpFileHeader->nRows; nY++ )
                {
                    for ( nX = 0; nX < mpFileHeader->nColumns; nX++ )
                    {
                        INT32 nDAT = pBlack[ nX + nY * mpFileHeader->nColumns ] * ( nBlackMax - 256 ) / 0x1ff;

                        aBitmapColor = mpReadAcc->GetPixel( nY, nX );
                        BYTE cR = (BYTE) MinMax( aBitmapColor.GetRed() - nDAT, 0L, 255L );
                        BYTE cG = (BYTE) MinMax( aBitmapColor.GetGreen() - nDAT, 0L, 255L );
                        BYTE cB = (BYTE) MinMax( aBitmapColor.GetBlue() - nDAT, 0L, 255L );
                        mpWriteAcc->SetPixel( nY, nX, BitmapColor( cR, cG, cB ) );
                    }
                }
                delete pBlack;
            }
        }
        break;
    }

    if ( mbTransparent )
    {
        // the psd is 24 or 8 bit grafix + alphachannel

        nY = nX = 0;
        while ( nY < mpFileHeader->nRows )
        {
            if ( mbCompression )        // else nRunCount = 0 -> so we use only single raw packets
                *mpPSD >> nRunCount;

            if ( nRunCount & 0x80 )     // a run length packet
            {
                *mpPSD >> nDat;
                if ( nDat )
                    nDat = 0;
                else
                    nDat = 1;
                if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                    *mpPSD >> nDummy;
                for ( USHORT i = 0; i < ( -nRunCount + 1 ); i++ )
                {
                    mpMaskWriteAcc->SetPixel( nY, nX, (BYTE)nDat );
                    if ( ++nX == mpFileHeader->nColumns )
                    {
                        nX = 0;
                        nY++;
                    }
                }
            }
            else                        // a raw packet
            {
                for ( USHORT i = 0; i < ( ( nRunCount & 0x7f ) + 1 ); i++ )
                {
                    *mpPSD >> nDat;
                    if ( nDat )
                        nDat = 0;
                    else
                        nDat = 1;
                    if ( mpFileHeader->nDepth == 16 )   // 16 bit depth is to be skipped
                        *mpPSD >> nDummy;
                    mpMaskWriteAcc->SetPixel( nY, nX, (BYTE)nDat );
                    if ( ++nX == mpFileHeader->nColumns )
                    {
                        nX = 0;
                        nY++;
                    }
                }
            }
        }
    }
    return TRUE;
}

//================== GraphicImport - die exportierte Funktion ================

#ifdef WNT
extern "C" BOOL _cdecl GraphicImport(SvStream & rStream, Graphic & rGraphic,
                              PFilterCallback pCallback, void * pCallerData,
                              Config *, BOOL)
#else
extern "C" BOOL GraphicImport(SvStream & rStream, Graphic & rGraphic,
                              PFilterCallback pCallback, void * pCallerData,
                              Config *, BOOL)
#endif
{
    PSDReader aPSDReader;

    return aPSDReader.ReadPSD( rStream, rGraphic, pCallback, pCallerData );
}

//================== ein bischen Muell fuer Windows ==========================

#pragma hdrstop

#ifdef WIN

static HINSTANCE hDLLInst = 0;      // HANDLE der DLL

extern "C" int CALLBACK LibMain( HINSTANCE hDLL, WORD, WORD nHeap, LPSTR )
{
#ifndef WNT
    if ( nHeap )
        UnlockData( 0 );
#endif

    hDLLInst = hDLL;

    return TRUE;
}

extern "C" int CALLBACK WEP( int )
{
    return 1;
}

#endif
