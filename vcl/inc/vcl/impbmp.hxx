/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: impbmp.hxx,v $
 * $Revision: 1.3 $
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
#include <vcl/sv.h>

// ---------------
// - ImpBitmap -
// ---------------

struct BitmapBuffer;
class SalBitmap;
class BitmapPalette;
class SalGraphics;
class ImplServerBitmap;
class Bitmap;
class OutputDevice;
class Color;
class AlphaMask;

class ImpBitmap
{
private:

    ULONG               mnRefCount;
    ULONG               mnChecksum;
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

    BOOL                ImplCreate( const Size& rSize, USHORT nBitCount, const BitmapPalette& rPal );
    BOOL                ImplCreate( const ImpBitmap& rImpBitmap );
    BOOL                ImplCreate( const ImpBitmap& rImpBitmap, SalGraphics* pGraphics );
    BOOL                ImplCreate( const ImpBitmap& rImpBitmap, USHORT nNewBitCount );

    void                ImplDestroy();

    Size                ImplGetSize() const;
    Size                ImplGetSourceSize() const;
    void                ImplSetSourceSize( const Size&);
    USHORT              ImplGetBitCount() const;

    BitmapBuffer*       ImplAcquireBuffer( BOOL bReadOnly );
    void                ImplReleaseBuffer( BitmapBuffer* pBuffer, BOOL bReadOnly );

public:

    ULONG               ImplGetRefCount() const { return mnRefCount; }
    void                ImplIncRefCount() { mnRefCount++; }
    void                ImplDecRefCount() { mnRefCount--; }

    inline void         ImplSetChecksum( ULONG nChecksum ) { mnChecksum = nChecksum; }
    inline ULONG        ImplGetChecksum() const { return mnChecksum; }

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
