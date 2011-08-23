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

#ifndef _SVTOOLS_LINGUPROPS_HXX_
#define _SVTOOLS_LINGUPROPS_HXX_


// UNO property names for general options
#define UPN_IS_GERMAN_PRE_REFORM			"IsGermanPreReform"
#define	UPN_IS_USE_DICTIONARY_LIST      	"IsUseDictionaryList"
#define UPN_IS_IGNORE_CONTROL_CHARACTERS	"IsIgnoreControlCharacters"
#define UPN_ACTIVE_DICTIONARIES				"ActiveDictionaries"

// UNO property names for SpellChecker
#define UPN_IS_SPELL_UPPER_CASE				"IsSpellUpperCase"
#define UPN_IS_SPELL_WITH_DIGITS			"IsSpellWithDigits"
#define UPN_IS_SPELL_CAPITALIZATION			"IsSpellCapitalization"

// UNO property names for Hyphenator
#define UPN_HYPH_MIN_LEADING				"HyphMinLeading"
#define UPN_HYPH_MIN_TRAILING				"HyphMinTrailing"
#define UPN_HYPH_MIN_WORD_LENGTH			"HyphMinWordLength"

// UNO property names for Lingu
// (those not covered by the SpellChecker and Hyphenator 
// properties and more likely to be used in other modules only)
#define UPN_DEFAULT_LANGUAGE				"DefaultLanguage"
#define UPN_DEFAULT_LOCALE					"DefaultLocale"
#define UPN_DEFAULT_LOCALE_CJK				"DefaultLocale_CJK"
#define UPN_DEFAULT_LOCALE_CTL				"DefaultLocale_CTL"
#define UPN_IS_HYPH_AUTO					"IsHyphAuto"
#define UPN_IS_HYPH_SPECIAL					"IsHyphSpecial"
#define UPN_IS_SPELL_AUTO					"IsSpellAuto"
#define UPN_IS_SPELL_HIDE					"IsSpellHide"
#define UPN_IS_SPELL_IN_ALL_LANGUAGES		"IsSpellInAllLanguages"
#define UPN_IS_SPELL_SPECIAL				"IsSpellSpecial"
#define UPN_IS_WRAP_REVERSE					"IsWrapReverse"
#define UPN_DATA_FILES_CHANGED_CHECK_VALUE  "DataFilesChangedCheckValue"

// UNO property names for text conversion options
#define UPN_ACTIVE_CONVERSION_DICTIONARIES  "ActiveConversionDictionaries"
#define UPN_IS_IGNORE_POST_POSITIONAL_WORD  "IsIgnorePostPositionalWord"
#define UPN_IS_AUTO_CLOSE_DIALOG            "IsAutoCloseDialog"
#define UPN_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST     "IsShowEntriesRecentlyUsedFirst"
#define UPN_IS_AUTO_REPLACE_UNIQUE_ENTRIES  "IsAutoReplaceUniqueEntries"
#define UPN_IS_DIRECTION_TO_SIMPLIFIED  "IsDirectionToSimplified"
#define UPN_IS_USE_CHARACTER_VARIANTS  "IsUseCharacterVariants"
#define UPN_IS_TRANSLATE_COMMON_TERMS  "IsTranslateCommonTerms"
#define UPN_IS_REVERSE_MAPPING  "IsReverseMapping"

// uno property handles
#define UPH_IS_GERMAN_PRE_REFORM			 0
#define	UPH_IS_USE_DICTIONARY_LIST			 1
#define UPH_IS_IGNORE_CONTROL_CHARACTERS	 2
#define UPH_IS_SPELL_UPPER_CASE				 3
#define UPH_IS_SPELL_WITH_DIGITS			 4
#define UPH_IS_SPELL_CAPITALIZATION			 5
#define UPH_HYPH_MIN_LEADING				 6
#define UPH_HYPH_MIN_TRAILING				 7
#define UPH_HYPH_MIN_WORD_LENGTH			 8
#define UPH_DEFAULT_LOCALE					 9
#define UPH_IS_SPELL_AUTO					10
#define UPH_IS_SPELL_HIDE					11
#define UPH_IS_SPELL_IN_ALL_LANGUAGES		12
#define UPH_IS_SPELL_SPECIAL				13
#define UPH_IS_HYPH_AUTO					14
#define UPH_IS_HYPH_SPECIAL					15
#define UPH_IS_WRAP_REVERSE					16
#define UPH_DATA_FILES_CHANGED_CHECK_VALUE  17
#define UPH_DEFAULT_LANGUAGE				21
#define UPH_DEFAULT_LOCALE_CJK				22
#define UPH_DEFAULT_LOCALE_CTL				23
#define UPH_ACTIVE_DICTIONARIES				24
#define UPH_ACTIVE_CONVERSION_DICTIONARIES  25
#define UPH_IS_IGNORE_POST_POSITIONAL_WORD  26
#define UPH_IS_AUTO_CLOSE_DIALOG            27
#define UPH_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST     28
#define UPH_IS_AUTO_REPLACE_UNIQUE_ENTRIES  29
#define UPH_IS_DIRECTION_TO_SIMPLIFIED      30
#define UPH_IS_USE_CHARACTER_VARIANTS       31
#define UPH_IS_TRANSLATE_COMMON_TERMS       32
#define UPH_IS_REVERSE_MAPPING              33
#endif

