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
#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/msgbox.hxx>
#include <svl/solar.hrc>
#include <vcl/fltcall.hxx>
#include <vcl/FilterConfigItem.hxx>

//============================ PBMWriter ==================================

class PBMWriter {

private:

    SvStream&           m_rOStm;            // the output PBM file
    sal_uInt16          mpOStmOldModus;

    sal_Bool            mbStatus;
    sal_Int32           mnMode;             // 0 -> raw, 1-> ascii
    BitmapReadAccess*   mpAcc;
    sal_uLong           mnWidth, mnHeight;  // size in pixel

    sal_Bool            ImplWriteHeader();
    void                ImplWriteBody();
    void                ImplWriteNumber( sal_Int32 );

    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > xStatusIndicator;

public:
    PBMWriter(SvStream &rPBM);
    ~PBMWriter();

    sal_Bool WritePBM( const Graphic& rGraphic, FilterConfigItem* pFilterConfigItem );
};

//=================== Methods of PBMWriter ==============================

PBMWriter::PBMWriter(SvStream &rPBM)
    : m_rOStm(rPBM)
    , mbStatus(sal_True)
    , mpAcc(NULL)
{
}

// ------------------------------------------------------------------------

PBMWriter::~PBMWriter()
{
}

// ------------------------------------------------------------------------

sal_Bool PBMWriter::WritePBM( const Graphic& rGraphic, FilterConfigItem* pFilterConfigItem )
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

    mpOStmOldModus = m_rOStm.GetNumberFormatInt();
    m_rOStm.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );

    mpAcc = aBmp.AcquireReadAccess();
    if( mpAcc )
    {
        if ( ImplWriteHeader() )
            ImplWriteBody();

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

sal_Bool PBMWriter::ImplWriteHeader()
{
    mnWidth = mpAcc->Width();
    mnHeight = mpAcc->Height();
    if ( mnWidth && mnHeight )
    {
        if ( mnMode == 0 )
            m_rOStm << "P4\x0a";
        else
            m_rOStm << "P1\x0a";

        ImplWriteNumber( mnWidth );
        m_rOStm << (sal_uInt8)32;
        ImplWriteNumber( mnHeight );
        m_rOStm << (sal_uInt8)10;
    }
    else mbStatus = sal_False;
    return mbStatus;
}

// ------------------------------------------------------------------------

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
                    m_rOStm << nBYTE;
            }
            if ( ( x & 7 ) != 0 )
                m_rOStm << (sal_uInt8)( nBYTE << ( ( x ^ 7 ) + 1 ) );
        }
    }
    else
    {
        int nxCount;
        for ( sal_uLong y = 0; y < mnHeight; y++ )
        {
            nxCount = 70;
            for ( sal_uLong x = 0; x < mnWidth; x++ )
            {
                if (!( --nxCount ) )
                {
                    nxCount = 69;
                    m_rOStm << (sal_uInt8)10;
                }
                m_rOStm << (sal_uInt8)( ( mpAcc->GetPixelIndex( y, x ) ^ 1 ) + '0' ) ;
            }
            m_rOStm << (sal_uInt8)10;
        }
    }
}

// ------------------------------------------------------------------------
// A decimal number in ascii format is written in the stream.

void PBMWriter::ImplWriteNumber(sal_Int32 nNumber)
{
    const OString aNum(OString::number(nNumber));
    m_rOStm << aNum.getStr();
}

// ------------------------------------------------------------------------

// ---------------------
// - exported function -
// ---------------------

// this needs to be kept in sync with
// ImpFilterLibCacheEntry::GetImportFunction() from
// vcl/source/filter/graphicfilter.cxx
#if defined(DISABLE_DYNLOADING)
#define GraphicExport epbGraphicExport
#endif

extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL
GraphicExport(SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pFilterConfigItem, sal_Bool)
{
    PBMWriter aPBMWriter(rStream);

    return aPBMWriter.WritePBM( rGraphic, pFilterConfigItem );
}

// ------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
