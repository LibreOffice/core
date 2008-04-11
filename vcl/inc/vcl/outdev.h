/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: outdev.h,v $
 * $Revision: 1.5 $
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

#ifndef _SV_OUTDEV_H
#define _SV_OUTDEV_H

#include <vcl/sv.h>
#include <vcl/outfont.hxx>

#include <vector>
#include <list>
#include <set>

class Size;
class Font;
class VirtualDevice;
class ImplServerGraphics;
class ImplGetDevFontList;
class GetDevSizeList;
namespace vcl {
class FontSubstConfiguration;
}

// -----------------------------------------------------------------------

void ImplGetEnglishSearchFontName( String& rName );

// -----------------------
// - ImplDevFontListData -
// -----------------------

// flags for mnMatchType member
#define IMPL_DEVFONT_SCALABLE       ((ULONG)0x00000001)
#define IMPL_DEVFONT_SYMBOL         ((ULONG)0x00000002)
#define IMPL_DEVFONT_NONESYMBOL     ((ULONG)0x00000004)
#define IMPL_DEVFONT_LIGHT          ((ULONG)0x00000010)
#define IMPL_DEVFONT_BOLD           ((ULONG)0x00000020)
#define IMPL_DEVFONT_NORMAL         ((ULONG)0x00000040)
#define IMPL_DEVFONT_NONEITALIC     ((ULONG)0x00000100)
#define IMPL_DEVFONT_ITALIC         ((ULONG)0x00000200)

// TODO: rename ImplDevFontListData to PhysicalFontFamily
class ImplDevFontListData
{
public:
                        ImplDevFontListData( const String& rSearchName );
                        ~ImplDevFontListData();

    const String&       GetFamilyName() const    { return maName; }
    const String&       GetSearchName() const    { return maSearchName; }
    const String&       GetAliasNames() const    { return maMapNames; }
    bool                IsScalable() const       { return mpFirst->IsScalable(); }
    int                 GetMinQuality() const    { return mnMinQuality; }

    bool                AddFontFace( ImplFontData* );
    void                InitMatchData( const vcl::FontSubstConfiguration&,
                            const String& rSearchName );
    ImplFontData*       FindBestFontFace( const ImplFontSelectData& rFSD ) const;

    void                GetFontHeights( std::set<int>& rHeights ) const;
    void                UpdateDevFontList( ImplGetDevFontList& ) const;
    void                UpdateCloneFontList( ImplDevFontList&,
                            bool bScalable, bool bEmbeddable ) const;

private:
friend class ImplDevFontList; // TODO: remove soon
    ImplFontData*       mpFirst;            // linked list of physical font faces
    String              maName;             // Fontname (original font family name)
    String              maSearchName;       // normalized font family name
    String              maMapNames;         // fontname aliases
    ULONG               mnTypeFaces;        // Typeface Flags
    ULONG               mnMatchType;        // MATCH - Type
    String              maMatchFamilyName;  // MATCH - FamilyName
    FontWeight          meMatchWeight;      // MATCH - Weight
    FontWidth           meMatchWidth;       // MATCH - Width
    FontFamily          meFamily;
    FontPitch           mePitch;
    int                 mnMinQuality;       // quality of the worst font face
};


// ---------------
// - ImplCvtChar -
// ---------------

class ImplCvtChar
{
public:
    const sal_Unicode*  mpCvtTab;
    const char*         mpSubsFontName;
    sal_Unicode         (*mpCvtFunc)( sal_Unicode );
};

sal_Unicode ImplRecodeChar( const ImplCvtChar* pCvtData, sal_Unicode c );
void ImplRecodeString( const ImplCvtChar* pCvtData, String& rStr,
                       xub_StrLen nIndex, xub_StrLen nLen );
const ImplCvtChar* ImplGetRecodeData( const String& rOrgFontName,
                                      const String& rMapFontName );

// ----------------------
// - ImplGetDevFontList -
// ----------------------

// an ImplGetDevFontList is created by an ImplDevFontList
// it becomes invalid when original ImplDevFontList is modified
class ImplGetDevFontList
{
private:
    std::vector<ImplFontData*> maDevFontVector;

public:
                        ImplGetDevFontList()        { maDevFontVector.reserve(1024); }
    void                Add( ImplFontData* pFace )  { maDevFontVector.push_back( pFace ); }
    ImplFontData*       Get( int nIndex ) const     { return maDevFontVector[ nIndex ]; }
    int                 Count() const               { return maDevFontVector.size(); }
};

// ----------------------
// - ImplGetDevSizeList -
// ----------------------

class ImplGetDevSizeList
{
private:
    String              maFontName;
    std::vector<int>    maSizeList;

public:
                        ImplGetDevSizeList( const String& rFontName )
                        : maFontName( rFontName ) { maSizeList.reserve( 32 ); }
    void                Add( int nHeight )      { maSizeList.push_back( nHeight ); }
    int                 Count() const           { return maSizeList.size(); }
    int                 Get( int nIndex ) const { return maSizeList[ nIndex ]; }
    const String&       GetFontName() const     { return maFontName; }
};

