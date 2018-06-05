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

#ifndef INCLUDED_VCL_INC_PHYSICALFONTFAMILY_HXX
#define INCLUDED_VCL_INC_PHYSICALFONTFAMILY_HXX

#include <vcl/dllapi.h>

#include <set>

#include <unotools/fontcfg.hxx>

#include "PhysicalFontFace.hxx"

class PhysicalFontFace;
class PhysicalFontCollection;

// flags for mnTypeFaces member
enum class FontTypeFaces {
    NONE          = 0x00,
    Scalable      = 0x01,
    Symbol        = 0x02,
    NoneSymbol    = 0x04,
    Light         = 0x08,
    Bold          = 0x10,
    Normal        = 0x20,
    NoneItalic    = 0x40,
    Italic        = 0x80
};
namespace o3tl {
    template<> struct typed_flags<FontTypeFaces> : is_typed_flags<FontTypeFaces, 0xff> {};
};

class VCL_PLUGIN_PUBLIC PhysicalFontFamily
{
public:
                        PhysicalFontFamily( const OUString& rSearchName );
                       ~PhysicalFontFamily();

    const OUString&     GetFamilyName() const    { return maFamilyName; }
    const OUString&     GetSearchName() const    { return maSearchName; }
    const OUString&     GetAliasNames() const    { return maMapNames; }
    int                 GetMinQuality() const    { return mnMinQuality; }
    FontTypeFaces       GetTypeFaces() const     { return mnTypeFaces; }
    void                GetFontHeights( std::set<int>& rHeights ) const;

    const OUString&     GetMatchFamilyName() const { return maMatchFamilyName; }
    ImplFontAttrs       GetMatchType() const     { return mnMatchType ; }
    FontWeight          GetMatchWeight() const   { return meMatchWeight ; }
    FontWidth           GetMatchWidth() const    { return meMatchWidth ; }
    void                InitMatchData( const utl::FontSubstConfiguration&,
                                       const OUString& rSearchName );

    void                AddFontFace( PhysicalFontFace* );

    PhysicalFontFace*   FindBestFontFace( const FontSelectPattern& rFSD ) const;

    void                UpdateDevFontList( ImplDeviceFontList& ) const;
    void                UpdateCloneFontList(PhysicalFontCollection&) const;

static void             CalcType( ImplFontAttrs& rType, FontWeight& rWeight, FontWidth& rWidth,
                                  FontFamily eFamily, const utl::FontNameAttr* pFontAttr );

private:
    std::vector< rtl::Reference<PhysicalFontFace> > maFontFaces;

    OUString            maFamilyName;       // original font family name
    OUString            maSearchName;       // normalized font family name
    OUString            maMapNames;         // fontname aliases
    FontTypeFaces       mnTypeFaces;        // Typeface Flags
    FontFamily          meFamily;
    FontPitch           mePitch;
    int                 mnMinQuality;       // quality of the worst font face

    ImplFontAttrs       mnMatchType;        // MATCH - Type
    OUString            maMatchFamilyName;  // MATCH - FamilyName
    FontWeight          meMatchWeight;      // MATCH - Weight
    FontWidth           meMatchWidth;       // MATCH - Width
};

#endif // INCLUDED_VCL_INC_PHYSICALFONTFAMILY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
