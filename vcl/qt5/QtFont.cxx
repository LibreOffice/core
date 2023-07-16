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

#include <sal/config.h>

#include <QtFont.hxx>
#include <QtTools.hxx>

#include <QtGui/QFont>
#include <QtGui/QRawFont>
#include <QtGui/QPainterPath>

static inline void applyWeight(QtFont& rFont, FontWeight eWeight)
{
    switch (eWeight)
    {
        case WEIGHT_THIN:
            rFont.setWeight(QFont::Thin);
            break;
        case WEIGHT_ULTRALIGHT:
            rFont.setWeight(QFont::ExtraLight);
            break;
        case WEIGHT_LIGHT:
            rFont.setWeight(QFont::Light);
            break;
        case WEIGHT_SEMILIGHT:
            [[fallthrough]];
        case WEIGHT_NORMAL:
            rFont.setWeight(QFont::Normal);
            break;
        case WEIGHT_MEDIUM:
            rFont.setWeight(QFont::Medium);
            break;
        case WEIGHT_SEMIBOLD:
            rFont.setWeight(QFont::DemiBold);
            break;
        case WEIGHT_BOLD:
            rFont.setWeight(QFont::Bold);
            break;
        case WEIGHT_ULTRABOLD:
            rFont.setWeight(QFont::ExtraBold);
            break;
        case WEIGHT_BLACK:
            rFont.setWeight(QFont::Black);
            break;
        default:
            break;
    }
}

static inline void applyStretch(QtFont& rFont, FontWidth eWidthType)
{
    switch (eWidthType)
    {
        case WIDTH_DONTKNOW:
            rFont.setStretch(QFont::AnyStretch);
            break;
        case WIDTH_ULTRA_CONDENSED:
            rFont.setStretch(QFont::UltraCondensed);
            break;
        case WIDTH_EXTRA_CONDENSED:
            rFont.setStretch(QFont::ExtraCondensed);
            break;
        case WIDTH_CONDENSED:
            rFont.setStretch(QFont::Condensed);
            break;
        case WIDTH_SEMI_CONDENSED:
            rFont.setStretch(QFont::SemiCondensed);
            break;
        case WIDTH_NORMAL:
            rFont.setStretch(QFont::Unstretched);
            break;
        case WIDTH_SEMI_EXPANDED:
            rFont.setStretch(QFont::SemiExpanded);
            break;
        case WIDTH_EXPANDED:
            rFont.setStretch(QFont::Expanded);
            break;
        case WIDTH_EXTRA_EXPANDED:
            rFont.setStretch(QFont::ExtraExpanded);
            break;
        case WIDTH_ULTRA_EXPANDED:
            rFont.setStretch(QFont::UltraExpanded);
            break;
        default:
            break;
    }
}

static inline void applyStyle(QtFont& rFont, FontItalic eItalic)
{
    switch (eItalic)
    {
        case ITALIC_NONE:
            rFont.setStyle(QFont::Style::StyleNormal);
            break;
        case ITALIC_OBLIQUE:
            rFont.setStyle(QFont::Style::StyleOblique);
            break;
        case ITALIC_NORMAL:
            rFont.setStyle(QFont::Style::StyleItalic);
            break;
        default:
            break;
    }
}

QtFont::QtFont(const vcl::font::PhysicalFontFace& rPFF, const vcl::font::FontSelectPattern& rFSP)
    : LogicalFontInstance(rPFF, rFSP)
{
    setFamily(toQString(rPFF.GetFamilyName()));
    applyWeight(*this, rPFF.GetWeight());
    setPixelSize(rFSP.mnHeight);
    applyStretch(*this, rPFF.GetWidthType());
    applyStyle(*this, rFSP.GetItalic());
}

bool QtFont::GetGlyphOutline(sal_GlyphId nId, basegfx::B2DPolyPolygon& rB2DPolyPoly, bool) const
{
    rB2DPolyPoly.clear();
    basegfx::B2DPolygon aPart;
    QRawFont aRawFont(QRawFont::fromFont(*this));
    QPainterPath aQPath = aRawFont.pathForGlyph(nId);

    for (int a(0); a < aQPath.elementCount(); a++)
    {
        const QPainterPath::Element aQElement = aQPath.elementAt(a);

        switch (aQElement.type)
        {
            case QPainterPath::MoveToElement:
            {
                if (aPart.count())
                {
                    aPart.setClosed(true);
                    rB2DPolyPoly.append(aPart);
                    aPart.clear();
                }

                aPart.append(basegfx::B2DPoint(aQElement.x, aQElement.y));
                break;
            }
            case QPainterPath::LineToElement:
            {
                aPart.append(basegfx::B2DPoint(aQElement.x, aQElement.y));
                break;
            }
            case QPainterPath::CurveToElement:
            {
                const QPainterPath::Element aQ2 = aQPath.elementAt(++a);
                const QPainterPath::Element aQ3 = aQPath.elementAt(++a);
                aPart.appendBezierSegment(basegfx::B2DPoint(aQElement.x, aQElement.y),
                                          basegfx::B2DPoint(aQ2.x, aQ2.y),
                                          basegfx::B2DPoint(aQ3.x, aQ3.y));
                break;
            }
            case QPainterPath::CurveToDataElement:
            {
                break;
            }
        }
    }

    if (aPart.count())
    {
        aPart.setClosed(true);
        rB2DPolyPoly.append(aPart);
        aPart.clear();
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
