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
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <tools/gen.hxx>
#include <vcl/dllapi.h>
#include <vcl/metric.hxx>

#include <sallayout.hxx>
#include <fontattributes.hxx>
#include <fontselect.hxx>
#include <impfontmetricdata.hxx>
#include <hb-ot.h>

#include <unordered_map>

class FreetypeManager;
class FreetypeFontInstance;
class FreetypeFontInfo;
class GlyphData;
class FontConfigFontOptions;
class PhysicalFontCollection;
class FreetypeFont;
class SvpGcpHelper;

namespace basegfx { class B2DPolyPolygon; }
namespace vcl { struct FontCapabilities; }

class VCL_DLLPUBLIC GlyphCache
{
public:
    explicit                GlyphCache();
    virtual                 ~GlyphCache();

    static GlyphCache&      GetInstance();

    void                    AddFontFile(
                                const OString& rNormalizedName,
                                int nFaceNum, sal_IntPtr nFontId,
                                const FontAttributes&);

    void                    AnnounceFonts( PhysicalFontCollection* ) const;

    FreetypeFont*           CacheFont( const FontSelectPattern& );
    void                    UncacheFont( FreetypeFont& );
    void                    ClearFontCache();
    void                    InvalidateAllGlyphs();
    void                    ClearFontOptions();

private:
    friend class FreetypeFont;
    // used by FreetypeFont class only
    void                    AddedGlyph( GlyphData& );
    void                    RemovingGlyph();
    void                    UsingGlyph( GlyphData const & );

private:
    void                    GarbageCollect();

    // the GlyphCache's FontList matches a font request to a serverfont instance
    // the FontList key's mpFontData member is reinterpreted as integer font id
    struct IFSD_Equal{  bool operator()( const FontSelectPattern&, const FontSelectPattern& ) const; };
    struct IFSD_Hash{ size_t operator()( const FontSelectPattern& ) const; };
    typedef std::unordered_map<FontSelectPattern,FreetypeFont*,IFSD_Hash,IFSD_Equal > FontList;

    FontList                maFontList;
    sal_uLong               mnMaxSize;      // max overall cache size in bytes
    mutable sal_uLong       mnBytesUsed;
    mutable long            mnLruIndex;
    mutable int             mnGlyphCount;
    FreetypeFont*           mpCurrentGCFont;

    std::unique_ptr<FreetypeManager>  mpFtManager;
};

class GlyphData
{
public:
                            GlyphData() : mnLruValue(0) {}

    const tools::Rectangle&        GetBoundRect() const        { return maBoundRect; }
    void                    SetBoundRect(tools::Rectangle r)   { maBoundRect = r; }

    void                    SetLruValue( int n ) const  { mnLruValue = n; }
    long                    GetLruValue() const         { return mnLruValue;}

private:
    tools::Rectangle               maBoundRect;

    // used by GlyphCache for cache LRU algorithm
    mutable long            mnLruValue;
};

class VCL_DLLPUBLIC FreetypeFont final
{
public:
                            FreetypeFont( const FontSelectPattern&, FreetypeFontInfo* );
                            ~FreetypeFont();

    const OString&          GetFontFileName() const;
    int                     GetFontFaceIndex() const;
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

    const tools::Rectangle&        GetGlyphBoundRect(const GlyphItem& rGlyph);
    bool                    GetGlyphOutline(const GlyphItem& rGlyph, basegfx::B2DPolyPolygon&) const;
    bool                    GetAntialiasAdvice() const;

    FreetypeFontInstance*   GetFontInstance() const { return mpFontInstance.get(); }

private:
    friend class GlyphCache;
    friend class FreetypeFontInstance;
    friend class X11SalGraphics;
    friend class CairoTextRender;

    void                    AddRef() const      { ++mnRefCount; }
    long                    GetRefCount() const { return mnRefCount; }
    long                    Release() const;
    sal_uLong               GetByteCount() const { return mnBytesUsed; }

    void                    InitGlyphData(const GlyphItem&, GlyphData&) const;
    void                    GarbageCollect( long );
    void                    ReleaseFromGarbageCollect();

    void                    ApplyGlyphTransform(bool bVertical, FT_Glyph) const;

    typedef std::unordered_map<int,GlyphData> GlyphList;
    mutable GlyphList       maGlyphList;

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
    int                     mnPrioAntiAlias;
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
