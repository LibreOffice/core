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
#ifndef INCLUDED_LINGUCOMPONENT_SOURCE_LANGUAGEGUESSING_SIMPLEGUESSER_HXX
#define INCLUDED_LINGUCOMPONENT_SOURCE_LANGUAGEGUESSING_SIMPLEGUESSER_HXX

#include <string.h>
#include <string>
#include <cstdlib>
#include <vector>
#include "guess.hxx"

#define MAX_STRING_LENGTH_TO_ANALYSE 200

using namespace std;

class SimpleGuesser final
{
public:
    /**inits the object with conf file "./conf.txt"*/
    SimpleGuesser();

    /**
     * @param SimpleGuesser& sg the other guesser
     */
    SimpleGuesser& operator=(const SimpleGuesser& sg);

    /**
     * destroy the object
     */
    ~SimpleGuesser();

    /**
     * Analyze a text and return the most probable languages of the text
     * @param char* text is the text to analyze
     * @return the list of guess
     */
    vector<Guess> GuessLanguage(const char* text);

    /**
     * Analyze a text and return the most probable language of the text
     * @param char* text is the text to analyze
     * @return the guess (containing language)
     */
    Guess GuessPrimaryLanguage(const char* text);

    /**
     * List all available languages (possibly to be in guesses)
     * @return the list of languages
     */
    vector<Guess> GetAvailableLanguages();

    /**
     * List all languages (possibly in guesses or not)
     * @return the list of languages
     */
    vector<Guess> GetAllManagedLanguages();

    /**
     * List all Unavailable languages (disable for any reason)
     * @return the list of languages
     */
    vector<Guess> GetUnavailableLanguages();

    /**
     * Mark a language enabled
     * @param string lang the language to enable (build like language-COUNTRY-encoding)
     */
    void EnableLanguage(const string& lang);

    /**
     * Mark a language disabled
     * @param string lang the language to disable (build like language-COUNTRY-encoding)
     */
    void DisableLanguage(const string& lang);

    /**
     * Load a new DB of fingerprints
     * @param const char* thePathOfConfFile self explaining
     * @param const char* prefix is the path where the directory which contains fingerprint files is stored
    */
    void SetDBPath(const char* thePathOfConfFile, const char* prefix);

private:
    //Where typical fingerprints (n-gram tables) are stored
    void* h;

    //Is used to select languages into the fingerprints DB, the mask is used to indicate if we want enabled disabled or both
    vector<Guess> GetManagedLanguages(const char mask);

    //Like getManagedLanguages, this function enable or disable a language and it depends of the mask
    void XableLanguage(const string& lang, char mask);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
