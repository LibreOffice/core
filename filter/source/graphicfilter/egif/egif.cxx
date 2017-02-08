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


#include <vcl/graph.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/window.hxx>
#include <svl/solar.hrc>
#include <vcl/fltcall.hxx>
#include <vcl/FilterConfigItem.hxx>
#include "giflzwc.hxx"
#include <memory>


class GIFWriter
{
    Bitmap              aAccBmp;
    SvStream& m_rGIF;
    BitmapReadAccess*   m_pAcc;
    sal_uLong           nMinPercent;
    sal_uLong           nMaxPercent;
    sal_uLong           nLastPercent;
    long                nActX;
    long                nActY;
    sal_Int32           nInterlaced;
    bool                bStatus;
    bool                bTransparent;

    void                MayCallback( sal_uLong nPercent );
    void                WriteSignature( bool bGIF89a );
    void                WriteGlobalHeader( const Size& rSize );
    void                WriteLoopExtension( const Animation& rAnimation );
    void                WriteLogSizeExtension( const Size& rSize100 );
    void                WriteImageExtension( long nTimer, Disposal eDisposal );
    void                WriteLocalHeader();
    void                WritePalette();
    void                WriteAccess();
    void                WriteTerminator();

    bool                CreateAccess( const BitmapEx& rBmpEx );
    void                DestroyAccess();

    void                WriteAnimation( const Animation& rAnimation );
    void                WriteBitmapEx( const BitmapEx& rBmpEx, const Point& rPoint, bool bExtended,
                                       long nTimer = 0, Disposal eDisposal = Disposal::Not );

    css::uno::Reference< css::task::XStatusIndicator > xStatusIndicator;

public:

    explicit GIFWriter(SvStream &rStream);

    bool WriteGIF( const Graphic& rGraphic, FilterConfigItem* pConfigItem );
};


GIFWriter::GIFWriter(SvStream &rStream)
    : m_rGIF(rStream)
    , m_pAcc(nullptr)
    , nMinPercent(0)
    , nMaxPercent(0)
    , nLastPercent(0)
    , nActX(0)
    , nActY(0)
    , nInterlaced(0)
    , bStatus(false)
    , bTransparent(false)
{
}


bool GIFWriter::WriteGIF(const Graphic& rGraphic, FilterConfigItem* pFilterConfigItem)
{
    if ( pFilterConfigItem )
    {
        xStatusIndicator = pFilterConfigItem->GetStatusIndicator();
        if ( xStatusIndicator.is() )
        {
            OUString aMsg;
            xStatusIndicator->start( aMsg, 100 );
        }
    }

    Size            aSize100;
    const MapMode   aMap( rGraphic.GetPrefMapMode() );
    bool            bLogSize = ( aMap.GetMapUnit() != MapUnit::MapPixel );

    if( bLogSize )
        aSize100 = OutputDevice::LogicToLogic( rGraphic.GetPrefSize(), aMap, MapUnit::Map100thMM );

    bStatus = true;
    nLastPercent = 0;
    nInterlaced = 0;
    m_pAcc = nullptr;

    if ( pFilterConfigItem )
        nInterlaced = pFilterConfigItem->ReadInt32( "Interlaced", 0 );

    m_rGIF.SetEndian( SvStreamEndian::LITTLE );

    if( rGraphic.IsAnimated() )
    {
        const Animation& rAnimation = rGraphic.GetAnimation();

        WriteSignature( true );

        if ( bStatus )
        {
            WriteGlobalHeader( rAnimation.GetDisplaySizePixel() );

            if( bStatus )
            {
                WriteLoopExtension( rAnimation );

                if( bStatus )
                    WriteAnimation( rAnimation );
            }
        }
    }
    else
    {
        const bool bGrafTrans = rGraphic.IsTransparent();

        BitmapEx aBmpEx;

        if( bGrafTrans )
            aBmpEx = rGraphic.GetBitmapEx();
        else
            aBmpEx = BitmapEx( rGraphic.GetBitmap() );

        nMinPercent = 0;
        nMaxPercent = 100;

        WriteSignature( bGrafTrans || bLogSize );

        if( bStatus )
        {
            WriteGlobalHeader( aBmpEx.GetSizePixel() );

            if( bStatus )
                WriteBitmapEx( aBmpEx, Point(), bGrafTrans );
        }
    }

    if( bStatus )
    {
        if( bLogSize )
            WriteLogSizeExtension( aSize100 );

        WriteTerminator();
    }

    if ( xStatusIndicator.is() )
        xStatusIndicator->end();

    return bStatus;
}


