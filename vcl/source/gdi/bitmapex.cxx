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
#include <basegfx/matrix/b2dhommatrixtools.hxx>

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

    const OUString aFileName( pResMgr->ReadString() );
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

    Size aLocalSize(rSize);
    if( pSalBmp->Create( xBitmapCanvas, aLocalSize ) )
    {
        if ( pSalMask->Create( xBitmapCanvas, aLocalSize, true ) )
        {
            *this = BitmapEx(Bitmap(pSalBmp), Bitmap(pSalMask) );
            return true;
        }
        else
        {
            *this = BitmapEx(Bitmap(pSalBmp));
            return true;
        }
    }

    delete pSalBmp;
    delete pSalMask;

    return false;
}

// ------------------------------------------------------------------

namespace
{
    Bitmap impTransformBitmap(
        const Bitmap& rSource,
        const Size aDestinationSize,
        const basegfx::B2DHomMatrix& rTransform,
        bool bSmooth)
    {
        Bitmap aDestination(aDestinationSize, 24);
        BitmapWriteAccess* pWrite = aDestination.AcquireWriteAccess();

        if(pWrite)
        {
            //const Size aContentSizePixel(rSource.GetSizePixel());
            BitmapReadAccess* pRead = (const_cast< Bitmap& >(rSource)).AcquireReadAccess();

            if(pRead)
            {
                const Size aDestinationSizePixel(aDestination.GetSizePixel());
                const BitmapColor aOutside(BitmapColor(0xff, 0xff, 0xff));

                for(sal_Int32 y(0L); y < aDestinationSizePixel.getHeight(); y++)
                {
                    for(sal_Int32 x(0L); x < aDestinationSizePixel.getWidth(); x++)
                    {
                        const basegfx::B2DPoint aSourceCoor(rTransform * basegfx::B2DPoint(x, y));

                        if(bSmooth)
                        {
                            pWrite->SetPixel(
                                y,
                                x,
                                pRead->GetInterpolatedColorWithFallback(
                                    aSourceCoor.getY(),
                                    aSourceCoor.getX(),
                                    aOutside));
                        }
                        else
                        {
                            // this version does the correct <= 0.0 checks, so no need
                            // to do the static_cast< sal_Int32 > self and make an error
                            pWrite->SetPixel(
                                y,
                                x,
                                pRead->GetColorWithFallback(
                                    aSourceCoor.getY(),
                                    aSourceCoor.getX(),
                                    aOutside));
                        }
                    }
                }

                delete pRead;
            }

            delete pWrite;
        }

        rSource.AdaptBitCount(aDestination);

        return aDestination;
    }
} // end of anonymous namespace

BitmapEx BitmapEx::TransformBitmapEx(
    double fWidth,
    double fHeight,
    const basegfx::B2DHomMatrix& rTransformation,
    bool bSmooth) const
{
    if(fWidth <= 1 || fHeight <= 1)
        return BitmapEx();

    // force destination to 24 bit, we want to smooth output
    const Size aDestinationSize(basegfx::fround(fWidth), basegfx::fround(fHeight));
    const Bitmap aDestination(impTransformBitmap(GetBitmap(), aDestinationSize, rTransformation, bSmooth));

    // create mask
    if(IsTransparent())
    {
        if(IsAlpha())
        {
            const Bitmap aAlpha(impTransformBitmap(GetAlpha().GetBitmap(), aDestinationSize, rTransformation, bSmooth));
            return BitmapEx(aDestination, AlphaMask(aAlpha));
        }
        else
        {
            const Bitmap aLclMask(impTransformBitmap(GetMask(), aDestinationSize, rTransformation, false));
            return BitmapEx(aDestination, aLclMask);
        }
    }

    return BitmapEx(aDestination);
}

// ------------------------------------------------------------------

BitmapEx BitmapEx::getTransformed(
    const basegfx::B2DHomMatrix& rTransformation,
    const basegfx::B2DRange& rVisibleRange,
    double fMaximumArea,
    bool bSmooth) const
{
    BitmapEx aRetval;

    if(IsEmpty())
        return aRetval;

    const sal_uInt32 nSourceWidth(GetSizePixel().Width());
    const sal_uInt32 nSourceHeight(GetSizePixel().Height());

    if(!nSourceWidth || !nSourceHeight)
        return aRetval;

    // Get aOutlineRange
    basegfx::B2DRange aOutlineRange(0.0, 0.0, 1.0, 1.0);

    aOutlineRange.transform(rTransformation);

    // create visible range from it by moving from relative to absolute
    basegfx::B2DRange aVisibleRange(rVisibleRange);

    aVisibleRange.transform(
        basegfx::tools::createScaleTranslateB2DHomMatrix(
            aOutlineRange.getRange(),
            aOutlineRange.getMinimum()));

    // get target size (which is visible range's size)
    double fWidth(aVisibleRange.getWidth());
    double fHeight(aVisibleRange.getHeight());

    if(fWidth < 1.0 || fHeight < 1.0)
    {
        return aRetval;
    }

    // test if discrete size (pixel) maybe too big and limit it
    const double fArea(fWidth * fHeight);
    const bool bNeedToReduce(basegfx::fTools::more(fArea, fMaximumArea));
    double fReduceFactor(1.0);

    if(bNeedToReduce)
    {
        fReduceFactor = sqrt(fMaximumArea / fArea);
        fWidth *= fReduceFactor;
        fHeight *= fReduceFactor;
    }

    // Build complete transform from source pixels to target pixels.
    // Start by scaling from source pixel size to unit coordinates
    basegfx::B2DHomMatrix aTransform(
        basegfx::tools::createScaleB2DHomMatrix(
            1.0 / nSourceWidth,
            1.0 / nSourceHeight));

    // multiply with given transform which leads from unit coordinates inside
    // aOutlineRange
    aTransform = rTransformation * aTransform;

    // substract top-left of absolute VisibleRange
    aTransform.translate(
        -aVisibleRange.getMinX(),
        -aVisibleRange.getMinY());

    // scale to target pixels (if needed)
    if(bNeedToReduce)
    {
        aTransform.scale(fReduceFactor, fReduceFactor);
    }

    // invert to get transformation from target pixel coordiates to source pixels
    aTransform.invert();

    // create bitmap using source, destination and linear back-transformation
    aRetval = TransformBitmapEx(fWidth, fHeight, aTransform, bSmooth);

    return aRetval;
}

