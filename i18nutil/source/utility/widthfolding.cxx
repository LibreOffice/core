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

#include <i18nutil/widthfolding.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include "widthfolding_data.h"

using namespace com::sun::star::uno;


namespace i18nutil {

sal_Unicode widthfolding::decompose_ja_voiced_sound_marksChar2Char (sal_Unicode inChar)
{
    if (0x30a0 <= inChar && inChar <= 0x30ff) {
      sal_Int16 i = inChar - 0x3040;
      if (decomposition_table[i].decomposited_character_1)
          return 0xFFFF;
    }
    return inChar;
}

/**
 * Decompose Japanese specific voiced and semi-voiced sound marks.
 */
OUString widthfolding::decompose_ja_voiced_sound_marks (const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset, bool useOffset )
{
  // Create a string buffer which can hold nCount * 2 + 1 characters.
  // Its size may become double of nCount.
  // The reference count is 1 now.
  rtl_uString * newStr = rtl_uString_alloc(nCount * 2);

  sal_Int32 *p = nullptr;
  sal_Int32 position = 0;
  if (useOffset) {
      // Allocate double of nCount length to offset argument.
      offset.realloc( nCount * 2 );
      p = offset.getArray();
      position = startPos;
  }

  // Prepare pointers of unicode character arrays.
  const sal_Unicode* src = inStr.getStr() + startPos;
  sal_Unicode* dst = newStr->buffer;

  // Decomposition: GA --> KA + voice-mark
  while (nCount -- > 0) {
    sal_Unicode c = *src++;
    // see http://charts.unicode.org/Web/U3040.html Hiragana (U+3040..U+309F)
    // see http://charts.unicode.org/Web/U30A0.html Katakana (U+30A0..U+30FF)
    // Hiragana is not applied to decomposition.
    // Only Katakana is applied to decomposition
    if (0x30a0 <= c && c <= 0x30ff) {
      int i = int(c - 0x3040);
      sal_Unicode first = decomposition_table[i].decomposited_character_1;
      if (first != 0x0000) {
        *dst ++ = first;
        *dst ++ = decomposition_table[i].decomposited_character_2; // second
        if (useOffset) {
            *p ++ = position;
            *p ++ = position ++;
        }
        continue;
      }
    }
    *dst ++ = c;
    if (useOffset)
        *p ++ = position ++;
  }
  *dst = u'\0';

  newStr->length = sal_Int32(dst - newStr->buffer);
  if (useOffset)
      offset.realloc(newStr->length);
  return OUString(newStr, SAL_NO_ACQUIRE); // take ownership
}

oneToOneMapping& widthfolding::getfull2halfTable()
{
    static oneToOneMappingWithFlag table(full2half, sizeof(full2half), FULL2HALF_NORMAL);
    table.makeIndex();
    return table;
}

/**
 * Compose Japanese specific voiced and semi-voiced sound marks.
 */
OUString widthfolding::compose_ja_voiced_sound_marks (const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset, bool useOffset, sal_Int32 nFlags )
{
  // Create a string buffer which can hold nCount + 1 characters.
  // Its size may become equal to nCount or smaller.
  // The reference count is 1 now.
  rtl_uString * newStr = rtl_uString_alloc(nCount);

  // Prepare pointers of unicode character arrays.
  const sal_Unicode* src = inStr.getStr() + startPos;
  sal_Unicode* dst = newStr->buffer;

  // This conversion algorithm requires at least one character.
  if (nCount > 0) {

  // .. .. KA         VOICE .. ..
  //       ^          ^
  //       previousChar   currentChar
  //       ^
  //       position
  //
  // will be converted to
  // .. .. GA       .. ..

  sal_Int32 *p = nullptr;
  sal_Int32 position = 0;
  if (useOffset) {
      // Allocate nCount length to offset argument.
      offset.realloc( nCount );
      p = offset.getArray();
      position = startPos;
  }

  //
  sal_Unicode previousChar = *src ++;
  sal_Unicode currentChar;

  // Composition: KA + voice-mark --> GA
  while (-- nCount > 0) {
    currentChar = *src ++;
    // see http://charts.unicode.org/Web/U3040.html Hiragana (U+3040..U+309F)
    // see http://charts.unicode.org/Web/U30A0.html Katakana (U+30A0..U+30FF)
    // 0x3099 COMBINING KATAKANA-HIRAGANA VOICED SOUND MARK
    // 0x309a COMBINING KATAKANA-HIRAGANA SEMI-VOICED SOUND MARK
    // 0x309b KATAKANA-HIRAGANA VOICED SOUND MARK
    // 0x309c KATAKANA-HIRAGANA SEMI-VOICED SOUND MARK
    int j = currentChar - 0x3099; // 0x3099, 0x309a, 0x309b, 0x309c ?

    if (2 <= j && j <= 3) // 0x309b or 0x309c
        j -= 2;

    if (0 <= j && j <= 1) {
      // 0 addresses a code point regarding 0x3099 or 0x309b (voiced sound mark),
      // 1 is 0x309a or 0x309c (semi-voiced sound mark)
      int i = int(previousChar - 0x3040); // i acts as an index of array
      bool bCompose = false;

      if (0 <= i && i <= (0x30ff - 0x3040) && composition_table[i][j])
        bCompose = true;

      // not to use combined KATAKANA LETTER VU
      if ( previousChar == 0x30a6 && (nFlags & WIDTHFOLDING_DONT_USE_COMBINED_VU) )
        bCompose = false;

      if( bCompose ){
        if (useOffset) {
            position ++;
            *p ++ = position ++;
        }
        *dst ++ =  composition_table[i][j];
        previousChar = *src ++;
        nCount --;
        continue;
      }
    }
    if (useOffset)
        *p ++ = position ++;
    *dst ++ = previousChar;
    previousChar = currentChar;
  }

  if (nCount == 0) {
    if (useOffset)
        *p = position;
    *dst ++ = previousChar;
  }

  *dst = u'\0';

  newStr->length = sal_Int32(dst - newStr->buffer);
 }
  if (useOffset)
      offset.realloc(newStr->length);
  return OUString(newStr, SAL_NO_ACQUIRE); // take ownership
}

oneToOneMapping& widthfolding::gethalf2fullTable()
{
    static oneToOneMappingWithFlag table(half2full, sizeof(half2full), HALF2FULL_NORMAL);
    table.makeIndex();
    return table;
}

sal_Unicode widthfolding::getCompositionChar(sal_Unicode c1, sal_Unicode c2)
{
    return composition_table[c1 - 0x3040][c2 - 0x3099];
}


oneToOneMapping& widthfolding::getfull2halfTableForASC()
{
    static oneToOneMappingWithFlag table(full2half, sizeof(full2half), FULL2HALF_ASC_FUNCTION);
    table.makeIndex();

    return table;
}

oneToOneMapping& widthfolding::gethalf2fullTableForJIS()
{
    static oneToOneMappingWithFlag table(half2full, sizeof(half2full), HALF2FULL_JIS_FUNCTION);
    table.makeIndex();

    return table;
}

oneToOneMapping& widthfolding::getfullKana2halfKanaTable()
{
    static oneToOneMappingWithFlag table(full2half, sizeof(full2half), FULL2HALF_KATAKANA_ONLY);
    table.makeIndex();
    return table;
}

oneToOneMapping& widthfolding::gethalfKana2fullKanaTable()
{
    static oneToOneMappingWithFlag table(half2full, sizeof(half2full), HALF2FULL_KATAKANA_ONLY);
    table.makeIndex();
    return table;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
