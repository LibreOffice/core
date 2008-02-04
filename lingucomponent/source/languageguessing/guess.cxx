/***************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: guess.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-04 13:44:39 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_lingucomponent.hxx"

#include <iostream>
#include <string.h>

#include <libtextcat/textcat.h>
#include <altstrfunc.hxx>
#include <guess.hxx>

using namespace std;

Guess::Guess()
{
    language_str = DEFAULT_LANGUAGE;
    country_str = DEFAULT_COUNTRY;
    encoding_str = DEFAULT_ENCODING;
    order = 0;
}

Guess::Guess(string& lang, string& country, string& enc, int p_order)
{
    Guess();
    language_str=lang;
    country_str=country;
    encoding_str=enc;
    order = p_order;
}

/*
* this use a char * string to build the guess object
* a string like those is made as : [language-country-encoding]...
*
*/

Guess::Guess(char * guess_str, int p_order)
{
    Guess();
    order = p_order;

    string lang;
    string country;
    string enc;

    //if the guess is not like "UNKNOWN" or "SHORT", go into the brackets
//     if(strncmp((const char*)(guess_str + 1), _TEXTCAT_RESULT_UNKOWN, strlen(_TEXTCAT_RESULT_UNKOWN)) != 0
//        &&
//        strncmp((const char*)(guess_str + 1), _TEXTCAT_RESULT_SHORT, strlen(_TEXTCAT_RESULT_SHORT)) != 0)
//     {
        if(strcmp((const char*)(guess_str + 1), _TEXTCAT_RESULT_UNKOWN) != 0
           &&
           strcmp((const char*)(guess_str + 1), _TEXTCAT_RESULT_SHORT) != 0)
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

        if(/*country!=""*/1){
            country_str=country;
        }
        else{//if we don't have an availlable country, we use the language as a country
            country_str=upperCase(language_str);
        }

        if(enc!=""){//if not we use the default value
            encoding_str=enc;
        }
    }
}

Guess::~Guess(){}

string Guess::GetLanguage()
{
    return language_str;
}

string Guess::GetCountry()
{
    return country_str;
}

string Guess::GetEncoding()
{
    return encoding_str;
}

int Guess::GetOrder()
{
    return order;
}

bool Guess::operator==(string lang)
{
    string toString;
    toString += GetLanguage();
    toString += "-";
    toString += GetCountry();
    toString += "-";
    toString += GetEncoding();
    return start(toString, lang);
}
