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

#include "css1kywd.hxx"

sal_Char CSS1_CONSTASCII_DEF( sCSS_mimetype, "text/css" );

// ein par allgemeine Strings

sal_Char CSS1_CONSTASCII_DEF( sCSS1_import, "import" );

// Feature: PrintExt
sal_Char CSS1_CONSTASCII_DEF( sCSS1_page, "page" );
//sal_Char CSS1_CONSTASCII_DEF( sCSS1_media, "media" );
// /Feature: PrintExt

sal_Char CSS1_CONSTASCII_DEF( sCSS1_important, "important" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_link, "link" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_visited, "visited" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_first_letter, "first-letter" );

// Feature: PrintExt
sal_Char CSS1_CONSTASCII_DEF( sCSS1_left, "left" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_right, "right" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_first, "first" );
// /Feature: PrintExt

sal_Char CSS1_CONSTASCII_DEF( sCSS1_url, "url" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_rgb, "rgb" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_UNIT_pt, "pt" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_UNIT_mm, "mm" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_UNIT_cm, "cm" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_UNIT_pc, "pc" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_UNIT_inch, "in" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_UNIT_px, "px" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_UNIT_em, "em" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_UNIT_ex, "ex" );

// Strings fuer Font-Properties

sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_font_family, "font-family" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_serif, "serif" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_sans_serif, "sans-serif" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_cursive, "cursive" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_fantasy, "fantasy" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_monospace, "monospace" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_font_style, "font-style" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_normal, "normal" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_italic, "italic" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_oblique, "oblique" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_font_variant, "font-variant" );

//sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_normal, "normal" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_small_caps, "small-caps" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_font_weight, "font-weight" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_extra_light, "extra-light" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_light, "light" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_demi_light, "demi-light" );
//sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_medium, "medium" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_demi_bold, "demi-bold" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_bold, "bold" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_extra_bold, "extra-bold" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_lighter, "lighter" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_bolder, "bolder" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_text_transform, "text-transform" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_capitalize, "capitalize" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_uppercase, "uppercase" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_lowercase, "lowercase" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_font_size, "font-size" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_xx_small, "xx-small" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_x_small, "x-small" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_small, "small" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_medium, "medium" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_large, "large" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_x_large, "x-large" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_xx_large, "xx-large" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_larger, "larger" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_smaller, "smaller" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_font, "font" );

// Strings fuer Farb- und Hintergrund-Properties

sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_color, "color" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_background, "background" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_background_color, "background-color" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_transparent, "transparent" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_repeat, "repeat" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_repeat_x, "repeat-x" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_repeat_y, "repeat-y" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_no_repeat, "no-repeat" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_top, "top" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_middle, "middle" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_bottom, "bottom" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_scroll, "scroll" );

// Strings fuer Text-Properties

sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_letter_spacing, "letter-spacing" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_text_decoration, "text-decoration" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_none, "none" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_underline, "underline" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_overline, "overline" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_line_through, "line-through" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_blink, "blink" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_text_align, "text-align" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_left, "left" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_center, "center" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_right, "right" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_justify, "justify" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_text_indent, "text-indent" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_line_height, "line-height" );

// Strings fuer Box-Properties

sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_margin_left, "margin-left" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_margin_right, "margin-right" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_margin_top, "margin-top" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_margin_bottom, "margin-bottom" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_margin, "margin" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_padding_top, "padding-top" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_padding_bottom, "padding-bottom" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_padding_left, "padding-left" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_padding_right, "padding-right" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_padding, "padding" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_auto, "auto" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_border_left_width, "border-left-width" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_border_right_width, "border-right-width" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_border_top_width, "border-top-width" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_border_bottom_width, "border-bottom-width" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_border_width, "border-width" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_border_color, "border-color" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_border_style, "border-style" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_border_left, "border-left" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_border_right, "border-right" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_border_top, "border-top" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_border_bottom, "border-bottom" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_border, "border" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_thin, "thin" );
//sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_medium, "medium" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_thick, "thick" );

//sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_none, "none" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_dotted, "dotted" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_dashed, "dashed" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_solid, "solid" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_double, "double" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_groove, "groove" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_ridge, "ridge" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_inset, "inset" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_outset, "outset" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_width, "width" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_height, "height" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_float, "float" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_column_count, "column-count" );

// Strings fuer Positioning

sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_position, "position" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_absolute, "absolute" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_relative, "relative" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_static, "static" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_left, "left" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_top, "top" );

// Feature: PrintExt

// Strings fuer Printing Extensions

sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_page_break_before, "page-break-before" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_page_break_after, "page-break-after" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_page_break_inside, "page-break-inside" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_size, "size" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_widows, "widows" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_orphans, "orphans" );
//sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_marks, "marks" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_always, "always" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_avoid, "avoid" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_portrait, "portrait" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_landscape, "landscape" );

//sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_crop, "crop" );
//sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_cross, "cross" );

// /Feature: PrintExt

sal_Char CSS1_CONSTASCII_DEF( sCSS1_class_abs_pos, "sd-abs-pos" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_so_language, "so-language" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_direction, "direction" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_ltr, "ltr" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_rtl, "rtl" );
sal_Char CSS1_CONSTASCII_DEF( sCSS1_PV_inherit, "inherit" );

sal_Char CSS1_CONSTASCII_DEF( sCSS1_P_display, "display" );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
