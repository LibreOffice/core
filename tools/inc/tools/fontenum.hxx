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

#ifndef _TOOLS_FONTENUM_HXX
#define _TOOLS_FONTENUM_HXX

#include <sal/types.h>
#include <tools/solar.h>

#ifndef ENUM_FONTFAMILY_DECLARED
#define ENUM_FONTFAMILY_DECLARED

enum FontFamily { FAMILY_DONTKNOW, FAMILY_DECORATIVE, FAMILY_MODERN,
                  FAMILY_ROMAN, FAMILY_SCRIPT, FAMILY_SWISS, FAMILY_SYSTEM, FontFamily_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

#endif

// ------------------------------------------------------------

#ifndef ENUM_FONTPITCH_DECLARED
#define ENUM_FONTPITCH_DECLARED

enum FontPitch { PITCH_DONTKNOW, PITCH_FIXED, PITCH_VARIABLE, FontPitch_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

#endif

// ------------------------------------------------------------

#ifndef ENUM_TEXTALIGN_DECLARED
#define ENUM_TEXTALIGN_DECLARED

enum TextAlign { ALIGN_TOP, ALIGN_BASELINE, ALIGN_BOTTOM, TextAlign_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

#endif

// ------------------------------------------------------------

#ifndef ENUM_FONTWEIGHT_DECLARED
#define ENUM_FONTWEIGHT_DECLARED

enum FontWeight { WEIGHT_DONTKNOW, WEIGHT_THIN, WEIGHT_ULTRALIGHT,
                  WEIGHT_LIGHT, WEIGHT_SEMILIGHT, WEIGHT_NORMAL,
                  WEIGHT_MEDIUM, WEIGHT_SEMIBOLD, WEIGHT_BOLD,
                  WEIGHT_ULTRABOLD, WEIGHT_BLACK, FontWeight_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

#endif

// ------------------------------------------------------------

#ifndef ENUM_FONTWIDTH_DECLARED
#define ENUM_FONTWIDTH_DECLARED

enum FontWidth { WIDTH_DONTKNOW, WIDTH_ULTRA_CONDENSED, WIDTH_EXTRA_CONDENSED,
                 WIDTH_CONDENSED, WIDTH_SEMI_CONDENSED, WIDTH_NORMAL,
                 WIDTH_SEMI_EXPANDED, WIDTH_EXPANDED, WIDTH_EXTRA_EXPANDED,
                 WIDTH_ULTRA_EXPANDED,
                 FontWidth_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

#endif

// ------------------------------------------------------------

#ifndef ENUM_FONTITALIC_DECLARED
#define ENUM_FONTITALIC_DECLARED

enum FontItalic { ITALIC_NONE, ITALIC_OBLIQUE, ITALIC_NORMAL, ITALIC_DONTKNOW, FontItalic_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

#endif

// ------------------------------------------------------------

#ifndef ENUM_FONTUNDERLINE_DECLARED
#define ENUM_FONTUNDERLINE_DECLARED

enum FontUnderline { UNDERLINE_NONE, UNDERLINE_SINGLE, UNDERLINE_DOUBLE,
                     UNDERLINE_DOTTED, UNDERLINE_DONTKNOW,
                     UNDERLINE_DASH, UNDERLINE_LONGDASH,
                     UNDERLINE_DASHDOT, UNDERLINE_DASHDOTDOT,
                     UNDERLINE_SMALLWAVE,
                     UNDERLINE_WAVE, UNDERLINE_DOUBLEWAVE,
                     UNDERLINE_BOLD, UNDERLINE_BOLDDOTTED,
                     UNDERLINE_BOLDDASH, UNDERLINE_BOLDLONGDASH,
                     UNDERLINE_BOLDDASHDOT, UNDERLINE_BOLDDASHDOTDOT,
                     UNDERLINE_BOLDWAVE,
                     FontUnderline_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

#endif

// ------------------------------------------------------------

#ifndef ENUM_FONTSTRIKEOUT_DECLARED
#define ENUM_FONTSTRIKEOUT_DECLARED

enum FontStrikeout { STRIKEOUT_NONE, STRIKEOUT_SINGLE, STRIKEOUT_DOUBLE,
                     STRIKEOUT_DONTKNOW, STRIKEOUT_BOLD,
                     STRIKEOUT_SLASH, STRIKEOUT_X,
                     FontStrikeout_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

#endif

// ------------------------------------------------------------

#ifndef ENUM_FONTEMPHASISMARK_DECLARED
#define ENUM_FONTEMPHASISMARK_DECLARED

typedef sal_uInt16 FontEmphasisMark;
#define EMPHASISMARK_NONE           ((FontEmphasisMark)0x0000)
#define EMPHASISMARK_DOT            ((FontEmphasisMark)0x0001)
#define EMPHASISMARK_CIRCLE         ((FontEmphasisMark)0x0002)
#define EMPHASISMARK_DISC           ((FontEmphasisMark)0x0003)
#define EMPHASISMARK_ACCENT         ((FontEmphasisMark)0x0004)
#define EMPHASISMARK_STYLE          ((FontEmphasisMark)0x00FF)
#define EMPHASISMARK_POS_ABOVE      ((FontEmphasisMark)0x1000)
#define EMPHASISMARK_POS_BELOW      ((FontEmphasisMark)0x2000)

// Only for kompability
#define EMPHASISMARK_DOTS_ABOVE     (EMPHASISMARK_DOT | EMPHASISMARK_POS_ABOVE)
#define EMPHASISMARK_DOTS_BELOW     (EMPHASISMARK_DOT | EMPHASISMARK_POS_BELOW)
#define EMPHASISMARK_SIDE_DOTS      (EMPHASISMARK_ACCENT | EMPHASISMARK_POS_ABOVE)
#define EMPHASISMARK_CIRCLE_ABOVE   (EMPHASISMARK_CIRCLE | EMPHASISMARK_POS_ABOVE)

#endif

// ------------------------------------------------------------

#ifndef ENUM_FONTTYPE_DECLARED
#define ENUM_FONTTYPE_DECLARED

enum FontType { TYPE_DONTKNOW, TYPE_RASTER, TYPE_VECTOR, TYPE_SCALABLE,
                FontType_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

#endif

#ifndef ENUM_FONTEMBEDDEDBITMAP_DECLARED
#define ENUM_FONTEMBEDDEDBITMAP_DECLARED

enum FontEmbeddedBitmap { EMBEDDEDBITMAP_DONTKNOW, EMBEDDEDBITMAP_FALSE, EMBEDDEDBITMAP_TRUE };

#endif

#ifndef ENUM_FONTANTIALIAS_DECLARED
#define ENUM_FONTANTIALIAS_DECLARED

enum FontAntiAlias { ANTIALIAS_DONTKNOW, ANTIALIAS_FALSE, ANTIALIAS_TRUE };

#endif

#endif  // _TOOLS_FONTENUM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
