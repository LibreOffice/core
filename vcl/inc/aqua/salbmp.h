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
