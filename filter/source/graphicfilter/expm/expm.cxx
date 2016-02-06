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

//============================ XPMWriter ==================================

class XPMWriter {

private:

    SvStream&           m_rOStm;            // the output XPM file

    bool            mbStatus;
    bool            mbTrans;
    BitmapReadAccess*   mpAcc;
    sal_uLong           mnWidth, mnHeight;  // size in Pixel
    sal_uInt16          mnColors;

    css::uno::Reference< css::task::XStatusIndicator > xStatusIndicator;

    void                ImplCallback( sal_uInt16 nPercent );
    bool            ImplWriteHeader();
    void                ImplWritePalette();
    void                ImplWriteColor( sal_uInt16 );
    void                ImplWriteBody();
    void                ImplWriteNumber( sal_Int32 );
    void                ImplWritePixel( sal_uLong ) const;

public:
    explicit XPMWriter(SvStream& rOStm);
    ~XPMWriter();

    bool            WriteXPM( const Graphic& rGraphic, FilterConfigItem* pFilterConfigItem );
};

//=================== Methoden von XPMWriter ==============================

XPMWriter::XPMWriter(SvStream& rOStm)
    : m_rOStm(rOStm)
    , mbStatus(true)
    , mbTrans(false)
    , mpAcc(nullptr)
    , mnWidth(0)
    , mnHeight(0)
    , mnColors(0)
{
}



XPMWriter::~XPMWriter()
{
}



void XPMWriter::ImplCallback( sal_uInt16 nPercent )
{
    if ( xStatusIndicator.is() )
    {
        if ( nPercent <= 100 )
            xStatusIndicator->setValue( nPercent );
    }
}



bool XPMWriter::WriteXPM( const Graphic& rGraphic, FilterConfigItem* pFilterConfigItem)
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

    if ( rGraphic.IsTransparent() )                 // possibly create transparent color
    {
        mbTrans = true;
        if ( aBmp.GetBitCount() >= 8 )              // if necessary convert image to 8 bit
            aBmp.Convert( BMP_CONVERSION_8BIT_TRANS );
        else
            aBmp.Convert( BMP_CONVERSION_4BIT_TRANS );
        aBmp.Replace( aBmpEx.GetMask(), BMP_COL_TRANS );
    }
    else
    {
        if ( aBmp.GetBitCount() > 8 )               // if necessary convert image to 8 bit
            aBmp.Convert( BMP_CONVERSION_8BIT_COLORS );
    }
    mpAcc = aBmp.AcquireReadAccess();
    if ( mpAcc )
    {
        SvStreamEndian nOStmOldModus = m_rOStm.GetEndian();
        m_rOStm.SetEndian( SvStreamEndian::BIG );

        mnColors = mpAcc->GetPaletteEntryCount();
        if ( ImplWriteHeader() )
        {
            ImplWritePalette();
            ImplWriteBody();
            m_rOStm.WriteCharPtr( "\x22XPMENDEXT\x22\x0a};" );
        }

        m_rOStm.SetEndian(nOStmOldModus);

        Bitmap::ReleaseAccess( mpAcc );
    }
    else
        mbStatus = false;


    if ( xStatusIndicator.is() )
        xStatusIndicator->end();

    return mbStatus;
}



bool XPMWriter::ImplWriteHeader()
{
    mnWidth = mpAcc->Width();
    mnHeight = mpAcc->Height();
    if ( mnWidth && mnHeight && mnColors )
    {
        m_rOStm.WriteCharPtr( "/* XPM */\x0astatic char * image[] = \x0a{\x0a\x22" );
        ImplWriteNumber( mnWidth );
        m_rOStm.WriteUChar( 32 );
        ImplWriteNumber( mnHeight );
        m_rOStm.WriteUChar( 32 );
        ImplWriteNumber( mnColors );
        m_rOStm.WriteUChar( 32 );
        ImplWriteNumber( ( mnColors > 26 ) ? 2 : 1 );
        m_rOStm.WriteCharPtr( "\x22,\x0a" );
    }
    else mbStatus = false;
    return mbStatus;
}



void XPMWriter::ImplWritePalette()
{
    sal_uInt16 nTransIndex = 0xffff;

    if ( mbTrans )
        nTransIndex = mpAcc->GetBestPaletteIndex( BMP_COL_TRANS );
    for ( sal_uInt16 i = 0; i < mnColors; i++ )
    {
        m_rOStm.WriteCharPtr( "\x22" );
        ImplWritePixel( i );
        m_rOStm.WriteUChar( 32 );
        if ( nTransIndex != i )
        {
            ImplWriteColor( i );
            m_rOStm.WriteCharPtr( "\x22,\x0a" );
        }
        else
            m_rOStm.WriteCharPtr( "c none\x22,\x0a" );
    }
}



void XPMWriter::ImplWriteBody()
{
    for ( sal_uLong y = 0; y < mnHeight; y++ )
    {
        ImplCallback( (sal_uInt16)( ( 100 * y ) / mnHeight ) );         // processing output in percent
        m_rOStm.WriteUChar( 0x22 );
        for ( sal_uLong x = 0; x < mnWidth; x++ )
        {
            ImplWritePixel( mpAcc->GetPixelIndex( y, x ) );
        }
        m_rOStm.WriteCharPtr( "\x22,\x0a" );
    }
}


// write a decimal number in ascii format into the stream

void XPMWriter::ImplWriteNumber(sal_Int32 nNumber)
{
    const OString aNum(OString::number(nNumber));
    m_rOStm.WriteCharPtr( aNum.getStr() );
}



void XPMWriter::ImplWritePixel( sal_uLong nCol ) const
{
    if ( mnColors > 26 )
    {
        sal_uInt8 nDiff = (sal_uInt8) ( nCol / 26 );
        m_rOStm.WriteUChar( nDiff + 'A' );
        m_rOStm.WriteUChar( nCol - ( nDiff*26 ) + 'A' );
    }
    else
        m_rOStm.WriteUChar( nCol + 'A' );
}


// write a color value in hex format into the stream
void XPMWriter::ImplWriteColor( sal_uInt16 nNumber )
{
    sal_uLong   nTmp;
    sal_uInt8   j;

    m_rOStm.WriteCharPtr( "c #" );   // # indicates a following hex value
    const BitmapColor& rColor = mpAcc->GetPaletteColor( nNumber );
    nTmp = ( rColor.GetRed() << 16 ) | ( rColor.GetGreen() << 8 ) | rColor.GetBlue();
    for ( signed char i = 20; i >= 0 ; i-=4 )
    {
        if ( ( j = (sal_uInt8)( nTmp >> i ) & 0xf ) > 9 )
            j += 'A' - 10;
        else
            j += '0';
        m_rOStm.WriteUChar( j );
    }
}


extern "C" SAL_DLLPUBLIC_EXPORT bool SAL_CALL
expGraphicExport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pFilterConfigItem )
{
    XPMWriter aXPMWriter(rStream);

    return aXPMWriter.WriteXPM( rGraphic, pFilterConfigItem );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
