/*************************************************************************
 *
 *  $RCSfile: lngprops.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: tl $ $Date: 2000-11-22 15:54:32 $
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

#ifndef _LINGUISTIC_LNGPROPS_HHX_
#define _LINGUISTIC_LNGPROPS_HHX_


#define UPN_IS_GERMAN_PRE_REFORM            "IsGermanPreReform"
#define UPN_IS_USE_DICTIONARY_LIST          "IsUseDictionaryList"
#define UPN_IS_IGNORE_CONTROL_CHARACTERS    "IsIgnoreControlCharacters"

// UNO property names for SpellChecker
#define UPN_IS_SPELL_UPPER_CASE             "IsSpellUpperCase"
#define UPN_IS_SPELL_WITH_DIGITS            "IsSpellWithDigits"
#define UPN_IS_SPELL_CAPITALIZATION         "IsSpellCapitalization"

// UNO property names for Hyphenator
#define UPN_HYPH_MIN_LEADING                "HyphMinLeading"
#define UPN_HYPH_MIN_TRAILING               "HyphMinTrailing"
#define UPN_HYPH_MIN_WORD_LENGTH            "HyphMinWordLength"

// UNO property names for OtherLingu (foreign Linguistik)
#define UPN_IS_STANDARD_HYPHENATOR          "IsStandardHyphenator"
#define UPN_IS_STANDARD_SPELL_CHECKER       "IsStandardSpellChecker"
#define UPN_IS_STANDARD_THESAURUS           "IsStandardThesaurus"
#define UPN_OTHER_LINGU_INDEX               "OtherLinguIndex"

// UNO property names for Lingu
// (those not covered by the SpellChecker, Hyphenator and OtherLingu properties
// and more likely used in other modules only)
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

// WIDs for property names
//!! Don't change values! They are used as the property handles in
//!! the service description
#define WID_IS_GERMAN_PRE_REFORM             0
#define WID_IS_USE_DICTIONARY_LIST           1
#define WID_IS_IGNORE_CONTROL_CHARACTERS     2
#define WID_IS_SPELL_UPPER_CASE              3
#define WID_IS_SPELL_WITH_DIGITS             4
#define WID_IS_SPELL_CAPITALIZATION          5
#define WID_HYPH_MIN_LEADING                 6
#define WID_HYPH_MIN_TRAILING                7
#define WID_HYPH_MIN_WORD_LENGTH             8
#define WID_DEFAULT_LOCALE                   9
#define WID_IS_SPELL_AUTO                   10
#define WID_IS_SPELL_HIDE                   11
#define WID_IS_SPELL_IN_ALL_LANGUAGES       12
#define WID_IS_SPELL_SPECIAL                13
#define WID_IS_HYPH_AUTO                    14
#define WID_IS_HYPH_SPECIAL                 15
#define WID_IS_WRAP_REVERSE                 16
#define WID_IS_STANDARD_HYPHENATOR          17
#define WID_IS_STANDARD_SPELL_CHECKER       18
#define WID_IS_STANDARD_THESAURUS           19
#define WID_OTHER_LINGU_INDEX               20
#define WID_DEFAULT_LANGUAGE                21
#define WID_DEFAULT_LOCALE_CJK              22
#define WID_DEFAULT_LOCALE_CTL              23


// UNO property handles
#define UPH_IS_GERMAN_PRE_REFORM            WID_IS_GERMAN_PRE_REFORM
#define UPH_IS_USE_DICTIONARY_LIST          WID_IS_USE_DICTIONARY_LIST
#define UPH_IS_IGNORE_CONTROL_CHARACTERS    WID_IS_IGNORE_CONTROL_CHARACTERS
//
#define UPH_IS_SPELL_UPPER_CASE             WID_IS_SPELL_UPPER_CASE
#define UPH_IS_SPELL_WITH_DIGITS            WID_IS_SPELL_WITH_DIGITS
#define UPH_IS_SPELL_CAPITALIZATION         WID_IS_SPELL_CAPITALIZATION
//
#define UPH_HYPH_MIN_LEADING                WID_HYPH_MIN_LEADING
#define UPH_HYPH_MIN_TRAILING               WID_HYPH_MIN_TRAILING
#define UPH_HYPH_MIN_WORD_LENGTH            WID_HYPH_MIN_WORD_LENGTH


#endif

