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

#include <unx/generictextrender.hxx>

#include <unotools/configmgr.hxx>
#include <vcl/settings.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/svapp.hxx>
#include <vcl/fontcharmap.hxx>
#include <sal/log.hxx>

#include <unx/font/fontmanager.hxx>
#include <unx/geninst.h>
#include <unx/font/GenericFontList.hxx>
#include <unx/font/fc_fontoptions.hxx>
#include <unx/font/GenericFont.hxx>
#include <font/PhysicalFontFace.hxx>
#include <font/FontMetricData.hxx>

#include <sallayout.hxx>

GenericTextRenderImpl::GenericTextRenderImpl()
    : mnTextColor(Color(0x00, 0x00, 0x00)) //black
{
}

GenericTextRenderImpl::~GenericTextRenderImpl()
{
    ReleaseFonts();
}

void GenericTextRenderImpl::SetFont(LogicalFontInstance *pEntry, int nFallbackLevel)
{
    // release all no longer needed font resources
    for( int i = nFallbackLevel; i < MAX_FALLBACK; ++i )
    {
        // old server side font is no longer referenced
        mpGenericFont[i] = nullptr;
    }

    // return early if there is no new font
    if( !pEntry )
        return;

    GenericFont* pGenericFont = static_cast<GenericFont*>(pEntry);
    mpGenericFont[ nFallbackLevel ] = pGenericFont;

    // ignore fonts with e.g. corrupted font files
    if (!mpGenericFont[nFallbackLevel]->TestFont())
        mpGenericFont[nFallbackLevel] = nullptr;
}

FontCharMapRef GenericTextRenderImpl::GetFontCharMap() const
{
    if (!mpGenericFont[0])
        return nullptr;
    return mpGenericFont[0]->GetFontFace()->GetFontCharMap();
}

bool GenericTextRenderImpl::GetFontCapabilities(vcl::FontCapabilities &rGetImplFontCapabilities) const
{
    if (!mpGenericFont[0])
        return false;
    return mpGenericFont[0]->GetFontFace()->GetFontCapabilities(rGetImplFontCapabilities);
}

// SalGraphics
void
GenericTextRenderImpl::SetTextColor( Color nColor )
{
    if( mnTextColor != nColor )
    {
        mnTextColor = nColor;
    }
}

bool GenericTextRenderImpl::AddTempDevFont(vcl::font::PhysicalFontCollection* pFontCollection,
                                            const OUString& rFileURL, const OUString& rFontName)
{
    GenericFontList& rFontList = GenericFontList::get();
    if (!rFontList.AddFontFile(rFileURL, rFontName))
        return false;

    // announce new font to device's font list
    rFontList.AnnounceFonts(pFontCollection);
    return true;
}

bool GenericTextRenderImpl::RemoveTempDevFont(const OUString& rFileURL, const OUString& /*rFontName*/)
{
    GenericFontList::get().RemoveFontFile(rFileURL);
    return true;
}

void GenericTextRenderImpl::ClearDevFontCache()
{
}

void GenericTextRenderImpl::GetDevFontList(vcl::font::PhysicalFontCollection* pFontCollection)
{
    GenericFontList::get().AnnounceFonts(pFontCollection);

    // register platform specific font substitutions if available
    SalGenericInstance::RegisterFontSubstitutors(pFontCollection);
}

void GenericTextRenderImpl::GetFontMetric( FontMetricDataRef& rxFontMetric, int nFallbackLevel )
{
    if( nFallbackLevel >= MAX_FALLBACK )
        return;

    if (mpGenericFont[nFallbackLevel])
        mpGenericFont[nFallbackLevel]->GetFontMetric(rxFontMetric);
}

std::unique_ptr<GenericSalLayout> GenericTextRenderImpl::GetTextLayout(int nFallbackLevel)
{
    if (!mpGenericFont[nFallbackLevel])
        return nullptr;
    return std::make_unique<GenericSalLayout>(*mpGenericFont[nFallbackLevel]);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
