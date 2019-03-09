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
#include <sal/log.hxx>
#include <o3tl/underlyingenumvalue.hxx>
#include <osl/diagnose.h>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/color/bcolormodifier.hxx>
#include <unotools/resmgr.hxx>

#include <vcl/ImageTree.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/outdev.hxx>
#include <vcl/alpha.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/svapp.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/virdev.hxx>
#include <vcl/settings.hxx>
#include <vcl/BitmapMonochromeFilter.hxx>

// BitmapEx::Create
#include <salbmp.hxx>
#include <salinst.hxx>
#include <svdata.hxx>
#include <BitmapFastScaleFilter.hxx>
#include <bitmapwriteaccess.hxx>
#include <image.h>

#include <o3tl/any.hxx>

#include <com/sun/star/beans/XFastPropertySet.hpp>

#include <memory>

using namespace ::com::sun::star;

BitmapEx::BitmapEx()
    : meTransparent(TransparentType::NONE)
    , mbAlpha(false)
{
}

BitmapEx::BitmapEx( const BitmapEx& ) = default;

BitmapEx::BitmapEx( const BitmapEx& rBitmapEx, Point aSrc, Size aSize )
    : meTransparent(TransparentType::NONE)
    , mbAlpha(false)
{
    if( rBitmapEx.IsEmpty() )
        return;

    maBitmap = Bitmap( aSize, rBitmapEx.maBitmap.GetBitCount() );
    SetSizePixel(aSize);
    if( rBitmapEx.IsAlpha() )
    {
        mbAlpha = true;
        maMask = AlphaMask( aSize ).ImplGetBitmap();
    }
    else if( rBitmapEx.IsTransparent() )
        maMask = Bitmap( aSize, rBitmapEx.maMask.GetBitCount() );

    tools::Rectangle aDestRect( Point( 0, 0 ), aSize );
    tools::Rectangle aSrcRect( aSrc, aSize );
    CopyPixel( aDestRect, aSrcRect, &rBitmapEx );
}

BitmapEx::BitmapEx( const OUString& rIconName )
    : meTransparent(TransparentType::NONE)
    , mbAlpha(false)
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
        maBitmap     ( rBmp ),
        maBitmapSize ( maBitmap.GetSizePixel() ),
        meTransparent( TransparentType::NONE ),
        mbAlpha      ( false )
{
}

BitmapEx::BitmapEx( const Bitmap& rBmp, const Bitmap& rMask ) :
        maBitmap         ( rBmp ),
        maMask           ( rMask ),
        maBitmapSize     ( maBitmap.GetSizePixel() ),
        meTransparent    ( !rMask ? TransparentType::NONE : TransparentType::Bitmap ),
        mbAlpha          ( false )
{
    // Ensure a mask is exactly one bit deep
    if( !!maMask && maMask.GetBitCount() != 1 )
    {
        SAL_WARN( "vcl", "BitmapEx: forced mask to monochrome");
        BitmapEx aMaskEx(maMask);
        BitmapFilter::Filter(aMaskEx, BitmapMonochromeFilter(255));
        maMask = aMaskEx.GetBitmap();
    }

    if (!!maBitmap && !!maMask && maBitmap.GetSizePixel() != maMask.GetSizePixel())
    {
        OSL_ENSURE(false, "Mask size differs from Bitmap size, corrected Mask (!)");
        maMask.Scale(maBitmap.GetSizePixel());
    }
}

BitmapEx::BitmapEx( const Bitmap& rBmp, const AlphaMask& rAlphaMask ) :
        maBitmap         ( rBmp ),
        maMask           ( rAlphaMask.ImplGetBitmap() ),
        maBitmapSize     ( maBitmap.GetSizePixel() ),
        meTransparent    ( !rAlphaMask ? TransparentType::NONE : TransparentType::Bitmap ),
        mbAlpha          ( !rAlphaMask.IsEmpty() )
{
    if (!!maBitmap && !!maMask && maBitmap.GetSizePixel() != maMask.GetSizePixel())
    {
        OSL_ENSURE(false, "Alpha size differs from Bitmap size, corrected Mask (!)");
        maMask.Scale(rBmp.GetSizePixel());
    }

    // #i75531# the workaround below can go when
    // X11SalGraphics::drawAlphaBitmap()'s render acceleration
    // can handle the bitmap depth mismatch directly
    if( maBitmap.GetBitCount() < maMask.GetBitCount() )
        maBitmap.Convert( BmpConversion::N24Bit );
}

