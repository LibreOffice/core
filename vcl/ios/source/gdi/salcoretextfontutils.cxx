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

#include <boost/assert.hpp>
#include <vector>
#include <set>

#include "vcl/svapp.hxx"

#include "ios/salgdi.h"
#include "ios/saldata.hxx"
#include "ios/salcoretextfontutils.hxx"

// we have to get the font attributes from the name table
// since neither head's macStyle nor OS/2's panose are easily available
// during font enumeration. macStyle bits would be not sufficient anyway
// and SFNT fonts on Mac usually do not contain an OS/2 table.
static void UpdateAttributesFromPSName( const String& rPSName, ImplDevFontAttributes& rDFA )
{
    rtl::OString aPSName( rtl::OUStringToOString( rPSName, RTL_TEXTENCODING_UTF8 ).toAsciiLowerCase() );

    // TODO: use a multi-string ignore-case matcher once it becomes available
    if( (aPSName.indexOf("regular") != -1)
    ||  (aPSName.indexOf("normal") != -1)
    ||  (aPSName.indexOf("roman") != -1)
    ||  (aPSName.indexOf("medium") != -1)
    ||  (aPSName.indexOf("plain") != -1)
    ||  (aPSName.indexOf("standard") != -1)
    ||  (aPSName.indexOf("std") != -1) )
    {
       rDFA.meWidthType = WIDTH_NORMAL;
       rDFA.meWeight    = WEIGHT_NORMAL;
       rDFA.meItalic    = ITALIC_NONE;
    }

    // heuristics for font weight
    if (aPSName.indexOf("extrablack") != -1)
        rDFA.meWeight = WEIGHT_BLACK;
    else if (aPSName.indexOf("black") != -1)
        rDFA.meWeight = WEIGHT_BLACK;
    //else if (aPSName.indexOf("book") != -1)
    //    rDFA.meWeight = WEIGHT_SEMIBOLD;
    else if( (aPSName.indexOf("semibold") != -1)
    ||  (aPSName.indexOf("smbd") != -1))
        rDFA.meWeight = WEIGHT_SEMIBOLD;
    else if (aPSName.indexOf("ultrabold") != -1)
        rDFA.meWeight = WEIGHT_ULTRABOLD;
    else if (aPSName.indexOf("extrabold") != -1)
        rDFA.meWeight = WEIGHT_BLACK;
    else if( (aPSName.indexOf("bold") != -1)
    ||  (aPSName.indexOf("-bd") != -1))
        rDFA.meWeight = WEIGHT_BOLD;
    else if (aPSName.indexOf("extralight") != -1)
        rDFA.meWeight = WEIGHT_ULTRALIGHT;
    else if (aPSName.indexOf("ultralight") != -1)
        rDFA.meWeight = WEIGHT_ULTRALIGHT;
    else if (aPSName.indexOf("light") != -1)
        rDFA.meWeight = WEIGHT_LIGHT;
    else if (aPSName.indexOf("thin") != -1)
        rDFA.meWeight = WEIGHT_THIN;
    else if (aPSName.indexOf("-w3") != -1)
        rDFA.meWeight = WEIGHT_LIGHT;
    else if (aPSName.indexOf("-w4") != -1)
        rDFA.meWeight = WEIGHT_SEMILIGHT;
    else if (aPSName.indexOf("-w5") != -1)
        rDFA.meWeight = WEIGHT_NORMAL;
    else if (aPSName.indexOf("-w6") != -1)
        rDFA.meWeight = WEIGHT_SEMIBOLD;
    else if (aPSName.indexOf("-w7") != -1)
        rDFA.meWeight = WEIGHT_BOLD;
    else if (aPSName.indexOf("-w8") != -1)
        rDFA.meWeight = WEIGHT_ULTRABOLD;
    else if (aPSName.indexOf("-w9") != -1)
        rDFA.meWeight = WEIGHT_BLACK;

    // heuristics for font slant
    if( (aPSName.indexOf("italic") != -1)
    ||  (aPSName.indexOf(" ital") != -1)
    ||  (aPSName.indexOf("cursive") != -1)
    ||  (aPSName.indexOf("-it") != -1)
    ||  (aPSName.indexOf("lightit") != -1)
    ||  (aPSName.indexOf("mediumit") != -1)
    ||  (aPSName.indexOf("boldit") != -1)
    ||  (aPSName.indexOf("cnit") != -1)
    ||  (aPSName.indexOf("bdcn") != -1)
    ||  (aPSName.indexOf("bdit") != -1)
    ||  (aPSName.indexOf("condit") != -1)
    ||  (aPSName.indexOf("bookit") != -1)
    ||  (aPSName.indexOf("blackit") != -1) )
        rDFA.meItalic = ITALIC_NORMAL;
    if( (aPSName.indexOf("oblique") != -1)
    ||  (aPSName.indexOf("inclined") != -1)
    ||  (aPSName.indexOf("slanted") != -1) )
        rDFA.meItalic = ITALIC_OBLIQUE;

    // heuristics for font width
    if( (aPSName.indexOf("condensed") != -1)
    ||  (aPSName.indexOf("-cond") != -1)
    ||  (aPSName.indexOf("boldcond") != -1)
    ||  (aPSName.indexOf("boldcn") != -1)
    ||  (aPSName.indexOf("cnit") != -1) )
        rDFA.meWidthType = WIDTH_CONDENSED;
    else if (aPSName.indexOf("narrow") != -1)
        rDFA.meWidthType = WIDTH_SEMI_CONDENSED;
    else if (aPSName.indexOf("expanded") != -1)
        rDFA.meWidthType = WIDTH_EXPANDED;
    else if (aPSName.indexOf("wide") != -1)
        rDFA.meWidthType = WIDTH_EXPANDED;

    // heuristics for font pitch
    if( (aPSName.indexOf("mono") != -1)
    ||  (aPSName.indexOf("courier") != -1)
    ||  (aPSName.indexOf("monaco") != -1)
    ||  (aPSName.indexOf("typewriter") != -1) )
        rDFA.mePitch = PITCH_FIXED;

    // heuristics for font family type
    if( (aPSName.indexOf("script") != -1)
    ||  (aPSName.indexOf("chancery") != -1)
    ||  (aPSName.indexOf("zapfino") != -1))
        rDFA.meFamily = FAMILY_SCRIPT;
    else if( (aPSName.indexOf("comic") != -1)
    ||  (aPSName.indexOf("outline") != -1)
    ||  (aPSName.indexOf("pinpoint") != -1) )
        rDFA.meFamily = FAMILY_DECORATIVE;
    else if( (aPSName.indexOf("sans") != -1)
    ||  (aPSName.indexOf("arial") != -1) )
        rDFA.meFamily = FAMILY_SWISS;
    else if( (aPSName.indexOf("roman") != -1)
    ||  (aPSName.indexOf("times") != -1) )
        rDFA.meFamily = FAMILY_ROMAN;

    // heuristics for codepoint semantic
    if( (aPSName.indexOf("symbol") != -1)
    ||  (aPSName.indexOf("dings") != -1)
    ||  (aPSName.indexOf("dingbats") != -1)
    ||  (aPSName.indexOf("ornaments") != -1)
    ||  (aPSName.indexOf("embellishments") != -1) )
        rDFA.mbSymbolFlag  = true;

   // #i100020# special heuristic for names with single-char styles
   // NOTE: we are checking name that hasn't been lower-cased
   if( rPSName.Len() > 3 )
   {
        int i = rPSName.Len();
        sal_Unicode c = rPSName.GetChar( --i );
        if( c == 'C' ) { // "capitals"
            rDFA.meFamily = FAMILY_DECORATIVE;
            c = rPSName.GetChar( --i );
        }
        if( c == 'O' ) { // CFF-based OpenType
            c = rPSName.GetChar( --i );
        }
        if( c == 'I' ) { // "italic"
            rDFA.meItalic = ITALIC_NORMAL;
            c = rPSName.GetChar( --i );
        }
        if( c == 'B' )   // "bold"
            rDFA.meWeight = WEIGHT_BOLD;
        if( c == 'C' )   // "capitals"
            rDFA.meFamily = FAMILY_DECORATIVE;
        // TODO: check that all single-char styles have been resolved?
    }
}

