/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SV_GLYPHCACHE_HXX
#define _SV_GLYPHCACHE_HXX

#include <vcl/dllapi.h>

class GlyphCache;
class GlyphMetric;
class GlyphData;
class ServerFont;
class GlyphCachePeer;
class ServerFontLayoutEngine;
class ServerFontLayout;
class ExtraKernInfo;
struct ImplKernPairData;
class ImplFontOptions;

#include <tools/gen.hxx>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/shared_ptr.hpp>

namespace basegfx { class B2DPolyPolygon; }

class RawBitmap;

#include <outfont.hxx>
#include <impfont.hxx>

class ServerFontLayout;
#include <sallayout.hxx>

#ifdef ENABLE_GRAPHITE
class GraphiteFaceWrapper;
#endif

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

namespace vcl
{
    struct FontCapabilities;
}

// =======================================================================

class VCL_DLLPUBLIC GlyphCache
{
public:
    explicit                    GlyphCache( GlyphCachePeer& );
    /*virtual*/                 ~GlyphCache();

    static GlyphCache&      GetInstance();

    void                        AddFontFile( const rtl::OString& rNormalizedName,
                                    int nFaceNum, sal_IntPtr nFontId, const ImplDevFontAttributes&,
                                    const ExtraKernInfo* = NULL );
    void                        AnnounceFonts( ImplDevFontList* ) const;

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
    void                        RemovingGlyph( ServerFont&, GlyphData&, int nGlyphIndex );
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

// =======================================================================

class GlyphMetric
{
public:
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

// -----------------------------------------------------------------------

// the glyph specific data needed by a GlyphCachePeer is usually trivial,
// not attaching it to the corresponding GlyphData would be overkill
struct ExtGlyphData
{
    int     meInfo;
    void*   mpData;

    ExtGlyphData() : meInfo(0), mpData(NULL) {}
};

// -----------------------------------------------------------------------

class GlyphData
{
public:
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

// =======================================================================

class FtFontInfo;

class VCL_DLLPUBLIC ServerFont
{
public:
    ServerFont( const FontSelectPattern&, FtFontInfo* );
    virtual ~ServerFont();

    const ::rtl::OString* GetFontFileName() const;
    bool                TestFont() const;
    FT_Face             GetFtFace() const;
    int                 GetLoadFlags() const { return (mnLoadFlags & ~FT_LOAD_IGNORE_TRANSFORM); }
    void                SetFontOptions( boost::shared_ptr<ImplFontOptions> );
    boost::shared_ptr<ImplFontOptions> GetFontOptions() const;
    bool                NeedsArtificialBold() const { return mbArtBold; }
    bool                NeedsArtificialItalic() const { return mbArtItalic; }

    const FontSelectPattern&   GetFontSelData() const      { return maFontSelData; }

    void                FetchFontMetric( ImplFontMetricData&, long& rFactor ) const;
    sal_uLong           GetKernPairs( ImplKernPairData** ) const;
    const unsigned char* GetTable( const char* pName, sal_uLong* pLength );
    int                 GetEmUnits() const;
    const FT_Size_Metrics& GetMetricsFT() const { return maSizeFT->metrics; }
    int                 GetGlyphKernValue( int, int ) const;
    const ImplFontCharMap* GetImplFontCharMap() const;
    bool                GetFontCapabilities(vcl::FontCapabilities &) const;

    GlyphData&                  GetGlyphData( int nGlyphIndex );
    const GlyphMetric&          GetGlyphMetric( int nGlyphIndex )
                                { return GetGlyphData( nGlyphIndex ).GetMetric(); }
#ifdef ENABLE_GRAPHITE
    virtual GraphiteFaceWrapper* GetGraphiteFace() const;
#endif

    int                 GetGlyphIndex( sal_UCS4 ) const;
    int                 GetRawGlyphIndex( sal_UCS4 ) const;
    int                 FixupGlyphIndex( int nGlyphIndex, sal_UCS4 ) const;
    bool                GetGlyphOutline( int nGlyphIndex, ::basegfx::B2DPolyPolygon& ) const;
    bool                GetAntialiasAdvice( void ) const;
    bool                GetGlyphBitmap1( int nGlyphIndex, RawBitmap& ) const;
    bool                GetGlyphBitmap8( int nGlyphIndex, RawBitmap& ) const;

    void                        SetExtended( int nInfo, void* ppVoid );
    int                         GetExtInfo() { return mnExtInfo; }
    void*                       GetExtPointer() { return mpExtData; }

private:
    friend class GlyphCache;
    friend class ServerFontLayout;
    friend class X11SalGraphics;

    void                        AddRef() const      { ++mnRefCount; }
    long                        GetRefCount() const { return mnRefCount; }
    long                        Release() const;
    sal_uLong                       GetByteCount() const { return mnBytesUsed; }

