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

const sal_Char* sCSS_mimetype = "text/css";

const sal_Char* sCSS1_import = "import";

const sal_Char* sCSS1_page = "page";
//const sal_Char* sCSS1_media = "media";

const sal_Char* sCSS1_important = "important";

const sal_Char* sCSS1_link = "link";
const sal_Char* sCSS1_visited = "visited";
const sal_Char* sCSS1_first_letter = "first-letter";

const sal_Char* sCSS1_left = "left";
const sal_Char* sCSS1_right = "right";
const sal_Char* sCSS1_first = "first";

const sal_Char* sCSS1_url = "url";
const sal_Char* sCSS1_rgb = "rgb";

const sal_Char* sCSS1_UNIT_pt = "pt";
const sal_Char* sCSS1_UNIT_mm = "mm";
const sal_Char* sCSS1_UNIT_cm = "cm";
const sal_Char* sCSS1_UNIT_pc = "pc";
const sal_Char* sCSS1_UNIT_inch = "in";
const sal_Char* sCSS1_UNIT_px = "px";
const sal_Char* sCSS1_UNIT_em = "em";
const sal_Char* sCSS1_UNIT_ex = "ex";

// Strings for font properties

const sal_Char* sCSS1_P_font_family = "font-family";

const sal_Char* sCSS1_PV_serif = "serif";
const sal_Char* sCSS1_PV_sans_serif = "sans-serif";
const sal_Char* sCSS1_PV_cursive = "cursive";
const sal_Char* sCSS1_PV_fantasy = "fantasy";
const sal_Char* sCSS1_PV_monospace = "monospace";

const sal_Char* sCSS1_P_font_style = "font-style";

const sal_Char* sCSS1_PV_normal = "normal";
const sal_Char* sCSS1_PV_italic = "italic";
const sal_Char* sCSS1_PV_oblique = "oblique";

const sal_Char* sCSS1_P_font_variant = "font-variant";

//const sal_Char* sCSS1_PV_normal = "normal";
const sal_Char* sCSS1_PV_small_caps = "small-caps";

const sal_Char* sCSS1_P_font_weight = "font-weight";

const sal_Char* sCSS1_PV_extra_light = "extra-light";
const sal_Char* sCSS1_PV_light = "light";
const sal_Char* sCSS1_PV_demi_light = "demi-light";
//const sal_Char* sCSS1_PV_medium = "medium";
const sal_Char* sCSS1_PV_demi_bold = "demi-bold";
const sal_Char* sCSS1_PV_bold = "bold";
const sal_Char* sCSS1_PV_extra_bold = "extra-bold";
const sal_Char* sCSS1_PV_lighter = "lighter";
const sal_Char* sCSS1_PV_bolder = "bolder";

const sal_Char* sCSS1_P_text_transform = "text-transform";

const sal_Char* sCSS1_PV_capitalize = "capitalize";
const sal_Char* sCSS1_PV_uppercase = "uppercase";
const sal_Char* sCSS1_PV_lowercase = "lowercase";

const sal_Char* sCSS1_P_font_size = "font-size";

const sal_Char* sCSS1_PV_xx_small = "xx-small";
const sal_Char* sCSS1_PV_x_small = "x-small";
const sal_Char* sCSS1_PV_small = "small";
const sal_Char* sCSS1_PV_medium = "medium";
const sal_Char* sCSS1_PV_large = "large";
const sal_Char* sCSS1_PV_x_large = "x-large";
const sal_Char* sCSS1_PV_xx_large = "xx-large";

const sal_Char* sCSS1_PV_larger = "larger";
const sal_Char* sCSS1_PV_smaller = "smaller";

const sal_Char* sCSS1_P_font = "font";

// Strings for color and background properties

const sal_Char* sCSS1_P_color = "color";

const sal_Char* sCSS1_P_background = "background";
const sal_Char* sCSS1_P_background_color = "background-color";

const sal_Char* sCSS1_PV_transparent = "transparent";

const sal_Char* sCSS1_PV_repeat = "repeat";
const sal_Char* sCSS1_PV_repeat_x = "repeat-x";
const sal_Char* sCSS1_PV_repeat_y = "repeat-y";
const sal_Char* sCSS1_PV_no_repeat = "no-repeat";

const sal_Char* sCSS1_PV_top = "top";
const sal_Char* sCSS1_PV_middle = "middle";
const sal_Char* sCSS1_PV_bottom = "bottom";

const sal_Char* sCSS1_PV_scroll = "scroll";

// Strings for text properties

const sal_Char* sCSS1_P_letter_spacing = "letter-spacing";

const sal_Char* sCSS1_P_text_decoration = "text-decoration";

const sal_Char* sCSS1_PV_none = "none";
const sal_Char* sCSS1_PV_underline = "underline";
const sal_Char* sCSS1_PV_overline = "overline";
const sal_Char* sCSS1_PV_line_through = "line-through";
const sal_Char* sCSS1_PV_blink = "blink";

