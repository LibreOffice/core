/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pdffontcache.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-04 08:05:28 $
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
#include "salgdi.hxx"
#include "outfont.hxx"
#include "sallayout.hxx"

using namespace vcl;

PDFFontCache::FontIdentifier::FontIdentifier( ImplFontData* pFont, bool bVertical ) :
    m_nFontId( pFont->GetFontId() ),
    m_nMagic( pFont->GetFontMagic() ),
    m_bVertical( bVertical )
{
}

PDFFontCache::FontData& PDFFontCache::getFont( ImplFontData* pFont, bool bVertical )
{
    FontIdentifier aId( pFont, bVertical );
    FontToIndexMap::iterator it = m_aFontToIndex.find( aId );
    if( it != m_aFontToIndex.end() )
        return m_aFonts[ it->second ];
    m_aFontToIndex[ aId ] = sal_uInt32(m_aFonts.size());
    m_aFonts.push_back( FontData() );
    return m_aFonts.back();
}

sal_Int32 PDFFontCache::getGlyphWidth( ImplFontData* pFont, sal_uInt32 nGlyph, bool bVertical, SalGraphics* pGraphics )
{
    sal_Int32 nWidth = 0;
    FontData& rFontData( getFont( pFont, bVertical ) );
    if( rFontData.m_nWidths.empty() )
    {
        pGraphics->GetGlyphWidths( pFont, bVertical, rFontData.m_nWidths, rFontData.m_aGlyphIdToIndex );
    }
    if( ! rFontData.m_nWidths.empty() )
    {
        sal_uInt32 nIndex = nGlyph;
        if( (nGlyph & GF_ISCHAR) != 0 )
        {
            std::map<sal_Unicode,sal_uInt32>::const_iterator it =
                rFontData.m_aGlyphIdToIndex.find( sal_Unicode(nGlyph & GF_IDXMASK) );
                nIndex = (it != rFontData.m_aGlyphIdToIndex.end()) ? it->second : 0;
        }
        nIndex &= GF_IDXMASK;
        if( nIndex < rFontData.m_nWidths.size() )
            nWidth = rFontData.m_nWidths[ nIndex ];
    }
    return nWidth;
}