void GIFWriter::WriteBitmapEx( const BitmapEx& rBmpEx, const Point& rPoint,
                               bool bExtended, long nTimer, Disposal eDisposal )
{
    if( CreateAccess( rBmpEx ) )
    {
        nActX = rPoint.X();
        nActY = rPoint.Y();

        if( bExtended )
            WriteImageExtension( nTimer, eDisposal );

        if( bStatus )
        {
            WriteLocalHeader();

            if( bStatus )
            {
                WritePalette();

                if( bStatus )
                    WriteAccess();
            }
        }

        DestroyAccess();
    }
}


void GIFWriter::WriteAnimation( const Animation& rAnimation )
{
    const sal_uInt16    nCount = rAnimation.Count();

    if( nCount )
    {
        const double fStep = 100. / nCount;

        nMinPercent = 0L;
        nMaxPercent = (sal_uLong) fStep;

        for( sal_uInt16 i = 0; i < nCount; i++ )
        {
            const AnimationBitmap& rAnimBmp = rAnimation.Get( i );

            WriteBitmapEx( rAnimBmp.aBmpEx, rAnimBmp.aPosPix, true,
                           rAnimBmp.nWait, rAnimBmp.eDisposal );
            nMinPercent = nMaxPercent;
            nMaxPercent = (sal_uLong) ( nMaxPercent + fStep );
        }
    }
}


void GIFWriter::MayCallback( sal_uLong nPercent )
{
    if ( xStatusIndicator.is() )
    {
        if( nPercent >= nLastPercent + 3 )
        {
            nLastPercent = nPercent;
            if ( nPercent <= 100 )
                xStatusIndicator->setValue( nPercent );
        }
    }
}


bool GIFWriter::CreateAccess( const BitmapEx& rBmpEx )
{
    if( bStatus )
    {
        Bitmap aMask( rBmpEx.GetMask() );

        aAccBmp = rBmpEx.GetBitmap();
        bTransparent = false;

        if( !!aMask )
        {
            if( aAccBmp.Convert( BmpConversion::N8BitTrans ) )
            {
                aMask.Convert( BmpConversion::N1BitThreshold );
                aAccBmp.Replace( aMask, BMP_COL_TRANS );
                bTransparent = true;
            }
            else
                aAccBmp.Convert( BmpConversion::N8BitColors );
        }
        else
            aAccBmp.Convert( BmpConversion::N8BitColors );

        m_pAcc = aAccBmp.AcquireReadAccess();

        if( !m_pAcc )
            bStatus = false;
    }

    return bStatus;
}


void GIFWriter::DestroyAccess()
{
    Bitmap::ReleaseAccess( m_pAcc );
    m_pAcc = nullptr;
}


void GIFWriter::WriteSignature( bool bGIF89a )
{
    if( bStatus )
    {
        m_rGIF.WriteBytes(bGIF89a ? "GIF89a" : "GIF87a" , 6);

        if( m_rGIF.GetError() )
            bStatus = false;
    }
}


void GIFWriter::WriteGlobalHeader( const Size& rSize )
{
    if( bStatus )
    {
        // 256 colors
        const sal_uInt16    nWidth = (sal_uInt16) rSize.Width();
        const sal_uInt16    nHeight = (sal_uInt16) rSize.Height();
        const sal_uInt8     cFlags = 128 | ( 7 << 4 );

        // write values
        m_rGIF.WriteUInt16( nWidth );
        m_rGIF.WriteUInt16( nHeight );
        m_rGIF.WriteUChar( cFlags );
        m_rGIF.WriteUChar( 0x00 );
        m_rGIF.WriteUChar( 0x00 );

        // write dummy palette with two entries (black/white);
        // we do this only because of a bug in Photoshop, since those can't
        // read pictures without a global color palette
        m_rGIF.WriteUInt16( 0 );
        m_rGIF.WriteUInt16( 255 );
        m_rGIF.WriteUInt16( 65535 );

        if( m_rGIF.GetError() )
            bStatus = false;
    }
}


