/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#include <svtools/fltcall.hxx>

//============================ XPMWriter ==================================

class XPMWriter {

private:

    SvStream*           mpOStm;             // Die auszugebende XPM-Datei
    sal_uInt16              mpOStmOldModus;

    sal_Bool                mbStatus;
    sal_Bool                mbTrans;
    BitmapReadAccess*   mpAcc;
    sal_uLong               mnWidth, mnHeight;  // Bildausmass in Pixeln
    sal_uInt16              mnColors;

    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > xStatusIndicator;

    void                ImplCallback( sal_uInt16 nPercent );
    sal_Bool                ImplWriteHeader();
    void                ImplWritePalette();
    void                ImplWriteColor( sal_uInt16 );
    void                ImplWriteBody();
    void                ImplWriteNumber( sal_Int32 );
    void                ImplWritePixel( sal_uLong );

public:
                        XPMWriter();
                        ~XPMWriter();

    sal_Bool                WriteXPM( const Graphic& rGraphic, SvStream& rXPM, FilterConfigItem* pFilterConfigItem );
};

//=================== Methoden von XPMWriter ==============================

XPMWriter::XPMWriter() :
    mbStatus    ( sal_True ),
    mbTrans     ( sal_False ),
    mpAcc       ( NULL )
{
}

// ------------------------------------------------------------------------

XPMWriter::~XPMWriter()
{
}

// ------------------------------------------------------------------------

void XPMWriter::ImplCallback( sal_uInt16 nPercent )
{
    if ( xStatusIndicator.is() )
    {
        if ( nPercent <= 100 )
            xStatusIndicator->setValue( nPercent );
    }
}

//  ------------------------------------------------------------------------

sal_Bool XPMWriter::WriteXPM( const Graphic& rGraphic, SvStream& rXPM, FilterConfigItem* pFilterConfigItem)
{
    Bitmap  aBmp;

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

    BitmapEx    aBmpEx( rGraphic.GetBitmapEx() );
    aBmp = aBmpEx.GetBitmap();

    if ( rGraphic.IsTransparent() )                 // event. transparente Farbe erzeugen
    {
        mbTrans = sal_True;
        if ( aBmp.GetBitCount() >= 8 )              // wenn noetig Bild auf 8 bit konvertieren
            aBmp.Convert( BMP_CONVERSION_8BIT_TRANS );
        else
            aBmp.Convert( BMP_CONVERSION_4BIT_TRANS );
        aBmp.Replace( aBmpEx.GetMask(), BMP_COL_TRANS );
    }
    else
    {
        if ( aBmp.GetBitCount() > 8 )               // wenn noetig Bild auf 8 bit konvertieren
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
        mbStatus = sal_False;

    mpOStm->SetNumberFormatInt( mpOStmOldModus );

    if ( xStatusIndicator.is() )
        xStatusIndicator->end();

    return mbStatus;
}

// ------------------------------------------------------------------------

sal_Bool XPMWriter::ImplWriteHeader()
{
    mnWidth = mpAcc->Width();
    mnHeight = mpAcc->Height();
    if ( mnWidth && mnHeight && mnColors )
    {
        *mpOStm << "/* XPM */\x0astatic char * image[] = \x0a{\x0a\x22";
        ImplWriteNumber( mnWidth );
        *mpOStm << (sal_uInt8)32;
        ImplWriteNumber( mnHeight );
        *mpOStm << (sal_uInt8)32;
        ImplWriteNumber( mnColors );
        *mpOStm << (sal_uInt8)32;
        ImplWriteNumber( ( mnColors > 26 ) ? 2 : 1 );
        *mpOStm << "\x22,\x0a";
    }
    else mbStatus = sal_False;
    return mbStatus;
}

// ------------------------------------------------------------------------

void XPMWriter::ImplWritePalette()
{
    sal_uInt16 nTransIndex = 0xffff;

    if ( mbTrans )
        nTransIndex = mpAcc->GetBestPaletteIndex( BMP_COL_TRANS );
    for ( sal_uInt16 i = 0; i < mnColors; i++ )
    {
        *mpOStm << "\x22";
        ImplWritePixel( i );
        *mpOStm << (sal_uInt8)32;
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
    for ( sal_uLong y = 0; y < mnHeight; y++ )
    {
        ImplCallback( (sal_uInt16)( ( 100 * y ) / mnHeight ) );         // processing output in percent
        *mpOStm << (sal_uInt8)0x22;
        for ( sal_uLong x = 0; x < mnWidth; x++ )
        {
            ImplWritePixel( mpAcc->GetPixelIndex( y, x ) );
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

void XPMWriter::ImplWritePixel( sal_uLong nCol )
{
    if ( mnColors > 26 )
    {
        sal_uInt8 nDiff = (sal_uInt8) ( nCol / 26 );
        *mpOStm << (sal_uInt8)( nDiff + 'A' );
        *mpOStm << (sal_uInt8)( nCol - ( nDiff*26 ) + 'A' );
    }
    else
        *mpOStm << (sal_uInt8)( nCol + 'A' );
}

// ------------------------------------------------------------------------
// ein Farbwert wird im Hexadezimalzahlformat in den Stream geschrieben
void XPMWriter::ImplWriteColor( sal_uInt16 nNumber )
{
    sal_uLong   nTmp;
    sal_uInt8   j;

    *mpOStm << "c #";   // # zeigt einen folgenden Hexwert an
    const BitmapColor& rColor = mpAcc->GetPaletteColor( nNumber );
    nTmp = ( rColor.GetRed() << 16 ) | ( rColor.GetGreen() << 8 ) | rColor.GetBlue();
    for ( signed char i = 20; i >= 0 ; i-=4 )
    {
        if ( ( j = (sal_uInt8)( nTmp >> i ) & 0xf ) > 9 )
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

extern "C" sal_Bool __LOADONCALLAPI GraphicExport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pFilterConfigItem, sal_Bool )
{
    XPMWriter aXPMWriter;

    return aXPMWriter.WriteXPM( rGraphic, rStream, pFilterConfigItem );
}

