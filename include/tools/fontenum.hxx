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
#ifndef INCLUDED_TOOLS_FONTENUM_HXX
#define INCLUDED_TOOLS_FONTENUM_HXX

#include <sal/types.h>
#include <sal/log.hxx>
#include <o3tl/typed_flags_set.hxx>

#include <ostream>

enum FontFamily { FAMILY_DONTKNOW, FAMILY_DECORATIVE, FAMILY_MODERN,
                  FAMILY_ROMAN, FAMILY_SCRIPT, FAMILY_SWISS, FAMILY_SYSTEM, FontFamily_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, FontFamily const& family)
{
    switch (family)
    {
        case FAMILY_DONTKNOW:
            return stream << "unknown";
            break;

        case FAMILY_DECORATIVE:
            return stream << "decorative";
            break;

        case FAMILY_MODERN:
            return stream << "modern";
            break;

        case FAMILY_ROMAN:
            return stream << "roman";
            break;

        case FAMILY_SCRIPT:
            return stream << "script";
            break;

        case FAMILY_SWISS:
            return stream << "swiss";
            break;

        case FAMILY_SYSTEM:
            return stream << "system";
            break;

        default:
            SAL_WARN("vcl.gdi", "FontFamily out of bounds");
            assert(false && "FontFamily out of bounds");
            break;
    }
}

enum FontPitch { PITCH_DONTKNOW, PITCH_FIXED, PITCH_VARIABLE, FontPitch_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, FontPitch const& pitch)
{
    switch (pitch)
    {
        case PITCH_DONTKNOW:
            return stream << "unknown";
            break;

        case PITCH_FIXED:
            return stream << "fixed";
            break;

        case PITCH_VARIABLE:
            return stream << "variable";
            break;

        default:
            SAL_WARN("vcl.gdi", "FontPitch out of bounds");
            assert(false && "FontPitch out of bounds");
            break;
    }
}

enum TextAlign { ALIGN_TOP, ALIGN_BASELINE, ALIGN_BOTTOM, TextAlign_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, TextAlign const& align)
{
    switch (align)
    {
        case ALIGN_TOP:
            return stream << "top";
            break;

        case ALIGN_BASELINE:
            return stream << "baseline";
            break;

        case ALIGN_BOTTOM:
            return stream << "bottom";
            break;

        default:
            SAL_WARN("vcl.gdi", "TextAlign out of bounds");
            assert(false && "TextAlign out of bounds");
            break;
    }
}

enum FontWeight { WEIGHT_DONTKNOW, WEIGHT_THIN, WEIGHT_ULTRALIGHT,
                  WEIGHT_LIGHT, WEIGHT_SEMILIGHT, WEIGHT_NORMAL,
                  WEIGHT_MEDIUM, WEIGHT_SEMIBOLD, WEIGHT_BOLD,
                  WEIGHT_ULTRABOLD, WEIGHT_BLACK, FontWeight_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, FontWeight const& weight)
{
    switch (weight)
    {
        case WEIGHT_DONTKNOW:
            return stream << "unknown";
            break;

        case WEIGHT_THIN:
            return stream << "thin";
            break;

        case WEIGHT_ULTRALIGHT:
            return stream << "ultralight";
            break;

        case WEIGHT_LIGHT:
            return stream << "light";
            break;

        case WEIGHT_SEMILIGHT:
            return stream << "semilight";
            break;

        case WEIGHT_NORMAL:
            return stream << "normal";
            break;

        case WEIGHT_MEDIUM:
            return stream << "medium";
            break;

        case WEIGHT_SEMIBOLD:
            return stream << "semibold";
            break;

        case WEIGHT_BOLD:
            return stream << "bold";
            break;

        case WEIGHT_ULTRABOLD:
            return stream << "ultrabold";
            break;

        case WEIGHT_BLACK:
            return stream << "black";
            break;

        default:
            SAL_WARN("vcl.gdi", "FontWeight out of bounds");
            assert(false && "FontWeight out of bounds");
            break;
    }
}

