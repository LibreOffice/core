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
#ifndef _SVX_SVXENUM_HXX
#define _SVX_SVXENUM_HXX

// -----------------------------------------------------------------------

enum SvxCaseMap
{
    SVX_CASEMAP_NOT_MAPPED,
    SVX_CASEMAP_VERSALIEN,
    SVX_CASEMAP_GEMEINE,
    SVX_CASEMAP_TITEL,
    SVX_CASEMAP_KAPITAELCHEN,
    SVX_CASEMAP_END
};

enum SvxEscapement
{
    SVX_ESCAPEMENT_OFF,
    SVX_ESCAPEMENT_SUPERSCRIPT,
    SVX_ESCAPEMENT_SUBSCRIPT,
    SVX_ESCAPEMENT_END
};

enum SvxShadowLocation
{
    SVX_SHADOW_NONE,
    SVX_SHADOW_TOPLEFT,
    SVX_SHADOW_TOPRIGHT,
    SVX_SHADOW_BOTTOMLEFT,
    SVX_SHADOW_BOTTOMRIGHT,
    SVX_SHADOW_END
};

enum SvxTabAdjust
{
    SVX_TAB_ADJUST_LEFT = 0,
    SVX_TAB_ADJUST_RIGHT,
    SVX_TAB_ADJUST_DECIMAL,
    SVX_TAB_ADJUST_CENTER,
    SVX_TAB_ADJUST_DEFAULT,
    SVX_TAB_ADJUST_END
};

enum SvxLineSpace
{
    SVX_LINE_SPACE_AUTO,
    SVX_LINE_SPACE_FIX,
    SVX_LINE_SPACE_MIN,
    SVX_LINE_SPACE_END
};

enum SvxInterLineSpace
{
    SVX_INTER_LINE_SPACE_OFF,
    SVX_INTER_LINE_SPACE_PROP,
    SVX_INTER_LINE_SPACE_FIX,
    SVX_INTER_LINE_SPACE_END
};

enum SvxAdjust
{
    SVX_ADJUST_LEFT,
    SVX_ADJUST_RIGHT,
    SVX_ADJUST_BLOCK,
    SVX_ADJUST_CENTER,
    SVX_ADJUST_BLOCKLINE,
    SVX_ADJUST_END
};

enum SvxSpecialLineSpace
{
    SVX_LINESPACE_USER,
    SVX_LINESPACE_ONE_LINE,
    SVX_LINESPACE_ONE_POINT_FIVE_LINES,
    SVX_LINESPACE_TWO_LINES,
    SVX_LINESPACE_END
};

enum SvxBreak
{
    SVX_BREAK_NONE,
    SVX_BREAK_COLUMN_BEFORE,
    SVX_BREAK_COLUMN_AFTER,
    SVX_BREAK_COLUMN_BOTH,
    SVX_BREAK_PAGE_BEFORE,
    SVX_BREAK_PAGE_AFTER,
    SVX_BREAK_PAGE_BOTH,
    SVX_BREAK_END
};

enum SvxCellHorJustify
{
    SVX_HOR_JUSTIFY_STANDARD,
    SVX_HOR_JUSTIFY_LEFT,
    SVX_HOR_JUSTIFY_CENTER,
    SVX_HOR_JUSTIFY_RIGHT,
    SVX_HOR_JUSTIFY_BLOCK,
    SVX_HOR_JUSTIFY_REPEAT
};

enum SvxCellJustifyMethod
{
    SVX_JUSTIFY_METHOD_AUTO,
    SVX_JUSTIFY_METHOD_DISTRIBUTE
};

enum SvxCellVerJustify
{
    SVX_VER_JUSTIFY_STANDARD,
    SVX_VER_JUSTIFY_TOP,
    SVX_VER_JUSTIFY_CENTER,
    SVX_VER_JUSTIFY_BOTTOM,
    SVX_VER_JUSTIFY_BLOCK
};

enum SvxCellOrientation
{
    SVX_ORIENTATION_STANDARD,
    SVX_ORIENTATION_TOPBOTTOM,
    SVX_ORIENTATION_BOTTOMTOP,
    SVX_ORIENTATION_STACKED
};

enum SvxDrawToolEnum
{
    SVX_SNAP_DRAW_SELECT,
    //
    SVX_SNAP_DRAW_LINE,
    SVX_SNAP_DRAW_RECT,
    SVX_SNAP_DRAW_ELLIPSE,
    SVX_SNAP_DRAW_POLYGON_NOFILL,
    SVX_SNAP_DRAW_BEZIER_NOFILL,
    SVX_SNAP_DRAW_FREELINE_NOFILL,
    SVX_SNAP_DRAW_ARC,
    SVX_SNAP_DRAW_PIE,
    SVX_SNAP_DRAW_CIRCLECUT,
    SVX_SNAP_DRAW_TEXT,
    SVX_SNAP_DRAW_TEXT_VERTICAL,
    SVX_SNAP_DRAW_TEXT_MARQUEE,
    SVX_SNAP_DRAW_CAPTION,
    SVX_SNAP_DRAW_CAPTION_VERTICAL
};

enum SvxDrawAlignEnum
{
    SVX_OBJECT_ALIGN_LEFT,
    SVX_OBJECT_ALIGN_CENTER,
    SVX_OBJECT_ALIGN_RIGHT,
    SVX_OBJECT_ALIGN_UP,
    SVX_OBJECT_ALIGN_MIDDLE,
    SVX_OBJECT_ALIGN_DOWN
};

enum SvxDrawBezierEnum
{
    SVX_BEZIER_MOVE,
    SVX_BEZIER_INSERT,
    SVX_BEZIER_DELETE,
    SVX_BEZIER_CUTLINE,
    SVX_BEZIER_CONVERT,
    SVX_BEZIER_EDGE,
    SVX_BEZIER_SMOOTH,
    SVX_BEZIER_SYMMTR,
    SVX_BEZIER_CLOSE,
    SVX_BEZIER_ELIMINATE_POINTS
};

enum SvxSpellArea
{
    SVX_SPELL_BODY = 0,
    SVX_SPELL_BODY_END,
    SVX_SPELL_BODY_START,
    SVX_SPELL_OTHER
};

enum SvxFrameAnchor
{
    SVX_FLY_AT_CNTNT,       // Paragraph-based frame
    SVX_FLY_IN_CNTNT,       // Character-based frame
    SVX_FLY_PAGE            // Page-based frame
};

enum SvxExtNumType
{
    SVX_NUM_CHARS_UPPER_LETTER, // Counts from a-z, aa - az, ba - bz, ...
    SVX_NUM_CHARS_LOWER_LETTER,
    SVX_NUM_ROMAN_UPPER,
    SVX_NUM_ROMAN_LOWER,
    SVX_NUM_ARABIC,
    SVX_NUM_NUMBER_NONE,
    SVX_NUM_CHAR_SPECIAL,   // Bullet
    SVX_NUM_PAGEDESC,       // Numbering from the page template
    SVX_NUM_BITMAP,
    SVX_NUM_CHARS_UPPER_LETTER_N, // Counts from  a-z, aa-zz, aaa-zzz
    SVX_NUM_CHARS_LOWER_LETTER_N
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
