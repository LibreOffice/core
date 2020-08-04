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
using namespace com::sun::star::i18n;
using namespace com::sun::star::lang;

namespace i18npool {

fullwidthToHalfwidth::fullwidthToHalfwidth()
{
    func = nullptr;
    table = &i18nutil::widthfolding::getfull2halfTable();
    transliterationName = "fullwidthToHalfwidth";
    implementationName = "com.sun.star.i18n.Transliteration.FULLWIDTH_HALFWIDTH";
}

/**
 * Transliterate fullwidth to halfwidth.
 * The output is a reference of OUString. You MUST delete this object when you do not need to use it any more
 * The output string contains a transliterated string only, not whole string.
 */
OUString
fullwidthToHalfwidth::transliterateImpl( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset, bool useOffset )
{
    // Decomposition: GA --> KA + voice-mark
    const OUString& newStr = i18nutil::widthfolding::decompose_ja_voiced_sound_marks (inStr, startPos, nCount, offset, useOffset);

    // One to One mapping
    return transliteration_OneToOne::transliterateImpl( newStr, 0, newStr.getLength(), offset, false);
}

sal_Unicode SAL_CALL
fullwidthToHalfwidth::transliterateChar2Char( sal_Unicode inChar)
{
    sal_Unicode newChar = i18nutil::widthfolding::decompose_ja_voiced_sound_marksChar2Char (inChar);
    if (newChar == 0xFFFF)
        throw MultipleCharsOutputException();
    return transliteration_OneToOne::transliterateChar2Char(inChar);
}

FULLWIDTHKATAKANA_HALFWIDTHKATAKANA::FULLWIDTHKATAKANA_HALFWIDTHKATAKANA()
{
    func = nullptr;
    table = &i18nutil::widthfolding::getfullKana2halfKanaTable();
    transliterationName = "fullwidthKatakanaToHalfwidthKatakana";
    implementationName = "com.sun.star.i18n.Transliteration.FULLWIDTHKATAKANA_HALFWIDTHKATAKANA";
}

/**
 * Transliterate fullwidth katakana to halfwidth katakana.
 */
OUString
FULLWIDTHKATAKANA_HALFWIDTHKATAKANA::transliterateImpl( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset, bool useOffset )
{
    // Decomposition: GA --> KA + voice-mark
    const OUString& newStr = i18nutil::widthfolding::decompose_ja_voiced_sound_marks (inStr, startPos, nCount, offset, useOffset);

    // One to One mapping
    return transliteration_OneToOne::transliterateImpl( newStr, 0, newStr.getLength(), offset, false);
}

sal_Unicode SAL_CALL
FULLWIDTHKATAKANA_HALFWIDTHKATAKANA::transliterateChar2Char( sal_Unicode inChar )
{
    sal_Unicode newChar = i18nutil::widthfolding::decompose_ja_voiced_sound_marksChar2Char (inChar);
    if (newChar == 0xFFFF)
        throw MultipleCharsOutputException();
    return transliteration_OneToOne::transliterateChar2Char(inChar);
}

FULLWIDTH_HALFWIDTH_LIKE_ASC::FULLWIDTH_HALFWIDTH_LIKE_ASC()
{
    func = nullptr;
    table = &i18nutil::widthfolding::getfull2halfTableForASC();
    transliterationName = "fullwidthToHalfwidthLikeASC";
    implementationName = "com.sun.star.i18n.Transliteration.FULLWIDTH_HALFWIDTH_LIKE_ASC";
}

/**
 * Transliterate fullwidth to halfwidth like Excel's ASC function.
 */
OUString
FULLWIDTH_HALFWIDTH_LIKE_ASC::transliterateImpl( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset, bool useOffset )
{
    // Decomposition: GA --> KA + voice-mark
    const OUString& newStr = i18nutil::widthfolding::decompose_ja_voiced_sound_marks (inStr, startPos, nCount, offset, useOffset);

    // One to One mapping
    return transliteration_OneToOne::transliterateImpl( newStr, 0, newStr.getLength(), offset, false);
}

sal_Unicode SAL_CALL
FULLWIDTH_HALFWIDTH_LIKE_ASC::transliterateChar2Char( sal_Unicode inChar )
{
    sal_Unicode newChar = i18nutil::widthfolding::decompose_ja_voiced_sound_marksChar2Char (inChar);
    if (newChar == 0xFFFF)
        throw MultipleCharsOutputException();
    return transliteration_OneToOne::transliterateChar2Char(inChar);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
