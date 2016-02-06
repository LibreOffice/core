/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <vcl/graphic.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/fltcall.hxx>
#include <vcl/FilterConfigItem.hxx>

//============================ RASWriter ==================================

class RASWriter {

private:

    SvStream & m_rOStm;

    bool                mbStatus;
    BitmapReadAccess*   mpAcc;

    sal_uLong               mnWidth, mnHeight;
    sal_uInt16              mnColors, mnDepth;

    sal_uLong               mnRepCount;
    sal_uInt8               mnRepVal;

    css::uno::Reference< css::task::XStatusIndicator > xStatusIndicator;

    void                ImplCallback( sal_uLong nCurrentYPos );
    bool                ImplWriteHeader();
    void                ImplWritePalette();
    void                ImplWriteBody();
    void                ImplPutByte( sal_uInt8 );   // RLE decoding

public:
    explicit RASWriter(SvStream &rStream);
    ~RASWriter();

    bool WriteRAS( const Graphic& rGraphic, FilterConfigItem* pFilterConfigItem );
};

//=================== Methoden von RASWriter ==============================
RASWriter::RASWriter(SvStream &rStream)
    : m_rOStm(rStream)
    , mbStatus(true)
    , mpAcc(nullptr)
    , mnWidth(0)
    , mnHeight(0)
    , mnColors(0)
    , mnDepth(0)
    , mnRepCount(0xffffffff)
    , mnRepVal(0)
{
}

RASWriter::~RASWriter()
{
}



void RASWriter::ImplCallback( sal_uLong nYPos )
{
    if ( xStatusIndicator.is() )
        xStatusIndicator->setValue( (sal_uInt16)( ( 100 * nYPos ) / mnHeight ) );
}



bool RASWriter::WriteRAS( const Graphic& rGraphic, FilterConfigItem* pFilterConfigItem)
{
    Bitmap  aBmp;

    if ( pFilterConfigItem )
    {
        xStatusIndicator = pFilterConfigItem->GetStatusIndicator();
        if ( xStatusIndicator.is() )
        {
            OUString aMsg;
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
        SvStreamEndian nOStmOldModus = m_rOStm.GetEndian();
        m_rOStm.SetEndian( SvStreamEndian::BIG );

        if ( ImplWriteHeader() )
        {
            if ( mnDepth <= 8 )
                ImplWritePalette();
            ImplWriteBody();
        }

        m_rOStm.SetEndian( nOStmOldModus );

        Bitmap::ReleaseAccess( mpAcc );
    }
    else
        mbStatus = false;

    if ( xStatusIndicator.is() )
        xStatusIndicator->end();

    return mbStatus;
}



bool RASWriter::ImplWriteHeader()
{
    mnWidth = mpAcc->Width();
    mnHeight = mpAcc->Height();
    if ( mnDepth <= 8 )
    {
        mnColors = mpAcc->GetPaletteEntryCount();
        if (mnColors == 0)
            mbStatus = false;
    }
        if ( mbStatus && mnWidth && mnHeight && mnDepth )
    {
        m_rOStm.WriteUInt32( 0x59a66a95 ).WriteUInt32( mnWidth ).WriteUInt32( mnHeight )
           .WriteUInt32( mnDepth )
           .WriteUInt32( ( ( ( ( mnWidth * mnDepth ) + 15 ) >> 4 ) << 1 ) * mnHeight )
           .WriteUInt32( 2 );

        if ( mnDepth > 8 )
            m_rOStm.WriteUInt32( 0 ).WriteUInt32( 0 );
        else
        {

            m_rOStm.WriteUInt32( 1 ).WriteUInt32( mnColors * 3 );
        }
    }
    else mbStatus = false;

    return mbStatus;
}



void RASWriter::ImplWritePalette()
{
    sal_uInt16 i;

    for ( i = 0; i < mnColors; m_rOStm.WriteUChar( mpAcc->GetPaletteColor( i++ ).GetRed() ) ) ;
    for ( i = 0; i < mnColors; m_rOStm.WriteUChar( mpAcc->GetPaletteColor( i++ ).GetGreen() ) ) ;
    for ( i = 0; i < mnColors; m_rOStm.WriteUChar( mpAcc->GetPaletteColor( i++ ).GetBlue() ) ) ;
}



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
                ImplPutByte ( mpAcc->GetPixelIndex( y, x ) );
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
                nDat = ( ( nDat << 1 ) | ( mpAcc->GetPixelIndex( y, x ) & 1 ) );
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
                m_rOStm.WriteUChar( mnRepVal );
                if ( mnRepVal == 0x80 )
                    m_rOStm.WriteUChar( 0 );
            }
            else
            {
                m_rOStm.WriteUChar( 0x80 );
                m_rOStm.WriteUChar( mnRepCount );
                m_rOStm.WriteUChar( mnRepVal );
            }
            mnRepVal = nPutThis;
            mnRepCount = 0;
        }
    }
}




// - exported function -


extern "C" SAL_DLLPUBLIC_EXPORT bool SAL_CALL
eraGraphicExport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pFilterConfigItem )
{
    RASWriter aRASWriter(rStream);

    return aRASWriter.WriteRAS( rGraphic, pFilterConfigItem );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
