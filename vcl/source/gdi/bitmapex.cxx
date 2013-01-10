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

#include <ctype.h>
#include <rtl/crc.h>
#include <rtl/strbuf.hxx>

#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <tools/rc.h>
#include <vcl/salbtype.hxx>
#include <vcl/outdev.hxx>
#include <vcl/alpha.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/pngread.hxx>
#include <vcl/svapp.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/virdev.hxx>

#include <image.h>
#include <impimagetree.hxx>

// BitmapEx::Create
#include <salbmp.hxx>
#include <salinst.hxx>
#include <svdata.hxx>
#include <com/sun/star/beans/XFastPropertySet.hpp>
using namespace ::com::sun::star;

BitmapEx::BitmapEx() :
        eTransparent( TRANSPARENT_NONE ),
        bAlpha      ( sal_False )
{
}

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
        bAlpha      ( sal_False )
{
    if( rBitmapEx.IsEmpty() )
        return;

    aBitmap = Bitmap( aSize, rBitmapEx.aBitmap.GetBitCount() );
    aBitmapSize = aSize;
    if( rBitmapEx.IsAlpha() )
    {
        bAlpha = sal_True;
        aMask = AlphaMask( aSize ).ImplGetBitmap();
    }
    else if( rBitmapEx.IsTransparent() )
        aMask = Bitmap( aSize, rBitmapEx.aMask.GetBitCount() );

    Rectangle aDestRect( Point( 0, 0 ), aSize );
    Rectangle aSrcRect( aSrc, aSize );
    CopyPixel( aDestRect, aSrcRect, &rBitmapEx );
}

BitmapEx::BitmapEx( const ResId& rResId ) :
        eTransparent( TRANSPARENT_NONE ),
        bAlpha      ( sal_False )
{
    static ImplImageTreeSingletonRef    aImageTree;
    ResMgr*                             pResMgr = NULL;

    ResMgr::GetResourceSkipHeader( rResId.SetRT( RSC_BITMAP ), &pResMgr );
    pResMgr->ReadLong();
    pResMgr->ReadLong();

    const String aFileName( pResMgr->ReadString() );
    OUString aCurrentSymbolsStyle = Application::GetSettings().GetStyleSettings().GetCurrentSymbolsStyleName();

    if( !aImageTree->loadImage( aFileName, aCurrentSymbolsStyle, *this, true ) )
    {
#ifdef DBG_UTIL
        OStringBuffer aErrorStr(
            "BitmapEx::BitmapEx( const ResId& rResId ): could not load image <");
        aErrorStr.append(OUStringToOString(aFileName, RTL_TEXTENCODING_ASCII_US)).append('>');
        OSL_FAIL(aErrorStr.getStr());
#endif
    }
}

BitmapEx::BitmapEx( const Bitmap& rBmp ) :
        aBitmap     ( rBmp ),
        aBitmapSize ( aBitmap.GetSizePixel() ),
        eTransparent( TRANSPARENT_NONE ),
        bAlpha      ( sal_False )
{
}

BitmapEx::BitmapEx( const Bitmap& rBmp, const Bitmap& rMask ) :
        aBitmap         ( rBmp ),
        aMask           ( rMask ),
        aBitmapSize     ( aBitmap.GetSizePixel() ),
        eTransparent    ( !rMask ? TRANSPARENT_NONE : TRANSPARENT_BITMAP ),
        bAlpha          ( sal_False )
{
    if(!!aBitmap && !!aMask && aBitmap.GetSizePixel() != aMask.GetSizePixel())
    {
        OSL_ENSURE(false, "Mask size differs from Bitmap size, corrected Mask (!)");
        aMask.Scale(aBitmap.GetSizePixel());
    }

    // Ensure a mask is exactly one bit deep
    if( !!aMask && aMask.GetBitCount() != 1 )
    {
        OSL_TRACE("BitmapEx: forced mask to monochrome");
        aMask.ImplMakeMono( 255 );
    }
}

BitmapEx::BitmapEx( const Bitmap& rBmp, const AlphaMask& rAlphaMask ) :
        aBitmap         ( rBmp ),
        aMask           ( rAlphaMask.ImplGetBitmap() ),
        aBitmapSize     ( aBitmap.GetSizePixel() ),
        eTransparent    ( !rAlphaMask ? TRANSPARENT_NONE : TRANSPARENT_BITMAP ),
        bAlpha          ( !rAlphaMask ? sal_False : sal_True )
{
    if(!!aBitmap && !!aMask && aBitmap.GetSizePixel() != aMask.GetSizePixel())
    {
        OSL_ENSURE(false, "Alpha size differs from Bitmap size, corrected Mask (!)");
        aMask.Scale(rBmp.GetSizePixel());
    }

    // #i75531# the workaround below can go when
    // X11SalGraphics::drawAlphaBitmap()'s render acceleration
    // can handle the bitmap depth mismatch directly
    if( aBitmap.GetBitCount() < aMask.GetBitCount() )
        aBitmap.Convert( BMP_CONVERSION_24BIT );
}

