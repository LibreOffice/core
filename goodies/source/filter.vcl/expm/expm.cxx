/*************************************************************************
 *
 *  $RCSfile: expm.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sj $ $Date: 2001-03-08 15:38:59 $
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

//============================ XPMWriter ==================================

class XPMWriter {

private:

    PFilterCallback     mpCallback;
    void *              mpCallerData;

    SvStream*           mpOStm;             // Die auszugebende XPM-Datei
    USHORT              mpOStmOldModus;

    BOOL                mbStatus;
    BOOL                mbTrans;
    BitmapReadAccess*   mpAcc;
    ULONG               mnWidth, mnHeight;  // Bildausmass in Pixeln
    USHORT              mnColors;

    BOOL                ImplCallback( USHORT nPercent );
    BOOL                ImplWriteHeader();
    void                ImplWritePalette();
    void                ImplWriteColor( USHORT );
    void                ImplWriteBody();
    void                ImplWriteNumber( sal_Int32 );
    void                ImplWritePixel( ULONG );

public:
                        XPMWriter();
                        ~XPMWriter();

    BOOL                WriteXPM( const Graphic& rGraphic, SvStream& rXPM,
                                  PFilterCallback pCallback, void* pCallerdata,
                                  FilterConfigItem* pConfigItem );
};

//=================== Methoden von XPMWriter ==============================

XPMWriter::XPMWriter() :
    mpAcc       ( NULL ),
    mbStatus    ( TRUE ),
    mbTrans     ( FALSE )
{
}

// ------------------------------------------------------------------------

XPMWriter::~XPMWriter()
{
}

// ------------------------------------------------------------------------

BOOL XPMWriter::ImplCallback( USHORT nPercent )
{
    if ( mpCallback != NULL )
    {
        if ( ( (*mpCallback)( mpCallerData, nPercent ) ) == TRUE )
        {
            mpOStm->SetError( SVSTREAM_FILEFORMAT_ERROR );
            return TRUE;
        }
    }
    return FALSE;
}

//  ------------------------------------------------------------------------

BOOL XPMWriter::WriteXPM( const Graphic& rGraphic, SvStream& rXPM,
                          PFilterCallback pCallback, void* pCallerdata,
                          FilterConfigItem* pConfigItem )
{
    Bitmap  aBmp;

    mpOStm = &rXPM;
    mpCallback = pCallback;
    mpCallerData = pCallerdata;

    BitmapEx    aBmpEx( rGraphic.GetBitmapEx() );
    aBmp = aBmpEx.GetBitmap();

    if ( rGraphic.IsTransparent() )                 // event. transparente Farbe erzeugen
    {
        mbTrans = TRUE;
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
    if ( ( mpAcc = aBmp.AcquireReadAccess() ) )
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
        ImplCallback( (USHORT)( ( 100 * y ) / mnHeight ) );         // processing output in percent
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
    ULONG   nTmp;
    BYTE    j;

    *mpOStm << "c #";   // # zeigt einen folgenden Hexwert an
    const BitmapColor& rColor = mpAcc->GetPaletteColor( nNumber );
    nTmp = ( rColor.GetRed() << 16 ) | ( rColor.GetGreen() << 8 ) | rColor.GetBlue();
    for ( char i = 20; i >= 0 ; i-=4 )
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

extern "C" BOOL __LOADONCALLAPI GraphicExport( SvStream& rStream, Graphic& rGraphic,
                                               PFilterCallback pCallback, void* pCallerData,
                                               FilterConfigItem* pConfigItem, BOOL )
{
    XPMWriter aXPMWriter;

    return aXPMWriter.WriteXPM( rGraphic, rStream, pCallback, pCallerData, pConfigItem );
}

#pragma hdrstop

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
