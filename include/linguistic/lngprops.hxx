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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
