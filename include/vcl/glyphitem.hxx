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

#ifndef INCLUDED_VCL_GLYPHITEM_HXX
#define INCLUDED_VCL_GLYPHITEM_HXX

#include <rtl/ref.hxx>
#include <tools/gen.hxx>
#include <vcl/dllapi.h>

class LogicalFontInstance;

typedef sal_uInt16 sal_GlyphId;

struct VCL_DLLPUBLIC GlyphItem
{
    int m_nFlags;
    int m_nCharPos; // index in string
    int m_nCharCount; // number of characters m_aking up this glyph

    int m_nOrigWidth; // original glyph width
    int m_nNewWidth; // width after adjustments
    int m_nXOffset;

    sal_GlyphId m_aGlyphId;
    Point m_aLinearPos; // absolute position of non rotated string

    LogicalFontInstance* m_pFontInstance;

    GlyphItem(int nCharPos, int nCharCount, sal_GlyphId aGlyphId, const Point& rLinearPos,
              long nFlags, int nOrigWidth, int nXOffset, LogicalFontInstance* pFontInstance)
        : m_nFlags(nFlags)
        , m_nCharPos(nCharPos)
        , m_nCharCount(nCharCount)
        , m_nOrigWidth(nOrigWidth)
        , m_nNewWidth(nOrigWidth)
        , m_nXOffset(nXOffset)
        , m_aGlyphId(aGlyphId)
        , m_aLinearPos(rLinearPos)
        , m_pFontInstance(pFontInstance)
    {
        assert(pFontInstance);
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
};

#endif // INCLUDED_VCL_GLYPHITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
