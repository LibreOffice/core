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

enum class GlyphItemFlags : sal_uInt8
{
    NONE = 0,
    IS_IN_CLUSTER = 0x01,
    IS_RTL_GLYPH = 0x02,
    IS_DIACRITIC = 0x04,
    IS_VERTICAL = 0x08,
    IS_SPACING = 0x10,
    ALLOW_KASHIDA = 0x20,
    IS_DROPPED = 0x40,
    IS_CLUSTER_START = 0x80
};
namespace o3tl
{
template <> struct typed_flags<GlyphItemFlags> : is_typed_flags<GlyphItemFlags, 0xff>
{
};
};

class VCL_DLLPUBLIC GlyphItem
{
    sal_Int32 m_nOrigWidth; // original glyph width
    sal_Int32 m_nCharPos; // index in string
    sal_Int32 m_nXOffset;
    sal_GlyphId m_aGlyphId;
    sal_Int8 m_nCharCount; // number of characters making up this glyph
    GlyphItemFlags m_nFlags;

public:
    Point m_aLinearPos; // absolute position of non rotated string
    sal_Int32 m_nNewWidth; // width after adjustments

    GlyphItem(int nCharPos, int nCharCount, sal_GlyphId aGlyphId, const Point& rLinearPos,
              GlyphItemFlags nFlags, int nOrigWidth, int nXOffset)
        : m_nOrigWidth(nOrigWidth)
        , m_nCharPos(nCharPos)
        , m_nXOffset(nXOffset)
        , m_aGlyphId(aGlyphId)
        , m_nCharCount(nCharCount)
        , m_nFlags(nFlags)
        , m_aLinearPos(rLinearPos)
        , m_nNewWidth(nOrigWidth)
    {
    }

    bool IsInCluster() const { return bool(m_nFlags & GlyphItemFlags::IS_IN_CLUSTER); }
    bool IsRTLGlyph() const { return bool(m_nFlags & GlyphItemFlags::IS_RTL_GLYPH); }
    bool IsDiacritic() const { return bool(m_nFlags & GlyphItemFlags::IS_DIACRITIC); }
    bool IsVertical() const { return bool(m_nFlags & GlyphItemFlags::IS_VERTICAL); }
    bool IsSpacing() const { return bool(m_nFlags & GlyphItemFlags::IS_SPACING); }
    bool AllowKashida() const { return bool(m_nFlags & GlyphItemFlags::ALLOW_KASHIDA); }
    bool IsDropped() const { return bool(m_nFlags & GlyphItemFlags::IS_DROPPED); }
    bool IsClusterStart() const { return bool(m_nFlags & GlyphItemFlags::IS_CLUSTER_START); }

    inline bool GetGlyphBoundRect(const LogicalFontInstance*, tools::Rectangle&) const;
    inline bool GetGlyphOutline(const LogicalFontInstance*, basegfx::B2DPolyPolygon&) const;
    inline void dropGlyph();

    sal_GlyphId glyphId() const { return m_aGlyphId; }
    int charCount() const { return m_nCharCount; }
    int origWidth() const { return m_nOrigWidth; }
    int charPos() const { return m_nCharPos; }
    int xOffset() const { return m_nXOffset; }
};

VCL_DLLPUBLIC bool GlyphItem::GetGlyphBoundRect(const LogicalFontInstance* pFontInstance,
                                                tools::Rectangle& rRect) const
{
    return pFontInstance->GetGlyphBoundRect(m_aGlyphId, rRect, IsVertical());
}

VCL_DLLPUBLIC bool GlyphItem::GetGlyphOutline(const LogicalFontInstance* pFontInstance,
                                              basegfx::B2DPolyPolygon& rPoly) const
{
    return pFontInstance->GetGlyphOutline(m_aGlyphId, rPoly, IsVertical());
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
    const rtl::Reference<LogicalFontInstance>& GetFont() const { return m_rFontInstance; }
    bool IsValid() const;

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
