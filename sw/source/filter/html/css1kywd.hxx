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

#ifndef INCLUDED_SW_SOURCE_FILTER_HTML_CSS1KYWD_HXX
#define INCLUDED_SW_SOURCE_FILTER_HTML_CSS1KYWD_HXX

#include <sal/types.h>

extern const char* const sCSS_mimetype;

extern const char* const sCSS1_page;

extern const char* const sCSS1_link;
extern const char* const sCSS1_visited;
extern const char* const sCSS1_first_letter;

extern const char* const sCSS1_left;
extern const char* const sCSS1_right;
extern const char* const sCSS1_first;

extern const char* const sCSS1_url;

extern const char* const sCSS1_UNIT_pt;
extern const char* const sCSS1_UNIT_mm;
extern const char* const sCSS1_UNIT_cm;
extern const char* const sCSS1_UNIT_pc;
extern const char* const sCSS1_UNIT_inch;
extern const char* const sCSS1_UNIT_px;

// Strings for font properties

extern const char* const sCSS1_P_font_family;

extern const char* const sCSS1_PV_serif;
extern const char* const sCSS1_PV_sans_serif;
extern const char* const sCSS1_PV_cursive;
extern const char* const sCSS1_PV_fantasy;
extern const char* const sCSS1_PV_monospace;

extern const char* const sCSS1_P_font_style;

extern const char* const sCSS1_PV_normal;
extern const char* const sCSS1_PV_italic;
extern const char* const sCSS1_PV_oblique;

extern const char* const sCSS1_P_font_variant;

//extern const char* const sCSS1_PV_normal;
extern const char* const sCSS1_PV_small_caps;

extern const char* const sCSS1_P_text_transform;

extern const char* const sCSS1_PV_capitalize;
extern const char* const sCSS1_PV_uppercase;
extern const char* const sCSS1_PV_lowercase;

extern const char* const sCSS1_P_font_weight;

extern const char* const sCSS1_PV_extra_light;
extern const char* const sCSS1_PV_light;
extern const char* const sCSS1_PV_demi_light;
//extern const char* const sCSS1_PV_medium;
extern const char* const sCSS1_PV_demi_bold;
extern const char* const sCSS1_PV_bold;
extern const char* const sCSS1_PV_extra_bold;

extern const char* const sCSS1_P_font_size;

extern const char* const sCSS1_P_font;

// Strings for color and background properties

extern const char* const sCSS1_P_color;

extern const char* const sCSS1_P_background;
extern const char* const sCSS1_P_background_color;

extern const char* const sCSS1_PV_transparent;

extern const char* const sCSS1_PV_repeat;
extern const char* const sCSS1_PV_no_repeat;

extern const char* const sCSS1_PV_top;
extern const char* const sCSS1_PV_middle;
extern const char* const sCSS1_PV_bottom;

extern const char* const sCSS1_PV_scroll;

// Strings for text properties

extern const char* const sCSS1_P_letter_spacing;

extern const char* const sCSS1_P_text_decoration;

extern const char* const sCSS1_PV_none;
extern const char* const sCSS1_PV_underline;
extern const char* const sCSS1_PV_overline;
extern const char* const sCSS1_PV_line_through;
extern const char* const sCSS1_PV_blink;

extern const char* const sCSS1_P_text_align;

extern const char* const sCSS1_PV_left;
extern const char* const sCSS1_PV_center;
extern const char* const sCSS1_PV_right;
extern const char* const sCSS1_PV_justify;

extern const char* const sCSS1_P_text_indent;

extern const char* const sCSS1_P_line_height;

extern const char* const sCSS1_P_list_style_type;

// Strings for box properties

extern const char* const sCSS1_P_margin_left;
extern const char* const sCSS1_P_margin_right;
extern const char* const sCSS1_P_margin_top;
extern const char* const sCSS1_P_margin_bottom;
extern const char* const sCSS1_P_margin;

extern const char* const sCSS1_P_padding_top;
extern const char* const sCSS1_P_padding_bottom;
extern const char* const sCSS1_P_padding_left;
extern const char* const sCSS1_P_padding_right;
extern const char* const sCSS1_P_padding;

extern const char* const sCSS1_PV_auto;

extern const char* const sCSS1_P_border_left_width;
extern const char* const sCSS1_P_border_right_width;
extern const char* const sCSS1_P_border_top_width;
extern const char* const sCSS1_P_border_bottom_width;
extern const char* const sCSS1_P_border_width;
extern const char* const sCSS1_P_border_color;
extern const char* const sCSS1_P_border_style;
extern const char* const sCSS1_P_border_left;
extern const char* const sCSS1_P_border_right;
extern const char* const sCSS1_P_border_top;
extern const char* const sCSS1_P_border_bottom;
extern const char* const sCSS1_P_border;

//extern const char* const sCSS1_PV_none;
extern const char* const sCSS1_PV_dotted;
extern const char* const sCSS1_PV_dashed;
extern const char* const sCSS1_PV_solid;
extern const char* const sCSS1_PV_double;
extern const char* const sCSS1_PV_groove;
extern const char* const sCSS1_PV_ridge;
extern const char* const sCSS1_PV_inset;
extern const char* const sCSS1_PV_outset;

extern const char* const sCSS1_P_width;
extern const char* const sCSS1_P_max_width;

extern const char* const sCSS1_P_height;

extern const char* const sCSS1_P_float;

extern const char* const sCSS1_P_column_count;

// Strings for positioning

extern const char* const sCSS1_P_position;

extern const char* const sCSS1_PV_absolute;

extern const char* const sCSS1_P_left;

extern const char* const sCSS1_P_top;

// Strings for printing extensions

extern const char* const sCSS1_P_page_break_before;
extern const char* const sCSS1_P_page_break_after;
extern const char* const sCSS1_P_page_break_inside;
extern const char* const sCSS1_P_size;
extern const char* const sCSS1_P_widows;
extern const char* const sCSS1_P_visibility;
extern const char* const sCSS1_P_orphans;
//extern const char* const sCSS1_P_marks;

extern const char* const sCSS1_PV_always;
extern const char* const sCSS1_PV_avoid;

extern const char* const sCSS1_PV_portrait;
extern const char* const sCSS1_PV_landscape;

//extern const char* const sCSS1_PV_crop;
//extern const char* const sCSS1_PV_cross;


extern const char* const sCSS1_P_so_language;
extern const char* const sCSS1_P_direction;
extern const char* const sCSS1_PV_ltr;
extern const char* const sCSS1_PV_rtl;
extern const char* const sCSS1_PV_inherit;

extern const char* const sCSS1_P_display;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
