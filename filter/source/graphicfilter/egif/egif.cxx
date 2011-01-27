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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include <vcl/graph.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/window.hxx>
#include <svl/solar.hrc>
#include <svtools/fltcall.hxx>
#include <svtools/FilterConfigItem.hxx>
#include "giflzwc.hxx"
#include "strings.hrc"
#include "dlgegif.hrc"
#include "dlgegif.hxx"

// -------------
// - GIFWriter -
// -------------

class GIFWriter
{
    Bitmap              aAccBmp;
    SvStream& m_rGIF;
    BitmapReadAccess*   m_pAcc;
    ULONG               nMinPercent;
    ULONG               nMaxPercent;
    ULONG               nLastPercent;
    long                nActX;
    long                nActY;
    sal_Int32           nInterlaced;
    BOOL                bStatus;
    BOOL                bTransparent;

    void                MayCallback( ULONG nPercent );
    void                WriteSignature( BOOL bGIF89a );
    void                WriteGlobalHeader( const Size& rSize );
    void                WriteLoopExtension( const Animation& rAnimation );
    void                WriteLogSizeExtension( const Size& rSize100 );
    void                WriteImageExtension( long nTimer, Disposal eDisposal );
    void                WriteLocalHeader();
    void                WritePalette();
    void                WriteAccess();
    void                WriteTerminator();

    BOOL                CreateAccess( const BitmapEx& rBmpEx );
    void                DestroyAccess();

    void                WriteAnimation( const Animation& rAnimation );
    void                WriteBitmapEx( const BitmapEx& rBmpEx, const Point& rPoint, BOOL bExtended,
                                       long nTimer = 0, Disposal eDisposal = DISPOSE_NOT );

    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > xStatusIndicator;

public:

    GIFWriter(SvStream &rStream);
    ~GIFWriter() {}

    BOOL WriteGIF( const Graphic& rGraphic, FilterConfigItem* pConfigItem );
};

GIFWriter::GIFWriter(SvStream &rStream)
    : m_rGIF(rStream)
    , m_pAcc(NULL)
    , nActX(0)
    , nActY(0)
{
}

// ------------------------------------------------------------------------

BOOL GIFWriter::WriteGIF(const Graphic& rGraphic, FilterConfigItem* pFilterConfigItem)
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
    BOOL            bLogSize = ( aMap.GetMapUnit() != MAP_PIXEL );

    if( bLogSize )
        aSize100 = Application::GetDefaultDevice()->LogicToLogic( rGraphic.GetPrefSize(), aMap, MAP_100TH_MM );

    bStatus = TRUE;
    nLastPercent = 0;
    nInterlaced = 0;
    m_pAcc = NULL;

    if ( pFilterConfigItem )
        nInterlaced = pFilterConfigItem->ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Interlaced" ) ), 0 );

    m_rGIF.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    if( rGraphic.IsAnimated() )
    {
        const Animation& rAnimation = rGraphic.GetAnimation();

        WriteSignature( TRUE );

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
        const BOOL bGrafTrans = rGraphic.IsTransparent();

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
                               BOOL bExtended, long nTimer, Disposal eDisposal )
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
    const USHORT    nCount = rAnimation.Count();

    if( nCount )
    {
        const double fStep = 100. / nCount;

        nMinPercent = 0L;
        nMaxPercent = (ULONG) fStep;

        for( USHORT i = 0; i < nCount; i++ )
        {
            const AnimationBitmap& rAnimBmp = rAnimation.Get( i );

            WriteBitmapEx( rAnimBmp.aBmpEx, rAnimBmp.aPosPix, TRUE,
                           rAnimBmp.nWait, rAnimBmp.eDisposal );
            nMinPercent = nMaxPercent;
            nMaxPercent = (ULONG) ( nMaxPercent + fStep );
        }
    }
}

// ------------------------------------------------------------------------

void GIFWriter::MayCallback( ULONG nPercent )
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

