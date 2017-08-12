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

#include <rtl/crc.h>
#include <rtl/strbuf.hxx>

#include <o3tl/any.hxx>
#include <tools/debug.hxx>
#include <unotools/resmgr.hxx>
#include <tools/stream.hxx>
#include <vcl/ImageTree.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/outdev.hxx>
#include <vcl/alpha.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/pngread.hxx>
#include <vcl/svapp.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/virdev.hxx>
#include <vcl/settings.hxx>

#include <image.h>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

// BitmapEx::Create
#include <salbmp.hxx>
#include <salinst.hxx>
#include <svdata.hxx>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <memory>

using namespace ::com::sun::star;

BitmapEx::BitmapEx()
    : eTransparent(TransparentType::NONE)
    , bAlpha(false)
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

BitmapEx::BitmapEx( const BitmapEx& rBitmapEx, Point aSrc, Size aSize )
    : eTransparent(TransparentType::NONE)
    , bAlpha(false)
{
    if( rBitmapEx.IsEmpty() )
        return;

    aBitmap = Bitmap( aSize, rBitmapEx.aBitmap.GetBitCount() );
    aBitmapSize = aSize;
    if( rBitmapEx.IsAlpha() )
    {
        bAlpha = true;
        aMask = AlphaMask( aSize ).ImplGetBitmap();
    }
    else if( rBitmapEx.IsTransparent() )
        aMask = Bitmap( aSize, rBitmapEx.aMask.GetBitCount() );

    tools::Rectangle aDestRect( Point( 0, 0 ), aSize );
    tools::Rectangle aSrcRect( aSrc, aSize );
    CopyPixel( aDestRect, aSrcRect, &rBitmapEx );
}

BitmapEx::BitmapEx( const OUString& rIconName )
    : eTransparent(TransparentType::NONE)
    , bAlpha(false)
{
    loadFromIconTheme( rIconName );
}

void BitmapEx::loadFromIconTheme( const OUString& rIconName )
{
    bool bSuccess;
    OUString aIconTheme;

    try
    {
        aIconTheme = Application::GetSettings().GetStyleSettings().DetermineIconTheme();
        bSuccess = ImageTree::get().loadImage(rIconName, aIconTheme, *this, true);
    }
    catch (...)
    {
        bSuccess = false;
    }

    SAL_WARN_IF( !bSuccess, "vcl", "BitmapEx::BitmapEx(): could not load image " << rIconName << " via icon theme " << aIconTheme);
}

BitmapEx::BitmapEx( const Bitmap& rBmp ) :
        aBitmap     ( rBmp ),
        aBitmapSize ( aBitmap.GetSizePixel() ),
        eTransparent( TransparentType::NONE ),
        bAlpha      ( false )
{
}

BitmapEx::BitmapEx( const Bitmap& rBmp, const Bitmap& rMask ) :
        aBitmap         ( rBmp ),
        aMask           ( rMask ),
        aBitmapSize     ( aBitmap.GetSizePixel() ),
        eTransparent    ( !rMask ? TransparentType::NONE : TransparentType::Bitmap ),
        bAlpha          ( false )
{
    // Ensure a mask is exactly one bit deep
    if( !!aMask && aMask.GetBitCount() != 1 )
    {
        SAL_WARN( "vcl", "BitmapEx: forced mask to monochrome");
        aMask.ImplMakeMono( 255 );
    }

    if(!!aBitmap && !!aMask && aBitmap.GetSizePixel() != aMask.GetSizePixel())
    {
        OSL_ENSURE(false, "Mask size differs from Bitmap size, corrected Mask (!)");
        aMask.Scale(aBitmap.GetSizePixel());
    }
}

BitmapEx::BitmapEx( const Bitmap& rBmp, const AlphaMask& rAlphaMask ) :
        aBitmap         ( rBmp ),
        aMask           ( rAlphaMask.ImplGetBitmap() ),
        aBitmapSize     ( aBitmap.GetSizePixel() ),
        eTransparent    ( !rAlphaMask ? TransparentType::NONE : TransparentType::Bitmap ),
        bAlpha          ( !rAlphaMask.IsEmpty() )
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
        aBitmap.Convert( BmpConversion::N24Bit );
}

