/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

    void                ImplDestroy();

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