enum FontWidth { WIDTH_DONTKNOW, WIDTH_ULTRA_CONDENSED, WIDTH_EXTRA_CONDENSED,
                 WIDTH_CONDENSED, WIDTH_SEMI_CONDENSED, WIDTH_NORMAL,
                 WIDTH_SEMI_EXPANDED, WIDTH_EXPANDED, WIDTH_EXTRA_EXPANDED,
                 WIDTH_ULTRA_EXPANDED,
                 FontWidth_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, FontWidth const& width)
{
    switch (width)
    {
        case WIDTH_DONTKNOW:
            return stream << "unknown";
            break;

        case WIDTH_ULTRA_CONDENSED:
            return stream << "ultra condensed";
            break;

        case WIDTH_EXTRA_CONDENSED:
            return stream << "extra ultra condensed";
            break;

        case WIDTH_CONDENSED:
            return stream << "condensed";
            break;

        case WIDTH_SEMI_CONDENSED:
            return stream << "semi condensed";
            break;

        case WIDTH_NORMAL:
            return stream << "normal";
            break;

        case WIDTH_SEMI_EXPANDED:
            return stream << "semi expanded";
            break;

        case WIDTH_EXPANDED:
            return stream << "expanded";
            break;

        case WIDTH_EXTRA_EXPANDED:
            return stream << "extra expanded";
            break;

        case WIDTH_ULTRA_EXPANDED:
            return stream << "ultra expanded";
            break;

        default:
            SAL_WARN("vcl.gdi", "FontWidth out of bounds");
            assert(false && "FontWidth out of bounds");
            break;
    }
}

enum FontItalic { ITALIC_NONE, ITALIC_OBLIQUE, ITALIC_NORMAL, ITALIC_DONTKNOW, FontItalic_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, FontItalic const& italic)
{
    switch (italic)
    {
        case ITALIC_DONTKNOW:
            return stream << "unknown";
            break;

        case ITALIC_OBLIQUE:
            return stream << "oblique";
            break;

        case ITALIC_NORMAL:
            return stream << "normal";
            break;

        case ITALIC_NONE:
            return stream << "none";
            break;

        default:
            SAL_WARN("vcl.gdi", "FontItalic out of bounds");
            assert(false && "FontItalic out of bounds");
            break;
    }
}

enum FontLineStyle { LINESTYLE_NONE, LINESTYLE_SINGLE, LINESTYLE_DOUBLE,
                     LINESTYLE_DOTTED, LINESTYLE_DONTKNOW,
                     LINESTYLE_DASH, LINESTYLE_LONGDASH,
                     LINESTYLE_DASHDOT, LINESTYLE_DASHDOTDOT,
                     LINESTYLE_SMALLWAVE,
                     LINESTYLE_WAVE, LINESTYLE_DOUBLEWAVE,
                     LINESTYLE_BOLD, LINESTYLE_BOLDDOTTED,
                     LINESTYLE_BOLDDASH, LINESTYLE_BOLDLONGDASH,
                     LINESTYLE_BOLDDASHDOT, LINESTYLE_BOLDDASHDOTDOT,
                     LINESTYLE_BOLDWAVE,
                     FontLineStyle_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, FontLineStyle const& linestyle)
{
    switch (linestyle)
    {
        case LINESTYLE_NONE:
            return stream << "none";
            break;

        case LINESTYLE_SINGLE:
            return stream << "single";
            break;

        case LINESTYLE_DOUBLE:
            return stream << "double";
            break;

        case LINESTYLE_DOTTED:
            return stream << "dotted";
            break;

        case LINESTYLE_DONTKNOW:
            return stream << "unknown";
            break;

        case LINESTYLE_DASH:
            return stream << "dash";
            break;

        case LINESTYLE_LONGDASH:
            return stream << "long dash";
            break;

        case LINESTYLE_DASHDOT:
            return stream << "dash dot";
            break;

        case LINESTYLE_DASHDOTDOT:
            return stream << "dash dot dot";
            break;

        case LINESTYLE_SMALLWAVE:
            return stream << "small wave";
            break;

        case LINESTYLE_WAVE:
            return stream << "wave";
            break;

        case LINESTYLE_DOUBLEWAVE:
            return stream << "double wave";
            break;

        case LINESTYLE_BOLD:
            return stream << "bold";
            break;

        case LINESTYLE_BOLDDOTTED:
            return stream << "bold dotted";
            break;

        case LINESTYLE_BOLDDASH:
            return stream << "bold dash";
            break;

        case LINESTYLE_BOLDLONGDASH:
            return stream << "bold long dash";
            break;

        case LINESTYLE_BOLDDASHDOT:
            return stream << "bold dash dot";
            break;

        case LINESTYLE_BOLDDASHDOTDOT:
            return stream << "bold dash dot dot";
            break;

        case LINESTYLE_BOLDWAVE:
            return stream << "bold wave";
            break;

        default:
            SAL_WARN("vcl.gdi", "FontLineStyle out of bounds");
            assert(false && "FontLineStyle out of bounds");
            break;
    }
}

