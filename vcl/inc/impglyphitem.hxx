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
#include <vcl/outdev.hxx>
#include <vector>

#include "font/LogicalFontInstance.hxx"
#include "glyphid.hxx"

enum class GlyphItemFlags : sal_uInt8
{
    NONE = 0,
    IS_IN_CLUSTER = 0x01,
    IS_RTL_GLYPH = 0x02,
    IS_VERTICAL = 0x04,
    IS_SPACING = 0x08,
    IS_DROPPED = 0x10,
    IS_CLUSTER_START = 0x20,
    IS_UNSAFE_TO_BREAK = 0x40, // HB_GLYPH_FLAG_UNSAFE_TO_BREAK from harfbuzz
    IS_SAFE_TO_INSERT_KASHIDA = 0x80 // HB_GLYPH_FLAG_SAFE_TO_INSERT_TATWEEL from harfbuzz
};
namespace o3tl
{
template <> struct typed_flags<GlyphItemFlags> : is_typed_flags<GlyphItemFlags, 0xff>
{
};
};

class VCL_DLLPUBLIC GlyphItem
{
    DevicePoint m_aLinearPos; // absolute position of non rotated string
    DeviceCoordinate m_nOrigWidth; // original glyph width
    sal_Int32 m_nCharPos; // index in string
    sal_Int32 m_nXOffset;
    sal_Int32 m_nYOffset;
    DeviceCoordinate m_nNewWidth; // width after adjustments
    sal_GlyphId m_aGlyphId;
    GlyphItemFlags m_nFlags;
    sal_Int8 m_nCharCount; // number of characters making up this glyph

public:
    GlyphItem(int nCharPos, int nCharCount, sal_GlyphId aGlyphId, const DevicePoint& rLinearPos,
              GlyphItemFlags nFlags, DeviceCoordinate nOrigWidth, int nXOffset, int nYOffset)
        : m_aLinearPos(rLinearPos)
        , m_nOrigWidth(nOrigWidth)
        , m_nCharPos(nCharPos)
        , m_nXOffset(nXOffset)
        , m_nYOffset(nYOffset)
        , m_nNewWidth(nOrigWidth)
        , m_aGlyphId(aGlyphId)
        , m_nFlags(nFlags)
        , m_nCharCount(nCharCount)
    {
    }

    bool IsInCluster() const { return bool(m_nFlags & GlyphItemFlags::IS_IN_CLUSTER); }
    bool IsRTLGlyph() const { return bool(m_nFlags & GlyphItemFlags::IS_RTL_GLYPH); }
    bool IsVertical() const { return bool(m_nFlags & GlyphItemFlags::IS_VERTICAL); }
    bool IsSpacing() const { return bool(m_nFlags & GlyphItemFlags::IS_SPACING); }
    bool IsDropped() const { return bool(m_nFlags & GlyphItemFlags::IS_DROPPED); }
    bool IsClusterStart() const { return bool(m_nFlags & GlyphItemFlags::IS_CLUSTER_START); }
    bool IsUnsafeToBreak() const { return bool(m_nFlags & GlyphItemFlags::IS_UNSAFE_TO_BREAK); }
    bool IsSafeToInsertKashida() const
    {
        return bool(m_nFlags & GlyphItemFlags::IS_SAFE_TO_INSERT_KASHIDA);
    }

    inline bool GetGlyphBoundRect(const LogicalFontInstance*, tools::Rectangle&) const;
    inline bool GetGlyphOutline(const LogicalFontInstance*, basegfx::B2DPolyPolygon&) const;
    inline void dropGlyph();

    sal_GlyphId glyphId() const { return m_aGlyphId; }
    int charCount() const { return m_nCharCount; }
    DeviceCoordinate origWidth() const { return m_nOrigWidth; }
    int charPos() const { return m_nCharPos; }
    int xOffset() const { return m_nXOffset; }
    int yOffset() const { return m_nYOffset; }
    DeviceCoordinate newWidth() const { return m_nNewWidth; }
    const DevicePoint& linearPos() const { return m_aLinearPos; }

    void setNewWidth(DeviceCoordinate width) { m_nNewWidth = width; }
    void addNewWidth(DeviceCoordinate width) { m_nNewWidth += width; }
    void setLinearPos(const DevicePoint& point) { m_aLinearPos = point; }
    void setLinearPosX(double x) { m_aLinearPos.setX(x); }
    void adjustLinearPosX(double diff) { m_aLinearPos.adjustX(diff); }
    bool isLayoutEquivalent(const GlyphItem& other) const
    {
        return m_aLinearPos == other.m_aLinearPos && m_nOrigWidth == other.m_nOrigWidth
               && m_nCharPos == other.m_nCharPos && m_nXOffset == other.m_nXOffset
               && m_nYOffset == other.m_nYOffset && m_nNewWidth == other.m_nNewWidth
               && m_aGlyphId == other.m_aGlyphId && m_nCharCount == other.m_nCharCount
               && (m_nFlags & ~GlyphItemFlags::IS_UNSAFE_TO_BREAK)
                      == (other.m_nFlags & ~GlyphItemFlags::IS_UNSAFE_TO_BREAK);
    }
};

bool GlyphItem::GetGlyphBoundRect(const LogicalFontInstance* pFontInstance,
                                  tools::Rectangle& rRect) const
{
    return pFontInstance->GetGlyphBoundRect(m_aGlyphId, rRect, IsVertical());
}

bool GlyphItem::GetGlyphOutline(const LogicalFontInstance* pFontInstance,
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
public:
    SalLayoutGlyphsImpl(LogicalFontInstance& rFontInstance)
        : m_rFontInstance(&rFontInstance)
    {
    }
    SalLayoutGlyphsImpl* clone() const;
    SalLayoutGlyphsImpl* cloneCharRange(sal_Int32 index, sal_Int32 length) const;
    const rtl::Reference<LogicalFontInstance>& GetFont() const { return m_rFontInstance; }
    bool IsValid() const;
    void SetFlags(SalLayoutFlags flags) { mnFlags = flags; }
    SalLayoutFlags GetFlags() const { return mnFlags; }
#ifdef DBG_UTIL
    bool isLayoutEquivalent(const SalLayoutGlyphsImpl* other) const;
#endif

private:
    bool isSafeToBreak(const_iterator pos, bool rtl) const;
    rtl::Reference<LogicalFontInstance> m_rFontInstance;
    SalLayoutFlags mnFlags = SalLayoutFlags::NONE;
};

#endif // INCLUDED_VCL_IMPGLYPHITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
