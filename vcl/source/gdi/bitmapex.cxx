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
#include "precompiled_vcl.hxx"
#include <ctype.h>
#include <rtl/crc.h>
#include <vcl/salbtype.hxx>
#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <vcl/outdev.hxx>
#include <vcl/alpha.hxx>
#include <vcl/image.h>
#include <vcl/bitmapex.hxx>
#include <vcl/pngread.hxx>
#include <vcl/impimagetree.hxx>
#include <tools/rc.h>
#include <vcl/svapp.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/virdev.hxx>

// ------------
// - BitmapEx -
// ------------

BitmapEx::BitmapEx() :
        eTransparent( TRANSPARENT_NONE ),
        bAlpha      ( FALSE )
{
}

// ------------------------------------------------------------------

BitmapEx::BitmapEx( const BitmapEx& rBitmapEx ) :
        aBitmap             ( rBitmapEx.aBitmap ),
        aMask               ( rBitmapEx.aMask ),
        aBitmapSize         ( rBitmapEx.aBitmapSize ),
        aTransparentColor   ( rBitmapEx.aTransparentColor ),
        eTransparent        ( rBitmapEx.eTransparent ),
        bAlpha              ( rBitmapEx.bAlpha )
{
}

BitmapEx::BitmapEx( const BitmapEx& rBitmapEx, Point aSrc, Size aSize ) :
        eTransparent( TRANSPARENT_NONE ),
        bAlpha      ( FALSE )
{
    if( rBitmapEx.IsEmpty() )
        return;

    aBitmap = Bitmap( aSize, rBitmapEx.aBitmap.GetBitCount() );
    aBitmapSize = aSize;
    if( rBitmapEx.IsAlpha() )
    {
        bAlpha = TRUE;
        aMask = AlphaMask( aSize ).ImplGetBitmap();
    }
    else if( rBitmapEx.IsTransparent() )
        aMask = Bitmap( aSize, rBitmapEx.aMask.GetBitCount() );

    Rectangle aDestRect( Point( 0, 0 ), aSize );
    Rectangle aSrcRect( aSrc, aSize );
    CopyPixel( aDestRect, aSrcRect, &rBitmapEx );
}

// ------------------------------------------------------------------

BitmapEx::BitmapEx( const ResId& rResId ) :
        eTransparent( TRANSPARENT_NONE ),
        bAlpha      ( FALSE )
{
    static ImplImageTreeSingletonRef    aImageTree;
    ResMgr*                             pResMgr = NULL;

    ResMgr::GetResourceSkipHeader( rResId.SetRT( RSC_BITMAP ), &pResMgr );
    pResMgr->ReadLong();
    pResMgr->ReadLong();

    const String aFileName( pResMgr->ReadString() );
    ::rtl::OUString aCurrentSymbolsStyle = Application::GetSettings().GetStyleSettings().GetCurrentSymbolsStyleName();

    if( !aImageTree->loadImage( aFileName, aCurrentSymbolsStyle, *this, true ) )
    {
#ifdef DBG_UTIL
        ByteString aErrorStr( "BitmapEx::BitmapEx( const ResId& rResId ): could not load image <" );
        OSL_FAIL( ( ( aErrorStr += ByteString( aFileName, RTL_TEXTENCODING_ASCII_US ) ) += '>' ).GetBuffer() );
#endif
    }
}

// ------------------------------------------------------------------

BitmapEx::BitmapEx( const Bitmap& rBmp ) :
        aBitmap     ( rBmp ),
        aBitmapSize ( aBitmap.GetSizePixel() ),
        eTransparent( TRANSPARENT_NONE ),
        bAlpha      ( FALSE )
{
}

// ------------------------------------------------------------------

BitmapEx::BitmapEx( const Bitmap& rBmp, const Bitmap& rMask ) :
        aBitmap         ( rBmp ),
        aMask           ( rMask ),
        aBitmapSize     ( aBitmap.GetSizePixel() ),
        eTransparent    ( !rMask ? TRANSPARENT_NONE : TRANSPARENT_BITMAP ),
        bAlpha          ( FALSE )
{
    DBG_ASSERT( !rMask || rBmp.GetSizePixel() == rMask.GetSizePixel(),
                "BitmapEx::BitmapEx(): size mismatch for bitmap and mask." );

    // #105489# Ensure a mask is exactly one bit deep
    if( !!aMask && aMask.GetBitCount() != 1 )
    {
        OSL_TRACE("BitmapEx: forced mask to monochrome");
        aMask.ImplMakeMono( 255 );
    }
}

