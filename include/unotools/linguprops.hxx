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
#pragma once

#include <rtl/ustring.hxx>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

// UNO property names for general options
inline constexpr OUStringLiteral UPN_IS_GERMAN_PRE_REFORM            = u"IsGermanPreReform";     /*! deprecated #i91949 !*/
inline constexpr OUStringLiteral UPN_IS_USE_DICTIONARY_LIST          = u"IsUseDictionaryList";
inline constexpr OUStringLiteral UPN_IS_IGNORE_CONTROL_CHARACTERS    = u"IsIgnoreControlCharacters";
inline constexpr OUStringLiteral UPN_ACTIVE_DICTIONARIES             = u"ActiveDictionaries";

// UNO property names for SpellChecker
inline constexpr OUStringLiteral UPN_IS_SPELL_UPPER_CASE             = u"IsSpellUpperCase";
inline constexpr OUStringLiteral UPN_IS_SPELL_WITH_DIGITS            = u"IsSpellWithDigits";
inline constexpr OUStringLiteral UPN_IS_SPELL_CAPITALIZATION         = u"IsSpellCapitalization";

// UNO property names for Hyphenator
inline constexpr OUStringLiteral UPN_HYPH_MIN_LEADING                = u"HyphMinLeading";
inline constexpr OUStringLiteral UPN_HYPH_MIN_TRAILING               = u"HyphMinTrailing";
inline constexpr OUStringLiteral UPN_HYPH_MIN_WORD_LENGTH            = u"HyphMinWordLength";
inline constexpr OUStringLiteral UPN_HYPH_NO_CAPS                    = u"HyphNoCaps";
inline constexpr OUStringLiteral UPN_HYPH_NO_LAST_WORD               = u"HyphNoLastWord";
inline constexpr OUStringLiteral UPN_HYPH_ZONE                       = u"HyphZone";

// UNO property names for Lingu
// (those not covered by the SpellChecker and Hyphenator
// properties and more likely to be used in other modules only)
inline constexpr OUStringLiteral UPN_DEFAULT_LANGUAGE                = u"DefaultLanguage";
inline constexpr OUStringLiteral UPN_DEFAULT_LOCALE                  = u"DefaultLocale";
inline constexpr OUStringLiteral UPN_DEFAULT_LOCALE_CJK              = u"DefaultLocale_CJK";
inline constexpr OUStringLiteral UPN_DEFAULT_LOCALE_CTL              = u"DefaultLocale_CTL";
inline constexpr OUStringLiteral UPN_IS_HYPH_AUTO                    = u"IsHyphAuto";
inline constexpr OUStringLiteral UPN_IS_HYPH_SPECIAL                 = u"IsHyphSpecial";
inline constexpr OUStringLiteral UPN_IS_SPELL_AUTO                   = u"IsSpellAuto";
inline constexpr OUStringLiteral UPN_IS_SPELL_HIDE                   = u"IsSpellHide";           /*! deprecated #i91949 !*/
inline constexpr OUStringLiteral UPN_IS_SPELL_IN_ALL_LANGUAGES       = u"IsSpellInAllLanguages"; /*! deprecated #i91949 !*/
inline constexpr OUStringLiteral UPN_IS_SPELL_SPECIAL                = u"IsSpellSpecial";
inline constexpr OUStringLiteral UPN_IS_WRAP_REVERSE                 = u"IsWrapReverse";
inline constexpr OUStringLiteral UPN_DATA_FILES_CHANGED_CHECK_VALUE  = u"DataFilesChangedCheckValue";

// UNO property names for text conversion options
inline constexpr OUStringLiteral UPN_ACTIVE_CONVERSION_DICTIONARIES  = u"ActiveConversionDictionaries";
inline constexpr OUStringLiteral UPN_IS_IGNORE_POST_POSITIONAL_WORD  = u"IsIgnorePostPositionalWord";
inline constexpr OUStringLiteral UPN_IS_AUTO_CLOSE_DIALOG            = u"IsAutoCloseDialog";
inline constexpr OUStringLiteral UPN_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST = u"IsShowEntriesRecentlyUsedFirst";
inline constexpr OUStringLiteral UPN_IS_AUTO_REPLACE_UNIQUE_ENTRIES  = u"IsAutoReplaceUniqueEntries";
inline constexpr OUStringLiteral UPN_IS_DIRECTION_TO_SIMPLIFIED      = u"IsDirectionToSimplified";
inline constexpr OUStringLiteral UPN_IS_USE_CHARACTER_VARIANTS       = u"IsUseCharacterVariants";
inline constexpr OUStringLiteral UPN_IS_TRANSLATE_COMMON_TERMS       = u"IsTranslateCommonTerms";
inline constexpr OUStringLiteral UPN_IS_REVERSE_MAPPING              = u"IsReverseMapping";

inline constexpr OUStringLiteral UPN_IS_GRAMMAR_AUTO                 = u"IsAutoGrammarCheck";
inline constexpr OUStringLiteral UPN_IS_GRAMMAR_INTERACTIVE          = u"IsInteractiveGrammarCheck";

// uno property handles
#define UPH_IS_GERMAN_PRE_REFORM             0
#define UPH_IS_USE_DICTIONARY_LIST           1
#define UPH_IS_IGNORE_CONTROL_CHARACTERS     2
#define UPH_IS_SPELL_UPPER_CASE              3
#define UPH_IS_SPELL_WITH_DIGITS             4
#define UPH_IS_SPELL_CAPITALIZATION          5
#define UPH_HYPH_MIN_LEADING                 6
#define UPH_HYPH_MIN_TRAILING                7
#define UPH_HYPH_MIN_WORD_LENGTH             8
#define UPH_DEFAULT_LOCALE                   9
#define UPH_IS_SPELL_AUTO                   10
#define UPH_IS_SPELL_HIDE                   11
#define UPH_IS_SPELL_IN_ALL_LANGUAGES       12
#define UPH_IS_SPELL_SPECIAL                13
#define UPH_IS_HYPH_AUTO                    14
#define UPH_IS_HYPH_SPECIAL                 15
#define UPH_IS_WRAP_REVERSE                 16
#define UPH_DATA_FILES_CHANGED_CHECK_VALUE  17
#define UPH_DEFAULT_LANGUAGE                21
#define UPH_DEFAULT_LOCALE_CJK              22
#define UPH_DEFAULT_LOCALE_CTL              23
#define UPH_ACTIVE_DICTIONARIES             24
#define UPH_ACTIVE_CONVERSION_DICTIONARIES  25
#define UPH_IS_IGNORE_POST_POSITIONAL_WORD  26
#define UPH_IS_AUTO_CLOSE_DIALOG            27
#define UPH_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST     28
#define UPH_IS_AUTO_REPLACE_UNIQUE_ENTRIES  29
#define UPH_IS_DIRECTION_TO_SIMPLIFIED      30
#define UPH_IS_USE_CHARACTER_VARIANTS       31
#define UPH_IS_TRANSLATE_COMMON_TERMS       32
#define UPH_IS_REVERSE_MAPPING              33
#define UPH_IS_GRAMMAR_AUTO                 34
#define UPH_IS_GRAMMAR_INTERACTIVE          35
#define UPH_HYPH_NO_CAPS                    36
#define UPH_HYPH_NO_LAST_WORD               37
#define UPH_HYPH_ZONE                       38

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
