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


#include "pdffontcache.hxx"
#include <salgdi.hxx>
#include <outfont.hxx>
#include <sallayout.hxx>

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