// ------------------------
// - ImplFontSubstitution -
// ------------------------
// nowadays these substitutions are needed for backward compatibility and tight platform integration:
// - substitutions from configuration entries (Tools->Options->FontReplacement and/or fontconfig)
// - device specific substitutions (e.g. for PS printer builtin fonts)
// - substitutions for missing fonts defined by configuration entries (generic and/or platform dependent fallbacks)
// - substitutions for missing fonts defined by multi-token fontnames (e.g. fontname="SpecialFont;FallbackA;FallbackB")
// - substitutions for incomplete fonts (implicit, generic, EUDC and/or platform dependent fallbacks)
// - substitutions for missing symbol fonts by translating code points into other symbol fonts

class ImplFontSubstitution
{
    // TODO: there is more commonality between the different substitutions
protected:
    virtual ~ImplFontSubstitution() {}
};

// ImplDirectFontSubstitution is for Tools->Options->FontReplacement and PsPrinter substitutions
// The clss is just a simple port of the unmaintainable manual-linked-list based mechanism
// TODO: get rid of this class when the Tools->Options->FontReplacement tabpage is gone for good

struct ImplFontSubstEntry
{
    String                  maName;
    String                  maReplaceName;
    String                  maSearchName;
    String                  maSearchReplaceName;
    USHORT                  mnFlags;

    ImplFontSubstEntry(  const String& rFontName, const String& rSubstFontName, USHORT nSubstFlags );
};

class ImplDirectFontSubstitution
:   public ImplFontSubstitution
{
private:
    typedef std::list<ImplFontSubstEntry> FontSubstList;
    FontSubstList maFontSubstList;
public:
    void    AddFontSubstitute( const String& rFontName, const String& rSubstName, USHORT nFlags );
    void    RemoveFontSubstitute( int nIndex );
    bool    GetFontSubstitute( int nIndex, String& rFontName, String& rSubstName, USHORT& rFlags ) const;
    int     GetFontSubstituteCount() const { return maFontSubstList.size(); };
    bool    Empty() const { return maFontSubstList.empty(); }
    void    Clear() { maFontSubstList.clear(); }

    bool    FindFontSubstitute( String& rSubstName, const String& rFontName, USHORT nFlags ) const;
};

// PreMatchFontSubstitution
// abstracts the concept of a configured font substitution
// before the availability of the originally selected font has been checked
class ImplPreMatchFontSubstitution
:   public ImplFontSubstitution
{
public:
    virtual bool FindFontSubstitute( ImplFontSelectData& ) const = 0;
};

// ImplGlyphFallbackFontSubstitution
// abstracts the concept of finding the best font to support an incomplete font
class ImplGlyphFallbackFontSubstitution
:   public ImplFontSubstitution
{
public:
    virtual bool FindFontSubstitute( ImplFontSelectData&, rtl::OUString& rMissingCodes ) const = 0;
};

// -----------------
// - ImplFontCache -
// -----------------
// TODO: closely couple with ImplDevFontList

class ImplFontCache
{
private:
    ImplFontEntry*      mpFirstEntry;
    int                 mnRef0Count;    // number of unreferenced ImplFontEntries
    bool                mbPrinter;

    // cache of recently used font instances
    struct IFSD_Equal { bool operator()( const ImplFontSelectData&, const ImplFontSelectData& ) const; };
    struct IFSD_Hash { size_t operator()( const ImplFontSelectData& ) const; };
    typedef ::std::hash_map<ImplFontSelectData,ImplFontEntry*,IFSD_Hash,IFSD_Equal > FontInstanceList;
    FontInstanceList    maFontInstanceList;

    // cache of recently requested font names vs. selected font names
    typedef ::std::hash_map<String,String,FontNameHash> FontNameList;
    FontNameList        maFontNameList;

public:
                        ImplFontCache( bool bPrinter );
                        ~ImplFontCache();

    ImplFontEntry*      GetFontEntry( ImplDevFontList*,
                             const Font&, const Size& rPixelSize, float fExactHeight,
                ImplDirectFontSubstitution* pDevSpecific );
    ImplFontEntry*      GetFontEntry( ImplDevFontList*,
                    ImplFontSelectData&, ImplDirectFontSubstitution* pDevSpecific );
    ImplFontEntry*      GetGlyphFallbackFont( ImplDevFontList*, ImplFontSelectData&,
                            int nFallbackLevel, rtl::OUString& rMissingCodes );
    void                Release( ImplFontEntry* );
    void                Invalidate();
};

// ------------------
// - ImplOutDevData -
// ------------------

namespace vcl { struct ControlLayoutData; }
// #i75163#
namespace basegfx { class B2DHomMatrix; }

struct ImplOutDevData
{
    VirtualDevice*              mpRotateDev;
    vcl::ControlLayoutData*     mpRecordLayout;
    Rectangle                   maRecordRect;
    ImplDirectFontSubstitution      maDevFontSubst;

    // #i75163#
    basegfx::B2DHomMatrix*      mpViewTransform;
    basegfx::B2DHomMatrix*      mpInverseViewTransform;
};

void ImplFreeOutDevFontData();

#endif // _SV_OUTDEV_H
