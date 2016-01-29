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

enum FontFamily { FAMILY_DONTKNOW, FAMILY_DECORATIVE, FAMILY_MODERN,
                  FAMILY_ROMAN, FAMILY_SCRIPT, FAMILY_SWISS, FAMILY_SYSTEM, FontFamily_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

enum FontPitch { PITCH_DONTKNOW, PITCH_FIXED, PITCH_VARIABLE, FontPitch_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

enum TextAlign { ALIGN_TOP, ALIGN_BASELINE, ALIGN_BOTTOM, TextAlign_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

enum FontWeight { WEIGHT_DONTKNOW, WEIGHT_THIN, WEIGHT_ULTRALIGHT,
                  WEIGHT_LIGHT, WEIGHT_SEMILIGHT, WEIGHT_NORMAL,
                  WEIGHT_MEDIUM, WEIGHT_SEMIBOLD, WEIGHT_BOLD,
                  WEIGHT_ULTRABOLD, WEIGHT_BLACK, FontWeight_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

enum FontWidth { WIDTH_DONTKNOW, WIDTH_ULTRA_CONDENSED, WIDTH_EXTRA_CONDENSED,
                 WIDTH_CONDENSED, WIDTH_SEMI_CONDENSED, WIDTH_NORMAL,
                 WIDTH_SEMI_EXPANDED, WIDTH_EXPANDED, WIDTH_EXTRA_EXPANDED,
                 WIDTH_ULTRA_EXPANDED,
                 FontWidth_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

enum FontItalic { ITALIC_NONE, ITALIC_OBLIQUE, ITALIC_NORMAL, ITALIC_DONTKNOW, FontItalic_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

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

enum FontStrikeout { STRIKEOUT_NONE, STRIKEOUT_SINGLE, STRIKEOUT_DOUBLE,
                     STRIKEOUT_DONTKNOW, STRIKEOUT_BOLD,
                     STRIKEOUT_SLASH, STRIKEOUT_X,
                     FontStrikeout_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

typedef sal_uInt16 FontEmphasisMark;
#define EMPHASISMARK_NONE           ((FontEmphasisMark)0x0000)
#define EMPHASISMARK_DOT            ((FontEmphasisMark)0x0001)
#define EMPHASISMARK_CIRCLE         ((FontEmphasisMark)0x0002)
#define EMPHASISMARK_DISC           ((FontEmphasisMark)0x0003)
#define EMPHASISMARK_ACCENT         ((FontEmphasisMark)0x0004)
#define EMPHASISMARK_STYLE          ((FontEmphasisMark)0x00FF)
#define EMPHASISMARK_POS_ABOVE      ((FontEmphasisMark)0x1000)
#define EMPHASISMARK_POS_BELOW      ((FontEmphasisMark)0x2000)

// Only for compatibility
#define EMPHASISMARK_DOTS_ABOVE     (EMPHASISMARK_DOT | EMPHASISMARK_POS_ABOVE)
#define EMPHASISMARK_DOTS_BELOW     (EMPHASISMARK_DOT | EMPHASISMARK_POS_BELOW)
#define EMPHASISMARK_SIDE_DOTS      (EMPHASISMARK_ACCENT | EMPHASISMARK_POS_ABOVE)
#define EMPHASISMARK_CIRCLE_ABOVE   (EMPHASISMARK_CIRCLE | EMPHASISMARK_POS_ABOVE)

enum FontType { TYPE_DONTKNOW, TYPE_RASTER, TYPE_VECTOR, TYPE_SCALABLE,
                FontType_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

enum FontEmbeddedBitmap { EMBEDDEDBITMAP_DONTKNOW, EMBEDDEDBITMAP_FALSE, EMBEDDEDBITMAP_TRUE };

enum FontAntiAlias { ANTIALIAS_DONTKNOW, ANTIALIAS_FALSE, ANTIALIAS_TRUE };

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
