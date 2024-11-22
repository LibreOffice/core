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

#include <token.hxx>

// Starmath color types
// In order to add them to starmath, edit the SmColorTokenTableEntry lists on
// /core/starmath/source/starmathdatabase.css .

// HTML
// https://developer.mozilla.org/en-US/docs/Web/CSS/color_value#Color_Keywords
/* CSS Level 1 */
inline constexpr Color COL_SM_BLACK(0x000000);
inline constexpr Color COL_SM_SILVER(0xC0C0C0);
inline constexpr Color COL_SM_GRAY(0x808080);
inline constexpr Color COL_SM_WHITE(0xFFFFFF);
inline constexpr Color COL_SM_MAROON(0x800000);
inline constexpr Color COL_SM_RED(0xFF0000);
inline constexpr Color COL_SM_PURPLE(0x800080);
inline constexpr Color COL_SM_FUCHSIA(0xFF00FF);
inline constexpr Color COL_SM_GREEN(0x008000);
inline constexpr Color COL_SM_LIME(0x00FF00);
inline constexpr Color COL_SM_OLIVE(0x808000);
inline constexpr Color COL_SM_YELLOW(0xFFFF00);
inline constexpr Color COL_SM_NAVY(0x000080);
inline constexpr Color COL_SM_BLUE(0x0000FF);
inline constexpr Color COL_SM_TEAL(0x008080);
inline constexpr Color COL_SM_AQUA(0x00FFFF);
/* CSS Level 2 */
inline constexpr Color COL_SM_ORANGE(0xFFA500);
/* CSS Level 3 */
inline constexpr Color COL_SM_ALICEBLUE(0xF0F8FF);
inline constexpr Color COL_SM_ANTIQUEWHITE(0xFAEBD7);
inline constexpr Color COL_SM_AQUAMARINE(0x7FFFD4);
inline constexpr Color COL_SM_AZURE(0xF0FFFF);
inline constexpr Color COL_SM_BEIGE(0xF5F5DC);
inline constexpr Color COL_SM_BISQUE(0xFFE4C4);
inline constexpr Color COL_SM_BLANCHEDALMOND(0xFFEBCD);
inline constexpr Color COL_SM_BLUEVIOLET(0x8A2BE2);
inline constexpr Color COL_SM_BROWN(0xA52A2A);
inline constexpr Color COL_SM_BURLYWOOD(0xDEB887);
inline constexpr Color COL_SM_CADETBLUE(0x5F9EA0);
inline constexpr Color COL_SM_CHARTREUSE(0x7FFF00);
inline constexpr Color COL_SM_CHOCOLATE(0xD2691E);
inline constexpr Color COL_SM_CORAL(0xFF7F50);
inline constexpr Color COL_SM_CORNFLOWERBLUE(0x6495ED);
inline constexpr Color COL_SM_CORNSILK(0xFFF8DC);
inline constexpr Color COL_SM_CRIMSON(0xDC143C);
inline constexpr Color COL_SM_CYAN(0x00FFFF);
inline constexpr Color COL_SM_DARKBLUE(0x00008B);
inline constexpr Color COL_SM_DARKCYAN(0x008B8B);
inline constexpr Color COL_SM_DARKGOLDENROD(0xB8860B);
inline constexpr Color COL_SM_DARKGRAY(0xA9A9A9);
inline constexpr Color COL_SM_DARKGREEN(0x006400);
inline constexpr Color COL_SM_DARKGREY(0xA9A9A9);
inline constexpr Color COL_SM_DARKKHAKI(0xBDB76B);
inline constexpr Color COL_SM_DARKMAGENTA(0x8B008B);
inline constexpr Color COL_SM_DARKOLIVEGREEN(0x556B2F);
inline constexpr Color COL_SM_DARKORANGE(0xFF8C00);
inline constexpr Color COL_SM_DARKORCHID(0x9932CC);
inline constexpr Color COL_SM_DARKRED(0x8B0000);
inline constexpr Color COL_SM_DARKSALMON(0xE9967A);
inline constexpr Color COL_SM_DARKSEAGREEN(0x8FBC8F);
inline constexpr Color COL_SM_DARKSLATEBLUE(0x483D8B);
inline constexpr Color COL_SM_DARKSLATEGRAY(0x2F4F4F);
inline constexpr Color COL_SM_DARKSLATEGREY(0x2F4F4F);
inline constexpr Color COL_SM_DARKTURQUOISE(0x00CED1);
inline constexpr Color COL_SM_DARKVIOLET(0x9400D3);
inline constexpr Color COL_SM_DEEPPINK(0xFF1493);
inline constexpr Color COL_SM_DEEPSKYBLUE(0x00BFFF);
inline constexpr Color COL_SM_DIMGRAY(0x696969);
inline constexpr Color COL_SM_DIMGREY(0x696969);
inline constexpr Color COL_SM_DODGERBLUE(0x1E90FF);
inline constexpr Color COL_SM_FIREBRICK(0xB22222);
inline constexpr Color COL_SM_FLORALWHITE(0xFFFAF0);
inline constexpr Color COL_SM_FORESTGREEN(0x228B22);
inline constexpr Color COL_SM_GAINSBORO(0xDCDCDC);
inline constexpr Color COL_SM_GHOSTWHITE(0xF8F8FF);
inline constexpr Color COL_SM_GOLD(0xFFD700);
inline constexpr Color COL_SM_GOLDENROD(0xDAA520);
inline constexpr Color COL_SM_GREENYELLOW(0xADFF2F);
inline constexpr Color COL_SM_GREY(0x808080);
inline constexpr Color COL_SM_HONEYDEW(0xF0FFF0);
inline constexpr Color COL_SM_HOTPINK(0xFF69B4);
inline constexpr Color COL_SM_INDIANRED(0xCD5C5C);
inline constexpr Color COL_SM_INDIGO(0x4B0082);
inline constexpr Color COL_SM_IVORY(0xFFFFF0);
inline constexpr Color COL_SM_KHAKI(0xF0E68C);
inline constexpr Color COL_SM_LAVENDER(0xE6E6FA);
inline constexpr Color COL_SM_LAVENDERBLUSH(0xFFF0F5);
inline constexpr Color COL_SM_LAWNGREEN(0x7CFC00);
inline constexpr Color COL_SM_LEMONCHIFFON(0xFFFACD);
inline constexpr Color COL_SM_LIGHTBLUE(0xADD8E6);
inline constexpr Color COL_SM_LIGHTCORAL(0xF08080);
inline constexpr Color COL_SM_LIGHTCYAN(0xE0FFFF);
inline constexpr Color COL_SM_LIGHTGOLDENRODYELLOW(0xFAFAD2);
inline constexpr Color COL_SM_LIGHTGRAY(0xD3D3D3);
inline constexpr Color COL_SM_LIGHTGREEN(0x90EE90);
inline constexpr Color COL_SM_LIGHTGREY(0xD3D3D3);
inline constexpr Color COL_SM_LIGHTPINK(0xFFB6C1);
inline constexpr Color COL_SM_LIGHTSALMON(0xFFA07A);
inline constexpr Color COL_SM_LIGHTSEAGREEN(0x20B2AA);
inline constexpr Color COL_SM_LIGHTSKYBLUE(0x87CEFA);
inline constexpr Color COL_SM_LIGHTSLATEGRAY(0x778899);
inline constexpr Color COL_SM_LIGHTSLATEGREY(0x778899);
inline constexpr Color COL_SM_LIGHTSTEELBLUE(0xB0C4DE);
inline constexpr Color COL_SM_LIGHTYELLOW(0xFFFFE0);
inline constexpr Color COL_SM_LIMEGREEN(0x32CD32);
inline constexpr Color COL_SM_LINEN(0xFAF0E6);
inline constexpr Color COL_SM_MAGENTA(0xFF00FF);
inline constexpr Color COL_SM_MEDIUMAQUAMARINE(0x66CDAA);
inline constexpr Color COL_SM_MEDIUMBLUE(0x0000CD);
inline constexpr Color COL_SM_MEDIUMORCHID(0xBA55D3);
inline constexpr Color COL_SM_MEDIUMPURPLE(0x9370DB);
inline constexpr Color COL_SM_MEDIUMSEAGREEN(0x3CB371);
inline constexpr Color COL_SM_MEDIUMSLATEBLUE(0x7B68EE);
inline constexpr Color COL_SM_MEDIUMSPRINGGREEN(0x00FA9A);
inline constexpr Color COL_SM_MEDIUMTURQUOISE(0x48D1CC);
inline constexpr Color COL_SM_MEDIUMVIOLETRED(0xC71585);
inline constexpr Color COL_SM_MIDNIGHTBLUE(0x191970);
inline constexpr Color COL_SM_MINTCREAM(0xF5FFFA);
inline constexpr Color COL_SM_MISTYROSE(0xFFE4E1);
inline constexpr Color COL_SM_MOCCASIN(0xFFE4B5);
inline constexpr Color COL_SM_NAVAJOWHITE(0xFFDEAD);
inline constexpr Color COL_SM_OLDLACE(0xFDF5E6);
inline constexpr Color COL_SM_OLIVEDRAB(0x6B8E23);
inline constexpr Color COL_SM_ORANGERED(0xFF4500);
inline constexpr Color COL_SM_ORCHID(0xDA70D6);
inline constexpr Color COL_SM_PALEGOLDENROD(0xEEE8AA);
inline constexpr Color COL_SM_PALEGREEN(0x98FB98);
inline constexpr Color COL_SM_PALETURQUOISE(0xAFEEEE);
inline constexpr Color COL_SM_PALEVIOLETRED(0xDB7093);
inline constexpr Color COL_SM_PAPAYAWHIP(0xFFEFD5);
inline constexpr Color COL_SM_PEACHPUFF(0xFFDAB9);
inline constexpr Color COL_SM_PERU(0xCD853F);
inline constexpr Color COL_SM_PINK(0xFFC0CB);
inline constexpr Color COL_SM_PLUM(0xDDA0DD);
inline constexpr Color COL_SM_POWDERBLUE(0xB0E0E6);
inline constexpr Color COL_SM_ROSYBROWN(0xBC8F8F);
inline constexpr Color COL_SM_ROYALBLUE(0x4169E1);
inline constexpr Color COL_SM_SADDLEBROWN(0x8B4513);
inline constexpr Color COL_SM_SALMON(0xFA8072);
inline constexpr Color COL_SM_SANDYBROWN(0xF4A460);
inline constexpr Color COL_SM_SEAGREEN(0x2E8B57);
inline constexpr Color COL_SM_SEASHELL(0xFFF5EE);
inline constexpr Color COL_SM_SIENNA(0xA0522D);
inline constexpr Color COL_SM_SKYBLUE(0x87CEEB);
inline constexpr Color COL_SM_SLATEBLUE(0x6A5ACD);
inline constexpr Color COL_SM_SLATEGRAY(0x708090);
inline constexpr Color COL_SM_SLATEGREY(0x708090);
inline constexpr Color COL_SM_SNOW(0xFFFAFA);
inline constexpr Color COL_SM_SPRINGGREEN(0x00FF7F);
inline constexpr Color COL_SM_STEELBLUE(0x4682B4);
inline constexpr Color COL_SM_TAN(0xD2B48C);
inline constexpr Color COL_SM_THISTLE(0xD8BFD8);
inline constexpr Color COL_SM_TOMATO(0xFF6347);
inline constexpr Color COL_SM_TURQUOISE(0x40E0D0);
inline constexpr Color COL_SM_VIOLET(0xEE82EE);
inline constexpr Color COL_SM_WHEAT(0xF5DEB3);
inline constexpr Color COL_SM_WHITESMOKE(0xF5F5F5);
inline constexpr Color COL_SM_YELLOWGREEN(0x9ACD32);
/* CSS Level 4 */
inline constexpr Color COL_SM_REBECCAPURPLE(0x663399);
/* dvipsnames */
// For now only five colors.
// In a future all of them.
// https://www.overleaf.com/learn/latex/Using_colours_in_LaTeX
inline constexpr Color COL_SM_DIV_APRICOT(0xFFB781);
inline constexpr Color COL_SM_DIV_AQUAMARINE(0x1BBEC1);
inline constexpr Color COL_SM_DIV_BITTERSWEET(0xCF4B16);
inline constexpr Color COL_SM_DIV_BLACK(0xCF4B16);
inline constexpr Color COL_SM_DIV_BLUE(0x102694);
/* Iconic colors */
// https://design.ubuntu.com/brand/colour-palette/
inline constexpr Color COL_SM_UBUNTU_ORANGE(0xE95420);
// https://www.debian.org/logos/   Picked from SVG logo
inline constexpr Color COL_SM_DEBIAN_MAGENTA(0xA80030);
// https://libreoffice.org/
inline constexpr Color COL_SM_LO_GREEN(0x00A500);