BitmapEx::BitmapEx( const Bitmap& rBmp, const Color& rTransparentColor ) :
        maBitmap             ( rBmp ),
        maBitmapSize         ( maBitmap.GetSizePixel() ),
        maTransparentColor   ( rTransparentColor ),
        meTransparent        ( TransparentType::Bitmap ),
        mbAlpha              ( false )
{
    maMask = maBitmap.CreateMask( maTransparentColor );

    SAL_WARN_IF(rBmp.GetSizePixel() != maMask.GetSizePixel(), "vcl",
                "BitmapEx::BitmapEx(): size mismatch for bitmap and alpha mask.");
}

BitmapEx& BitmapEx::operator=( const BitmapEx& ) = default;

bool BitmapEx::operator==( const BitmapEx& rBitmapEx ) const
{
    if (meTransparent != rBitmapEx.meTransparent)
        return false;

    if (!maBitmap.ShallowEquals(rBitmapEx.maBitmap))
        return false;

    if (GetSizePixel() != rBitmapEx.GetSizePixel())
        return false;

    if (meTransparent == TransparentType::NONE)
        return true;

    if (meTransparent == TransparentType::Color)
        return maTransparentColor == rBitmapEx.maTransparentColor;

    return maMask.ShallowEquals(rBitmapEx.maMask) && mbAlpha == rBitmapEx.mbAlpha;
}

bool BitmapEx::IsEmpty() const
{
    return( maBitmap.IsEmpty() && maMask.IsEmpty() );
}

void BitmapEx::SetEmpty()
{
    maBitmap.SetEmpty();
    maMask.SetEmpty();
    meTransparent = TransparentType::NONE;
    mbAlpha = false;
}

void BitmapEx::Clear()
{
    SetEmpty();
}

bool BitmapEx::IsTransparent() const
{
    return( meTransparent != TransparentType::NONE );
}

bool BitmapEx::IsAlpha() const
{
    return( IsTransparent() && mbAlpha );
}

const Bitmap& BitmapEx::GetBitmapRef() const
{
    return maBitmap;
}

Bitmap BitmapEx::GetBitmap( const Color* pTransReplaceColor ) const
{
    Bitmap aRetBmp( maBitmap );

    if( pTransReplaceColor && ( meTransparent != TransparentType::NONE ) )
    {
        Bitmap aTempMask;

        if( meTransparent == TransparentType::Color )
            aTempMask = maBitmap.CreateMask( maTransparentColor );
        else
            aTempMask = maMask;

        if( !IsAlpha() )
            aRetBmp.Replace( aTempMask, *pTransReplaceColor );
        else
            aRetBmp.Replace( GetAlpha(), *pTransReplaceColor );
    }

    return aRetBmp;
}

Bitmap BitmapEx::GetMask() const
{
    if (!IsAlpha())
        return maMask;

    BitmapEx aMaskEx(maMask);
    BitmapFilter::Filter(aMaskEx, BitmapMonochromeFilter(255));
    return aMaskEx.GetBitmap();
}

AlphaMask BitmapEx::GetAlpha() const
{
    if( IsAlpha() )
    {
        AlphaMask aAlpha;
        aAlpha.ImplSetBitmap( maMask );
        return aAlpha;
    }
    else
    {
        return AlphaMask(maMask);
    }
}

sal_uLong BitmapEx::GetSizeBytes() const
{
    sal_uLong nSizeBytes = maBitmap.GetSizeBytes();

    if( meTransparent == TransparentType::Bitmap )
        nSizeBytes += maMask.GetSizeBytes();

    return nSizeBytes;
}

BitmapChecksum BitmapEx::GetChecksum() const
{
    BitmapChecksum  nCrc = maBitmap.GetChecksum();
    SVBT32      aBT32;
    BitmapChecksumOctetArray aBCOA;

    UInt32ToSVBT32( o3tl::underlyingEnumValue(meTransparent), aBT32 );
    nCrc = vcl_get_checksum( nCrc, aBT32, 4 );

    UInt32ToSVBT32( sal_uInt32(mbAlpha), aBT32 );
    nCrc = vcl_get_checksum( nCrc, aBT32, 4 );

    if( ( TransparentType::Bitmap == meTransparent ) && !maMask.IsEmpty() )
    {
        BCToBCOA( maMask.GetChecksum(), aBCOA );
        nCrc = vcl_get_checksum( nCrc, aBCOA, BITMAP_CHECKSUM_SIZE );
    }

    return nCrc;
}

void BitmapEx::SetSizePixel(const Size& rNewSize)
{
    maBitmapSize = rNewSize;
}

