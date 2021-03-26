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

#include <tools/color.hxx>
#include <vcl/alpha.hxx>

#include <bitmap/BitmapWriteAccess.hxx>
#include <salinst.hxx>
#include <svdata.hxx>
#include <salbmp.hxx>
#include <sal/log.hxx>

AlphaMask::AlphaMask() = default;

AlphaMask::AlphaMask( const Bitmap& rBitmap ) :
    Bitmap( rBitmap )
{
    if( !rBitmap.IsEmpty() )
        Convert( BmpConversion::N8BitNoConversion );
}

AlphaMask::AlphaMask( const AlphaMask& ) = default;

AlphaMask::AlphaMask( AlphaMask&& ) = default;

AlphaMask::AlphaMask( const Size& rSizePixel, const sal_uInt8* pEraseTransparency )
    : Bitmap(rSizePixel, vcl::PixelFormat::N8_BPP, &Bitmap::GetGreyPalette(256))
{
    if( pEraseTransparency )
        Bitmap::Erase( Color( *pEraseTransparency, *pEraseTransparency, *pEraseTransparency ) );
}

AlphaMask::~AlphaMask() = default;

AlphaMask& AlphaMask::operator=( const Bitmap& rBitmap )
{
    *static_cast<Bitmap*>(this) = rBitmap;

    if( !rBitmap.IsEmpty() )
        Convert( BmpConversion::N8BitNoConversion );

    return *this;
}

const Bitmap& AlphaMask::ImplGetBitmap() const
{
    return *this;
}

void AlphaMask::ImplSetBitmap( const Bitmap& rBitmap )
{
    SAL_WARN_IF( 8 != rBitmap.GetBitCount(), "vcl.gdi", "Bitmap should be 8bpp, not " << rBitmap.GetBitCount() << "bpp" );
    SAL_WARN_IF( !rBitmap.HasGreyPalette8Bit(), "vcl.gdi", "Bitmap isn't greyscale" );
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

void AlphaMask::Replace( sal_uInt8 cSearchTransparency, sal_uInt8 cReplaceTransparency )
{
    AlphaScopedWriteAccess pAcc(*this);

    if( !(pAcc && pAcc->GetBitCount() == 8) )
        return;

    const tools::Long nWidth = pAcc->Width(), nHeight = pAcc->Height();

    if( pAcc->GetScanlineFormat() == ScanlineFormat::N8BitPal )
    {
        for( tools::Long nY = 0; nY < nHeight; nY++ )
        {
            Scanline pScan = pAcc->GetScanline( nY );

            for( tools::Long nX = 0; nX < nWidth; nX++, pScan++ )
            {
                if( *pScan == cSearchTransparency )
                    *pScan = cReplaceTransparency;
            }
        }
    }
    else
    {
        BitmapColor aReplace( cReplaceTransparency );

        for( tools::Long nY = 0; nY < nHeight; nY++ )
        {
            Scanline pScanline = pAcc->GetScanline(nY);
            for( tools::Long nX = 0; nX < nWidth; nX++ )
            {
                if( pAcc->GetIndexFromData( pScanline, nX ) == cSearchTransparency )
                    pAcc->SetPixelOnData( pScanline, nX, aReplace );
            }
        }
    }
}

void AlphaMask::BlendWith(const Bitmap& rOther)
{
    std::shared_ptr<SalBitmap> xImpBmp(ImplGetSVData()->mpDefInst->CreateSalBitmap());
    if (xImpBmp->Create(*ImplGetSalBitmap()) && xImpBmp->AlphaBlendWith(*rOther.ImplGetSalBitmap()))
    {
        ImplSetSalBitmap(xImpBmp);
        return;
    }
    AlphaMask aOther(rOther); // to 8 bits
    Bitmap::ScopedReadAccess pOtherAcc(aOther);
    AlphaScopedWriteAccess pAcc(*this);
    if (!(pOtherAcc && pAcc && pOtherAcc->GetBitCount() == 8 && pAcc->GetBitCount() == 8))
        return;

    const tools::Long nHeight = std::min(pOtherAcc->Height(), pAcc->Height());
    const tools::Long nWidth = std::min(pOtherAcc->Width(), pAcc->Width());
    for (tools::Long y = 0; y < nHeight; ++y)
    {
        Scanline scanline = pAcc->GetScanline( y );
        ConstScanline otherScanline = pOtherAcc->GetScanline( y );
        for (tools::Long x = 0; x < nWidth; ++x)
        {
            // Use sal_uInt16 for following multiplication
            const sal_uInt16 nGrey1 = *scanline;
            const sal_uInt16 nGrey2 = *otherScanline;
            *scanline = static_cast<sal_uInt8>(nGrey1 + nGrey2 - nGrey1 * nGrey2 / 255);
            ++scanline;
            ++otherScanline;
        }
    }
}

void AlphaMask::ReleaseAccess( BitmapReadAccess* pAccess )
{
    if( pAccess )
    {
        Bitmap::ReleaseAccess( pAccess );
        Convert( BmpConversion::N8BitNoConversion );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
