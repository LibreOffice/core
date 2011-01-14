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

#include <vcl/svapp.hxx>
#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/msgbox.hxx>
#include <svl/solar.hrc>
#include <svtools/fltcall.hxx>
#include <svtools/FilterConfigItem.hxx>

//============================ PBMWriter ==================================

class PBMWriter {

private:

    SvStream*           mpOStm;             // Die auszugebende PBM-Datei
    sal_uInt16              mpOStmOldModus;

    sal_Bool                mbStatus;
    sal_Int32           mnMode;             // 0 -> raw, 1-> ascii
    BitmapReadAccess*   mpAcc;
    sal_uLong               mnWidth, mnHeight;  // Bildausmass in Pixeln

    sal_Bool                ImplWriteHeader();
    void                ImplWriteBody();
    void                ImplWriteNumber( sal_Int32 );

    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > xStatusIndicator;

public:
                        PBMWriter();
                        ~PBMWriter();

    sal_Bool                WritePBM( const Graphic& rGraphic, SvStream& rPBM, FilterConfigItem* pFilterConfigItem );
};

//=================== Methoden von PBMWriter ==============================

PBMWriter::PBMWriter() :
    mbStatus    ( sal_True ),
    mpAcc       ( NULL )
{
}

// ------------------------------------------------------------------------

PBMWriter::~PBMWriter()
{
}

// ------------------------------------------------------------------------

sal_Bool PBMWriter::WritePBM( const Graphic& rGraphic, SvStream& rPBM, FilterConfigItem* pFilterConfigItem )
{

    mpOStm = &rPBM;

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
    aBmp.Convert( BMP_CONVERSION_1BIT_THRESHOLD );

    mpOStmOldModus = mpOStm->GetNumberFormatInt();
    mpOStm->SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );

    mpAcc = aBmp.AcquireReadAccess();
    if( mpAcc )
    {
        if ( ImplWriteHeader() )
            ImplWriteBody();

        aBmp.ReleaseAccess( mpAcc );
    }
    else
        mbStatus = sal_False;

    mpOStm->SetNumberFormatInt( mpOStmOldModus );

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
            *mpOStm << "P4\x0a";
        else
            *mpOStm << "P1\x0a";

        ImplWriteNumber( mnWidth );
        *mpOStm << (sal_uInt8)32;
        ImplWriteNumber( mnHeight );
        *mpOStm << (sal_uInt8)10;
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
                if (!(mpAcc->GetPixel( y, x ) & 1 ) )
                    nBYTE++;
                if ( ( x & 7 ) == 7 )
                    *mpOStm << nBYTE;
            }
            if ( ( x & 7 ) != 0 )
                *mpOStm << (sal_uInt8)( nBYTE << ( ( x ^ 7 ) + 1 ) );
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
                    *mpOStm << (sal_uInt8)10;
                }
                *mpOStm << (sal_uInt8)( ( mpAcc->GetPixel( y, x ) ^ 1 ) + '0' ) ;
            }
            *mpOStm << (sal_uInt8)10;
        }
    }
}

// ------------------------------------------------------------------------
// eine Dezimalzahl im ASCII format wird in den Stream geschrieben

void PBMWriter::ImplWriteNumber( sal_Int32 nNumber )
{
    const ByteString aNum( ByteString::CreateFromInt32( nNumber ) );

    for( sal_Int16 n = 0, nLen = aNum.Len(); n < nLen; n++ )
        *mpOStm << aNum.GetChar( n );

}

// ------------------------------------------------------------------------

// ---------------------
// - exported function -
// ---------------------

extern "C" sal_Bool __LOADONCALLAPI GraphicExport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pFilterConfigItem, sal_Bool )
{
    PBMWriter aPBMWriter;

    return aPBMWriter.WritePBM( rGraphic, rStream, pFilterConfigItem );
}

// ------------------------------------------------------------------------
