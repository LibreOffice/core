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

#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#include <svtools/fltcall.hxx>
#include <svtools/FilterConfigItem.hxx>

//============================ RASWriter ==================================

class RASWriter {

private:

    SvStream*           mpOStm;
    sal_uInt16              mpOStmOldModus;

    sal_Bool                mbStatus;
    BitmapReadAccess*   mpAcc;

    sal_uLong               mnWidth, mnHeight;
    sal_uInt16              mnColors, mnDepth;

    sal_uLong               mnRepCount;
    sal_uInt8               mnRepVal;

    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > xStatusIndicator;

    void                ImplCallback( sal_uLong nCurrentYPos );
    sal_Bool                ImplWriteHeader();
    void                ImplWritePalette();
    void                ImplWriteBody();
    void                ImplPutByte( sal_uInt8 );   // RLE decoding

public:
                        RASWriter();
                        ~RASWriter();

    sal_Bool                WriteRAS( const Graphic& rGraphic, SvStream& rRAS, FilterConfigItem* pFilterConfigItem );
};

//=================== Methoden von RASWriter ==============================

RASWriter::RASWriter() :
    mbStatus    ( sal_True ),
    mpAcc       ( NULL ),
    mnRepCount  ( 0xffffffff )
{
}

// ------------------------------------------------------------------------

RASWriter::~RASWriter()
{
}

// ------------------------------------------------------------------------

void RASWriter::ImplCallback( sal_uLong nYPos )
{
    if ( xStatusIndicator.is() )
        xStatusIndicator->setValue( (sal_uInt16)( ( 100 * nYPos ) / mnHeight ) );
}

//  ------------------------------------------------------------------------

