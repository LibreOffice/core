/*************************************************************************
 *
 *  $RCSfile: gcach_ftyp.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hdu $ $Date: 2001-03-08 11:40:53 $
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

#ifndef NO_FREETYPE_FONTS

#include <glyphcache.hxx>
typedef int FT_Int;

// -----------------------------------------------------------------------

struct FtFontInfo
{
    ImplFontData    aFontData;
    ::rtl::OString  aNativeFileName;
    int             nFaceNum;
    int             nFontId;
};

// -----------------------------------------------------------------------

class FreetypeManager
{
public:
                                FreetypeManager();
                                ~FreetypeManager();

    long                        AddFontDir( const String& rNormalizedName );
    void                        AddFontFile( const String& rNormalizedName, int nFaceNum,
                                    int nFontId, const ImplFontData* );
    long                        FetchFontList( ImplDevFontList* ) const;
    void                        ClearFontList();

    class FreetypeServerFont*   CreateFont( const ImplFontSelectData& );

private:
    struct std::hash<FtFontInfo*> { size_t operator()( const FtFontInfo* ) const; };
    struct std::equal_to<FtFontInfo*> { bool operator()( const FtFontInfo*, const FtFontInfo* ) const; };
    typedef ::std::hash_multiset<FtFontInfo*> FontList;
    FontList                    maFontList;
};

// -----------------------------------------------------------------------

class FreetypeServerFont : public ServerFont
{
    public:
                                FreetypeServerFont( const ImplFontSelectData&, const FtFontInfo& );
    virtual                     ~FreetypeServerFont();

    virtual const ::rtl::OString*   GetFontFileName() const { return &mrFontInfo.aNativeFileName; }
    virtual int                 GetFontFaceNum() const { return mrFontInfo.nFaceNum; }
    virtual int                 GetFontId() const { return mrFontInfo.nFontId; }
    virtual void                SetFontId( int nFontId ) { mrFontInfo.nFontId = nFontId; }
    virtual bool                TestFont() { return (maFaceFT != NULL); }

    virtual void                FetchFontMetric( ImplFontMetricData&, long& rFactor ) const;

    virtual int                 GetGlyphIndex( sal_Unicode ) const;
    virtual bool                GetGlyphBitmap1( int nGlyphIndex, RawBitmap& ) const;
    virtual bool                GetGlyphBitmap8( int nGlyphIndex, RawBitmap& ) const;
    virtual bool                GetGlyphOutline( int nGlyphIndex, PolyPolygon& ) const;

protected:
friend GlyphCache;

    virtual void                InitGlyphData( int nGlyphIndex, GlyphData& ) const;
    virtual ULONG               GetKernPairs( ImplKernPairData** ) const;
    bool                        ApplyGSUB( const char* );

private:
    int                         mnWidth;
    struct FT_FaceRec_*         maFaceFT;
    FtFontInfo&                 mrFontInfo;
    FT_Int                      mnLoadFlags;

    typedef ::std::hash_map<int,int> GlyphSubstitution;
    GlyphSubstitution           aGlyphSubstitution;
};

// -----------------------------------------------------------------------

#endif // NO_FREETYPE_FONTS