    void                InitGlyphData( int nGlyphIndex, GlyphData& ) const;
    void                GarbageCollect( long );
    void                        ReleaseFromGarbageCollect();

    int                 ApplyGlyphTransform( int nGlyphFlags, FT_GlyphRec_*, bool ) const;
    bool                ApplyGSUB( const FontSelectPattern& );

    ServerFontLayoutEngine* GetLayoutEngine();

    typedef ::boost::unordered_map<int,GlyphData> GlyphList;
    mutable GlyphList           maGlyphList;

    const FontSelectPattern    maFontSelData;

    // info for GlyphcachePeer
    int                         mnExtInfo;
    void*                       mpExtData;

    // used by GlyphCache for cache LRU algorithm
    mutable long                mnRefCount;
    mutable sal_uLong               mnBytesUsed;

    ServerFont*                 mpPrevGCFont;
    ServerFont*                 mpNextGCFont;

    // 16.16 fixed point values used for a rotated font
    long                        mnCos;
    long                        mnSin;

    int                         mnZWJ;
    int                         mnZWNJ;
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
    rtl_UnicodeToTextConverter  maRecodeConverter;

    ServerFontLayoutEngine*     mpLayoutEngine;
};

// =======================================================================

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
    void           SetServerFont( ServerFont* p) { mpServerFont = p; }
    void           HandleFontOptions();
};

// =======================================================================

class VCL_DLLPUBLIC ServerFontLayout : public GenericSalLayout
{
private:
    ServerFont&     mrServerFont;

    // enforce proper copy semantic
    SAL_DLLPRIVATE  ServerFontLayout( const ServerFontLayout& );
    SAL_DLLPRIVATE  ServerFontLayout& operator=( const ServerFontLayout& );

public:
                    ServerFontLayout( ServerFont& );
    virtual bool    LayoutText( ImplLayoutArgs& );
    virtual void    AdjustLayout( ImplLayoutArgs& );
    virtual void    DrawText( SalGraphics& ) const;
    ServerFont&     GetServerFont() const   { return mrServerFont; }
};

// =======================================================================

class ServerFontLayoutEngine
{
public:
    virtual         ~ServerFontLayoutEngine() {}
    virtual bool    layout(ServerFontLayout&, ImplLayoutArgs&) = 0;
};

// =======================================================================

class GlyphCachePeer
{
protected:
                    GlyphCachePeer() : mnBytesUsed(0) {}
    virtual         ~GlyphCachePeer() {}

public:
    sal_Int32       GetByteCount() const { return mnBytesUsed; }
    virtual void    RemovingFont( ServerFont& ) {}
    virtual void    RemovingGlyph( ServerFont&, GlyphData&, int ) {}

protected:
    sal_Int32       mnBytesUsed;
};

// =======================================================================

class VCL_DLLPUBLIC RawBitmap
{
public:
                    RawBitmap();
                    ~RawBitmap();
    bool            Rotate( int nAngle );

public:
    unsigned char*  mpBits;
    sal_uLong           mnAllocated;

    sal_uLong           mnWidth;
    sal_uLong           mnHeight;

    sal_uLong           mnScanlineSize;
    sal_uLong           mnBitCount;

    int             mnXOffset;
    int             mnYOffset;
};

// =======================================================================

inline void ServerFont::SetExtended( int nInfo, void* pVoid )
{
    mnExtInfo = nInfo;
    mpExtData = pVoid;
}

// =======================================================================

// ExtraKernInfo allows an on-demand query of extra kerning info #i29881#
// The kerning values have to be scaled to match the font size before use
class VCL_DLLPUBLIC ExtraKernInfo
{
public:
    ExtraKernInfo( sal_IntPtr nFontId );
    virtual ~ExtraKernInfo() {}

    int     GetUnscaledKernPairs( ImplKernPairData** ) const;
    int     GetUnscaledKernValue( sal_Unicode cLeft, sal_Unicode cRight ) const;

protected:
    mutable bool mbInitialized;
    virtual void Initialize() const = 0;

protected:
    sal_IntPtr     mnFontId;

    // container to map a unicode pair to an unscaled kerning value
    struct PairEqual{ int operator()(const ImplKernPairData& rA, const ImplKernPairData& rB) const
                          { return (rA.mnChar1 == rB.mnChar1) && (rA.mnChar2 == rB.mnChar2); } };
    struct PairHash{ int operator()(const ImplKernPairData& rA) const
                         { return (rA.mnChar1) * 256 ^ rA.mnChar2; } };
    typedef boost::unordered_set< ImplKernPairData, PairHash, PairEqual > UnicodeKernPairs;
    mutable UnicodeKernPairs maUnicodeKernPairs;
};

// =======================================================================

#endif // _SV_GLYPHCACHE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
