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

#include <set>

#include <unotools/fontcfg.hxx>

#include "PhysicalFontFace.hxx"
#include "PhysicalFontFamily.hxx"

class PhysicalFontFace;
class PhysicalFontCollection;

// flags for mnMatchType member
#define FONT_FAMILY_SCALABLE      (1<<0)
#define FONT_FAMILY_SYMBOL        (1<<1)
#define FONT_FAMILY_NONESYMBOL    (1<<2)
#define FONT_FAMILY_LIGHT         (1<<4)
#define FONT_FAMILY_BOLD          (1<<5)
#define FONT_FAMILY_NORMAL        (1<<6)
#define FONT_FAMILY_NONEITALIC    (1<<8)
#define FONT_FAMILY_ITALIC        (1<<9)

class PhysicalFontFamily
{
public:
                        PhysicalFontFamily( const OUString& rSearchName );
                       ~PhysicalFontFamily();

    const OUString&     GetFamilyName() const    { return maName; }
    const OUString&     GetSearchName() const    { return maSearchName; }
    const OUString&     GetAliasNames() const    { return maMapNames; }
    const OUString&     GetMatchFamilyName() const { return maMatchFamilyName; }
    sal_uLong           GetMatchType() const     { return mnMatchType ; }
    FontWeight          GetMatchWeight() const   { return meMatchWeight ; }
    FontWidth           GetMatchWidth() const    { return meMatchWidth ; }
    bool                IsScalable() const       { return mpFirst->IsScalable(); }
    int                 GetMinQuality() const    { return mnMinQuality; }
    int                 GetTypeFaces() const     { return mnTypeFaces; }
    bool                AddFontFace( PhysicalFontFace* );
    void                InitMatchData( const utl::FontSubstConfiguration&,
                                       const OUString& rSearchName );
    PhysicalFontFace*   FindBestFontFace( const FontSelectPattern& rFSD ) const;

    void                GetFontHeights( std::set<int>& rHeights ) const;
    void                UpdateDevFontList( ImplGetDevFontList& ) const;
    void                UpdateCloneFontList( PhysicalFontCollection&,
                                             bool bScalable, bool bEmbeddable ) const;

static void             CalcType( sal_uLong& rType, FontWeight& rWeight, FontWidth& rWidth,
                                  FontFamily eFamily, const utl::FontNameAttr* pFontAttr );

private:
    PhysicalFontFace*   mpFirst;            // linked list of physical font faces
    OUString            maName;             // Fontname (original font family name)
    OUString            maSearchName;       // normalized font family name
    OUString            maMapNames;         // fontname aliases
    int                 mnTypeFaces;        // Typeface Flags
    sal_uLong           mnMatchType;        // MATCH - Type
    OUString            maMatchFamilyName;  // MATCH - FamilyName
    FontWeight          meMatchWeight;      // MATCH - Weight
    FontWidth           meMatchWidth;       // MATCH - Width
    FontFamily          meFamily;
    FontPitch           mePitch;
    int                 mnMinQuality;       // quality of the worst font face
};

