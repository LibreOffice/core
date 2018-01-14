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
#ifndef INCLUDED_I18NUTIL_TRANSLITERATION_HXX
#define INCLUDED_I18NUTIL_TRANSLITERATION_HXX

#include <sal/types.h>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <com/sun/star/i18n/TransliterationModulesExtra.hpp>
#include <o3tl/typed_flags_set.hxx>

/**
 * This is a superset type of the com::sun::star::i18n::TransliterationModules and TransliterationModulesExtra,
 *  with some extra type checking
 */
enum class TransliterationFlags {
    NONE                           = 0,
    /// Transliterate a string from upper case to lower case
    UPPERCASE_LOWERCASE            = int(css::i18n::TransliterationModules_UPPERCASE_LOWERCASE),
    /// Transliterate a string from lower case to upper case
    LOWERCASE_UPPERCASE            = int(css::i18n::TransliterationModules_LOWERCASE_UPPERCASE),
    /// Transliterate a string from half width character to full width character
    HALFWIDTH_FULLWIDTH            = int(css::i18n::TransliterationModules_HALFWIDTH_FULLWIDTH),
    /// Transliterate a string from full width character to half width character
    FULLWIDTH_HALFWIDTH            = int(css::i18n::TransliterationModules_FULLWIDTH_HALFWIDTH),
    /// Transliterate a Japanese string from Katakana to Hiragana
    KATAKANA_HIRAGANA              = int(css::i18n::TransliterationModules_KATAKANA_HIRAGANA),
    /// Transliterate a Japanese string from Hiragana to Katakana
    HIRAGANA_KATAKANA              = int(css::i18n::TransliterationModules_HIRAGANA_KATAKANA),
    /// Transliterate an ASCII number string to Simplified Chinese lower case number string in spellout format
    NumToTextLower_zh_CN           = int(css::i18n::TransliterationModules_NumToTextLower_zh_CN),
    /// Transliterate an ASCII number string to Simplified Chinese upper case number string in spellout format
    NumToTextUpper_zh_CN           = int(css::i18n::TransliterationModules_NumToTextUpper_zh_CN),
    /// Transliterate an ASCII number string to Traditional Chinese lower case number string in spellout format
    NumToTextLower_zh_TW           = int(css::i18n::TransliterationModules_NumToTextLower_zh_TW),
    /// Transliterate an ASCII number string to Traditional Chinese upper case number string in spellout format
    NumToTextUpper_zh_TW           = int(css::i18n::TransliterationModules_NumToTextUpper_zh_TW),
    /// Transliterate an ASCII number string to formal Korean Hangul number string in spellout format
    NumToTextFormalHangul_ko       = int(css::i18n::TransliterationModules_NumToTextFormalHangul_ko),
    /// Transliterate an ASCII number string to formal Korean Hanja lower case number string in spellout format
    NumToTextFormalLower_ko        = int(css::i18n::TransliterationModules_NumToTextFormalLower_ko),
    /// Transliterate an ASCII number string to formal Korean Hanja upper case number string in spellout format
    NumToTextFormalUpper_ko        = int(css::i18n::TransliterationModules_NumToTextFormalUpper_ko),

    /** The first character of the sentence is put in upper case
     */
    SENTENCE_CASE                  = int(css::i18n::TransliterationModulesExtra::SENTENCE_CASE),


    /** The first character of the word is put in upper case.
     * This one is part
     */
    TITLE_CASE                     = int(css::i18n::TransliterationModulesExtra::TITLE_CASE),


    /** All characters of the word are to change their case from small letters
     * to capital letters and vice versa.
     */
    TOGGLE_CASE                    = int(css::i18n::TransliterationModulesExtra::TOGGLE_CASE),

    NON_IGNORE_MASK                = int(css::i18n::TransliterationModules_NON_IGNORE_MASK),
    IGNORE_MASK                    = 0x7fffff00,

