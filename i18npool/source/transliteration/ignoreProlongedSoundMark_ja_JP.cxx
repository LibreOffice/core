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
#include <comphelper/string.hxx>
#define TRANSLITERATION_ProlongedSoundMark_ja_JP
#include <transliteration_Ignore.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;


namespace com { namespace sun { namespace star { namespace i18n {

static const sal_Unicode table_normalwidth[] = {
    
    0x3041,       
    0x3042,       
    0x3043,       
    0x3044,       
    0x3045,       
    0x3046,       
    0x3047,       
    0x3048,       
    0x3049,       
    0x304a,       
    0x3042,       
    0x3042,       
    0x3044,       
    0x3044,       
    0x3046,       
    0x3046,       
    0x3048,       
    0x3048,       
    0x304a,       
    0x304a,       
    0x3042,       
    0x3042,       
    0x3044,       
    0x3044,       
    0x3046,       
    0x3046,       
    0x3048,       
    0x3048,       
    0x304a,       
    0x304a,       
    0x3042,       
    0x3042,       
    0x3044,       
    0x3044,       
    0x3045,       
    0x3046,       
    0x3046,       
    0x3048,       
    0x3048,       
    0x304a,       
    0x304a,       
    0x3042,       
    0x3044,       
    0x3046,       
    0x3048,       
    0x304a,       
    0x3042,       
    0x3042,       
    0x3042,       
    0x3044,       
    0x3044,       
    0x3044,       
    0x3046,       
    0x3046,       
    0x3046,       
    0x3048,       
    0x3048,       
    0x3048,       
    0x304a,       
    0x304a,       
    0x304a,       
    0x3042,       
    0x3044,       
    0x3046,       
    0x3048,       
    0x304a,       
    0x3041,       
    0x3042,       
    0x3045,       
    0x3046,       
    0x3049,       
    0x304a,       
    0x3042,       
    0x3044,       
    0x3046,       
    0x3048,       
    0x304a,       
    0x3041,       
    0x3042,       
    0x3044,       
    0x3048,       
    0x304a,       
    0x0000,       
    0x3046,       
    0x0000,       
    0x0000,       
    0x0000,       
    0x0000,       
    0x0000,       
    0x0000,       
    0x0000,       
    0x0000,       
    0x0000,       
    0x0000,       
    0x0000,       
    0x0000,       
    0x30a1,       
    0x30a2,       
    0x30a3,       
    0x30a4,       
    0x30a5,       
    0x30a6,       
    0x30a7,       
    0x30a8,       
    0x30a9,       
    0x30aa,       
    0x30a2,       
    0x30a2,       
    0x30a4,       
    0x30a4,       
    0x30a6,       
    0x30a6,       
    0x30a8,       
    0x30a8,       
    0x30aa,       
    0x30aa,       
    0x30a2,       
    0x30a2,       
    0x30a4,       
    0x30a4,       
    0x30a6,       
    0x30a6,       
    0x30a8,       
    0x30a8,       
    0x30aa,       
    0x30aa,       
    0x30a2,       
    0x30a2,       
    0x30a4,       
    0x30a4,       
    0x30a5,       
    0x30a6,       
    0x30a6,       
    0x30a8,       
    0x30a8,       
    0x30aa,       
    0x30aa,       
    0x30a2,       
    0x30a4,       
    0x30a6,       
    0x30a8,       
    0x30aa,       
    0x30a2,       
    0x30a2,       
    0x30a2,       
    0x30a4,       
    0x30a4,       
    0x30a4,       
    0x30a6,       
    0x30a6,       
    0x30a6,       
    0x30a8,       
    0x30a8,       
    0x30a8,       
    0x30aa,       
    0x30aa,       
    0x30aa,       
    0x30a2,       
    0x30a4,       
    0x30a6,       
    0x30a8,       
    0x30aa,       
    0x30a1,       
    0x30a2,       
    0x30a5,       
    0x30a6,       
    0x30a9,       
    0x30aa,       
    0x30a2,       
    0x30a4,       
    0x30a6,       
    0x30a8,       
    0x30aa,       
    0x30a1,       
    0x30a2,       
    0x30a4,       
    0x30a8,       
    0x30aa,       
    0x0000,       
    0x30a6,       
    0x30a1,       
    0x30a7,       
    0x30a2,       
    0x30a4,       
    0x30a8,       
    0x30aa        
    
    
    
    
    
};

static const sal_Unicode table_halfwidth[] = {
    
    
    
    
    
    0xff75,       
    0xff67,       
    0xff68,       
    0xff69,       
    0xff6a,       
    0xff6b,       
    0xff67,       
    0xff69,       
    0xff6b,       
    0xff69,       
    0x0000,       
    0xff71,       
    0xff72,       
    0xff73,       
    0xff74,       
    0xff75,       
    0xff71,       
    0xff72,       
    0xff73,       
    0xff74,       
    0xff75,       
    0xff71,       
    0xff72,       
    0xff73,       
    0xff74,       
    0xff75,       
    0xff71,       
    0xff72,       
    0xff73,       
    0xff74,       
    0xff75,       
    0xff71,       
    0xff72,       
    0xff73,       
    0xff74,       
    0xff75,       
    0xff71,       
    0xff72,       
    0xff73,       
    0xff74,       
    0xff75,       
    0xff71,       
    0xff72,       
    0xff73,       
    0xff74,       
    0xff75,       
    0xff71,       
    0xff73,       
    0xff75,       
    0xff71,       
    0xff72,       
    0xff73,       
    0xff74,       
    0xff75,       
    0xff71        
    
    
    
};


OUString SAL_CALL
ignoreProlongedSoundMark_ja_JP::folding( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset )
  throw(RuntimeException)
{
    
    
    rtl_uString * newStr = rtl_uString_alloc(nCount);
    sal_Unicode * dst = newStr->buffer;
    const sal_Unicode * src = inStr.getStr() + startPos;

    sal_Int32 *p = 0;
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

        if (currentChar == 0x30fc || 
                currentChar == 0xff70) { 

            if (0x3041 <= previousChar && previousChar <= 0x30fa) {
                currentChar = table_normalwidth[ previousChar - 0x3041 ];
            }
            else if (0xff66 <= previousChar && previousChar <= 0xff9c) {
                currentChar = table_halfwidth[ previousChar - 0xff66 ];
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
    if (useOffset)
        offset.realloc(newStr->length);
    return OUString(newStr, SAL_NO_ACQUIRE); 

}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
