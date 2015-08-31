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

#ifndef INCLUDED_VCL_INC_OUTDEV_H
#define INCLUDED_VCL_INC_OUTDEV_H

#include <list>
#include <set>
#include <vector>

#include <tools/gen.hxx>
#include <vcl/vclptr.hxx>

#include "outfont.hxx"
#include "PhysicalFontFace.hxx"

class Size;
namespace vcl { class Font; }
class VirtualDevice;
class ImplGetDevFontList;
class GetDevSizeList;
class PhysicalFontCollection;
enum class AddFontSubstituteFlags;

// an ImplGetDevFontList is created by an PhysicalFontCollection
// it becomes invalid when original PhysicalFontCollection is modified
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

class ImplGetDevSizeList
{
private:
    OUString            maFontName;
    std::vector<int>    maSizeList;

public:
                        ImplGetDevSizeList( const OUString& rFontName )
                        : maFontName( rFontName ) { maSizeList.reserve( 32 ); }
    void                Add( int nHeight )      { maSizeList.push_back( nHeight ); }
    int                 Count() const           { return maSizeList.size(); }
    int                 Get( int nIndex ) const { return maSizeList[ nIndex ]; }
    const OUString&     GetFontName() const     { return maFontName; }
};

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
    OUString                  maName;
    OUString                  maReplaceName;
    OUString                  maSearchName;
    OUString                  maSearchReplaceName;
    AddFontSubstituteFlags    mnFlags;

    ImplFontSubstEntry(  const OUString& rFontName, const OUString& rSubstFontName, AddFontSubstituteFlags nSubstFlags );
};

class ImplDirectFontSubstitution
:   public ImplFontSubstitution
{
private:
    typedef std::list<ImplFontSubstEntry> FontSubstList;
    FontSubstList maFontSubstList;
public:
    void    AddFontSubstitute( const OUString& rFontName, const OUString& rSubstName, AddFontSubstituteFlags nFlags );
    void    RemoveFontSubstitute( int nIndex );
    int     GetFontSubstituteCount() const { return maFontSubstList.size(); };
    bool    Empty() const { return maFontSubstList.empty(); }
    void    Clear() { maFontSubstList.clear(); }

    bool    FindFontSubstitute( OUString& rSubstName, const OUString& rFontName, AddFontSubstituteFlags nFlags ) const;
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
    virtual bool FindFontSubstitute( FontSelectPattern&, OUString& rMissingCodes ) const = 0;
};

// TODO: closely couple with PhysicalFontCollection

class ImplFontCache
{
private:
    ImplFontEntry*      mpFirstEntry;
    int                 mnRef0Count;    // number of unreferenced ImplFontEntries

    // cache of recently used font instances
    struct IFSD_Equal { bool operator()( const FontSelectPattern&, const FontSelectPattern& ) const; };
    struct IFSD_Hash { size_t operator()( const FontSelectPattern& ) const; };
    typedef std::unordered_map<FontSelectPattern,ImplFontEntry*,IFSD_Hash,IFSD_Equal > FontInstanceList;
    FontInstanceList    maFontInstanceList;

    int                 CountUnreferencedEntries() const;

public:
                        ImplFontCache();
                        ~ImplFontCache();

    ImplFontEntry*      GetFontEntry( PhysicalFontCollection*,
                             const vcl::Font&, const Size& rPixelSize, float fExactHeight);
    ImplFontEntry*      GetFontEntry( PhysicalFontCollection*, FontSelectPattern& );
    ImplFontEntry*      GetGlyphFallbackFont( PhysicalFontCollection*, FontSelectPattern&,
                            int nFallbackLevel, OUString& rMissingCodes );

    /// Increase the refcount of the given ImplFontEntry.
    void                Acquire(ImplFontEntry*);
    /// Decrease the refcount and potentially cleanup the entries with zero refcount from the cache.
    void                Release(ImplFontEntry*);

    void                Invalidate();
};

namespace vcl { struct ControlLayoutData; }
// #i75163#
namespace basegfx { class B2DHomMatrix; }

struct ImplOutDevData
{
    VclPtr<VirtualDevice>       mpRotateDev;
    vcl::ControlLayoutData*     mpRecordLayout;
    Rectangle                   maRecordRect;

    // #i75163#
    basegfx::B2DHomMatrix*      mpViewTransform;
    basegfx::B2DHomMatrix*      mpInverseViewTransform;
};

void ImplFontSubstitute( OUString& rFontName );

#endif // INCLUDED_VCL_INC_OUTDEV_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
