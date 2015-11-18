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

#ifndef INCLUDED_VCL_INC_WIN_SALBMP_H
#define INCLUDED_VCL_INC_WIN_SALBMP_H

#include <tools/gen.hxx>
#include <win/wincomp.hxx>
#include <salbmp.hxx>
#include <memory>

// - SalBitmap  -

struct  BitmapBuffer;
class   BitmapColor;
class   BitmapPalette;
class   SalGraphics;
namespace Gdiplus { class Bitmap; }
typedef std::shared_ptr< Gdiplus::Bitmap > GdiPlusBmpPtr;

class WinSalBitmap : public SalBitmap
{
private:
    friend class GdiPlusBuffer; // allow buffer to remove maGdiPlusBitmap and mpAssociatedAlpha eventually

    Size                maSize;
    HGLOBAL             mhDIB;
    HBITMAP             mhDDB;

    // the buffered evtl. used Gdiplus::Bitmap instance. It is managed by
    // GdiPlusBuffer. To make this safe, it is only handed out as shared
    // pointer; the GdiPlusBuffer may delete the local instance.

    // mpAssociatedAlpha holds the last WinSalBitmap used to construct an
    // evtl. buffered GdiPlusBmp. This is needed since the GdiPlusBmp is a single
    // instance and remembered only on the content-WinSalBitmap, not on the
    // alpha-WinSalBitmap.
    GdiPlusBmpPtr       maGdiPlusBitmap;
    const WinSalBitmap* mpAssociatedAlpha;

    sal_uInt16          mnBitCount;

    Gdiplus::Bitmap*    ImplCreateGdiPlusBitmap(const WinSalBitmap& rAlphaSource);
    Gdiplus::Bitmap*    ImplCreateGdiPlusBitmap();

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
    virtual bool                Create( const Size& rSize, sal_uInt16 nBitCount, const BitmapPalette& rPal ) override;
    virtual bool                Create( const SalBitmap& rSalBmpImpl ) override;
    virtual bool                Create( const SalBitmap& rSalBmpImpl, SalGraphics* pGraphics ) override;
    virtual bool                Create( const SalBitmap& rSalBmpImpl, sal_uInt16 nNewBitCount ) override;
    virtual bool                Create( const css::uno::Reference< css::rendering::XBitmapCanvas >& rBitmapCanvas,
                                           Size& rSize,
                                           bool bMask = false ) override;

    virtual void                Destroy() override;

    virtual Size                GetSize() const override { return maSize; }
    virtual sal_uInt16          GetBitCount() const override { return mnBitCount; }

    virtual BitmapBuffer*       AcquireBuffer( BitmapAccessMode nMode ) override;
    virtual void                ReleaseBuffer( BitmapBuffer* pBuffer, BitmapAccessMode nMode ) override;
    virtual bool                GetSystemData( BitmapSystemData& rData ) override;

    virtual bool                Scale( const double& rScaleX, const double& rScaleY, BmpScaleFlag nScaleFlag ) override;
    virtual bool                Replace( const Color& rSearchColor, const Color& rReplaceColor, sal_uLong nTol ) override;
};

#endif // INCLUDED_VCL_INC_WIN_SALBMP_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
