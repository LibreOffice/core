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

#ifndef INCLUDED_VCL_INC_PHYSICALFONTCOLLECTION_HXX
#define INCLUDED_VCL_INC_PHYSICALFONTCOLLECTION_HXX

#include <vcl/dllapi.h>

#include "fontinstance.hxx"
#include "PhysicalFontFamily.hxx"

class ImplGlyphFallbackFontSubstitution;
class ImplPreMatchFontSubstitution;

// - PhysicalFontCollection -

// TODO: merge with ImplFontCache
// TODO: rename to LogicalFontManager

class VCL_PLUGIN_PUBLIC PhysicalFontCollection
{
public:
    explicit                PhysicalFontCollection();
    virtual                 ~PhysicalFontCollection();

    // fill the list with device font faces
    void                    Add( PhysicalFontFace* );
    void                    Clear();
    int                     Count() const { return maPhysicalFontFamilies.size(); }

    // find the device font family
    PhysicalFontFamily*     FindFontFamily( const OUString& rFontName ) const;
    PhysicalFontFamily*     FindOrCreateFontFamily( const OUString &rFamilyName );
    PhysicalFontFamily*     FindFontFamilyByFont( FontSelectPattern& ) const;
    PhysicalFontFamily*     FindFontFamilyByTokenNames(const OUString& rTokenStr) const;
    PhysicalFontFamily*     FindFontFamilyByAttributes(ImplFontAttrs nSearchType, FontWeight, FontWidth,
                                             FontItalic, const OUString& rSearchFamily) const;

    // suggest fonts for glyph fallback
    PhysicalFontFamily*     GetGlyphFallbackFont( FontSelectPattern&,
                                                  OUString& rMissingCodes, int nFallbackLevel ) const;

    // prepare platform specific font substitutions
    void                    SetPreMatchHook( ImplPreMatchFontSubstitution* );
    void                    SetFallbackHook( ImplGlyphFallbackFontSubstitution* );

    // misc utilities
    PhysicalFontCollection* Clone( bool bScalable, bool bEmbeddable ) const;
    ImplDeviceFontList*     GetDeviceFontList() const;
    ImplDeviceFontSizeList* GetDeviceFontSizeList( const OUString& rFontName ) const;

private:
    mutable bool            mbMatchData;    // true if matching attributes are initialized
    bool                    mbMapNames;     // true if MapNames are available

    typedef std::unordered_map<OUString, PhysicalFontFamily*,OUStringHash> PhysicalFontFamilies;
    PhysicalFontFamilies    maPhysicalFontFamilies;

    ImplPreMatchFontSubstitution* mpPreMatchHook;       // device specific prematch substitution
    ImplGlyphFallbackFontSubstitution* mpFallbackHook;  // device specific glyph fallback substitution

    mutable PhysicalFontFamily**  mpFallbackList;
    mutable int             mnFallbackCount;

    void                    ImplInitMatchData() const;
    void                    ImplInitGenericGlyphFallback() const;

    PhysicalFontFamily*     ImplFindFontFamilyBySearchName( const OUString& ) const;
    PhysicalFontFamily*     ImplFindFontFamilyByAliasName ( const OUString& rSearchName, const OUString& rShortName) const;
    PhysicalFontFamily*     ImplFindFontFamilyBySubstFontAttr( const utl::FontNameAttr& ) const;

    PhysicalFontFamily*     ImplFindFontFamilyOfDefaultFont() const;

};

#endif // INCLUDED_VCL_INC_PHYSICALFONTCOLLECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
