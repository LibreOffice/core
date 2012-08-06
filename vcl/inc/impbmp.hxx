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

#ifndef _SV_IMPBMP_HXX
#define _SV_IMPBMP_HXX

#include <tools/gen.hxx>
#include <tools/solar.h>

// ---------------
// - ImpBitmap -
// ---------------

struct BitmapBuffer;
class SalBitmap;
class BitmapPalette;
class SalGraphics;
class Bitmap;
class OutputDevice;
class Color;
class AlphaMask;

class ImpBitmap
{
private:

    sal_uLong               mnRefCount;
    sal_uLong               mnChecksum;
    SalBitmap*          mpSalBitmap;
    Size                maSourceSize;

public:

                        ImpBitmap();
                        ~ImpBitmap();

#if _SOLAR__PRIVATE

public:

    void                ImplSetSalBitmap( SalBitmap* pSalBitmap );
    SalBitmap*          ImplGetSalBitmap() const { return mpSalBitmap; }

public:

    sal_Bool                ImplCreate( const Size& rSize, sal_uInt16 nBitCount, const BitmapPalette& rPal );
    sal_Bool                ImplCreate( const ImpBitmap& rImpBitmap );
    sal_Bool                ImplCreate( const ImpBitmap& rImpBitmap, SalGraphics* pGraphics );
    sal_Bool                ImplCreate( const ImpBitmap& rImpBitmap, sal_uInt16 nNewBitCount );

    Size                ImplGetSize() const;
    Size                ImplGetSourceSize() const;
    void                ImplSetSourceSize( const Size&);
    sal_uInt16              ImplGetBitCount() const;

    BitmapBuffer*       ImplAcquireBuffer( sal_Bool bReadOnly );
    void                ImplReleaseBuffer( BitmapBuffer* pBuffer, sal_Bool bReadOnly );

public:

    sal_uLong               ImplGetRefCount() const { return mnRefCount; }
    void                ImplIncRefCount() { mnRefCount++; }
    void                ImplDecRefCount() { mnRefCount--; }

    inline void         ImplSetChecksum( sal_uLong nChecksum ) { mnChecksum = nChecksum; }
    inline sal_uLong        ImplGetChecksum() const { return mnChecksum; }

#endif // PRIVATE
};

inline Size ImpBitmap::ImplGetSourceSize() const
{
    return maSourceSize;
}

inline void ImpBitmap::ImplSetSourceSize( const Size& rSize)
{
    maSourceSize = rSize;
}

#endif // _SV_IMPBMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
