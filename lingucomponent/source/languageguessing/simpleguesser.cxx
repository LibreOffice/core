/***************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: simpleguesser.cxx,v $
 * $Revision: 1.5 $
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

 /**
  *
  *
  *
  *
  * TODO
  * - Add exception throwing when h == NULL
  * - Not init h when implicit constructor is launched
  */

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_lingucomponent.hxx"

#include <string.h>
#include <sstream>
#include <iostream>

#include <libtextcat/textcat.h>
#include <libtextcat/common.h>
#include <libtextcat/constants.h>
#include <libtextcat/fingerprint.h>
#include <libtextcat/utf8misc.h>

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

SimpleGuesser::SimpleGuesser(const char* confFile, const char* prefix)
{
    h = special_textcat_Init(confFile, prefix);
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
vector<Guess> SimpleGuesser::GuessLanguage(char* text)
{
        vector<Guess> guesses;

        if(!h){return guesses;}

        //calculate le number of unicode charcters (symbols)
        int len = utfstrlen(text);

    if( len > MAX_STRING_LENGTH_TO_ANALYSE ){len = MAX_STRING_LENGTH_TO_ANALYSE ;}

        char *guess_list = textcat_Classify(h, text, len);

        if(strcmp(guess_list, _TEXTCAT_RESULT_SHORT) == 0){
            return guesses;
        }

        int current_pointer = 0;

        for(int i = 0; guess_list[current_pointer] != '\0'; i++)
        {
            while(guess_list[current_pointer] != GUESS_SEPARATOR_OPEN && guess_list[current_pointer] != '\0'){
                current_pointer++;
            }
            if(guess_list[current_pointer] != '\0')
            {
                Guess g((char*)(guess_list + current_pointer),i);

                guesses.push_back(g);

                current_pointer++;
            }
        }

    return guesses;
}

/*!
    \fn SimpleGuesser::GuessPrimaryLanguage(char* text)
 */
Guess SimpleGuesser::GuessPrimaryLanguage(char* text)
{
    vector<Guess> ret = GuessLanguage(text);
    if(ret.size() > 0){
        return GuessLanguage(text)[0];
    }
    else{
        return Guess();
    }
}
/**
 * Is used to know wich language is available, unavailable or both
 * when mask = 0xF0, return only Available
 * when mask = 0x0F, return only Unavailable
 * when mask = 0xFF, return both Available and Unavailable
 */
vector<Guess> SimpleGuesser::GetManagedLanguages(const char mask)
{
    size_t i;
    textcat_t *tables = (textcat_t*)h;

    vector<Guess> lang;
    if(!h){return lang;}

    for (i=0; i<tables->size; i++) {
        if(tables->fprint_disable[i] & mask){
            string langStr = "[";
            langStr += (char*)fp_Name(tables->fprint[i]);
            Guess g( (char *)langStr.c_str() , i);
            lang.push_back(g);
        }
    }

    return lang;
}

vector<Guess> SimpleGuesser::GetAvailableLanguages(){
    return GetManagedLanguages( sal::static_int_cast< char >( 0xF0 ) );
}

vector<Guess> SimpleGuesser::GetUnavailableLanguages(){
    return GetManagedLanguages( sal::static_int_cast< char >( 0x0F ));
}

vector<Guess> SimpleGuesser::GetAllManagedLanguages(){
    return GetManagedLanguages( sal::static_int_cast< char >( 0xFF ));
}

void SimpleGuesser::XableLanguage(string lang, char mask){
    size_t i;
    textcat_t *tables = (textcat_t*)h;

    if(!h){return;}

    for (i=0; i<tables->size; i++) {
        string language(fp_Name(tables->fprint[i]));
        if(start(language,lang) == 0){
            //cout << language << endl;
            tables->fprint_disable[i] = mask;
            //continue;
        }
    }
}

void SimpleGuesser::EnableLanguage(string lang){
    XableLanguage(lang,  sal::static_int_cast< char >( 0xF0 ));
}

void SimpleGuesser::DisableLanguage(string lang){
    XableLanguage(lang,  sal::static_int_cast< char >( 0x0F ));
}

/**
*
*/
void SimpleGuesser::SetDBPath(const char* path, const char* prefix){
    if(h){
        textcat_Done(h);
    }
    h = special_textcat_Init(path, prefix);
}