// ------------------------------------------------------------------

BitmapEx::BitmapEx( const Bitmap& rBmp, const AlphaMask& rAlphaMask ) :
        aBitmap         ( rBmp ),
        aMask           ( rAlphaMask.ImplGetBitmap() ),
        aBitmapSize     ( aBitmap.GetSizePixel() ),
        eTransparent    ( !rAlphaMask ? TRANSPARENT_NONE : TRANSPARENT_BITMAP ),
        bAlpha          ( !rAlphaMask ? FALSE : TRUE )
{
    DBG_ASSERT( !rAlphaMask || rBmp.GetSizePixel() == rAlphaMask.GetSizePixel(),
                "BitmapEx::BitmapEx(): size mismatch for bitmap and alpha mask." );

    // #i75531# the workaround below can go when
    // X11SalGraphics::drawAlphaBitmap()'s render acceleration
    // can handle the bitmap depth mismatch directly
    if( aBitmap.GetBitCount() < aMask.GetBitCount() )
        aBitmap.Convert( BMP_CONVERSION_24BIT );
}

// ------------------------------------------------------------------

BitmapEx::BitmapEx( const Bitmap& rBmp, const Color& rTransparentColor ) :
        aBitmap             ( rBmp ),
        aBitmapSize         ( aBitmap.GetSizePixel() ),
        aTransparentColor   ( rTransparentColor ),
        eTransparent        ( TRANSPARENT_BITMAP ),
        bAlpha              ( FALSE )
{
    aMask = aBitmap.CreateMask( aTransparentColor );

    DBG_ASSERT( rBmp.GetSizePixel() == aMask.GetSizePixel(),
                "BitmapEx::BitmapEx(): size mismatch for bitmap and alpha mask." );
}

// ------------------------------------------------------------------

BitmapEx::~BitmapEx()
{
}

// ------------------------------------------------------------------

// ------------------------------------------------------------------

BitmapEx& BitmapEx::operator=( const BitmapEx& rBitmapEx )
{
    if( &rBitmapEx != this )
    {
        aBitmap = rBitmapEx.aBitmap;
        aMask = rBitmapEx.aMask;
        aBitmapSize = rBitmapEx.aBitmapSize;
        aTransparentColor = rBitmapEx.aTransparentColor;
        eTransparent = rBitmapEx.eTransparent;
        bAlpha = rBitmapEx.bAlpha;
    }

    return *this;
}

// ------------------------------------------------------------------

BOOL BitmapEx::operator==( const BitmapEx& rBitmapEx ) const
{
    if( eTransparent != rBitmapEx.eTransparent )
        return FALSE;

    if( aBitmap != rBitmapEx.aBitmap )
        return FALSE;

    if( aBitmapSize != rBitmapEx.aBitmapSize )
        return FALSE;

    if( eTransparent == TRANSPARENT_NONE )
        return TRUE;

    if( eTransparent == TRANSPARENT_COLOR )
        return aTransparentColor == rBitmapEx.aTransparentColor;

    return( ( aMask == rBitmapEx.aMask ) && ( bAlpha == rBitmapEx.bAlpha ) );
}

// ------------------------------------------------------------------

BOOL BitmapEx::IsEqual( const BitmapEx& rBmpEx ) const
{
    return( rBmpEx.eTransparent == eTransparent &&
            rBmpEx.bAlpha == bAlpha &&
            rBmpEx.aBitmap.IsEqual( aBitmap ) &&
            rBmpEx.aMask.IsEqual( aMask ) );
}

// ------------------------------------------------------------------

BOOL BitmapEx::IsEmpty() const
{
    return( aBitmap.IsEmpty() && aMask.IsEmpty() );
}

// ------------------------------------------------------------------

void BitmapEx::SetEmpty()
{
    aBitmap.SetEmpty();
    aMask.SetEmpty();
    eTransparent = TRANSPARENT_NONE;
    bAlpha = FALSE;
}

// ------------------------------------------------------------------

void BitmapEx::Clear()
{
    SetEmpty();
}

// ------------------------------------------------------------------

