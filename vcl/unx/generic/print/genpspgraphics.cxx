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

#include <sal/config.h>
#include <sal/log.hxx>

#include <vector>

#include <sal/types.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <i18nlangtag/mslangid.hxx>
#include <jobdata.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/fontcharmap.hxx>
#include <config_cairo_canvas.h>

#include <fontsubset.hxx>
#include <unx/freetype_glyphcache.hxx>
#include <unx/geninst.h>
#include <unx/genpspgraphics.h>
#include <langboost.hxx>
#include <font/LogicalFontInstance.hxx>
#include <fontattributes.hxx>
#include <font/FontMetricData.hxx>
#include <font/FontSelectPattern.hxx>
#include <font/PhysicalFontCollection.hxx>
#include <font/PhysicalFontFace.hxx>
#include <o3tl/string_view.hxx>
#include <sallayout.hxx>

using namespace psp;

/*******************************************************
 * GenPspGraphics
 *******************************************************/

GenPspGraphics::GenPspGraphics()
    : m_pJobData( nullptr )
    , m_aTextRenderImpl(m_aCairoCommon)
    , m_pBackend(new SvpGraphicsBackend(m_aCairoCommon))
{
}

void GenPspGraphics::Init(psp::JobData* pJob)
{
    m_pJobData = pJob;
    SetLayout( SalLayoutFlags::NONE );
}

GenPspGraphics::~GenPspGraphics()
{
    ReleaseFonts();
}

void GenPspGraphics::GetResolution( sal_Int32 &rDPIX, sal_Int32 &rDPIY )
{
    if (m_pJobData != nullptr)
    {
        int x = m_pJobData->m_aContext.getRenderResolution();

        rDPIX = x;
        rDPIY = x;
    }
}

void GenPspGraphics::DrawTextLayout(const GenericSalLayout& rLayout)
{
    m_aTextRenderImpl.DrawTextLayout(rLayout, *this);
}

FontCharMapRef GenPspGraphics::GetFontCharMap() const
{
    return m_aTextRenderImpl.GetFontCharMap();
}

bool GenPspGraphics::GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const
{
    return m_aTextRenderImpl.GetFontCapabilities(rFontCapabilities);
}

void GenPspGraphics::SetFont(LogicalFontInstance *pFontInstance, int nFallbackLevel)
{
    m_aTextRenderImpl.SetFont(pFontInstance, nFallbackLevel);
}

void GenPspGraphics::SetTextColor( Color nColor )
{
    m_aTextRenderImpl.SetTextColor(nColor);
}

bool GenPspGraphics::AddTempDevFont( vcl::font::PhysicalFontCollection* pFontCollection,
                                     const OUString& rFileURL,
                                     const OUString& rFontName )
{
    return m_aTextRenderImpl.AddTempDevFont(pFontCollection, rFileURL, rFontName);
}

void GenPspGraphics::GetDevFontList( vcl::font::PhysicalFontCollection *pFontCollection )
{
    m_aTextRenderImpl.GetDevFontList(pFontCollection);
}

void GenPspGraphics::ClearDevFontCache()
{
    m_aTextRenderImpl.ClearDevFontCache();
}

void GenPspGraphics::GetFontMetric(FontMetricDataRef& rxFontMetric, int nFallbackLevel)
{
    m_aTextRenderImpl.GetFontMetric(rxFontMetric, nFallbackLevel);
}

std::unique_ptr<GenericSalLayout> GenPspGraphics::GetTextLayout(int nFallbackLevel)
{
    return m_aTextRenderImpl.GetTextLayout(nFallbackLevel);
}

namespace vcl
{
    const char* getLangBoost()
    {
        const char* pLangBoost;
        const LanguageType eLang = Application::GetSettings().GetUILanguageTag().getLanguageType();
        if (eLang == LANGUAGE_JAPANESE)
            pLangBoost = "jan";
        else if (MsLangId::isKorean(eLang))
            pLangBoost = "kor";
        else if (MsLangId::isSimplifiedChinese(eLang))
            pLangBoost = "zhs";
        else if (MsLangId::isTraditionalChinese(eLang))
            pLangBoost = "zht";
        else
            pLangBoost = nullptr;
        return pLangBoost;
    }
}

SystemGraphicsData GenPspGraphics::GetGraphicsData() const
{
    return SystemGraphicsData();
}

#if ENABLE_CAIRO_CANVAS

bool GenPspGraphics::SupportsCairo() const
{
    return false;
}

cairo::SurfaceSharedPtr GenPspGraphics::CreateSurface(const cairo::CairoSurfaceSharedPtr& /*rSurface*/) const
{
    return cairo::SurfaceSharedPtr();
}

cairo::SurfaceSharedPtr GenPspGraphics::CreateSurface(const OutputDevice& /*rRefDevice*/, int /*x*/, int /*y*/, int /*width*/, int /*height*/) const
{
    return cairo::SurfaceSharedPtr();
}

cairo::SurfaceSharedPtr GenPspGraphics::CreateBitmapSurface(const OutputDevice& /*rRefDevice*/, const BitmapSystemData& /*rData*/, const Size& /*rSize*/) const
{
    return cairo::SurfaceSharedPtr();
}

css::uno::Any GenPspGraphics::GetNativeSurfaceHandle(cairo::SurfaceSharedPtr& /*rSurface*/, const basegfx::B2ISize& /*rSize*/) const
{
    return css::uno::Any();
}

#endif // ENABLE_CAIRO_CANVAS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
