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
#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/metric.hxx>

#include <sallayout.hxx>
#include <fontattributes.hxx>
#include <fontselect.hxx>
#include <impfontmetricdata.hxx>
#include <hb-ot.h>

#include <unordered_map>

class FreetypeFont;
class FreetypeFontFile;
class FreetypeFontInstance;
class FreetypeFontInfo;
class FontConfigFontOptions;
class PhysicalFontCollection;
class FreetypeFont;
class SvpGcpHelper;

namespace basegfx { class B2DPolyPolygon; }
namespace vcl { struct FontCapabilities; }

 /**
  * The GlyphCache caches various aspects of Freetype fonts
  *
  * It mainly consists of three std::unordered_map lists, which hold the items of the cache.
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
  *   FreetypeFont     = holds the FT_SizeRec_.
  *
  * FreetypeFontInfo therefore is embedded in the Freetype subclass of PhysicalFontFace.
  * FreetypeFont is embedded in the Freetype subclass of LogicalFontInstance.
  *
  * Nowadays there is not really a reason to have seperate files for the classes, as the GlyphCache is
  * just about handling of Freetype based fonts, not some abstract glyphs.
  *
  * One additional note: the byte-size based garbage collection of unused fonts can currently be assumed
  * to be broken. Since the move of the glyph rect cache into the ImplFontCache, so it can be used by all
  * platforms, it just takes too long to kick-in, as there is no real accounting left.
  **/
class VCL_DLLPUBLIC GlyphCache final
{
public:
    ~GlyphCache();

    static GlyphCache&      GetInstance();

    void                    AddFontFile(const OString& rNormalizedName,
                                int nFaceNum, int nVariantNum,
                                sal_IntPtr nFontId,
                                const FontAttributes&);

    void                    AnnounceFonts( PhysicalFontCollection* ) const;

    FreetypeFont*           CacheFont(LogicalFontInstance* pFontInstance);
    void                    UncacheFont( FreetypeFont& );

    /** Try to GarbageCollect an explicit logical font
     *
     * This should just be called from the ~ImplFontCache destructor, which holds the mapping of the
     * FontSelectPattern to the LogicalFontInstance per OutputDevice. All other users should just
     * call CacheFont and UncacheFont correctly. When the ImplFontCache is destroyed with its
     * OutputDevice, we can safely garbage collection its unused entries, as these can't be reused.
     *
     * It's always safe to call this, as it just ignores the used bytes when considering a font for
     * garbage collection, which normally keeps unreferenced fonts alive.
     **/
    void TryGarbageCollectFont(LogicalFontInstance*);

    void                    ClearFontCache();
    void                    ClearFontOptions();

private:
    // to access the constructor (can't use InitFreetypeManager function, because it's private?!)
    friend class GenericUnixSalData;
    explicit GlyphCache();

    static void             InitFreetype();
    void                    GarbageCollect();
    FreetypeFont*           CreateFont(LogicalFontInstance* pLogicalFont);
    FreetypeFontFile* FindFontFile(const OString& rNativeFileName);

    // the GlyphCache's FontList matches a font request to a serverfont instance
    // the FontList key's mpFontData member is reinterpreted as integer font id
    struct IFSD_Equal{  bool operator()( const rtl::Reference<LogicalFontInstance>&, const rtl::Reference<LogicalFontInstance>& ) const; };
    struct IFSD_Hash{ size_t operator()( const rtl::Reference<LogicalFontInstance>& ) const; };
    typedef std::unordered_map<rtl::Reference<LogicalFontInstance>,std::unique_ptr<FreetypeFont>,IFSD_Hash,IFSD_Equal > FontList;
    typedef std::unordered_map<sal_IntPtr, std::unique_ptr<FreetypeFontInfo>> FontInfoList;
    typedef std::unordered_map<const char*, std::unique_ptr<FreetypeFontFile>, rtl::CStringHash, rtl::CStringEqual> FontFileList;

    FontList                maFontList;
    static constexpr sal_uLong gnMaxSize = 1500000;  // max overall cache size in bytes
    mutable sal_uLong       mnBytesUsed;
    FreetypeFont*           mpCurrentGCFont;

    FontInfoList            m_aFontInfoList;
    sal_IntPtr              m_nMaxFontId;

    FontFileList            m_aFontFileList;
};

class VCL_DLLPUBLIC FreetypeFont final
{
public:
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

    // tdf#127189 FreeType <= 2.8 will fail to render stretched horizontal brace glyphs
    // in starmath at a fairly low stretch ratio. This appears fixed in 2.9 with
    // https://git.savannah.gnu.org/cgit/freetype/freetype2.git/commit/?id=91015cb41d8f56777f93394f5a60914bc0c0f330
    // "Improve complex rendering at high ppem"
    static bool             AlmostHorizontalDrainsRenderingPool();

private:
    friend class GlyphCache;
    explicit FreetypeFont(LogicalFontInstance*, FreetypeFontInfo*);

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
