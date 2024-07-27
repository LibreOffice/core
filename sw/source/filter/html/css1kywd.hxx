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

#include <sal/config.h>

#include <string_view>

constexpr inline std::u16string_view sCSS_mimetype = u"text/css";

constexpr inline std::u16string_view sCSS1_page = u"page";
//constexpr inline std::u16string_view sCSS1_media = u"media";

constexpr inline OUString sCSS1_link = u"link"_ustr;
constexpr inline OUString sCSS1_visited = u"visited"_ustr;
constexpr inline std::u16string_view sCSS1_first_letter = u"first-letter";

constexpr inline std::u16string_view sCSS1_left = u"left";
constexpr inline std::u16string_view sCSS1_right = u"right";
constexpr inline std::u16string_view sCSS1_first = u"first";

constexpr inline std::u16string_view sCSS1_url = u"url";

constexpr inline std::string_view sCSS1_UNIT_pt = "pt";
constexpr inline std::string_view sCSS1_UNIT_mm = "mm";
constexpr inline std::string_view sCSS1_UNIT_cm = "cm";
constexpr inline std::string_view sCSS1_UNIT_pc = "pc";
constexpr inline std::string_view sCSS1_UNIT_inch = "in";
constexpr inline std::string_view sCSS1_UNIT_px = "px";

// Strings for font properties

constexpr inline std::string_view sCSS1_P_font_family = "font-family";

constexpr inline OString sCSS1_PV_serif = "serif"_ostr;
constexpr inline OString sCSS1_PV_sans_serif = "sans-serif"_ostr;
constexpr inline OString sCSS1_PV_cursive = "cursive"_ostr;
constexpr inline OString sCSS1_PV_fantasy = "fantasy"_ostr;
constexpr inline OString sCSS1_PV_monospace = "monospace"_ostr;

constexpr inline std::string_view sCSS1_P_font_style = "font-style";

constexpr inline std::string_view sCSS1_PV_normal = "normal";
constexpr inline std::string_view sCSS1_PV_italic = "italic";
constexpr inline std::string_view sCSS1_PV_oblique = "oblique";

constexpr inline std::string_view sCSS1_P_font_variant = "font-variant";

//constexpr inline std::string_view sCSS1_PV_normal = "normal";
constexpr inline std::string_view sCSS1_PV_small_caps = "small-caps";

constexpr inline std::string_view sCSS1_P_text_transform = "text-transform";

constexpr inline std::string_view sCSS1_PV_capitalize = "capitalize";
constexpr inline std::string_view sCSS1_PV_uppercase = "uppercase";
constexpr inline std::string_view sCSS1_PV_lowercase = "lowercase";

constexpr inline std::string_view sCSS1_P_font_weight = "font-weight";

constexpr inline std::string_view sCSS1_PV_extra_light = "extra-light";
constexpr inline std::string_view sCSS1_PV_light = "light";
constexpr inline std::string_view sCSS1_PV_demi_light = "demi-light";
//constexpr inline std::string_view sCSS1_PV_medium = "medium";
constexpr inline std::string_view sCSS1_PV_demi_bold = "demi-bold";
constexpr inline std::string_view sCSS1_PV_bold = "bold";
constexpr inline std::string_view sCSS1_PV_extra_bold = "extra-bold";

constexpr inline std::string_view sCSS1_P_font_size = "font-size";

constexpr inline std::string_view sCSS1_P_font = "font";

// Strings for color and background properties

constexpr inline std::string_view sCSS1_P_color = "color";

constexpr inline std::string_view sCSS1_P_background = "background";
constexpr inline std::string_view sCSS1_P_background_color = "background-color";

constexpr inline std::u16string_view sCSS1_PV_transparent = u"transparent";

constexpr inline std::u16string_view sCSS1_PV_repeat = u"repeat";
constexpr inline std::u16string_view sCSS1_PV_no_repeat = u"no-repeat";

constexpr inline std::u16string_view sCSS1_PV_top = u"top";
constexpr inline std::u16string_view sCSS1_PV_middle = u"middle";
constexpr inline std::u16string_view sCSS1_PV_bottom = u"bottom";

constexpr inline std::u16string_view sCSS1_PV_scroll = u"scroll";

// Strings for text properties

constexpr inline std::string_view sCSS1_P_letter_spacing = "letter-spacing";

constexpr inline std::string_view sCSS1_P_text_decoration = "text-decoration";

constexpr inline std::string_view sCSS1_PV_none = "none";
constexpr inline std::string_view sCSS1_PV_underline = "underline";
constexpr inline std::string_view sCSS1_PV_overline = "overline";
constexpr inline std::string_view sCSS1_PV_line_through = "line-through";
constexpr inline std::string_view sCSS1_PV_blink = "blink";

constexpr inline std::string_view sCSS1_P_text_align = "text-align";

constexpr inline std::string_view sCSS1_PV_left = "left";
constexpr inline std::string_view sCSS1_PV_center = "center";
constexpr inline std::string_view sCSS1_PV_right = "right";
constexpr inline std::string_view sCSS1_PV_justify = "justify";

