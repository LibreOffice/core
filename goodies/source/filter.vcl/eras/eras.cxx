/*************************************************************************
 *
 *  $RCSfile: eras.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sj $ $Date: 2001-03-08 15:41:32 $
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

//============================ RASWriter ==================================

class RASWriter {

private:

    PFilterCallback     mpCallback;
    void *              mpCallerData;

    SvStream*           mpOStm;
    USHORT              mpOStmOldModus;

    BOOL                mbStatus;
    BitmapReadAccess*   mpAcc;

    ULONG               mnWidth, mnHeight;
    USHORT              mnColors, mnDepth;

    ULONG               mnRepCount;
    BYTE                mnRepVal;

    BOOL                ImplCallback( ULONG nCurrentYPos );
    BOOL                ImplWriteHeader();
    void                ImplWritePalette();
    void                ImplWriteBody();
    void                ImplPutByte( BYTE );    // RLE decoding

public:
                        RASWriter();
                        ~RASWriter();

    BOOL                WriteRAS( const Graphic& rGraphic, SvStream& rRAS,
                            PFilterCallback pCallback, void* pCallerdata,
                                FilterConfigItem* pConfigItem );
};

//=================== Methoden von RASWriter ==============================

RASWriter::RASWriter() :
    mpAcc       ( NULL ),
    mbStatus    ( TRUE ),
    mnRepCount  ( 0xffffffff )
{
}

// ------------------------------------------------------------------------

RASWriter::~RASWriter()
{
}

// ------------------------------------------------------------------------

BOOL RASWriter::ImplCallback( ULONG nYPos )
{
    if ( mpCallback != NULL )
    {
        if ( ( (*mpCallback)( mpCallerData, (USHORT)( ( 100 * nYPos ) / mnHeight ) ) ) == TRUE )
        {
            mpOStm->SetError( SVSTREAM_FILEFORMAT_ERROR );
            return TRUE;
        }
    }
    return FALSE;
}

//  ------------------------------------------------------------------------

BOOL RASWriter::WriteRAS( const Graphic& rGraphic, SvStream& rRAS,
                            PFilterCallback pCallback, void* pCallerdata,
                                FilterConfigItem* pConfigItem )
{
    Bitmap  aBmp;

    mpOStm = &rRAS;
    mpCallback = pCallback;
    mpCallerData = pCallerdata;

    BitmapEx    aBmpEx( rGraphic.GetBitmapEx() );
    aBmp = aBmpEx.GetBitmap();

    if ( aBmp.GetBitCount() == 4 )
        aBmp.Convert( BMP_CONVERSION_8BIT_COLORS );

    mnDepth = aBmp.GetBitCount();

    if ( ( mpAcc = aBmp.AcquireReadAccess() ) )
    {
        mpOStmOldModus = mpOStm->GetNumberFormatInt();
        mpOStm->SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );

        if ( ImplWriteHeader() )
        {
            if ( mnDepth <= 8 )
                ImplWritePalette();
            ImplWriteBody();
        }
        aBmp.ReleaseAccess( mpAcc );
    }
    else
        mbStatus = FALSE;

    mpOStm->SetNumberFormatInt( mpOStmOldModus );

    return mbStatus;
}

// ------------------------------------------------------------------------

BOOL RASWriter::ImplWriteHeader()
{
    mnWidth = mpAcc->Width();
    mnHeight = mpAcc->Height();
    if ( mnDepth <= 8 )
    {
        if (!( mnColors = mpAcc->GetPaletteEntryCount() ) )
            mbStatus = FALSE;
    }
        if ( mbStatus && mnWidth && mnHeight && mnDepth )
    {
        *mpOStm << (UINT32)0x59a66a95 << (UINT32)mnWidth << (UINT32)mnHeight
            << (UINT32)mnDepth
            << (UINT32) ( ( ( ( mnWidth * mnDepth ) + 15 ) >> 4 ) << 1 ) * mnHeight
            << (UINT32)2;

        if ( mnDepth > 8 )
            *mpOStm << (UINT32)0 << (UINT32)0;
        else
        {

            *mpOStm << (UINT32)1 << (UINT32)( mnColors * 3 );
        }
    }
    else mbStatus = FALSE;

    return mbStatus;
}

// ------------------------------------------------------------------------

void RASWriter::ImplWritePalette()
{
    USHORT i;

    for ( i = 0; i < mnColors; *mpOStm << mpAcc->GetPaletteColor( i++ ).GetRed() );
    for ( i = 0; i < mnColors; *mpOStm << mpAcc->GetPaletteColor( i++ ).GetGreen() );
    for ( i = 0; i < mnColors; *mpOStm << mpAcc->GetPaletteColor( i++ ).GetBlue() );
}

// ------------------------------------------------------------------------

void RASWriter::ImplWriteBody()
{
    ULONG   x, y;

    if ( mnDepth == 24 )
    {
        for ( y = 0; y < mnHeight; y++ )
        {
            ImplCallback( y );                              // processing output
            for ( x = 0; x < mnWidth; x++ )
            {
                BitmapColor aColor( mpAcc->GetPixel( y, x ) );
                ImplPutByte( aColor.GetBlue() );            // Format ist BGR
                ImplPutByte( aColor.GetGreen() );
                ImplPutByte( aColor.GetRed() );
            }
            if ( x & 1 ) ImplPutByte( 0 );      // WORD ALIGNMENT ???
        }
    }
    else if ( mnDepth == 8 )
    {
        for ( y = 0; y < mnHeight; y++ )
        {
            ImplCallback( y );                              // processing output
            for ( x = 0; x < mnWidth; x++ )
            {
                ImplPutByte ( mpAcc->GetPixel( y, x ) );
            }
            if ( x & 1 ) ImplPutByte( 0 );      // WORD ALIGNMENT ???
        }
    }
    else if ( mnDepth == 1 )
    {
        BYTE nDat;

        for ( y = 0; y < mnHeight; y++ )
        {
            ImplCallback( y );                              // processing output
            for ( x = 0; x < mnWidth; x++ )
            {
                nDat = ( ( nDat << 1 ) | ( mpAcc->GetPixel ( y, x ) & 1 ) );
                if ( ( x & 7 ) == 7 )
                    ImplPutByte( nDat );
            }
            if ( x & 7 )
                ImplPutByte( nDat << ( ( ( x & 7 ) ^ 7 ) + 1) );// write remaining bits
            if (!( ( x - 1 ) & 0x8 ) )
                ImplPutByte( 0 );               // WORD ALIGNMENT ???
        }
    }
    ImplPutByte( mnRepVal + 1 );    // end of RLE decoding
}

// ------------------------------------------------------------------------

void RASWriter::ImplPutByte( BYTE nPutThis )
{
    if ( mnRepCount == 0xffffffff )
    {
        mnRepCount = 0;
        mnRepVal = nPutThis;
    }
    else
    {
        if ( ( nPutThis == mnRepVal ) && ( mnRepCount != 0xff ) )
            mnRepCount++;
        else
        {
            if ( mnRepCount == 0 )
            {
                *mpOStm << (BYTE)mnRepVal;
                if ( mnRepVal == 0x80 )
                    *mpOStm << (BYTE)0;
            }
            else
            {
                *mpOStm << (BYTE)0x80;
                *mpOStm << (BYTE)mnRepCount;
                *mpOStm << (BYTE)mnRepVal;
            }
            mnRepVal = nPutThis;
            mnRepCount = 0;
        }
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
    RASWriter aRASWriter;

    return aRASWriter.WriteRAS( rGraphic, rStream, pCallback, pCallerData, pConfigItem );
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