static bool GetDevFontAttributes( CTFontRef nFontRef, ImplDevFontAttributes& rDFA )
{
    // all CT fonts are device fonts that can be directly rotated
    rDFA.mbOrientation = true;
    rDFA.mbDevice      = true;
    rDFA.mnQuality     = 0;

    // reset the attributes
    rDFA.meFamily     = FAMILY_DONTKNOW;
    rDFA.mePitch      = PITCH_VARIABLE;
    rDFA.meWidthType  = WIDTH_NORMAL;
    rDFA.meWeight     = WEIGHT_NORMAL;
    rDFA.meItalic     = ITALIC_NONE;
    rDFA.mbSymbolFlag = false;

    // Implement...

    return false;
}

// =======================================================================

SystemFontList::SystemFontList()
{
}

SystemFontList::~SystemFontList()
{
}

void SystemFontList::AnnounceFonts( ImplDevFontList& rFontList ) const
{
}

// not all fonts are suitable for glyph fallback => sort them
struct GfbCompare{ bool operator()(const ImplIosFontData*, const ImplIosFontData*); };

inline bool GfbCompare::operator()( const ImplIosFontData* pA, const ImplIosFontData* pB )
{
    // use symbol fonts only as last resort
    bool bPreferA = !pA->IsSymbolFont();
    bool bPreferB = !pB->IsSymbolFont();
    if( bPreferA != bPreferB )
        return bPreferA;
    // prefer scalable fonts
    bPreferA = pA->IsScalable();
    bPreferB = pB->IsScalable();
    if( bPreferA != bPreferB )
        return bPreferA;
    // prefer non-slanted fonts
    bPreferA = (pA->GetSlant() == ITALIC_NONE);
    bPreferB = (pB->GetSlant() == ITALIC_NONE);
    if( bPreferA != bPreferB )
        return bPreferA;
    // prefer normal weight fonts
    bPreferA = (pA->GetWeight() == WEIGHT_NORMAL);
    bPreferB = (pB->GetWeight() == WEIGHT_NORMAL);
    if( bPreferA != bPreferB )
        return bPreferA;
    // prefer normal width fonts
    bPreferA = (pA->GetWidthType() == WIDTH_NORMAL);
    bPreferB = (pB->GetWidthType() == WIDTH_NORMAL);
    if( bPreferA != bPreferB )
        return bPreferA;
    return false;
}

void SystemFontList::InitGlyphFallbacks()
{
}

ImplIosFontData* SystemFontList::GetFontDataFromRef( CTFontRef nFontRef ) const
{
    IosFontContainer::const_iterator it = maFontContainer.find( nFontRef );
    if( it == maFontContainer.end() )
        return NULL;
    return (*it).second;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
