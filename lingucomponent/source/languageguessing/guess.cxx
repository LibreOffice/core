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

inline bool isSeparator(const char c){
    return c == GUESS_SEPARATOR_OPEN || c == GUESS_SEPARATOR_SEP || c == GUESS_SEPARATOR_CLOSE || c == '\0';
}


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
    string lang;
    string country;
    string enc;

    //if the guess is not like "UNKNOWN" or "SHORT", go into the brackets
    if(strcmp(guess_str + 1, TEXTCAT_RESULT_UNKNOWN_STR) != 0
       &&
       strcmp(guess_str + 1, TEXTCAT_RESULT_SHORT_STR) != 0)
    {

        int current_pointer = 0;

        //this is to go to the first char of the guess string ( the '[' of "[en-US-utf8]" )
        while(!isSeparator(guess_str[current_pointer])){
            current_pointer++;
        }
        current_pointer++;

        //this is to pick up the language ( the "en" from "[en-US-utf8]" )
        while(!isSeparator(guess_str[current_pointer])){
            lang+=guess_str[current_pointer];
            current_pointer++;
        }
        current_pointer++;

        //this is to pick up the country ( the "US" from "[en-US-utf8]" )
        while(!isSeparator(guess_str[current_pointer])){
            country+=guess_str[current_pointer];
            current_pointer++;
        }
        current_pointer++;

        //this is to pick up the encoding ( the "utf8" from "[en-US-utf8]" )
        while(!isSeparator(guess_str[current_pointer])){
            enc+=guess_str[current_pointer];
            current_pointer++;
        }

        if(lang!=""){//if not we use the default value
            language_str=lang;
        }
        country_str=country;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