sal_Bool RASWriter::WriteRAS( const Graphic& rGraphic, SvStream& rRAS, FilterConfigItem* pFilterConfigItem)
{
    Bitmap  aBmp;

    mpOStm = &rRAS;

    if ( pFilterConfigItem )
    {
        xStatusIndicator = pFilterConfigItem->GetStatusIndicator();
        if ( xStatusIndicator.is() )
        {
            rtl::OUString aMsg;
            xStatusIndicator->start( aMsg, 100 );
        }
    }

    BitmapEx    aBmpEx( rGraphic.GetBitmapEx() );
    aBmp = aBmpEx.GetBitmap();

    if ( aBmp.GetBitCount() == 4 )
        aBmp.Convert( BMP_CONVERSION_8BIT_COLORS );

    mnDepth = aBmp.GetBitCount();

    // export code below only handles three discrete cases
    mnDepth = mnDepth <= 1 ? 1 : mnDepth <= 8 ? 8 : 24;

    mpAcc = aBmp.AcquireReadAccess();
    if ( mpAcc )
    {
        mpOStmOldModus = mpOStm->GetNumberFormatInt();
        mpOStm->SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );

        if ( ImplWriteHeader() )
        {
            if ( mnDepth <= 8 )
                ImplWritePalette();
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

sal_Bool RASWriter::ImplWriteHeader()
{
    mnWidth = mpAcc->Width();
    mnHeight = mpAcc->Height();
    if ( mnDepth <= 8 )
    {
        mnColors = mpAcc->GetPaletteEntryCount();
        if (mnColors == 0)
            mbStatus = sal_False;
    }
        if ( mbStatus && mnWidth && mnHeight && mnDepth )
    {
        *mpOStm << (sal_uInt32)0x59a66a95 << (sal_uInt32)mnWidth << (sal_uInt32)mnHeight
            << (sal_uInt32)mnDepth
            << (sal_uInt32)(( ( ( ( mnWidth * mnDepth ) + 15 ) >> 4 ) << 1 ) * mnHeight)
            << (sal_uInt32)2;

        if ( mnDepth > 8 )
            *mpOStm << (sal_uInt32)0 << (sal_uInt32)0;
        else
        {

            *mpOStm << (sal_uInt32)1 << (sal_uInt32)( mnColors * 3 );
        }
    }
    else mbStatus = sal_False;

    return mbStatus;
}

// ------------------------------------------------------------------------

void RASWriter::ImplWritePalette()
{
    sal_uInt16 i;

    for ( i = 0; i < mnColors; *mpOStm << mpAcc->GetPaletteColor( i++ ).GetRed() ) ;
    for ( i = 0; i < mnColors; *mpOStm << mpAcc->GetPaletteColor( i++ ).GetGreen() ) ;
    for ( i = 0; i < mnColors; *mpOStm << mpAcc->GetPaletteColor( i++ ).GetBlue() ) ;
}

// ------------------------------------------------------------------------

void RASWriter::ImplWriteBody()
{
    sal_uLong   x, y;

    if ( mnDepth == 24 )
    {
        for ( y = 0; y < mnHeight; y++ )
        {
            ImplCallback( y );                              // processing output
            for ( x = 0; x < mnWidth; x++ )
            {
                BitmapColor aColor( mpAcc->GetPixel( y, x ) );
                ImplPutByte( aColor.GetBlue() );            // Format ist BGR
                ImplPutByte( aColor.GetGreen() );
                ImplPutByte( aColor.GetRed() );
            }
            if ( x & 1 ) ImplPutByte( 0 );      // WORD ALIGNMENT ???
        }
    }
    else if ( mnDepth == 8 )
    {
        for ( y = 0; y < mnHeight; y++ )
        {
            ImplCallback( y );                              // processing output
            for ( x = 0; x < mnWidth; x++ )
            {
                ImplPutByte ( mpAcc->GetPixelIndex( y, x ) );
            }
            if ( x & 1 ) ImplPutByte( 0 );      // WORD ALIGNMENT ???
        }
    }
    else if ( mnDepth == 1 )
    {
        sal_uInt8 nDat = 0;

        for ( y = 0; y < mnHeight; y++ )
        {
            ImplCallback( y );                              // processing output
            for ( x = 0; x < mnWidth; x++ )
            {
                nDat = ( ( nDat << 1 ) | ( mpAcc->GetPixelIndex( y, x ) & 1 ) );
                if ( ( x & 7 ) == 7 )
                    ImplPutByte( nDat );
            }
            if ( x & 7 )
                ImplPutByte( sal::static_int_cast< sal_uInt8 >(nDat << ( ( ( x & 7 ) ^ 7 ) + 1)) );// write remaining bits
            if (!( ( x - 1 ) & 0x8 ) )
                ImplPutByte( 0 );               // WORD ALIGNMENT ???
        }
    }
    ImplPutByte( mnRepVal + 1 );    // end of RLE decoding
}

// ------------------------------------------------------------------------

void RASWriter::ImplPutByte( sal_uInt8 nPutThis )
{
    if ( mnRepCount == 0xffffffff )
    {
        mnRepCount = 0;
        mnRepVal = nPutThis;
    }
    else
    {
        if ( ( nPutThis == mnRepVal ) && ( mnRepCount != 0xff ) )
            mnRepCount++;
        else
        {
            if ( mnRepCount == 0 )
            {
                *mpOStm << (sal_uInt8)mnRepVal;
                if ( mnRepVal == 0x80 )
                    *mpOStm << (sal_uInt8)0;
            }
            else
            {
                *mpOStm << (sal_uInt8)0x80;
                *mpOStm << (sal_uInt8)mnRepCount;
                *mpOStm << (sal_uInt8)mnRepVal;
            }
            mnRepVal = nPutThis;
            mnRepCount = 0;
        }
    }
}

// ------------------------------------------------------------------------

// ---------------------
// - exported function -
// ---------------------

extern "C" sal_Bool __LOADONCALLAPI GraphicExport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pFilterConfigItem, sal_Bool )
{
    RASWriter aRASWriter;

    return aRASWriter.WriteRAS( rGraphic, rStream, pFilterConfigItem );
}
