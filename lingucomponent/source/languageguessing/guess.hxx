/***************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: guess.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2007-06-22 08:32:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
         * Init from 3 strings (language, country and encoding) and 1 int witch represents the rank
         */
        Guess(string& lang, string& country, string& enc, int order);

        /**
         * Init from a string like [en-UK-utf8] and the rank
         */
        Guess(char * guess_str, int order);

        ~Guess();

        string GetLanguage();
        string GetCountry();
        string GetEncoding();
        int GetOrder();

        bool operator==(string lang);

    protected:
        string language_str;
        string country_str;
        string encoding_str;
        int order;
};

#endif
