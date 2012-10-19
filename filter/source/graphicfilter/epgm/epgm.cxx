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

//============================ PGMWriter ==================================

class PGMWriter {

private:

    SvStream*           mpOStm;             // Die auszugebende PGM-Datei
    sal_uInt16              mpOStmOldModus;

    sal_Bool                mbStatus;
    sal_uInt32              mnMode;
    BitmapReadAccess*   mpAcc;
    sal_uLong               mnWidth, mnHeight;  // Bildausmass in Pixeln

    sal_Bool                ImplWriteHeader();
    void                ImplWriteBody();
    void                ImplWriteNumber( sal_Int32 );

    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > xStatusIndicator;

public:
                        PGMWriter();
                        ~PGMWriter();

    sal_Bool                WritePGM( const Graphic& rGraphic, SvStream& rPGM, FilterConfigItem* pFilterConfigItem );
};

//=================== Methoden von PGMWriter ==============================

PGMWriter::PGMWriter() :
    mbStatus    ( sal_True ),
    mpAcc       ( NULL )
{
}

// ------------------------------------------------------------------------

PGMWriter::~PGMWriter()
{
}

// ------------------------------------------------------------------------

sal_Bool PGMWriter::WritePGM( const Graphic& rGraphic, SvStream& rPGM, FilterConfigItem* pFilterConfigItem )
{

    mpOStm = &rPGM;

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
    aBmp.Convert( BMP_CONVERSION_8BIT_GREYS );

    mpOStmOldModus = mpOStm->GetNumberFormatInt();
    mpOStm->SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );

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

    mpOStm->SetNumberFormatInt( mpOStmOldModus );

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
            *mpOStm << "P5\x0a";
        else
            *mpOStm << "P2\x0a";

        ImplWriteNumber( mnWidth );
        *mpOStm << (sal_uInt8)32;
        ImplWriteNumber( mnHeight );
        *mpOStm << (sal_uInt8)32;
        ImplWriteNumber( 255 );         // max. gray value
        *mpOStm << (sal_uInt8)10;
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
                *mpOStm << mpAcc->GetPixelIndex( y, x );
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
                    *mpOStm << (sal_uInt8)10;
                }
                nDat = mpAcc->GetPixelIndex( y, x );
                nNumb = nDat / 100;
                if ( nNumb )
                {
                    *mpOStm << (sal_uInt8)( nNumb + '0' );
                    nDat -= ( nNumb * 100 );
                    nNumb = nDat / 10;
                    *mpOStm << (sal_uInt8)( nNumb + '0' );
                    nDat -= ( nNumb * 10 );
                    *mpOStm << (sal_uInt8)( nDat + '0' );
                    nCount -= 4;
                }
                else
                {
                    nNumb = nDat / 10;
                    if ( nNumb )
                    {
                        *mpOStm << (sal_uInt8)( nNumb + '0' );
                        nDat -= ( nNumb * 10 );
                        *mpOStm << (sal_uInt8)( nDat + '0' );
                        nCount -= 3;
                    }
                    else
                    {
                        *mpOStm << (sal_uInt8)( nDat + '0' );
                        nCount -= 2;
                    }
                }
                *mpOStm << (sal_uInt8)' ';
            }
            *mpOStm << (sal_uInt8)10;
        }
    }
}

// ------------------------------------------------------------------------
// eine Dezimalzahl im ASCII format wird in den Stream geschrieben

void PGMWriter::ImplWriteNumber( sal_Int32 nNumber )
{
    const ByteString aNum( ByteString::CreateFromInt32( nNumber ) );

    for( sal_Int16 n = 0UL, nLen = aNum.Len(); n < nLen; n++  )
        *mpOStm << aNum.GetChar( n );

}

// ------------------------------------------------------------------------

// ---------------------
// - exported function -
// ---------------------

extern "C" sal_Bool __LOADONCALLAPI GraphicExport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pFilterConfigItem, sal_Bool )
{
    PGMWriter aPGMWriter;

    return aPGMWriter.WritePGM( rGraphic, rStream, pFilterConfigItem );
}

// ------------------------------------------------------------------------
