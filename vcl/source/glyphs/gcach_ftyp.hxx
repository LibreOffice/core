/*************************************************************************
 *
 *  $RCSfile: gcach_ftyp.hxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 09:30:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SV_GCACHFTYP_HXX
#define _SV_GCACHFTYP_HXX

#include <glyphcache.hxx>
#include <rtl/textcvt.h>

#include <freetype/freetype.h>
class FreetypeServerFont;
struct FT_GlyphRec_;

// -----------------------------------------------------------------------

// FtFontFile has the responsibility that a font file is only mapped once.
// (#86621#) the old directly ft-managed solution caused it to be mapped
// in up to nTTC*nSizes*nOrientation*nSynthetic times
class FtFontFile
{
public:
    static FtFontFile*      FindFontFile( const ::rtl::OString& rNativeFileName );

    bool                    Map();
    void                    Unmap();

    const unsigned char*    GetBuffer() const { return mpFileMap; }
    int                     GetFileSize() const { return mnFileSize; }
    const ::rtl::OString*   GetFileName() const { return &maNativeFileName; }
    int                     GetLangBoost() const { return mnLangBoost; }

private:
                            FtFontFile( const ::rtl::OString& rNativeFileName );

    const ::rtl::OString    maNativeFileName;
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
                               const ::rtl::OString& rNativeFileName,
                               int nFaceNum, int nFontId, int nSynthetic,
                                const ExtraKernInfo* );
                          ~FtFontInfo();

    const unsigned char*  GetTable( const char*, ULONG* pLength=0 ) const;

    FT_FaceRec_*          GetFaceFT();
    void                  ReleaseFaceFT( FT_FaceRec_* );

    const ::rtl::OString* GetFontFileName() const   { return mpFontFile->GetFileName(); }
    int                   GetFaceNum() const        { return mnFaceNum; }
    int                   GetSynthetic() const      { return mnSynthetic; }
    int                   GetFontId() const         { return mnFontId; }

    bool                  IsSymbolFont() const      { return maDevFontAttributes.IsSymbolFont(); }
    const ImplFontAttributes& GetFontAttributes() const { return maDevFontAttributes; }

    void                  AnnounceFont( ImplDevFontList* );

    int                   GetGlyphIndex( sal_UCS4 cChar ) const;
    void                  CacheGlyphIndex( sal_UCS4 cChar, int nGI ) const;

    bool                  HasExtraKerning() const;
    int                   GetExtraKernPairs( ImplKernPairData** ) const;
    int                   GetExtraGlyphKernValue( int nLeftGlyph, int nRightGlyph ) const;

private:
    FT_FaceRec_*    maFaceFT;
    FtFontFile*     mpFontFile;
    const int       mnFaceNum;
    int             mnRefCount;
    const int       mnSynthetic;

    int             mnFontId;
    ImplDevFontAttributes maDevFontAttributes;

    // cache unicode->glyphid mapping because looking it up is expensive
    // TODO: change to hash_multimap when a use case requires a m:n mapping
    typedef ::std::hash_map<int,int> Int2IntMap;
    mutable Int2IntMap maChar2Glyph;
    mutable Int2IntMap maGlyph2Char;

    const ExtraKernInfo* mpExtraKernInfo;
};

// these two inlines are very important for performance

inline int FtFontInfo::GetGlyphIndex( sal_UCS4 cChar ) const
{
    Int2IntMap::const_iterator it = maChar2Glyph.find( cChar );
    if( it == maChar2Glyph.end() )
        return -1;
    return it->second;
}

inline void FtFontInfo::CacheGlyphIndex( sal_UCS4 cChar, int nIndex ) const
{
    maChar2Glyph[ cChar ] = nIndex;
    maGlyph2Char[ nIndex ] = cChar;
}

// -----------------------------------------------------------------------

class FreetypeManager
{
public:
                        FreetypeManager();
                        ~FreetypeManager();

    long                AddFontDir( const String& rUrlName );
    void                AddFontFile( const rtl::OString& rNormalizedName,
                            int nFaceNum, int nFontId, const ImplDevFontAttributes&,
                            const ExtraKernInfo* );
    void                AnnounceFonts( ImplDevFontList* ) const;
    void                ClearFontList();

    FreetypeServerFont* CreateFont( const ImplFontSelectData& );

private:
    typedef ::std::hash_map<int,FtFontInfo*> FontList;
    FontList            maFontList;

    int                 mnMaxFontId;
    int                 mnNextFontId;
};

// -----------------------------------------------------------------------

class FreetypeServerFont : public ServerFont
{
public:
                                FreetypeServerFont( const ImplFontSelectData&, FtFontInfo* );
    virtual                     ~FreetypeServerFont();

    virtual const ::rtl::OString* GetFontFileName() const { return mpFontInfo->GetFontFileName(); }
    virtual int                 GetFontFaceNum() const { return mpFontInfo->GetFaceNum(); }
    virtual bool                TestFont() const;

    virtual void                FetchFontMetric( ImplFontMetricData&, long& rFactor ) const;

    virtual int                 GetGlyphIndex( sal_UCS4 ) const;
    int                         GetRawGlyphIndex( sal_UCS4 ) const;
    int                         FixupGlyphIndex( int nGlyphIndex, sal_UCS4 ) const;

    virtual bool                GetAntialiasAdvice( void ) const;
    virtual bool                GetGlyphBitmap1( int nGlyphIndex, RawBitmap& ) const;
    virtual bool                GetGlyphBitmap8( int nGlyphIndex, RawBitmap& ) const;
    virtual bool                GetGlyphOutline( int nGlyphIndex, PolyPolygon& ) const;
    virtual int                 GetGlyphKernValue( int nLeftGlyph, int nRightGlyph ) const;
    virtual ULONG               GetKernPairs( ImplKernPairData** ) const;

    const unsigned char*        GetTable( const char* pName, ULONG* pLength )
                                { return mpFontInfo->GetTable( pName, pLength ); }
    int                         GetEmUnits() const;
    const FT_Size_Metrics&      GetMetricsFT() const { return maSizeFT->metrics; }

protected:
    friend class GlyphCache;

    int                         ApplyGlyphTransform( int nGlyphFlags, FT_GlyphRec_* ) const;
    virtual void                InitGlyphData( int nGlyphIndex, GlyphData& ) const;
    virtual ULONG               GetFontCodeRanges( sal_uInt32* pCodes ) const;
    bool                        ApplyGSUB( const ImplFontSelectData& );
    virtual ServerFontLayoutEngine* GetLayoutEngine();

private:
    int                         mnWidth;
    FtFontInfo*                 mpFontInfo;
    FT_Int                      mnLoadFlags;
    double                      mfStretch;
    FT_FaceRec_*                maFaceFT;
    FT_SizeRec_*                maSizeFT;

    typedef ::std::hash_map<int,int> GlyphSubstitution;
    GlyphSubstitution           maGlyphSubstitution;
    rtl_UnicodeToTextConverter  maRecodeConverter;

    ServerFontLayoutEngine*     mpLayoutEngine;
};

// -----------------------------------------------------------------------

class ImplFTSFontData : public ImplFontData
{
private:
    FtFontInfo*             mpFtFontInfo;
    enum { IFTSFONT_MAGIC = 0x1F150A1C };

public:
                            ImplFTSFontData( FtFontInfo*, const ImplDevFontAttributes& );
    virtual                 ~ImplFTSFontData();

    FtFontInfo*             GetFtFontInfo() const { return mpFtFontInfo; }

    virtual ImplFontEntry*  CreateFontInstance( ImplFontSelectData& ) const;
    virtual ImplFontData*   Clone() const   { return new ImplFTSFontData( *this ); }
    virtual int             GetFontId() const { return mpFtFontInfo->GetFontId(); }

    static bool             CheckFontData( const ImplFontData& r ) { return r.CheckMagic( IFTSFONT_MAGIC ); }
};

// -----------------------------------------------------------------------

#endif // _SV_GCACHFTYP_HXX
