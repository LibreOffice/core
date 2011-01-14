/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#include <svtools/fltcall.hxx>

//============================ PBMReader ==================================

class PBMReader {

private:

    SvStream*           mpPBM;          // Die einzulesende PBM-Datei

    sal_Bool                mbStatus;
    sal_Bool                mbRemark;       // sal_False wenn sich stream in einem Kommentar befindet
    sal_Bool                mbRaw;          // RAW/ASCII MODE
    sal_uLong               mnMode;         // 0->PBM, 1->PGM, 2->PPM
    Bitmap              maBmp;
    BitmapWriteAccess*  mpAcc;
    sal_uLong               mnWidth, mnHeight;  // Bildausmass in Pixeln
    sal_uLong               mnCol;
    sal_uLong               mnMaxVal;           // maximaler wert in den
    sal_Bool                ImplCallback( sal_uInt16 nPercent );
    sal_Bool                ImplReadBody();
    sal_Bool                ImplReadHeader();

public:
                        PBMReader();
                        ~PBMReader();
    sal_Bool                ReadPBM( SvStream & rPBM, Graphic & rGraphic );
};

//=================== Methoden von PBMReader ==============================

PBMReader::PBMReader() :
    mbStatus    ( sal_True ),
    mbRemark    ( sal_False ),
    mbRaw       ( sal_True ),
    mpAcc       ( NULL )
{
}

PBMReader::~PBMReader()
{
}

sal_Bool PBMReader::ImplCallback( sal_uInt16 /*nPercent*/ )
{
/*
    if ( pCallback != NULL )
    {
        if ( ( (*pCallback)( pCallerData, nPercent ) ) == sal_True )
        {
            mpPBM->SetError( SVSTREAM_FILEFORMAT_ERROR );
            return sal_True;
        }
    }
*/
    return sal_False;
}

