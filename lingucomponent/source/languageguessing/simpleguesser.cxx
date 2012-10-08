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

#include <libexttextcat/textcat.h>
#include <libexttextcat/common.h>
#include <libexttextcat/constants.h>
#include <libexttextcat/fingerprint.h>
#include <libexttextcat/utf8misc.h>

#include <sal/types.h>

#include "altstrfunc.hxx"
#include "simpleguesser.hxx"

#ifndef _UTF8_
#define _UTF8_
#endif


using namespace std;


/**
 * This 3 following structures are from fingerprint.c and textcat.c
 */

typedef struct ngram_t {

    sint2 rank;
    char str[MAXNGRAMSIZE+1];

} ngram_t;

typedef struct fp_t {

    const char *name;
    ngram_t *fprint;
    uint4 size;

} fp_t;

typedef struct textcat_t{

    void **fprint;
    char *fprint_disable;
    uint4 size;
    uint4 maxsize;

    char output[MAXOUTPUTSIZE];

} textcat_t;
/** end of the 3 structs */

SimpleGuesser::SimpleGuesser()
{
    h = NULL;
}

void SimpleGuesser::operator=(SimpleGuesser& sg){
    if(h){textcat_Done(h);}
    h = sg.h;
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

    if (strcmp(guess_list, _TEXTCAT_RESULT_SHORT) == 0)
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
 * Is used to know wich language is available, unavailable or both
 * when mask = 0xF0, return only Available
 * when mask = 0x0F, return only Unavailable
 * when mask = 0xFF, return both Available and Unavailable
 */
vector<Guess> SimpleGuesser::GetManagedLanguages(const char mask)
{
    textcat_t *tables = (textcat_t*)h;

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

void SimpleGuesser::XableLanguage(string lang, char mask)
{
    textcat_t *tables = (textcat_t*)h;

    if(!h){return;}

    for (size_t i=0; i<tables->size; i++)
    {
        string language(fp_Name(tables->fprint[i]));
        if (start(language,lang) == 0)
            tables->fprint_disable[i] = mask;
    }
}

void SimpleGuesser::EnableLanguage(string lang)
{
    XableLanguage(lang,  sal::static_int_cast< char >( 0xF0 ));
}

void SimpleGuesser::DisableLanguage(string lang)
{
    XableLanguage(lang,  sal::static_int_cast< char >( 0x0F ));
}

/**
*
*/
void SimpleGuesser::SetDBPath(const char* path, const char* prefix)
{
    if (h)
        textcat_Done(h);
    h = special_textcat_Init(path, prefix);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