constexpr inline std::string_view sCSS1_P_text_indent = "text-indent";

constexpr inline std::string_view sCSS1_P_line_height = "line-height";

constexpr inline std::string_view sCSS1_P_list_style_type = "list-style-type";

// Strings for box properties

constexpr inline std::string_view sCSS1_P_margin_left = "margin-left";
constexpr inline std::string_view sCSS1_P_margin_right = "margin-right";
constexpr inline std::string_view sCSS1_P_margin_top = "margin-top";
constexpr inline std::string_view sCSS1_P_margin_bottom = "margin-bottom";
constexpr inline std::string_view sCSS1_P_margin = "margin";

constexpr inline std::string_view sCSS1_P_padding_top = "padding-top";
constexpr inline std::string_view sCSS1_P_padding_bottom = "padding-bottom";
constexpr inline std::string_view sCSS1_P_padding_left = "padding-left";
constexpr inline std::string_view sCSS1_P_padding_right = "padding-right";
constexpr inline std::string_view sCSS1_P_padding = "padding";

constexpr inline std::string_view sCSS1_PV_auto = "auto";

constexpr inline std::string_view sCSS1_P_border_left_width = "border-left-width";
constexpr inline std::string_view sCSS1_P_border_right_width = "border-right-width";
constexpr inline std::string_view sCSS1_P_border_top_width = "border-top-width";
constexpr inline std::string_view sCSS1_P_border_bottom_width = "border-bottom-width";
constexpr inline std::string_view sCSS1_P_border_width = "border-width";
constexpr inline std::string_view sCSS1_P_border_color = "border-color";
constexpr inline std::string_view sCSS1_P_border_style = "border-style";
constexpr inline std::string_view sCSS1_P_border_left = "border-left";
constexpr inline std::string_view sCSS1_P_border_right = "border-right";
constexpr inline std::string_view sCSS1_P_border_top = "border-top";
constexpr inline std::string_view sCSS1_P_border_bottom = "border-bottom";
constexpr inline std::string_view sCSS1_P_border = "border";

//constexpr inline std::string_view sCSS1_PV_none = "none";
constexpr inline std::string_view sCSS1_PV_dotted = "dotted";
constexpr inline std::string_view sCSS1_PV_dashed = "dashed";
constexpr inline std::string_view sCSS1_PV_solid = "solid";
constexpr inline std::string_view sCSS1_PV_double = "double";
constexpr inline std::string_view sCSS1_PV_groove = "groove";
constexpr inline std::string_view sCSS1_PV_ridge = "ridge";
constexpr inline std::string_view sCSS1_PV_inset = "inset";
constexpr inline std::string_view sCSS1_PV_outset = "outset";

constexpr inline std::string_view sCSS1_P_width = "width";
constexpr inline std::string_view sCSS1_P_max_width = "max-width";

constexpr inline std::string_view sCSS1_P_height = "height";

constexpr inline std::string_view sCSS1_P_float = "float";

constexpr inline std::string_view sCSS1_P_column_count = "column-count";
constexpr inline std::string_view sCSS1_P_dir = "dir";

// Strings for positioning

constexpr inline std::string_view sCSS1_P_position = "position";

constexpr inline std::string_view sCSS1_PV_absolute = "absolute";

constexpr inline std::string_view sCSS1_P_left = "left";

constexpr inline std::string_view sCSS1_P_top = "top";

// Strings for printing extensions

constexpr inline std::string_view sCSS1_P_page_break_before = "page-break-before";
constexpr inline std::string_view sCSS1_P_page_break_after = "page-break-after";
constexpr inline std::string_view sCSS1_P_page_break_inside = "page-break-inside";
constexpr inline std::string_view sCSS1_P_size = "size";
constexpr inline std::string_view sCSS1_P_widows = "widows";
constexpr inline std::string_view sCSS1_P_visibility = "visibility";
constexpr inline std::string_view sCSS1_P_orphans = "orphans";
//constexpr inline std::string_view sCSS1_P_marks = "marks";

constexpr inline std::string_view sCSS1_PV_always = "always";
constexpr inline std::string_view sCSS1_PV_avoid = "avoid";

constexpr inline std::string_view sCSS1_PV_portrait = "portrait";
constexpr inline std::string_view sCSS1_PV_landscape = "landscape";

//constexpr inline std::string_view sCSS1_PV_crop = "crop";
//constexpr inline std::string_view sCSS1_PV_cross = "cross";

constexpr inline std::string_view sCSS1_P_so_language = "so-language";
constexpr inline std::string_view sCSS1_P_direction = "direction";
constexpr inline std::string_view sCSS1_PV_ltr = "ltr";
constexpr inline std::string_view sCSS1_PV_rtl = "rtl";
constexpr inline std::string_view sCSS1_PV_inherit = "inherit";

constexpr inline std::string_view sCSS1_P_display = "display";

constexpr inline std::string_view sCSS1_white_space = "white-space";

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
