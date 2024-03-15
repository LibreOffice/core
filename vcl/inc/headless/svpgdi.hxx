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

#ifdef IOS
#error This file is not for iOS
#endif

#include <sal/config.h>

#include <osl/endian.h>
#include <vcl/sysdata.hxx>
#include <config_cairo_canvas.h>

#include <salgdi.hxx>
#include <sallayout.hxx>
#include <unx/cairotextrender.hxx>
#include <font/FontMetricData.hxx>

#include <headless/SvpGraphicsBackend.hxx>
#include <headless/CairoCommon.hxx>

struct BitmapBuffer;
class FreetypeFont;

class VCL_DLLPUBLIC SvpSalGraphics : public SalGraphicsAutoDelegateToImpl
{
    CairoCommon m_aCairoCommon;
    CairoTextRender m_aTextRenderImpl;
    std::unique_ptr<SvpGraphicsBackend> m_pBackend;

public:
    void setSurface(cairo_surface_t* pSurface, const basegfx::B2IVector& rSize);
    cairo_surface_t* getSurface() const { return m_aCairoCommon.m_pSurface; }
    static cairo_user_data_key_t* getDamageKey()
    {
        return CairoCommon::getDamageKey();
    }

protected:

    cairo_t* createTmpCompatibleCairoContext() const;

public:
    SvpSalGraphics();
    virtual ~SvpSalGraphics() override;

    virtual SalGraphicsImpl* GetImpl() const override { return m_pBackend.get(); }
    std::unique_ptr<SvpGraphicsBackend> const& getSvpBackend() { return m_pBackend; }

    SAL_DLLPRIVATE virtual void            GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY ) override;

    virtual void            SetTextColor( Color nColor ) override;
    virtual void            SetFont(LogicalFontInstance*, int nFallbackLevel) override;
    virtual void            GetFontMetric( FontMetricDataRef&, int nFallbackLevel ) override;
    virtual FontCharMapRef  GetFontCharMap() const override;
    virtual bool GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const override;
    virtual void            GetDevFontList( vcl::font::PhysicalFontCollection* ) override;
    virtual void ClearDevFontCache() override;
    virtual bool            AddTempDevFont( vcl::font::PhysicalFontCollection*, const OUString& rFileURL, const OUString& rFontName ) override;
    virtual std::unique_ptr<GenericSalLayout>
                            GetTextLayout(int nFallbackLevel) override;
    virtual void            DrawTextLayout( const GenericSalLayout& ) override;

    virtual bool            ShouldDownscaleIconsAtSurface(double* pScaleOut) const override;

    virtual SystemGraphicsData GetGraphicsData() const override;

#if ENABLE_CAIRO_CANVAS
    SAL_DLLPRIVATE virtual bool            SupportsCairo() const override;
    SAL_DLLPRIVATE virtual cairo::SurfaceSharedPtr CreateSurface(const cairo::CairoSurfaceSharedPtr& rSurface) const override;
    SAL_DLLPRIVATE virtual cairo::SurfaceSharedPtr CreateSurface(const OutputDevice& rRefDevice, int x, int y, int width, int height) const override;
    virtual cairo::SurfaceSharedPtr CreateBitmapSurface(const OutputDevice& rRefDevice, const BitmapSystemData& rData, const Size& rSize) const override;
    virtual css::uno::Any   GetNativeSurfaceHandle(cairo::SurfaceSharedPtr& rSurface, const basegfx::B2ISize& rSize) const override;
#endif // ENABLE_CAIRO_CANVAS

    cairo_t* getCairoContext() const
    {
        return m_aCairoCommon.getCairoContext(/*bXorModeAllowed*/false, getAntiAlias());
    }

    void clipRegion(cairo_t* cr)
    {
        m_aCairoCommon.clipRegion(cr);
    }

    void copySource(const SalTwoRect& rTR, cairo_surface_t* source)
    {
        m_aCairoCommon.copySource(rTR, source, getAntiAlias());
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
