/*************************************************************************
 *
 *  $RCSfile: widthfolding.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 12:26:04 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// prevent internal compiler error with MSVC6SP3
#include <stl/utility>
#include <i18nutil/widthfolding.hxx>
#include <i18nutil/x_rtl_ustring.h>
#include "widthfolding_data.h"

using namespace com::sun::star::uno;
using namespace rtl;

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

  sal_Int32 *p, position;
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
    static oneToOneMapping table(full2half, sizeof(full2half));
    table.makeIndex();
    return table;
}

/**
 * Compose Japanese specific voiced and semi-voiced sound marks.
 */
OUString widthfolding::compose_ja_voiced_sound_marks (const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset, sal_Bool useOffset )
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

  sal_Int32 *p, position;
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
    int j = currentChar - 0x3099; // 0x3099 or 0x309a ?
    if (0 <= j && j <= 1) {  // 0 addresses a code point regarding 0x3099, 1 is 0x309a
      int i = int(previousChar - 0x3040); // i acts as an index of array
      if (0 <= i && i <= (0x30ff - 0x3040) && composition_table[i][j]) {
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
    static oneToOneMapping table(half2full, sizeof(half2full));
    table.makeIndex();
    return table;
}

sal_Unicode widthfolding::getCompositionChar(sal_Unicode c1, sal_Unicode c2)
{
    return composition_table[c1 - 0x3040][c2 - 0x3099];
}

} } } }
