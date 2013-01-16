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

#ifndef _SV_ALPHA_HXX
#define _SV_ALPHA_HXX

#include <vcl/dllapi.h>
#include <vcl/bitmap.hxx>

// -------------
// - AlphaMask -
// -------------

class ImageList;
class BitmapEx;

class VCL_DLLPUBLIC AlphaMask : private Bitmap
{
    friend class BitmapEx;
    friend class OutputDevice;
    friend VCL_DLLPUBLIC SvStream& operator<<( SvStream&, const ImageList& );

private:

    SAL_DLLPRIVATE const Bitmap&    ImplGetBitmap() const;
    SAL_DLLPRIVATE void             ImplSetBitmap( const Bitmap& rBitmap );

public:

    AlphaMask();
    AlphaMask( const Bitmap& rBitmap );
    AlphaMask( const AlphaMask& rAlphaMask );
    AlphaMask( const Size& rSizePixel, sal_uInt8* pEraseTransparency = NULL );
    virtual ~AlphaMask();

    AlphaMask&  operator=( const Bitmap& rBitmap );
    AlphaMask&  operator=( const AlphaMask& rAlphaMask )
    {
        return (AlphaMask&) Bitmap::operator=( rAlphaMask );
    }

    sal_Bool operator!() const
    {
        return Bitmap::operator!();
    }

    sal_Bool operator==( const AlphaMask& rAlphaMask ) const
    {
        return Bitmap::operator==( rAlphaMask );
    }

    sal_Bool operator!=( const AlphaMask& rAlphaMask ) const
    {
        return Bitmap::operator!=( rAlphaMask );
    }

    const MapMode&  GetPrefMapMode() const
    {
        return Bitmap::GetPrefMapMode();
    }

    void    SetPrefMapMode( const MapMode& rMapMode )
    {
        Bitmap::SetPrefMapMode( rMapMode );
    }

    const Size& GetPrefSize() const
    {
        return Bitmap::GetPrefSize();
    }

    void    SetPrefSize( const Size& rSize )
    {
        Bitmap::SetPrefSize( rSize );
    }

    Size    GetSizePixel() const
    {
        return Bitmap::GetSizePixel();
    }

    sal_uLong   GetSizeBytes() const
    {
        return Bitmap::GetSizeBytes();
    }
    sal_uLong   GetChecksum() const
    {
        return Bitmap::GetChecksum();
    }

    Bitmap  GetBitmap() const;

    sal_Bool    CopyPixel( const Rectangle& rRectDst, const Rectangle& rRectSrc,
                           const AlphaMask* pAlphaSrc = NULL);

    sal_Bool    Erase( sal_uInt8 cTransparency );
    sal_Bool    Replace( const Bitmap& rMask, sal_uInt8 rReplaceTransparency );
    sal_Bool    Replace( sal_uInt8 cSearchTransparency, sal_uInt8 cReplaceTransparency,
                         sal_uLong nTol = 0UL );

    BitmapReadAccess*   AcquireReadAccess()
    {
        return Bitmap::AcquireReadAccess();
    }

    BitmapWriteAccess*  AcquireWriteAccess()
    {
        return Bitmap::AcquireWriteAccess();
    }

    void    ReleaseAccess( BitmapReadAccess* pAccess );

    typedef vcl::ScopedBitmapAccess< BitmapReadAccess, AlphaMask, &AlphaMask::AcquireReadAccess >
        ScopedReadAccess;
    typedef vcl::ScopedBitmapAccess< BitmapWriteAccess, AlphaMask, &AlphaMask::AcquireWriteAccess >
        ScopedWriteAccess;

    sal_Bool    Read( SvStream& rIStm, sal_Bool bFileHeader = sal_True )
    {
        return Bitmap::Read( rIStm, bFileHeader );
    }
    sal_Bool    Write( SvStream& rOStm, sal_Bool bCompressed = sal_True, sal_Bool bFileHeader = sal_True ) const
    {
        return Bitmap::Write( rOStm, bCompressed, bFileHeader );
    }

    friend VCL_DLLPUBLIC SvStream& operator<<( SvStream& rOStm, const BitmapEx& rBitmapEx );
    friend VCL_DLLPUBLIC SvStream& operator>>( SvStream& rIStm, BitmapEx& rBitmapEx );
};

#endif // _SV_ALPHA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
