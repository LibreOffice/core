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

#include <stdlib.h>

#include <unx/font/GenericFontList.hxx>
#include <unx/font/GenericFontFace.hxx>
#include <unx/font/fontmanager.hxx>
#include <unx/gendata.hxx>

#include <font/PhysicalFontCollection.hxx>

#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <tools/urlobj.hxx>

// TODO: remove when the priorities are selected by UI
// if (AH==0) => disable autohinting
// if (AA==0) => disable antialiasing
// if (EB==0) => disable embedded bitmaps
// if (AA prio <= AH prio) => antialias + autohint
// if (AH<AA) => do not autohint when antialiasing
// if (EB<AH) => do not autohint for monochrome
static int nDefaultPrioAntiAlias = 1;

int GetDefaultAntiAliasPrio() { return nDefaultPrioAntiAlias; }

GenericFontList::GenericFontList()
{
    // TODO: remove when the priorities are selected by UI
    if (const char* pEnv = ::getenv("SAL_ANTIALIASED_TEXT_PRIORITY"))
        nDefaultPrioAntiAlias = pEnv[0] - '0';
}

void GenericFontList::Init()
{
    // GenericUnixSalData::GetGenericFontList() constructs the FontConfigManager
    // before us, so fontconfig is up, knows about our private font directories,
    // and has already collected the fonts it will hand over here.
    for (const auto& rFont : FontConfigManager::get().takeSystemFonts())
    {
        FontAttributes aDFA = rFont.m_aFontAttributes;
        aDFA.IncreaseQualityBy(4096);
        AddFontFace(aDFA, rFont.m_aFontFile, rFont.m_nCollectionEntry, rFont.m_nVariationEntry);
    }

    SAL_INFO("vcl.fonts", "have " << m_aFontFaceList.size() << " fonts");
}

GenericFontList::~GenericFontList() {}

GenericFontList& GenericFontList::get()
{
    GenericUnixSalData* const pSalData(GetGenericUnixSalData());
    assert(pSalData);
    return *pSalData->GetGenericFontList();
}

void GenericFontList::AddFontFace(const FontAttributes& rDevFontAttr, const OString& rFileName,
                                   int nFaceNum, int nVariationNum)
{
    if( rFileName.isEmpty() )
        return;

    const sal_IntPtr nFontId = m_nNextFontId++;
    m_aFontFaceList[nFontId]
        = new GenericFontFace(rDevFontAttr, rFileName, nFaceNum, nVariationNum, nFontId);
    m_aFontFileToFontId[ rFileName ].insert( nFontId );
}

bool GenericFontList::AddFontFile(std::u16string_view rFileUrl, const OUString& rFontName)
{
    INetURLObject aPath( rFileUrl );
    const OString aFullPath = OUStringToOString(aPath.GetFull(), osl_getThreadTextEncoding());

    // already added? then the faces are there already
    if (m_aFontFileToFontId.contains(aFullPath))
        return true;

    std::vector<FontconfigFont> aFonts = FontConfigManager::get().addFontFile(aFullPath);
    if (aFonts.empty())
        return false;

    for (const auto& rFont : aFonts)
    {
        FontAttributes aDFA = rFont.m_aFontAttributes;
        aDFA.IncreaseQualityBy(5800);
        if (!rFontName.isEmpty())
            aDFA.SetFamilyName(rFontName);
        AddFontFace(aDFA, rFont.m_aFontFile, rFont.m_nCollectionEntry, rFont.m_nVariationEntry);
    }

    return true;
}

void GenericFontList::RemoveFontFile(std::u16string_view rFileUrl)
{
    INetURLObject aPath( rFileUrl );
    const OString aFullPath = OUStringToOString(aPath.GetFull(), osl_getThreadTextEncoding());

    auto it = m_aFontFileToFontId.find(aFullPath);
    if (it == m_aFontFileToFontId.end())
        return; // not a font of ours; leave fontconfig alone

    for (sal_IntPtr nFontId : it->second)
        m_aFontFaceList.erase(nFontId);
    m_aFontFileToFontId.erase(it);

    FontConfigManager::removeFontFile(aFullPath);
}

const GenericFontFace* GenericFontList::FindFontFace(const OString& rFileName, int nFaceNum,
                                                       int nVariationNum) const
{
    auto it = m_aFontFileToFontId.find(rFileName);
    if (it == m_aFontFileToFontId.end())
        return nullptr;

    for (sal_IntPtr nFontId : it->second)
    {
        auto face_it = m_aFontFaceList.find(nFontId);
        if (face_it == m_aFontFaceList.end())
            continue;
        const GenericFontFace* pFace = face_it->second.get();
        if (pFace->GetFontFaceIndex() == nFaceNum
            && pFace->GetFontFaceVariation() == nVariationNum)
            return pFace;
    }

    return nullptr;
}

void GenericFontList::AnnounceFonts( vcl::font::PhysicalFontCollection* pToAdd ) const
{
    for (auto const& font : m_aFontFaceList)
        pToAdd->Add( font.second.get() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
