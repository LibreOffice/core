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

#include <cassert>
#include <iostream>
#include <string.h>

#ifdef SYSTEM_LIBEXTTEXTCAT
#include <libexttextcat/textcat.h>
#else
#include <textcat.h>
#endif

#include "guess.hxx"

/* Old textcat.h versions defined bad spelled constants. */
#ifndef TEXTCAT_RESULT_UNKNOWN_STR
#define TEXTCAT_RESULT_UNKNOWN_STR _TEXTCAT_RESULT_UNKOWN
#endif

#ifndef TEXTCAT_RESULT_SHORT_STR
#define TEXTCAT_RESULT_SHORT_STR _TEXTCAT_RESULT_SHORT
#endif

using namespace std;

Guess::Guess()
    : language_str(DEFAULT_LANGUAGE)
    , country_str(DEFAULT_COUNTRY)
{
}

/*
* this use a char * string to build the guess object
* a string like those is made as : [language-country-encoding]...
*
*/
Guess::Guess(const char * guess_str)
    : language_str(DEFAULT_LANGUAGE)
    , country_str(DEFAULT_COUNTRY)
{
    //if the guess is not like "UNKNOWN" or "SHORT", go into the brackets
    if(strcmp(guess_str + 1, TEXTCAT_RESULT_UNKNOWN_STR) != 0
       &&
       strcmp(guess_str + 1, TEXTCAT_RESULT_SHORT_STR) != 0)
    {
        // From how this ctor is called from SimpleGuesser::GuessLanguage and
        // SimpleGuesser::GetManagedLanguages in
        // lingucomponent/source/languageguessing/simpleguesser.cxx, guess_str must start with "[":
        assert(guess_str[0] == GUESS_SEPARATOR_OPEN);
        auto const start = guess_str + 1;
        // Only look at the prefix of guess_str, delimited by the next "]" or "[" or end-of-string;
        // split it into at most three segments separated by "-" (where excess occurrences of "-"
        // would become part of the third segment), like "en-US-utf8"; the first segment denotes the
        // language; if there are three segments, the second denotes the country and the third the
        // encoding; otherwise, the second segment, if any (e.g., in "haw-utf8"), denotes the
        // encoding:
        char const * dash1 = nullptr;
        char const * dash2 = nullptr;
        auto p = start;
        for (;; ++p) {
            auto const c = *p;
            if (c == '\0' || c == GUESS_SEPARATOR_OPEN || c == GUESS_SEPARATOR_CLOSE) {
                break;
            }
            if (c == GUESS_SEPARATOR_SEP) {
                if (dash1 == nullptr) {
                    dash1 = p;
                } else {
                    dash2 = p;
                    // The encoding is ignored, so we can stop as soon as we found the second "-":
                    break;
                }
            }
        }
        auto const langLen = (dash1 == nullptr ? p : dash1) - start;
        if (langLen != 0) { // if not we use the default value
            language_str.assign(start, langLen);
        }
        if (dash2 != nullptr) {
            country_str.assign(dash1 + 1, dash2 - (dash1 + 1));
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
