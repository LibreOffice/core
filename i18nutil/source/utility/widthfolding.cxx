/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// prevent internal compiler error with MSVC6SP3
#include <utility>
#include <i18nutil/widthfolding.hxx>
#include <i18nutil/x_rtl_ustring.h>
#include "widthfolding_data.h"

using namespace com::sun::star::uno;

using ::rtl::OUString;

namespace com { namespace sun { namespace star { namespace i18n {

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
OUString widthfolding::decompose_ja_voiced_sound_marks (const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset, sal_Bool useOffset )
{
  // Create a string buffer which can hold nCount * 2 + 1 characters.
  // Its size may become double of nCount.
  rtl_uString * newStr;
  x_rtl_uString_new_WithLength( &newStr, nCount * 2 ); // defined in x_rtl_ustring.h  The reference count is 0 now.

  sal_Int32 *p = NULL;
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
  *dst = (sal_Unicode) 0;

  newStr->length = sal_Int32(dst - newStr->buffer);
  if (useOffset)
      offset.realloc(newStr->length);
  return OUString( newStr ); // defined in rtl/usrting. The reference count is increased from 0 to 1.
}

oneToOneMapping& widthfolding::getfull2halfTable(void)
{
    static oneToOneMappingWithFlag table(full2half, sizeof(full2half), FULL2HALF_NORMAL);
    table.makeIndex();
    return table;
}

/**
 * Compose Japanese specific voiced and semi-voiced sound marks.
 */
OUString widthfolding::compose_ja_voiced_sound_marks (const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset, sal_Bool useOffset, sal_Int32 nFlags )
{
  // Create a string buffer which can hold nCount + 1 characters.
  // Its size may become equal to nCount or smaller.
  // The reference count is 0 now.
  rtl_uString * newStr = x_rtl_uString_new_WithLength( nCount ); // defined in x_rtl_ustring.h

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

  sal_Int32 *p = NULL;
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
      sal_Bool bCompose = sal_False;

      if (0 <= i && i <= (0x30ff - 0x3040) && composition_table[i][j])
        bCompose = sal_True;

      // not to use combined KATAKANA LETTER VU
      if ( previousChar == 0x30a6 && (nFlags & WIDTHFOLDNIG_DONT_USE_COMBINED_VU) )
        bCompose = sal_False;

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

  *dst = (sal_Unicode) 0;

  newStr->length = sal_Int32(dst - newStr->buffer);
 }
  if (useOffset)
      offset.realloc(newStr->length);
  return OUString( newStr ); // defined in rtl/usrting. The reference count is increased from 0 to 1.
}

oneToOneMapping& widthfolding::gethalf2fullTable(void)
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

    // bluedwarf: dirty hack!
    // There is an exception. Additional conversion is required following:
    //  0xFFE5 (FULLWIDTH YEN SIGN)  --> 0x005C (REVERSE SOLIDUS)
    //
    //  See the following page for detail:
    // http://wiki.services.openoffice.org/wiki/Calc/Features/JIS_and_ASC_functions
    int i, j, high, low;
    int n = sizeof(full2halfASCException) / sizeof(UnicodePairWithFlag);
    for( i = 0; i < n; i++ )
    {
        high = (full2halfASCException[i].first >> 8) & 0xFF;
        low  = (full2halfASCException[i].first)      & 0xFF;

        if( !table.mpIndex[high] )
        {
            table.mpIndex[high] = new UnicodePairWithFlag*[256];

            for( j = 0; j < 256; j++ )
                table.mpIndex[high][j] = NULL;
        }
        table.mpIndex[high][low] = &full2halfASCException[i];
    }

    return table;
}

oneToOneMapping& widthfolding::gethalf2fullTableForJIS()
{
    static oneToOneMappingWithFlag table(half2full, sizeof(half2full), HALF2FULL_JIS_FUNCTION);
    table.makeIndex();

    // bluedwarf: dirty hack!
    //  There are some exceptions. Additional conversion are required following:
    //  0x0022 (QUOTATION MARK)  --> 0x201D (RIGHT DOUBLE QUOTATION MARK)
    //  0x0027 (APOSTROPHE)      --> 0x2019 (RIGHT SINGLE QUOTATION MARK)
    //  0x005C (REVERSE SOLIDUS) --> 0xFFE5 (FULLWIDTH YEN SIGN)
    //  0x0060 (GRAVE ACCENT)    --> 0x2018 (LEFT SINGLE QUOTATION MARK)
    //
    //  See the following page for detail:
    // http://wiki.services.openoffice.org/wiki/Calc/Features/JIS_and_ASC_functions
    int i, j, high, low;
    int n = sizeof(half2fullJISException) / sizeof(UnicodePairWithFlag);
    for( i = 0; i < n; i++ )
    {
        high = (half2fullJISException[i].first >> 8) & 0xFF;
        low  = (half2fullJISException[i].first)      & 0xFF;

        if( !table.mpIndex[high] )
        {
            table.mpIndex[high] = new UnicodePairWithFlag*[256];

            for( j = 0; j < 256; j++ )
                table.mpIndex[high][j] = NULL;
        }
        table.mpIndex[high][low] = &half2fullJISException[i];
    }

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

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