bool BitmapEx::Invert()
{
    bool bRet = false;

    if (!!maBitmap)
    {
        bRet = maBitmap.Invert();

        if (bRet && (meTransparent == TransparentType::Color))
            maTransparentColor = BitmapColor(maTransparentColor).Invert().GetColor();
    }

    return bRet;
}

bool BitmapEx::Mirror( BmpMirrorFlags nMirrorFlags )
{
    bool bRet = false;

    if( !!maBitmap )
    {
        bRet = maBitmap.Mirror( nMirrorFlags );

        if( bRet && ( meTransparent == TransparentType::Bitmap ) && !!maMask )
            maMask.Mirror( nMirrorFlags );
    }

    return bRet;
}

bool BitmapEx::Scale( const double& rScaleX, const double& rScaleY, BmpScaleFlag nScaleFlag )
{
    bool bRet = false;

    if( !!maBitmap )
    {
        bRet = maBitmap.Scale( rScaleX, rScaleY, nScaleFlag );

        if( bRet && ( meTransparent == TransparentType::Bitmap ) && !!maMask )
        {
            maMask.Scale( rScaleX, rScaleY, nScaleFlag );
        }

        SetSizePixel(maBitmap.GetSizePixel());

        SAL_WARN_IF( !!maMask && maBitmap.GetSizePixel() != maMask.GetSizePixel(), "vcl",
                    "BitmapEx::Scale(): size mismatch for bitmap and alpha mask." );
    }

    return bRet;
}

bool BitmapEx::Scale( const Size& rNewSize, BmpScaleFlag nScaleFlag )
{
    bool bRet;

    if (GetSizePixel().Width() && GetSizePixel().Height()
            && (rNewSize.Width()  != GetSizePixel().Width()
                    || rNewSize.Height() != GetSizePixel().Height() ) )
    {
        bRet = Scale( static_cast<double>(rNewSize.Width()) / GetSizePixel().Width(),
                      static_cast<double>(rNewSize.Height()) / GetSizePixel().Height(),
                      nScaleFlag );
    }
    else
    {
        bRet = true;
    }

    return bRet;
}

bool BitmapEx::Rotate( long nAngle10, const Color& rFillColor )
{
    bool bRet = false;

    if( !!maBitmap )
    {
        const bool bTransRotate = ( COL_TRANSPARENT == rFillColor );

        if( bTransRotate )
        {
            if( meTransparent == TransparentType::Color )
                bRet = maBitmap.Rotate( nAngle10, maTransparentColor );
            else
            {
                bRet = maBitmap.Rotate( nAngle10, COL_BLACK );

                if( meTransparent == TransparentType::NONE )
                {
                    maMask = Bitmap(GetSizePixel(), 1);
                    maMask.Erase( COL_BLACK );
                    meTransparent = TransparentType::Bitmap;
                }

                if( bRet && !!maMask )
                    maMask.Rotate( nAngle10, COL_WHITE );
            }
        }
        else
        {
            bRet = maBitmap.Rotate( nAngle10, rFillColor );

            if( bRet && ( meTransparent == TransparentType::Bitmap ) && !!maMask )
                maMask.Rotate( nAngle10, COL_WHITE );
        }

        SetSizePixel(maBitmap.GetSizePixel());

        SAL_WARN_IF(!!maMask && maBitmap.GetSizePixel() != maMask.GetSizePixel(), "vcl",
                    "BitmapEx::Rotate(): size mismatch for bitmap and alpha mask.");
    }

    return bRet;
}

bool BitmapEx::Crop( const tools::Rectangle& rRectPixel )
{
    bool bRet = false;

    if( !!maBitmap )
    {
        bRet = maBitmap.Crop( rRectPixel );

        if( bRet && ( meTransparent == TransparentType::Bitmap ) && !!maMask )
            maMask.Crop( rRectPixel );

        SetSizePixel(maBitmap.GetSizePixel());

        SAL_WARN_IF(!!maMask && maBitmap.GetSizePixel() != maMask.GetSizePixel(), "vcl",
                    "BitmapEx::Crop(): size mismatch for bitmap and alpha mask.");
    }

    return bRet;
}

bool BitmapEx::Convert( BmpConversion eConversion )
{
    return !!maBitmap && maBitmap.Convert( eConversion );
}

void BitmapEx::Expand( sal_uLong nDX, sal_uLong nDY, bool bExpandTransparent )
{
    bool bRet = false;

    if( !!maBitmap )
    {
        bRet = maBitmap.Expand( nDX, nDY );

        if( bRet && ( meTransparent == TransparentType::Bitmap ) && !!maMask )
        {
            Color aColor( bExpandTransparent ? COL_WHITE : COL_BLACK );
            maMask.Expand( nDX, nDY, &aColor );
        }

        SetSizePixel(maBitmap.GetSizePixel());

        SAL_WARN_IF(!!maMask && maBitmap.GetSizePixel() != maMask.GetSizePixel(), "vcl",
                    "BitmapEx::Expand(): size mismatch for bitmap and alpha mask.");
    }
}

