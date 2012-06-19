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

#ifndef VCL_PDFFONTCACHE_HXX
#define VCL_PDFFONTCACHE_HXX

#include <sal/types.h>

#include <sallayout.hxx>
#include <salgdi.hxx>

namespace vcl
{
    class PDFFontCache
    {
        struct FontIdentifier
        {
            sal_IntPtr      m_nFontId;
            int             m_nMagic;
            bool            m_bVertical;

            FontIdentifier( const PhysicalFontFace*, bool bVertical );
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
            Int32Vector  m_nWidths;
            Ucs2UIntMap  m_aGlyphIdToIndex;
        };
        typedef std::map< FontIdentifier, sal_uInt32 > FontToIndexMap;

        std::vector< FontData >     m_aFonts;
        FontToIndexMap              m_aFontToIndex;

        FontData& getFont( const PhysicalFontFace*, bool bVertical );
        public:
        PDFFontCache() {}
        ~PDFFontCache() {}

        sal_Int32 getGlyphWidth( const PhysicalFontFace*, sal_GlyphId, bool bVertical, SalGraphics* );
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
