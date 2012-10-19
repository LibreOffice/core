/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
                if (!(mpAcc->GetPixelIndex( y, x ) & 1 ) )
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
                *mpOStm << (sal_uInt8)( ( mpAcc->GetPixelIndex( y, x ) ^ 1 ) + '0' ) ;
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
