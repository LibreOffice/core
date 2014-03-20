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

#include <sal/types.h>

#include "outfont.hxx"
#include "PhysicalFontFace.hxx"
#include "salgdi.hxx"
#include "sallayout.hxx"

#include "pdffontcache.hxx"

using namespace vcl;

PDFFontCache::FontIdentifier::FontIdentifier( const PhysicalFontFace* pFont, bool bVertical ) :
    m_nFontId( pFont->GetFontId() ),
    m_nMagic( pFont->GetFontMagic() ),
    m_bVertical( bVertical )
{
}

PDFFontCache::FontData& PDFFontCache::getFont( const PhysicalFontFace* pFont, bool bVertical )
{
    FontIdentifier aId( pFont, bVertical );
    FontToIndexMap::iterator it = m_aFontToIndex.find( aId );
    if( it != m_aFontToIndex.end() )
        return m_aFonts[ it->second ];
    m_aFontToIndex[ aId ] = sal_uInt32(m_aFonts.size());
    m_aFonts.push_back( FontData() );
    return m_aFonts.back();
}

sal_Int32 PDFFontCache::getGlyphWidth( const PhysicalFontFace* pFont, sal_GlyphId nGlyph, bool bVertical, SalGraphics* pGraphics )
{
    sal_Int32 nWidth = 0;
    FontData& rFontData( getFont( pFont, bVertical ) );
    if( rFontData.m_nWidths.empty() )
    {
        pGraphics->GetGlyphWidths( pFont, bVertical, rFontData.m_nWidths, rFontData.m_aGlyphIdToIndex );
    }
    if( ! rFontData.m_nWidths.empty() )
    {
        sal_GlyphId nIndex = nGlyph;
        if( (nGlyph & GF_ISCHAR) != 0 )
        {
            const sal_Ucs cCode = static_cast<sal_Ucs>(nGlyph & GF_IDXMASK);
            Ucs2UIntMap::const_iterator it = rFontData.m_aGlyphIdToIndex.find( cCode );

            // allow symbol aliasing U+00xx -> U+F0xx if there is no direct match
            if( it == rFontData.m_aGlyphIdToIndex.end()
            &&  pFont->IsSymbolFont()
            &&  (cCode < 0x0100) )
                it = rFontData.m_aGlyphIdToIndex.find( cCode+0xF000 );

            nIndex = (it != rFontData.m_aGlyphIdToIndex.end()) ? it->second : 0;
        }
        nIndex &= GF_IDXMASK;
        if( nIndex < rFontData.m_nWidths.size() )
            nWidth = rFontData.m_nWidths[ nIndex ];
    }
    return nWidth;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
