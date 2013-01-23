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



#ifndef _SV_SALBMP_H
#define _SV_SALBMP_H

#include <tools/gen.hxx>
#include <win/wincomp.hxx>
#include <salbmp.hxx>
#include <boost/shared_ptr.hpp>

// --------------
// - SalBitmap  -
// --------------

struct  BitmapBuffer;
class   BitmapColor;
class   BitmapPalette;
class   SalGraphics;
namespace Gdiplus { class Bitmap; }
typedef boost::shared_ptr< Gdiplus::Bitmap > GdiPlusBmpPtr;

class WinSalBitmap : public SalBitmap
{
private:
    friend class GdiPlusBuffer; // allow buffer to remove maGdiPlusBitmap eventually

    Size                maSize;
    HGLOBAL             mhDIB;
    HBITMAP             mhDDB;

    // the buffered evtl. used Gdiplus::Bitmap instance. It is managed by
    // GdiPlusBuffer. To make this safe, it is only handed out as shared
    // pointer; the GdiPlusBuffer may delete the local instance
    GdiPlusBmpPtr       maGdiPlusBitmap;

    sal_uInt16          mnBitCount;

    Gdiplus::Bitmap* ImplCreateGdiPlusBitmap(const WinSalBitmap& rAlphaSource);
    Gdiplus::Bitmap* ImplCreateGdiPlusBitmap();

public:

    HGLOBAL             ImplGethDIB() const { return mhDIB; }
    HBITMAP             ImplGethDDB() const { return mhDDB; }

    GdiPlusBmpPtr ImplGetGdiPlusBitmap(const WinSalBitmap* pAlphaSource = 0) const;

    static HGLOBAL      ImplCreateDIB( const Size& rSize, sal_uInt16 nBitCount, const BitmapPalette& rPal );
    static HANDLE       ImplCopyDIBOrDDB( HANDLE hHdl, bool bDIB );
    static sal_uInt16   ImplGetDIBColorCount( HGLOBAL hDIB );
    static void         ImplDecodeRLEBuffer( const BYTE* pSrcBuf, BYTE* pDstBuf,
                                             const Size& rSizePixel, bool bRLE4 );

public:

                        WinSalBitmap();
    virtual             ~WinSalBitmap();

public:

    bool                        Create( HANDLE hBitmap, bool bDIB, bool bCopyHandle );
    virtual bool                Create( const Size& rSize, sal_uInt16 nBitCount, const BitmapPalette& rPal );
    virtual bool                Create( const SalBitmap& rSalBmpImpl );
    virtual bool                Create( const SalBitmap& rSalBmpImpl, SalGraphics* pGraphics );
    virtual bool                Create( const SalBitmap& rSalBmpImpl, sal_uInt16 nNewBitCount );

    virtual void                Destroy();

    virtual Size                GetSize() const { return maSize; }
    virtual sal_uInt16              GetBitCount() const { return mnBitCount; }

    virtual BitmapBuffer*       AcquireBuffer( bool bReadOnly );
    virtual void                ReleaseBuffer( BitmapBuffer* pBuffer, bool bReadOnly );
    virtual bool                GetSystemData( BitmapSystemData& rData );
};

#endif // _SV_SALBMP_HXX
