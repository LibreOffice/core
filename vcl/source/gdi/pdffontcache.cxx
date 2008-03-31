/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pdffontcache.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-31 13:25:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "precompiled_vcl.hxx"

#include "pdffontcache.hxx"
#include <vcl/salgdi.hxx>
#include <vcl/outfont.hxx>
#include <vcl/sallayout.hxx>

using namespace vcl;

PDFFontCache::FontIdentifier::FontIdentifier( const ImplFontData* pFont, bool bVertical ) :
    m_nFontId( pFont->GetFontId() ),
    m_nMagic( pFont->GetFontMagic() ),
    m_bVertical( bVertical )
{
}

PDFFontCache::FontData& PDFFontCache::getFont( const ImplFontData* pFont, bool bVertical )
{
    FontIdentifier aId( pFont, bVertical );
    FontToIndexMap::iterator it = m_aFontToIndex.find( aId );
    if( it != m_aFontToIndex.end() )
        return m_aFonts[ it->second ];
    m_aFontToIndex[ aId ] = sal_uInt32(m_aFonts.size());
    m_aFonts.push_back( FontData() );
    return m_aFonts.back();
}

sal_Int32 PDFFontCache::getGlyphWidth( const ImplFontData* pFont, sal_GlyphId nGlyph, bool bVertical, SalGraphics* pGraphics )
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