bool BitmapEx::CopyPixel( const tools::Rectangle& rRectDst, const tools::Rectangle& rRectSrc,
                          const BitmapEx* pBmpExSrc )
{
    bool bRet = false;

    if( !pBmpExSrc || pBmpExSrc->IsEmpty() )
    {
        if( !maBitmap.IsEmpty() )
        {
            bRet = maBitmap.CopyPixel( rRectDst, rRectSrc );

            if( bRet && ( meTransparent == TransparentType::Bitmap ) && !!maMask )
                maMask.CopyPixel( rRectDst, rRectSrc );
        }
    }
    else
    {
        if( !maBitmap.IsEmpty() )
        {
            bRet = maBitmap.CopyPixel( rRectDst, rRectSrc, &pBmpExSrc->maBitmap );

            if( bRet )
            {
                if( pBmpExSrc->IsAlpha() )
                {
                    if( IsAlpha() )
                        // cast to use the optimized AlphaMask::CopyPixel
                        maMask.CopyPixel_AlphaOptimized( rRectDst, rRectSrc, &pBmpExSrc->maMask );
                    else if( IsTransparent() )
                    {
                        std::unique_ptr<AlphaMask> pAlpha(new AlphaMask( maMask ));

                        maMask = pAlpha->ImplGetBitmap();
                        pAlpha.reset();
                        mbAlpha = true;
                        maMask.CopyPixel( rRectDst, rRectSrc, &pBmpExSrc->maMask );
                    }
                    else
                    {
                        sal_uInt8 cBlack = 0;
                        std::unique_ptr<AlphaMask> pAlpha(new AlphaMask(GetSizePixel(), &cBlack));

                        maMask = pAlpha->ImplGetBitmap();
                        pAlpha.reset();
                        meTransparent = TransparentType::Bitmap;
                        mbAlpha = true;
                        maMask.CopyPixel( rRectDst, rRectSrc, &pBmpExSrc->maMask );
                    }
                }
                else if( pBmpExSrc->IsTransparent() )
                {
                    if (IsAlpha())
                    {
                        AlphaMask aAlpha( pBmpExSrc->maMask );
                        maMask.CopyPixel( rRectDst, rRectSrc, &aAlpha.ImplGetBitmap() );
                    }
                    else if (IsTransparent())
                    {
                        maMask.CopyPixel( rRectDst, rRectSrc, &pBmpExSrc->maMask );
                    }
                    else
                    {
                        maMask = Bitmap(GetSizePixel(), 1);
                        maMask.Erase(COL_BLACK);
                        meTransparent = TransparentType::Bitmap;
                        maMask.CopyPixel( rRectDst, rRectSrc, &pBmpExSrc->maMask );
                    }
                }
                else if (IsAlpha())
                {
                    sal_uInt8 cBlack = 0;
                    const AlphaMask aAlphaSrc(pBmpExSrc->GetSizePixel(), &cBlack);

                    maMask.CopyPixel( rRectDst, rRectSrc, &aAlphaSrc.ImplGetBitmap() );
                }
                else if (IsTransparent())
                {
                    Bitmap aMaskSrc(pBmpExSrc->GetSizePixel(), 1);

                    aMaskSrc.Erase( COL_BLACK );
                    maMask.CopyPixel( rRectDst, rRectSrc, &aMaskSrc );
                }
            }
        }
    }

    return bRet;
}

bool BitmapEx::Erase( const Color& rFillColor )
{
    bool bRet = false;

    if( !!maBitmap )
    {
        bRet = maBitmap.Erase( rFillColor );

        if( bRet && ( meTransparent == TransparentType::Bitmap ) && !!maMask )
        {
            // Respect transparency on fill color
            if( rFillColor.GetTransparency() )
            {
                const Color aFill( rFillColor.GetTransparency(), rFillColor.GetTransparency(), rFillColor.GetTransparency() );
                maMask.Erase( aFill );
            }
            else
            {
                const Color aBlack( COL_BLACK );
                maMask.Erase( aBlack );
            }
        }
    }

    return bRet;
}

void BitmapEx::Replace( const Color& rSearchColor, const Color& rReplaceColor )
{
    if (!!maBitmap)
        maBitmap.Replace( rSearchColor, rReplaceColor );
}