BitmapEx::BitmapEx( const Bitmap& rBmp, const Color& rTransparentColor ) :
        aBitmap             ( rBmp ),
        aBitmapSize         ( aBitmap.GetSizePixel() ),
        aTransparentColor   ( rTransparentColor ),
        eTransparent        ( TRANSPARENT_BITMAP ),
        bAlpha              ( sal_False )
{
    aMask = aBitmap.CreateMask( aTransparentColor );

    DBG_ASSERT( rBmp.GetSizePixel() == aMask.GetSizePixel(),
                "BitmapEx::BitmapEx(): size mismatch for bitmap and alpha mask." );
}

BitmapEx::~BitmapEx()
{
}

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

sal_Bool BitmapEx::operator==( const BitmapEx& rBitmapEx ) const
{
    if( eTransparent != rBitmapEx.eTransparent )
        return sal_False;

    if( aBitmap != rBitmapEx.aBitmap )
        return sal_False;

    if( aBitmapSize != rBitmapEx.aBitmapSize )
        return sal_False;

    if( eTransparent == TRANSPARENT_NONE )
        return sal_True;

    if( eTransparent == TRANSPARENT_COLOR )
        return aTransparentColor == rBitmapEx.aTransparentColor;

    return( ( aMask == rBitmapEx.aMask ) && ( bAlpha == rBitmapEx.bAlpha ) );
}

sal_Bool BitmapEx::IsEqual( const BitmapEx& rBmpEx ) const
{
    return( rBmpEx.eTransparent == eTransparent &&
            rBmpEx.bAlpha == bAlpha &&
            rBmpEx.aBitmap.IsEqual( aBitmap ) &&
            rBmpEx.aMask.IsEqual( aMask ) );
}

sal_Bool BitmapEx::IsEmpty() const
{
    return( aBitmap.IsEmpty() && aMask.IsEmpty() );
}

void BitmapEx::SetEmpty()
{
    aBitmap.SetEmpty();
    aMask.SetEmpty();
    eTransparent = TRANSPARENT_NONE;
    bAlpha = sal_False;
}

void BitmapEx::Clear()
{
    SetEmpty();
}

sal_Bool BitmapEx::IsTransparent() const
{
    return( eTransparent != TRANSPARENT_NONE );
}

sal_Bool BitmapEx::IsAlpha() const
{
    return( IsTransparent() && bAlpha );
}

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

Bitmap BitmapEx::GetMask() const
{
    Bitmap aRet( aMask );

    if( IsAlpha() )
        aRet.ImplMakeMono( 255 );

    return aRet;
}

AlphaMask BitmapEx::GetAlpha() const
{
    if( IsAlpha() )
    {
        AlphaMask aAlpha;
        aAlpha.ImplSetBitmap( aMask );
        return aAlpha;
    }
    else
    {
        return aMask;
    }
}

sal_uLong BitmapEx::GetSizeBytes() const
{
    sal_uLong nSizeBytes = aBitmap.GetSizeBytes();

    if( eTransparent == TRANSPARENT_BITMAP )
        nSizeBytes += aMask.GetSizeBytes();

    return nSizeBytes;
}

sal_uLong BitmapEx::GetChecksum() const
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

void BitmapEx::SetSizePixel( const Size& rNewSize, sal_uInt32 nScaleFlag )
{
    if(GetSizePixel() != rNewSize)
    {
        Scale( rNewSize, nScaleFlag );
    }
}

sal_Bool BitmapEx::Invert()
{
    sal_Bool bRet = sal_False;

    if( !!aBitmap )
    {
        bRet = aBitmap.Invert();

        if( bRet && ( eTransparent == TRANSPARENT_COLOR ) )
            aTransparentColor = BitmapColor( aTransparentColor ).Invert();
    }

    return bRet;
}

sal_Bool BitmapEx::Mirror( sal_uLong nMirrorFlags )
{
    sal_Bool bRet = sal_False;

    if( !!aBitmap )
    {
        bRet = aBitmap.Mirror( nMirrorFlags );

        if( bRet && ( eTransparent == TRANSPARENT_BITMAP ) && !!aMask )
            aMask.Mirror( nMirrorFlags );
    }

    return bRet;
}

