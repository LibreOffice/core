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

#include <font/LogicalFontInstance.hxx>

#include "PhysicalFontFamily.hxx"

#include <array>
#include <string_view>

#define MAX_GLYPHFALLBACK 16

namespace vcl::font
{
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
    SAL_DLLPRIVATE void     Clear();
    int                     Count() const { return maPhysicalFontFamilies.size(); }

    // find the device font family
    vcl::font::PhysicalFontFamily* FindFontFamily( std::u16string_view rFontName ) const;
    vcl::font::PhysicalFontFamily* FindOrCreateFontFamily( const OUString &rFamilyName );
    SAL_DLLPRIVATE vcl::font::PhysicalFontFamily* FindFontFamily( vcl::font::FontSelectPattern& ) const;
    vcl::font::PhysicalFontFamily* FindFontFamilyByTokenNames(std::u16string_view rTokenStr) const;
    vcl::font::PhysicalFontFamily* FindFontFamilyByAttributes(ImplFontAttrs nSearchType, FontWeight, FontWidth,
                                             FontItalic, std::u16string_view rSearchFamily) const;

    // suggest fonts for glyph fallback
    SAL_DLLPRIVATE vcl::font::PhysicalFontFamily* GetGlyphFallbackFont( vcl::font::FontSelectPattern&,
                                                  LogicalFontInstance* pLogicalFont,
                                                  OUString& rMissingCodes, int nFallbackLevel ) const;

    // prepare platform specific font substitutions
    SAL_DLLPRIVATE void SetPreMatchHook( vcl::font::PreMatchFontSubstitution* );
    void SetFallbackHook( vcl::font::GlyphFallbackFontSubstitution* );

    // misc utilities
    SAL_DLLPRIVATE std::shared_ptr<PhysicalFontCollection> Clone() const;
    SAL_DLLPRIVATE std::unique_ptr<vcl::font::PhysicalFontFaceCollection> GetFontFaceCollection() const;

private:
    mutable bool            mbMatchData;    // true if matching attributes are initialized

    typedef std::unordered_map<OUString, std::unique_ptr<vcl::font::PhysicalFontFamily>> PhysicalFontFamilies;
    PhysicalFontFamilies    maPhysicalFontFamilies;

    vcl::font::PreMatchFontSubstitution* mpPreMatchHook;       // device specific prematch substitution
    vcl::font::GlyphFallbackFontSubstitution* mpFallbackHook;  // device specific glyph fallback substitution

    mutable std::unique_ptr<std::array<vcl::font::PhysicalFontFamily*,MAX_GLYPHFALLBACK>> mpFallbackList;
    mutable int             mnFallbackCount;

    SAL_DLLPRIVATE void ImplInitMatchData() const;
    SAL_DLLPRIVATE void ImplInitGenericGlyphFallback() const;

    SAL_DLLPRIVATE vcl::font::PhysicalFontFamily* ImplFindFontFamilyBySearchName( const OUString& ) const;
    SAL_DLLPRIVATE vcl::font::PhysicalFontFamily* ImplFindFontFamilyBySubstFontAttr( const utl::FontNameAttr& ) const;

    SAL_DLLPRIVATE vcl::font::PhysicalFontFamily* ImplFindFontFamilyOfDefaultFont() const;

};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