void BitmapEx::Replace( const Color* pSearchColors, const Color* pReplaceColors, sal_uLong nColorCount )
{
    if (!!maBitmap)
        maBitmap.Replace( pSearchColors, pReplaceColors, nColorCount, /*pTols*/nullptr );
}

bool BitmapEx::Adjust( short nLuminancePercent, short nContrastPercent,
                       short nChannelRPercent, short nChannelGPercent, short nChannelBPercent,
                       double fGamma, bool bInvert, bool msoBrightness )
{
    return !!maBitmap && maBitmap.Adjust( nLuminancePercent, nContrastPercent,
                                        nChannelRPercent, nChannelGPercent, nChannelBPercent,
                                        fGamma, bInvert, msoBrightness );
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
    double imgOldHeight = aRet.GetSizePixel().Height();

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
    Point aPointPixel( static_cast<long>(imgposX), static_cast<long>(imgposY) );
    aVirDevice->DrawBitmapEx( aPointPixel, aRet );
    aRet = aVirDevice->GetBitmapEx( aEmptyPoint, aStdSize );

    return aRet;
}

sal_uInt8 BitmapEx::GetTransparency(sal_Int32 nX, sal_Int32 nY) const
{
    sal_uInt8 nTransparency(0xff);

    if(!maBitmap.IsEmpty())
    {
        if (nX >= 0 && nX < GetSizePixel().Width() && nY >= 0 && nY < GetSizePixel().Height())
        {
            switch(meTransparent)
            {
                case TransparentType::NONE:
                {
                    // Not transparent, ergo all covered
                    nTransparency = 0x00;
                    break;
                }
                case TransparentType::Color:
                {
                    Bitmap aTestBitmap(maBitmap);
                    Bitmap::ScopedReadAccess pRead(aTestBitmap);

                    if(pRead)
                    {
                        const BitmapColor aBmpColor = pRead->GetColor(nY, nX);
                        const Color aColor = aBmpColor.GetColor();

                        // If color is not equal to TransparentColor, we are not transparent
                        if (aColor != maTransparentColor)
                            nTransparency = 0x00;

                    }
                    break;
                }
                case TransparentType::Bitmap:
                {
                    if(!maMask.IsEmpty())
                    {
                        Bitmap aTestBitmap(maMask);
                        Bitmap::ScopedReadAccess pRead(aTestBitmap);

                        if(pRead)
                        {
                            const BitmapColor aBitmapColor(pRead->GetPixel(nY, nX));

                            if(mbAlpha)
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


Color BitmapEx::GetPixelColor(sal_Int32 nX, sal_Int32 nY) const
{
    Bitmap::ScopedReadAccess pReadAccess( const_cast<Bitmap&>(maBitmap) );
    assert( pReadAccess );

    Color aColor = pReadAccess->GetColor( nY, nX ).GetColor();

    if( IsAlpha() )
    {
        Bitmap::ScopedReadAccess pAlphaReadAccess( const_cast<Bitmap&>(maMask).AcquireReadAccess(), const_cast<Bitmap&>(maMask) );
        aColor.SetTransparency( pAlphaReadAccess->GetPixel( nY, nX ).GetIndex() );
    }
    else
        aColor.SetTransparency(255);
    return aColor;
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

    std::shared_ptr<SalBitmap> pSalBmp;
    std::shared_ptr<SalBitmap> pSalMask;

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
            *this = BitmapEx(Bitmap(pSalBmp));
            return true;
        }
    }

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
        BitmapScopedWriteAccess xWrite(aDestination);

        if(xWrite)
        {
            Bitmap::ScopedReadAccess xRead(const_cast< Bitmap& >(rSource));

            if (xRead)
            {
                const Size aDestinationSizePixel(aDestination.GetSizePixel());
                const BitmapColor aOutside(BitmapColor(0xff, 0xff, 0xff));

                for(long y(0); y < aDestinationSizePixel.getHeight(); y++)
                {
                    Scanline pScanline = xWrite->GetScanline( y );
                    for(long x(0); x < aDestinationSizePixel.getWidth(); x++)
                    {
                        const basegfx::B2DPoint aSourceCoor(rTransform * basegfx::B2DPoint(x, y));

                        if(bSmooth)
                        {
                            xWrite->SetPixelOnData(
                                pScanline,
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
                            xWrite->SetPixelOnData(
                                pScanline,
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
    const Bitmap aDestination(impTransformBitmap(GetBitmapRef(), aDestinationSize, rTransformation, bSmooth));

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
    double fMaximumArea) const
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
        basegfx::utils::createScaleTranslateB2DHomMatrix(
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
        basegfx::utils::createScaleB2DHomMatrix(
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
    aRetval = TransformBitmapEx(fWidth, fHeight, aTransform, /*bSmooth*/true);

    return aRetval;
}

BitmapEx BitmapEx::ModifyBitmapEx(const basegfx::BColorModifierStack& rBColorModifierStack) const
{
    Bitmap aChangedBitmap(GetBitmapRef());
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
            BitmapScopedWriteAccess xContent(aChangedBitmap);

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
                    for(sal_uInt32 y(0); y < static_cast<sal_uInt32>(xContent->Height()); y++)
                    {
                        Scanline pScan = xContent->GetScanline(y);

                        for(sal_uInt32 x(0); x < static_cast<sal_uInt32>(xContent->Width()); x++)
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
                    for(sal_uInt32 y(0); y < static_cast<sal_uInt32>(xContent->Height()); y++)
                    {
                        Scanline pScan = xContent->GetScanline(y);

                        for(sal_uInt32 x(0); x < static_cast<sal_uInt32>(xContent->Width()); x++)
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
                    for(sal_uInt32 y(0); y < static_cast<sal_uInt32>(xContent->Height()); y++)
                    {
                        Scanline pScanline = xContent->GetScanline( y );
                        for(sal_uInt32 x(0); x < static_cast<sal_uInt32>(xContent->Width()); x++)
                        {
                            const BitmapColor aBMCol(xContent->GetColor(y, x));
                            const basegfx::BColor aBSource(
                                static_cast<double>(aBMCol.GetRed()) * fConvertColor,
                                static_cast<double>(aBMCol.GetGreen()) * fConvertColor,
                                static_cast<double>(aBMCol.GetBlue()) * fConvertColor);
                            const basegfx::BColor aBDest(rModifier->getModifiedColor(aBSource));

                            xContent->SetPixelOnData(pScanline, x, BitmapColor(Color(aBDest)));
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

        BitmapScopedWriteAccess pContent(aContent);
        AlphaScopedWriteAccess pAlpha(aAlpha);

        if(pContent && pAlpha)
        {
            long x(0);
            long y(0);
            Scanline pScanContent = pContent->GetScanline( 0 );
            Scanline pScanAlpha = pContent->GetScanline( 0 );

            // x == 0, y == 0, top-left corner
            pContent->SetPixelOnData(pScanContent, 0, aColorTopLeft);
            pAlpha->SetPixelOnData(pScanAlpha, 0, BitmapColor(nAlpha));

            // y == 0, top line left to right
            for(x = 1; x < nW - 1; x++)
            {
                Color aMix(aColorTopLeft);

                aMix.Merge(aColorTopRight, 255 - sal_uInt8((x * 255) / nW));
                pContent->SetPixelOnData(pScanContent, x, aMix);
                pAlpha->SetPixelOnData(pScanAlpha, x, BitmapColor(nAlpha));
            }

            // x == nW - 1, y == 0, top-right corner
            // #i123690# Caution! When nW is 1, x == nW is possible (!)
            if(x < nW)
            {
                pContent->SetPixelOnData(pScanContent, x, aColorTopRight);
                pAlpha->SetPixelOnData(pScanAlpha, x, BitmapColor(nAlpha));
            }

            // x == 0 and nW - 1, left and right line top-down
            for(y = 1; y < nH - 1; y++)
            {
                pScanContent = pContent->GetScanline( y );
                pScanAlpha = pContent->GetScanline( y );
                Color aMixA(aColorTopLeft);

                aMixA.Merge(aColorBottomLeft, 255 - sal_uInt8((y * 255) / nH));
                pContent->SetPixelOnData(pScanContent, 0, aMixA);
                pAlpha->SetPixelOnData(pScanAlpha, 0, BitmapColor(nAlpha));

                // #i123690# Caution! When nW is 1, x == nW is possible (!)
                if(x < nW)
                {
                    Color aMixB(aColorTopRight);

                    aMixB.Merge(aColorBottomRight, 255 - sal_uInt8((y * 255) / nH));
                    pContent->SetPixelOnData(pScanContent, x, aMixB);
                    pAlpha->SetPixelOnData(pScanAlpha, x, BitmapColor(nAlpha));
                }
            }

            // #i123690# Caution! When nH is 1, y == nH is possible (!)
            if(y < nH)
            {
                // x == 0, y == nH - 1, bottom-left corner
                pContent->SetPixelOnData(pScanContent, 0, aColorBottomLeft);
                pAlpha->SetPixelOnData(pScanAlpha, 0, BitmapColor(nAlpha));

                // y == nH - 1, bottom line left to right
                for(x = 1; x < nW - 1; x++)
                {
                    Color aMix(aColorBottomLeft);

                    aMix.Merge(aColorBottomRight, 255 - sal_uInt8(((x - 0)* 255) / nW));
                    pContent->SetPixelOnData(pScanContent, x, aMix);
                    pAlpha->SetPixelOnData(pScanAlpha, x, BitmapColor(nAlpha));
                }

                // x == nW - 1, y == nH - 1, bottom-right corner
                // #i123690# Caution! When nW is 1, x == nW is possible (!)
                if(x < nW)
                {
                    pContent->SetPixelOnData(pScanContent, x, aColorBottomRight);
                    pAlpha->SetPixelOnData(pScanAlpha, x, BitmapColor(nAlpha));
                }
            }

            pContent.reset();
            pAlpha.reset();

            pBlendFrameCache->m_aLastResult = BitmapEx(aContent, aAlpha);
        }
    }

    return pBlendFrameCache->m_aLastResult;
}

void BitmapEx::Replace(const Color& rSearchColor,
                           const Color& rReplaceColor,
                           sal_uInt8 nTolerance)
{
    maBitmap.Replace(rSearchColor, rReplaceColor, nTolerance);
}

void BitmapEx::setAlphaFrom( sal_uInt8 cIndexFrom, sal_Int8 nAlphaTo )
{
    AlphaMask aAlphaMask(GetAlpha());
    BitmapScopedWriteAccess pWriteAccess(aAlphaMask);
    Bitmap::ScopedReadAccess pReadAccess(maBitmap);
    assert( pReadAccess.get() && pWriteAccess.get() );
    if ( pReadAccess.get() && pWriteAccess.get() )
    {
        for ( long nY = 0; nY < pReadAccess->Height(); nY++ )
        {
            Scanline pScanline = pWriteAccess->GetScanline( nY );
            Scanline pScanlineRead = pReadAccess->GetScanline( nY );
            for ( long nX = 0; nX < pReadAccess->Width(); nX++ )
            {
                const sal_uInt8 cIndex = pReadAccess->GetPixelFromData( pScanlineRead, nX ).GetBlueOrIndex();
                if ( cIndex == cIndexFrom )
                    pWriteAccess->SetPixelOnData( pScanline, nX, BitmapColor(nAlphaTo) );
            }
        }
    }
}

void BitmapEx::AdjustTransparency(sal_uInt8 cTrans)
{
    AlphaMask   aAlpha;

    if (!IsTransparent())
    {
        aAlpha = AlphaMask(GetSizePixel(), &cTrans);
    }
    else if( !IsAlpha() )
    {
        aAlpha = GetMask();
        aAlpha.Replace( 0, cTrans );
    }
    else
    {
        aAlpha = GetAlpha();
        BitmapScopedWriteAccess pA(aAlpha);
        assert(pA);

        if( !pA )
            return;

        sal_uLong       nTrans = cTrans, nNewTrans;
        const long  nWidth = pA->Width(), nHeight = pA->Height();

        if( pA->GetScanlineFormat() == ScanlineFormat::N8BitPal )
        {
            for( long nY = 0; nY < nHeight; nY++ )
            {
                Scanline pAScan = pA->GetScanline( nY );

                for( long nX = 0; nX < nWidth; nX++ )
                {
                    nNewTrans = nTrans + *pAScan;
                    *pAScan++ = static_cast<sal_uInt8>( ( nNewTrans & 0xffffff00 ) ? 255 : nNewTrans );
                }
            }
        }
        else
        {
            BitmapColor aAlphaValue( 0 );

            for( long nY = 0; nY < nHeight; nY++ )
            {
                Scanline pScanline = pA->GetScanline( nY );
                for( long nX = 0; nX < nWidth; nX++ )
                {
                    nNewTrans = nTrans + pA->GetIndexFromData( pScanline, nX );
                    aAlphaValue.SetIndex( static_cast<sal_uInt8>( ( nNewTrans & 0xffffff00 ) ? 255 : nNewTrans ) );
                    pA->SetPixelOnData( pScanline, nX, aAlphaValue );
                }
            }
        }
    }
    *this = BitmapEx( GetBitmapRef(), aAlpha );
}

// AS: Because JPEGs require the alpha channel provided separately (JPEG does not
//  natively support alpha channel, but SWF lets you provide it separately), we
//  extract the alpha channel into a separate array here.
void BitmapEx::GetSplitData( std::vector<sal_uInt8>& rvColorData, std::vector<sal_uInt8>& rvAlphaData ) const
{
    if( IsEmpty() )
        return;

    Bitmap::ScopedReadAccess pRAcc(const_cast<Bitmap&>(maBitmap));

    assert( pRAcc );

    AlphaMask   aAlpha;
    sal_uInt32 nWidth = pRAcc->Width();
    sal_uInt32 nHeight = pRAcc->Height();
    rvColorData.resize(nWidth*nHeight*4);
    rvAlphaData.resize(nWidth*nHeight);
    sal_uInt8* p = rvColorData.data(), *pAlpha = rvAlphaData.data();


    if (IsAlpha())
    {
        aAlpha = GetAlpha();
    }
    else if (IsTransparent())
    {
        aAlpha = GetMask();
    }
    else
    {
        sal_uInt8 cAlphaVal = 0;
        aAlpha = AlphaMask(maBitmap.GetSizePixel(), &cAlphaVal);
    }

    AlphaMask::ScopedReadAccess pAAcc(aAlpha);

    assert( pAAcc );

    for( sal_uInt32 nY = 0; nY < nHeight; nY++ )
    {
        Scanline pScanlineAA = pAAcc->GetScanline( nY );
        for( sal_uInt32 nX = 0; nX < nWidth; nX++ )
        {
            const sal_uInt8     nAlpha = pAAcc->GetIndexFromData( pScanlineAA, nX );
            const BitmapColor   aPixelColor( pRAcc->GetColor( nY, nX ) );

            if( nAlpha == 0xff )
            {
                *p++ = 0;
                *p++ = 0;
                *p++ = 0;
                *p++ = 0;
            }
            else
            {
                *p++ = 0xff-nAlpha;
                *p++ = aPixelColor.GetRed();
                *p++ = aPixelColor.GetGreen();
                *p++ = aPixelColor.GetBlue();
            }
            *pAlpha++ = 0xff - nAlpha;
        }
    }
}

void BitmapEx::CombineMaskOr(Color maskColor, sal_uInt8 nTol)
{
    Bitmap aNewMask = maBitmap.CreateMask( maskColor, nTol );
    if ( IsTransparent() )
         aNewMask.CombineSimple( maMask, BmpCombine::Or );
    maMask = aNewMask;
    meTransparent = TransparentType::Bitmap;
}

/**
 * Retrieves the color model data we need for the XImageConsumer stuff.
 */
void  BitmapEx::GetColorModel(css::uno::Sequence< sal_Int32 >& rRGBPalette,
        sal_uInt32& rnRedMask, sal_uInt32& rnGreenMask, sal_uInt32& rnBlueMask, sal_uInt32& rnAlphaMask, sal_uInt32& rnTransparencyIndex,
        sal_uInt32& rnWidth, sal_uInt32& rnHeight, sal_uInt8& rnBitCount)
{
    Bitmap::ScopedReadAccess pReadAccess( maBitmap );
    assert( pReadAccess );

    if( pReadAccess->HasPalette() )
    {
        sal_uInt16 nPalCount = pReadAccess->GetPaletteEntryCount();

        if( nPalCount )
        {
            rRGBPalette = css::uno::Sequence< sal_Int32 >( nPalCount + 1 );

            sal_Int32* pTmp = rRGBPalette.getArray();

            for( sal_uInt32 i = 0; i < nPalCount; i++, pTmp++ )
            {
                const BitmapColor& rCol = pReadAccess->GetPaletteColor( static_cast<sal_uInt16>(i) );

                *pTmp = static_cast<sal_Int32>(rCol.GetRed()) << sal_Int32(24);
                *pTmp |= static_cast<sal_Int32>(rCol.GetGreen()) << sal_Int32(16);
                *pTmp |= static_cast<sal_Int32>(rCol.GetBlue()) << sal_Int32(8);
                *pTmp |= sal_Int32(0x000000ffL);
            }

            if( IsTransparent() )
            {
                // append transparent entry
                *pTmp = sal_Int32(0xffffff00L);
                rnTransparencyIndex = nPalCount;
                nPalCount++;
            }
            else
                rnTransparencyIndex = 0;
        }
    }
    else
    {
        rnRedMask = 0xff000000UL;
        rnGreenMask = 0x00ff0000UL;
        rnBlueMask = 0x0000ff00UL;
        rnAlphaMask = 0x000000ffUL;
        rnTransparencyIndex = 0;
    }

    rnWidth = pReadAccess->Width();
    rnHeight = pReadAccess->Height();
    rnBitCount = pReadAccess->GetBitCount();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