BitmapEx::BitmapEx( const Bitmap& rBmp, const Color& rTransparentColor ) :
        aBitmap             ( rBmp ),
        aBitmapSize         ( aBitmap.GetSizePixel() ),
        aTransparentColor   ( rTransparentColor ),
        eTransparent        ( TransparentType::Bitmap ),
        bAlpha              ( false )
{
    aMask = aBitmap.CreateMask( aTransparentColor );

    SAL_WARN_IF( rBmp.GetSizePixel() != aMask.GetSizePixel(), "vcl",
                "BitmapEx::BitmapEx(): size mismatch for bitmap and alpha mask." );
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

bool BitmapEx::operator==( const BitmapEx& rBitmapEx ) const
{
    if( eTransparent != rBitmapEx.eTransparent )
        return false;

    if( aBitmap != rBitmapEx.aBitmap )
        return false;

    if( aBitmapSize != rBitmapEx.aBitmapSize )
        return false;

    if( eTransparent == TransparentType::NONE )
        return true;

    if( eTransparent == TransparentType::Color )
        return aTransparentColor == rBitmapEx.aTransparentColor;

    return( ( aMask == rBitmapEx.aMask ) && ( bAlpha == rBitmapEx.bAlpha ) );
}

bool BitmapEx::IsEmpty() const
{
    return( aBitmap.IsEmpty() && aMask.IsEmpty() );
}

void BitmapEx::SetEmpty()
{
    aBitmap.SetEmpty();
    aMask.SetEmpty();
    eTransparent = TransparentType::NONE;
    bAlpha = false;
}

void BitmapEx::Clear()
{
    SetEmpty();
}

bool BitmapEx::IsTransparent() const
{
    return( eTransparent != TransparentType::NONE );
}

bool BitmapEx::IsAlpha() const
{
    return( IsTransparent() && bAlpha );
}

const Bitmap& BitmapEx::GetBitmapRef() const
{
    return aBitmap;
}

Bitmap BitmapEx::GetBitmap( const Color* pTransReplaceColor ) const
{
    Bitmap aRetBmp( aBitmap );

    if( pTransReplaceColor && ( eTransparent != TransparentType::NONE ) )
    {
        Bitmap aTempMask;

        if( eTransparent == TransparentType::Color )
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

    if( eTransparent == TransparentType::Bitmap )
        nSizeBytes += aMask.GetSizeBytes();

    return nSizeBytes;
}

BitmapChecksum BitmapEx::GetChecksum() const
{
    BitmapChecksum  nCrc = aBitmap.GetChecksum();
    SVBT32      aBT32;
    BitmapChecksumOctetArray aBCOA;

    UInt32ToSVBT32( (long) eTransparent, aBT32 );
    nCrc = vcl_get_checksum( nCrc, aBT32, 4 );

    UInt32ToSVBT32( (long) bAlpha, aBT32 );
    nCrc = vcl_get_checksum( nCrc, aBT32, 4 );

    if( ( TransparentType::Bitmap == eTransparent ) && !aMask.IsEmpty() )
    {
        BCToBCOA( aMask.GetChecksum(), aBCOA );
        nCrc = vcl_get_checksum( nCrc, aBCOA, BITMAP_CHECKSUM_SIZE );
    }

    return nCrc;
}

void BitmapEx::SetSizePixel( const Size& rNewSize, BmpScaleFlag nScaleFlag )
{
    if(GetSizePixel() != rNewSize)
    {
        Scale( rNewSize, nScaleFlag );
    }
}

bool BitmapEx::Invert()
{
    bool bRet = false;

    if( !!aBitmap )
    {
        bRet = aBitmap.Invert();

        if( bRet && ( eTransparent == TransparentType::Color ) )
            aTransparentColor = BitmapColor( aTransparentColor ).Invert();
    }

    return bRet;
}

bool BitmapEx::Mirror( BmpMirrorFlags nMirrorFlags )
{
    bool bRet = false;

    if( !!aBitmap )
    {
        bRet = aBitmap.Mirror( nMirrorFlags );

        if( bRet && ( eTransparent == TransparentType::Bitmap ) && !!aMask )
            aMask.Mirror( nMirrorFlags );
    }

    return bRet;
}

bool BitmapEx::Scale( const double& rScaleX, const double& rScaleY, BmpScaleFlag nScaleFlag )
{
    bool bRet = false;

    if( !!aBitmap )
    {
        bRet = aBitmap.Scale( rScaleX, rScaleY, nScaleFlag );

        if( bRet && ( eTransparent == TransparentType::Bitmap ) && !!aMask )
        {
            aMask.Scale( rScaleX, rScaleY, nScaleFlag );
        }

        aBitmapSize = aBitmap.GetSizePixel();

        SAL_WARN_IF( !!aMask && aBitmap.GetSizePixel() != aMask.GetSizePixel(), "vcl",
                    "BitmapEx::Scale(): size mismatch for bitmap and alpha mask." );
    }

    return bRet;
}

bool BitmapEx::Scale( const Size& rNewSize, BmpScaleFlag nScaleFlag )
{
    bool bRet;

    if( aBitmapSize.Width() && aBitmapSize.Height() &&
        ( rNewSize.Width()  != aBitmapSize.Width() ||
          rNewSize.Height() != aBitmapSize.Height() ) )
    {
        bRet = Scale( (double) rNewSize.Width() / aBitmapSize.Width(),
                      (double) rNewSize.Height() / aBitmapSize.Height(),
                      nScaleFlag );
    }
    else
        bRet = true;

    return bRet;
}

bool BitmapEx::Rotate( long nAngle10, const Color& rFillColor )
{
    bool bRet = false;

    if( !!aBitmap )
    {
        const bool bTransRotate = ( Color( COL_TRANSPARENT ) == rFillColor );

        if( bTransRotate )
        {
            if( eTransparent == TransparentType::Color )
                bRet = aBitmap.Rotate( nAngle10, aTransparentColor );
            else
            {
                bRet = aBitmap.Rotate( nAngle10, COL_BLACK );

                if( eTransparent == TransparentType::NONE )
                {
                    aMask = Bitmap( aBitmapSize, 1 );
                    aMask.Erase( COL_BLACK );
                    eTransparent = TransparentType::Bitmap;
                }

                if( bRet && !!aMask )
                    aMask.Rotate( nAngle10, COL_WHITE );
            }
        }
        else
        {
            bRet = aBitmap.Rotate( nAngle10, rFillColor );

            if( bRet && ( eTransparent == TransparentType::Bitmap ) && !!aMask )
                aMask.Rotate( nAngle10, COL_WHITE );
        }

        aBitmapSize = aBitmap.GetSizePixel();

        SAL_WARN_IF( !!aMask && aBitmap.GetSizePixel() != aMask.GetSizePixel(), "vcl",
                    "BitmapEx::Rotate(): size mismatch for bitmap and alpha mask." );
    }

    return bRet;
}

bool BitmapEx::Crop( const tools::Rectangle& rRectPixel )
{
    bool bRet = false;

    if( !!aBitmap )
    {
        bRet = aBitmap.Crop( rRectPixel );

        if( bRet && ( eTransparent == TransparentType::Bitmap ) && !!aMask )
            aMask.Crop( rRectPixel );

        aBitmapSize = aBitmap.GetSizePixel();

        SAL_WARN_IF( !!aMask && aBitmap.GetSizePixel() != aMask.GetSizePixel(), "vcl",
                    "BitmapEx::Crop(): size mismatch for bitmap and alpha mask." );
    }

    return bRet;
}

bool BitmapEx::Convert( BmpConversion eConversion )
{
    return !!aBitmap && aBitmap.Convert( eConversion );
}

bool BitmapEx::ReduceColors( sal_uInt16 nNewColorCount )
{
    return !!aBitmap && aBitmap.ReduceColors( nNewColorCount, BMP_REDUCE_POPULAR );
}

bool BitmapEx::Expand( sal_uLong nDX, sal_uLong nDY, bool bExpandTransparent )
{
    bool bRet = false;

    if( !!aBitmap )
    {
        bRet = aBitmap.Expand( nDX, nDY );

        if( bRet && ( eTransparent == TransparentType::Bitmap ) && !!aMask )
        {
            Color aColor( bExpandTransparent ? COL_WHITE : COL_BLACK );
            aMask.Expand( nDX, nDY, &aColor );
        }

        aBitmapSize = aBitmap.GetSizePixel();

        SAL_WARN_IF( !!aMask && aBitmap.GetSizePixel() != aMask.GetSizePixel(), "vcl",
                    "BitmapEx::Expand(): size mismatch for bitmap and alpha mask." );
    }

    return bRet;
}

bool BitmapEx::CopyPixel( const tools::Rectangle& rRectDst, const tools::Rectangle& rRectSrc,
                          const BitmapEx* pBmpExSrc )
{
    bool bRet = false;

    if( !pBmpExSrc || pBmpExSrc->IsEmpty() )
    {
        if( !aBitmap.IsEmpty() )
        {
            bRet = aBitmap.CopyPixel( rRectDst, rRectSrc );

            if( bRet && ( eTransparent == TransparentType::Bitmap ) && !!aMask )
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
                        aMask.CopyPixel_AlphaOptimized( rRectDst, rRectSrc, &pBmpExSrc->aMask );
                    else if( IsTransparent() )
                    {
                        AlphaMask* pAlpha = new AlphaMask( aMask );

                        aMask = pAlpha->ImplGetBitmap();
                        delete pAlpha;
                        bAlpha = true;
                        aMask.CopyPixel( rRectDst, rRectSrc, &pBmpExSrc->aMask );
                    }
                    else
                    {
                        sal_uInt8   cBlack = 0;
                        AlphaMask*  pAlpha = new AlphaMask( GetSizePixel(), &cBlack );

                        aMask = pAlpha->ImplGetBitmap();
                        delete pAlpha;
                        eTransparent = TransparentType::Bitmap;
                        bAlpha = true;
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
                        eTransparent = TransparentType::Bitmap;
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

bool BitmapEx::Erase( const Color& rFillColor )
{
    bool bRet = false;

    if( !!aBitmap )
    {
        bRet = aBitmap.Erase( rFillColor );

        if( bRet && ( eTransparent == TransparentType::Bitmap ) && !!aMask )
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

bool BitmapEx::Dither()
{
    return !!aBitmap && aBitmap.Dither();
}

bool BitmapEx::Replace( const Color& rSearchColor, const Color& rReplaceColor )
{
    return !!aBitmap && aBitmap.Replace( rSearchColor, rReplaceColor );
}

bool BitmapEx::Replace( const Color* pSearchColors, const Color* pReplaceColors, sal_uLong nColorCount, const sal_uLong* pTols )
{
    return !!aBitmap && aBitmap.Replace( pSearchColors, pReplaceColors, nColorCount, const_cast<sal_uLong*>(pTols) );
}

bool BitmapEx::Adjust( short nLuminancePercent, short nContrastPercent,
                       short nChannelRPercent, short nChannelGPercent, short nChannelBPercent,
                       double fGamma, bool bInvert, bool msoBrightness )
{
    return !!aBitmap && aBitmap.Adjust( nLuminancePercent, nContrastPercent,
                                        nChannelRPercent, nChannelGPercent, nChannelBPercent,
                                        fGamma, bInvert, msoBrightness );
}

bool BitmapEx::Filter( BmpFilter eFilter, const BmpFilterParam* pFilterParam )
{
    return !!aBitmap && aBitmap.Filter( eFilter, pFilterParam );
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

BitmapEx BitmapEx:: AutoScaleBitmap(BitmapEx const & aBitmap, const long aStandardSize)
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
        aRet.Scale( aScaledSize, BmpScaleFlag::BestQuality );
    }
    else
    {
        imgposX = (aStandardSize - imgOldWidth) / 2 + 0.5;
        imgposY = (aStandardSize - imgOldHeight) / 2 + 0.5;
    }

    Size aStdSize( aStandardSize, aStandardSize );
    tools::Rectangle aRect(aEmptyPoint, aStdSize );

    ScopedVclPtrInstance< VirtualDevice > aVirDevice(*Application::GetDefaultDevice(),
                                                     DeviceFormat::DEFAULT, DeviceFormat::BITMASK);
    aVirDevice->SetOutputSizePixel( aStdSize );
    aVirDevice->SetFillColor( COL_TRANSPARENT );
    aVirDevice->SetLineColor( COL_TRANSPARENT );

    // Draw a rect into virDevice
    aVirDevice->DrawRect( aRect );
    Point aPointPixel( (long)imgposX, (long)imgposY );
    aVirDevice->DrawBitmapEx( aPointPixel, aRet );
    aRet = aVirDevice->GetBitmapEx( aEmptyPoint, aStdSize );

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
                case TransparentType::NONE:
                {
                    // Not transparent, ergo all covered
                    nTransparency = 0x00;
                    break;
                }
                case TransparentType::Color:
                {
                    Bitmap aTestBitmap(aBitmap);
                    Bitmap::ScopedReadAccess pRead(aTestBitmap);

                    if(pRead)
                    {
                        const Color aColor = pRead->GetColor(nY, nX);

                        // If color is not equal to TransparentColor, we are not transparent
                        if(aColor != aTransparentColor)
                        {
                            nTransparency = 0x00;
                        }

                    }
                    break;
                }
                case TransparentType::Bitmap:
                {
                    if(!aMask.IsEmpty())
                    {
                        Bitmap aTestBitmap(aMask);
                        Bitmap::ScopedReadAccess pRead(aTestBitmap);

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
bool BitmapEx::Create( const css::uno::Reference< css::rendering::XBitmapCanvas > &xBitmapCanvas,
                       const Size &rSize )
{
    uno::Reference< beans::XFastPropertySet > xFastPropertySet( xBitmapCanvas, uno::UNO_QUERY );
    if( xFastPropertySet.get() )
    {
        // 0 means get BitmapEx
        uno::Any aAny = xFastPropertySet->getFastPropertyValue( 0 );
        std::unique_ptr<BitmapEx> xBitmapEx(reinterpret_cast<BitmapEx*>(*o3tl::doAccess<sal_Int64>(aAny)));
        if( xBitmapEx )
        {
            *this = *xBitmapEx;
            return true;
        }
    }

    SalBitmap* pSalBmp = nullptr;
    SalBitmap* pSalMask = nullptr;

    pSalBmp = ImplGetSVData()->mpDefInst->CreateSalBitmap();

    Size aLocalSize(rSize);
    if( pSalBmp->Create( xBitmapCanvas, aLocalSize ) )
    {
        pSalMask = ImplGetSVData()->mpDefInst->CreateSalBitmap();
        if ( pSalMask->Create( xBitmapCanvas, aLocalSize, true ) )
        {
            *this = BitmapEx(Bitmap(pSalBmp), Bitmap(pSalMask) );
            return true;
        }
        else
        {
            delete pSalMask;
            *this = BitmapEx(Bitmap(pSalBmp));
            return true;
        }
    }

    delete pSalBmp;
    delete pSalMask;

    return false;
}

namespace
{
    Bitmap impTransformBitmap(
        const Bitmap& rSource,
        const Size& rDestinationSize,
        const basegfx::B2DHomMatrix& rTransform,
        bool bSmooth)
    {
        Bitmap aDestination(rDestinationSize, 24);
        Bitmap::ScopedWriteAccess xWrite(aDestination);

        if(xWrite)
        {
            Bitmap::ScopedReadAccess xRead(const_cast< Bitmap& >(rSource));

            if (xRead)
            {
                const Size aDestinationSizePixel(aDestination.GetSizePixel());
                const BitmapColor aOutside(BitmapColor(0xff, 0xff, 0xff));

                for(long y(0); y < aDestinationSizePixel.getHeight(); y++)
                {
                    for(long x(0); x < aDestinationSizePixel.getWidth(); x++)
                    {
                        const basegfx::B2DPoint aSourceCoor(rTransform * basegfx::B2DPoint(x, y));

                        if(bSmooth)
                        {
                            xWrite->SetPixel(
                                y,
                                x,
                                xRead->GetInterpolatedColorWithFallback(
                                    aSourceCoor.getY(),
                                    aSourceCoor.getX(),
                                    aOutside));
                        }
                        else
                        {
                            // this version does the correct <= 0.0 checks, so no need
                            // to do the static_cast< sal_Int32 > self and make an error
                            xWrite->SetPixel(
                                y,
                                x,
                                xRead->GetColorWithFallback(
                                    aSourceCoor.getY(),
                                    aSourceCoor.getX(),
                                    aOutside));
                        }
                    }
                }
            }
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

    // subtract top-left of absolute VisibleRange
    aTransform.translate(
        -aVisibleRange.getMinX(),
        -aVisibleRange.getMinY());

    // scale to target pixels (if needed)
    if(bNeedToReduce)
    {
        aTransform.scale(fReduceFactor, fReduceFactor);
    }

    // invert to get transformation from target pixel coordinates to source pixels
    aTransform.invert();

    // create bitmap using source, destination and linear back-transformation
    aRetval = TransformBitmapEx(fWidth, fHeight, aTransform, bSmooth);

    return aRetval;
}

BitmapEx BitmapEx::ModifyBitmapEx(const basegfx::BColorModifierStack& rBColorModifierStack) const
{
    Bitmap aChangedBitmap(GetBitmap());
    bool bDone(false);

    for(sal_uInt32 a(rBColorModifierStack.count()); a && !bDone; )
    {
        const basegfx::BColorModifierSharedPtr& rModifier = rBColorModifierStack.getBColorModifier(--a);
        const basegfx::BColorModifier_replace* pReplace = dynamic_cast< const basegfx::BColorModifier_replace* >(rModifier.get());

        if(pReplace)
        {
            // complete replace
            if(IsTransparent())
            {
                // clear bitmap with dest color
                if(aChangedBitmap.GetBitCount() <= 8)
                {
                    // do NOT use erase; for e.g. 8bit Bitmaps, the nearest color to the given
                    // erase color is determined and used -> this may be different from what is
                    // wanted here. Better create a new bitmap with the needed color explicitly
                    Bitmap::ScopedReadAccess xReadAccess(aChangedBitmap);
                    OSL_ENSURE(xReadAccess, "Got no Bitmap ReadAccess ?!?");

                    if(xReadAccess)
                    {
                        BitmapPalette aNewPalette(xReadAccess->GetPalette());
                        aNewPalette[0] = BitmapColor(Color(pReplace->getBColor()));
                        aChangedBitmap = Bitmap(
                            aChangedBitmap.GetSizePixel(),
                            aChangedBitmap.GetBitCount(),
                            &aNewPalette);
                    }
                }
                else
                {
                    aChangedBitmap.Erase(Color(pReplace->getBColor()));
                }
            }
            else
            {
                // erase bitmap, caller will know to paint direct
                aChangedBitmap.SetEmpty();
            }

            bDone = true;
        }
        else
        {
            Bitmap::ScopedWriteAccess xContent(aChangedBitmap);

            if(xContent)
            {
                const double fConvertColor(1.0 / 255.0);

                if(xContent->HasPalette())
                {
                    const sal_uInt16 nCount(xContent->GetPaletteEntryCount());

                    for(sal_uInt16 b(0); b < nCount; b++)
                    {
                        const BitmapColor& rCol = xContent->GetPaletteColor(b);
                        const basegfx::BColor aBSource(
                            rCol.GetRed() * fConvertColor,
                            rCol.GetGreen() * fConvertColor,
                            rCol.GetBlue() * fConvertColor);
                        const basegfx::BColor aBDest(rModifier->getModifiedColor(aBSource));
                        xContent->SetPaletteColor(b, BitmapColor(Color(aBDest)));
                    }
                }
                else if(ScanlineFormat::N24BitTcBgr == xContent->GetScanlineFormat())
                {
                    for(sal_uInt32 y(0); y < (sal_uInt32)xContent->Height(); y++)
                    {
                        Scanline pScan = xContent->GetScanline(y);

                        for(sal_uInt32 x(0); x < (sal_uInt32)xContent->Width(); x++)
                        {
                            const basegfx::BColor aBSource(
                                *(pScan + 2)* fConvertColor,
                                *(pScan + 1) * fConvertColor,
                                *pScan * fConvertColor);
                            const basegfx::BColor aBDest(rModifier->getModifiedColor(aBSource));
                            *pScan++ = static_cast< sal_uInt8 >(aBDest.getBlue() * 255.0);
                            *pScan++ = static_cast< sal_uInt8 >(aBDest.getGreen() * 255.0);
                            *pScan++ = static_cast< sal_uInt8 >(aBDest.getRed() * 255.0);
                        }
                    }
                }
                else if(ScanlineFormat::N24BitTcRgb == xContent->GetScanlineFormat())
                {
                    for(sal_uInt32 y(0); y < (sal_uInt32)xContent->Height(); y++)
                    {
                        Scanline pScan = xContent->GetScanline(y);

                        for(sal_uInt32 x(0); x < (sal_uInt32)xContent->Width(); x++)
                        {
                            const basegfx::BColor aBSource(
                                *pScan * fConvertColor,
                                *(pScan + 1) * fConvertColor,
                                *(pScan + 2) * fConvertColor);
                            const basegfx::BColor aBDest(rModifier->getModifiedColor(aBSource));
                            *pScan++ = static_cast< sal_uInt8 >(aBDest.getRed() * 255.0);
                            *pScan++ = static_cast< sal_uInt8 >(aBDest.getGreen() * 255.0);
                            *pScan++ = static_cast< sal_uInt8 >(aBDest.getBlue() * 255.0);
                        }
                    }
                }
                else
                {
                    for(sal_uInt32 y(0); y < (sal_uInt32)xContent->Height(); y++)
                    {
                        for(sal_uInt32 x(0); x < (sal_uInt32)xContent->Width(); x++)
                        {
                            const BitmapColor aBMCol(xContent->GetColor(y, x));
                            const basegfx::BColor aBSource(
                                (double)aBMCol.GetRed() * fConvertColor,
                                (double)aBMCol.GetGreen() * fConvertColor,
                                (double)aBMCol.GetBlue() * fConvertColor);
                            const basegfx::BColor aBDest(rModifier->getModifiedColor(aBSource));

                            xContent->SetPixel(y, x, BitmapColor(Color(aBDest)));
                        }
                    }
                }
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

BitmapEx createBlendFrame(
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

BitmapEx createBlendFrame(
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

        Bitmap::ScopedWriteAccess pContent(aContent);
        AlphaMask::ScopedWriteAccess pAlpha(aAlpha);

        if(pContent && pAlpha)
        {
            long x(0);
            long y(0);

            // x == 0, y == 0, top-left corner
            pContent->SetPixel(0, 0, aColorTopLeft);
            pAlpha->SetPixelIndex(0, 0, nAlpha);

            // y == 0, top line left to right
            for(x = 1; x < nW - 1; x++)
            {
                Color aMix(aColorTopLeft);

                aMix.Merge(aColorTopRight, 255 - sal_uInt8((x * 255) / nW));
                pContent->SetPixel(0, x, aMix);
                pAlpha->SetPixelIndex(0, x, nAlpha);
            }

            // x == nW - 1, y == 0, top-right corner
            // #i123690# Caution! When nW is 1, x == nW is possible (!)
            if(x < nW)
            {
                pContent->SetPixel(0, x, aColorTopRight);
                pAlpha->SetPixelIndex(0, x, nAlpha);
            }

            // x == 0 and nW - 1, left and right line top-down
            for(y = 1; y < nH - 1; y++)
            {
                Color aMixA(aColorTopLeft);

                aMixA.Merge(aColorBottomLeft, 255 - sal_uInt8((y * 255) / nH));
                pContent->SetPixel(y, 0, aMixA);
                pAlpha->SetPixelIndex(y, 0, nAlpha);

                // #i123690# Caution! When nW is 1, x == nW is possible (!)
                if(x < nW)
                {
                    Color aMixB(aColorTopRight);

                    aMixB.Merge(aColorBottomRight, 255 - sal_uInt8((y * 255) / nH));
                    pContent->SetPixel(y, x, aMixB);
                    pAlpha->SetPixelIndex(y, x, nAlpha);
                }
            }

            // #i123690# Caution! When nH is 1, y == nH is possible (!)
            if(y < nH)
            {
                // x == 0, y == nH - 1, bottom-left corner
                pContent->SetPixel(y, 0, aColorBottomLeft);
                pAlpha->SetPixelIndex(y, 0, nAlpha);

                // y == nH - 1, bottom line left to right
                for(x = 1; x < nW - 1; x++)
                {
                    Color aMix(aColorBottomLeft);

                    aMix.Merge(aColorBottomRight, 255 - sal_uInt8(((x - 0)* 255) / nW));
                    pContent->SetPixel(y, x, aMix);
                    pAlpha->SetPixelIndex(y, x, nAlpha);
                }

                // x == nW - 1, y == nH - 1, bottom-right corner
                // #i123690# Caution! When nW is 1, x == nW is possible (!)
                if(x < nW)
                {
                    pContent->SetPixel(y, x, aColorBottomRight);
                    pAlpha->SetPixelIndex(y, x, nAlpha);
                }
            }

            pContent.reset();
            pAlpha.reset();

            pBlendFrameCache->m_aLastResult = BitmapEx(aContent, aAlpha);
        }
    }

    return pBlendFrameCache->m_aLastResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