const sal_Char* sCSS1_P_text_align = "text-align";

const sal_Char* sCSS1_PV_left = "left";
const sal_Char* sCSS1_PV_center = "center";
const sal_Char* sCSS1_PV_right = "right";
const sal_Char* sCSS1_PV_justify = "justify";

const sal_Char* sCSS1_P_text_indent = "text-indent";

const sal_Char* sCSS1_P_line_height = "line-height";
const sal_Char* sCSS1_P_list_style_type = "list-style-type";

// Strings for box properties

const sal_Char* sCSS1_P_margin_left = "margin-left";
const sal_Char* sCSS1_P_margin_right = "margin-right";
const sal_Char* sCSS1_P_margin_top = "margin-top";
const sal_Char* sCSS1_P_margin_bottom = "margin-bottom";
const sal_Char* sCSS1_P_margin = "margin";

const sal_Char* sCSS1_P_padding_top = "padding-top";
const sal_Char* sCSS1_P_padding_bottom = "padding-bottom";
const sal_Char* sCSS1_P_padding_left = "padding-left";
const sal_Char* sCSS1_P_padding_right = "padding-right";
const sal_Char* sCSS1_P_padding = "padding";

const sal_Char* sCSS1_PV_auto = "auto";

const sal_Char* sCSS1_P_border_left_width = "border-left-width";
const sal_Char* sCSS1_P_border_right_width = "border-right-width";
const sal_Char* sCSS1_P_border_top_width = "border-top-width";
const sal_Char* sCSS1_P_border_bottom_width = "border-bottom-width";
const sal_Char* sCSS1_P_border_width = "border-width";
const sal_Char* sCSS1_P_border_color = "border-color";
const sal_Char* sCSS1_P_border_style = "border-style";
const sal_Char* sCSS1_P_border_left = "border-left";
const sal_Char* sCSS1_P_border_right = "border-right";
const sal_Char* sCSS1_P_border_top = "border-top";
const sal_Char* sCSS1_P_border_bottom = "border-bottom";
const sal_Char* sCSS1_P_border = "border";

const sal_Char* sCSS1_PV_thin = "thin";
//const sal_Char* sCSS1_PV_medium = "medium";
const sal_Char* sCSS1_PV_thick = "thick";

//const sal_Char* sCSS1_PV_none = "none";
const sal_Char* sCSS1_PV_dotted = "dotted";
const sal_Char* sCSS1_PV_dashed = "dashed";
const sal_Char* sCSS1_PV_solid = "solid";
const sal_Char* sCSS1_PV_double = "double";
const sal_Char* sCSS1_PV_groove = "groove";
const sal_Char* sCSS1_PV_ridge = "ridge";
const sal_Char* sCSS1_PV_inset = "inset";
const sal_Char* sCSS1_PV_outset = "outset";

const sal_Char* sCSS1_P_width = "width";
const sal_Char* sCSS1_P_max_width = "max-width";

const sal_Char* sCSS1_P_height = "height";

const sal_Char* sCSS1_P_float = "float";

const sal_Char* sCSS1_P_column_count = "column-count";

// Strings for positioning

const sal_Char* sCSS1_P_position = "position";

const sal_Char* sCSS1_PV_absolute = "absolute";
const sal_Char* sCSS1_PV_relative = "relative";
const sal_Char* sCSS1_PV_static = "static";

const sal_Char* sCSS1_P_left = "left";

const sal_Char* sCSS1_P_top = "top";

// Strings for printing extensions

const sal_Char* sCSS1_P_page_break_before = "page-break-before";
const sal_Char* sCSS1_P_page_break_after = "page-break-after";
const sal_Char* sCSS1_P_page_break_inside = "page-break-inside";
const sal_Char* sCSS1_P_size = "size";
const sal_Char* sCSS1_P_widows = "widows";
const sal_Char* sCSS1_P_visibility = "visibility";
const sal_Char* sCSS1_P_orphans = "orphans";
//const sal_Char* sCSS1_P_marks = "marks";

const sal_Char* sCSS1_PV_always = "always";
const sal_Char* sCSS1_PV_avoid = "avoid";

const sal_Char* sCSS1_PV_portrait = "portrait";
const sal_Char* sCSS1_PV_landscape = "landscape";

//const sal_Char* sCSS1_PV_crop = "crop";
//const sal_Char* sCSS1_PV_cross = "cross";

const sal_Char* sCSS1_class_abs_pos = "sd-abs-pos";

const sal_Char* sCSS1_P_so_language = "so-language";

const sal_Char* sCSS1_P_direction = "direction";
const sal_Char* sCSS1_PV_ltr = "ltr";
const sal_Char* sCSS1_PV_rtl = "rtl";
const sal_Char* sCSS1_PV_inherit = "inherit";

const sal_Char* sCSS1_P_display = "display";

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
