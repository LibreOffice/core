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
inline constexpr OUString UPN_IS_USE_DICTIONARY_LIST          = u"IsUseDictionaryList"_ustr;
inline constexpr OUString UPN_IS_IGNORE_CONTROL_CHARACTERS    = u"IsIgnoreControlCharacters"_ustr;
inline constexpr OUString UPN_ACTIVE_DICTIONARIES             = u"ActiveDictionaries"_ustr;

// UNO property names for SpellChecker
inline constexpr OUString UPN_IS_SPELL_UPPER_CASE             = u"IsSpellUpperCase"_ustr;
inline constexpr OUString UPN_IS_SPELL_WITH_DIGITS            = u"IsSpellWithDigits"_ustr;

// UNO property names for Hyphenator
inline constexpr OUString UPN_HYPH_MIN_LEADING                = u"HyphMinLeading"_ustr;
inline constexpr OUString UPN_HYPH_MIN_TRAILING               = u"HyphMinTrailing"_ustr;
inline constexpr OUString UPN_HYPH_MIN_WORD_LENGTH            = u"HyphMinWordLength"_ustr;
inline constexpr OUString UPN_HYPH_NO_CAPS                    = u"HyphNoCaps"_ustr;
inline constexpr OUString UPN_HYPH_NO_LAST_WORD               = u"HyphNoLastWord"_ustr;
inline constexpr OUString UPN_HYPH_ZONE                       = u"HyphZone"_ustr;
inline constexpr OUString UPN_HYPH_KEEP                       = u"HyphKeep"_ustr;

// UNO property names for Lingu
// (those not covered by the SpellChecker and Hyphenator
// properties and more likely to be used in other modules only)
inline constexpr OUString UPN_DEFAULT_LANGUAGE                = u"DefaultLanguage"_ustr;
inline constexpr OUString UPN_DEFAULT_LOCALE                  = u"DefaultLocale"_ustr;
inline constexpr OUString UPN_DEFAULT_LOCALE_CJK              = u"DefaultLocale_CJK"_ustr;
inline constexpr OUString UPN_DEFAULT_LOCALE_CTL              = u"DefaultLocale_CTL"_ustr;
inline constexpr OUString UPN_IS_HYPH_AUTO                    = u"IsHyphAuto"_ustr;
inline constexpr OUString UPN_IS_HYPH_SPECIAL                 = u"IsHyphSpecial"_ustr;
inline constexpr OUString UPN_IS_SPELL_AUTO                   = u"IsSpellAuto"_ustr;
inline constexpr OUString UPN_IS_SPELL_CLOSED_COMPOUND        = u"IsSpellClosedCompound"_ustr;
inline constexpr OUString UPN_IS_SPELL_HYPHENATED_COMPOUND    = u"IsSpellHyphenatedCompound"_ustr;
inline constexpr OUString UPN_IS_SPELL_SPECIAL                = u"IsSpellSpecial"_ustr;
inline constexpr OUString UPN_IS_WRAP_REVERSE                 = u"IsWrapReverse"_ustr;
inline constexpr OUString UPN_DATA_FILES_CHANGED_CHECK_VALUE  = u"DataFilesChangedCheckValue"_ustr;

// UNO property names for text conversion options
inline constexpr OUString UPN_ACTIVE_CONVERSION_DICTIONARIES  = u"ActiveConversionDictionaries"_ustr;
inline constexpr OUString UPN_IS_IGNORE_POST_POSITIONAL_WORD  = u"IsIgnorePostPositionalWord"_ustr;
inline constexpr OUString UPN_IS_AUTO_CLOSE_DIALOG            = u"IsAutoCloseDialog"_ustr;
inline constexpr OUString UPN_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST = u"IsShowEntriesRecentlyUsedFirst"_ustr;
inline constexpr OUString UPN_IS_AUTO_REPLACE_UNIQUE_ENTRIES  = u"IsAutoReplaceUniqueEntries"_ustr;
inline constexpr OUString UPN_IS_DIRECTION_TO_SIMPLIFIED      = u"IsDirectionToSimplified"_ustr;
inline constexpr OUString UPN_IS_USE_CHARACTER_VARIANTS       = u"IsUseCharacterVariants"_ustr;
inline constexpr OUString UPN_IS_TRANSLATE_COMMON_TERMS       = u"IsTranslateCommonTerms"_ustr;
inline constexpr OUString UPN_IS_REVERSE_MAPPING              = u"IsReverseMapping"_ustr;

inline constexpr OUString UPN_IS_GRAMMAR_AUTO                 = u"IsAutoGrammarCheck"_ustr;
inline constexpr OUString UPN_IS_GRAMMAR_INTERACTIVE          = u"IsInteractiveGrammarCheck"_ustr;

// uno property handles
#define UPH_IS_USE_DICTIONARY_LIST           0
#define UPH_IS_IGNORE_CONTROL_CHARACTERS     1
#define UPH_IS_SPELL_UPPER_CASE              2
#define UPH_IS_SPELL_WITH_DIGITS             3
#define UPH_HYPH_MIN_LEADING                 4
#define UPH_HYPH_MIN_TRAILING                5
#define UPH_HYPH_MIN_WORD_LENGTH             6
#define UPH_DEFAULT_LOCALE                   7
#define UPH_IS_SPELL_AUTO                    8
#define UPH_IS_SPELL_CLOSED_COMPOUND         9
#define UPH_IS_SPELL_HYPHENATED_COMPOUND    10
#define UPH_IS_SPELL_SPECIAL                11
#define UPH_IS_HYPH_AUTO                    12
#define UPH_IS_HYPH_SPECIAL                 13
#define UPH_IS_WRAP_REVERSE                 14
#define UPH_DATA_FILES_CHANGED_CHECK_VALUE  15
#define UPH_DEFAULT_LANGUAGE                16
#define UPH_DEFAULT_LOCALE_CJK              17
#define UPH_DEFAULT_LOCALE_CTL              18
#define UPH_ACTIVE_DICTIONARIES             19
#define UPH_ACTIVE_CONVERSION_DICTIONARIES  20
#define UPH_IS_IGNORE_POST_POSITIONAL_WORD  21
#define UPH_IS_AUTO_CLOSE_DIALOG            22
#define UPH_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST     23
#define UPH_IS_AUTO_REPLACE_UNIQUE_ENTRIES  24
#define UPH_IS_DIRECTION_TO_SIMPLIFIED      25
#define UPH_IS_USE_CHARACTER_VARIANTS       26
#define UPH_IS_TRANSLATE_COMMON_TERMS       27
#define UPH_IS_REVERSE_MAPPING              28
#define UPH_IS_GRAMMAR_AUTO                 29
#define UPH_IS_GRAMMAR_INTERACTIVE          30
#define UPH_HYPH_NO_CAPS                    31
#define UPH_HYPH_NO_LAST_WORD               32
#define UPH_HYPH_ZONE                       33
#define UPH_HYPH_KEEP                       34

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
