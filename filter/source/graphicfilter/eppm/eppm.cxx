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


#include <vcl/svapp.hxx>
#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/msgbox.hxx>
#include <svl/solar.hrc>
#include <svtools/fltcall.hxx>
#include <svtools/FilterConfigItem.hxx>

//============================ PPMWriter ==================================

class PPMWriter {

private:

    SvStream& m_rOStm;          // Die auszugebende PPM-Datei
    sal_uInt16              mpOStmOldModus;

    sal_Bool                mbStatus;
    sal_Int32           mnMode;
    BitmapReadAccess*   mpAcc;
    sal_uLong               mnWidth, mnHeight;  // Bildausmass in Pixeln

    sal_Bool                ImplWriteHeader();
    void                ImplWriteBody();
    void                ImplWriteNumber( sal_Int32 );

    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > xStatusIndicator;

public:
                        PPMWriter(SvStream &rStrm);
                        ~PPMWriter();

    sal_Bool                WritePPM( const Graphic& rGraphic, FilterConfigItem* pFilterConfigItem );
};

//=================== Methods of PPMWriter ==============================

PPMWriter::PPMWriter(SvStream &rStrm)
    : m_rOStm(rStrm)
    , mbStatus  ( sal_True )
    , mpAcc     ( NULL )
{
}

// ------------------------------------------------------------------------

PPMWriter::~PPMWriter()
{
}

// ------------------------------------------------------------------------

sal_Bool PPMWriter::WritePPM( const Graphic& rGraphic, FilterConfigItem* pFilterConfigItem )
{
    if ( pFilterConfigItem )
    {
        mnMode = pFilterConfigItem->ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "FileFormat" ) ), 0 );

        xStatusIndicator = pFilterConfigItem->GetStatusIndicator();
        if ( xStatusIndicator.is() )
        {
            rtl::OUString aMsg;
            xStatusIndicator->start( aMsg, 100 );
        }
    }

    BitmapEx    aBmpEx( rGraphic.GetBitmapEx() );
    Bitmap      aBmp = aBmpEx.GetBitmap();
    aBmp.Convert( BMP_CONVERSION_24BIT );

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

sal_Bool PPMWriter::ImplWriteHeader()
{
    mnWidth = mpAcc->Width();
    mnHeight = mpAcc->Height();
    if ( mnWidth && mnHeight )
    {
        if ( mnMode == 0 )
            m_rOStm << "P6\x0a";
        else
            m_rOStm << "P3\x0a";

        ImplWriteNumber( mnWidth );
        m_rOStm << (sal_uInt8)32;
        ImplWriteNumber( mnHeight );
        m_rOStm << (sal_uInt8)32;
        ImplWriteNumber( 255 );         // max. col.
        m_rOStm << (sal_uInt8)10;
    }
    else
        mbStatus = sal_False;

    return mbStatus;
}

// ------------------------------------------------------------------------

void PPMWriter::ImplWriteBody()
{
    if ( mnMode == 0 )
    {
        for ( sal_uLong y = 0; y < mnHeight; y++ )
        {
            for ( sal_uLong x = 0; x < mnWidth; x++ )
            {
                const BitmapColor& rColor = mpAcc->GetPixel( y, x );
                m_rOStm << rColor.GetRed();
                m_rOStm << rColor.GetGreen();
                m_rOStm << rColor.GetBlue();
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
                    m_rOStm << (sal_uInt8)10;
                }
                nDat[0] = mpAcc->GetPixel( y, x ).GetRed();
                nDat[1] = mpAcc->GetPixel( y, x ).GetGreen();
                nDat[2] = mpAcc->GetPixel( y, x ).GetBlue();
                for ( i = 0; i < 3; i++ )
                {
                    nNumb = nDat[ i ] / 100;
                    if ( nNumb )
                    {
                        m_rOStm << (sal_uInt8)( nNumb + '0' );
                        nDat[ i ] -= ( nNumb * 100 );
                        nNumb = nDat[ i ] / 10;
                        m_rOStm << (sal_uInt8)( nNumb + '0' );
                        nDat[ i ] -= ( nNumb * 10 );
                        m_rOStm << (sal_uInt8)( nDat[ i ] + '0' );
                        nCount -= 4;
                    }
                    else
                    {
                        nNumb = nDat[ i ] / 10;
                        if ( nNumb )
                        {
                            m_rOStm << (sal_uInt8)( nNumb + '0' );
                            nDat[ i ] -= ( nNumb * 10 );
                            m_rOStm << (sal_uInt8)( nDat[ i ] + '0' );
                            nCount -= 3;
                        }
                        else
                        {
                            m_rOStm << (sal_uInt8)( nDat[ i ] + '0' );
                            nCount -= 2;
                        }
                    }
                    m_rOStm << (sal_uInt8)' ';
                }
            }
            m_rOStm << (sal_uInt8)10;
        }
    }
}

// ------------------------------------------------------------------------
// a decimal number in ASCII format is being written into the stream

void PPMWriter::ImplWriteNumber(sal_Int32 nNumber)
{
    const rtl::OString aNum(rtl::OString::valueOf(nNumber));
    m_rOStm << aNum.getStr();
}

// ------------------------------------------------------------------------

// ---------------------
// - exported function -
// ---------------------

extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool __LOADONCALLAPI
GraphicExport(SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pFilterConfigItem, sal_Bool)
{
    PPMWriter aPPMWriter(rStream);
    return aPPMWriter.WritePPM( rGraphic, pFilterConfigItem );
}

// ------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
