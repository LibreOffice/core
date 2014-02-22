/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <utility>
#include <i18nutil/widthfolding.hxx>
#include <comphelper/string.hxx>
#include "widthfolding_data.h"

using namespace com::sun::star::uno;


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
OUString widthfolding::decompose_ja_voiced_sound_marks (const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset, bool useOffset )
{
  
  
  
  rtl_uString * newStr = rtl_uString_alloc(nCount * 2);

  sal_Int32 *p = NULL;
  sal_Int32 position = 0;
  if (useOffset) {
      
      offset.realloc( nCount * 2 );
      p = offset.getArray();
      position = startPos;
  }

  
  const sal_Unicode* src = inStr.getStr() + startPos;
  sal_Unicode* dst = newStr->buffer;

  
  while (nCount -- > 0) {
    sal_Unicode c = *src++;
    
    
    
    
    if (0x30a0 <= c && c <= 0x30ff) {
      int i = int(c - 0x3040);
      sal_Unicode first = decomposition_table[i].decomposited_character_1;
      if (first != 0x0000) {
    *dst ++ = first;
    *dst ++ = decomposition_table[i].decomposited_character_2; 
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
  return OUString(newStr, SAL_NO_ACQUIRE); 
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
OUString widthfolding::compose_ja_voiced_sound_marks (const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset, bool useOffset, sal_Int32 nFlags )
{
  
  
  
  rtl_uString * newStr = rtl_uString_alloc(nCount);

  
  const sal_Unicode* src = inStr.getStr() + startPos;
  sal_Unicode* dst = newStr->buffer;

  
 if (nCount > 0) {

  
  
  
  
  
  //
  
  

  sal_Int32 *p = NULL;
  sal_Int32 position = 0;
  if (useOffset) {
      
      offset.realloc( nCount );
      p = offset.getArray();
      position = startPos;
  }

  //
  sal_Unicode previousChar = *src ++;
  sal_Unicode currentChar;

  
  while (-- nCount > 0) {
    currentChar = *src ++;
    
    
    
    
    
    
    int j = currentChar - 0x3099; 

    if (2 <= j && j <= 3) 
        j -= 2;

    if (0 <= j && j <= 1) {
      
      
      int i = int(previousChar - 0x3040); 
      bool bCompose = false;

      if (0 <= i && i <= (0x30ff - 0x3040) && composition_table[i][j])
        bCompose = true;

      
      if ( previousChar == 0x30a6 && (nFlags & WIDTHFOLDNIG_DONT_USE_COMBINED_VU) )
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

  *dst = (sal_Unicode) 0;

  newStr->length = sal_Int32(dst - newStr->buffer);
 }
  if (useOffset)
      offset.realloc(newStr->length);
  return OUString(newStr, SAL_NO_ACQUIRE); 
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

    
    
    
    //
    
    
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

    
    
    
    
    
    
    //
    
    
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
