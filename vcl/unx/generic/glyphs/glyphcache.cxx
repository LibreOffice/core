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
#include <unx/freetype_glyphcache.hxx>
#include <unx/gendata.hxx>

#include <font/LogicalFontInstance.hxx>

#include <rtl/ustring.hxx>
#include <sal/log.hxx>

FreetypeManager::FreetypeManager()
{
    InitFreetype();
}

FreetypeManager::~FreetypeManager()
{
    ClearFontCache();
}

void FreetypeManager::ClearFontCache()
{
    m_aFontInfoList.clear();
}

FreetypeManager& FreetypeManager::get()
{
    GenericUnixSalData* const pSalData(GetGenericUnixSalData());
    assert(pSalData);
    return *pSalData->GetFreetypeManager();
}

FreetypeFontFile* FreetypeManager::FindFontFile(const OString& rNativeFileName)
{
    // font file already known? (e.g. for ttc, synthetic, aliased fonts)
    const char* pFileName = rNativeFileName.getStr();
    FontFileList::const_iterator it = m_aFontFileList.find(pFileName);
    if (it != m_aFontFileList.end())
        return it->second.get();

    // no => create new one
    FreetypeFontFile* pFontFile = new FreetypeFontFile(rNativeFileName);
    pFileName = pFontFile->maNativeFileName.getStr();
    m_aFontFileList[pFileName].reset(pFontFile);
    return pFontFile;
}

FreetypeFontInstance::FreetypeFontInstance(const vcl::font::PhysicalFontFace& rPFF, const vcl::font::FontSelectPattern& rFSP)
    : LogicalFontInstance(rPFF, rFSP)
    , mxFreetypeFont(FreetypeManager::get().CreateFont(this))
{
}

FreetypeFontInstance::~FreetypeFontInstance()
{
}

bool FreetypeFontInstance::GetGlyphOutline(sal_GlyphId nId, basegfx::B2DPolyPolygon& rPoly, bool bVertical) const
{
    assert(mxFreetypeFont);
    if (!mxFreetypeFont)
        return false;
    return mxFreetypeFont->GetGlyphOutline(nId, rPoly, bVertical);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
