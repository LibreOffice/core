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
#include <vcl/outdev.hxx>

#include <unotools/fontcfg.hxx>

namespace vcl::font
{
// flags for mnTypeFaces member
enum class FontTypeFaces
{
    NONE = 0x00,
    Scalable = 0x01,
    Symbol = 0x02,
    NoneSymbol = 0x04,
    Light = 0x08,
    Bold = 0x10,
    Normal = 0x20,
    NoneItalic = 0x40,
    Italic = 0x80
};
}
namespace o3tl
{
template <>
struct typed_flags<vcl::font::FontTypeFaces> : is_typed_flags<vcl::font::FontTypeFaces, 0xff>
{
};
};

namespace vcl::font
{
class FontSelectPattern;
class PhysicalFontCollection;
class PhysicalFontFace;
class PhysicalFontFaceCollection;

class VCL_PLUGIN_PUBLIC PhysicalFontFamily
{
public:
    PhysicalFontFamily(OUString aSearchName);
    ~PhysicalFontFamily();

    // Avoid implicitly defined copy constructors/assignments for the DLLPUBLIC class (they may
    // require forward-declared classes used internally to be defined in places using this)
    PhysicalFontFamily(const PhysicalFontFamily&) = delete;
    PhysicalFontFamily(PhysicalFontFamily&&) = delete;
    PhysicalFontFamily& operator=(const PhysicalFontFamily&) = delete;
    PhysicalFontFamily& operator=(PhysicalFontFamily&&) = delete;

    const OUString& GetFamilyName() const { return maFamilyName; }
    const OUString& GetSearchName() const { return maSearchName; }
    int GetMinQuality() const { return mnMinQuality; }
    FontTypeFaces GetTypeFaces() const { return mnTypeFaces; }

    const OUString& GetMatchFamilyName() const { return maMatchFamilyName; }
    ImplFontAttrs GetMatchType() const { return mnMatchType; }
    FontWeight GetMatchWeight() const { return meMatchWeight; }
    FontWidth GetMatchWidth() const { return meMatchWidth; }
    SAL_DLLPRIVATE void InitMatchData(const utl::FontSubstConfiguration&,
                                      const OUString& rSearchName);

    void AddFontFace(PhysicalFontFace*);

    SAL_DLLPRIVATE PhysicalFontFace*
    FindBestFontFace(const vcl::font::FontSelectPattern& rFSD) const;

    SAL_DLLPRIVATE void UpdateDevFontList(PhysicalFontFaceCollection&) const;
    SAL_DLLPRIVATE void UpdateCloneFontList(PhysicalFontCollection&) const;

    SAL_DLLPRIVATE static void CalcType(ImplFontAttrs& rType, FontWeight& rWeight,
                                        FontWidth& rWidth, FontFamily eFamily,
                                        const utl::FontNameAttr* pFontAttr);

private:
    std::vector<rtl::Reference<PhysicalFontFace>> maFontFaces;

    OUString maFamilyName; // original font family name
    OUString maSearchName; // normalized font family name
    FontTypeFaces mnTypeFaces; // Typeface Flags
    FontFamily meFamily;
    FontPitch mePitch;
    int mnMinQuality; // quality of the worst font face

    ImplFontAttrs mnMatchType; // MATCH - Type
    OUString maMatchFamilyName; // MATCH - FamilyName
    FontWeight meMatchWeight; // MATCH - Weight
    FontWidth meMatchWidth; // MATCH - Width
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