// ------------------------------------------------------------------

BitmapEx BitmapEx::ModifyBitmapEx(const basegfx::BColorModifierStack& rBColorModifierStack) const
{
    Bitmap aChangedBitmap(GetBitmap());
    bool bDone(false);

    for(sal_uInt32 a(rBColorModifierStack.count()); a && !bDone; )
    {
        const basegfx::BColorModifier& rModifier = rBColorModifierStack.getBColorModifier(--a);

        switch(rModifier.getMode())
        {
            case basegfx::BCOLORMODIFYMODE_REPLACE :
            {
                // complete replace
                if(IsTransparent())
                {
                    // clear bitmap with dest color
                    if(aChangedBitmap.GetBitCount() <= 8)
                    {
                        // do NOT use erase; for e.g. 8bit Bitmaps, the nearest color to the given
                        // erase color is determined and used -> this may be different from what is
                        // wanted here. Better create a new bitmap with the needed color explicitely
                        BitmapReadAccess* pReadAccess = aChangedBitmap.AcquireReadAccess();
                        OSL_ENSURE(pReadAccess, "Got no Bitmap ReadAccess ?!?");

                        if(pReadAccess)
                        {
                            BitmapPalette aNewPalette(pReadAccess->GetPalette());
                            aNewPalette[0] = BitmapColor(Color(rModifier.getBColor()));
                            aChangedBitmap = Bitmap(
                                aChangedBitmap.GetSizePixel(),
                                aChangedBitmap.GetBitCount(),
                                &aNewPalette);
                            delete pReadAccess;
                        }
                    }
                    else
                    {
                        aChangedBitmap.Erase(Color(rModifier.getBColor()));
                    }
                }
                else
                {
                    // erase bitmap, caller will know to paint direct
                    aChangedBitmap.SetEmpty();
                }

                bDone = true;
                break;
            }

            default : // BCOLORMODIFYMODE_INTERPOLATE, BCOLORMODIFYMODE_GRAY, BCOLORMODIFYMODE_BLACKANDWHITE
            {
                BitmapWriteAccess* pContent = aChangedBitmap.AcquireWriteAccess();

                if(pContent)
                {
                    const double fConvertColor(1.0 / 255.0);

                    for(sal_uInt32 y(0L); y < (sal_uInt32)pContent->Height(); y++)
                    {
                        for(sal_uInt32 x(0L); x < (sal_uInt32)pContent->Width(); x++)
                        {
                            const BitmapColor aBMCol(pContent->GetColor(y, x));
                            const basegfx::BColor aBSource(
                                (double)aBMCol.GetRed() * fConvertColor,
                                (double)aBMCol.GetGreen() * fConvertColor,
                                (double)aBMCol.GetBlue() * fConvertColor);
                            const basegfx::BColor aBDest(rModifier.getModifiedColor(aBSource));

                            pContent->SetPixel(y, x, BitmapColor(Color(aBDest)));
                        }
                    }

                    delete pContent;
                }

                break;
            }
        }
    }

    if(aChangedBitmap.IsEmpty())
    {
        return BitmapEx();
    }
    else
    {
        if(IsTransparent())
        {
            if(IsAlpha())
            {
                return BitmapEx(aChangedBitmap, GetAlpha());
            }
            else
            {
                return BitmapEx(aChangedBitmap, GetMask());
            }
        }
        else
        {
            return BitmapEx(aChangedBitmap);
        }
    }
}

// -----------------------------------------------------------------------------

