/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

    SvStream & m_rOStm;
    sal_uInt16              mpOStmOldModus;

    sal_Bool                mbStatus;
    BitmapReadAccess*   mpAcc;

    sal_uLong               mnWidth, mnHeight;
    sal_uInt16              mnColors, mnDepth;

    sal_uLong               mnRepCount;
    sal_uInt8               mnRepVal;

    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > xStatusIndicator;

    void                ImplCallback( sal_uLong nCurrentYPos );
    sal_Bool                ImplWriteHeader();
    void                ImplWritePalette();
    void                ImplWriteBody();
    void                ImplPutByte( sal_uInt8 );   // RLE decoding

public:
    RASWriter(SvStream &rStream);
    ~RASWriter();

    sal_Bool WriteRAS( const Graphic& rGraphic, FilterConfigItem* pFilterConfigItem );
};

//=================== Methoden von RASWriter ==============================

RASWriter::RASWriter(SvStream &rStream)
    : m_rOStm(rStream)
    , mbStatus(sal_True)
    , mpAcc(NULL)
    , mnRepCount( 0xffffffff )
{
}

// ------------------------------------------------------------------------

RASWriter::~RASWriter()
{
}

// ------------------------------------------------------------------------

void RASWriter::ImplCallback( sal_uLong nYPos )
{
    if ( xStatusIndicator.is() )
        xStatusIndicator->setValue( (sal_uInt16)( ( 100 * nYPos ) / mnHeight ) );
}

//  ------------------------------------------------------------------------

sal_Bool RASWriter::WriteRAS( const Graphic& rGraphic, FilterConfigItem* pFilterConfigItem)
{
    Bitmap  aBmp;

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

    if ( aBmp.GetBitCount() == 4 )
        aBmp.Convert( BMP_CONVERSION_8BIT_COLORS );

    mnDepth = aBmp.GetBitCount();

    // export code below only handles three discrete cases
    mnDepth = mnDepth <= 1 ? 1 : mnDepth <= 8 ? 8 : 24;

    mpAcc = aBmp.AcquireReadAccess();
    if ( mpAcc )
    {
        mpOStmOldModus = m_rOStm.GetNumberFormatInt();
        m_rOStm.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );

        if ( ImplWriteHeader() )
        {
            if ( mnDepth <= 8 )
                ImplWritePalette();
            ImplWriteBody();
        }
        aBmp.ReleaseAccess( mpAcc );
    }
    else
        mbStatus = sal_False;

    m_rOStm.SetNumberFormatInt( mpOStmOldModus );

    if ( xStatusIndicator.is() )
        xStatusIndicator->end();

    return mbStatus;
}

// ------------------------------------------------------------------------

sal_Bool RASWriter::ImplWriteHeader()
{
    mnWidth = mpAcc->Width();
    mnHeight = mpAcc->Height();
    if ( mnDepth <= 8 )
    {
        mnColors = mpAcc->GetPaletteEntryCount();
        if (mnColors == 0)
            mbStatus = sal_False;
    }
        if ( mbStatus && mnWidth && mnHeight && mnDepth )
    {
        m_rOStm << (sal_uInt32)0x59a66a95 << (sal_uInt32)mnWidth << (sal_uInt32)mnHeight
            << (sal_uInt32)mnDepth
            << (sal_uInt32)(( ( ( ( mnWidth * mnDepth ) + 15 ) >> 4 ) << 1 ) * mnHeight)
            << (sal_uInt32)2;

        if ( mnDepth > 8 )
            m_rOStm << (sal_uInt32)0 << (sal_uInt32)0;
        else
        {

            m_rOStm << (sal_uInt32)1 << (sal_uInt32)( mnColors * 3 );
        }
    }
    else mbStatus = sal_False;

    return mbStatus;
}

// ------------------------------------------------------------------------

void RASWriter::ImplWritePalette()
{
    sal_uInt16 i;

    for ( i = 0; i < mnColors; m_rOStm << mpAcc->GetPaletteColor( i++ ).GetRed() ) ;
    for ( i = 0; i < mnColors; m_rOStm << mpAcc->GetPaletteColor( i++ ).GetGreen() ) ;
    for ( i = 0; i < mnColors; m_rOStm << mpAcc->GetPaletteColor( i++ ).GetBlue() ) ;
}

// ------------------------------------------------------------------------

void RASWriter::ImplWriteBody()
{
    sal_uLong   x, y;

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
        sal_uInt8 nDat = 0;

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
                ImplPutByte( sal::static_int_cast< sal_uInt8 >(nDat << ( ( ( x & 7 ) ^ 7 ) + 1)) );// write remaining bits
            if (!( ( x - 1 ) & 0x8 ) )
                ImplPutByte( 0 );               // WORD ALIGNMENT ???
        }
    }
    ImplPutByte( mnRepVal + 1 );    // end of RLE decoding
}

// ------------------------------------------------------------------------

void RASWriter::ImplPutByte( sal_uInt8 nPutThis )
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
                m_rOStm << (sal_uInt8)mnRepVal;
                if ( mnRepVal == 0x80 )
                    m_rOStm << (sal_uInt8)0;
            }
            else
            {
                m_rOStm << (sal_uInt8)0x80;
                m_rOStm << (sal_uInt8)mnRepCount;
                m_rOStm << (sal_uInt8)mnRepVal;
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

extern "C" sal_Bool __LOADONCALLAPI GraphicExport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pFilterConfigItem, sal_Bool )
{
    RASWriter aRASWriter(rStream);

    return aRASWriter.WriteRAS( rGraphic, pFilterConfigItem );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
