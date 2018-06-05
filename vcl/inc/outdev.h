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

#include <set>
#include <vector>

#include <tools/gen.hxx>
#include <vcl/vclptr.hxx>

#include "fontinstance.hxx"
#include "PhysicalFontFace.hxx"
#include "impfontcache.hxx"

class Size;
namespace vcl { class Font; }
class VirtualDevice;
class PhysicalFontCollection;
enum class AddFontSubstituteFlags;

// an ImplDeviceFontList is created by an PhysicalFontCollection
// it becomes invalid when original PhysicalFontCollection is modified
class ImplDeviceFontList
{
private:
    std::vector<rtl::Reference<PhysicalFontFace>> maDevFontVector;

public:
                        ImplDeviceFontList()        { maDevFontVector.reserve(1024); }
    void                Add( PhysicalFontFace* pFace )  { maDevFontVector.push_back( pFace ); }
    PhysicalFontFace*   Get( int nIndex ) const     { return maDevFontVector[ nIndex ].get(); }
    int                 Count() const               { return maDevFontVector.size(); }
};

class ImplDeviceFontSizeList
{
private:
    std::vector<int>    maSizeList;

public:
                        ImplDeviceFontSizeList()
                        { maSizeList.reserve( 32 ); }
    void                Add( int nHeight )      { maSizeList.push_back( nHeight ); }
    int                 Count() const           { return maSizeList.size(); }
    int                 Get( int nIndex ) const { return maSizeList[ nIndex ]; }
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
// The class is just a simple port of the unmaintainable manual-linked-list based mechanism
// TODO: get rid of this class when the Tools->Options->FontReplacement tabpage is gone for good

struct ImplFontSubstEntry
{
    OUString                  maSearchName;
    OUString                  maSearchReplaceName;
    AddFontSubstituteFlags    mnFlags;

    ImplFontSubstEntry(  const OUString& rFontName, const OUString& rSubstFontName, AddFontSubstituteFlags nSubstFlags );
};

class ImplDirectFontSubstitution
:   public ImplFontSubstitution
{
private:
    std::vector<ImplFontSubstEntry> maFontSubstList;
public:
    void    AddFontSubstitute( const OUString& rFontName, const OUString& rSubstName, AddFontSubstituteFlags nFlags );
    void    RemoveFontsSubstitute();

    bool    FindFontSubstitute( OUString& rSubstName, const OUString& rFontName ) const;
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

namespace vcl { struct ControlLayoutData; }
// #i75163#
namespace basegfx { class B2DHomMatrix; }

struct ImplOutDevData
{
    VclPtr<VirtualDevice>       mpRotateDev;
    vcl::ControlLayoutData*     mpRecordLayout;
    tools::Rectangle                   maRecordRect;

    // #i75163#
    basegfx::B2DHomMatrix*      mpViewTransform;
    basegfx::B2DHomMatrix*      mpInverseViewTransform;
};

void ImplFontSubstitute( OUString& rFontName );

#endif // INCLUDED_VCL_INC_OUTDEV_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
