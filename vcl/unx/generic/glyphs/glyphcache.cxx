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

#include <stdlib.h>
#include <unx/freetype_glyphcache.hxx>
#include <unx/gendata.hxx>

#include <fontinstance.hxx>

#include <rtl/ustring.hxx>
#include <sal/log.hxx>

FreetypeManager::FreetypeManager()
    : m_nMaxFontId(0)
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

FreetypeFontInstance::FreetypeFontInstance(const PhysicalFontFace& rPFF, const FontSelectPattern& rFSP)
    : LogicalFontInstance(rPFF, rFSP)
    , mxFreetypeFont(FreetypeManager::get().CreateFont(this))
{
}

FreetypeFontInstance::~FreetypeFontInstance()
{
}

static hb_blob_t* getFontTable(hb_face_t* /*face*/, hb_tag_t nTableTag, void* pUserData)
{
    char pTagName[5];
    LogicalFontInstance::DecodeOpenTypeTag( nTableTag, pTagName );

    sal_uLong nLength = 0;
    FreetypeFontInstance* pFontInstance = static_cast<FreetypeFontInstance*>( pUserData );
    FreetypeFont& rFont = pFontInstance->GetFreetypeFont();
    const char* pBuffer = reinterpret_cast<const char*>(
        rFont.GetTable(pTagName, &nLength) );

    hb_blob_t* pBlob = nullptr;
    if (pBuffer != nullptr)
        pBlob = hb_blob_create(pBuffer, nLength, HB_MEMORY_MODE_READONLY, nullptr, nullptr);

    return pBlob;
}

hb_font_t* FreetypeFontInstance::ImplInitHbFont()
{
    hb_font_t* pRet = InitHbFont(hb_face_create_for_tables(getFontTable, this, nullptr));
    assert(mxFreetypeFont);
    mxFreetypeFont->SetFontVariationsOnHBFont(pRet);
    return pRet;
}

bool FreetypeFontInstance::ImplGetGlyphBoundRect(sal_GlyphId nId, tools::Rectangle& rRect, bool bVertical) const
{
    assert(mxFreetypeFont);
    if (!mxFreetypeFont)
        return false;
    return mxFreetypeFont->GetGlyphBoundRect(nId, rRect, bVertical);
}

bool FreetypeFontInstance::GetGlyphOutline(sal_GlyphId nId, basegfx::B2DPolyPolygon& rPoly, bool bVertical) const
{
    assert(mxFreetypeFont);
    if (!mxFreetypeFont)
        return false;
    return mxFreetypeFont->GetGlyphOutline(nId, rPoly, bVertical);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
