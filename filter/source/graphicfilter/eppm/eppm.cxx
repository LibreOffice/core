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


#include <vcl/svapp.hxx>
#include <vcl/graphic.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/msgbox.hxx>
#include <svl/solar.hrc>
#include <vcl/fltcall.hxx>
#include <vcl/FilterConfigItem.hxx>

//============================ PPMWriter ==================================

class PPMWriter {

private:

    SvStream&           m_rOStm;          // Die auszugebende PPM-Datei

    bool                mbStatus;
    sal_Int32           mnMode;
    BitmapReadAccess*   mpAcc;
    sal_uLong           mnWidth, mnHeight;  // Bildausmass in Pixeln

    bool                ImplWriteHeader();
    void                ImplWriteBody();
    void                ImplWriteNumber( sal_Int32 );

    css::uno::Reference< css::task::XStatusIndicator > xStatusIndicator;

public:
    explicit PPMWriter(SvStream &rStrm);
    ~PPMWriter();

    bool                WritePPM( const Graphic& rGraphic, FilterConfigItem* pFilterConfigItem );
};

//=================== Methods of PPMWriter ==============================
PPMWriter::PPMWriter(SvStream &rStrm)
    : m_rOStm(rStrm)
    , mbStatus(true)
    , mnMode(0)
    , mpAcc(nullptr)
    , mnWidth(0)
    , mnHeight(0)
{
}

PPMWriter::~PPMWriter()
{
}



bool PPMWriter::WritePPM( const Graphic& rGraphic, FilterConfigItem* pFilterConfigItem )
{
    if ( pFilterConfigItem )
    {
        mnMode = pFilterConfigItem->ReadInt32( "FileFormat", 0 );

        xStatusIndicator = pFilterConfigItem->GetStatusIndicator();
        if ( xStatusIndicator.is() )
        {
            OUString aMsg;
            xStatusIndicator->start( aMsg, 100 );
        }
    }

    BitmapEx    aBmpEx( rGraphic.GetBitmapEx() );
    Bitmap      aBmp = aBmpEx.GetBitmap();
    aBmp.Convert( BMP_CONVERSION_24BIT );

    SvStreamEndian aOStmOldModus = m_rOStm.GetEndian();
    m_rOStm.SetEndian( SvStreamEndian::BIG );

    mpAcc = aBmp.AcquireReadAccess();
    if( mpAcc )
    {
        if ( ImplWriteHeader() )
        {
            ImplWriteBody();
        }
        Bitmap::ReleaseAccess( mpAcc );
    }
    else
        mbStatus = false;

    m_rOStm.SetEndian( aOStmOldModus );

    if ( xStatusIndicator.is() )
        xStatusIndicator->end();

    return mbStatus;
}



bool PPMWriter::ImplWriteHeader()
{
    mnWidth = mpAcc->Width();
    mnHeight = mpAcc->Height();
    if ( mnWidth && mnHeight )
    {
        if ( mnMode == 0 )
            m_rOStm.WriteCharPtr( "P6\x0a" );
        else
            m_rOStm.WriteCharPtr( "P3\x0a" );

        ImplWriteNumber( mnWidth );
        m_rOStm.WriteUChar( 32 );
        ImplWriteNumber( mnHeight );
        m_rOStm.WriteUChar( 32 );
        ImplWriteNumber( 255 );         // max. col.
        m_rOStm.WriteUChar( 10 );
    }
    else
        mbStatus = false;

    return mbStatus;
}



void PPMWriter::ImplWriteBody()
{
    if ( mnMode == 0 )
    {
        for ( sal_uLong y = 0; y < mnHeight; y++ )
        {
            for ( sal_uLong x = 0; x < mnWidth; x++ )
            {
                const BitmapColor& rColor = mpAcc->GetPixel( y, x );
                m_rOStm.WriteUChar( rColor.GetRed() );
                m_rOStm.WriteUChar( rColor.GetGreen() );
                m_rOStm.WriteUChar( rColor.GetBlue() );
            }
        }
    }
    else
    {
        for ( sal_uLong y = 0; y < mnHeight; y++ )
        {
            int nCount = 70;
            for ( sal_uLong x = 0; x < mnWidth; x++ )
            {
                sal_uInt8 i, nDat[3], nNumb;
                if ( nCount < 0 )
                {
                    nCount = 69;
                    m_rOStm.WriteUChar( 10 );
                }
                nDat[0] = mpAcc->GetPixel( y, x ).GetRed();
                nDat[1] = mpAcc->GetPixel( y, x ).GetGreen();
                nDat[2] = mpAcc->GetPixel( y, x ).GetBlue();
                for ( i = 0; i < 3; i++ )
                {
                    nNumb = nDat[ i ] / 100;
                    if ( nNumb )
                    {
                        m_rOStm.WriteUChar( nNumb + '0' );
                        nDat[ i ] -= ( nNumb * 100 );
                        nNumb = nDat[ i ] / 10;
                        m_rOStm.WriteUChar( nNumb + '0' );
                        nDat[ i ] -= ( nNumb * 10 );
                        m_rOStm.WriteUChar( nDat[ i ] + '0' );
                        nCount -= 4;
                    }
                    else
                    {
                        nNumb = nDat[ i ] / 10;
                        if ( nNumb )
                        {
                            m_rOStm.WriteUChar( nNumb + '0' );
                            nDat[ i ] -= ( nNumb * 10 );
                            m_rOStm.WriteUChar( nDat[ i ] + '0' );
                            nCount -= 3;
                        }
                        else
                        {
                            m_rOStm.WriteUChar( nDat[ i ] + '0' );
                            nCount -= 2;
                        }
                    }
                    m_rOStm.WriteUChar( ' ' );
                }
            }
            m_rOStm.WriteUChar( 10 );
        }
    }
}


// a decimal number in ASCII format is being written into the stream

void PPMWriter::ImplWriteNumber(sal_Int32 nNumber)
{
    const OString aNum(OString::number(nNumber));
    m_rOStm.WriteCharPtr( aNum.getStr() );
}




// - exported function -


extern "C" SAL_DLLPUBLIC_EXPORT bool SAL_CALL
eppGraphicExport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pFilterConfigItem )
{
    PPMWriter aPPMWriter(rStream);
    return aPPMWriter.WritePPM( rGraphic, pFilterConfigItem );
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
