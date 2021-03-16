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

#include <o3tl/typed_flags_set.hxx>
#include <tools/gen.hxx>
#include <vcl/dllapi.h>
#include <vcl/glyphitem.hxx>
#include <vcl/outdev.hxx>
#include <vector>

#include "fontinstance.hxx"

enum class GlyphItemFlags
{
    NONE = 0,
    IS_IN_CLUSTER = 0x001,
    IS_RTL_GLYPH = 0x002,
    IS_DIACRITIC = 0x004,
    IS_VERTICAL = 0x008,
    IS_SPACING = 0x010,
    ALLOW_KASHIDA = 0x020,
    IS_DROPPED = 0x040,
    IS_CLUSTER_START = 0x080
};
namespace o3tl
{
template <> struct typed_flags<GlyphItemFlags> : is_typed_flags<GlyphItemFlags, 0xff>
{
};
};

class VCL_DLLPUBLIC GlyphItem
{
    sal_GlyphId m_aGlyphId;
    int m_nCharCount; // number of characters making up this glyph
    int m_nOrigWidth; // original glyph width
    LogicalFontInstance* m_pFontInstance;
    int m_nCharPos; // index in string
    GlyphItemFlags m_nFlags;
    int m_nXOffset;

public:
    int m_nNewWidth; // width after adjustments
    Point m_aLinearPos; // absolute position of non rotated string

    GlyphItem(int nCharPos, int nCharCount, sal_GlyphId aGlyphId, const Point& rLinearPos,
              GlyphItemFlags nFlags, int nOrigWidth, int nXOffset,
              LogicalFontInstance* pFontInstance)
        : m_aGlyphId(aGlyphId)
        , m_nCharCount(nCharCount)
        , m_nOrigWidth(nOrigWidth)
        , m_pFontInstance(pFontInstance)
        , m_nCharPos(nCharPos)
        , m_nFlags(nFlags)
        , m_nXOffset(nXOffset)
        , m_nNewWidth(nOrigWidth)
        , m_aLinearPos(rLinearPos)
    {
        assert(m_pFontInstance);
    }

    bool IsInCluster() const { return bool(m_nFlags & GlyphItemFlags::IS_IN_CLUSTER); }
    bool IsRTLGlyph() const { return bool(m_nFlags & GlyphItemFlags::IS_RTL_GLYPH); }
    bool IsDiacritic() const { return bool(m_nFlags & GlyphItemFlags::IS_DIACRITIC); }
    bool IsVertical() const { return bool(m_nFlags & GlyphItemFlags::IS_VERTICAL); }
    bool IsSpacing() const { return bool(m_nFlags & GlyphItemFlags::IS_SPACING); }
    bool AllowKashida() const { return bool(m_nFlags & GlyphItemFlags::ALLOW_KASHIDA); }
    bool IsDropped() const { return bool(m_nFlags & GlyphItemFlags::IS_DROPPED); }
    bool IsClusterStart() const { return bool(m_nFlags & GlyphItemFlags::IS_CLUSTER_START); }

    inline bool GetGlyphBoundRect(tools::Rectangle&) const;
    inline bool GetGlyphOutline(basegfx::B2DPolyPolygon&) const;
    inline void dropGlyph();

    sal_GlyphId glyphId() const { return m_aGlyphId; }
    int charCount() const { return m_nCharCount; }
    int origWidth() const { return m_nOrigWidth; }
    int charPos() const { return m_nCharPos; }
    int xOffset() const { return m_nXOffset; }
};

VCL_DLLPUBLIC bool GlyphItem::GetGlyphBoundRect(tools::Rectangle& rRect) const
{
    return m_pFontInstance->GetGlyphBoundRect(m_aGlyphId, rRect, IsVertical());
}

VCL_DLLPUBLIC bool GlyphItem::GetGlyphOutline(basegfx::B2DPolyPolygon& rPoly) const
{
    return m_pFontInstance->GetGlyphOutline(m_aGlyphId, rPoly, IsVertical());
}

void GlyphItem::dropGlyph()
{
    m_nCharPos = -1;
    m_nFlags |= GlyphItemFlags::IS_DROPPED;
}

class SalLayoutGlyphsImpl : public std::vector<GlyphItem>
{
    friend class GenericSalLayout;

public:
    SalLayoutGlyphsImpl* clone() const;
    LogicalFontInstance& GetFont() const { return *m_rFontInstance; }
    bool IsValid() const;
    void Invalidate();

private:
    rtl::Reference<LogicalFontInstance> m_rFontInstance;
    SalLayoutFlags mnFlags = SalLayoutFlags::NONE;

    SalLayoutGlyphsImpl(LogicalFontInstance& rFontInstance)
        : m_rFontInstance(&rFontInstance)
    {
    }
};

#endif // INCLUDED_VCL_IMPGLYPHITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