// ------------------------------------------------------------------

sal_Bool BitmapEx::Scale( const double& rScaleX, const double& rScaleY, sal_uInt32 nScaleFlag )
{
    sal_Bool bRet = sal_False;

    if( !!aBitmap )
    {
        bRet = aBitmap.Scale( rScaleX, rScaleY, nScaleFlag );

        if( bRet && ( eTransparent == TRANSPARENT_BITMAP ) && !!aMask )
        {
            aMask.Scale( rScaleX, rScaleY, nScaleFlag );
        }

        aBitmapSize = aBitmap.GetSizePixel();

        DBG_ASSERT( !aMask || aBitmap.GetSizePixel() == aMask.GetSizePixel(),
                    "BitmapEx::Scale(): size mismatch for bitmap and alpha mask." );
    }

    return bRet;
}

// ------------------------------------------------------------------------

sal_Bool BitmapEx::Scale( const Size& rNewSize, sal_uInt32 nScaleFlag )
{
    sal_Bool bRet;

    if( aBitmapSize.Width() && aBitmapSize.Height() &&
        ( rNewSize.Width()  != aBitmapSize.Width() ||
          rNewSize.Height() != aBitmapSize.Height() ) )
    {
        bRet = Scale( (double) rNewSize.Width() / aBitmapSize.Width(),
                      (double) rNewSize.Height() / aBitmapSize.Height(),
                      nScaleFlag );
    }
    else
        bRet = sal_True;

    return bRet;
}

