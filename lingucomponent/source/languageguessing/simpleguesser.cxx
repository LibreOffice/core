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

#ifdef SYSTEM_LIBEXTTEXTCAT
#include <libexttextcat/textcat.h>
#else
#include <textcat.h>
#endif

#include <sal/types.h>

#include<rtl/character.hxx>
#include "simpleguesser.hxx"

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
std::vector<Guess> SimpleGuesser::GuessLanguage(const char* text)
{
    std::vector<Guess> guesses;

    if (!h)
        return guesses;

    int len = strlen(text);

    if (len > MAX_STRING_LENGTH_TO_ANALYSE)
        len = MAX_STRING_LENGTH_TO_ANALYSE;

    const char *guess_list = textcat_Classify(h, text, len);

    if (strcmp(guess_list, TEXTCAT_RESULT_SHORT_STR) == 0)
        return guesses;

    int current_pointer = 0;

    while(guess_list[current_pointer] != '\0')
    {
        while (guess_list[current_pointer] != GUESS_SEPARATOR_OPEN && guess_list[current_pointer] != '\0')
            current_pointer++;
        if(guess_list[current_pointer] != '\0')
        {
            guesses.emplace_back(guess_list + current_pointer);
            current_pointer++;
        }
    }

    return guesses;
}

Guess SimpleGuesser::GuessPrimaryLanguage(const char* text)
{
    std::vector<Guess> ret = GuessLanguage(text);
    return ret.empty() ? Guess() : ret[0];
}
/**
 * Is used to know which language is available, unavailable or both
 * when mask = 0xF0, return only Available
 * when mask = 0x0F, return only Unavailable
 * when mask = 0xFF, return both Available and Unavailable
 */
std::vector<Guess> SimpleGuesser::GetManagedLanguages(LanguageState eState)
{
    std::vector<Guess> lang;
    if(!h){return lang;}

    const uint4 nCount = textcat_GetLanguageCount(h);
    for (uint4 i = 0; i < nCount; ++i)
    {
        const bool bEnabled = textcat_IsLanguageEnabled(h, i) == TC_TRUE;
        if (eState == LanguageState::Any || bEnabled == (eState == LanguageState::Enabled))
        {
            std::string langStr = "[";
            langStr += textcat_GetLanguageName(h, i);
            lang.emplace_back(langStr.c_str());
        }
    }

    return lang;
}

std::vector<Guess> SimpleGuesser::GetAvailableLanguages()
{
    return GetManagedLanguages(LanguageState::Enabled);
}

std::vector<Guess> SimpleGuesser::GetUnavailableLanguages()
{
    return GetManagedLanguages(LanguageState::Disabled);
}

std::vector<Guess> SimpleGuesser::GetAllManagedLanguages()
{
    return GetManagedLanguages(LanguageState::Any);
}

void SimpleGuesser::SetLanguageEnabled(const std::string& lang, bool bEnabled)
{
    if(!h){return;}

    const uint4 nCount = textcat_GetLanguageCount(h);
    for (uint4 i = 0; i < nCount; ++i)
    {
        std::string language(textcat_GetLanguageName(h, i));
        if (startsAsciiCaseInsensitive(language,lang) == 0)
            textcat_SetLanguageEnabled(h, i, bEnabled ? TC_TRUE : TC_FALSE);
    }
}

void SimpleGuesser::EnableLanguage(const std::string& lang)
{
    SetLanguageEnabled(lang, true);
}

void SimpleGuesser::DisableLanguage(const std::string& lang)
{
    SetLanguageEnabled(lang, false);
}

void SimpleGuesser::SetDBPath(const char* path, const char* prefix)
{
    if (h)
        textcat_Done(h);
    h = special_textcat_Init(path, prefix);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
