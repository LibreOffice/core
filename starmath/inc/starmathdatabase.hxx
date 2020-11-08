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

#include "token.hxx"

// Starmath color types
// In order to add them to starmath, edit the SmColorTokenTableEntry lists on
// /core/starmath/source/starmathdatabase.css .

// HTML
// https://developer.mozilla.org/en-US/docs/Web/CSS/color_value#Color_Keywords
/* CSS Level 1 */
constexpr Color COL_SM_BLACK(0x000000);
constexpr Color COL_SM_SILVER(0xC0C0C0);
constexpr Color COL_SM_GRAY(0x808080);
constexpr Color COL_SM_WHITE(0xFFFFFF);
constexpr Color COL_SM_MAROON(0x800000);
constexpr Color COL_SM_RED(0xFF0000);
constexpr Color COL_SM_PURPLE(0x800080);
constexpr Color COL_SM_FUCHSIA(0xFF00FF);
constexpr Color COL_SM_GREEN(0x008000);
constexpr Color COL_SM_LIME(0x00FF00);
constexpr Color COL_SM_OLIVE(0x808000);
constexpr Color COL_SM_YELLOW(0xFFFF00);
constexpr Color COL_SM_NAVY(0x000080);
constexpr Color COL_SM_BLUE(0x0000FF);
constexpr Color COL_SM_TEAL(0x008080);
constexpr Color COL_SM_AQUA(0x00FFFF);
/* CSS Level 2 */
// TODO
/* CSS Level 3 */
// TODO
/* CSS Level 4 */
constexpr Color COL_SM_REBECCAPURPLE(0x663399);
/* dvipsnames */
// For now only five colors.
// In a future all of them.
// https://www.overleaf.com/learn/latex/Using_colours_in_LaTeX
constexpr Color COL_SM_DIV_APRICOT(0xFFB781);
constexpr Color COL_SM_DIV_AQUAMARINE(0x1BBEC1);
constexpr Color COL_SM_DIV_BITTERSWEET(0xCF4B16);
constexpr Color COL_SM_DIV_BLACK(0xCF4B16);
constexpr Color COL_SM_DIV_BLUE(0x102694);
/* Iconic colors */
// https://design.ubuntu.com/brand/colour-palette/
constexpr Color COL_SM_UBUNTU_ORANGE(0xE95420);
// https://www.debian.org/logos/   Picked from SVG logo
constexpr Color COL_SM_DEBIAN_MAGENTA(0xA80030);
// https://libreoffice.org/
constexpr Color COL_SM_LO_GREEN(0x00A500);

namespace starmathdatabase
{
// Variables containing color information.
extern const SmColorTokenTableEntry aColorTokenTableParse[31];
extern const SmColorTokenTableEntry aColorTokenTableHTML[20];
extern const SmColorTokenTableEntry aColorTokenTableDVIPSNAMES[5];

/**
  * Identifies operator chars tokens for importing mathml.
  * Identifies from char cChar
  * @param cChar
  * @return closing fences' token
  */
SmToken Identify_SmXMLOperatorContext_Impl(sal_Unicode cChar, bool bIsStretchy = true);

/**
  * Identifies opening / closing brace tokens for importing mathml.
  * Identifies from char cChar
  * @param cChar
  * @return closing fences' token
  */
SmToken Identify_PrefixPostfix_SmXMLOperatorContext_Impl(sal_Unicode cChar);

/**
  * Identifies opening brace tokens for importing mathml.
  * Identifies from char cChar
  * @param cChar
  * @return closing fences' token
  */
SmToken Identify_Prefix_SmXMLOperatorContext_Impl(sal_Unicode cChar);

/**
  * Identifies closing brace tokens for importing mathml.
  * Identifies from char cChar
  * @param cChar
  * @return closing fences' token
  */
SmToken Identify_Postfix_SmXMLOperatorContext_Impl(sal_Unicode cChar);

/**
  * Identifies color from color code cColor.
  * It will be returned with the parser syntax.
  * @param cColor
  * @param parser color
  */
std::unique_ptr<SmColorTokenTableEntry> Identify_Color_Parser(sal_uInt32 cColor);

/**
  * Identifies color from color code cColor.
  * It will be returned with the HTML syntax.
  * @param cColor
  * @param parser color
  */
std::unique_ptr<SmColorTokenTableEntry> Identify_Color_HTML(sal_uInt32 cColor);

/**
  * Identifies color from color code cColor.
  * It will be returned with the dvipsnames syntax.
  * @param cColor
  * @param parser color
  */
std::unique_ptr<SmColorTokenTableEntry> Identify_Color_DVIPSNAMES(sal_uInt32 cColor);

/**
  * Identifies color from color name.
  * It will be returned with the parser syntax.
  * @param cColor
  * @param parser color
  */
std::unique_ptr<SmColorTokenTableEntry> Identify_ColorName_Parser(const OUString& colorname);

/**
  * Identifies color from color name.
  * It will be returned with the HTML syntax.
  * @param cColor
  * @param parser color
  */
std::unique_ptr<SmColorTokenTableEntry> Identify_ColorName_HTML(const OUString& colorname);

/**
  * Identifies color from color name.
  * It will be returned with the dvipsnames syntax.
  * @param cColor
  * @param parser color
  */
std::unique_ptr<SmColorTokenTableEntry> Identify_ColorName_DVIPSNAMES(const OUString& colorname);
}
