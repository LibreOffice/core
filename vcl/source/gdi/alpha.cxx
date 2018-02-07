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

#include <vcl/bitmapaccess.hxx>
#include <tools/color.hxx>
#include <vcl/alpha.hxx>

AlphaMask::AlphaMask()
{
}

AlphaMask::AlphaMask( const Bitmap& rBitmap ) :
    Bitmap( rBitmap )
{
    if( !!rBitmap )
        Convert( BmpConversion::N8BitGreys );
}

AlphaMask::AlphaMask( const AlphaMask& rAlphaMask ) :
    Bitmap( rAlphaMask )
{
}

AlphaMask::AlphaMask( AlphaMask&& rAlphaMask ) :
    Bitmap( std::move(rAlphaMask) )
{
}

AlphaMask::AlphaMask( const Size& rSizePixel, const sal_uInt8* pEraseTransparency ) :
    Bitmap( rSizePixel, 8, &Bitmap::GetGreyPalette( 256 ) )
{
    if( pEraseTransparency )
        Bitmap::Erase( Color( *pEraseTransparency, *pEraseTransparency, *pEraseTransparency ) );
}

AlphaMask::~AlphaMask()
{
}

AlphaMask& AlphaMask::operator=( const Bitmap& rBitmap )
{
    *static_cast<Bitmap*>(this) = rBitmap;

    if( !!rBitmap )
        Convert( BmpConversion::N8BitGreys );

    return *this;
}

const Bitmap& AlphaMask::ImplGetBitmap() const
{
    return *this;
}

void AlphaMask::ImplSetBitmap( const Bitmap& rBitmap )
{
    SAL_WARN_IF( 8 != rBitmap.GetBitCount(), "vcl.gdi", "Bitmap should be 8bpp, not " << rBitmap.GetBitCount() << "bpp" );
    SAL_WARN_IF( !rBitmap.HasGreyPalette(), "vcl.gdi", "Bitmap isn't greyscale" );
    *static_cast<Bitmap*>(this) = rBitmap;
}

Bitmap const & AlphaMask::GetBitmap() const
{
    return ImplGetBitmap();
}

void AlphaMask::Erase( sal_uInt8 cTransparency )
{
    Bitmap::Erase( Color( cTransparency, cTransparency, cTransparency ) );
}

bool AlphaMask::Replace( const Bitmap& rMask, sal_uInt8 cReplaceTransparency )
{
    Bitmap::ScopedReadAccess pMaskAcc( const_cast<Bitmap&>(rMask) );
    AlphaMask::ScopedWriteAccess pAcc(*this);

    if( pMaskAcc && pAcc )
    {
        const BitmapColor   aReplace( cReplaceTransparency );
        const long          nWidth = std::min( pMaskAcc->Width(), pAcc->Width() );
        const long          nHeight = std::min( pMaskAcc->Height(), pAcc->Height() );
        const BitmapColor   aMaskWhite( pMaskAcc->GetBestMatchingColor( Color( COL_WHITE ) ) );

        for( long nY = 0; nY < nHeight; nY++ )
        {
            Scanline pScanline = pAcc->GetScanline(nY);
            Scanline pScanlineMask = pMaskAcc->GetScanline(nY);
            for( long nX = 0; nX < nWidth; nX++ )
                if( pMaskAcc->GetPixelFromData( pScanlineMask, nX ) == aMaskWhite )
                    pAcc->SetPixelOnData( pScanline, nX, aReplace );
        }
    }
    return false;
}

void AlphaMask::Replace( sal_uInt8 cSearchTransparency, sal_uInt8 cReplaceTransparency )
{
    AlphaMask::ScopedWriteAccess pAcc(*this);

    if( pAcc && pAcc->GetBitCount() == 8 )
    {
        const long nWidth = pAcc->Width(), nHeight = pAcc->Height();

        if( pAcc->GetScanlineFormat() == ScanlineFormat::N8BitPal )
        {
            for( long nY = 0; nY < nHeight; nY++ )
            {
                Scanline pScan = pAcc->GetScanline( nY );

                for( long nX = 0; nX < nWidth; nX++, pScan++ )
                {
                    if( *pScan == cSearchTransparency )
                        *pScan = cReplaceTransparency;
                }
            }
        }
        else
        {
            BitmapColor aReplace( cReplaceTransparency );

            for( long nY = 0; nY < nHeight; nY++ )
            {
                Scanline pScanline = pAcc->GetScanline(nY);
                for( long nX = 0; nX < nWidth; nX++ )
                {
                    if( pAcc->GetIndexFromData( pScanline, nX ) == cSearchTransparency )
                        pAcc->SetPixelOnData( pScanline, nX, aReplace );
                }
            }
        }
    }
}

void AlphaMask::ReleaseAccess( BitmapReadAccess* pAccess )
{
    if( pAccess )
    {
        Bitmap::ReleaseAccess( pAccess );
        Convert( BmpConversion::N8BitGreys );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
