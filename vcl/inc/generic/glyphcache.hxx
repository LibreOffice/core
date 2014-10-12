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

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include <basebmp/bitmapdevice.hxx>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <tools/gen.hxx>
#include <vcl/dllapi.h>
#include <vcl/metric.hxx>

#include <outfont.hxx>
#include <sallayout.hxx>

class FtFontInfo;
class GlyphCachePeer;
class GlyphData;
class GraphiteFaceWrapper;
class ImplFontOptions;
class PhysicalFontCollection;
class RawBitmap;
class ServerFont;
class ServerFontLayout;
class ServerFontLayoutEngine;
namespace basegfx { class B2DPolyPolygon; }
namespace vcl { struct FontCapabilities; }

class VCL_DLLPUBLIC GlyphCache
{
public:
    explicit                    GlyphCache( GlyphCachePeer& );
                                ~GlyphCache();

    static GlyphCache&      GetInstance();

    void                        AddFontFile( const OString& rNormalizedName,
                                             int nFaceNum, sal_IntPtr nFontId,
                                             const ImplDevFontAttributes&);
    void                        AnnounceFonts( PhysicalFontCollection* ) const;

    ServerFont*                 CacheFont( const FontSelectPattern& );
    void                        UncacheFont( ServerFont& );
    void                        ClearFontCache();
    void                        InvalidateAllGlyphs();

protected:
    GlyphCachePeer&             mrPeer;

private:
    friend class ServerFont;
    // used by ServerFont class only
    void                        AddedGlyph( ServerFont&, GlyphData& );
    void                        RemovingGlyph( GlyphData& );
    void                        UsingGlyph( ServerFont&, GlyphData& );
    void                        GrowNotify();

private:
    void                        GarbageCollect();

    // the GlyphCache's FontList matches a font request to a serverfont instance
    // the FontList key's mpFontData member is reinterpreted as integer font id
    struct IFSD_Equal{  bool operator()( const FontSelectPattern&, const FontSelectPattern& ) const; };
    struct IFSD_Hash{ size_t operator()( const FontSelectPattern& ) const; };
    typedef ::boost::unordered_map<FontSelectPattern,ServerFont*,IFSD_Hash,IFSD_Equal > FontList;
    FontList                    maFontList;
    sal_uLong                       mnMaxSize;      // max overall cache size in bytes
    mutable sal_uLong               mnBytesUsed;
    mutable long                mnLruIndex;
    mutable int                 mnGlyphCount;
    ServerFont*                 mpCurrentGCFont;

    class FreetypeManager*      mpFtManager;
};

class GlyphMetric
{
public:
    GlyphMetric() : mnAdvanceWidth(0) {}

    Point                   GetOffset() const   { return maOffset; }
    Point                   GetDelta() const    { return maDelta; }
    Size                    GetSize() const     { return maSize; }
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

// the glyph specific data needed by a GlyphCachePeer is usually trivial,
// not attaching it to the corresponding GlyphData would be overkill;
// this is currently only used by the headless (aka svp) plugin, where meInfo is
// basebmp::Format and mpData is SvpGcpHelper*
struct ExtGlyphData
{
    int     meInfo;
    void*   mpData;

    ExtGlyphData() : meInfo(0), mpData(NULL) {}
};

class GlyphData
{
public:
    GlyphData() : mnLruValue(0) {}

    const GlyphMetric&      GetMetric() const           { return maMetric; }
    Size                    GetSize() const             { return maMetric.GetSize(); }

    void                    SetSize( const Size& s)     { maMetric.SetSize( s ); }
    void                    SetOffset( int nX, int nY ) { maMetric.SetOffset( nX, nY ); }
    void                    SetDelta( int nX, int nY )  { maMetric.SetDelta( nX, nY ); }
    void                    SetCharWidth( long nW )     { maMetric.SetCharWidth( nW ); }

    void                    SetLruValue( int n ) const  { mnLruValue = n; }
    long                    GetLruValue() const         { return mnLruValue;}

    ExtGlyphData&           ExtDataRef()                { return maExtData; }
    const ExtGlyphData&     ExtDataRef() const          { return maExtData; }

private:
    GlyphMetric             maMetric;
    ExtGlyphData            maExtData;

    // used by GlyphCache for cache LRU algorithm
    mutable long            mnLruValue;
};

class VCL_DLLPUBLIC ServerFont
{
public:
    ServerFont( const FontSelectPattern&, FtFontInfo* );
    virtual ~ServerFont();

    const OString&      GetFontFileName() const;
    bool                TestFont() const { return mbFaceOk;}
    FT_Face             GetFtFace() const;
    int                 GetLoadFlags() const { return (mnLoadFlags & ~FT_LOAD_IGNORE_TRANSFORM); }
    void                SetFontOptions( boost::shared_ptr<ImplFontOptions> );
    boost::shared_ptr<ImplFontOptions> GetFontOptions() const;
    bool                NeedsArtificialBold() const { return mbArtBold; }
    bool                NeedsArtificialItalic() const { return mbArtItalic; }

    const FontSelectPattern&   GetFontSelData() const      { return maFontSelData; }

    void                FetchFontMetric( ImplFontMetricData&, long& rFactor ) const;
    const unsigned char* GetTable( const char* pName, sal_uLong* pLength );
    int                 GetEmUnits() const { return maFaceFT->units_per_EM;}
    const FT_Size_Metrics& GetMetricsFT() const { return maSizeFT->metrics; }
    const FontCharMapPtr GetFontCharMap() const;
    bool                GetFontCapabilities(vcl::FontCapabilities &) const;

