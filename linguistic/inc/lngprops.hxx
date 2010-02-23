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

#ifndef _LINGUISTIC_LNGPROPS_HHX_
#define _LINGUISTIC_LNGPROPS_HHX_

#include <unotools/linguprops.hxx>

// maximal number of suggestions to be returned in spelling  context-menu
// (may not include results added by looking up user dictionaries)
#define UPN_MAX_NUMBER_OF_SUGGESTIONS       "MaxNumberOfSuggestions"

// WIDs for property names
//!! Don't change values! They are used as the property handles in
//!! the service description
#define WID_IS_GERMAN_PRE_REFORM            UPH_IS_GERMAN_PRE_REFORM        /*! deprecated !*/
#define WID_IS_USE_DICTIONARY_LIST          UPH_IS_USE_DICTIONARY_LIST
#define WID_IS_IGNORE_CONTROL_CHARACTERS    UPH_IS_IGNORE_CONTROL_CHARACTERS
#define WID_IS_SPELL_UPPER_CASE             UPH_IS_SPELL_UPPER_CASE
#define WID_IS_SPELL_WITH_DIGITS            UPH_IS_SPELL_WITH_DIGITS
#define WID_IS_SPELL_CAPITALIZATION         UPH_IS_SPELL_CAPITALIZATION
#define WID_HYPH_MIN_LEADING                UPH_HYPH_MIN_LEADING
#define WID_HYPH_MIN_TRAILING               UPH_HYPH_MIN_TRAILING
#define WID_HYPH_MIN_WORD_LENGTH            UPH_HYPH_MIN_WORD_LENGTH
#define WID_DEFAULT_LOCALE                  UPH_DEFAULT_LOCALE
#define WID_IS_SPELL_AUTO                   UPH_IS_SPELL_AUTO
#define WID_IS_SPELL_HIDE                   UPH_IS_SPELL_HIDE               /*! deprecated !*/
#define WID_IS_SPELL_IN_ALL_LANGUAGES       UPH_IS_SPELL_IN_ALL_LANGUAGES   /*! deprecated !*/
#define WID_IS_SPELL_SPECIAL                UPH_IS_SPELL_SPECIAL
#define WID_IS_HYPH_AUTO                    UPH_IS_HYPH_AUTO
#define WID_IS_HYPH_SPECIAL                 UPH_IS_HYPH_SPECIAL
#define WID_IS_WRAP_REVERSE                 UPH_IS_WRAP_REVERSE
#define WID_DEFAULT_LANGUAGE                UPH_DEFAULT_LANGUAGE
#define WID_DEFAULT_LOCALE_CJK              UPH_DEFAULT_LOCALE_CJK
#define WID_DEFAULT_LOCALE_CTL              UPH_DEFAULT_LOCALE_CTL

#endif

