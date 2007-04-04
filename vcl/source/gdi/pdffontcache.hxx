/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pdffontcache.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-04 08:05:41 $
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

#ifndef VCL_PDFFONTCACHE_HXX
#define VCL_PDFFONTCACHE_HXX

#include <sal/types.h>

#include <vector>
#include <map>

class ImplFontData;
class SalGraphics;

namespace vcl
{
    class PDFFontCache
    {
        struct FontIdentifier
        {
            sal_IntPtr      m_nFontId;
            int             m_nMagic;
            bool            m_bVertical;

            FontIdentifier( ImplFontData*, bool bVertical );
            FontIdentifier() : m_nFontId(0), m_nMagic(0), m_bVertical( false ) {}

            bool operator==( const FontIdentifier& rRight ) const
            {
                return m_nFontId == rRight.m_nFontId &&
                       m_nMagic == rRight.m_nMagic &&
                       m_bVertical == rRight.m_bVertical;
            }
            bool operator<( const FontIdentifier& rRight ) const
            {
                return m_nFontId < rRight.m_nFontId ||
                       m_nMagic < rRight.m_nMagic ||
                       m_bVertical < rRight.m_bVertical;
            }
        };
        struct FontData
        {
            std::vector< sal_Int32 >                m_nWidths;
            std::map< sal_Unicode, sal_uInt32 >     m_aGlyphIdToIndex;
        };
        typedef std::map< FontIdentifier, sal_uInt32 > FontToIndexMap;

        std::vector< FontData >     m_aFonts;
        FontToIndexMap              m_aFontToIndex;

        FontData& getFont( ImplFontData*, bool bVertical );
        public:
        PDFFontCache() {}
        ~PDFFontCache() {}

        sal_Int32 getGlyphWidth( ImplFontData* pFont, sal_uInt32 nGlyphId, bool bVertical, SalGraphics* pGraphics );
    };
}

#endif
