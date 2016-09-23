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

#include <config_graphite.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <tools/gen.hxx>
#include <vcl/dllapi.h>
#include <vcl/metric.hxx>

#include <fontinstance.hxx>
#include <sallayout.hxx>
#include "fontattributes.hxx"
#include "impfontmetricdata.hxx"
#include "hb-ot.h"

#include <unordered_map>

class FreetypeManager;
class FreetypeFontInfo;
class GlyphData;
class GraphiteFaceWrapper;
class FontConfigFontOptions;
class PhysicalFontCollection;
class ServerFont;
class ServerFontLayout;
class ServerFontLayoutEngine;
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

    ServerFont*             CacheFont( const FontSelectPattern& );
    void                    UncacheFont( ServerFont& );
    void                    ClearFontCache();
    void                    InvalidateAllGlyphs();

private:
    friend class ServerFont;
    // used by ServerFont class only
    void                    AddedGlyph( ServerFont&, GlyphData& );
    void                    RemovingGlyph();
    void                    UsingGlyph( ServerFont&, GlyphData& );
    void                    GrowNotify();

private:
    void                    GarbageCollect();

    // the GlyphCache's FontList matches a font request to a serverfont instance
    // the FontList key's mpFontData member is reinterpreted as integer font id
    struct IFSD_Equal{  bool operator()( const FontSelectPattern&, const FontSelectPattern& ) const; };
    struct IFSD_Hash{ size_t operator()( const FontSelectPattern& ) const; };
    typedef std::unordered_map<FontSelectPattern,ServerFont*,IFSD_Hash,IFSD_Equal > FontList;

    FontList                maFontList;
    sal_uLong               mnMaxSize;      // max overall cache size in bytes
    mutable sal_uLong       mnBytesUsed;
    mutable long            mnLruIndex;
    mutable int             mnGlyphCount;
    ServerFont*             mpCurrentGCFont;

    FreetypeManager*        mpFtManager;
};

class GlyphMetric
{
public:
                            GlyphMetric() : mnAdvanceWidth(0) {}

    const Point&            GetOffset() const   { return maOffset; }
    const Size&             GetSize() const     { return maSize; }
    long                    GetCharWidth() const { return mnAdvanceWidth; }

protected:
    friend class GlyphData;
    void                    SetOffset( int nX, int nY ) { maOffset = Point( nX, nY); }
    void                    SetDelta( int nX, int nY )  { maDelta = Point( nX, nY); }
    void                    SetSize( const Size& s )    { maSize = s; }
    void                    SetCharWidth( long nW )     { mnAdvanceWidth = nW; }

private:
    long                    mnAdvanceWidth;
    Point                   maDelta;
    Point                   maOffset;
    Size                    maSize;
};

class GlyphData
{
public:
                            GlyphData() : mnLruValue(0) {}

    const GlyphMetric&      GetMetric() const           { return maGlyphMetric; }

    void                    SetSize( const Size& s)     { maGlyphMetric.SetSize( s ); }
    void                    SetOffset( int nX, int nY ) { maGlyphMetric.SetOffset( nX, nY ); }
    void                    SetDelta( int nX, int nY )  { maGlyphMetric.SetDelta( nX, nY ); }
    void                    SetCharWidth( long nW )     { maGlyphMetric.SetCharWidth( nW ); }

    void                    SetLruValue( int n ) const  { mnLruValue = n; }
    long                    GetLruValue() const         { return mnLruValue;}

private:
    GlyphMetric             maGlyphMetric;

    // used by GlyphCache for cache LRU algorithm
    mutable long            mnLruValue;
};

class VCL_DLLPUBLIC ServerFont
{
public:
                            ServerFont( const FontSelectPattern&, FreetypeFontInfo* );
    virtual                 ~ServerFont();

    const OString&          GetFontFileName() const;
    bool                    TestFont() const { return mbFaceOk;}
    FT_Face                 GetFtFace() const;
    int                     GetLoadFlags() const { return (mnLoadFlags & ~FT_LOAD_IGNORE_TRANSFORM); }
    void                    SetFontOptions(const std::shared_ptr<FontConfigFontOptions>&);
    const std::shared_ptr<FontConfigFontOptions>& GetFontOptions() const;
    bool                    NeedsArtificialBold() const { return mbArtBold; }
    bool                    NeedsArtificialItalic() const { return mbArtItalic; }

    const FontSelectPattern& GetFontSelData() const      { return maFontSelData; }

    void                    GetFontMetric( ImplFontMetricDataRef&, long& rFactor ) const;
    const unsigned char*    GetTable( const char* pName, sal_uLong* pLength );
    int                     GetEmUnits() const { return maFaceFT->units_per_EM;}
    double                  GetStretch() { return mfStretch; }
    const FontCharMapRef    GetFontCharMap() const;
    bool                    GetFontCapabilities(vcl::FontCapabilities &) const;