sal_Bool BitmapEx::Rotate( long nAngle10, const Color& rFillColor )
{
    sal_Bool bRet = sal_False;

    if( !!aBitmap )
    {
        const bool bTransRotate = ( Color( COL_TRANSPARENT ) == rFillColor );

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

sal_Bool BitmapEx::Crop( const Rectangle& rRectPixel )
{
    sal_Bool bRet = sal_False;

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

sal_Bool BitmapEx::Convert( BmpConversion eConversion )
{
    return( !!aBitmap ? aBitmap.Convert( eConversion ) : sal_False );
}

sal_Bool BitmapEx::ReduceColors( sal_uInt16 nNewColorCount, BmpReduce eReduce )
{
    return( !!aBitmap ? aBitmap.ReduceColors( nNewColorCount, eReduce ) : sal_False );
}

sal_Bool BitmapEx::Expand( sal_uLong nDX, sal_uLong nDY, const Color* pInitColor, sal_Bool bExpandTransparent )
{
    sal_Bool bRet = sal_False;

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

sal_Bool BitmapEx::CopyPixel( const Rectangle& rRectDst, const Rectangle& rRectSrc,
                          const BitmapEx* pBmpExSrc )
{
    sal_Bool bRet = sal_False;

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
                        bAlpha = sal_True;
                        aMask.CopyPixel( rRectDst, rRectSrc, &pBmpExSrc->aMask );
                    }
                    else
                    {
                        sal_uInt8   cBlack = 0;
                        AlphaMask*  pAlpha = new AlphaMask( GetSizePixel(), &cBlack );

                        aMask = pAlpha->ImplGetBitmap();
                        delete pAlpha;
                        eTransparent = TRANSPARENT_BITMAP;
                        bAlpha = sal_True;
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

sal_Bool BitmapEx::Erase( const Color& rFillColor )
{
    sal_Bool bRet = sal_False;

    if( !!aBitmap )
    {
        bRet = aBitmap.Erase( rFillColor );

        if( bRet && ( eTransparent == TRANSPARENT_BITMAP ) && !!aMask )
        {
            // Respect transparency on fill color
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

sal_Bool BitmapEx::Dither( sal_uLong nDitherFlags )
{
    return( !!aBitmap ? aBitmap.Dither( nDitherFlags ) : sal_False );
}

sal_Bool BitmapEx::Replace( const Color& rSearchColor, const Color& rReplaceColor, sal_uLong nTol )
{
    return( !!aBitmap ? aBitmap.Replace( rSearchColor, rReplaceColor, nTol ) : sal_False );
}

sal_Bool BitmapEx::Replace( const Color* pSearchColors, const Color* pReplaceColors, sal_uLong nColorCount, const sal_uLong* pTols )
{
    return( !!aBitmap ? aBitmap.Replace( pSearchColors, pReplaceColors, nColorCount, (sal_uLong*) pTols ) : sal_False );
}

sal_Bool BitmapEx::Adjust( short nLuminancePercent, short nContrastPercent,
                       short nChannelRPercent, short nChannelGPercent, short nChannelBPercent,
                       double fGamma, sal_Bool bInvert )
{
    return( !!aBitmap ? aBitmap.Adjust( nLuminancePercent, nContrastPercent,
                                        nChannelRPercent, nChannelGPercent, nChannelBPercent,
                                        fGamma, bInvert ) : sal_False );
}

sal_Bool BitmapEx::Filter( BmpFilter eFilter, const BmpFilterParam* pFilterParam, const Link* pProgress )
{
    return( !!aBitmap ? aBitmap.Filter( eFilter, pFilterParam, pProgress ) : sal_False );
}

void BitmapEx::Draw( OutputDevice* pOutDev, const Point& rDestPt ) const
{
    pOutDev->DrawBitmapEx( rDestPt, *this );
}

void BitmapEx::Draw( OutputDevice* pOutDev,
                     const Point& rDestPt, const Size& rDestSize ) const
{
    pOutDev->DrawBitmapEx( rDestPt, rDestSize, *this );
}

BitmapEx BitmapEx:: AutoScaleBitmap(BitmapEx & aBitmap, const long aStandardSize)
{
    Point aEmptyPoint(0,0);
    double imgposX = 0;
    double imgposY = 0;
    BitmapEx  aRet = aBitmap;
    double imgOldWidth = aRet.GetSizePixel().Width();
    double imgOldHeight =aRet.GetSizePixel().Height();

    Size aScaledSize;
    if (imgOldWidth >= aStandardSize || imgOldHeight >= aStandardSize)
    {
        sal_Int32 imgNewWidth = 0;
        sal_Int32 imgNewHeight = 0;
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
        aRet.Scale( aScaledSize, BMP_SCALE_BESTQUALITY );
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

    // Draw a rect into virDevice
    aVirDevice.DrawRect( aRect );
    Point aPointPixel( (long)imgposX, (long)imgposY );
    aVirDevice.DrawBitmapEx( aPointPixel, aRet );
    aRet = aVirDevice.GetBitmapEx( aEmptyPoint, aStdSize );

    return aRet;
}

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
                    // Not transparent, ergo all covered
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

                        // If color is not equal to TransparentColor, we are not transparent
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

// Shift alpha transparent pixels between cppcanvas/ implementations
// and vcl in a generally grotesque and under-performing fashion
bool BitmapEx::Create( const ::com::sun::star::uno::Reference<
                       ::com::sun::star::rendering::XBitmapCanvas > &xBitmapCanvas,
                       const Size &rSize )
{
    SetEmpty();
    Size aSize( rSize );

    uno::Reference< beans::XFastPropertySet > xFastPropertySet( xBitmapCanvas, uno::UNO_QUERY );
    if( xFastPropertySet.get() )
    {
        // 0 means get BitmapEx
        uno::Any aAny = xFastPropertySet->getFastPropertyValue( 0 );
        BitmapEx* pBitmapEx = (BitmapEx*) *reinterpret_cast<const sal_Int64*>(aAny.getValue());
        if( pBitmapEx )
        {
            *this = *pBitmapEx;
            delete pBitmapEx;
            return true;
        }
    }

    SalBitmap* pSalBmp, *pSalMask;

    pSalBmp = ImplGetSVData()->mpDefInst->CreateSalBitmap();
    pSalMask = ImplGetSVData()->mpDefInst->CreateSalBitmap();

    if( pSalBmp->Create( xBitmapCanvas, aSize ) )
    {
#ifdef CLAMP_BITDEPTH_PARANOIA
        // did we get alpha mixed up in the bitmap itself
        // eg. Cairo Canvas ... yes performance of this is awful.
        if( pSalBmp->GetBitCount() > 24 )
        {
            // Format convert the pixels with generic code
            Bitmap aSrcPixels( pSalBmp );
            aBitmap = Bitmap( rSize, 24 );
            BitmapReadAccess aSrcRead( aSrcPixels );
            BitmapWriteAccess aDestWrite( aBitmap );
            aDestWrite.CopyBuffer( aSrcRead );
        }
        else
#endif
            aBitmap = Bitmap( pSalBmp );

        aBitmapSize = rSize;
        if ( pSalMask->Create( xBitmapCanvas, aSize, true ) )
        {
            aMask = Bitmap( pSalMask );
            bAlpha = sal_True;
            aBitmapSize = rSize;
            eTransparent = !aMask ? TRANSPARENT_NONE : TRANSPARENT_BITMAP;

            return true;
        }
        else
        {
            bAlpha = sal_False;
            eTransparent = TRANSPARENT_NONE;
            return true;
        }
    }

    delete pSalBmp;
    delete pSalMask;

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