sal_Bool PBMReader::ReadPBM( SvStream & rPBM, Graphic & rGraphic )
{
    sal_uInt16 i;

    if ( rPBM.GetError() )
        return sal_False;

    mpPBM = &rPBM;
    mpPBM->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    // Kopf einlesen:

    if ( ( mbStatus = ImplReadHeader() ) == sal_False )
        return sal_False;

    if ( ( mnMaxVal == 0 ) || ( mnWidth == 0 ) || ( mnHeight == 0 ) )
        return sal_False;

    // 0->PBM, 1->PGM, 2->PPM
    switch ( mnMode )
    {
        case 0 :
            maBmp = Bitmap( Size( mnWidth, mnHeight ), 1 );
            if ( ( mpAcc = maBmp.AcquireWriteAccess() ) == sal_False )
                return sal_False;
            mpAcc->SetPaletteEntryCount( 2 );
            mpAcc->SetPaletteColor( 0, BitmapColor( 0xff, 0xff, 0xff ) );
            mpAcc->SetPaletteColor( 1, BitmapColor( 0x00, 0x00, 0x00 ) );
            break;

        case 1 :
            if ( mnMaxVal <= 1 )
                maBmp = Bitmap( Size( mnWidth, mnHeight ), 1);
            else if ( mnMaxVal <= 15 )
                maBmp = Bitmap( Size( mnWidth, mnHeight ), 4);
            else
                maBmp = Bitmap( Size( mnWidth, mnHeight ), 8);

            if ( ( mpAcc = maBmp.AcquireWriteAccess() ) == sal_False )
                return sal_False;
            mnCol = (sal_uInt16)mnMaxVal + 1;
            if ( mnCol > 256 )
                mnCol = 256;

            mpAcc->SetPaletteEntryCount( 256 );
            for ( i = 0; i < mnCol; i++ )
            {
                sal_uLong nCount = 255 * i / mnCol;
                mpAcc->SetPaletteColor( i, BitmapColor( (sal_uInt8)nCount, (sal_uInt8)nCount, (sal_uInt8)nCount ) );
            }
            break;
        case 2 :
            maBmp = Bitmap( Size( mnWidth, mnHeight ), 24 );
            if ( ( mpAcc = maBmp.AcquireWriteAccess() ) == sal_False )
                return sal_False;
            break;
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

sal_Bool PBMReader::ImplReadHeader()
{
    sal_uInt8   nID[ 2 ];
    sal_uInt8   nDat;
    sal_uInt8   nMax, nCount = 0;
    sal_Bool    bFinished = sal_False;

    *mpPBM >> nID[ 0 ] >> nID[ 1 ];
    if ( nID[ 0 ] != 'P' )
        return sal_False;
    mnMaxVal = mnWidth = mnHeight = 0;
    switch ( nID[ 1 ] )
    {
        case '1' :
            mbRaw = sal_False;
        case '4' :
            mnMode = 0;
            nMax = 2;               // number of parameters in Header
            mnMaxVal = 1;
            break;
        case '2' :
            mbRaw = sal_False;
        case '5' :
            mnMode = 1;
            nMax = 3;
            break;
        case '3' :
            mbRaw = sal_False;
        case '6' :
            mnMode = 2;
            nMax = 3;
            break;
        default:
            return sal_False;
    }
    while ( bFinished == sal_False )
    {
        if ( mpPBM->GetError() )
            return sal_False;

        *mpPBM >> nDat;

        if ( nDat == '#' )
        {
            mbRemark = sal_True;
            continue;
        }
        else if ( ( nDat == 0x0d ) || ( nDat == 0x0a ) )
        {
            mbRemark = sal_False;
            nDat = 0x20;
        }
        if ( mbRemark )
            continue;

        if ( ( nDat == 0x20 ) || ( nDat == 0x09 ) )
        {
            if ( ( nCount == 0 ) && mnWidth )
                nCount++;
            else if ( ( nCount == 1 ) && mnHeight )
            {
                if ( ++nCount == nMax )
                    bFinished = sal_True;
            }
            else if ( ( nCount == 2 ) && mnMaxVal )
            {
                bFinished = sal_True;
            }
            continue;
        }
        if ( ( nDat >= '0' ) && ( nDat <= '9' ) )
        {
            nDat -= '0';
            if ( nCount == 0 )
            {
                mnWidth *= 10;
                mnWidth += nDat;
            }
            else if ( nCount == 1 )
            {
                mnHeight *= 10;
                mnHeight += nDat;
            }
            else if ( nCount == 2 )
            {
                mnMaxVal *= 10;
                mnMaxVal += nDat;
            }
        }
        else
            return sal_False;
    }
    return mbStatus;
}

sal_Bool PBMReader::ImplReadBody()
{
    sal_Bool    bPara, bFinished = sal_False;
    sal_uInt8   nDat = 0, nCount;
    sal_uLong   nGrey, nRGB[3];
    sal_uLong   nWidth = 0;
    sal_uLong   nHeight = 0;
    signed char nShift = 0;

    if ( mbRaw )
    {
        switch ( mnMode )
        {

            // PBM
            case 0 :
                while ( nHeight != mnHeight )
                {
                    if ( mpPBM->IsEof() || mpPBM->GetError() )
                        return sal_False;

                    if ( --nShift < 0 )
                    {
                        *mpPBM >> nDat;
                        nShift = 7;
                    }
                    mpAcc->SetPixel( nHeight, nWidth, nDat >> nShift );
                    if ( ++nWidth == mnWidth )
                    {
                        nShift = 0;
                        nWidth = 0;
                        nHeight++;
                        ImplCallback( (sal_uInt16)( ( 100 * nHeight ) / mnHeight ) );   // processing output in percent
                    }
                }
                break;

            // PGM
            case 1 :
                while ( nHeight != mnHeight )
                {
                    if ( mpPBM->IsEof() || mpPBM->GetError() )
                        return sal_False;

                    *mpPBM >> nDat;
                    mpAcc->SetPixel( nHeight, nWidth++, nDat);

                    if ( nWidth == mnWidth )
                    {
                        nWidth = 0;
                        nHeight++;
                        ImplCallback( (sal_uInt16)( ( 100 * nHeight ) / mnHeight ) );   // processing output in percent
                    }
                }
                break;

            // PPM
            case 2 :
                while ( nHeight != mnHeight )
                {
                    if ( mpPBM->IsEof() || mpPBM->GetError() )
                        return sal_False;

                    sal_uInt8   nR, nG, nB;
                    sal_uLong   nRed, nGreen, nBlue;
                    *mpPBM >> nR >> nG >> nB;
                    nRed = 255 * nR / mnMaxVal;
                    nGreen = 255 * nG / mnMaxVal;
                    nBlue = 255 * nB / mnMaxVal;
                    mpAcc->SetPixel( nHeight, nWidth++, BitmapColor( (sal_uInt8)nRed, (sal_uInt8)nGreen, (sal_uInt8)nBlue ) );
                    if ( nWidth == mnWidth )
                    {
                        nWidth = 0;
                        nHeight++;
                        ImplCallback( (sal_uInt16) ( ( 100 * nHeight ) / mnHeight ) );  // processing output in percent
                    }
                }
                break;
        }
    }
    else switch  ( mnMode )
    {
        // PBM
        case 0 :
            while ( bFinished == sal_False )
            {
                if ( mpPBM->IsEof() || mpPBM->GetError() )
                    return sal_False;

                *mpPBM >> nDat;

                if ( nDat == '#' )
                {
                    mbRemark = sal_True;
                    continue;
                }
                else if ( ( nDat == 0x0d ) || ( nDat == 0x0a ) )
                {
                    mbRemark = sal_False;
                    continue;
                }
                if ( mbRemark || nDat == 0x20 || nDat == 0x09 )
                    continue;

                if ( nDat == '0' || nDat == '1' )
                {
                    mpAcc->SetPixel( nHeight, nWidth, (sal_uInt8)nDat-'0' );
                    nWidth++;
                    if ( nWidth == mnWidth )
                    {
                        nWidth = 0;
                        if ( ++nHeight == mnHeight )
                            bFinished = sal_True;
                        ImplCallback( (sal_uInt16) ( ( 100 * nHeight ) / mnHeight ) );  // processing output in percent
                    }
                }
                else
                    return sal_False;
            }
            break;

        // PGM
        case 1 :

            bPara = sal_False;
            nCount = 0;
            nGrey = 0;

            while ( bFinished == sal_False )
            {
                if ( nCount )
                {
                    nCount--;
                    if ( nGrey <= mnMaxVal )
                        nGrey = 255 * nGrey / mnMaxVal;
                        mpAcc->SetPixel( nHeight, nWidth++, (sal_uInt8)nGrey );
                    nGrey = 0;
                    if ( nWidth == mnWidth )
                    {
                        nWidth = 0;
                        if ( ++nHeight == mnHeight )
                            bFinished = sal_True;
                        ImplCallback( (sal_uInt16) ( ( 100 * nHeight ) / mnHeight ) );  // processing output in percent
                    }
                    continue;
                }

                if ( mpPBM->IsEof() || mpPBM->GetError() )
                    return sal_False;

                *mpPBM >> nDat;

                if ( nDat == '#' )
                {
                    mbRemark = sal_True;
                    if ( bPara )
                    {
                        bPara = sal_False;
                        nCount++;
                    }
                    continue;
                }
                else if ( ( nDat == 0x0d ) || ( nDat == 0x0a ) )
                {
                    mbRemark = sal_False;
                    if ( bPara )
                    {
                        bPara = sal_False;
                        nCount++;
                    }
                    continue;
                }

                if ( nDat == 0x20 || nDat == 0x09 )
                {
                    if ( bPara )
                    {
                        bPara = sal_False;
                        nCount++;
                    }
                    continue;
                }
                if ( nDat >= '0' && nDat <= '9' )
                {
                    bPara = sal_True;
                    nGrey *= 10;
                    nGrey += nDat-'0';
                    continue;
                }
                else
                    return sal_False;
            }
            break;



        // PPM
        case 2 :

            bPara = sal_False;
            nCount = 0;
            nRGB[ 0 ] = nRGB[ 1 ] = nRGB[ 2 ] = 0;

            while ( bFinished == sal_False )
            {
                if ( nCount == 3 )
                {
                    nCount = 0;
                    mpAcc->SetPixel( nHeight, nWidth++, BitmapColor( static_cast< sal_uInt8 >( ( nRGB[ 0 ] * 255 ) / mnMaxVal ),
                                                                     static_cast< sal_uInt8 >( ( nRGB[ 1 ] * 255 ) / mnMaxVal ),
                                                                     static_cast< sal_uInt8 >( ( nRGB[ 2 ] * 255 ) / mnMaxVal ) ) );
                    nCount = 0;
                    nRGB[ 0 ] = nRGB[ 1 ] = nRGB[ 2 ] = 0;
                    if ( nWidth == mnWidth )
                    {
                        nWidth = 0;
                        if ( ++nHeight == mnHeight )
                            bFinished = sal_True;
                        ImplCallback( (sal_uInt16) ( ( 100 * nHeight ) / mnHeight ) );  // processing output in percent
                    }
                    continue;
                }

                if ( mpPBM->IsEof() || mpPBM->GetError() )
                    return sal_False;

                *mpPBM >> nDat;

                if ( nDat == '#' )
                {
                    mbRemark = sal_True;
                    if ( bPara )
                    {
                        bPara = sal_False;
                        nCount++;
                    }
                    continue;
                }
                else if ( ( nDat == 0x0d ) || ( nDat == 0x0a ) )
                {
                    mbRemark = sal_False;
                    if ( bPara )
                    {
                        bPara = sal_False;
                        nCount++;
                    }
                    continue;
                }

                if ( nDat == 0x20 || nDat == 0x09 )
                {
                    if ( bPara )
                    {
                        bPara = sal_False;
                        nCount++;
                    }
                    continue;
                }
                if ( nDat >= '0' && nDat <= '9' )
                {
                    bPara = sal_True;
                    nRGB[ nCount ] *= 10;
                    nRGB[ nCount ] += nDat-'0';
                    continue;
                }
                else
                    return sal_False;
            }
            break;
    }
    return mbStatus;
}

//================== GraphicImport - die exportierte Funktion ================

extern "C" sal_Bool __LOADONCALLAPI GraphicImport(SvStream & rStream, Graphic & rGraphic, FilterConfigItem*, sal_Bool )
{
    PBMReader aPBMReader;

    return aPBMReader.ReadPBM( rStream, rGraphic );
}

