/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>
#include <cassert>

#include "premac.h"
#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>
#include "postmac.h"

#include "CTRunData.hxx"

CTRunData::CTRunData( CTRunRef pRun, int start)
    : ownership_flags(0)
    , m_StartPos(start)
    , m_pRun(pRun)
{
    assert(pRun);

    CFDictionaryRef pRunAttributes = CTRunGetAttributes( m_pRun );
    m_pFont = (CTFontRef)CFDictionaryGetValue( pRunAttributes, kCTFontAttributeName );

    m_nGlyphs = CTRunGetGlyphCount(m_pRun);
    m_EndPos = m_StartPos + m_nGlyphs;
    const CFRange aAll = CFRangeMake( 0, m_nGlyphs );

    m_pAdvances = CTRunGetAdvancesPtr( pRun );
    if( !m_pAdvances )
    {
        m_pAdvances = new CGSize[m_nGlyphs];
        ownership_flags |= CTRUNDATA_F_OWN_ADVANCES;
        CTRunGetAdvances( pRun, aAll, (CGSize*)m_pAdvances );
    }

    m_pGlyphs = CTRunGetGlyphsPtr( m_pRun );
    if( !m_pGlyphs )
    {
        m_pGlyphs = new CGGlyph[m_nGlyphs];
        ownership_flags |= CTRUNDATA_F_OWN_GLYPHS;
        CTRunGetGlyphs( pRun, aAll, (CGGlyph*)m_pGlyphs);
    }

    m_pStringIndices = CTRunGetStringIndicesPtr( pRun );
    if( !m_pStringIndices )
    {
        m_pStringIndices = new CFIndex[m_nGlyphs];
        ownership_flags |= CTRUNDATA_F_OWN_INDICES;
        CTRunGetStringIndices( pRun, aAll, (CFIndex*)m_pStringIndices );
    }

    m_pPositions = CTRunGetPositionsPtr( pRun );
    if( !m_pPositions )
    {
        m_pPositions = new CGPoint[m_nGlyphs];
        ownership_flags |= CTRUNDATA_F_OWN_POSITIONS;
        CTRunGetPositions( pRun, aAll, (CGPoint*)m_pPositions );
    }
}

CTRunData::~CTRunData()
{
    if(ownership_flags & CTRUNDATA_F_OWN_ADVANCES)
    {
        delete [] m_pAdvances;
    }

    if(ownership_flags & CTRUNDATA_F_OWN_GLYPHS)
    {
        delete [] m_pGlyphs;
    }

    if(ownership_flags & CTRUNDATA_F_OWN_INDICES)
    {
        delete [] m_pStringIndices;
    }

    if(ownership_flags & CTRUNDATA_F_OWN_POSITIONS)
    {
        delete [] m_pPositions;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
