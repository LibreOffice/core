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

class VCL_DLLPUBLIC SvpSalBitmap final : public SalBitmap, public basegfx::SystemDependentDataHolder // MM02
{
    std::unique_ptr<BitmapBuffer> mpDIB;
public:
             SvpSalBitmap();
    virtual ~SvpSalBitmap() override;

    // SalBitmap
    virtual bool            Create(const Size& rSize,
                                   vcl::PixelFormat ePixelFormat,
                                   const BitmapPalette& rPalette) override;
    virtual bool            Create( const SalBitmap& rSalBmp ) override;
    virtual bool            Create( const SalBitmap& rSalBmp,
                                    SalGraphics* pGraphics ) override;
    virtual bool            Create(const SalBitmap& rSalBmp,
                                   vcl::PixelFormat eNewPixelFormat) override;
    virtual bool            Create( const css::uno::Reference< css::rendering::XBitmapCanvas >& rBitmapCanvas,
                                    Size& rSize,
                                    bool bMask = false ) override;
    void                    Create(std::unique_ptr<BitmapBuffer> pBuf);
    const BitmapBuffer*     GetBuffer() const
    {
        return mpDIB.get();
    }
    virtual void            Destroy() final override;
    virtual Size            GetSize() const override;
    virtual sal_uInt16      GetBitCount() const override;

    virtual BitmapBuffer*   AcquireBuffer( BitmapAccessMode nMode ) override;
    virtual void            ReleaseBuffer( BitmapBuffer* pBuffer, BitmapAccessMode nMode ) override;
    virtual bool            GetSystemData( BitmapSystemData& rData ) override;

    virtual bool            ScalingSupported() const override;
    virtual bool            Scale( const double& rScaleX, const double& rScaleY, BmpScaleFlag nScaleFlag ) override;
    virtual bool            Replace( const Color& rSearchColor, const Color& rReplaceColor, sal_uInt8 nTol ) override;

    // MM02 exclusive management op's for SystemDependentData at WinSalBitmap
    template<class T>
    std::shared_ptr<T> getSystemDependentData() const
    {
        return std::static_pointer_cast<T>(basegfx::SystemDependentDataHolder::getSystemDependentData(typeid(T).hash_code()));
    }

    template<class T, class... Args>
    std::shared_ptr<T> addOrReplaceSystemDependentData(basegfx::SystemDependentDataManager& manager, Args&&... args) const
    {
        std::shared_ptr<T> r = std::make_shared<T>(manager, std::forward<Args>(args)...);

        // tdf#129845 only add to buffer if a relevant buffer time is estimated
        if(r->calculateCombinedHoldCyclesInSeconds() > 0)
        {
            basegfx::SystemDependentData_SharedPtr r2(r);
            const_cast< SvpSalBitmap* >(this)->basegfx::SystemDependentDataHolder::addOrReplaceSystemDependentData(r2);
        }

        return r;
    }
};

#endif // INCLUDED_VCL_INC_HEADLESS_SVPBMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