BOOL BitmapEx::IsTransparent() const
{
    return( eTransparent != TRANSPARENT_NONE );
}

// ------------------------------------------------------------------

BOOL BitmapEx::IsAlpha() const
{
    return( IsTransparent() && bAlpha );
}

// ------------------------------------------------------------------

Bitmap BitmapEx::GetBitmap( const Color* pTransReplaceColor ) const
{
    Bitmap aRetBmp( aBitmap );

    if( pTransReplaceColor && ( eTransparent != TRANSPARENT_NONE ) )
    {
        Bitmap aTempMask;

        if( eTransparent == TRANSPARENT_COLOR )
            aTempMask = aBitmap.CreateMask( aTransparentColor );
        else
            aTempMask = aMask;

        if( !IsAlpha() )
            aRetBmp.Replace( aTempMask, *pTransReplaceColor );
        else
            aRetBmp.Replace( GetAlpha(), *pTransReplaceColor );
    }

    return aRetBmp;
}

// ------------------------------------------------------------------

Bitmap BitmapEx::GetMask() const
{
    Bitmap aRet( aMask );

    if( IsAlpha() )
        aRet.ImplMakeMono( 255 );

    return aRet;
}

// ------------------------------------------------------------------

AlphaMask BitmapEx::GetAlpha() const
{
    AlphaMask aAlpha;

    if( IsAlpha() )
        aAlpha.ImplSetBitmap( aMask );
    else
        aAlpha = aMask;

    return aAlpha;
}

// ------------------------------------------------------------------

ULONG BitmapEx::GetSizeBytes() const
{
    ULONG nSizeBytes = aBitmap.GetSizeBytes();

    if( eTransparent == TRANSPARENT_BITMAP )
        nSizeBytes += aMask.GetSizeBytes();

    return nSizeBytes;
}

// ------------------------------------------------------------------

ULONG BitmapEx::GetChecksum() const
{
    sal_uInt32  nCrc = aBitmap.GetChecksum();
    SVBT32      aBT32;

    UInt32ToSVBT32( (long) eTransparent, aBT32 );
    nCrc = rtl_crc32( nCrc, aBT32, 4 );

    UInt32ToSVBT32( (long) bAlpha, aBT32 );
    nCrc = rtl_crc32( nCrc, aBT32, 4 );

    if( ( TRANSPARENT_BITMAP == eTransparent ) && !aMask.IsEmpty() )
    {
        UInt32ToSVBT32( aMask.GetChecksum(), aBT32 );
        nCrc = rtl_crc32( nCrc, aBT32, 4 );
    }

    return nCrc;
}

// ------------------------------------------------------------------

void BitmapEx::SetSizePixel( const Size& rNewSize )
{
    Scale( rNewSize );
}

// ------------------------------------------------------------------

BOOL BitmapEx::Invert()
{
    BOOL bRet = FALSE;

    if( !!aBitmap )
    {
        bRet = aBitmap.Invert();

        if( bRet && ( eTransparent == TRANSPARENT_COLOR ) )
            aTransparentColor = BitmapColor( aTransparentColor ).Invert();
    }

    return bRet;
}

// ------------------------------------------------------------------

BOOL BitmapEx::Mirror( ULONG nMirrorFlags )
{
    BOOL bRet = FALSE;

    if( !!aBitmap )
    {
        bRet = aBitmap.Mirror( nMirrorFlags );

        if( bRet && ( eTransparent == TRANSPARENT_BITMAP ) && !!aMask )
            aMask.Mirror( nMirrorFlags );
    }

    return bRet;
}

// ------------------------------------------------------------------

BOOL BitmapEx::Scale( const double& rScaleX, const double& rScaleY, ULONG nScaleFlag )
{
    BOOL bRet = FALSE;

    if( !!aBitmap )
    {
        bRet = aBitmap.Scale( rScaleX, rScaleY, nScaleFlag );

        if( bRet && ( eTransparent == TRANSPARENT_BITMAP ) && !!aMask )
            aMask.Scale( rScaleX, rScaleY, BMP_SCALE_FAST );

        aBitmapSize = aBitmap.GetSizePixel();

        DBG_ASSERT( !aMask || aBitmap.GetSizePixel() == aMask.GetSizePixel(),
                    "BitmapEx::Scale(): size mismatch for bitmap and alpha mask." );
    }

    return bRet;
}

// ------------------------------------------------------------------------

