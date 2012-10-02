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
#include <svtools/fltcall.hxx>
#include <svtools/FilterConfigItem.hxx>
#include "giflzwc.hxx"

// -------------
// - GIFWriter -
// -------------

class GIFWriter
{
    Bitmap              aAccBmp;
    SvStream& m_rGIF;
    BitmapReadAccess*   m_pAcc;
    sal_uLong               nMinPercent;
    sal_uLong               nMaxPercent;
    sal_uLong               nLastPercent;
    long                nActX;
    long                nActY;
    sal_Int32           nInterlaced;
    sal_Bool                bStatus;
    sal_Bool                bTransparent;

    void                MayCallback( sal_uLong nPercent );
    void                WriteSignature( sal_Bool bGIF89a );
    void                WriteGlobalHeader( const Size& rSize );
    void                WriteLoopExtension( const Animation& rAnimation );
    void                WriteLogSizeExtension( const Size& rSize100 );
    void                WriteImageExtension( long nTimer, Disposal eDisposal );
    void                WriteLocalHeader();
    void                WritePalette();
    void                WriteAccess();
    void                WriteTerminator();

    sal_Bool                CreateAccess( const BitmapEx& rBmpEx );
    void                DestroyAccess();

    void                WriteAnimation( const Animation& rAnimation );
    void                WriteBitmapEx( const BitmapEx& rBmpEx, const Point& rPoint, sal_Bool bExtended,
                                       long nTimer = 0, Disposal eDisposal = DISPOSE_NOT );

    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > xStatusIndicator;

public:

    GIFWriter(SvStream &rStream);
    ~GIFWriter() {}

    sal_Bool WriteGIF( const Graphic& rGraphic, FilterConfigItem* pConfigItem );
};

GIFWriter::GIFWriter(SvStream &rStream)
    : m_rGIF(rStream)
    , m_pAcc(NULL)
    , nActX(0)
    , nActY(0)
{
}

// ------------------------------------------------------------------------

sal_Bool GIFWriter::WriteGIF(const Graphic& rGraphic, FilterConfigItem* pFilterConfigItem)
{
    if ( pFilterConfigItem )
    {
        xStatusIndicator = pFilterConfigItem->GetStatusIndicator();
        if ( xStatusIndicator.is() )
        {
            rtl::OUString aMsg;
            xStatusIndicator->start( aMsg, 100 );
        }
    }

    Size            aSize100;
    const MapMode   aMap( rGraphic.GetPrefMapMode() );
    sal_Bool            bLogSize = ( aMap.GetMapUnit() != MAP_PIXEL );

    if( bLogSize )
        aSize100 = Application::GetDefaultDevice()->LogicToLogic( rGraphic.GetPrefSize(), aMap, MAP_100TH_MM );

    bStatus = sal_True;
    nLastPercent = 0;
    nInterlaced = 0;
    m_pAcc = NULL;

    if ( pFilterConfigItem )
        nInterlaced = pFilterConfigItem->ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Interlaced" ) ), 0 );

    m_rGIF.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    if( rGraphic.IsAnimated() )
    {
        const Animation& rAnimation = rGraphic.GetAnimation();

        WriteSignature( sal_True );

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
        const sal_Bool bGrafTrans = rGraphic.IsTransparent();

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

// ------------------------------------------------------------------------

void GIFWriter::WriteBitmapEx( const BitmapEx& rBmpEx, const Point& rPoint,
                               sal_Bool bExtended, long nTimer, Disposal eDisposal )
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

// ------------------------------------------------------------------------

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

            WriteBitmapEx( rAnimBmp.aBmpEx, rAnimBmp.aPosPix, sal_True,
                           rAnimBmp.nWait, rAnimBmp.eDisposal );
            nMinPercent = nMaxPercent;
            nMaxPercent = (sal_uLong) ( nMaxPercent + fStep );
        }
    }
}

// ------------------------------------------------------------------------

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

// ------------------------------------------------------------------------

sal_Bool GIFWriter::CreateAccess( const BitmapEx& rBmpEx )
{
    if( bStatus )
    {
        Bitmap aMask( rBmpEx.GetMask() );

        aAccBmp = rBmpEx.GetBitmap();
        bTransparent = sal_False;

        if( !!aMask )
        {
            if( aAccBmp.Convert( BMP_CONVERSION_8BIT_TRANS ) )
            {
                aMask.Convert( BMP_CONVERSION_1BIT_THRESHOLD );
                aAccBmp.Replace( aMask, BMP_COL_TRANS );
                bTransparent = sal_True;
            }
            else
                aAccBmp.Convert( BMP_CONVERSION_8BIT_COLORS );
        }
        else
            aAccBmp.Convert( BMP_CONVERSION_8BIT_COLORS );

        m_pAcc = aAccBmp.AcquireReadAccess();

        if( !m_pAcc )
            bStatus = sal_False;
    }

    return bStatus;
}

// ------------------------------------------------------------------------

void GIFWriter::DestroyAccess()
{
    aAccBmp.ReleaseAccess( m_pAcc );
    m_pAcc = NULL;
}

// ------------------------------------------------------------------------

void GIFWriter::WriteSignature( sal_Bool bGIF89a )
{
    if( bStatus )
    {
        m_rGIF.Write( bGIF89a ? "GIF89a" : "GIF87a" , 6 );

        if( m_rGIF.GetError() )
            bStatus = sal_False;
    }
}

// ------------------------------------------------------------------------

