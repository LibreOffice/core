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

#include "basiccharclass.hxx"

#include <unotools/charclass.hxx>
#include <vcl/svapp.hxx>

BasicCharClass::BasicCharClass()
{
  for( sal_uInt16 i = 0; i < 256; ++i )
    IsLetterTab[i] = false;

  IsLetterTab[0xC0] = true;   // À , CAPITAL LETTER A WITH GRAVE ACCENT
  IsLetterTab[0xC1] = true;   // Á , CAPITAL LETTER A WITH ACUTE ACCENT
  IsLetterTab[0xC2] = true;   // Â , CAPITAL LETTER A WITH CIRCUMFLEX ACCENT
  IsLetterTab[0xC3] = true;   // Ã , CAPITAL LETTER A WITH TILDE
  IsLetterTab[0xC4] = true;   // Ä , CAPITAL LETTER A WITH DIAERESIS
  IsLetterTab[0xC5] = true;   // Å , CAPITAL LETTER A WITH RING ABOVE
  IsLetterTab[0xC6] = true;   // Æ , CAPITAL LIGATURE AE
  IsLetterTab[0xC7] = true;   // Ç , CAPITAL LETTER C WITH CEDILLA
  IsLetterTab[0xC8] = true;   // È , CAPITAL LETTER E WITH GRAVE ACCENT
  IsLetterTab[0xC9] = true;   // É , CAPITAL LETTER E WITH ACUTE ACCENT
  IsLetterTab[0xCA] = true;   // Ê , CAPITAL LETTER E WITH CIRCUMFLEX ACCENT
  IsLetterTab[0xCB] = true;   // Ë , CAPITAL LETTER E WITH DIAERESIS
  IsLetterTab[0xCC] = true;   // Ì , CAPITAL LETTER I WITH GRAVE ACCENT
  IsLetterTab[0xCD] = true;   // Í , CAPITAL LETTER I WITH ACUTE ACCENT
  IsLetterTab[0xCE] = true;   // Î , CAPITAL LETTER I WITH CIRCUMFLEX ACCENT
  IsLetterTab[0xCF] = true;   // Ï , CAPITAL LETTER I WITH DIAERESIS
  IsLetterTab[0xD0] = true;   // Ð , CAPITAL LETTER ETH
  IsLetterTab[0xD1] = true;   // Ñ , CAPITAL LETTER N WITH TILDE
  IsLetterTab[0xD2] = true;   // Ò , CAPITAL LETTER O WITH GRAVE ACCENT
  IsLetterTab[0xD3] = true;   // Ó , CAPITAL LETTER O WITH ACUTE ACCENT
  IsLetterTab[0xD4] = true;   // Ô , CAPITAL LETTER O WITH CIRCUMFLEX ACCENT
  IsLetterTab[0xD5] = true;   // Õ , CAPITAL LETTER O WITH TILDE
  IsLetterTab[0xD6] = true;   // Ö , CAPITAL LETTER O WITH DIAERESIS
  IsLetterTab[0xD8] = true;   // Ø , CAPITAL LETTER O WITH STROKE
  IsLetterTab[0xD9] = true;   // Ù , CAPITAL LETTER U WITH GRAVE ACCENT
  IsLetterTab[0xDA] = true;   // Ú , CAPITAL LETTER U WITH ACUTE ACCENT
  IsLetterTab[0xDB] = true;   // Û , CAPITAL LETTER U WITH CIRCUMFLEX ACCENT
  IsLetterTab[0xDC] = true;   // Ü , CAPITAL LETTER U WITH DIAERESIS
  IsLetterTab[0xDD] = true;   // Ý , CAPITAL LETTER Y WITH ACUTE ACCENT
  IsLetterTab[0xDE] = true;   // Þ , CAPITAL LETTER THORN
  IsLetterTab[0xDF] = true;   // ß , SMALL LETTER SHARP S
  IsLetterTab[0xE0] = true;   // à , SMALL LETTER A WITH GRAVE ACCENT
  IsLetterTab[0xE1] = true;   // á , SMALL LETTER A WITH ACUTE ACCENT
  IsLetterTab[0xE2] = true;   // â , SMALL LETTER A WITH CIRCUMFLEX ACCENT
  IsLetterTab[0xE3] = true;   // ã , SMALL LETTER A WITH TILDE
  IsLetterTab[0xE4] = true;   // ä , SMALL LETTER A WITH DIAERESIS
  IsLetterTab[0xE5] = true;   // å , SMALL LETTER A WITH RING ABOVE
  IsLetterTab[0xE6] = true;   // æ , SMALL LIGATURE AE
  IsLetterTab[0xE7] = true;   // ç , SMALL LETTER C WITH CEDILLA
  IsLetterTab[0xE8] = true;   // è , SMALL LETTER E WITH GRAVE ACCENT
  IsLetterTab[0xE9] = true;   // é , SMALL LETTER E WITH ACUTE ACCENT
  IsLetterTab[0xEA] = true;   // ê , SMALL LETTER E WITH CIRCUMFLEX ACCENT
  IsLetterTab[0xEB] = true;   // ë , SMALL LETTER E WITH DIAERESIS
  IsLetterTab[0xEC] = true;   // ì , SMALL LETTER I WITH GRAVE ACCENT
  IsLetterTab[0xED] = true;   // í , SMALL LETTER I WITH ACUTE ACCENT
  IsLetterTab[0xEE] = true;   // î , SMALL LETTER I WITH CIRCUMFLEX ACCENT
  IsLetterTab[0xEF] = true;   // ï , SMALL LETTER I WITH DIAERESIS
  IsLetterTab[0xF0] = true;   // ð , SMALL LETTER ETH
  IsLetterTab[0xF1] = true;   // ñ , SMALL LETTER N WITH TILDE
  IsLetterTab[0xF2] = true;   // ò , SMALL LETTER O WITH GRAVE ACCENT
  IsLetterTab[0xF3] = true;   // ó , SMALL LETTER O WITH ACUTE ACCENT
  IsLetterTab[0xF4] = true;   // ô , SMALL LETTER O WITH CIRCUMFLEX ACCENT
  IsLetterTab[0xF5] = true;   // õ , SMALL LETTER O WITH TILDE
  IsLetterTab[0xF6] = true;   // ö , SMALL LETTER O WITH DIAERESIS
  IsLetterTab[0xF8] = true;   // ø , SMALL LETTER O WITH OBLIQUE BAR
  IsLetterTab[0xF9] = true;   // ù , SMALL LETTER U WITH GRAVE ACCENT
  IsLetterTab[0xFA] = true;   // ú , SMALL LETTER U WITH ACUTE ACCENT
  IsLetterTab[0xFB] = true;   // û , SMALL LETTER U WITH CIRCUMFLEX ACCENT
  IsLetterTab[0xFC] = true;   // ü , SMALL LETTER U WITH DIAERESIS
  IsLetterTab[0xFD] = true;   // ý , SMALL LETTER Y WITH ACUTE ACCENT
  IsLetterTab[0xFE] = true;   // þ , SMALL LETTER THORN
  IsLetterTab[0xFF] = true;   // ÿ , SMALL LETTER Y WITH DIAERESIS
}

bool BasicCharClass::isLetter( sal_Unicode c )
{
  return (c < 256 ? IsLetterTab[c] : isLetterUnicode( c ));
}

bool BasicCharClass::isLetterUnicode( sal_Unicode c )
{
  static CharClass* pCharClass = NULL;
  if( pCharClass == NULL )
    pCharClass = new CharClass( Application::GetSettings().GetLanguageTag() );
  // can we get pCharClass to accept a sal_Unicode instead of this waste?
  return pCharClass->isLetter( OUString(c), 0 );
}

bool BasicCharClass::isAlpha( sal_Unicode c, bool bCompatible )
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
    || (bCompatible && isLetter( c ));
}

bool BasicCharClass::isDigit( sal_Unicode c )
{
  return (c >= '0' && c <= '9');
}

bool BasicCharClass::isAlphaNumeric( sal_Unicode c, bool bCompatible )
{
  return isDigit( c ) || isAlpha( c, bCompatible );
}

bool BasicCharClass::isWhitespace( sal_Unicode c )
{
  return (c == ' ') || (c == '\t') || (c == '\f');
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