BOOL BitmapEx::Scale( const Size& rNewSize, ULONG nScaleFlag )
{
    BOOL bRet;

    if( aBitmapSize.Width() && aBitmapSize.Height() )
    {
        bRet = Scale( (double) rNewSize.Width() / aBitmapSize.Width(),
                      (double) rNewSize.Height() / aBitmapSize.Height(),
                      nScaleFlag );
    }
    else
        bRet = TRUE;

    return bRet;
}

// ------------------------------------------------------------------

BOOL BitmapEx::Rotate( long nAngle10, const Color& rFillColor )
{
    BOOL bRet = FALSE;

    if( !!aBitmap )
    {
        const BOOL bTransRotate = ( Color( COL_TRANSPARENT ) == rFillColor );

        if( bTransRotate )
        {
            if( eTransparent == TRANSPARENT_COLOR )
                bRet = aBitmap.Rotate( nAngle10, aTransparentColor );
            else
            {
                bRet = aBitmap.Rotate( nAngle10, COL_BLACK );

                if( eTransparent == TRANSPARENT_NONE )
                {
                    aMask = Bitmap( aBitmapSize, 1 );
                    aMask.Erase( COL_BLACK );
                    eTransparent = TRANSPARENT_BITMAP;
                }

                if( bRet && !!aMask )
                    aMask.Rotate( nAngle10, COL_WHITE );
            }
        }
        else
        {
            bRet = aBitmap.Rotate( nAngle10, rFillColor );

            if( bRet && ( eTransparent == TRANSPARENT_BITMAP ) && !!aMask )
                aMask.Rotate( nAngle10, COL_WHITE );
        }

        aBitmapSize = aBitmap.GetSizePixel();

        DBG_ASSERT( !aMask || aBitmap.GetSizePixel() == aMask.GetSizePixel(),
                    "BitmapEx::Rotate(): size mismatch for bitmap and alpha mask." );
    }

    return bRet;
}

// ------------------------------------------------------------------

BOOL BitmapEx::Crop( const Rectangle& rRectPixel )
{
    BOOL bRet = FALSE;

    if( !!aBitmap )
    {
        bRet = aBitmap.Crop( rRectPixel );

        if( bRet && ( eTransparent == TRANSPARENT_BITMAP ) && !!aMask )
            aMask.Crop( rRectPixel );

        aBitmapSize = aBitmap.GetSizePixel();

        DBG_ASSERT( !aMask || aBitmap.GetSizePixel() == aMask.GetSizePixel(),
                    "BitmapEx::Crop(): size mismatch for bitmap and alpha mask." );
    }

    return bRet;
}

// ------------------------------------------------------------------

BOOL BitmapEx::Convert( BmpConversion eConversion )
{
    return( !!aBitmap ? aBitmap.Convert( eConversion ) : FALSE );
}

// ------------------------------------------------------------------

BOOL BitmapEx::ReduceColors( USHORT nNewColorCount, BmpReduce eReduce )
{
    return( !!aBitmap ? aBitmap.ReduceColors( nNewColorCount, eReduce ) : FALSE );
}

// ------------------------------------------------------------------

BOOL BitmapEx::Expand( ULONG nDX, ULONG nDY, const Color* pInitColor, BOOL bExpandTransparent )
{
    BOOL bRet = FALSE;

    if( !!aBitmap )
    {
        bRet = aBitmap.Expand( nDX, nDY, pInitColor );

        if( bRet && ( eTransparent == TRANSPARENT_BITMAP ) && !!aMask )
        {
            Color aColor( bExpandTransparent ? COL_WHITE : COL_BLACK );
            aMask.Expand( nDX, nDY, &aColor );
        }

        aBitmapSize = aBitmap.GetSizePixel();

        DBG_ASSERT( !aMask || aBitmap.GetSizePixel() == aMask.GetSizePixel(),
                    "BitmapEx::Expand(): size mismatch for bitmap and alpha mask." );
    }

    return bRet;
}

// ------------------------------------------------------------------