void GIFWriter::WriteGlobalHeader( const Size& rSize )
{
    if( bStatus )
    {
        // 256 colors
        const sal_uInt16    nWidth = (sal_uInt16) rSize.Width();
        const sal_uInt16    nHeight = (sal_uInt16) rSize.Height();
        const sal_uInt8     cFlags = 128 | ( 7 << 4 );

        // write values
        m_rGIF << nWidth;
        m_rGIF << nHeight;
        m_rGIF << cFlags;
        m_rGIF << (sal_uInt8) 0x00;
        m_rGIF << (sal_uInt8) 0x00;
        
        // write dummy palette with two entries (black/white);
        // we do this only because of a bug in Photoshop, since those can't
        // read pictures without a global color palette
        m_rGIF << (sal_uInt16) 0;
        m_rGIF << (sal_uInt16) 255;
        m_rGIF << (sal_uInt16) 65535;

        if( m_rGIF.GetError() )
            bStatus = sal_False;
    }
}

// ------------------------------------------------------------------------

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

        m_rGIF << (sal_uInt8) 0x21;
        m_rGIF << (sal_uInt8) 0xff;
        m_rGIF << (sal_uInt8) 0x0b;
        m_rGIF.Write( "NETSCAPE2.0", 11 );
        m_rGIF << (sal_uInt8) 0x03;
        m_rGIF << (sal_uInt8) 0x01;
        m_rGIF << cLoByte;
        m_rGIF << cHiByte;
        m_rGIF << (sal_uInt8) 0x00;
    }
}

// ------------------------------------------------------------------------

void GIFWriter::WriteLogSizeExtension( const Size& rSize100 )
{
    // writer PrefSize in 100th-mm as ApplicationExtension
    if( rSize100.Width() && rSize100.Height() )
    {
        m_rGIF << (sal_uInt8) 0x21;
        m_rGIF << (sal_uInt8) 0xff;
        m_rGIF << (sal_uInt8) 0x0b;
        m_rGIF.Write( "STARDIV 5.0", 11 );
        m_rGIF << (sal_uInt8) 0x09;
        m_rGIF << (sal_uInt8) 0x01;
        m_rGIF << (sal_uInt32) rSize100.Width();
        m_rGIF << (sal_uInt32) rSize100.Height();
        m_rGIF << (sal_uInt8) 0x00;
    }
}

// ------------------------------------------------------------------------

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
        if( eDisposal == DISPOSE_BACK )
            cFlags |= ( 2 << 2 );
        else if( eDisposal == DISPOSE_PREVIOUS )
            cFlags |= ( 3 << 2 );

        m_rGIF << (sal_uInt8) 0x21;
        m_rGIF << (sal_uInt8) 0xf9;
        m_rGIF << (sal_uInt8) 0x04;
        m_rGIF << cFlags;
        m_rGIF << nDelay;
        m_rGIF << (sal_uInt8) m_pAcc->GetBestPaletteIndex( BMP_COL_TRANS );
        m_rGIF << (sal_uInt8) 0x00;

        if( m_rGIF.GetError() )
            bStatus = sal_False;
    }
}

// ------------------------------------------------------------------------

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
        m_rGIF << (sal_uInt8) 0x2c;
        m_rGIF << nPosX;
        m_rGIF << nPosY;
        m_rGIF << nWidth;
        m_rGIF << nHeight;
        m_rGIF << cFlags;

        if( m_rGIF.GetError() )
            bStatus = sal_False;
    }
}

// ------------------------------------------------------------------------

void GIFWriter::WritePalette()
{
    if( bStatus && m_pAcc->HasPalette() )
    {
        const sal_uInt16 nCount = m_pAcc->GetPaletteEntryCount();
        const sal_uInt16 nMaxCount = ( 1 << m_pAcc->GetBitCount() );

        for ( sal_uInt16 i = 0; i < nCount; i++ )
        {
            const BitmapColor& rColor = m_pAcc->GetPaletteColor( i );

            m_rGIF << rColor.GetRed();
            m_rGIF << rColor.GetGreen();
            m_rGIF << rColor.GetBlue();
        }

        // fill up the rest with 0
        if( nCount < nMaxCount )
            m_rGIF.SeekRel( ( nMaxCount - nCount ) * 3 );

        if( m_rGIF.GetError() )
            bStatus = sal_False;
    }
}

// ------------------------------------------------------------------------

void GIFWriter::WriteAccess()
{
    GIFLZWCompressor    aCompressor;
    const long          nWidth = m_pAcc->Width();
    const long          nHeight = m_pAcc->Height();
    sal_uInt8*              pBuffer = NULL;
    const sal_uLong         nFormat = m_pAcc->GetScanlineFormat();
    sal_Bool                bNative = ( BMP_FORMAT_8BIT_PAL == nFormat );

    if( !bNative )
        pBuffer = new sal_uInt8[ nWidth ];

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
                    pBuffer[ nX ] = (sal_uInt8) m_pAcc->GetPixel( nY, nX );

                aCompressor.Compress( pBuffer, nWidth );
            }

            if ( m_rGIF.GetError() )
                bStatus = sal_False;

            MayCallback( nMinPercent + ( nMaxPercent - nMinPercent ) * i / nHeight );

            if( !bStatus )
                break;
        }

        aCompressor.EndCompression();

        if ( m_rGIF.GetError() )
            bStatus = sal_False;
    }

    delete[] pBuffer;
}

// ------------------------------------------------------------------------

void GIFWriter::WriteTerminator()
{
    if( bStatus )
    {
        m_rGIF << (sal_uInt8) 0x3b;

        if( m_rGIF.GetError() )
            bStatus = sal_False;
    }
}

// ------------------------------------------------------------------------

extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool __LOADONCALLAPI
GraphicExport( SvStream& rStream, Graphic& rGraphic,
                                               FilterConfigItem* pConfigItem, sal_Bool )
{
    GIFWriter aWriter(rStream);
    return aWriter.WriteGIF(rGraphic, pConfigItem);
}

// ------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
