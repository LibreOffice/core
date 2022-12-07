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

#include <unx/glyphcache.hxx>
#include <font/PhysicalFontFace.hxx>
#include <font/LogicalFontInstance.hxx>

#include <glyphid.hxx>

// FreetypeFontFile has the responsibility that a font file is only mapped once.
// (#86621#) the old directly ft-managed solution caused it to be mapped
// in up to nTTC*nSizes*nOrientation*nSynthetic times
class FreetypeFontFile final
{
public:
    bool                    Map();
    void                    Unmap();

    const unsigned char*    GetBuffer() const { return mpFileMap; }
    int                     GetFileSize() const { return mnFileSize; }
    const OString&          GetFileName() const { return maNativeFileName; }
    int                     GetLangBoost() const { return mnLangBoost; }

private:
    friend class FreetypeManager;
    explicit                FreetypeFontFile( OString aNativeFileName );

    const OString    maNativeFileName;
    unsigned char*   mpFileMap;
    int                     mnFileSize;
    int                     mnRefCount;
    int                     mnLangBoost;
};

// FreetypeFontInfo corresponds to an unscaled font face
class FreetypeFontInfo final
{
public:
    ~FreetypeFontInfo();

    FT_FaceRec_*          GetFaceFT();
    void                  ReleaseFaceFT();

    const OString&        GetFontFileName() const   { return mpFontFile->GetFileName(); }
    int                   GetFontFaceIndex() const  { return mnFaceNum; }
    int                   GetFontFaceVariation() const  { return mnFaceVariation; }
    sal_IntPtr            GetFontId() const         { return mnFontId; }
    const FontAttributes& GetFontAttributes() const { return maDevFontAttributes; }

    void                  AnnounceFont( vcl::font::PhysicalFontCollection* );

private:
    friend class FreetypeManager;
    explicit FreetypeFontInfo(FontAttributes , FreetypeFontFile* const pFontFile,
                              int nFaceNum, int nFaceVariation, sal_IntPtr nFontId);

    FT_FaceRec_*    maFaceFT;
    FreetypeFontFile* const mpFontFile;
    const int       mnFaceNum;
    const int       mnFaceVariation;
    int             mnRefCount;
    sal_IntPtr      mnFontId;
    FontAttributes  maDevFontAttributes;

};

class FreetypeFontFace : public vcl::font::PhysicalFontFace
{
private:
    FreetypeFontInfo*             mpFreetypeFontInfo;

public:
                            FreetypeFontFace( FreetypeFontInfo*, const FontAttributes& );

    virtual rtl::Reference<LogicalFontInstance> CreateFontInstance(const vcl::font::FontSelectPattern&) const override;
    virtual sal_IntPtr      GetFontId() const override { return mpFreetypeFontInfo->GetFontId(); }

    virtual hb_face_t* GetHbFace() const override;
    virtual hb_blob_t* GetHbTable(hb_tag_t nTag) const override;

    const std::vector<hb_variation_t>& GetVariations(const LogicalFontInstance&) const override;
};

class SAL_DLLPUBLIC_RTTI FreetypeFontInstance final : public LogicalFontInstance
{
    friend rtl::Reference<LogicalFontInstance> FreetypeFontFace::CreateFontInstance(const vcl::font::FontSelectPattern&) const;

    std::unique_ptr<FreetypeFont> mxFreetypeFont;

    virtual bool ImplGetGlyphBoundRect(sal_GlyphId, tools::Rectangle&, bool) const override;

    explicit FreetypeFontInstance(const vcl::font::PhysicalFontFace& rPFF, const vcl::font::FontSelectPattern& rFSP);

public:
    virtual ~FreetypeFontInstance() override;

    FreetypeFont& GetFreetypeFont() const { return *mxFreetypeFont; }

    virtual bool GetGlyphOutline(sal_GlyphId, basegfx::B2DPolyPolygon&, bool) const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