    GlyphData&                  GetGlyphData( sal_GlyphId );
    const GlyphMetric&          GetGlyphMetric( sal_GlyphId aGlyphId )
                                { return GetGlyphData( aGlyphId ).GetMetric(); }
#if ENABLE_GRAPHITE
    virtual GraphiteFaceWrapper* GetGraphiteFace() const;
#endif

    sal_GlyphId         GetGlyphIndex( sal_UCS4 ) const;
    sal_GlyphId         GetRawGlyphIndex( sal_UCS4, sal_UCS4 = 0 ) const;
    sal_GlyphId         FixupGlyphIndex( sal_GlyphId aGlyphId, sal_UCS4 ) const;
    bool                GetGlyphOutline( sal_GlyphId aGlyphId, ::basegfx::B2DPolyPolygon& ) const;
    bool                GetAntialiasAdvice( void ) const;
    bool                GetGlyphBitmap1( sal_GlyphId aGlyphId, RawBitmap& ) const;
    bool                GetGlyphBitmap8( sal_GlyphId aGlyphId, RawBitmap& ) const;

private:
    friend class GlyphCache;
    friend class ServerFontLayout;
    friend class ImplServerFontEntry;
    friend class X11SalGraphics;

    void                        AddRef() const      { ++mnRefCount; }
    long                        GetRefCount() const { return mnRefCount; }
    long                        Release() const;
    sal_uLong                       GetByteCount() const { return mnBytesUsed; }

    void                InitGlyphData( sal_GlyphId, GlyphData& ) const;
    void                GarbageCollect( long );
    void                        ReleaseFromGarbageCollect();

    int                 ApplyGlyphTransform( int nGlyphFlags, FT_GlyphRec_*, bool ) const;
    bool                ApplyGSUB( const FontSelectPattern& );

    ServerFontLayoutEngine* GetLayoutEngine();

    typedef ::boost::unordered_map<int,GlyphData> GlyphList;
    mutable GlyphList           maGlyphList;

    const FontSelectPattern    maFontSelData;

    // used by GlyphCache for cache LRU algorithm
    mutable long                mnRefCount;
    mutable sal_uLong               mnBytesUsed;

    ServerFont*                 mpPrevGCFont;
    ServerFont*                 mpNextGCFont;

    // 16.16 fixed point values used for a rotated font
    long                        mnCos;
    long                        mnSin;

    bool                        mbCollectedZW;

    int                         mnWidth;
    int                         mnPrioEmbedded;
    int                         mnPrioAntiAlias;
    int                         mnPrioAutoHint;
    FtFontInfo*                 mpFontInfo;
    FT_Int                      mnLoadFlags;
    double                      mfStretch;
    FT_FaceRec_*                maFaceFT;
    FT_SizeRec_*                maSizeFT;

    boost::shared_ptr<ImplFontOptions> mpFontOptions;

    bool                        mbFaceOk;
    bool            mbArtItalic;
    bool            mbArtBold;
    bool            mbUseGamma;

    typedef ::boost::unordered_map<int,int> GlyphSubstitution;
    GlyphSubstitution           maGlyphSubstitution;

    ServerFontLayoutEngine*     mpLayoutEngine;
};

// a class for cache entries for physical font instances that are based on serverfonts
class VCL_DLLPUBLIC ImplServerFontEntry : public ImplFontEntry
{
private:
    ServerFont*    mpServerFont;
    boost::shared_ptr<ImplFontOptions> mpFontOptions;
    bool           mbGotFontOptions;

public:
                   ImplServerFontEntry( FontSelectPattern& );
    virtual        ~ImplServerFontEntry();
    void           SetServerFont(ServerFont* p);
    void           HandleFontOptions();
};

class VCL_DLLPUBLIC ServerFontLayout : public GenericSalLayout
{
private:
    ServerFont&     mrServerFont;
    com::sun::star::uno::Reference<com::sun::star::i18n::XBreakIterator> mxBreak;

    // enforce proper copy semantic
    SAL_DLLPRIVATE  ServerFontLayout( const ServerFontLayout& );
    SAL_DLLPRIVATE  ServerFontLayout& operator=( const ServerFontLayout& );

public:
                    ServerFontLayout( ServerFont& );
    virtual bool    LayoutText( ImplLayoutArgs& ) SAL_OVERRIDE;
    virtual void    AdjustLayout( ImplLayoutArgs& ) SAL_OVERRIDE;
    virtual void    DrawText( SalGraphics& ) const SAL_OVERRIDE;
    void            setNeedFallback(ImplLayoutArgs& rArgs, sal_Int32 nIndex,
                        bool bRightToLeft);

    ServerFont&     GetServerFont() const   { return mrServerFont; }
};

class ServerFontLayoutEngine
{
public:
    virtual         ~ServerFontLayoutEngine() {}
    virtual bool    layout(ServerFontLayout&, ImplLayoutArgs&) = 0;
};

class GlyphCachePeer
{
protected:
                    GlyphCachePeer() : mnBytesUsed(0) {}
    virtual         ~GlyphCachePeer() {}

public:
    sal_Int32       GetByteCount() const { return mnBytesUsed; }
    virtual void    RemovingFont( ServerFont& ) {}
    virtual void    RemovingGlyph( GlyphData& ) {}

protected:
    sal_Int32       mnBytesUsed;
};

class VCL_DLLPUBLIC RawBitmap
{
public:
                    RawBitmap();
                    ~RawBitmap();
    bool            Rotate( int nAngle );

public:
    basebmp::RawMemorySharedArray mpBits;
    sal_uLong           mnAllocated;

    sal_uLong           mnWidth;
    sal_uLong           mnHeight;

    sal_uLong           mnScanlineSize;
    sal_uLong           mnBitCount;

    int             mnXOffset;
    int             mnYOffset;
};

#endif // INCLUDED_VCL_INC_GENERIC_GLYPHCACHE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