BOOL BitmapEx::CopyPixel( const Rectangle& rRectDst, const Rectangle& rRectSrc,
                          const BitmapEx* pBmpExSrc )
{
    BOOL bRet = FALSE;

    if( !pBmpExSrc || pBmpExSrc->IsEmpty() )
    {
        if( !aBitmap.IsEmpty() )
        {
            bRet = aBitmap.CopyPixel( rRectDst, rRectSrc );

            if( bRet && ( eTransparent == TRANSPARENT_BITMAP ) && !!aMask )
                aMask.CopyPixel( rRectDst, rRectSrc );
        }
    }
    else
    {
        if( !aBitmap.IsEmpty() )
        {
            bRet = aBitmap.CopyPixel( rRectDst, rRectSrc, &pBmpExSrc->aBitmap );

            if( bRet )
            {
                if( pBmpExSrc->IsAlpha() )
                {
                    if( IsAlpha() )
                        // cast to use the optimized AlphaMask::CopyPixel
                        ((AlphaMask*) &aMask)->CopyPixel( rRectDst, rRectSrc, (AlphaMask*)&pBmpExSrc->aMask );
                    else if( IsTransparent() )
                    {
                        AlphaMask* pAlpha = new AlphaMask( aMask );

                        aMask = pAlpha->ImplGetBitmap();
                        delete pAlpha;
                        bAlpha = TRUE;
                        aMask.CopyPixel( rRectDst, rRectSrc, &pBmpExSrc->aMask );
                    }
                    else
                    {
                        sal_uInt8   cBlack = 0;
                        AlphaMask*  pAlpha = new AlphaMask( GetSizePixel(), &cBlack );

                        aMask = pAlpha->ImplGetBitmap();
                        delete pAlpha;
                        eTransparent = TRANSPARENT_BITMAP;
                        bAlpha = TRUE;
                        aMask.CopyPixel( rRectDst, rRectSrc, &pBmpExSrc->aMask );
                    }
                }
                else if( pBmpExSrc->IsTransparent() )
                {
                    if( IsAlpha() )
                    {
                        AlphaMask aAlpha( pBmpExSrc->aMask );
                        aMask.CopyPixel( rRectDst, rRectSrc, &aAlpha.ImplGetBitmap() );
                    }
                    else if( IsTransparent() )
                        aMask.CopyPixel( rRectDst, rRectSrc, &pBmpExSrc->aMask );
                    else
                    {
                        aMask = Bitmap( GetSizePixel(), 1 );
                        aMask.Erase( Color( COL_BLACK ) );
                        eTransparent = TRANSPARENT_BITMAP;
                        aMask.CopyPixel( rRectDst, rRectSrc, &pBmpExSrc->aMask );
                    }
                }
                else if( IsAlpha() )
                {
                    sal_uInt8         cBlack = 0;
                    const AlphaMask   aAlphaSrc( pBmpExSrc->GetSizePixel(), &cBlack );

                    aMask.CopyPixel( rRectDst, rRectSrc, &aAlphaSrc.ImplGetBitmap() );
                }
                else if( IsTransparent() )
                {
                    Bitmap aMaskSrc( pBmpExSrc->GetSizePixel(), 1 );

                    aMaskSrc.Erase( Color( COL_BLACK ) );
                    aMask.CopyPixel( rRectDst, rRectSrc, &aMaskSrc );
                }
            }
        }
    }

    return bRet;
}

// ------------------------------------------------------------------

BOOL BitmapEx::Erase( const Color& rFillColor )
{
    BOOL bRet = FALSE;

    if( !!aBitmap )
    {
        bRet = aBitmap.Erase( rFillColor );

        if( bRet && ( eTransparent == TRANSPARENT_BITMAP ) && !!aMask )
        {
            // #104416# Respect transparency on fill color
            if( rFillColor.GetTransparency() )
            {
                const Color aFill( rFillColor.GetTransparency(), rFillColor.GetTransparency(), rFillColor.GetTransparency() );
                aMask.Erase( aFill );
            }
            else
            {
                const Color aBlack( COL_BLACK );
                aMask.Erase( aBlack );
            }
        }
    }

    return bRet;
}

// ------------------------------------------------------------------

BOOL BitmapEx::Dither( ULONG nDitherFlags )
{
    return( !!aBitmap ? aBitmap.Dither( nDitherFlags ) : FALSE );
}

// ------------------------------------------------------------------

BOOL BitmapEx::Replace( const Color& rSearchColor, const Color& rReplaceColor, ULONG nTol )
{
    return( !!aBitmap ? aBitmap.Replace( rSearchColor, rReplaceColor, nTol ) : FALSE );
}

