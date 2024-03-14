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

#include <sal/config.h>

#include <config_cairo_canvas.h>

#include <salgdi.hxx>
#include <sallayout.hxx>

#include <unx/cairotextrender.hxx>

#include <headless/SvpGraphicsBackend.hxx>
#include <headless/CairoCommon.hxx>

namespace vcl::font
{
class PhysicalFontFace;
class PhysicalFontCollection;
}

namespace psp { struct JobData; }

class FreetypeFontInstance;
class FontAttributes;
class SalInfoPrinter;
class FontMetricData;

class VCL_DLLPUBLIC GenPspGraphics final : public SalGraphicsAutoDelegateToImpl
{

    psp::JobData*           m_pJobData;

    CairoCommon             m_aCairoCommon;
    CairoTextRender         m_aTextRenderImpl;
    std::unique_ptr<SvpGraphicsBackend> m_pBackend;

public:
                            GenPspGraphics();
    SAL_DLLPRIVATE virtual ~GenPspGraphics() override;

    SAL_DLLPRIVATE void     Init(psp::JobData* pJob);

    // override all pure virtual methods
    virtual SalGraphicsImpl* GetImpl() const override
    {
        return m_pBackend.get();
    }

    SAL_DLLPRIVATE virtual void GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY ) override;

    SAL_DLLPRIVATE virtual void SetTextColor( Color nColor ) override;
    SAL_DLLPRIVATE virtual void SetFont(LogicalFontInstance*, int nFallbackLevel) override;
    SAL_DLLPRIVATE virtual void GetFontMetric( FontMetricDataRef&, int nFallbackLevel ) override;
    SAL_DLLPRIVATE virtual FontCharMapRef GetFontCharMap() const override;
    SAL_DLLPRIVATE virtual bool GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const override;
    SAL_DLLPRIVATE virtual void GetDevFontList( vcl::font::PhysicalFontCollection* ) override;
    // graphics must drop any cached font info
    SAL_DLLPRIVATE virtual void ClearDevFontCache() override;
    SAL_DLLPRIVATE virtual bool AddTempDevFont( vcl::font::PhysicalFontCollection*,
                                            const OUString& rFileURL,
                                            const OUString& rFontName ) override;

    SAL_DLLPRIVATE virtual std::unique_ptr<GenericSalLayout>
                            GetTextLayout(int nFallbackLevel) override;
    SAL_DLLPRIVATE virtual void DrawTextLayout( const GenericSalLayout& ) override;

    SAL_DLLPRIVATE virtual SystemGraphicsData GetGraphicsData() const override;

#if ENABLE_CAIRO_CANVAS
    SAL_DLLPRIVATE virtual bool SupportsCairo() const override;
    SAL_DLLPRIVATE virtual cairo::SurfaceSharedPtr CreateSurface(const cairo::CairoSurfaceSharedPtr& rSurface) const override;
    SAL_DLLPRIVATE virtual cairo::SurfaceSharedPtr CreateSurface(const OutputDevice& rRefDevice, int x, int y, int width, int height) const override;
    SAL_DLLPRIVATE virtual cairo::SurfaceSharedPtr CreateBitmapSurface(const OutputDevice& rRefDevice, const BitmapSystemData& rData, const Size& rSize) const override;
    SAL_DLLPRIVATE virtual css::uno::Any   GetNativeSurfaceHandle(cairo::SurfaceSharedPtr& rSurface, const basegfx::B2ISize& rSize) const override;
#endif // ENABLE_CAIRO_CANVAS
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
