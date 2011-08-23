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
#include <svtools/FilterConfigItem.hxx>

//============================ RASWriter ==================================

class RASWriter {

private:

    SvStream*			mpOStm;
    USHORT				mpOStmOldModus;

    BOOL				mbStatus;
    BitmapReadAccess*	mpAcc;

    ULONG				mnWidth, mnHeight;
    USHORT				mnColors, mnDepth;

    ULONG				mnRepCount;
    BYTE				mnRepVal;

    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > xStatusIndicator;

    void				ImplCallback( ULONG nCurrentYPos );
    BOOL				ImplWriteHeader();
    void				ImplWritePalette();
    void				ImplWriteBody();
    void				ImplPutByte( BYTE );	// RLE decoding

public:
                        RASWriter();
                        ~RASWriter();

    BOOL				WriteRAS( const Graphic& rGraphic, SvStream& rRAS, FilterConfigItem* pFilterConfigItem );
};

//=================== Methoden von RASWriter ==============================

RASWriter::RASWriter() :
    mbStatus	( TRUE ),
    mpAcc		( NULL ),
    mnRepCount	( 0xffffffff )
{
}

// ------------------------------------------------------------------------

RASWriter::~RASWriter()
{
}

// ------------------------------------------------------------------------

void RASWriter::ImplCallback( ULONG nYPos )
{
    if ( xStatusIndicator.is() )
        xStatusIndicator->setValue( (USHORT)( ( 100 * nYPos ) / mnHeight ) );
}

//	------------------------------------------------------------------------

BOOL RASWriter::WriteRAS( const Graphic& rGraphic, SvStream& rRAS, FilterConfigItem* pFilterConfigItem)
{
    Bitmap	aBmp;

    mpOStm = &rRAS;

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

    if ( aBmp.GetBitCount() == 4 )
        aBmp.Convert( BMP_CONVERSION_8BIT_COLORS );

    mnDepth = aBmp.GetBitCount();

    // export code below only handles three discrete cases
    mnDepth = mnDepth <= 1 ? 1 : mnDepth <= 8 ? 8 : 24;

    mpAcc = aBmp.AcquireReadAccess();
    if ( mpAcc )
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

    if ( xStatusIndicator.is() )
        xStatusIndicator->end();

    return mbStatus;
}

// ------------------------------------------------------------------------

BOOL RASWriter::ImplWriteHeader()
{
    mnWidth = mpAcc->Width();
    mnHeight = mpAcc->Height();
    if ( mnDepth <= 8 )
    {
        mnColors = mpAcc->GetPaletteEntryCount();
        if (mnColors == 0)
            mbStatus = FALSE;
    }
        if ( mbStatus && mnWidth && mnHeight && mnDepth )
    {
        *mpOStm << (UINT32)0x59a66a95 << (UINT32)mnWidth << (UINT32)mnHeight
            << (UINT32)mnDepth
            << (UINT32)(( ( ( ( mnWidth * mnDepth ) + 15 ) >> 4 ) << 1 ) * mnHeight)
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

    for ( i = 0; i < mnColors; *mpOStm << mpAcc->GetPaletteColor( i++ ).GetRed() ) ;
    for ( i = 0; i < mnColors; *mpOStm << mpAcc->GetPaletteColor( i++ ).GetGreen() ) ;
    for ( i = 0; i < mnColors; *mpOStm << mpAcc->GetPaletteColor( i++ ).GetBlue() ) ;
}

// ------------------------------------------------------------------------

void RASWriter::ImplWriteBody()
{
    ULONG	x, y;

    if ( mnDepth == 24 )
    {
        for ( y = 0; y < mnHeight; y++ )
        {
            ImplCallback( y );								// processing output
            for ( x = 0; x < mnWidth; x++ )
            {
                BitmapColor aColor( mpAcc->GetPixel( y, x ) );
                ImplPutByte( aColor.GetBlue() );			// Format ist BGR
                ImplPutByte( aColor.GetGreen() );
                ImplPutByte( aColor.GetRed() );
            }
            if ( x & 1 ) ImplPutByte( 0 );		// WORD ALIGNMENT ???
        }
    }
    else if ( mnDepth == 8 )
    {
        for ( y = 0; y < mnHeight; y++ )
        {
            ImplCallback( y );								// processing output
            for ( x = 0; x < mnWidth; x++ )
            {
                ImplPutByte ( mpAcc->GetPixel( y, x ) );
            }
            if ( x & 1 ) ImplPutByte( 0 );		// WORD ALIGNMENT ???
        }
    }
    else if ( mnDepth == 1 )
    {
        BYTE nDat = 0;

        for ( y = 0; y < mnHeight; y++ )
        {
            ImplCallback( y );								// processing output
            for ( x = 0; x < mnWidth; x++ )
            {
                nDat = ( ( nDat << 1 ) | ( mpAcc->GetPixel ( y, x ) & 1 ) );
                if ( ( x & 7 ) == 7 )
                    ImplPutByte( nDat );
            }
            if ( x & 7 )
                ImplPutByte( sal::static_int_cast< BYTE >(nDat << ( ( ( x & 7 ) ^ 7 ) + 1)) );// write remaining bits
            if (!( ( x - 1 ) & 0x8 ) )
                ImplPutByte( 0 );				// WORD ALIGNMENT ???
        }
    }
    ImplPutByte( mnRepVal + 1 );	// end of RLE decoding
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

extern "C" BOOL __LOADONCALLAPI GraphicExport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pFilterConfigItem, BOOL )
{
    RASWriter aRASWriter;

    return aRASWriter.WriteRAS( rGraphic, rStream, pFilterConfigItem );
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