// ------------------------------------------------------------------

BOOL BitmapEx::Replace( const Color* pSearchColors, const Color* pReplaceColors, ULONG nColorCount, const ULONG* pTols )
{
    return( !!aBitmap ? aBitmap.Replace( pSearchColors, pReplaceColors, nColorCount, (ULONG*) pTols ) : FALSE );
}

// ------------------------------------------------------------------

BOOL BitmapEx::Adjust( short nLuminancePercent, short nContrastPercent,
                       short nChannelRPercent, short nChannelGPercent, short nChannelBPercent,
                       double fGamma, BOOL bInvert )
{
    return( !!aBitmap ? aBitmap.Adjust( nLuminancePercent, nContrastPercent,
                                        nChannelRPercent, nChannelGPercent, nChannelBPercent,
                                        fGamma, bInvert ) : FALSE );
}

// ------------------------------------------------------------------

BOOL BitmapEx::Filter( BmpFilter eFilter, const BmpFilterParam* pFilterParam, const Link* pProgress )
{
    return( !!aBitmap ? aBitmap.Filter( eFilter, pFilterParam, pProgress ) : FALSE );
}

// ------------------------------------------------------------------

void BitmapEx::Draw( OutputDevice* pOutDev, const Point& rDestPt ) const
{
    pOutDev->DrawBitmapEx( rDestPt, *this );
}

// ------------------------------------------------------------------

void BitmapEx::Draw( OutputDevice* pOutDev,
                     const Point& rDestPt, const Size& rDestSize ) const
{
    pOutDev->DrawBitmapEx( rDestPt, rDestSize, *this );
}

// ------------------------------------------------------------------

void BitmapEx::Draw( OutputDevice* pOutDev,
                     const Point& rDestPt, const Size& rDestSize,
                     const Point& rSrcPtPixel, const Size& rSrcSizePixel ) const
{
    pOutDev->DrawBitmapEx( rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel, *this );
}

BitmapEx BitmapEx:: AutoScaleBitmap(BitmapEx & aBitmap, const long aStandardSize)
{
    Point aEmptyPoint(0,0);
    sal_Int32 imgNewWidth = 0;
    sal_Int32 imgNewHeight = 0;
    double imgposX = 0;
    double imgposY = 0;
    BitmapEx  aRet = aBitmap;
    double imgOldWidth = aRet.GetSizePixel().Width();
    double imgOldHeight =aRet.GetSizePixel().Height();

    Size aScaledSize;
    if (imgOldWidth >= aStandardSize || imgOldHeight >= aStandardSize)
    {
        if (imgOldWidth >= imgOldHeight)
        {
            imgNewWidth = aStandardSize;
            imgNewHeight = sal_Int32(imgOldHeight / (imgOldWidth / aStandardSize) + 0.5);
            imgposX = 0;
            imgposY = (aStandardSize - (imgOldHeight / (imgOldWidth / aStandardSize) + 0.5)) / 2 + 0.5;
        }
        else
        {
            imgNewHeight = aStandardSize;
            imgNewWidth = sal_Int32(imgOldWidth / (imgOldHeight / aStandardSize) + 0.5);
            imgposY = 0;
            imgposX = (aStandardSize - (imgOldWidth / (imgOldHeight / aStandardSize) + 0.5)) / 2 + 0.5;
        }

        aScaledSize = Size( imgNewWidth, imgNewHeight );
        aRet.Scale( aScaledSize, BMP_SCALE_INTERPOLATE );
    }
    else
    {
        imgposX = (aStandardSize - imgOldWidth) / 2 + 0.5;
        imgposY = (aStandardSize - imgOldHeight) / 2 + 0.5;
    }

    Size aStdSize( aStandardSize, aStandardSize );
    Rectangle aRect(aEmptyPoint, aStdSize );

    VirtualDevice aVirDevice( *Application::GetDefaultDevice(), 0, 1 );
    aVirDevice.SetOutputSizePixel( aStdSize );
    aVirDevice.SetFillColor( COL_TRANSPARENT );
    aVirDevice.SetLineColor( COL_TRANSPARENT );

    //draw a rect into virDevice
    aVirDevice.DrawRect( aRect );
    Point aPointPixel( (long)imgposX, (long)imgposY );
    aVirDevice.DrawBitmapEx( aPointPixel, aRet );
    aRet = aVirDevice.GetBitmapEx( aEmptyPoint, aStdSize );

    return aRet;
}
// ------------------------------------------------------------------

