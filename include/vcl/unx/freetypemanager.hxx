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

#include <unordered_map>

class FontAttributes;
class FreetypeFont;
class FreetypeFontFile;
class FreetypeFontInfo;
class FreetypeFontInstance;

namespace vcl
{
namespace font
{
class PhysicalFontCollection;
}
}

/**
  * The FreetypeManager caches various aspects of Freetype fonts
  *
  * It mainly consists of two std::unordered_map lists, which hold the items of the cache.
  *
  * They form kind of a tree, with FreetypeFontFile as the roots, referenced by multiple FreetypeFontInfo
  * entries, which are referenced by the FreetypeFont items.
  *
  * All of these items have reference counters, but these don't control the items life-cycle, but that of
  * the managed resources.
  *
  * The respective resources are:
  *   FreetypeFontFile = holds the mmapped font file, as long as it's used by any FreetypeFontInfo.
  *   FreetypeFontInfo = holds the FT_FaceRec_ object, as long as it's used by any FreetypeFont.
  *   FreetypeFont     = holds the FT_SizeRec_ and is owned by a FreetypeFontInstance
  *
  * FreetypeFontInfo therefore is embedded in the Freetype subclass of PhysicalFontFace.
  * FreetypeFont is owned by FreetypeFontInstance, the Freetype subclass of LogicalFontInstance.
  *
  * Nowadays there is not really a reason to have separate files for the classes, as the FreetypeManager
  * is just about handling of Freetype based fonts, not some abstract glyphs.
  **/
class VCL_DLLPUBLIC FreetypeManager final
{
public:
    ~FreetypeManager();

    static FreetypeManager& get();

    void AddFontFile(const OString& rNormalizedName, int nFaceNum, int nVariantNum,
                     sal_IntPtr nFontId, const FontAttributes&);

    void AnnounceFonts(vcl::font::PhysicalFontCollection*) const;

    void ClearFontCache();

    FreetypeFont* CreateFont(FreetypeFontInstance* pLogicalFont);

    FreetypeFontFile* FindFontFile(const OString& rNativeFileName);

    static void MapFontFile(FreetypeFontFile* pFontFile);

private:
    // to access the constructor (can't use InitFreetypeManager function, because it's private?!)
    friend class GenericUnixSalData;
    explicit FreetypeManager();

    static void InitFreetype();

    typedef std::unordered_map<sal_IntPtr, std::shared_ptr<FreetypeFontInfo>> FontInfoList;
    typedef std::unordered_map<const char*, std::unique_ptr<FreetypeFontFile>, rtl::CStringHash,
                               rtl::CStringEqual>
        FontFileList;

    FontInfoList m_aFontInfoList;
    sal_IntPtr m_nMaxFontId;

    FontFileList m_aFontFileList;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
