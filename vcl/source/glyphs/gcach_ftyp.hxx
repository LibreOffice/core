/*************************************************************************
 *
 *  $RCSfile: gcach_ftyp.hxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-17 15:18:54 $
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

typedef int FT_Int;
struct FT_GlyphRec_;
class FreetypeServerFont;

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
    int                     GetSize() const { return mnFileSize; }
    const ::rtl::OString*   GetFileName() const { return &maNativeFileName; }

private:
                            FtFontFile( const ::rtl::OString& rNativeFileName );

    const ::rtl::OString    maNativeFileName;
    const unsigned char*    mpFileMap;
    int                     mnFileSize;
    int                     mnRefCount;
};

// -----------------------------------------------------------------------

// FtFontInfo corresponds to an unscaled font face
class FtFontInfo
{
public:
    FtFontInfo( const ImplFontData&, const ::rtl::OString&,
        int nFaceNum, int nFontId, int nSynthetic );

    bool                  MapFile() { return mpFontFile->Map(); }
    void                  Unmap() { mpFontFile->Unmap(); }

    int                   GetFileSize() const { return mpFontFile->GetSize(); }
    const unsigned char*  GetBuffer() const { return mpFontFile->GetBuffer(); }
    const unsigned char*  GetTable( const char*, ULONG* pLength=0 ) const;

    const ::rtl::OString* GetFontFileName() const { return mpFontFile->GetFileName(); }
    const ImplFontData&   GetFontData() const { return maFontData; }
    int                   GetFaceNum() const { return mnFaceNum; }
    int                   GetSynthetic() const { return mnSynthetic; }

    int                   GetFontId() const { return mnFontId; }
    void                  SetFontId( int nFontId ) { mnFontId = nFontId; }

    int                   GetGlyphIndex( sal_Unicode cChar ) const;
    void                  CacheGlyphIndex( sal_Unicode cChar, int nGI ) const;

private:
    ImplFontData    maFontData;
    FtFontFile*     mpFontFile;
    const int       mnFaceNum;
    const int       mnSynthetic;
    int             mnFontId;

    typedef ::std::hash_map<sal_Unicode,int> FIGlyphMap;
    mutable FIGlyphMap maGlyphMap;
};

// these two inlines are very important for performance

inline int FtFontInfo::GetGlyphIndex( sal_Unicode cChar ) const
{
    FIGlyphMap::const_iterator it = maGlyphMap.find( cChar );
    if( it != maGlyphMap.end() )
        return it->second;
    return -1;
}

inline void FtFontInfo::CacheGlyphIndex( sal_Unicode cChar, int nGI ) const
{
    maGlyphMap[ cChar ] = nGI;
}

// -----------------------------------------------------------------------

class FreetypeManager
{
public:
                        FreetypeManager();
                        ~FreetypeManager();

    long                AddFontDir( const String& rUrlName );
    void                AddFontFile( const rtl::OString& rNormalizedName,
                            int nFaceNum, int nFontId, const ImplFontData* );
    long                FetchFontList( ImplDevFontList* ) const;
    void                ClearFontList();

    FreetypeServerFont* CreateFont( const ImplFontSelectData& );
    void*               GetFontHandle (int nFontId);

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
    virtual int                 GetFontId() const { return mpFontInfo->GetFontId(); }
    virtual void                SetFontId( int nFontId ) { mpFontInfo->SetFontId( nFontId ); }
    virtual bool                TestFont() const;

    virtual void                FetchFontMetric( ImplFontMetricData&, long& rFactor ) const;

    virtual int                 GetGlyphIndex( sal_Unicode ) const;
    int                         GetRawGlyphIndex( sal_Unicode ) const;
    int                         FixupGlyphIndex( int nGlyphIndex, sal_Unicode ) const;

    virtual bool                GetAntialiasAdvice( void ) const;
    virtual bool                GetGlyphBitmap1( int nGlyphIndex, RawBitmap& ) const;
    virtual bool                GetGlyphBitmap8( int nGlyphIndex, RawBitmap& ) const;
    virtual bool                GetGlyphOutline( int nGlyphIndex, PolyPolygon& ) const;
    virtual int                 GetGlyphKernValue( int nLeftGlyph, int nRightGlyph ) const;

    const unsigned char*        GetTable( const char* pName, ULONG* pLength )
                                { return mpFontInfo->GetTable( pName, pLength ); }
    int                         GetEmUnits() const;

protected:
friend GlyphCache;

    int                         ApplyGlyphTransform( int nGlyphFlags, FT_GlyphRec_* ) const;
    virtual void                InitGlyphData( int nGlyphIndex, GlyphData& ) const;
    virtual ULONG               GetKernPairs( ImplKernPairData** ) const;
    virtual ULONG               GetFontCodeRanges( sal_uInt32* pCodes ) const;
    bool                        ApplyGSUB( const ImplFontSelectData& );
    virtual ServerFontLayoutEngine* GetLayoutEngine();

private:
    int                         mnWidth;
    struct FT_FaceRec_*         maFaceFT;
    FtFontInfo*                 mpFontInfo;
    FT_Int                      mnLoadFlags;
    double                      mfStretch;

    typedef ::std::hash_map<int,int> GlyphSubstitution;
    GlyphSubstitution           maGlyphSubstitution;
    rtl_UnicodeToTextConverter  maRecodeConverter;


    ServerFontLayoutEngine*     mpLayoutEngine;
};

// -----------------------------------------------------------------------

#endif // _SV_GCACHFTYP_HXX
