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

#ifndef INCLUDED_VCL_INC_QUARTZ_SALBMP_H
#define INCLUDED_VCL_INC_QUARTZ_SALBMP_H

#include "tools/gen.hxx"

#include "basebmp/bitmapdevice.hxx"

#include "vcl/salbtype.hxx"

#include "quartz/salgdi.h"

#include "salinst.hxx"
#include "salvd.hxx"
#include "salbmp.hxx"

// - SalBitmap  -

struct  BitmapBuffer;
class   BitmapPalette;

class QuartzSalBitmap : public SalBitmap
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
    QuartzSalBitmap();
    virtual ~QuartzSalBitmap();

public:

    // SalBitmap methods
    bool            Create( const Size& rSize, sal_uInt16 nBitCount, const BitmapPalette& rPal ) override;
    bool            Create( const SalBitmap& rSalBmp ) override;
    bool            Create( const SalBitmap& rSalBmp, SalGraphics* pGraphics ) override;
    bool            Create( const SalBitmap& rSalBmp, sal_uInt16 nNewBitCount ) override;
    virtual bool    Create( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapCanvas >& rBitmapCanvas,
                            Size& rSize,
                            bool bMask = false ) override;

    void            Destroy() override;

    Size            GetSize() const override;
    sal_uInt16          GetBitCount() const override;

    BitmapBuffer   *AcquireBuffer( BitmapAccessMode nMode ) override;
    void            ReleaseBuffer( BitmapBuffer* pBuffer, BitmapAccessMode nMode ) override;

    bool            GetSystemData( BitmapSystemData& rData ) override;

    bool            Scale( const double& rScaleX, const double& rScaleY, BmpScaleFlag nScaleFlag ) override;
    bool            Replace( const Color& rSearchColor, const Color& rReplaceColor, sal_uLong nTol ) override;

private:
    // quartz helper
    bool            CreateContext();
    void            DestroyContext();
    bool            AllocateUserData();

    void            ConvertBitmapData( sal_uInt32 nWidth, sal_uInt32 nHeight,
                                       sal_uInt16 nDestBits, sal_uInt32 nDestBytesPerRow, const BitmapPalette& rDestPalette, sal_uInt8* pDestData,
                                       sal_uInt16 nSrcBits, sal_uInt32 nSrcBytesPerRow, const BitmapPalette& rSrcPalette, sal_uInt8* pSrcData );

public:
    bool            Create( CGLayerRef xLayer, int nBitCount, int nX, int nY, int nWidth, int nHeight );

public:
    CGImageRef      CreateWithMask( const QuartzSalBitmap& rMask, int nX, int nY, int nWidth, int nHeight ) const;
    CGImageRef      CreateColorMask( int nX, int nY, int nWidth, int nHeight, SalColor nMaskColor ) const;
    CGImageRef      CreateCroppedImage( int nX, int nY, int nWidth, int nHeight ) const;
};

#endif // INCLUDED_VCL_INC_QUARTZ_SALBMP_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
