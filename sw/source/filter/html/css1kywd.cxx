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

const char* const sCSS_mimetype = "text/css";

const char* const sCSS1_import = "import";

const char* const sCSS1_page = "page";
//const char* const sCSS1_media = "media";

const char* const sCSS1_important = "important";

const char* const sCSS1_link = "link";
const char* const sCSS1_visited = "visited";
const char* const sCSS1_first_letter = "first-letter";

const char* const sCSS1_left = "left";
const char* const sCSS1_right = "right";
const char* const sCSS1_first = "first";

const char* const sCSS1_url = "url";
const char* const sCSS1_rgb = "rgb";

const char* const sCSS1_UNIT_pt = "pt";
const char* const sCSS1_UNIT_mm = "mm";
const char* const sCSS1_UNIT_cm = "cm";
const char* const sCSS1_UNIT_pc = "pc";
const char* const sCSS1_UNIT_inch = "in";
const char* const sCSS1_UNIT_px = "px";
const char* const sCSS1_UNIT_em = "em";
const char* const sCSS1_UNIT_ex = "ex";

// Strings for font properties

const char* const sCSS1_P_font_family = "font-family";

const char* const sCSS1_PV_serif = "serif";
const char* const sCSS1_PV_sans_serif = "sans-serif";
const char* const sCSS1_PV_cursive = "cursive";
const char* const sCSS1_PV_fantasy = "fantasy";
const char* const sCSS1_PV_monospace = "monospace";

const char* const sCSS1_P_font_style = "font-style";

const char* const sCSS1_PV_normal = "normal";
const char* const sCSS1_PV_italic = "italic";
const char* const sCSS1_PV_oblique = "oblique";

const char* const sCSS1_P_font_variant = "font-variant";

//const char* const sCSS1_PV_normal = "normal";
const char* const sCSS1_PV_small_caps = "small-caps";

const char* const sCSS1_P_font_weight = "font-weight";

const char* const sCSS1_PV_extra_light = "extra-light";
const char* const sCSS1_PV_light = "light";
const char* const sCSS1_PV_demi_light = "demi-light";
//const char* const sCSS1_PV_medium = "medium";
const char* const sCSS1_PV_demi_bold = "demi-bold";
const char* const sCSS1_PV_bold = "bold";
const char* const sCSS1_PV_extra_bold = "extra-bold";
const char* const sCSS1_PV_lighter = "lighter";
const char* const sCSS1_PV_bolder = "bolder";

const char* const sCSS1_P_text_transform = "text-transform";

const char* const sCSS1_PV_capitalize = "capitalize";
const char* const sCSS1_PV_uppercase = "uppercase";
const char* const sCSS1_PV_lowercase = "lowercase";

const char* const sCSS1_P_font_size = "font-size";

const char* const sCSS1_PV_xx_small = "xx-small";
const char* const sCSS1_PV_x_small = "x-small";
const char* const sCSS1_PV_small = "small";
const char* const sCSS1_PV_medium = "medium";
const char* const sCSS1_PV_large = "large";
const char* const sCSS1_PV_x_large = "x-large";
const char* const sCSS1_PV_xx_large = "xx-large";

const char* const sCSS1_PV_larger = "larger";
const char* const sCSS1_PV_smaller = "smaller";

const char* const sCSS1_P_font = "font";

// Strings for color and background properties

const char* const sCSS1_P_color = "color";

const char* const sCSS1_P_background = "background";
const char* const sCSS1_P_background_color = "background-color";

const char* const sCSS1_PV_transparent = "transparent";

const char* const sCSS1_PV_repeat = "repeat";
const char* const sCSS1_PV_repeat_x = "repeat-x";
const char* const sCSS1_PV_repeat_y = "repeat-y";
const char* const sCSS1_PV_no_repeat = "no-repeat";

const char* const sCSS1_PV_top = "top";
const char* const sCSS1_PV_middle = "middle";
const char* const sCSS1_PV_bottom = "bottom";

const char* const sCSS1_PV_scroll = "scroll";

// Strings for text properties

const char* const sCSS1_P_letter_spacing = "letter-spacing";

const char* const sCSS1_P_text_decoration = "text-decoration";

const char* const sCSS1_PV_none = "none";
const char* const sCSS1_PV_underline = "underline";
const char* const sCSS1_PV_overline = "overline";
const char* const sCSS1_PV_line_through = "line-through";
const char* const sCSS1_PV_blink = "blink";

