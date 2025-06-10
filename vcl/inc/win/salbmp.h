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

#pragma once

#include <tools/gen.hxx>
#include <win/wincomp.hxx>
#include <salbmp.hxx>
#include <basegfx/utils/systemdependentdata.hxx>
#include <memory>


struct  BitmapBuffer;
class   BitmapColor;
class   BitmapPalette;
class   SalGraphics;
namespace Gdiplus { class Bitmap; }

class WinSalBitmap final: public SalBitmap
{
private:
    Size                maSize;
    void*               mpDIB;
    sal_Int32           mnDIBSize { 0 };
    HBITMAP             mhDDB;

    sal_uInt16          mnBitCount;

    std::shared_ptr<Gdiplus::Bitmap>    ImplCreateGdiPlusBitmap(const WinSalBitmap& rAlphaSource);
    std::shared_ptr<Gdiplus::Bitmap> ImplCreateGdiPlusBitmap();

public:

    void*               ImplGethDIB() const { return mpDIB; }
    HBITMAP             ImplGethDDB() const { return mhDDB; }

    std::shared_ptr< Gdiplus::Bitmap > ImplGetGdiPlusBitmap(const WinSalBitmap* pAlphaSource = nullptr) const;

    static void         ImplCreateDIB( const Size& rSize, vcl::PixelFormat ePixelFormat, const BitmapPalette& rPal,
                                       void*& rpDIB, sal_Int32 &rnDIBSize );
    static HBITMAP      ImplCopyDDB( HBITMAP hHdl );
    static void*        ImplCopyDIB( void* pDIB, sal_Int32 nDIBSize );
    static sal_uInt16   ImplGetDIBColorCount( void* pDIB );

public:

                        WinSalBitmap();
    virtual             ~WinSalBitmap() override;

    using SalBitmap::addOrReplaceSystemDependentData;
    using SalBitmap::getSystemDependentData;
    using SystemDependentDataHolder::addOrReplaceSystemDependentData;
    using SystemDependentDataHolder::getSystemDependentData;

public:

    bool                        Create( HBITMAP hBitmap );
    virtual bool                Create( const Size& rSize, vcl::PixelFormat ePixelFormat, const BitmapPalette& rPal ) override;
    virtual bool                Create( const SalBitmap& rSalBmpImpl ) override;
    virtual bool                Create( const SalBitmap& rSalBmpImpl, SalGraphics* pGraphics ) override;
    virtual bool                Create( const SalBitmap& rSalBmpImpl, vcl::PixelFormat eNewPixelFormat ) override;
    virtual bool                Create( const css::uno::Reference< css::rendering::XBitmapCanvas >& rBitmapCanvas,
                                           Size& rSize,
                                           bool bMask = false ) override;

    virtual void                Destroy() override;

    virtual Size                GetSize() const override { return maSize; }
    virtual sal_uInt16          GetBitCount() const override { return mnBitCount; }

    virtual BitmapBuffer*       AcquireBuffer( BitmapAccessMode nMode ) override;
    virtual void                ReleaseBuffer( BitmapBuffer* pBuffer, BitmapAccessMode nMode ) override;
    virtual bool                GetSystemData( BitmapSystemData& rData ) override;

    virtual bool                ScalingSupported() const override;
    virtual bool                Scale( const double& rScaleX, const double& rScaleY, BmpScaleFlag nScaleFlag ) override;
    virtual bool                Replace( const Color& rSearchColor, const Color& rReplaceColor, sal_uInt8 nTol ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
