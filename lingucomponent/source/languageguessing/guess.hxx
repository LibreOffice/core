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
#ifndef INCLUDED_LINGUCOMPONENT_SOURCE_LANGUAGEGUESSING_GUESS_HXX
#define INCLUDED_LINGUCOMPONENT_SOURCE_LANGUAGEGUESSING_GUESS_HXX

#define GUESS_SEPARATOR_OPEN     '['
#define GUESS_SEPARATOR_CLOSE    ']'
#define GUESS_SEPARATOR_SEP      '-'
#define DEFAULT_LANGUAGE         ""
#define DEFAULT_COUNTRY          ""
#define DEFAULT_ENCODING         ""

#include <string>

using namespace std;

class Guess final {
    public:

        /**
         * Default init
         */
        Guess();

        /**
         * Init from a string like [en-UK-utf8] and the rank
         */
        Guess(const char * guess_str);

        const string& GetLanguage() { return language_str;}
        const string& GetCountry() { return country_str;}

    private:
        string language_str;
        string country_str;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
