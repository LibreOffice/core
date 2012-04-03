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

SystemFontList::SystemFontList()
{
}

SystemFontList::~SystemFontList()
{
}

void SystemFontList::AnnounceFonts( ImplDevFontList& rFontList ) const
{
    (void) rFontList;

    // Implement...
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