const char* const sCSS1_P_text_align = "text-align";

const char* const sCSS1_PV_left = "left";
const char* const sCSS1_PV_center = "center";
const char* const sCSS1_PV_right = "right";
const char* const sCSS1_PV_justify = "justify";

const char* const sCSS1_P_text_indent = "text-indent";

const char* const sCSS1_P_line_height = "line-height";
const char* const sCSS1_P_list_style_type = "list-style-type";

// Strings for box properties

const char* const sCSS1_P_margin_left = "margin-left";
const char* const sCSS1_P_margin_right = "margin-right";
const char* const sCSS1_P_margin_top = "margin-top";
const char* const sCSS1_P_margin_bottom = "margin-bottom";
const char* const sCSS1_P_margin = "margin";

const char* const sCSS1_P_padding_top = "padding-top";
const char* const sCSS1_P_padding_bottom = "padding-bottom";
const char* const sCSS1_P_padding_left = "padding-left";
const char* const sCSS1_P_padding_right = "padding-right";
const char* const sCSS1_P_padding = "padding";

const char* const sCSS1_PV_auto = "auto";

const char* const sCSS1_P_border_left_width = "border-left-width";
const char* const sCSS1_P_border_right_width = "border-right-width";
const char* const sCSS1_P_border_top_width = "border-top-width";
const char* const sCSS1_P_border_bottom_width = "border-bottom-width";
const char* const sCSS1_P_border_width = "border-width";
const char* const sCSS1_P_border_color = "border-color";
const char* const sCSS1_P_border_style = "border-style";
const char* const sCSS1_P_border_left = "border-left";
const char* const sCSS1_P_border_right = "border-right";
const char* const sCSS1_P_border_top = "border-top";
const char* const sCSS1_P_border_bottom = "border-bottom";
const char* const sCSS1_P_border = "border";

const char* const sCSS1_PV_thin = "thin";
//const char* const sCSS1_PV_medium = "medium";
const char* const sCSS1_PV_thick = "thick";

//const char* const sCSS1_PV_none = "none";
const char* const sCSS1_PV_dotted = "dotted";
const char* const sCSS1_PV_dashed = "dashed";
const char* const sCSS1_PV_solid = "solid";
const char* const sCSS1_PV_double = "double";
const char* const sCSS1_PV_groove = "groove";
const char* const sCSS1_PV_ridge = "ridge";
const char* const sCSS1_PV_inset = "inset";
const char* const sCSS1_PV_outset = "outset";

const char* const sCSS1_P_width = "width";
const char* const sCSS1_P_max_width = "max-width";

const char* const sCSS1_P_height = "height";

const char* const sCSS1_P_float = "float";

const char* const sCSS1_P_column_count = "column-count";

// Strings for positioning

const char* const sCSS1_P_position = "position";

const char* const sCSS1_PV_absolute = "absolute";
const char* const sCSS1_PV_relative = "relative";
const char* const sCSS1_PV_static = "static";

const char* const sCSS1_P_left = "left";

const char* const sCSS1_P_top = "top";

// Strings for printing extensions

const char* const sCSS1_P_page_break_before = "page-break-before";
const char* const sCSS1_P_page_break_after = "page-break-after";
const char* const sCSS1_P_page_break_inside = "page-break-inside";
const char* const sCSS1_P_size = "size";
const char* const sCSS1_P_widows = "widows";
const char* const sCSS1_P_visibility = "visibility";
const char* const sCSS1_P_orphans = "orphans";
//const char* const sCSS1_P_marks = "marks";

const char* const sCSS1_PV_always = "always";
const char* const sCSS1_PV_avoid = "avoid";

const char* const sCSS1_PV_portrait = "portrait";
const char* const sCSS1_PV_landscape = "landscape";

//const char* const sCSS1_PV_crop = "crop";
//const char* const sCSS1_PV_cross = "cross";

const char* const sCSS1_class_abs_pos = "sd-abs-pos";

const char* const sCSS1_P_so_language = "so-language";

const char* const sCSS1_P_direction = "direction";
const char* const sCSS1_PV_ltr = "ltr";
const char* const sCSS1_PV_rtl = "rtl";
const char* const sCSS1_PV_inherit = "inherit";

const char* const sCSS1_P_display = "display";

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
