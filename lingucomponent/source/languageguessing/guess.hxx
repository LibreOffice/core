/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/***************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef GUESS_H
#define GUESS_H

#define GUESS_SEPARATOR_OPEN     '['
#define GUESS_SEPARATOR_CLOSE    ']'
#define GUESS_SEPARATOR_SEP      '-'
#define DEFAULT_LANGUAGE         ""
#define DEFAULT_COUNTRY          ""
#define DEFAULT_ENCODING         ""

#include <string>

using namespace std;

/**
@author Jocelyn Merand
 */
class Guess{
    public:

        /**
         * Default init
         */
        Guess();

        /**
         * Init from a string like [en-UK-utf8] and the rank
         */
        Guess(char * guess_str);

        ~Guess();

        string GetLanguage();
        string GetCountry();
        string GetEncoding();

        bool operator==(string lang);

    protected:
        string language_str;
        string country_str;
        string encoding_str;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