enum FontStrikeout { STRIKEOUT_NONE, STRIKEOUT_SINGLE, STRIKEOUT_DOUBLE,
                     STRIKEOUT_DONTKNOW, STRIKEOUT_BOLD,
                     STRIKEOUT_SLASH, STRIKEOUT_X,
                     FontStrikeout_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, FontStrikeout const& strikeout)
{
    switch (strikeout)
    {
        case STRIKEOUT_NONE:
            return stream << "none";
            break;

        case STRIKEOUT_SINGLE:
            return stream << "single";
            break;

        case STRIKEOUT_DOUBLE:
            return stream << "double";
            break;

        case STRIKEOUT_DONTKNOW:
            return stream << "unknown";
            break;

        case STRIKEOUT_BOLD:
            return stream << "bold";
            break;

        case STRIKEOUT_SLASH:
            return stream << "slash";
            break;

        case STRIKEOUT_X:
            return stream << "x";
            break;

        default:
            SAL_WARN("vcl.gdi", "FontStrikeout out of bounds");
            assert(false && "FontStrikeout out of bounds");
            break;
    }
}

enum class FontEmphasisMark {
    NONE          = 0x0000, // capitalisation to avoid conflict with X11 macro
    Dot           = 0x0001,
    Circle        = 0x0002,
    Disc          = 0x0003,
    Accent        = 0x0004,
    Style         = 0x000f,
    PosAbove      = 0x1000,
    PosBelow      = 0x2000
};
namespace o3tl
{
    template<> struct typed_flags<FontEmphasisMark> : is_typed_flags<FontEmphasisMark, 0x300f> {};
}

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, FontEmphasisMark const& emphasismark)
{
    switch (emphasismark)
    {
        case FontEmphasisMark::NONE:
            return stream << "none";
            break;

        case FontEmphasisMark::Dot:
            return stream << "Dot";
            break;

        case FontEmphasisMark::Circle:
            return stream << "Circle";
            break;

        case FontEmphasisMark::Disc:
            return stream << "Disc";
            break;

        case FontEmphasisMark::Accent:
            return stream << "Accent";
            break;

        case FontEmphasisMark::Style:
            return stream << "Style";
            break;

        case FontEmphasisMark::PosAbove:
            return stream << "PosAbove";
            break;

        case FontEmphasisMark::PosBelow:
            return stream << "PosBelow";
            break;

        default:
            SAL_WARN("vcl.gdi", "FontEmphasisMark out of bounds");
            assert(false && "FontEmphasisMark out of bounds");
            break;
    }
}

enum FontEmbeddedBitmap { EMBEDDEDBITMAP_DONTKNOW, EMBEDDEDBITMAP_FALSE, EMBEDDEDBITMAP_TRUE };

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, FontEmbeddedBitmap const& embeddedbitmap)
{
    switch (embeddedbitmap)
    {
        case EMBEDDEDBITMAP_DONTKNOW:
            return stream << "unknown";
            break;

        case EMBEDDEDBITMAP_FALSE:
            return stream << "false";
            break;

        case EMBEDDEDBITMAP_TRUE:
            return stream << "true";
            break;
    }
}

enum FontAntiAlias { ANTIALIAS_DONTKNOW, ANTIALIAS_FALSE, ANTIALIAS_TRUE };

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, FontAntiAlias const& antialias)
{
    switch (antialias)
    {
        case ANTIALIAS_DONTKNOW:
            return stream << "unknown";
            break;

        case ANTIALIAS_FALSE:
            return stream << "false";
            break;

        case ANTIALIAS_TRUE:
            return stream << "true";
            break;
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
