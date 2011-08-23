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

//============================ XPMWriter ==================================

class XPMWriter {

private:

    SvStream*			mpOStm; 			// Die auszugebende XPM-Datei
    USHORT				mpOStmOldModus;

    BOOL				mbStatus;
    BOOL				mbTrans;
    BitmapReadAccess*	mpAcc;
    ULONG				mnWidth, mnHeight;	// Bildausmass in Pixeln
    USHORT				mnColors;

    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > xStatusIndicator;

    void				ImplCallback( USHORT nPercent );
    BOOL				ImplWriteHeader();
    void				ImplWritePalette();
    void				ImplWriteColor( USHORT );
    void				ImplWriteBody();
    void				ImplWriteNumber( sal_Int32 );
    void				ImplWritePixel( ULONG );

public:
                        XPMWriter();
                        ~XPMWriter();

    BOOL				WriteXPM( const Graphic& rGraphic, SvStream& rXPM, FilterConfigItem* pFilterConfigItem );
};

//=================== Methoden von XPMWriter ==============================

XPMWriter::XPMWriter() :
    mbStatus	( TRUE ),
    mbTrans		( FALSE ),
    mpAcc		( NULL )
{
}

// ------------------------------------------------------------------------

XPMWriter::~XPMWriter()
{
}

// ------------------------------------------------------------------------

void XPMWriter::ImplCallback( USHORT nPercent )
{
    if ( xStatusIndicator.is() )
    {
        if ( nPercent <= 100 )
            xStatusIndicator->setValue( nPercent );
    }
}

//	------------------------------------------------------------------------

BOOL XPMWriter::WriteXPM( const Graphic& rGraphic, SvStream& rXPM, FilterConfigItem* pFilterConfigItem)
{
    Bitmap	aBmp;

    mpOStm = &rXPM;

    if ( pFilterConfigItem )
    {
        xStatusIndicator = pFilterConfigItem->GetStatusIndicator();
        if ( xStatusIndicator.is() )
        {
            rtl::OUString aMsg;
            xStatusIndicator->start( aMsg, 100 );
        }
    }

    BitmapEx	aBmpEx( rGraphic.GetBitmapEx() );
    aBmp = aBmpEx.GetBitmap();

    if ( rGraphic.IsTransparent() )					// event. transparente Farbe erzeugen
    {
        mbTrans = TRUE;
        if ( aBmp.GetBitCount() >= 8 )				// wenn noetig Bild auf 8 bit konvertieren
            aBmp.Convert( BMP_CONVERSION_8BIT_TRANS );
        else
            aBmp.Convert( BMP_CONVERSION_4BIT_TRANS );
        aBmp.Replace( aBmpEx.GetMask(), BMP_COL_TRANS );
    }
    else
    {
        if ( aBmp.GetBitCount() > 8 )				// wenn noetig Bild auf 8 bit konvertieren
            aBmp.Convert( BMP_CONVERSION_8BIT_COLORS );
    }
    mpAcc = aBmp.AcquireReadAccess();
    if ( mpAcc )
    {
        mnColors = mpAcc->GetPaletteEntryCount();
        mpOStmOldModus = mpOStm->GetNumberFormatInt();
        mpOStm->SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );

        if ( ImplWriteHeader() )
        {
            ImplWritePalette();
            ImplWriteBody();
            *mpOStm << "\x22XPMENDEXT\x22\x0a};";
        }
        aBmp.ReleaseAccess( mpAcc );
    }
    else
        mbStatus = FALSE;

    mpOStm->SetNumberFormatInt( mpOStmOldModus );

    if ( xStatusIndicator.is() )
        xStatusIndicator->end();

    return mbStatus;
}

// ------------------------------------------------------------------------

BOOL XPMWriter::ImplWriteHeader()
{
    mnWidth = mpAcc->Width();
    mnHeight = mpAcc->Height();
    if ( mnWidth && mnHeight && mnColors )
    {
        *mpOStm << "/* XPM */\x0astatic char * image[] = \x0a{\x0a\x22";
        ImplWriteNumber( mnWidth );
        *mpOStm << (BYTE)32;
        ImplWriteNumber( mnHeight );
        *mpOStm << (BYTE)32;
        ImplWriteNumber( mnColors );
        *mpOStm << (BYTE)32;
        ImplWriteNumber( ( mnColors > 26 ) ? 2 : 1 );
        *mpOStm << "\x22,\x0a";
    }
    else mbStatus = FALSE;
    return mbStatus;
}