BitmapEx VCL_DLLPUBLIC createBlendFrame(
    const Size& rSize,
    sal_uInt8 nAlpha,
    Color aColorTopLeft,
    Color aColorBottomRight)
{
    const sal_uInt32 nW(rSize.Width());
    const sal_uInt32 nH(rSize.Height());

    if(nW || nH)
    {
        Color aColTopRight(aColorTopLeft);
        Color aColBottomLeft(aColorTopLeft);
        const sal_uInt32 nDE(nW + nH);

        aColTopRight.Merge(aColorBottomRight, 255 - sal_uInt8((nW * 255) / nDE));
        aColBottomLeft.Merge(aColorBottomRight, 255 - sal_uInt8((nH * 255) / nDE));

        return createBlendFrame(rSize, nAlpha, aColorTopLeft, aColTopRight, aColorBottomRight, aColBottomLeft);
    }

    return BitmapEx();
}

BitmapEx VCL_DLLPUBLIC createBlendFrame(
    const Size& rSize,
    sal_uInt8 nAlpha,
    Color aColorTopLeft,
    Color aColorTopRight,
    Color aColorBottomRight,
    Color aColorBottomLeft)
{
    BlendFrameCache* pBlendFrameCache = ImplGetBlendFrameCache();

    if(pBlendFrameCache->m_aLastSize == rSize
        && pBlendFrameCache->m_nLastAlpha == nAlpha
        && pBlendFrameCache->m_aLastColorTopLeft == aColorTopLeft
        && pBlendFrameCache->m_aLastColorTopRight == aColorTopRight
        && pBlendFrameCache->m_aLastColorBottomRight == aColorBottomRight
        && pBlendFrameCache->m_aLastColorBottomLeft == aColorBottomLeft)
    {
        return pBlendFrameCache->m_aLastResult;
    }

    pBlendFrameCache->m_aLastSize = rSize;
    pBlendFrameCache->m_nLastAlpha = nAlpha;
    pBlendFrameCache->m_aLastColorTopLeft = aColorTopLeft;
    pBlendFrameCache->m_aLastColorTopRight = aColorTopRight;
    pBlendFrameCache->m_aLastColorBottomRight = aColorBottomRight;
    pBlendFrameCache->m_aLastColorBottomLeft = aColorBottomLeft;
    pBlendFrameCache->m_aLastResult.Clear();

    const long nW(rSize.Width());
    const long nH(rSize.Height());

    if(nW > 1 && nH > 1)
    {
        sal_uInt8 aEraseTrans(0xff);
        Bitmap aContent(rSize, 24);
        AlphaMask aAlpha(rSize, &aEraseTrans);

        aContent.Erase(COL_BLACK);

        BitmapWriteAccess* pContent = aContent.AcquireWriteAccess();
        BitmapWriteAccess* pAlpha = aAlpha.AcquireWriteAccess();

        if(pContent && pAlpha)
        {
            long x(0);
            long y(0);

            // x == 0, y == 0
            pContent->SetPixel(y, x, aColorTopLeft);
            pAlpha->SetPixelIndex(y, x, nAlpha);

            for(x = 1; x < nW - 1; x++) // y == 0
            {
                Color aMix(aColorTopLeft);

                aMix.Merge(aColorTopRight, 255 - sal_uInt8((x * 255) / nW));
                pContent->SetPixel(y, x, aMix);
                pAlpha->SetPixelIndex(y, x, nAlpha);
            }

            // x == nW - 1, y == 0
            pContent->SetPixel(y, x, aColorTopRight);
            pAlpha->SetPixelIndex(y, x, nAlpha);

            for(y = 1; y < nH - 1; y++) // x == 0 and nW - 1
            {
                Color aMixA(aColorTopLeft);
                Color aMixB(aColorTopRight);

                aMixA.Merge(aColorBottomLeft, 255 - sal_uInt8((y * 255) / nH));
                pContent->SetPixel(y, 0, aMixA);
                pAlpha->SetPixelIndex(y, 0, nAlpha);

                aMixB.Merge(aColorBottomRight, 255 - sal_uInt8((y * 255) / nH));
                pContent->SetPixel(y, nW - 1, aMixB);
                pAlpha->SetPixelIndex(y, nW - 1, nAlpha);
            }

            x = 0; // x == 0, y == nH - 1
            pContent->SetPixel(y, x, aColorBottomLeft);
            pAlpha->SetPixelIndex(y, x, nAlpha);

            for(x = 1; x < nW - 1; x++) // y == nH - 1
            {
                Color aMix(aColorBottomLeft);

                aMix.Merge(aColorBottomRight, 255 - sal_uInt8(((x - 0)* 255) / nW));
                pContent->SetPixel(y, x, aMix);
                pAlpha->SetPixelIndex(y, x, nAlpha);
            }

            // x == nW - 1, y == nH - 1
            pContent->SetPixel(y, x, aColorBottomRight);
            pAlpha->SetPixelIndex(y, x, nAlpha);

            aContent.ReleaseAccess(pContent);
            aAlpha.ReleaseAccess(pAlpha);

            pBlendFrameCache->m_aLastResult = BitmapEx(aContent, aAlpha);
        }
        else
        {
            if(pContent)
            {
                aContent.ReleaseAccess(pContent);
            }

            if(pAlpha)
            {
                aAlpha.ReleaseAccess(pAlpha);
            }
        }
    }

    return pBlendFrameCache->m_aLastResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
