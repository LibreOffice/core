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

#ifndef INCLUDED_VCL_IMPGLYPHITEM_HXX
#define INCLUDED_VCL_IMPGLYPHITEM_HXX

#include <tools/gen.hxx>
#include <vcl/dllapi.h>
#include <vcl/glyphitem.hxx>
#include <vector>

#include "fontinstance.hxx"

struct VCL_DLLPUBLIC GlyphItem
{
    sal_GlyphId m_aGlyphId;
    int m_nCharCount; // number of characters making up this glyph
    int m_nOrigWidth; // original glyph width
    LogicalFontInstance* m_pFontInstance;

    int m_nCharPos; // index in string
    int m_nFlags;
    int m_nNewWidth; // width after adjustments
    int m_nXOffset;
    Point m_aLinearPos; // absolute position of non rotated string

    GlyphItem(int nCharPos, int nCharCount, sal_GlyphId aGlyphId, const Point& rLinearPos,
              long nFlags, int nOrigWidth, int nXOffset, LogicalFontInstance* pFontInstance)
        : m_aGlyphId(aGlyphId)
        , m_nCharCount(nCharCount)
        , m_nOrigWidth(nOrigWidth)
        , m_pFontInstance(pFontInstance)
        , m_nCharPos(nCharPos)
        , m_nFlags(nFlags)
        , m_nNewWidth(nOrigWidth)
        , m_nXOffset(nXOffset)
        , m_aLinearPos(rLinearPos)
    {
        assert(m_pFontInstance);
    }

    enum
    {
        IS_IN_CLUSTER = 0x001,
        IS_RTL_GLYPH = 0x002,
        IS_DIACRITIC = 0x004,
        IS_VERTICAL = 0x008,
        IS_SPACING = 0x010,
        ALLOW_KASHIDA = 0x020,
        IS_DROPPED = 0x040,
        IS_CLUSTER_START = 0x080
    };

    bool IsInCluster() const { return ((m_nFlags & IS_IN_CLUSTER) != 0); }
    bool IsRTLGlyph() const { return ((m_nFlags & IS_RTL_GLYPH) != 0); }
    bool IsDiacritic() const { return ((m_nFlags & IS_DIACRITIC) != 0); }
    bool IsVertical() const { return ((m_nFlags & IS_VERTICAL) != 0); }
    bool IsSpacing() const { return ((m_nFlags & IS_SPACING) != 0); }
    bool AllowKashida() const { return ((m_nFlags & ALLOW_KASHIDA) != 0); }
    bool IsDropped() const { return ((m_nFlags & IS_DROPPED) != 0); }
    bool IsClusterStart() const { return ((m_nFlags & IS_CLUSTER_START) != 0); }

    inline bool GetGlyphBoundRect(tools::Rectangle&) const;
    inline bool GetGlyphOutline(basegfx::B2DPolyPolygon&) const;
};

VCL_DLLPUBLIC bool GlyphItem::GetGlyphBoundRect(tools::Rectangle& rRect) const
{
    return m_pFontInstance->GetGlyphBoundRect(m_aGlyphId, rRect, IsVertical());
}

VCL_DLLPUBLIC bool GlyphItem::GetGlyphOutline(basegfx::B2DPolyPolygon& rPoly) const
{
    return m_pFontInstance->GetGlyphOutline(m_aGlyphId, rPoly, IsVertical());
}

class SalLayoutGlyphsImpl : public std::vector<GlyphItem>
{
    friend class GenericSalLayout;

public:
    ~SalLayoutGlyphsImpl();
    SalLayoutGlyphsImpl* clone(SalLayoutGlyphs& rGlyphs) const;
    LogicalFontInstance& GetFont() const { return *m_rFontInstance; }
    bool IsValid() const;
    void Invalidate();

private:
    mutable rtl::Reference<LogicalFontInstance> m_rFontInstance;

    SalLayoutGlyphsImpl(SalLayoutGlyphs& rGlyphs, LogicalFontInstance& rFontInstance)
        : m_rFontInstance(&rFontInstance)
    {
        rGlyphs.m_pImpl = this;
    }
};

#endif // INCLUDED_VCL_IMPGLYPHITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