    GlyphData&              GetGlyphData( sal_GlyphId );
    const GlyphMetric&      GetGlyphMetric( sal_GlyphId aGlyphId )
                            { return GetGlyphData( aGlyphId ).GetMetric(); }

#if ENABLE_GRAPHITE
    GraphiteFaceWrapper* GetGraphiteFace() const;
#endif

    sal_GlyphId             GetGlyphIndex( sal_UCS4 ) const;
    sal_GlyphId             GetRawGlyphIndex( sal_UCS4, sal_UCS4 = 0 ) const;
    sal_GlyphId             FixupGlyphIndex( sal_GlyphId aGlyphId, sal_UCS4 ) const;
    bool                    GetGlyphOutline( sal_GlyphId aGlyphId, basegfx::B2DPolyPolygon& ) const;
    bool                    GetAntialiasAdvice() const;
    hb_face_t*              GetHbFace() { return mpHbFace; }
    void                    SetHbFace( hb_face_t* pHbFace ) { mpHbFace=pHbFace; }

private:
    friend class GlyphCache;
    friend class ServerFontLayout;
    friend class ServerFontInstance;
    friend class X11SalGraphics;
    friend class CairoTextRender;

    void                    AddRef() const      { ++mnRefCount; }
    long                    GetRefCount() const { return mnRefCount; }
    long                    Release() const;
    sal_uLong               GetByteCount() const { return mnBytesUsed; }

    void                    InitGlyphData( sal_GlyphId, GlyphData& ) const;
    void                    GarbageCollect( long );
    void                    ReleaseFromGarbageCollect();

    void                    ApplyGlyphTransform( int nGlyphFlags, FT_GlyphRec_*, bool ) const;
    void                    ApplyGSUB( const FontSelectPattern& );

    ServerFontLayoutEngine* GetLayoutEngine();

    typedef std::unordered_map<int,GlyphData> GlyphList;
    mutable GlyphList       maGlyphList;

    const FontSelectPattern maFontSelData;

    // used by GlyphCache for cache LRU algorithm
    mutable long            mnRefCount;
    mutable sal_uLong       mnBytesUsed;

    ServerFont*             mpPrevGCFont;
    ServerFont*             mpNextGCFont;

    // 16.16 fixed point values used for a rotated font
    long                    mnCos;
    long                    mnSin;

    int                     mnWidth;
    int                     mnPrioEmbedded;
    int                     mnPrioAntiAlias;
    int                     mnPrioAutoHint;
    FreetypeFontInfo*             mpFontInfo;
    FT_Int                  mnLoadFlags;
    double                  mfStretch;
    FT_FaceRec_*            maFaceFT;
    FT_SizeRec_*            maSizeFT;

    std::shared_ptr<FontConfigFontOptions> mxFontOptions;

    bool                    mbFaceOk;
    bool                    mbArtItalic;
    bool                    mbArtBold;
    bool                    mbUseGamma;

    typedef std::unordered_map<int,int> GlyphSubstitution;
    GlyphSubstitution       maGlyphSubstitution;

    ServerFontLayoutEngine* mpLayoutEngine;
    hb_face_t*              mpHbFace;
};

// a class for cache entries for physical font instances that are based on serverfonts
class VCL_DLLPUBLIC ServerFontInstance : public LogicalFontInstance
{
public:
                            ServerFontInstance( FontSelectPattern& );
    virtual                 ~ServerFontInstance() override;

    void                    SetServerFont(ServerFont* p);
    void                    HandleFontOptions();

private:
    ServerFont*             mpServerFont;
    std::shared_ptr<FontConfigFontOptions> mxFontOptions;
    bool                    mbGotFontOptions;

};

class VCL_DLLPUBLIC ServerFontLayout : public GenericSalLayout
{
public:
                            ServerFontLayout( ServerFont& );

    virtual bool            LayoutText( ImplLayoutArgs& ) override;
    virtual void            AdjustLayout( ImplLayoutArgs& ) override;
    virtual void            DrawText( SalGraphics& ) const override;

    void                    SetNeedFallback(
                                ImplLayoutArgs& rArgs,
                                sal_Int32 nIndex,
                                bool bRightToLeft);

    ServerFont&             GetServerFont() const   { return mrServerFont; }

    virtual std::shared_ptr<vcl::TextLayoutCache>
        CreateTextLayoutCache(OUString const&) const override;

private:
    ServerFont&             mrServerFont;
    css::uno::Reference<css::i18n::XBreakIterator> mxBreak;

                            ServerFontLayout( const ServerFontLayout& ) = delete;
                            ServerFontLayout& operator=( const ServerFontLayout& ) = delete;

};

class ServerFontLayoutEngine
{
public:
    virtual                 ~ServerFontLayoutEngine() {}

    virtual bool            Layout(ServerFontLayout&, ImplLayoutArgs&) = 0;
};

#endif // INCLUDED_VCL_INC_GENERIC_GLYPHCACHE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
