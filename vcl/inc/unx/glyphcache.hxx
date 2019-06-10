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

#ifndef INCLUDED_VCL_INC_GENERIC_GLYPHCACHE_HXX
#define INCLUDED_VCL_INC_GENERIC_GLYPHCACHE_HXX

#include <memory>
#include <freetype/config/ftheader.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <tools/gen.hxx>
#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/outdev.hxx>

#include <fontattributes.hxx>
#include <fontinstance.hxx>
#include <impfontmetricdata.hxx>

#include <unordered_map>

class FreetypeFont;
class FreetypeFontInstance;
class FreetypeFontInfo;
class FontConfigFontOptions;
class PhysicalFontCollection;
class FreetypeFont;
class SvpGcpHelper;

namespace basegfx { class B2DPolyPolygon; }
namespace vcl { struct FontCapabilities; }

class VCL_DLLPUBLIC GlyphCache final
{
public:
    explicit                GlyphCache();
    virtual                 ~GlyphCache();

    static GlyphCache&      GetInstance();

    void                    AddFontFile(const OString& rNormalizedName,
                                int nFaceNum, int nVariantNum,
                                sal_IntPtr nFontId,
                                const FontAttributes&);

    void                    AnnounceFonts( PhysicalFontCollection* ) const;

    FreetypeFont*           CacheFont(LogicalFontInstance* pFontInstance);
    void                    UncacheFont( FreetypeFont& );
    void                    ClearFontCache();
    void                    ClearFontOptions();

private:
    static void             InitFreetype();
    void                    GarbageCollect();
    FreetypeFont*           CreateFont(LogicalFontInstance* pLogicalFont);

    // the GlyphCache's FontList matches a font request to a serverfont instance
    // the FontList key's mpFontData member is reinterpreted as integer font id
    struct IFSD_Equal{  bool operator()( const rtl::Reference<LogicalFontInstance>&, const rtl::Reference<LogicalFontInstance>& ) const; };
    struct IFSD_Hash{ size_t operator()( const rtl::Reference<LogicalFontInstance>& ) const; };
    typedef std::unordered_map<rtl::Reference<LogicalFontInstance>,std::unique_ptr<FreetypeFont>,IFSD_Hash,IFSD_Equal > FontList;
    typedef std::unordered_map<sal_IntPtr, std::unique_ptr<FreetypeFontInfo>> FontInfoList;

    FontList                maFontList;
    static constexpr sal_uLong gnMaxSize = 1500000;  // max overall cache size in bytes
    mutable sal_uLong       mnBytesUsed;
    FreetypeFont*           mpCurrentGCFont;

    FontInfoList            m_aFontInfoList;
    sal_IntPtr              m_nMaxFontId;
};

class VCL_DLLPUBLIC FreetypeFont final
{
public:
                            FreetypeFont(LogicalFontInstance* pFontInstance, FreetypeFontInfo*);
                            ~FreetypeFont();

    const OString&          GetFontFileName() const;
    int                     GetFontFaceIndex() const;
    int                     GetFontFaceVariation() const;
    bool                    TestFont() const { return mbFaceOk;}
    FT_Face                 GetFtFace() const;
    int                     GetLoadFlags() const { return (mnLoadFlags & ~FT_LOAD_IGNORE_TRANSFORM); }
    const FontConfigFontOptions* GetFontOptions() const;
    void                    ClearFontOptions();
    bool                    NeedsArtificialBold() const { return mbArtBold; }
    bool                    NeedsArtificialItalic() const { return mbArtItalic; }

    void                    GetFontMetric(ImplFontMetricDataRef const &) const;
    const unsigned char*    GetTable( const char* pName, sal_uLong* pLength ) const;
    const FontCharMapRef    GetFontCharMap() const;
    bool                    GetFontCapabilities(vcl::FontCapabilities &) const;

    bool                    GetGlyphBoundRect(sal_GlyphId, tools::Rectangle&, bool) const;
    bool                    GetGlyphOutline(sal_GlyphId, basegfx::B2DPolyPolygon&, bool) const;
    bool                    GetAntialiasAdvice() const;

    FreetypeFontInstance*   GetFontInstance() const { return mpFontInstance.get(); }

    void                    SetFontVariationsOnHBFont(hb_font_t* pHbFace) const;
private:
    friend class GlyphCache;
    friend class FreetypeFontInstance;
    friend class X11SalGraphics;
    friend class CairoTextRender;

    void                    AddRef() const      { ++mnRefCount; }
    long                    GetRefCount() const { return mnRefCount; }
    long                    Release() const;
    sal_uLong               GetByteCount() const { return mnBytesUsed; }

    void                    ReleaseFromGarbageCollect();

    void                    ApplyGlyphTransform(bool bVertical, FT_Glyph) const;

    rtl::Reference<FreetypeFontInstance> mpFontInstance;

    // used by GlyphCache for cache LRU algorithm
    mutable long            mnRefCount;
    mutable sal_uLong       mnBytesUsed;

    FreetypeFont*           mpPrevGCFont;
    FreetypeFont*           mpNextGCFont;

    // 16.16 fixed point values used for a rotated font
    long                    mnCos;
    long                    mnSin;

    int                     mnWidth;
    int const               mnPrioAntiAlias;
    FreetypeFontInfo*       mpFontInfo;
    FT_Int                  mnLoadFlags;
    double                  mfStretch;
    FT_FaceRec_*            maFaceFT;
    FT_SizeRec_*            maSizeFT;

    mutable std::unique_ptr<FontConfigFontOptions> mxFontOptions;

    bool                    mbFaceOk;
    bool                    mbArtItalic;
    bool                    mbArtBold;
};

#endif // INCLUDED_VCL_INC_GENERIC_GLYPHCACHE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
