/*************************************************************************
 *
 *  $RCSfile: linguprops.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-04-27 15:59:19 $
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

#ifndef _SVTOOLS_LINGUPROPS_HXX_
#define _SVTOOLS_LINGUPROPS_HXX_


// UNO property names for general options
#define UPN_IS_GERMAN_PRE_REFORM            "IsGermanPreReform"
#define UPN_IS_USE_DICTIONARY_LIST          "IsUseDictionaryList"
#define UPN_IS_IGNORE_CONTROL_CHARACTERS    "IsIgnoreControlCharacters"
#define UPN_ACTIVE_DICTIONARIES             "ActiveDictionaries"

// UNO property names for SpellChecker
#define UPN_IS_SPELL_UPPER_CASE             "IsSpellUpperCase"
#define UPN_IS_SPELL_WITH_DIGITS            "IsSpellWithDigits"
#define UPN_IS_SPELL_CAPITALIZATION         "IsSpellCapitalization"

// UNO property names for Hyphenator
#define UPN_HYPH_MIN_LEADING                "HyphMinLeading"
#define UPN_HYPH_MIN_TRAILING               "HyphMinTrailing"
#define UPN_HYPH_MIN_WORD_LENGTH            "HyphMinWordLength"

// UNO property names for Lingu
// (those not covered by the SpellChecker and Hyphenator
// properties and more likely to be used in other modules only)
#define UPN_DEFAULT_LANGUAGE                "DefaultLanguage"
#define UPN_DEFAULT_LOCALE                  "DefaultLocale"
#define UPN_DEFAULT_LOCALE_CJK              "DefaultLocale_CJK"
#define UPN_DEFAULT_LOCALE_CTL              "DefaultLocale_CTL"
#define UPN_IS_HYPH_AUTO                    "IsHyphAuto"
#define UPN_IS_HYPH_SPECIAL                 "IsHyphSpecial"
#define UPN_IS_SPELL_AUTO                   "IsSpellAuto"
#define UPN_IS_SPELL_HIDE                   "IsSpellHide"
#define UPN_IS_SPELL_IN_ALL_LANGUAGES       "IsSpellInAllLanguages"
#define UPN_IS_SPELL_SPECIAL                "IsSpellSpecial"
#define UPN_IS_WRAP_REVERSE                 "IsWrapReverse"

// UNO property names for text conversion options
#define UPN_ACTIVE_CONVERSION_DICTIONARIES  "ActiveConversionDictionaries"
#define UPN_IS_IGNORE_POST_POSITIONAL_WORD  "IsIgnorePostPositionalWord"
#define UPN_IS_AUTO_CLOSE_DIALOG            "IsAutoCloseDialog"
#define UPN_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST     "IsShowEntriesRecentlyUsedFirst"
#define UPN_IS_AUTO_REPLACE_UNIQUE_ENTRIES  "IsAutoReplaceUniqueEntries"

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
#define UPH_DEFAULT_LANGUAGE                21
#define UPH_DEFAULT_LOCALE_CJK              22
#define UPH_DEFAULT_LOCALE_CTL              23
#define UPH_ACTIVE_DICTIONARIES             24
#define UPH_ACTIVE_CONVERSION_DICTIONARIES  25
#define UPH_IS_IGNORE_POST_POSITIONAL_WORD  26
#define UPH_IS_AUTO_CLOSE_DIALOG            27
#define UPH_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST     28
#define UPH_IS_AUTO_REPLACE_UNIQUE_ENTRIES  29

#endif