// ------------------------------------------------------------------------

void XPMWriter::ImplWritePalette()
{
    USHORT nTransIndex = 0xffff;

    if ( mbTrans )
        nTransIndex = mpAcc->GetBestMatchingColor( BMP_COL_TRANS );
    for ( USHORT i = 0; i < mnColors; i++ )
    {
        *mpOStm << "\x22";
        ImplWritePixel( i );
        *mpOStm << (BYTE)32;
        if ( nTransIndex != i )
        {
            ImplWriteColor( i );
            *mpOStm << "\x22,\x0a";
        }
        else
            *mpOStm << "c none\x22,\x0a";
    }
}

// ------------------------------------------------------------------------

void XPMWriter::ImplWriteBody()
{
    for ( ULONG y = 0; y < mnHeight; y++ )
    {
        ImplCallback( (USHORT)( ( 100 * y ) / mnHeight ) );			// processing output in percent
        *mpOStm << (BYTE)0x22;
        for ( ULONG x = 0; x < mnWidth; x++ )
        {
            ImplWritePixel( (BYTE)(mpAcc->GetPixel( y, x ) ) );
        }
        *mpOStm << "\x22,\x0a";
    }
}

// ------------------------------------------------------------------------
// eine Dezimalzahl im ASCII format wird in den Stream geschrieben

void XPMWriter::ImplWriteNumber( sal_Int32 nNumber )
{
    const ByteString aNum( ByteString::CreateFromInt32( nNumber ) );

    for( sal_Int16 n = 0UL, nLen = aNum.Len(); n < nLen; n++  )
        *mpOStm << aNum.GetChar( n );

}

// ------------------------------------------------------------------------

void XPMWriter::ImplWritePixel( ULONG nCol )
{
    if ( mnColors > 26 )
    {
        BYTE nDiff = (BYTE) ( nCol / 26 );
        *mpOStm << (BYTE)( nDiff + 'A' );
        *mpOStm << (BYTE)( nCol - ( nDiff*26 ) + 'A' );
    }
    else
        *mpOStm << (BYTE)( nCol + 'A' );
}

// ------------------------------------------------------------------------
// ein Farbwert wird im Hexadezimalzahlformat in den Stream geschrieben
void XPMWriter::ImplWriteColor( USHORT nNumber )
{
    ULONG	nTmp;
    BYTE	j;

    *mpOStm << "c #";	// # zeigt einen folgenden Hexwert an
    const BitmapColor& rColor = mpAcc->GetPaletteColor( nNumber );
    nTmp = ( rColor.GetRed() << 16 ) | ( rColor.GetGreen() << 8 ) | rColor.GetBlue();
    for ( signed char i = 20; i >= 0 ; i-=4 )
    {
        if ( ( j = (BYTE)( nTmp >> i ) & 0xf ) > 9 )
            j += 'A' - 10;
        else
            j += '0';
        *mpOStm << j;
    }
}

// ------------------------------------------------------------------------

// ---------------------
// - exported function -
// ---------------------

extern "C" BOOL __LOADONCALLAPI GraphicExport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pFilterConfigItem, BOOL )
{
    XPMWriter aXPMWriter;

    return aXPMWriter.WriteXPM( rGraphic, rStream, pFilterConfigItem );
}

#ifndef GCC
#endif

// ---------------
// - Win16 trash -
// ---------------

#ifdef WIN

static HINSTANCE hDLLInst = 0;

extern "C" int CALLBACK LibMain( HINSTANCE hDLL, WORD, WORD nHeap, LPSTR )
{
    if ( nHeap )
        UnlockData( 0 );

    hDLLInst = hDLL;

    return TRUE;
}

// ------------------------------------------------------------------------

extern "C" int CALLBACK WEP( int )
{
    return 1;
}

#endif
