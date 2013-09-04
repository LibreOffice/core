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

//============================ PGMWriter ==================================

class PGMWriter {

private:

    SvStream&           m_rOStm;            // the output PGM file
    sal_uInt16          mpOStmOldModus;

    sal_Bool            mbStatus;
    sal_uInt32          mnMode;
    BitmapReadAccess*   mpAcc;
    sal_uLong           mnWidth, mnHeight;  // image size in pixeln

    sal_Bool            ImplWriteHeader();
    void                ImplWriteBody();
    void                ImplWriteNumber( sal_Int32 );

    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > xStatusIndicator;

public:
    PGMWriter(SvStream &rStream);
    ~PGMWriter();

    sal_Bool WritePGM( const Graphic& rGraphic, FilterConfigItem* pFilterConfigItem );
};

//=================== Methoden von PGMWriter ==============================

PGMWriter::PGMWriter(SvStream &rStream)
    : m_rOStm(rStream)
    , mbStatus(sal_True)
    , mpAcc(NULL)
{
}

// ------------------------------------------------------------------------

PGMWriter::~PGMWriter()
{
}

// ------------------------------------------------------------------------

sal_Bool PGMWriter::WritePGM( const Graphic& rGraphic, FilterConfigItem* pFilterConfigItem )
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
    aBmp.Convert( BMP_CONVERSION_8BIT_GREYS );

    mpOStmOldModus = m_rOStm.GetNumberFormatInt();
    m_rOStm.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );

    mpAcc = aBmp.AcquireReadAccess();
    if( mpAcc )
    {
        if ( ImplWriteHeader() )
        {
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

sal_Bool PGMWriter::ImplWriteHeader()
{
    mnWidth = mpAcc->Width();
    mnHeight = mpAcc->Height();
    if ( mnWidth && mnHeight )
    {
        if ( mnMode == 0 )
            m_rOStm << "P5\x0a";
        else
            m_rOStm << "P2\x0a";

        ImplWriteNumber( mnWidth );
        m_rOStm << (sal_uInt8)32;
        ImplWriteNumber( mnHeight );
        m_rOStm << (sal_uInt8)32;
        ImplWriteNumber( 255 );         // max. gray value
        m_rOStm << (sal_uInt8)10;
    }
    else
        mbStatus = sal_False;

    return mbStatus;
}

// ------------------------------------------------------------------------

void PGMWriter::ImplWriteBody()
{
    if ( mnMode == 0 )
    {
        for ( sal_uLong y = 0; y < mnHeight; y++ )
        {
            for ( sal_uLong x = 0; x < mnWidth; x++ )
            {
                m_rOStm << mpAcc->GetPixelIndex( y, x );
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
                sal_uInt8 nDat, nNumb;
                if ( nCount < 0 )
                {
                    nCount = 69;
                    m_rOStm << (sal_uInt8)10;
                }
                nDat = mpAcc->GetPixelIndex( y, x );
                nNumb = nDat / 100;
                if ( nNumb )
                {
                    m_rOStm << (sal_uInt8)( nNumb + '0' );
                    nDat -= ( nNumb * 100 );
                    nNumb = nDat / 10;
                    m_rOStm << (sal_uInt8)( nNumb + '0' );
                    nDat -= ( nNumb * 10 );
                    m_rOStm << (sal_uInt8)( nDat + '0' );
                    nCount -= 4;
                }
                else
                {
                    nNumb = nDat / 10;
                    if ( nNumb )
                    {
                        m_rOStm << (sal_uInt8)( nNumb + '0' );
                        nDat -= ( nNumb * 10 );
                        m_rOStm << (sal_uInt8)( nDat + '0' );
                        nCount -= 3;
                    }
                    else
                    {
                        m_rOStm << (sal_uInt8)( nDat + '0' );
                        nCount -= 2;
                    }
                }
                m_rOStm << (sal_uInt8)' ';
            }
            m_rOStm << (sal_uInt8)10;
        }
    }
}

// ------------------------------------------------------------------------
// write a decimal number in ascii format into the stream
void PGMWriter::ImplWriteNumber(sal_Int32 nNumber)
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
#define GraphicExport epgGraphicExport
#endif

extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL
GraphicExport(SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pFilterConfigItem, sal_Bool)
{
    PGMWriter aPGMWriter(rStream);

    return aPGMWriter.WritePGM( rGraphic, pFilterConfigItem );
}

// ------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
