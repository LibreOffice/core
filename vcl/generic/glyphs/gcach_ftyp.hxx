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

#ifndef _SV_GCACHFTYP_HXX
#define _SV_GCACHFTYP_HXX

#include "generic/glyphcache.hxx"

#include <rtl/textcvt.h>

#include <config_graphite.h>
#if ENABLE_GRAPHITE
class GraphiteFaceWrapper;
#endif

// -----------------------------------------------------------------------

// FtFontFile has the responsibility that a font file is only mapped once.
// (#86621#) the old directly ft-managed solution caused it to be mapped
// in up to nTTC*nSizes*nOrientation*nSynthetic times
class FtFontFile
{
public:
    static FtFontFile*      FindFontFile( const OString& rNativeFileName );

    bool                    Map();
    void                    Unmap();

    const unsigned char*    GetBuffer() const { return mpFileMap; }
    int                     GetFileSize() const { return mnFileSize; }
    const OString*   GetFileName() const { return &maNativeFileName; }
    int                     GetLangBoost() const { return mnLangBoost; }

private:
                            FtFontFile( const OString& rNativeFileName );

    const OString    maNativeFileName;
    const unsigned char*    mpFileMap;
    int                     mnFileSize;
    int                     mnRefCount;
    int                     mnLangBoost;
};

// -----------------------------------------------------------------------

// FtFontInfo corresponds to an unscaled font face
class FtFontInfo
{
public:
                           FtFontInfo( const ImplDevFontAttributes&,
                               const OString& rNativeFileName,
                               int nFaceNum, sal_IntPtr nFontId, int nSynthetic,
                                const ExtraKernInfo* );
                          ~FtFontInfo();

    const unsigned char*  GetTable( const char*, sal_uLong* pLength=0 ) const;

    FT_FaceRec_*          GetFaceFT();
#if ENABLE_GRAPHITE
    GraphiteFaceWrapper*  GetGraphiteFace();
#endif
    void                  ReleaseFaceFT();

    const OString* GetFontFileName() const   { return mpFontFile->GetFileName(); }
    int                   GetFaceNum() const        { return mnFaceNum; }
    int                   GetSynthetic() const      { return mnSynthetic; }
    sal_IntPtr            GetFontId() const         { return mnFontId; }
    bool                  IsSymbolFont() const      { return maDevFontAttributes.IsSymbolFont(); }
    const ImplFontAttributes& GetFontAttributes() const { return maDevFontAttributes; }

    void                  AnnounceFont( ImplDevFontList* );

    int                   GetGlyphIndex( sal_UCS4 cChar ) const;
    void                  CacheGlyphIndex( sal_UCS4 cChar, int nGI ) const;

    bool                  GetFontCodeRanges( CmapResult& ) const;
    const ImplFontCharMap* GetImplFontCharMap( void );

    bool                  HasExtraKerning() const;
    int                   GetExtraKernPairs( ImplKernPairData** ) const;

private:
    FT_FaceRec_*    maFaceFT;
    FtFontFile*     mpFontFile;
    const int       mnFaceNum;
    int             mnRefCount;
    const int       mnSynthetic;
#if ENABLE_GRAPHITE
    bool            mbCheckedGraphite;
    GraphiteFaceWrapper * mpGraphiteFace;
#endif
    sal_IntPtr      mnFontId;
    ImplDevFontAttributes maDevFontAttributes;

    const ImplFontCharMap* mpFontCharMap;

    // cache unicode->glyphid mapping because looking it up is expensive
    // TODO: change to boost::unordered_multimap when a use case requires a m:n mapping
    typedef ::boost::unordered_map<int,int> Int2IntMap;
    mutable Int2IntMap* mpChar2Glyph;
    mutable Int2IntMap* mpGlyph2Char;
    void InitHashes() const;

    const ExtraKernInfo* mpExtraKernInfo;
};

// these two inlines are very important for performance

inline int FtFontInfo::GetGlyphIndex( sal_UCS4 cChar ) const
{
    if( !mpChar2Glyph )
        return -1;
    Int2IntMap::const_iterator it = mpChar2Glyph->find( cChar );
    if( it == mpChar2Glyph->end() )
        return -1;
    return it->second;
}

inline void FtFontInfo::CacheGlyphIndex( sal_UCS4 cChar, int nIndex ) const
{
    if( !mpChar2Glyph )
        InitHashes();
    (*mpChar2Glyph)[ cChar ] = nIndex;
    (*mpGlyph2Char)[ nIndex ] = cChar;
}

// -----------------------------------------------------------------------

class FreetypeManager
{
public:
                        FreetypeManager();
                        ~FreetypeManager();

    void                AddFontFile( const OString& rNormalizedName,
                            int nFaceNum, sal_IntPtr nFontId, const ImplDevFontAttributes&,
                            const ExtraKernInfo* );
    void                AnnounceFonts( ImplDevFontList* ) const;
    void                ClearFontList();

    ServerFont* CreateFont( const FontSelectPattern& );

private:
    typedef ::boost::unordered_map<sal_IntPtr,FtFontInfo*> FontList;
    FontList            maFontList;

    sal_IntPtr          mnMaxFontId;
};

// -----------------------------------------------------------------------

class ImplFTSFontData : public PhysicalFontFace
{
private:
    FtFontInfo*             mpFtFontInfo;
    enum { IFTSFONT_MAGIC = 0x1F150A1C };

public:
                            ImplFTSFontData( FtFontInfo*, const ImplDevFontAttributes& );

    FtFontInfo*             GetFtFontInfo() const { return mpFtFontInfo; }

    virtual ImplFontEntry*  CreateFontInstance( FontSelectPattern& ) const;
    virtual PhysicalFontFace* Clone() const   { return new ImplFTSFontData( *this ); }
    virtual sal_IntPtr      GetFontId() const { return mpFtFontInfo->GetFontId(); }

    static bool             CheckFontData( const PhysicalFontFace& r ) { return r.CheckMagic( IFTSFONT_MAGIC ); }
};

// -----------------------------------------------------------------------

#endif // _SV_GCACHFTYP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
