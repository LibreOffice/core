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

 /**
  *
  *
  *
  *
  * TODO
  * - Add exception throwing when h == NULL
  * - Not init h when implicit constructor is launched
  */

#include <string.h>
#include <sstream>
#include <iostream>

#ifdef SYSTEM_LIBEXTTEXTCAT
#include <libexttextcat/textcat.h>
#include <libexttextcat/common.h>
#include <libexttextcat/constants.h>
#include <libexttextcat/fingerprint.h>
#include <libexttextcat/utf8misc.h>
#else
#include <textcat.h>
#include <common.h>
#include <constants.h>
#include <fingerprint.h>
#include <utf8misc.h>
#endif

#include <sal/types.h>

#include<rtl/character.hxx>
#include "simpleguesser.hxx"

using namespace std;

static int startsAsciiCaseInsensitive(const std::string &s1, const std::string &s2){
            size_t i;
            int ret = 0;

            size_t min = s1.length();
            if (min > s2.length())
                min = s2.length();

            for(i = 0; i < min && s2[i] && s1[i] && !ret; i++){
                    ret = rtl::toAsciiUpperCase(static_cast<unsigned char>(s1[i]))
                        -  rtl::toAsciiUpperCase(static_cast<unsigned char>(s2[i]));
                    if(s1[i] == '.' || s2[i] == '.') {ret = 0;}    //. is a neutral character
            }
            return ret;
 }

/**
 * This following structure is from textcat.c
 */
typedef struct textcat_t{

    void **fprint;
    char *fprint_disable;
    uint4 size;
    uint4 maxsize;

    char output[MAXOUTPUTSIZE];

} textcat_t;
// end of the 3 structs

SimpleGuesser::SimpleGuesser()
{
    h = nullptr;
}

SimpleGuesser& SimpleGuesser::operator=(const SimpleGuesser& sg){
    // Check for self-assignment!
    if (this == &sg)      // Same object?
      return *this;       // Yes, so skip assignment, and just return *this.

    if(h){textcat_Done(h);}
    h = sg.h;
    return *this;
}

SimpleGuesser::~SimpleGuesser()
{
    if(h){textcat_Done(h);}
}

/*!
    \fn SimpleGuesser::GuessLanguage(char* text)
 */
vector<Guess> SimpleGuesser::GuessLanguage(const char* text)
{
    vector<Guess> guesses;

    if (!h)
        return guesses;

    int len = strlen(text);

    if (len > MAX_STRING_LENGTH_TO_ANALYSE)
        len = MAX_STRING_LENGTH_TO_ANALYSE;

    const char *guess_list = textcat_Classify(h, text, len);

    if (strcmp(guess_list, TEXTCAT_RESULT_SHORT_STR) == 0)
        return guesses;

    int current_pointer = 0;

    for(int i = 0; guess_list[current_pointer] != '\0'; i++)
    {
        while (guess_list[current_pointer] != GUESS_SEPARATOR_OPEN && guess_list[current_pointer] != '\0')
            current_pointer++;
        if(guess_list[current_pointer] != '\0')
        {
            Guess g(guess_list + current_pointer);

            guesses.push_back(g);

            current_pointer++;
        }
    }

    return guesses;
}

Guess SimpleGuesser::GuessPrimaryLanguage(const char* text)
{
    vector<Guess> ret = GuessLanguage(text);
    return ret.empty() ? Guess() : ret[0];
}
/**
 * Is used to know which language is available, unavailable or both
 * when mask = 0xF0, return only Available
 * when mask = 0x0F, return only Unavailable
 * when mask = 0xFF, return both Available and Unavailable
 */
vector<Guess> SimpleGuesser::GetManagedLanguages(const char mask)
{
    textcat_t *tables = static_cast<textcat_t*>(h);

    vector<Guess> lang;
    if(!h){return lang;}

    for (size_t i=0; i<tables->size; ++i)
    {
        if (tables->fprint_disable[i] & mask)
        {
            string langStr = "[";
            langStr += fp_Name(tables->fprint[i]);
            Guess g(langStr.c_str());
            lang.push_back(g);
        }
    }

    return lang;
}

vector<Guess> SimpleGuesser::GetAvailableLanguages()
{
    return GetManagedLanguages( sal::static_int_cast< char >( 0xF0 ) );
}

vector<Guess> SimpleGuesser::GetUnavailableLanguages()
{
    return GetManagedLanguages( sal::static_int_cast< char >( 0x0F ));
}

vector<Guess> SimpleGuesser::GetAllManagedLanguages()
{
    return GetManagedLanguages( sal::static_int_cast< char >( 0xFF ));
}

void SimpleGuesser::XableLanguage(const string& lang, char mask)
{
    textcat_t *tables = static_cast<textcat_t*>(h);

    if(!h){return;}

    for (size_t i=0; i<tables->size; i++)
    {
        string language(fp_Name(tables->fprint[i]));
        if (startsAsciiCaseInsensitive(language,lang) == 0)
            tables->fprint_disable[i] = mask;
    }
}

void SimpleGuesser::EnableLanguage(const string& lang)
{
    XableLanguage(lang,  sal::static_int_cast< char >( 0xF0 ));
}

void SimpleGuesser::DisableLanguage(const string& lang)
{
    XableLanguage(lang,  sal::static_int_cast< char >( 0x0F ));
}

void SimpleGuesser::SetDBPath(const char* path, const char* prefix)
{
    if (h)
        textcat_Done(h);
    h = special_textcat_Init(path, prefix);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
