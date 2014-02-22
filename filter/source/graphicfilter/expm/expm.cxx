/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/fltcall.hxx>



class XPMWriter {

private:

    SvStream&           m_rOStm;            

    sal_Bool            mbStatus;
    sal_Bool            mbTrans;
    BitmapReadAccess*   mpAcc;
    sal_uLong           mnWidth, mnHeight;  
    sal_uInt16          mnColors;

    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > xStatusIndicator;

    void                ImplCallback( sal_uInt16 nPercent );
    sal_Bool            ImplWriteHeader();
    void                ImplWritePalette();
    void                ImplWriteColor( sal_uInt16 );
    void                ImplWriteBody();
    void                ImplWriteNumber( sal_Int32 );
    void                ImplWritePixel( sal_uLong ) const;

public:
    XPMWriter(SvStream& rOStm);
    ~XPMWriter();

    sal_Bool            WriteXPM( const Graphic& rGraphic, FilterConfigItem* pFilterConfigItem );
};



XPMWriter::XPMWriter(SvStream& rOStm)
    : m_rOStm(rOStm)
    , mbStatus(sal_True)
    , mbTrans(sal_False)
    , mpAcc(NULL)
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



sal_Bool XPMWriter::WriteXPM( const Graphic& rGraphic, FilterConfigItem* pFilterConfigItem)
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

    if ( rGraphic.IsTransparent() )                 
    {
        mbTrans = sal_True;
        if ( aBmp.GetBitCount() >= 8 )              
            aBmp.Convert( BMP_CONVERSION_8BIT_TRANS );
        else
            aBmp.Convert( BMP_CONVERSION_4BIT_TRANS );
        aBmp.Replace( aBmpEx.GetMask(), BMP_COL_TRANS );
    }
    else
    {
        if ( aBmp.GetBitCount() > 8 )               
            aBmp.Convert( BMP_CONVERSION_8BIT_COLORS );
    }
    mpAcc = aBmp.AcquireReadAccess();
    if ( mpAcc )
    {
        sal_uInt16 nOStmOldModus = m_rOStm.GetNumberFormatInt();
        m_rOStm.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );

        mnColors = mpAcc->GetPaletteEntryCount();
        if ( ImplWriteHeader() )
        {
            ImplWritePalette();
            ImplWriteBody();
            m_rOStm.WriteCharPtr( "\x22XPMENDEXT\x22\x0a};" );
        }

        m_rOStm.SetNumberFormatInt(nOStmOldModus);

        aBmp.ReleaseAccess( mpAcc );
    }
    else
        mbStatus = sal_False;


    if ( xStatusIndicator.is() )
        xStatusIndicator->end();

    return mbStatus;
}



sal_Bool XPMWriter::ImplWriteHeader()
{
    mnWidth = mpAcc->Width();
    mnHeight = mpAcc->Height();
    if ( mnWidth && mnHeight && mnColors )
    {
        m_rOStm.WriteCharPtr( "/* XPM */\x0astatic char * image[] = \x0a{\x0a\x22" );
        ImplWriteNumber( mnWidth );
        m_rOStm.WriteUChar( (sal_uInt8)32 );
        ImplWriteNumber( mnHeight );
        m_rOStm.WriteUChar( (sal_uInt8)32 );
        ImplWriteNumber( mnColors );
        m_rOStm.WriteUChar( (sal_uInt8)32 );
        ImplWriteNumber( ( mnColors > 26 ) ? 2 : 1 );
        m_rOStm.WriteCharPtr( "\x22,\x0a" );
    }
    else mbStatus = sal_False;
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
        m_rOStm.WriteUChar( (sal_uInt8)32 );
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
        ImplCallback( (sal_uInt16)( ( 100 * y ) / mnHeight ) );         
        m_rOStm.WriteUChar( (sal_uInt8)0x22 );
        for ( sal_uLong x = 0; x < mnWidth; x++ )
        {
            ImplWritePixel( mpAcc->GetPixelIndex( y, x ) );
        }
        m_rOStm.WriteCharPtr( "\x22,\x0a" );
    }
}




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
        m_rOStm.WriteUChar( (sal_uInt8)( nDiff + 'A' ) );
        m_rOStm.WriteUChar( (sal_uInt8)( nCol - ( nDiff*26 ) + 'A' ) );
    }
    else
        m_rOStm.WriteUChar( (sal_uInt8)( nCol + 'A' ) );
}



void XPMWriter::ImplWriteColor( sal_uInt16 nNumber )
{
    sal_uLong   nTmp;
    sal_uInt8   j;

    m_rOStm.WriteCharPtr( "c #" );   
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






#if defined(DISABLE_DYNLOADING)
#define GraphicExport expGraphicExport
#endif

extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL
GraphicExport(SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pFilterConfigItem, sal_Bool)
{
    XPMWriter aXPMWriter(rStream);

    return aXPMWriter.WriteXPM( rGraphic, pFilterConfigItem );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
