/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
                Guess g((char*)(guess_list + current_pointer));

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
 * Is used to know which language is available, unavailable or both
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
            Guess g( (char *)langStr.c_str());
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
