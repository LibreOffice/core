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

#pragma once

#include <sal/config.h>

#include <vcl/dllapi.h>

#include <fontinstance.hxx>

#include "PhysicalFontFamily.hxx"

#include <array>
#include <string_view>

#define MAX_GLYPHFALLBACK 16

namespace vcl::font
{
const int CJK_MATCH_VALUE = 10'000'000;
const int CTL_MATCH_VALUE = 10'000'000;
const int NONLATIN_MATCH_VALUE = 10'000'000;
const int SYMBOL_MATCH_VALUE = 10'000'000;
const int FAMILY_NAME_MATCH_VALUE = 1'000'000;
const int SCRIPT_MATCH_VALUE = 1'000'000;
const int FIXED_MATCH_VALUE = 1'000'000;
const int SPECIAL_MATCH_VALUE = 1'000'000;
const int DECORATIVE_MATCH_VALUE = 1'000'000;
const int CAPITALS_TITLING_MATCH_VALUE = 1'000'000;
const int OUTLINE_SHADOW_MATCH_VALUE = 1'000'000;
const int FONTNAME_MATCH_VALUE = 5'000;
const int SERIF_MATCH_VALUE = 1'000'000;
const int SANSERIF_MATCH_VALUE = 1'000'000;
const int ITALIC_MATCH_VALUE = 1'000'000;
const int WIDTH_MATCH_VALUE = 1'000'000;
const int WEIGHT_MATCH_VALUE = 1'000'000;
const int SCALABLE_MATCH_VALUE = 10'000;
const int STANDARD_MATCH_VALUE = 10'000;
const int DEFAULT_MATCH_VALUE = 10'000;
const int FULL_MATCH_VALUE = 10'000;
const int NORMAL_MATCH_VALUE = 10'000;
const int OTHERSTYLE_MATCH_VALUE = 10'000;
const int ROUNDED_MATCH_VALUE = 1'000;
const int TYPEWRITER_MATCH_VALUE = 1'000;
const int GOTHIC_MATCH_VALUE = 1'000;
const int SCHOOLBOOK_MATCH_VALUE = 1'000;

class GlyphFallbackFontSubstitution;
class PreMatchFontSubstitution;
}

// TODO: merge with ImplFontCache
// TODO: rename to LogicalFontManager

namespace vcl::font
{

class VCL_PLUGIN_PUBLIC PhysicalFontCollection final
{
public:
    explicit                PhysicalFontCollection();
                            ~PhysicalFontCollection();

    // fill the list with device font faces
    void                    Add( vcl::font::PhysicalFontFace* );
    void                    Clear();
    int                     Count() const { return maPhysicalFontFamilies.size(); }

    // find the device font family
    vcl::font::PhysicalFontFamily* FindFontFamily( std::u16string_view rFontName ) const;
    vcl::font::PhysicalFontFamily* FindOrCreateFontFamily( const OUString &rFamilyName );
    vcl::font::PhysicalFontFamily* FindFontFamily( vcl::font::FontSelectPattern& ) const;
    vcl::font::PhysicalFontFamily* FindFontFamilyByTokenNames(const OUString& rTokenStr) const;
    vcl::font::PhysicalFontFamily* FindFontFamilyByAttributes(ImplFontAttrs nSearchType, FontWeight, FontWidth,
                                             FontItalic, const OUString& rSearchFamily) const;

    // suggest fonts for glyph fallback
    vcl::font::PhysicalFontFamily* GetGlyphFallbackFont( vcl::font::FontSelectPattern&,
                                                  LogicalFontInstance* pLogicalFont,
                                                  OUString& rMissingCodes, int nFallbackLevel ) const;

    // prepare platform specific font substitutions
    void                    SetPreMatchHook( vcl::font::PreMatchFontSubstitution* );
    void                    SetFallbackHook( vcl::font::GlyphFallbackFontSubstitution* );

    // misc utilities
    std::shared_ptr<PhysicalFontCollection> Clone() const;
    std::unique_ptr<vcl::font::PhysicalFontFaceCollection> GetFontFaceCollection() const;

private:
    mutable bool            mbMatchData;    // true if matching attributes are initialized

    typedef std::unordered_map<OUString, std::unique_ptr<vcl::font::PhysicalFontFamily>> PhysicalFontFamilies;
    PhysicalFontFamilies    maPhysicalFontFamilies;

    vcl::font::PreMatchFontSubstitution* mpPreMatchHook;       // device specific prematch substitution
    vcl::font::GlyphFallbackFontSubstitution* mpFallbackHook;  // device specific glyph fallback substitution

    mutable std::unique_ptr<std::array<vcl::font::PhysicalFontFamily*,MAX_GLYPHFALLBACK>> mpFallbackList;
    mutable int             mnFallbackCount;

    void                    ImplInitMatchData() const;
    void                    ImplInitGenericGlyphFallback() const;

    vcl::font::PhysicalFontFamily* ImplFindFontFamilyBySearchName( const OUString& ) const;
    vcl::font::PhysicalFontFamily* ImplFindFontFamilyBySubstFontAttr( const utl::FontNameAttr& ) const;

    vcl::font::PhysicalFontFamily* ImplFindFontFamilyOfDefaultFont() const;

};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
