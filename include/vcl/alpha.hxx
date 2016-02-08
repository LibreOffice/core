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

#ifndef INCLUDED_VCL_ALPHA_HXX
#define INCLUDED_VCL_ALPHA_HXX

#include <vcl/dllapi.h>
#include <vcl/bitmap.hxx>


class ImageList;
class BitmapEx;

class VCL_DLLPUBLIC AlphaMask : private Bitmap
{
public:

                AlphaMask();
                AlphaMask( const Bitmap& rBitmap );
                AlphaMask( const AlphaMask& rAlphaMask );
                AlphaMask( const Size& rSizePixel, sal_uInt8* pEraseTransparency = nullptr );
    virtual     ~AlphaMask();

    AlphaMask&  operator=( const Bitmap& rBitmap );
    AlphaMask&  operator=( const AlphaMask& rAlphaMask ) { return static_cast<AlphaMask&>( Bitmap::operator=( rAlphaMask ) ); }
    bool        operator!() const { return Bitmap::operator!(); }
    bool        operator==( const AlphaMask& rAlphaMask ) const { return Bitmap::operator==( rAlphaMask ); }
    bool        operator!=( const AlphaMask& rAlphaMask ) const { return Bitmap::operator!=( rAlphaMask ); }

    void        SetPrefMapMode( const MapMode& rMapMode ) { Bitmap::SetPrefMapMode( rMapMode ); }

    void        SetPrefSize( const Size& rSize ) { Bitmap::SetPrefSize( rSize ); }

    Size        GetSizePixel() const { return Bitmap::GetSizePixel(); }

    BitmapChecksum   GetChecksum() const { return Bitmap::GetChecksum(); }

    Bitmap      GetBitmap() const;

    bool        Erase( sal_uInt8 cTransparency );
    bool        Replace( const Bitmap& rMask, sal_uInt8 rReplaceTransparency );
    bool        Replace( sal_uInt8 cSearchTransparency, sal_uInt8 cReplaceTransparency, sal_uLong nTol = 0UL );

    BitmapReadAccess*  AcquireReadAccess() { return Bitmap::AcquireReadAccess(); }
    BitmapWriteAccess* AcquireWriteAccess() { return Bitmap::AcquireWriteAccess(); }

    void        ReleaseAccess( BitmapReadAccess* pAccess );

    typedef vcl::ScopedBitmapAccess< BitmapReadAccess, AlphaMask, &AlphaMask::AcquireReadAccess >
        ScopedReadAccess;
    typedef vcl::ScopedBitmapAccess< BitmapWriteAccess, AlphaMask, &AlphaMask::AcquireWriteAccess >
        ScopedWriteAccess;

private:
    friend class BitmapEx;
    friend class ::OutputDevice;
    friend bool VCL_DLLPUBLIC ReadDIBBitmapEx(BitmapEx& rTarget, SvStream& rIStm);

    SAL_DLLPRIVATE const Bitmap&    ImplGetBitmap() const;
    SAL_DLLPRIVATE void             ImplSetBitmap( const Bitmap& rBitmap );

};

#endif // INCLUDED_VCL_ALPHA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
