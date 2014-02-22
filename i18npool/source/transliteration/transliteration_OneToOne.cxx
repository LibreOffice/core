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
#include <transliteration_OneToOne.hxx>

using namespace com::sun::star::uno;


namespace com { namespace sun { namespace star { namespace i18n {

sal_Int16 SAL_CALL transliteration_OneToOne::getType() throw(RuntimeException)
{
        
        return TransliterationType::ONE_TO_ONE;
}

OUString SAL_CALL
transliteration_OneToOne::folding( const OUString& /*inStr*/, sal_Int32 /*startPos*/,
        sal_Int32 /*nCount*/, Sequence< sal_Int32 >& /*offset*/) throw(RuntimeException)
{
        throw RuntimeException();
}

sal_Bool SAL_CALL
transliteration_OneToOne::equals( const OUString& /*str1*/, sal_Int32 /*pos1*/, sal_Int32 /*nCount1*/,
        sal_Int32& /*nMatch1*/, const OUString& /*str2*/, sal_Int32 /*pos2*/, sal_Int32 /*nCount2*/, sal_Int32& /*nMatch2*/ )
        throw(RuntimeException)
{
    throw RuntimeException();
}

Sequence< OUString > SAL_CALL
transliteration_OneToOne::transliterateRange( const OUString& /*str1*/, const OUString& /*str2*/ )
        throw(RuntimeException)
{
    throw RuntimeException();
}

OUString SAL_CALL
transliteration_OneToOne::transliterate( const OUString& inStr, sal_Int32 startPos,
    sal_Int32 nCount, Sequence< sal_Int32 >& offset)
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

    
    while (nCount -- > 0) {
    sal_Unicode c = *src++;
    *dst ++ = func ? func( c) : (*table)[ c ];
    if (useOffset)
        *p ++ = position ++;
    }
    *dst = (sal_Unicode) 0;

    return OUString(newStr, SAL_NO_ACQUIRE); 
}

sal_Unicode SAL_CALL
transliteration_OneToOne::transliterateChar2Char( sal_Unicode inChar) throw(RuntimeException, MultipleCharsOutputException)
{
    return func ? func( inChar) : (*table)[ inChar ];
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
