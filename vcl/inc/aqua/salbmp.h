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

#include "tools/gen.hxx"

#include "basebmp/bitmapdevice.hxx"

#include "vcl/salbtype.hxx"

#include "aqua/salconst.h"
#include "aqua/salgdi.h"

#include "saldata.hxx"
#include "salinst.hxx"
#include "salvd.hxx"
#include "salbmp.hxx"

#include "salcolorutils.hxx"


// --------------
// - SalBitmap  -
// --------------

struct  BitmapBuffer;
class   BitmapColor;
class   BitmapPalette;
class   AquaSalVirtualDevice;
class   AquaSalGraphics;

class AquaSalBitmap : public SalBitmap
{
public:
    CGContextRef                    mxGraphicContext;
    mutable CGImageRef              mxCachedImage;
    BitmapPalette                   maPalette;
    basebmp::RawMemorySharedArray   maUserBuffer;
    basebmp::RawMemorySharedArray   maContextBuffer;
    sal_uInt16                      mnBits;
    int                             mnWidth;
    int                             mnHeight;
    sal_uInt32                      mnBytesPerRow;

public:
    AquaSalBitmap();
    virtual ~AquaSalBitmap();

public:

    // SalBitmap methods
    bool            Create( const Size& rSize, sal_uInt16 nBitCount, const BitmapPalette& rPal );
    bool            Create( const SalBitmap& rSalBmp );
    bool            Create( const SalBitmap& rSalBmp, SalGraphics* pGraphics );
    bool            Create( const SalBitmap& rSalBmp, sal_uInt16 nNewBitCount );

    void            Destroy();

    Size            GetSize() const;
    sal_uInt16          GetBitCount() const;

    BitmapBuffer   *AcquireBuffer( bool bReadOnly );
    void            ReleaseBuffer( BitmapBuffer* pBuffer, bool bReadOnly );

    bool            GetSystemData( BitmapSystemData& rData );

private:
    // quartz helper
    bool            CreateContext();
    void            DestroyContext();
    bool            AllocateUserData();

    void            ConvertBitmapData( sal_uInt32 nWidth, sal_uInt32 nHeight,
                                       sal_uInt16 nDestBits, sal_uInt32 nDestBytesPerRow, const BitmapPalette& rDestPalette, sal_uInt8* pDestData,
                                       sal_uInt16 nSrcBits, sal_uInt32 nSrcBytesPerRow, const BitmapPalette& rSrcPalette, sal_uInt8* pSrcData );

public:
    bool            Create( CGLayerRef xLayer, int nBitCount, int nX, int nY, int nWidth, int nHeight, bool bMirrorVert = true );

public:
    CGImageRef      CreateWithMask( const AquaSalBitmap& rMask, int nX, int nY, int nWidth, int nHeight ) const;
    CGImageRef      CreateColorMask( int nX, int nY, int nWidth, int nHeight, SalColor nMaskColor ) const;
    CGImageRef      CreateCroppedImage( int nX, int nY, int nWidth, int nHeight ) const;
};

#endif // _SV_SALBMP_HXX