BOOL GIFWriter::CreateAccess( const BitmapEx& rBmpEx )
{
    if( bStatus )
    {
        Bitmap aMask( rBmpEx.GetMask() );

        aAccBmp = rBmpEx.GetBitmap();
        bTransparent = FALSE;

        if( !!aMask )
        {
            if( aAccBmp.Convert( BMP_CONVERSION_8BIT_TRANS ) )
            {
                aMask.Convert( BMP_CONVERSION_1BIT_THRESHOLD );
                aAccBmp.Replace( aMask, BMP_COL_TRANS );
                bTransparent = TRUE;
            }
            else
                aAccBmp.Convert( BMP_CONVERSION_8BIT_COLORS );
        }
        else
            aAccBmp.Convert( BMP_CONVERSION_8BIT_COLORS );

        m_pAcc = aAccBmp.AcquireReadAccess();

        if( !m_pAcc )
            bStatus = FALSE;
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

void GIFWriter::WriteSignature( BOOL bGIF89a )
{
    if( bStatus )
    {
        m_rGIF.Write( bGIF89a ? "GIF89a" : "GIF87a" , 6 );

        if( m_rGIF.GetError() )
            bStatus = FALSE;
    }
}

// ------------------------------------------------------------------------

void GIFWriter::WriteGlobalHeader( const Size& rSize )
{
    if( bStatus )
    {
        // 256 Farben
        const UINT16    nWidth = (UINT16) rSize.Width();
        const UINT16    nHeight = (UINT16) rSize.Height();
        const BYTE      cFlags = 128 | ( 7 << 4 );

        // Werte rausschreiben
        m_rGIF << nWidth;
        m_rGIF << nHeight;
        m_rGIF << cFlags;
        m_rGIF << (BYTE) 0x00;
        m_rGIF << (BYTE) 0x00;

        // Dummy-Palette mit zwei Eintraegen (Schwarz/Weiss) schreiben;
        // dieses nur wegen Photoshop-Bug, da die keine Bilder ohne
        // globale Farbpalette lesen koennen
        m_rGIF << (UINT16) 0;
        m_rGIF << (UINT16) 255;
        m_rGIF << (UINT16) 65535;

        if( m_rGIF.GetError() )
            bStatus = FALSE;
    }
}

// ------------------------------------------------------------------------

void GIFWriter::WriteLoopExtension( const Animation& rAnimation )
{
    DBG_ASSERT( rAnimation.Count() > 0, "Animation has no bitmaps!" );

    USHORT nLoopCount = (USHORT) rAnimation.GetLoopCount();

    // falls nur ein Durchlauf stattfinden soll,
    // wird keine LoopExtension geschrieben;
    // Default ist dann immer ein Durchlauf
    if( nLoopCount != 1 )
    {
        // Netscape interpretiert den LoopCount
        // als reine Anzahl der _Wiederholungen_
        if( nLoopCount )
            nLoopCount--;

        const BYTE cLoByte = (const BYTE) nLoopCount;
        const BYTE cHiByte = (const BYTE) ( nLoopCount >> 8 );

        m_rGIF << (BYTE) 0x21;
        m_rGIF << (BYTE) 0xff;
        m_rGIF << (BYTE) 0x0b;
        m_rGIF.Write( "NETSCAPE2.0", 11 );
        m_rGIF << (BYTE) 0x03;
        m_rGIF << (BYTE) 0x01;
        m_rGIF << cLoByte;
        m_rGIF << cHiByte;
        m_rGIF << (BYTE) 0x00;
    }
}

// ------------------------------------------------------------------------

void GIFWriter::WriteLogSizeExtension( const Size& rSize100 )
{
    // PrefSize in 100th-mm als ApplicationExtension schreiben
    if( rSize100.Width() && rSize100.Height() )
    {
        m_rGIF << (BYTE) 0x21;
        m_rGIF << (BYTE) 0xff;
        m_rGIF << (BYTE) 0x0b;
        m_rGIF.Write( "STARDIV 5.0", 11 );
        m_rGIF << (BYTE) 0x09;
        m_rGIF << (BYTE) 0x01;
        m_rGIF << (UINT32) rSize100.Width();
        m_rGIF << (UINT32) rSize100.Height();
        m_rGIF << (BYTE) 0x00;
    }
}

// ------------------------------------------------------------------------

void GIFWriter::WriteImageExtension( long nTimer, Disposal eDisposal )
{
    if( bStatus )
    {
        const UINT16    nDelay = (UINT16) nTimer;
        BYTE            cFlags = 0;

        // Transparent-Flag setzen
        if( bTransparent )
            cFlags |= 1;

        // Disposal-Wert setzen
        if( eDisposal == DISPOSE_BACK )
            cFlags |= ( 2 << 2 );
        else if( eDisposal == DISPOSE_PREVIOUS )
            cFlags |= ( 3 << 2 );

        m_rGIF << (BYTE) 0x21;
        m_rGIF << (BYTE) 0xf9;
        m_rGIF << (BYTE) 0x04;
        m_rGIF << cFlags;
        m_rGIF << nDelay;
        m_rGIF << (BYTE) m_pAcc->GetBestPaletteIndex( BMP_COL_TRANS );
        m_rGIF << (BYTE) 0x00;

        if( m_rGIF.GetError() )
            bStatus = FALSE;
    }
}

// ------------------------------------------------------------------------

void GIFWriter::WriteLocalHeader()
{
    if( bStatus )
    {
        const UINT16    nPosX = (UINT16) nActX;
        const UINT16    nPosY = (UINT16) nActY;
        const UINT16    nWidth = (UINT16) m_pAcc->Width();
        const UINT16    nHeight = (UINT16) m_pAcc->Height();
        BYTE            cFlags = (BYTE) ( m_pAcc->GetBitCount() - 1 );

        // Interlaced-Flag setzen
        if( nInterlaced )
            cFlags |= 0x40;

        // Flag fuer lokale Farbpalette setzen
        cFlags |= 0x80;

        // alles rausschreiben
        m_rGIF << (BYTE) 0x2c;
        m_rGIF << nPosX;
        m_rGIF << nPosY;
        m_rGIF << nWidth;
        m_rGIF << nHeight;
        m_rGIF << cFlags;

        if( m_rGIF.GetError() )
            bStatus = FALSE;
    }
}

// ------------------------------------------------------------------------

void GIFWriter::WritePalette()
{
    if( bStatus && m_pAcc->HasPalette() )
    {
        const USHORT nCount = m_pAcc->GetPaletteEntryCount();
        const USHORT nMaxCount = ( 1 << m_pAcc->GetBitCount() );

        for ( USHORT i = 0; i < nCount; i++ )
        {
            const BitmapColor& rColor = m_pAcc->GetPaletteColor( i );

            m_rGIF << rColor.GetRed();
            m_rGIF << rColor.GetGreen();
            m_rGIF << rColor.GetBlue();
        }

        // Rest mit 0 auffuellen
        if( nCount < nMaxCount )
            m_rGIF.SeekRel( ( nMaxCount - nCount ) * 3 );

        if( m_rGIF.GetError() )
            bStatus = FALSE;
    }
}

// ------------------------------------------------------------------------

void GIFWriter::WriteAccess()
{
    GIFLZWCompressor    aCompressor;
    const long          nWidth = m_pAcc->Width();
    const long          nHeight = m_pAcc->Height();
    BYTE*               pBuffer = NULL;
    const ULONG         nFormat = m_pAcc->GetScanlineFormat();
    BOOL                bNative = ( BMP_FORMAT_8BIT_PAL == nFormat );

    if( !bNative )
        pBuffer = new BYTE[ nWidth ];

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
                    pBuffer[ nX ] = (BYTE) m_pAcc->GetPixel( nY, nX );

                aCompressor.Compress( pBuffer, nWidth );
            }

            if ( m_rGIF.GetError() )
                bStatus = FALSE;

            MayCallback( nMinPercent + ( nMaxPercent - nMinPercent ) * i / nHeight );

            if( !bStatus )
                break;
        }

        aCompressor.EndCompression();

        if ( m_rGIF.GetError() )
            bStatus = FALSE;
    }

    delete[] pBuffer;
}

