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

//============================ PBMWriter ==================================

class PBMWriter {

private:

    SvStream&           m_rOStm;            // the output PBM file

    bool                mbStatus;
    sal_Int32           mnMode;             // 0 -> raw, 1-> ascii
    BitmapReadAccess*   mpAcc;
    sal_uLong           mnWidth, mnHeight;  // size in pixel

    bool                ImplWriteHeader();
    void                ImplWriteBody();
    void                ImplWriteNumber( sal_Int32 );

    css::uno::Reference< css::task::XStatusIndicator > xStatusIndicator;

public:
    explicit PBMWriter(SvStream &rPBM);
    ~PBMWriter();

    bool WritePBM( const Graphic& rGraphic, FilterConfigItem* pFilterConfigItem );
};

//=================== Methods of PBMWriter ==============================

PBMWriter::PBMWriter(SvStream &rPBM)
    : m_rOStm(rPBM)
    , mbStatus(true)
    , mnMode(0)
    , mpAcc(nullptr)
    , mnWidth(0)
    , mnHeight(0)
{
}



PBMWriter::~PBMWriter()
{
}



bool PBMWriter::WritePBM( const Graphic& rGraphic, FilterConfigItem* pFilterConfigItem )
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
    aBmp.Convert( BMP_CONVERSION_1BIT_THRESHOLD );

    SvStreamEndian aOStmOldModus = m_rOStm.GetEndian();
    m_rOStm.SetEndian( SvStreamEndian::BIG );

    mpAcc = aBmp.AcquireReadAccess();
    if( mpAcc )
    {
        if ( ImplWriteHeader() )
            ImplWriteBody();

        Bitmap::ReleaseAccess( mpAcc );
    }
    else
        mbStatus = false;

    m_rOStm.SetEndian( aOStmOldModus );

    if ( xStatusIndicator.is() )
        xStatusIndicator->end();

    return mbStatus;
}



bool PBMWriter::ImplWriteHeader()
{
    mnWidth = mpAcc->Width();
    mnHeight = mpAcc->Height();
    if ( mnWidth && mnHeight )
    {
        if ( mnMode == 0 )
            m_rOStm.WriteCharPtr( "P4\x0a" );
        else
            m_rOStm.WriteCharPtr( "P1\x0a" );

        ImplWriteNumber( mnWidth );
        m_rOStm.WriteUChar( 32 );
        ImplWriteNumber( mnHeight );
        m_rOStm.WriteUChar( 10 );
    }
    else mbStatus = false;
    return mbStatus;
}



void PBMWriter::ImplWriteBody()
{
    if ( mnMode == 0 )
    {
        sal_uInt8   nBYTE = 0;
        for ( sal_uLong y = 0; y < mnHeight; y++ )
        {
            sal_uLong x;
            for ( x = 0; x < mnWidth; x++ )
            {
                nBYTE <<= 1;
                if (!(mpAcc->GetPixelIndex( y, x ) & 1 ) )
                    nBYTE++;
                if ( ( x & 7 ) == 7 )
                    m_rOStm.WriteUChar( nBYTE );
            }
            if ( ( x & 7 ) != 0 )
                m_rOStm.WriteUChar( nBYTE << ( ( x ^ 7 ) + 1 ) );
        }
    }
    else
    {
        for ( sal_uLong y = 0; y < mnHeight; y++ )
        {
            int nxCount = 70;
            for ( sal_uLong x = 0; x < mnWidth; x++ )
            {
                if (!( --nxCount ) )
                {
                    nxCount = 69;
                    m_rOStm.WriteUChar( 10 );
                }
                m_rOStm.WriteUChar( ( mpAcc->GetPixelIndex( y, x ) ^ 1 ) + '0' ) ;
            }
            m_rOStm.WriteUChar( 10 );
        }
    }
}


// A decimal number in ascii format is written in the stream.

void PBMWriter::ImplWriteNumber(sal_Int32 nNumber)
{
    const OString aNum(OString::number(nNumber));
    m_rOStm.WriteCharPtr( aNum.getStr() );
}




// - exported function -



extern "C" SAL_DLLPUBLIC_EXPORT bool SAL_CALL
epbGraphicExport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pFilterConfigItem )
{
    PBMWriter aPBMWriter(rStream);

    return aPBMWriter.WritePBM( rGraphic, pFilterConfigItem );
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
