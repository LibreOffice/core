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

#pragma once

#include "token.hxx"

// Starmath color types
// In order to add them to starmath, edit the SmColorTokenTableEntry lists on
// /core/starmath/source/starmathdatabase.css .

// HTML
// https://developer.mozilla.org/en-US/docs/Web/CSS/color_value#Color_Keywords
/* CSS Level 1 */
constexpr Color COL_SM_BLACK(0x00, 0x00, 0x00);
constexpr Color COL_SM_SILVER(0xC0, 0xC0, 0xC0);
constexpr Color COL_SM_GRAY(0x80, 0x80, 0x80);
constexpr Color COL_SM_WHITE(0xFF, 0xFF, 0xFF);
constexpr Color COL_SM_MAROON(0x80, 0x00, 0x00);
constexpr Color COL_SM_RED(0xFF, 0x00, 0x00);
constexpr Color COL_SM_PURPLE(0x80, 0x00, 0x80);
constexpr Color COL_SM_FUCHSIA(0xFF, 0x00, 0xFF);
constexpr Color COL_SM_GREEN(0x00, 0x80, 0x00);
constexpr Color COL_SM_LIME(0x00, 0xFF, 0x00);
constexpr Color COL_SM_OLIVE(0x80, 0x80, 0x00);
constexpr Color COL_SM_YELLOW(0xFF, 0xFF, 0x00);
constexpr Color COL_SM_NAVY(0x00, 0x00, 0x80);
constexpr Color COL_SM_BLUE(0x00, 0x00, 0xFF);
constexpr Color COL_SM_TEAL(0x00, 0x80, 0x80);
constexpr Color COL_SM_AQUA(0x00, 0xFF, 0xFF);
/* CSS Level 2 */
constexpr Color COL_SM_ORANGE(0xFF, 0xA5, 0x00);
/* CSS Level 3 */
constexpr Color COL_SM_ALICEBLUE(0xF0, 0xF8, 0xFF);
constexpr Color COL_SM_ANTIQUEWHITE(0xFA, 0xEB, 0xD7);
constexpr Color COL_SM_AQUAMARINE(0x7F, 0xFF, 0xD4);
constexpr Color COL_SM_AZURE(0xF0, 0xFF, 0xFF);
constexpr Color COL_SM_BEIGE(0xF5, 0xF5, 0xDC);
constexpr Color COL_SM_BISQUE(0xFF, 0xE4, 0xC4);
constexpr Color COL_SM_BLANCHEDALMOND(0xFF, 0xEB, 0xCD);
constexpr Color COL_SM_BLUEVIOLET(0x8A, 0x2B, 0xE2);
constexpr Color COL_SM_BROWN(0xA5, 0x2A, 0x2A);
constexpr Color COL_SM_BURLYWOOD(0xDE, 0xB8, 0x87);
constexpr Color COL_SM_CADETBLUE(0x5F, 0x9E, 0xA0);
constexpr Color COL_SM_CHARTREUSE(0x7F, 0xFF, 0x00);
constexpr Color COL_SM_CHOCOLATE(0xD2, 0x69, 0x1E);
constexpr Color COL_SM_CORAL(0xFF, 0x7F, 0x50);
constexpr Color COL_SM_CORNFLOWERBLUE(0x64, 0x95, 0xED);
constexpr Color COL_SM_CORNSILK(0xFF, 0xF8, 0xDC);
constexpr Color COL_SM_CRIMSON(0xDC, 0x14, 0x3C);
constexpr Color COL_SM_CYAN(0x00, 0xFF, 0xFF);
constexpr Color COL_SM_DARKBLUE(0x00, 0x00, 0x8B);
constexpr Color COL_SM_DARKCYAN(0x00, 0x8B, 0x8B);
constexpr Color COL_SM_DARKGOLDENROD(0xB8, 0x86, 0x0B);
constexpr Color COL_SM_DARKGRAY(0xA9, 0xA9, 0xA9);
constexpr Color COL_SM_DARKGREEN(0x00, 0x64, 0x00);
constexpr Color COL_SM_DARKGREY(0xA9, 0xA9, 0xA9);
constexpr Color COL_SM_DARKKHAKI(0xBD, 0xB7, 0x6B);
constexpr Color COL_SM_DARKMAGENTA(0x8B, 0x00, 0x8B);
constexpr Color COL_SM_DARKOLIVEGREEN(0x55, 0x6B, 0x2F);
constexpr Color COL_SM_DARKORANGE(0xFF, 0x8C, 0x00);
constexpr Color COL_SM_DARKORCHID(0x99, 0x32, 0xCC);
constexpr Color COL_SM_DARKRED(0x8B, 0x00, 0x00);
constexpr Color COL_SM_DARKSALMON(0xE9, 0x96, 0x7A);
constexpr Color COL_SM_DARKSEAGREEN(0x8F, 0xBC, 0x8F);
constexpr Color COL_SM_DARKSLATEBLUE(0x48, 0x3D, 0x8B);
constexpr Color COL_SM_DARKSLATEGRAY(0x2F, 0x4F, 0x4F);
constexpr Color COL_SM_DARKSLATEGREY(0x2F, 0x4F, 0x4F);
constexpr Color COL_SM_DARKTURQUOISE(0x00, 0xCE, 0xD1);
constexpr Color COL_SM_DARKVIOLET(0x94, 0x00, 0xD3);
constexpr Color COL_SM_DEEPPINK(0xFF, 0x14, 0x93);
constexpr Color COL_SM_DEEPSKYBLUE(0x00, 0xBF, 0xFF);
constexpr Color COL_SM_DIMGRAY(0x69, 0x69, 0x69);
constexpr Color COL_SM_DIMGREY(0x69, 0x69, 0x69);
constexpr Color COL_SM_DODGERBLUE(0x1E, 0x90, 0xFF);
constexpr Color COL_SM_FIREBRICK(0xB2, 0x22, 0x22);
constexpr Color COL_SM_FLORALWHITE(0xFF, 0xFA, 0xF0);
constexpr Color COL_SM_FORESTGREEN(0x22, 0x8B, 0x22);
constexpr Color COL_SM_GAINSBORO(0xDC, 0xDC, 0xDC);
constexpr Color COL_SM_GHOSTWHITE(0xF8, 0xF8, 0xFF);
constexpr Color COL_SM_GOLD(0xFF, 0xD7, 0x00);
constexpr Color COL_SM_GOLDENROD(0xDA, 0xA5, 0x20);
constexpr Color COL_SM_GREENYELLOW(0xAD, 0xFF, 0x2F);
constexpr Color COL_SM_GREY(0x80, 0x80, 0x80);
constexpr Color COL_SM_HONEYDEW(0xF0, 0xFF, 0xF0);
constexpr Color COL_SM_HOTPINK(0xFF, 0x69, 0xB4);
constexpr Color COL_SM_INDIANRED(0xCD, 0x5C, 0x5C);
constexpr Color COL_SM_INDIGO(0x4B, 0x00, 0x82);
constexpr Color COL_SM_IVORY(0xFF, 0xFF, 0xF0);
constexpr Color COL_SM_KHAKI(0xF0, 0xE6, 0x8C);
constexpr Color COL_SM_LAVENDER(0xE6, 0xE6, 0xFA);
constexpr Color COL_SM_LAVENDERBLUSH(0xFF, 0xF0, 0xF5);
constexpr Color COL_SM_LAWNGREEN(0x7C, 0xFC, 0x00);
constexpr Color COL_SM_LEMONCHIFFON(0xFF, 0xFA, 0xCD);
constexpr Color COL_SM_LIGHTBLUE(0xAD, 0xD8, 0xE6);
constexpr Color COL_SM_LIGHTCORAL(0xF0, 0x80, 0x80);
constexpr Color COL_SM_LIGHTCYAN(0xE0, 0xFF, 0xFF);
constexpr Color COL_SM_LIGHTGOLDENRODYELLOW(0xFA, 0xFA, 0xD2);
constexpr Color COL_SM_LIGHTGRAY(0xD3, 0xD3, 0xD3);
constexpr Color COL_SM_LIGHTGREEN(0x90, 0xEE, 0x90);
constexpr Color COL_SM_LIGHTGREY(0xD3, 0xD3, 0xD3);
constexpr Color COL_SM_LIGHTPINK(0xFF, 0xB6, 0xC1);
constexpr Color COL_SM_LIGHTSALMON(0xFF, 0xA0, 0x7A);
constexpr Color COL_SM_LIGHTSEAGREEN(0x20, 0xB2, 0xAA);
constexpr Color COL_SM_LIGHTSKYBLUE(0x87, 0xCE, 0xFA);
constexpr Color COL_SM_LIGHTSLATEGRAY(0x77, 0x88, 0x99);
constexpr Color COL_SM_LIGHTSLATEGREY(0x77, 0x88, 0x99);
constexpr Color COL_SM_LIGHTSTEELBLUE(0xB0, 0xC4, 0xDE);
constexpr Color COL_SM_LIGHTYELLOW(0xFF, 0xFF, 0xE0);
constexpr Color COL_SM_LIMEGREEN(0x32, 0xCD, 0x32);
constexpr Color COL_SM_LINEN(0xFA, 0xF0, 0xE6);
constexpr Color COL_SM_MAGENTA(0xFF, 0x00, 0xFF);
constexpr Color COL_SM_MEDIUMAQUAMARINE(0x66, 0xCD, 0xAA);
constexpr Color COL_SM_MEDIUMBLUE(0x00, 0x00, 0xCD);
constexpr Color COL_SM_MEDIUMORCHID(0xBA, 0x55, 0xD3);
constexpr Color COL_SM_MEDIUMPURPLE(0x93, 0x70, 0xDB);
constexpr Color COL_SM_MEDIUMSEAGREEN(0x3C, 0xB3, 0x71);
constexpr Color COL_SM_MEDIUMSLATEBLUE(0x7B, 0x68, 0xEE);
constexpr Color COL_SM_MEDIUMSPRINGGREEN(0x00, 0xFA, 0x9A);
constexpr Color COL_SM_MEDIUMTURQUOISE(0x48, 0xD1, 0xCC);
constexpr Color COL_SM_MEDIUMVIOLETRED(0xC7, 0x15, 0x85);
constexpr Color COL_SM_MIDNIGHTBLUE(0x19, 0x19, 0x70);
constexpr Color COL_SM_MINTCREAM(0xF5, 0xFF, 0xFA);
constexpr Color COL_SM_MISTYROSE(0xFF, 0xE4, 0xE1);
constexpr Color COL_SM_MOCCASIN(0xFF, 0xE4, 0xB5);
constexpr Color COL_SM_NAVAJOWHITE(0xFF, 0xDE, 0xAD);
constexpr Color COL_SM_OLDLACE(0xFD, 0xF5, 0xE6);
constexpr Color COL_SM_OLIVEDRAB(0x6B, 0x8E, 0x23);
constexpr Color COL_SM_ORANGERED(0xFF, 0x45, 0x00);
constexpr Color COL_SM_ORCHID(0xDA, 0x70, 0xD6);
constexpr Color COL_SM_PALEGOLDENROD(0xEE, 0xE8, 0xAA);
constexpr Color COL_SM_PALEGREEN(0x98, 0xFB, 0x98);
constexpr Color COL_SM_PALETURQUOISE(0xAF, 0xEE, 0xEE);
constexpr Color COL_SM_PALEVIOLETRED(0xDB, 0x70, 0x93);
constexpr Color COL_SM_PAPAYAWHIP(0xFF, 0xEF, 0xD5);
constexpr Color COL_SM_PEACHPUFF(0xFF, 0xDA, 0xB9);
constexpr Color COL_SM_PERU(0xCD, 0x85, 0x3F);
constexpr Color COL_SM_PINK(0xFF, 0xC0, 0xCB);
constexpr Color COL_SM_PLUM(0xDD, 0xA0, 0xDD);
constexpr Color COL_SM_POWDERBLUE(0xB0, 0xE0, 0xE6);
constexpr Color COL_SM_ROSYBROWN(0xBC, 0x8F, 0x8F);
constexpr Color COL_SM_ROYALBLUE(0x41, 0x69, 0xE1);
constexpr Color COL_SM_SADDLEBROWN(0x8B, 0x45, 0x13);
constexpr Color COL_SM_SALMON(0xFA, 0x80, 0x72);
constexpr Color COL_SM_SANDYBROWN(0xF4, 0xA4, 0x60);
constexpr Color COL_SM_SEAGREEN(0x2E, 0x8B, 0x57);
constexpr Color COL_SM_SEASHELL(0xFF, 0xF5, 0xEE);
constexpr Color COL_SM_SIENNA(0xA0, 0x52, 0x2D);
constexpr Color COL_SM_SKYBLUE(0x87, 0xCE, 0xEB);
constexpr Color COL_SM_SLATEBLUE(0x6A, 0x5A, 0xCD);
constexpr Color COL_SM_SLATEGRAY(0x70, 0x80, 0x90);
constexpr Color COL_SM_SLATEGREY(0x70, 0x80, 0x90);
constexpr Color COL_SM_SNOW(0xFF, 0xFA, 0xFA);
constexpr Color COL_SM_SPRINGGREEN(0x00, 0xFF, 0x7F);
constexpr Color COL_SM_STEELBLUE(0x46, 0x82, 0xB4);
constexpr Color COL_SM_TAN(0xD2, 0xB4, 0x8C);
constexpr Color COL_SM_THISTLE(0xD8, 0xBF, 0xD8);
constexpr Color COL_SM_TOMATO(0xFF, 0x63, 0x47);
constexpr Color COL_SM_TURQUOISE(0x40, 0xE0, 0xD0);
constexpr Color COL_SM_VIOLET(0xEE, 0x82, 0xEE);
constexpr Color COL_SM_WHEAT(0xF5, 0xDE, 0xB3);
constexpr Color COL_SM_WHITESMOKE(0xF5, 0xF5, 0xF5);
constexpr Color COL_SM_YELLOWGREEN(0x9A, 0xCD, 0x32);
/* CSS Level 4 */
constexpr Color COL_SM_REBECCAPURPLE(0x66, 0x33, 0x99);
/* dvipsnames */
// For now only five colors.
// In a future all of them.
// https://www.overleaf.com/learn/latex/Using_colours_in_LaTeX
constexpr Color COL_SM_DIV_APRICOT(0xFF, 0xB7, 0x81);
constexpr Color COL_SM_DIV_AQUAMARINE(0x1B, 0xBE, 0xC1);
constexpr Color COL_SM_DIV_BITTERSWEET(0xCF, 0x4B, 0x16);
constexpr Color COL_SM_DIV_BLACK(0xCF, 0x4B, 0x16);
constexpr Color COL_SM_DIV_BLUE(0x10, 0x26, 0x94);
/* Iconic colors */
// https://design.ubuntu.com/brand/colour-palette/
constexpr Color COL_SM_UBUNTU_ORANGE(0xE9, 0x54, 0x20);
// https://www.debian.org/logos/   Picked from SVG logo
constexpr Color COL_SM_DEBIAN_MAGENTA(0xA8, 0x00, 0x30);
// https://libreoffice.org/
constexpr Color COL_SM_LO_GREEN(0x00, 0xA5, 0x00);

