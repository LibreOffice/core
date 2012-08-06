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

#ifndef _SV_OUTDEV_H
#define _SV_OUTDEV_H

#include <tools/solar.h>
#include <outfont.hxx>

#include <vector>
#include <list>
#include <set>

class Size;
class Font;
class VirtualDevice;
class ImplGetDevFontList;
class GetDevSizeList;

// -----------------------
// - ImplDevFontListData -
// -----------------------

// flags for mnMatchType member
#define IMPL_DEVFONT_SCALABLE       ((sal_uIntPtr)0x00000001)
#define IMPL_DEVFONT_SYMBOL         ((sal_uIntPtr)0x00000002)
#define IMPL_DEVFONT_NONESYMBOL     ((sal_uIntPtr)0x00000004)
#define IMPL_DEVFONT_LIGHT          ((sal_uIntPtr)0x00000010)
#define IMPL_DEVFONT_BOLD           ((sal_uIntPtr)0x00000020)
#define IMPL_DEVFONT_NORMAL         ((sal_uIntPtr)0x00000040)
#define IMPL_DEVFONT_NONEITALIC     ((sal_uIntPtr)0x00000100)
#define IMPL_DEVFONT_ITALIC         ((sal_uIntPtr)0x00000200)

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

    bool                AddFontFace( PhysicalFontFace* );
    void                InitMatchData( const utl::FontSubstConfiguration&,
                            const String& rSearchName );
    PhysicalFontFace*   FindBestFontFace( const FontSelectPattern& rFSD ) const;

    void                GetFontHeights( std::set<int>& rHeights ) const;
    void                UpdateDevFontList( ImplGetDevFontList& ) const;
    void                UpdateCloneFontList( ImplDevFontList&,
                                             bool bScalable, bool bEmbeddable ) const;

private:
friend class ImplDevFontList; // TODO: remove soon
    PhysicalFontFace*   mpFirst;            // linked list of physical font faces
    String              maName;             // Fontname (original font family name)
    String              maSearchName;       // normalized font family name
    String              maMapNames;         // fontname aliases
    sal_uIntPtr         mnTypeFaces;        // Typeface Flags
    sal_uIntPtr         mnMatchType;        // MATCH - Type
    String              maMatchFamilyName;  // MATCH - FamilyName
    FontWeight          meMatchWeight;      // MATCH - Weight
    FontWidth           meMatchWidth;       // MATCH - Width
    FontFamily          meFamily;
    FontPitch           mePitch;
    int                 mnMinQuality;       // quality of the worst font face
};


// ----------------------
// - ImplGetDevFontList -
// ----------------------

// an ImplGetDevFontList is created by an ImplDevFontList
// it becomes invalid when original ImplDevFontList is modified
class ImplGetDevFontList
{
private:
    std::vector<PhysicalFontFace*> maDevFontVector;

public:
                        ImplGetDevFontList()        { maDevFontVector.reserve(1024); }
    void                Add( PhysicalFontFace* pFace )  { maDevFontVector.push_back( pFace ); }
    PhysicalFontFace*   Get( int nIndex ) const     { return maDevFontVector[ nIndex ]; }
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
    sal_uInt16                  mnFlags;

    ImplFontSubstEntry(  const String& rFontName, const String& rSubstFontName, sal_uInt16 nSubstFlags );
};

class ImplDirectFontSubstitution
:   public ImplFontSubstitution
{
private:
    typedef std::list<ImplFontSubstEntry> FontSubstList;
    FontSubstList maFontSubstList;
public:
    void    AddFontSubstitute( const String& rFontName, const String& rSubstName, sal_uInt16 nFlags );
    void    RemoveFontSubstitute( int nIndex );
    int     GetFontSubstituteCount() const { return maFontSubstList.size(); };
    bool    Empty() const { return maFontSubstList.empty(); }
    void    Clear() { maFontSubstList.clear(); }

    bool    FindFontSubstitute( String& rSubstName, const String& rFontName, sal_uInt16 nFlags ) const;
};

// PreMatchFontSubstitution
// abstracts the concept of a configured font substitution
// before the availability of the originally selected font has been checked
class ImplPreMatchFontSubstitution
:   public ImplFontSubstitution
{
public:
    virtual bool FindFontSubstitute( FontSelectPattern& ) const = 0;
};

// ImplGlyphFallbackFontSubstitution
// abstracts the concept of finding the best font to support an incomplete font
class ImplGlyphFallbackFontSubstitution
:   public ImplFontSubstitution
{
public:
    virtual bool FindFontSubstitute( FontSelectPattern&, rtl::OUString& rMissingCodes ) const = 0;
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
    struct IFSD_Equal { bool operator()( const FontSelectPattern&, const FontSelectPattern& ) const; };
    struct IFSD_Hash { size_t operator()( const FontSelectPattern& ) const; };
    typedef ::boost::unordered_map<FontSelectPattern,ImplFontEntry*,IFSD_Hash,IFSD_Equal > FontInstanceList;
    FontInstanceList    maFontInstanceList;

    // cache of recently requested font names vs. selected font names
    typedef ::boost::unordered_map<String,String,FontNameHash> FontNameList;
    FontNameList        maFontNameList;

public:
                        ImplFontCache( bool bPrinter );
                        ~ImplFontCache();

    ImplFontEntry*      GetFontEntry( ImplDevFontList*,
                             const Font&, const Size& rPixelSize, float fExactHeight,
                ImplDirectFontSubstitution* pDevSpecific );
    ImplFontEntry*      GetFontEntry( ImplDevFontList*,
                    FontSelectPattern&, ImplDirectFontSubstitution* pDevSpecific );
    ImplFontEntry*      GetGlyphFallbackFont( ImplDevFontList*, FontSelectPattern&,
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