sal_uInt8 BitmapEx::GetTransparency(sal_Int32 nX, sal_Int32 nY) const
{
    sal_uInt8 nTransparency(0xff);

    if(!aBitmap.IsEmpty())
    {
        if(nX >= 0 && nX < aBitmapSize.Width() && nY >= 0 && nY < aBitmapSize.Height())
        {
            switch(eTransparent)
            {
                case TRANSPARENT_NONE:
                {
                    // not transparent, ergo all covered
                    nTransparency = 0x00;
                    break;
                }
                case TRANSPARENT_COLOR:
                {
                    Bitmap aTestBitmap(aBitmap);
                    BitmapReadAccess* pRead = aTestBitmap.AcquireReadAccess();

                    if(pRead)
                    {
                        const Color aColor = pRead->GetColor(nY, nX);

                        // if color is not equal to TransparentColor, we are not transparent
                        if(aColor != aTransparentColor)
                        {
                            nTransparency = 0x00;
                        }

                        aTestBitmap.ReleaseAccess(pRead);
                    }
                    break;
                }
                case TRANSPARENT_BITMAP:
                {
                    if(!aMask.IsEmpty())
                    {
                        Bitmap aTestBitmap(aMask);
                        BitmapReadAccess* pRead = aTestBitmap.AcquireReadAccess();

                        if(pRead)
                        {
                            const BitmapColor aBitmapColor(pRead->GetPixel(nY, nX));

                            if(bAlpha)
                            {
                                nTransparency = aBitmapColor.GetIndex();
                            }
                            else
                            {
                                if(0x00 == aBitmapColor.GetIndex())
                                {
                                    nTransparency = 0x00;
                                }
                            }

                            aTestBitmap.ReleaseAccess(pRead);
                        }
                    }
                    break;
                }
            }
        }
    }

    return nTransparency;
}

// ------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStm, const BitmapEx& rBitmapEx )
{
    rBitmapEx.aBitmap.Write( rOStm );

    rOStm << (UINT32) 0x25091962;
    rOStm << (UINT32) 0xACB20201;
    rOStm << (BYTE) rBitmapEx.eTransparent;

    if( rBitmapEx.eTransparent == TRANSPARENT_BITMAP )
        rBitmapEx.aMask.Write( rOStm );
    else if( rBitmapEx.eTransparent == TRANSPARENT_COLOR )
        rOStm << rBitmapEx.aTransparentColor;

    return rOStm;
}

// ------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStm, BitmapEx& rBitmapEx )
{
    Bitmap aBmp;

    rIStm >> aBmp;

    if( !rIStm.GetError() )
    {
        const ULONG nStmPos = rIStm.Tell();
        UINT32      nMagic1 = 0;
        UINT32      nMagic2 = 0;

        rIStm >> nMagic1 >> nMagic2;

        if( ( nMagic1 != 0x25091962 ) || ( nMagic2 != 0xACB20201 ) || rIStm.GetError() )
        {
            rIStm.ResetError();
            rIStm.Seek( nStmPos );
            rBitmapEx = aBmp;
        }
        else
        {
            BYTE bTransparent = false;

            rIStm >> bTransparent;

            if( bTransparent == (BYTE) TRANSPARENT_BITMAP )
            {
                Bitmap aMask;

                rIStm >> aMask;

                if( !!aMask)
                {
                    // do we have an alpha mask?
                    if( ( 8 == aMask.GetBitCount() ) && aMask.HasGreyPalette() )
                    {
                        AlphaMask aAlpha;

                        // create alpha mask quickly (without greyscale conversion)
                        aAlpha.ImplSetBitmap( aMask );
                        rBitmapEx = BitmapEx( aBmp, aAlpha );
                    }
                    else
                        rBitmapEx = BitmapEx( aBmp, aMask );
                }
                else
                    rBitmapEx = aBmp;
            }
            else if( bTransparent == (BYTE) TRANSPARENT_COLOR )
            {
                Color aTransparentColor;

                rIStm >> aTransparentColor;
                rBitmapEx = BitmapEx( aBmp, aTransparentColor );
            }
            else
                rBitmapEx = aBmp;
        }
    }

    return rIStm;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