// ------------------------------------------------------------------------

void GIFWriter::WriteTerminator()
{
    if( bStatus )
    {
        m_rGIF << (BYTE) 0x3b;

        if( m_rGIF.GetError() )
            bStatus = FALSE;
    }
}

// ------------------------------------------------------------------------

extern "C" BOOL __LOADONCALLAPI GraphicExport( SvStream& rStream, Graphic& rGraphic,
                                               FilterConfigItem* pConfigItem, BOOL )
{
    GIFWriter aWriter(rStream);
    return aWriter.WriteGIF(rGraphic, pConfigItem);
}

// ------------------------------------------------------------------------

extern "C" BOOL __LOADONCALLAPI DoExportDialog( FltCallDialogParameter& rPara )
{
    BOOL bRet = FALSE;

    if ( rPara.pWindow )
    {
        ByteString  aResMgrName( "egi" );
        ResMgr*     pResMgr;

        pResMgr = ResMgr::CreateResMgr( aResMgrName.GetBuffer(), Application::GetSettings().GetUILocale() );

        if( pResMgr )
        {
            rPara.pResMgr = pResMgr;
            bRet = ( DlgExportEGIF( rPara ).Execute() == RET_OK );
            delete pResMgr;
        }
        else
            bRet = TRUE;
    }

    return bRet;
}

// ------------------------------------------------------------------------

#ifdef WIN

static HINSTANCE hDLLInst = 0;

extern "C" int CALLBACK LibMain( HINSTANCE hDLL, WORD, WORD nHeap, LPSTR )
{
#ifndef WNT
    if ( nHeap )
        UnlockData( 0 );
#endif

    hDLLInst = hDLL;

    return TRUE;
}

// ------------------------------------------------------------------------

extern "C" int CALLBACK WEP( int )
{
    return 1;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