void GIFWriter::WriteLoopExtension( const Animation& rAnimation )
{
    DBG_ASSERT( rAnimation.Count() > 0, "Animation has no bitmaps!" );

    sal_uInt16 nLoopCount = (sal_uInt16) rAnimation.GetLoopCount();

    // if only one run should take place
    // the LoopExtension won't be written
    // The default in this case is a single run
    if( nLoopCount != 1 )
    {
        // Netscape interprets the LoopCount
        // as the sole number of _repetitions_
        if( nLoopCount )
            nLoopCount--;

        const sal_uInt8 cLoByte = (const sal_uInt8) nLoopCount;
        const sal_uInt8 cHiByte = (const sal_uInt8) ( nLoopCount >> 8 );

        m_rGIF.WriteUChar( 0x21 );
        m_rGIF.WriteUChar( 0xff );
        m_rGIF.WriteUChar( 0x0b );
        m_rGIF.WriteBytes( "NETSCAPE2.0", 11 );
        m_rGIF.WriteUChar( 0x03 );
        m_rGIF.WriteUChar( 0x01 );
        m_rGIF.WriteUChar( cLoByte );
        m_rGIF.WriteUChar( cHiByte );
        m_rGIF.WriteUChar( 0x00 );
    }
}


void GIFWriter::WriteLogSizeExtension( const Size& rSize100 )
{
    // writer PrefSize in 100th-mm as ApplicationExtension
    if( rSize100.Width() && rSize100.Height() )
    {
        m_rGIF.WriteUChar( 0x21 );
        m_rGIF.WriteUChar( 0xff );
        m_rGIF.WriteUChar( 0x0b );
        m_rGIF.WriteBytes( "STARDIV 5.0", 11 );
        m_rGIF.WriteUChar( 0x09 );
        m_rGIF.WriteUChar( 0x01 );
        m_rGIF.WriteUInt32( rSize100.Width() );
        m_rGIF.WriteUInt32( rSize100.Height() );
        m_rGIF.WriteUChar( 0x00 );
    }
}


void GIFWriter::WriteImageExtension( long nTimer, Disposal eDisposal )
{
    if( bStatus )
    {
        const sal_uInt16    nDelay = (sal_uInt16) nTimer;
        sal_uInt8           cFlags = 0;

        // set Transparency-Flag
        if( bTransparent )
            cFlags |= 1;

        // set Disposal-value
        if( eDisposal == Disposal::Back )
            cFlags |= ( 2 << 2 );
        else if( eDisposal == Disposal::Previous )
            cFlags |= ( 3 << 2 );

        m_rGIF.WriteUChar( 0x21 );
        m_rGIF.WriteUChar( 0xf9 );
        m_rGIF.WriteUChar( 0x04 );
        m_rGIF.WriteUChar( cFlags );
        m_rGIF.WriteUInt16( nDelay );
        m_rGIF.WriteUChar( m_pAcc->GetBestPaletteIndex( BMP_COL_TRANS ) );
        m_rGIF.WriteUChar( 0x00 );

        if( m_rGIF.GetError() )
            bStatus = false;
    }
}