namespace starmathdatabase
{
// Variables containing color information.
extern const SmColorTokenTableEntry aColorTokenTableParse[159];
extern const SmColorTokenTableEntry aColorTokenTableHTML[148];
extern const SmColorTokenTableEntry aColorTokenTableMATHML[16];
extern const SmColorTokenTableEntry aColorTokenTableDVIPSNAMES[5];

/**
  * Identifies operator chars tokens for importing mathml.
  * Identifies from char cChar
  *
  * While loading MO or MI elements might find an unicode16 symbol.
  * This code allows to generate appropriate token for them.
  *
  * @param cChar
  * @return closing fences' token
  */
SmToken Identify_SmXMLOperatorContext_Impl(sal_Unicode cChar, bool bIsStretchy = true);

/**
  * Identifies opening / closing brace tokens for importing mathml.
  * Identifies from char cChar
  *
  * While loading MO fenced elements might find braces symbols.
  * This code allows to generate appropriate token for them.
  *
  * @param cChar
  * @return closing fences' token
  */
SmToken Identify_PrefixPostfix_SmXMLOperatorContext_Impl(sal_Unicode cChar);

/**
  * Identifies opening brace tokens for importing mathml.
  * Identifies from char cChar
  *
  * While loading MO elements ( with prefix value for form attribute ) might find braces symbols.
  * This code allows to generate appropriate token for them.
  *
  * @param cChar
  * @return closing fences' token
  */
SmToken Identify_Prefix_SmXMLOperatorContext_Impl(sal_Unicode cChar);

/**
  * Identifies closing brace tokens for importing mathml.
  * Identifies from char cChar
  *
  * While loading MO elements ( with postfix value for form attribute ) might find braces symbols.
  * This code allows to generate appropriate token for them.
  *
  * @param cChar
  * @return closing fences' token
  */
SmToken Identify_Postfix_SmXMLOperatorContext_Impl(sal_Unicode cChar);

/**
  * Identifies color from color code cColor.
  * It will be returned with the parser syntax.
  *
  * For a given color returns the way it would be in the parser.
  * Used for nodes to text visitors.
  *
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
  * It will be returned with the MATHML syntax.
  *
  * This is used to export mathml.
  * Identifies the color and allows it to export it in proper mathml code.
  *
  * @param cColor
  * @param parser color
  */
std::unique_ptr<SmColorTokenTableEntry> Identify_Color_MATHML(sal_uInt32 cColor);

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
  *
  * This finds color values for the color names loaded by the parser.
  *
  * @param cColor
  * @param parser color
  */
std::unique_ptr<SmColorTokenTableEntry> Identify_ColorName_Parser(const OUString& colorname);

/**
  * Identifies color from color name.
  * It will be returned with the HTML syntax.
  *
  * This finds color values for the color names loaded by mathmlimport.
  * In theory mathml only supports HTML4 colors, but most browsers support all HTML5 colors.
  * That's why there is an high risk of finding them inside mathml and have to give support.
  *
  * @param cColor
  * @param parser color
  */
std::unique_ptr<SmColorTokenTableEntry> Identify_ColorName_HTML(const OUString& colorname);

/**
  * Identifies color from color name.
  * It will be returned with the dvipsnames syntax.
  *
  * This code has been implemented to add a compatibility layer to import / export latex.
  *
  * @param cColor
  * @param parser color
  */
std::unique_ptr<SmColorTokenTableEntry> Identify_ColorName_DVIPSNAMES(const OUString& colorname);
}