    /// Ignore case when comparing strings by transliteration service
    IGNORE_CASE                    = int(css::i18n::TransliterationModules_IGNORE_CASE),
    /// Ignore Hiragana and Katakana when comparing strings by transliteration service
    IGNORE_KANA                    = int(css::i18n::TransliterationModules_IGNORE_KANA), // ja_JP
    /// Ignore full width and half width character when comparing strings by transliteration service
    IGNORE_WIDTH                   = int(css::i18n::TransliterationModules_IGNORE_WIDTH), // ja_JP
    /// Ignore Japanese traditional Kanji character in Japanese fuzzy search
    ignoreTraditionalKanji_ja_JP   = int(css::i18n::TransliterationModules_ignoreTraditionalKanji_ja_JP),
    /// Ignore Japanese traditional Katakana and Hiragana character in Japanese fuzzy search
    ignoreTraditionalKana_ja_JP    = int(css::i18n::TransliterationModules_ignoreTraditionalKana_ja_JP),
    /// Ignore dash or minus sign in Japanese fuzzy search
    ignoreMinusSign_ja_JP          = int(css::i18n::TransliterationModules_ignoreMinusSign_ja_JP),
    /// Ignore Hiragana and Katakana iteration mark in Japanese fuzzy search
    ignoreIterationMark_ja_JP      = int(css::i18n::TransliterationModules_ignoreIterationMark_ja_JP),
    /// Ignore separator punctuations in Japanese fuzzy search
    ignoreSeparator_ja_JP          = int(css::i18n::TransliterationModules_ignoreSeparator_ja_JP),
    /// Ignore Katakana and Hiragana Zi/Zi and Zu/Zu  in Japanese fuzzy search
    ignoreZiZu_ja_JP               = int(css::i18n::TransliterationModules_ignoreZiZu_ja_JP),
    /// Ignore Katakana and Hiragana Ba/Gua and Ha/Fa in Japanese fuzzy search
    ignoreBaFa_ja_JP               = int(css::i18n::TransliterationModules_ignoreBaFa_ja_JP),
    /// Ignore Katakana and Hiragana Tsui/Tea/Ti and Dyi/Ji in Japanese fuzzy search
    ignoreTiJi_ja_JP               = int(css::i18n::TransliterationModules_ignoreTiJi_ja_JP),
    /// Ignore Katakana and Hiragana Hyu/Fyu and Byu/Gyu in Japanese fuzzy search
    ignoreHyuByu_ja_JP             = int(css::i18n::TransliterationModules_ignoreHyuByu_ja_JP),
    /// Ignore Katakana and Hiragana Se/Sye and Ze/Je in Japanese fuzzy search
    ignoreSeZe_ja_JP               = int(css::i18n::TransliterationModules_ignoreSeZe_ja_JP),
    /// Ignore Katakana YA/A which follows the character in either I or E row in Japanese fuzzy search
    ignoreIandEfollowedByYa_ja_JP  = int(css::i18n::TransliterationModules_ignoreIandEfollowedByYa_ja_JP),
    /// Ignore Katakana KI/KU which follows the character in SA column in Japanese fuzzy search
    ignoreKiKuFollowedBySa_ja_JP   = int(css::i18n::TransliterationModules_ignoreKiKuFollowedBySa_ja_JP),
    /// Ignore Japanese normal and small sized character in Japanese fuzzy search
    ignoreSize_ja_JP               = int(css::i18n::TransliterationModules_ignoreSize_ja_JP),
    /// Ignore Japanese prolonged sound mark in Japanese fuzzy search
    ignoreProlongedSoundMark_ja_JP = int(css::i18n::TransliterationModules_ignoreProlongedSoundMark_ja_JP),
    /// Ignore middle dot in Japanese fuzzy search
    ignoreMiddleDot_ja_JP          = int(css::i18n::TransliterationModules_ignoreMiddleDot_ja_JP),
    /// Ignore white space characters, include space, TAB, return, etc. in Japanese fuzzy search
    ignoreSpace_ja_JP              = int(css::i18n::TransliterationModules_ignoreSpace_ja_JP),
    /// transliterate Japanese small sized character to normal sized character
    smallToLarge_ja_JP             = int(css::i18n::TransliterationModules_smallToLarge_ja_JP),
    /// transliterate Japanese normal sized character to small sized character
    largeToSmall_ja_JP             = int(css::i18n::TransliterationModules_largeToSmall_ja_JP),

    IGNORE_DIACRITICS_CTL          = int(css::i18n::TransliterationModulesExtra::IGNORE_DIACRITICS_CTL),
    IGNORE_KASHIDA_CTL             = int(css::i18n::TransliterationModulesExtra::IGNORE_KASHIDA_CTL)
};
namespace o3tl {
    template<> struct typed_flags<TransliterationFlags> : is_typed_flags<TransliterationFlags, 0x7fffffff> {};
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