namespace starmathdatabase
{
// Variables containing color information.
extern const SmColorTokenTableEntry aColorTokenTableParse[159];
extern const SmColorTokenTableEntry aColorTokenTableHTML[148];
extern const SmColorTokenTableEntry aColorTokenTableMATHML[16];
extern const SmColorTokenTableEntry aColorTokenTableDVIPS[5];
extern const SmColorTokenTableEntry aColorTokenTableERROR[1];

/**
  * Identifies operator chars tokens for importing mathml.
  * Identifies from rText
  *
  * While loading MO or MI elements might find an unicode16 symbol.
  * This code allows to generate appropriate token for them.
  *
  * @param rText
  * @return closing fences' token
  */
SmToken Identify_SmXMLOperatorContext_Impl(std::u16string_view rText, bool bIsStretchy = true,
                                           sal_Int32 nIndex = 0);

/**
  * Identifies opening / closing brace tokens for importing mathml.
  * Identifies from rText
  *
  * While loading MO fenced elements might find braces symbols.
  * This code allows to generate appropriate token for them.
  *
  * @param rText
  * @return closing fences' token
  */
SmToken Identify_PrefixPostfix_SmXMLOperatorContext_Impl(std::u16string_view rText,
                                                         sal_Int32 nIndex = 0);

/**
  * Identifies opening brace tokens for importing mathml.
  * Identifies from rText
  *
  * While loading MO elements ( with prefix value for form attribute ) might find braces symbols.
  * This code allows to generate appropriate token for them.
  *
  * @param rText
  * @return closing fences' token
  */
SmToken Identify_Prefix_SmXMLOperatorContext_Impl(std::u16string_view rText, sal_Int32 nIndex = 0);

/**
  * Identifies closing brace tokens for importing mathml.
  * Identifies from rText
  *
  * While loading MO elements ( with postfix value for form attribute ) might find braces symbols.
  * This code allows to generate appropriate token for them.
  *
  * @param rText
  * @return closing fences' token
  */
SmToken Identify_Postfix_SmXMLOperatorContext_Impl(std::u16string_view rText, sal_Int32 nIndex = 0);

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
SmColorTokenTableEntry Identify_Color_Parser(sal_uInt32 cColor);

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
SmColorTokenTableEntry Identify_Color_MATHML(sal_uInt32 cColor);

/**
  * Identifies color from color code cColor.
  * It will be returned with the dvipsnames syntax.
  * @param cColor
  * @param parser color
  */
SmColorTokenTableEntry Identify_Color_DVIPSNAMES(sal_uInt32 cColor);

/**
  * Identifies color from color name.
  * It will be returned with the parser syntax.
  *
  * This finds color values for the color names loaded by the parser.
  *
  * @param cColor
  * @param parser color
  */
const SmColorTokenTableEntry* Identify_ColorName_Parser(std::u16string_view colorname);

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
SmColorTokenTableEntry Identify_ColorName_HTML(std::u16string_view colorname);

/**
  * Identifies color from color name.
  * It will be returned with the dvipsnames syntax.
  *
  * This code has been implemented to add a compatibility layer to import / export latex.
  *
  * @param cColor
  * @param parser color
  */
const SmColorTokenTableEntry* Identify_ColorName_DVIPSNAMES(std::u16string_view colorname);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
