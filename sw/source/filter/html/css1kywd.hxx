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

#ifndef _CSS1KYWD_HXX
#define _CSS1KYWD_HXX

#include <sal/types.h>

#ifndef CSS1_CONSTASCII_DECL
#define CSS1_CONSTASCII_DECL( n, s ) n[sizeof(s)]
#endif
#ifndef CSS1_CONSTASCII_DEF
#define CSS1_CONSTASCII_DEF( n, s ) n[sizeof(s)] = s
#endif


extern sal_Char CSS1_CONSTASCII_DECL( sCSS_mimetype, "text/css" );


// ein par allgemeine Strings

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_import, "import" );

// Feature: PrintExt
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_page, "page" );
//sal_Char CSS1_CONSTASCII_DECL( sCSS1_media, "media" );
// /Feature: PrintExt

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_important, "important" );

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_link, "link" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_visited, "visited" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_first_letter, "first-letter" );

// Feature: PrintExt
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_left, "left" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_right, "right" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_first, "first" );
// /Feature: PrintExt

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_url, "url" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_rgb, "rgb" );

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_UNIT_pt, "pt" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_UNIT_mm, "mm" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_UNIT_cm, "cm" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_UNIT_pc, "pc" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_UNIT_inch, "in" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_UNIT_px, "px" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_UNIT_em, "em" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_UNIT_ex, "ex" );


// Strings fuer Font-Properties

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_font_family, "font-family" );

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_serif, "serif" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_sans_serif, "sans-serif" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_cursive, "cursive" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_fantasy, "fantasy" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_monospace, "monospace" );


extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_font_style, "font-style" );

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_normal, "normal" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_italic, "italic" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_oblique, "oblique" );


extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_font_variant, "font-variant" );

//sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_normal, "normal" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_small_caps, "small-caps" );

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_text_transform, "text-transform" );

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_capitalize, "capitalize" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_uppercase, "uppercase" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_lowercase, "lowercase" );

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_font_weight, "font-weight" );

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_extra_light, "extra-light" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_light, "light" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_demi_light, "demi-light" );
//sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_medium, "medium" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_demi_bold, "demi-bold" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_bold, "bold" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_extra_bold, "extra-bold" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_lighter, "lighter" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_bolder, "bolder" );


extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_font_size, "font-size" );

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_xx_small, "xx-small" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_x_small, "x-small" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_small, "small" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_medium, "medium" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_large, "large" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_x_large, "x-large" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_xx_large, "xx-large" );

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_larger, "larger" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_smaller, "smaller" );


extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_font, "font" );




// Strings fuer Farb- und Hintergrund-Properties

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_color, "color" );


extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_background, "background" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_background_color, "background-color" );

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_transparent, "transparent" );

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_repeat, "repeat" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_repeat_x, "repeat-x" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_repeat_y, "repeat-y" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_no_repeat, "no-repeat" );

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_top, "top" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_middle, "middle" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_bottom, "bottom" );

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_scroll, "scroll" );



// Strings fuer Text-Properties

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_letter_spacing, "letter-spacing" );


extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_text_decoration, "text-decoration" );

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_none, "none" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_underline, "underline" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_overline, "overline" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_line_through, "line-through" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_blink, "blink" );


extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_text_align, "text-align" );

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_left, "left" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_center, "center" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_right, "right" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_justify, "justify" );


extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_text_indent, "text-indent" );


extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_line_height, "line-height" );



// Strings fuer Box-Properties

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_margin_left, "margin-left" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_margin_right, "margin-right" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_margin_top, "margin-top" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_margin_bottom, "margin-bottom" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_margin, "margin" );

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_padding_top, "padding-top" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_padding_bottom, "padding-bottom" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_padding_left, "padding-left" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_padding_right, "padding-right" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_padding, "padding" );

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_auto, "auto" );

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_border_left_width, "border-left-width" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_border_right_width, "border-right-width" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_border_top_width, "border-top-width" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_border_bottom_width, "border-bottom-width" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_border_width, "border-width" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_border_color, "border-color" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_border_style, "border-style" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_border_left, "border-left" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_border_right, "border-right" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_border_top, "border-top" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_border_bottom, "border-bottom" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_border, "border" );

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_thin, "thin" );
//sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_medium, "medium" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_thick, "thick" );

//sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_none, "none" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_dotted, "dotted" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_dashed, "dashed" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_solid, "solid" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_double, "double" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_groove, "groove" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_ridge, "ridge" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_inset, "inset" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_outset, "outset" );

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_width, "width" );

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_height, "height" );

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_float, "float" );

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_column_count, "column-count" );

// Strings fuer Positioning

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_position, "position" );

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_absolute, "absolute" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_relative, "relative" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_static, "static" );

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_left, "left" );

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_top, "top" );


// Feature: PrintExt

// Strings fuer Printing Extensions

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_page_break_before, "page-break-before" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_page_break_after, "page-break-after" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_page_break_inside, "page-break-inside" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_size, "size" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_widows, "widows" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_orphans, "orphans" );
//sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_marks, "marks" );

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_always, "always" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_avoid, "avoid" );

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_portrait, "portrait" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_landscape, "landscape" );

//sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_crop, "crop" );
//sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_cross, "cross" );

// /Feature: PrintExt

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_class_abs_pos, "sd-abs-pos" );


extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_so_language, "so-language" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_direction, "direction" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_ltr, "ltr" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_rtl, "rtl" );
extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_PV_inherit, "inherit" );

extern sal_Char CSS1_CONSTASCII_DECL( sCSS1_P_display, "display" );

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
