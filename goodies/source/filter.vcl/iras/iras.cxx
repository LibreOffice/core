/*************************************************************************
 *
 *  $RCSfile: iras.cxx,v $
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

#define RAS_TYPE_OLD            0x00000000      // supported formats by this filter
#define RAS_TYPE_STANDARD       0x00000001
#define RAS_TYPE_BYTE_ENCODED   0x00000002
#define RAS_TYPE_RGB_FORMAT     0x00000003

#define RAS_COLOR_NO_MAP        0x00000000
#define RAS_COLOR_RGB_MAP       0x00000001
#define RAS_COLOR_RAW_MAP       0x00000002

#define SUNRASTER_MAGICNUMBER   0x59a66a95

//============================ RASReader ==================================

class RASReader {

private:

    PFilterCallback pCallback;
    void * pCallerData;

    SvStream*           mpRAS;                  // Die einzulesende RAS-Datei

    BOOL                mbStatus;
    Bitmap              maBmp;
    BitmapWriteAccess*  mpAcc;
    ULONG               mnWidth, mnHeight;      // Bildausmass in Pixeln
    USHORT              mnDstBitsPerPix;
    USHORT              mnDstColors;
    ULONG               mnDepth, mnImageDatSize, mnType;
    ULONG               mnColorMapType, mnColorMapSize;
    BYTE                mnRepCount, mnRepVal;   // RLE Decoding
    BOOL                mbPalette;

    BOOL                ImplCallback( USHORT nPercent );
    BOOL                ImplReadBody();
    BOOL                ImplReadHeader();
    BYTE                ImplGetByte();

public:
                        RASReader();
                        ~RASReader();
    BOOL                ReadRAS( SvStream & rRAS, Graphic & rGraphic, PFilterCallback pcallback, void * pcallerdata );
};

//=================== Methoden von RASReader ==============================

RASReader::RASReader() :
    mpAcc       ( NULL ),
    mbStatus    ( TRUE ),
    mbPalette   ( FALSE ),
    mnRepCount  ( 0 )
{
}

RASReader::~RASReader()
{
}

//----------------------------------------------------------------------------

BOOL RASReader::ImplCallback( USHORT nPercent )
{
    if ( pCallback != NULL )
    {
        if ( ( (*pCallback)( pCallerData, nPercent ) ) == TRUE )
        {
            mpRAS->SetError( SVSTREAM_FILEFORMAT_ERROR );
            return TRUE;
        }
    }
    return FALSE;
}

//----------------------------------------------------------------------------

BOOL RASReader::ReadRAS( SvStream & rRAS, Graphic & rGraphic, PFilterCallback pcallback, void * pcallerdata)
{
    UINT32 nMagicNumber;

    if ( rRAS.GetError() ) return FALSE;

    pCallback = pcallback;
    pCallerData = pcallerdata;

    mpRAS = &rRAS;
    mpRAS->SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
    *mpRAS >> nMagicNumber;
    if ( nMagicNumber != SUNRASTER_MAGICNUMBER )
        return FALSE;

    // Kopf einlesen:

    if ( ( mbStatus = ImplReadHeader() ) == FALSE )
        return FALSE;

    maBmp = Bitmap( Size( mnWidth, mnHeight ), mnDstBitsPerPix );
    if ( ( mpAcc = maBmp.AcquireWriteAccess() ) == FALSE )
        return FALSE;

    if ( mnDstBitsPerPix <= 8 )     // paletten bildchen
    {
        if ( mnColorMapType == RAS_COLOR_RAW_MAP )      // RAW Colormap wird geskipped
        {
            ULONG nCurPos = mpRAS->Tell();
            mpRAS->Seek( nCurPos + mnColorMapSize );
        }
        else if ( mnColorMapType == RAS_COLOR_RGB_MAP ) // RGB koennen wir auslesen
        {
            mnDstColors = (USHORT)( mnColorMapSize / 3 );

            if ( ( 1 << mnDstBitsPerPix ) < mnDstColors )
                return FALSE;

            if ( ( mnDstColors >= 2 ) && ( ( mnColorMapSize % 3 ) == 0 ) )
            {
                mpAcc->SetPaletteEntryCount( mnDstColors );
                USHORT  i;
                BYTE    nRed[256], nGreen[256], nBlue[256];
                for ( i = 0; i < mnDstColors; i++ ) *mpRAS >> nRed[ i ];
                for ( i = 0; i < mnDstColors; i++ ) *mpRAS >> nGreen[ i ];
                for ( i = 0; i < mnDstColors; i++ ) *mpRAS >> nBlue[ i ];
                for ( i = 0; i < mnDstColors; i++ )
                {
                    mpAcc->SetPaletteColor( i, BitmapColor( nRed[ i ], nGreen[ i ], nBlue[ i ] ) );
                }
                mbPalette = TRUE;
            }
            else
                return FALSE;

        }
        else if ( mnColorMapType != RAS_COLOR_NO_MAP )  // alles andere ist kein standard
            return FALSE;

        if ( !mbPalette )
        {
            mnDstColors = 1 << mnDstBitsPerPix;
            mpAcc->SetPaletteEntryCount( mnDstColors );
            for ( USHORT i = 0; i < mnDstColors; i++ )
            {
                ULONG nCount = 255 - ( 255 * i / ( mnDstColors - 1 ) );
                mpAcc->SetPaletteColor( i, BitmapColor( (BYTE)nCount, (BYTE)nCount, (BYTE)nCount ) );
            }
        }
    }
    else
    {
        if ( mnColorMapType != RAS_COLOR_NO_MAP )   // when graphic has more then 256 colors and a color map we skip
        {                                           // the colormap
            ULONG nCurPos = mpRAS->Tell();
            mpRAS->Seek( nCurPos + mnColorMapSize );
        }
    }

    // Bitmap-Daten einlesen
    mbStatus = ImplReadBody();

    if ( mpAcc )
    {
        maBmp.ReleaseAccess( mpAcc ), mpAcc = NULL;
    }
    if ( mbStatus )
        rGraphic = maBmp;

    return mbStatus;
}

//----------------------------------------------------------------------------

BOOL RASReader::ImplReadHeader()
{
    *mpRAS >> mnWidth >> mnHeight >> mnDepth >> mnImageDatSize >>
        mnType >> mnColorMapType >> mnColorMapSize;

    if ( mnWidth == 0 || mnHeight == 0 )
        mbStatus = FALSE;

    switch ( mnDepth )
    {
        case 24 :
        case  8 :
        case  1 :
            mnDstBitsPerPix = (USHORT)mnDepth;
            break;
        case 32 :
            mnDstBitsPerPix = 24;
            break;

        default :
            mbStatus = FALSE;
    }

    switch ( mnType )
    {
        case RAS_TYPE_OLD :
        case RAS_TYPE_STANDARD :
        case RAS_TYPE_RGB_FORMAT :
        case RAS_TYPE_BYTE_ENCODED :            // this type will be supported later
            break;

        default:
            mbStatus = FALSE;
    }
    return mbStatus;
}

//----------------------------------------------------------------------------

BOOL RASReader::ImplReadBody()
{
    ULONG   x, y;
    BYTE    nDat, nRed, nGreen, nBlue;
    switch ( mnDstBitsPerPix )
    {
        case 1 :
            for ( y = 0; y < mnHeight; y++ )
            {
                for ( x = 0; x < mnWidth; x++ )
                {
                    if (!(x & 7))
                        nDat = ImplGetByte();
                    mpAcc->SetPixel ( y, x, nDat >> ( ( x & 7 ) ^ 7 ) );
                }
                if (!( ( x - 1 ) & 0x8 ) ) ImplGetByte();       // WORD ALIGNMENT ???
            }
            break;

        case 8 :
            for ( y = 0; y < mnHeight; y++ )
            {
                for ( x = 0; x < mnWidth; x++ )
                {
                    nDat = ImplGetByte();
                    mpAcc->SetPixel ( y, x, nDat );
                }
                if ( x & 1 ) ImplGetByte();                     // WORD ALIGNMENT ???
            }
            break;

        case 24 :
            switch ( mnDepth )
            {

                case 24 :
                    for ( y = 0; y < mnHeight; y++ )
                    {
                        for ( x = 0; x < mnWidth; x++ )
                        {
                            if ( mnType == RAS_TYPE_RGB_FORMAT )
                            {
                                nRed = ImplGetByte();
                                nGreen = ImplGetByte();
                                nBlue = ImplGetByte();
                            }
                            else
                            {
                                nBlue = ImplGetByte();
                                nGreen = ImplGetByte();
                                nRed = ImplGetByte();
                            }
                            mpAcc->SetPixel ( y, x, BitmapColor( nRed, nGreen, nBlue ) );
                        }
                        if ( x & 1 ) ImplGetByte();                     // WORD ALIGNMENT ???
                    }
                    break;

                case 32 :
                    for ( y = 0; y < mnHeight; y++ )
                    {
                        for ( x = 0; x < mnWidth; x++ )
                        {
                            nDat = ImplGetByte();               // pad byte > nil
                            if ( mnType == RAS_TYPE_RGB_FORMAT )
                            {
                                nRed = ImplGetByte();
                                nGreen = ImplGetByte();
                                nBlue = ImplGetByte();
                            }
                            else
                            {
                                nBlue = ImplGetByte();
                                nGreen = ImplGetByte();
                                nRed = ImplGetByte();
                            }
                            mpAcc->SetPixel ( y, x, BitmapColor( nRed, nGreen, nBlue ) );
                        }
                    }
                    break;
            }
            break;

        default:
            mbStatus = FALSE;
            break;
    }
    return mbStatus;
}

//----------------------------------------------------------------------------

BYTE RASReader::ImplGetByte()
{
    BYTE nRetVal;
    if ( mnType != RAS_TYPE_BYTE_ENCODED )
    {
        *mpRAS >> nRetVal;
        return nRetVal;
    }
    else
    {
        if ( mnRepCount )
        {
            mnRepCount--;
            return mnRepVal;
        }
        else
        {
            *mpRAS >> nRetVal;
            if ( nRetVal != 0x80 )
                return nRetVal;
            *mpRAS >> nRetVal;
            if ( nRetVal == 0 )
                return 0x80;
            mnRepCount = nRetVal    ;
            *mpRAS >> mnRepVal;
            return mnRepVal;
        }
    }
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
    RASReader aRASReader;

    return aRASReader.ReadRAS( rStream, rGraphic, pCallback, pCallerData );
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

