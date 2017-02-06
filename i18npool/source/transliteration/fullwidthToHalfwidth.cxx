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

#include <sal/config.h>

#include <com/sun/star/i18n/MultipleCharsOutputException.hpp>
#include <i18nutil/widthfolding.hxx>

#include <transliteration_OneToOne.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

namespace com { namespace sun { namespace star { namespace i18n {

fullwidthToHalfwidth::fullwidthToHalfwidth()
{
    func = nullptr;
    table = &widthfolding::getfull2halfTable();
    transliterationName = "fullwidthToHalfwidth";
    implementationName = "com.sun.star.i18n.Transliteration.FULLWIDTH_HALFWIDTH";
}

/**
 * Transliterate fullwidth to halfwidth.
 * The output is a reference of OUString. You MUST delete this object when you do not need to use it any more
 * The output string contains a transliterated string only, not whole string.
 */
OUString SAL_CALL
fullwidthToHalfwidth::transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset )
{
    // Decomposition: GA --> KA + voice-mark
    const OUString& newStr = widthfolding::decompose_ja_voiced_sound_marks (inStr, startPos, nCount, offset, useOffset);

    // One to One mapping
    useOffset = false;
    const OUString &tmp = transliteration_OneToOne::transliterate( newStr, 0, newStr.getLength(), offset);
    useOffset = true;
    return tmp;
}

sal_Unicode SAL_CALL
fullwidthToHalfwidth::transliterateChar2Char( sal_Unicode inChar)
{
    sal_Unicode newChar = widthfolding::decompose_ja_voiced_sound_marksChar2Char (inChar);
    if (newChar == 0xFFFF)
        throw MultipleCharsOutputException();
    return transliteration_OneToOne::transliterateChar2Char(inChar);
}

fullwidthKatakanaToHalfwidthKatakana::fullwidthKatakanaToHalfwidthKatakana()
{
    func = nullptr;
    table = &widthfolding::getfullKana2halfKanaTable();
    transliterationName = "fullwidthKatakanaToHalfwidthKatakana";
    implementationName = "com.sun.star.i18n.Transliteration.FULLWIDTHKATAKANA_HALFWIDTHKATAKANA";
}

/**
 * Transliterate fullwidth katakana to halfwidth katakana.
 */
OUString SAL_CALL
fullwidthKatakanaToHalfwidthKatakana::transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset )
{
    // Decomposition: GA --> KA + voice-mark
    const OUString& newStr = widthfolding::decompose_ja_voiced_sound_marks (inStr, startPos, nCount, offset, useOffset);

    // One to One mapping
    useOffset = false;
    const OUString &tmp = transliteration_OneToOne::transliterate( newStr, 0, newStr.getLength(), offset);
    useOffset = true;
    return tmp;
}

sal_Unicode SAL_CALL
fullwidthKatakanaToHalfwidthKatakana::transliterateChar2Char( sal_Unicode inChar )
{
    sal_Unicode newChar = widthfolding::decompose_ja_voiced_sound_marksChar2Char (inChar);
    if (newChar == 0xFFFF)
        throw MultipleCharsOutputException();
    return transliteration_OneToOne::transliterateChar2Char(inChar);
}

fullwidthToHalfwidthLikeASC::fullwidthToHalfwidthLikeASC()
{
    func = nullptr;
    table = &widthfolding::getfull2halfTableForASC();
    transliterationName = "fullwidthToHalfwidthLikeASC";
    implementationName = "com.sun.star.i18n.Transliteration.FULLWIDTH_HALFWIDTH_LIKE_ASC";
}

/**
 * Transliterate fullwidth to halfwidth like Excel's ASC function.
 */
OUString SAL_CALL
fullwidthToHalfwidthLikeASC::transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset )
{
    // Decomposition: GA --> KA + voice-mark
    const OUString& newStr = widthfolding::decompose_ja_voiced_sound_marks (inStr, startPos, nCount, offset, useOffset);

    // One to One mapping
    useOffset = false;
    const OUString &tmp = transliteration_OneToOne::transliterate( newStr, 0, newStr.getLength(), offset);
    useOffset = true;

    return tmp;
}

sal_Unicode SAL_CALL
fullwidthToHalfwidthLikeASC::transliterateChar2Char( sal_Unicode inChar )
{
    sal_Unicode newChar = widthfolding::decompose_ja_voiced_sound_marksChar2Char (inChar);
    if (newChar == 0xFFFF)
        throw MultipleCharsOutputException();
    return transliteration_OneToOne::transliterateChar2Char(inChar);
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