void GIFWriter::WriteLocalHeader()
{
    if( bStatus )
    {
        const sal_uInt16    nPosX = (sal_uInt16) nActX;
        const sal_uInt16    nPosY = (sal_uInt16) nActY;
        const sal_uInt16    nWidth = (sal_uInt16) m_pAcc->Width();
        const sal_uInt16    nHeight = (sal_uInt16) m_pAcc->Height();
        sal_uInt8       cFlags = (sal_uInt8) ( m_pAcc->GetBitCount() - 1 );

        // set Interlaced-Flag
        if( nInterlaced )
            cFlags |= 0x40;

        // set Flag for the local color palette
        cFlags |= 0x80;

        // alles rausschreiben
        m_rGIF.WriteUChar( 0x2c );
        m_rGIF.WriteUInt16( nPosX );
        m_rGIF.WriteUInt16( nPosY );
        m_rGIF.WriteUInt16( nWidth );
        m_rGIF.WriteUInt16( nHeight );
        m_rGIF.WriteUChar( cFlags );

        if( m_rGIF.GetError() )
            bStatus = false;
    }
}


void GIFWriter::WritePalette()
{
    if( bStatus && m_pAcc->HasPalette() )
    {
        const sal_uInt16 nCount = m_pAcc->GetPaletteEntryCount();
        const sal_uInt16 nMaxCount = ( 1 << m_pAcc->GetBitCount() );

        for ( sal_uInt16 i = 0; i < nCount; i++ )
        {
            const BitmapColor& rColor = m_pAcc->GetPaletteColor( i );

            m_rGIF.WriteUChar( rColor.GetRed() );
            m_rGIF.WriteUChar( rColor.GetGreen() );
            m_rGIF.WriteUChar( rColor.GetBlue() );
        }

        // fill up the rest with 0
        if( nCount < nMaxCount )
            m_rGIF.SeekRel( ( nMaxCount - nCount ) * 3 );

        if( m_rGIF.GetError() )
            bStatus = false;
    }
}


void GIFWriter::WriteAccess()
{
    GIFLZWCompressor    aCompressor;
    const long          nWidth = m_pAcc->Width();
    const long          nHeight = m_pAcc->Height();
    std::unique_ptr<sal_uInt8[]> pBuffer;
    bool                bNative = m_pAcc->GetScanlineFormat() == ScanlineFormat::N8BitPal;

    if( !bNative )
        pBuffer.reset(new sal_uInt8[ nWidth ]);

    if( bStatus && ( 8 == m_pAcc->GetBitCount() ) && m_pAcc->HasPalette() )
    {
        aCompressor.StartCompression( m_rGIF, m_pAcc->GetBitCount() );

        long nY, nT;

        for( long i = 0; i < nHeight; ++i )
        {
            if( nInterlaced )
            {
                nY = i << 3;

                if( nY >= nHeight )
                {
                    nT = i - ( ( nHeight + 7 ) >> 3 );
                    nY= ( nT << 3 ) + 4;

                    if( nY >= nHeight )
                    {
                        nT -= ( nHeight + 3 ) >> 3;
                        nY = ( nT << 2 ) + 2;

                        if ( nY >= nHeight )
                        {
                            nT -= ( ( nHeight + 1 ) >> 2 );
                            nY = ( nT << 1 ) + 1;
                        }
                    }
                }
            }
            else
                nY = i;

            if( bNative )
                aCompressor.Compress( m_pAcc->GetScanline( nY ), nWidth );
            else
            {
                for( long nX = 0L; nX < nWidth; nX++ )
                    pBuffer[ nX ] = m_pAcc->GetPixelIndex( nY, nX );

                aCompressor.Compress( pBuffer.get(), nWidth );
            }

            if ( m_rGIF.GetError() )
                bStatus = false;

            MayCallback( nMinPercent + ( nMaxPercent - nMinPercent ) * i / nHeight );

            if( !bStatus )
                break;
        }

        aCompressor.EndCompression();

        if ( m_rGIF.GetError() )
            bStatus = false;
    }
}


void GIFWriter::WriteTerminator()
{
    if( bStatus )
    {
        m_rGIF.WriteUChar( 0x3b );

        if( m_rGIF.GetError() )
            bStatus = false;
    }
}


extern "C" SAL_DLLPUBLIC_EXPORT bool SAL_CALL
egiGraphicExport( SvStream& rStream, Graphic& rGraphic, FilterConfigItem* pConfigItem )
{
    GIFWriter aWriter(rStream);
    return aWriter.WriteGIF(rGraphic, pConfigItem);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
