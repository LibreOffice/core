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

#ifndef INCLUDED_VCL_INC_HEADLESS_SVPBMP_HXX
#define INCLUDED_VCL_INC_HEADLESS_SVPBMP_HXX

#include <sal/config.h>

#include <salbmp.hxx>
#include <basegfx/utils/systemdependentdata.hxx>
#include <optional>

class VCL_DLLPUBLIC SvpSalBitmap final : public SalBitmap, public basegfx::SystemDependentDataHolder // MM02
{
    std::optional<BitmapBuffer> moDIB;
public:
             SvpSalBitmap();
    virtual ~SvpSalBitmap() override;

    SAL_DLLPRIVATE bool        ImplCreate(const Size& rSize,
                           vcl::PixelFormat ePixelFormat,
                           const BitmapPalette& rPalette,
                           bool bClear);

    // SalBitmap
    SAL_DLLPRIVATE virtual bool            Create(const Size& rSize,
                                   vcl::PixelFormat ePixelFormat,
                                   const BitmapPalette& rPalette) override;
    SAL_DLLPRIVATE virtual bool            Create( const SalBitmap& rSalBmp ) override;
    SAL_DLLPRIVATE virtual bool            Create( const SalBitmap& rSalBmp,
                                    SalGraphics* pGraphics ) override;
    SAL_DLLPRIVATE virtual bool            Create(const SalBitmap& rSalBmp,
                                   vcl::PixelFormat eNewPixelFormat) override;
    SAL_DLLPRIVATE virtual bool            Create( const css::uno::Reference< css::rendering::XBitmapCanvas >& rBitmapCanvas,
                                    Size& rSize,
                                    bool bMask = false ) override;
    SAL_DLLPRIVATE void                    Create(const std::optional<BitmapBuffer> & pBuf);
    const BitmapBuffer*     GetBuffer() const
    {
        return moDIB ? &*moDIB : nullptr;
    }
    SAL_DLLPRIVATE virtual void            Destroy() final override;
    SAL_DLLPRIVATE virtual Size            GetSize() const override;
    SAL_DLLPRIVATE virtual sal_uInt16      GetBitCount() const override;

    SAL_DLLPRIVATE virtual BitmapBuffer*   AcquireBuffer( BitmapAccessMode nMode ) override;
    SAL_DLLPRIVATE virtual void            ReleaseBuffer( BitmapBuffer* pBuffer, BitmapAccessMode nMode ) override;
    SAL_DLLPRIVATE virtual bool            GetSystemData( BitmapSystemData& rData ) override;

    SAL_DLLPRIVATE virtual bool            ScalingSupported() const override;
    SAL_DLLPRIVATE virtual bool            Scale( const double& rScaleX, const double& rScaleY, BmpScaleFlag nScaleFlag ) override;
    SAL_DLLPRIVATE virtual bool            Replace( const Color& rSearchColor, const Color& rReplaceColor, sal_uInt8 nTol ) override;

    SAL_DLLPRIVATE virtual const basegfx::SystemDependentDataHolder* accessSystemDependentDataHolder() const override;
};

#endif // INCLUDED_VCL_INC_HEADLESS_SVPBMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
