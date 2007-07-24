/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: impbmp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-24 09:59:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_IMPBMP_HXX
#define _SV_IMPBMP_HXX

#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif

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
