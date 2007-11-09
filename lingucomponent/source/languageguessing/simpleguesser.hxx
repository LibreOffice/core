/***************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: simpleguesser.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-09 10:53:46 $
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
#ifndef SIMPLEGUESSER_H
#define SIMPLEGUESSER_H

#include <string.h>
#include <string>
#include <cstdlib>
#include <vector>
#include <guess.hxx>

#define MAX_STRING_LENGTH_TO_ANALYSE 200

using namespace std;

/**
@author Jocelyn Merand
*/
class SimpleGuesser{
public:
    /**inits the object with conf file "./conf.txt"*/
    SimpleGuesser();

    /** inits the object with conFile config file
     * @param const char* confFile the string representing the config file
     * @param const char* prefix the path where fingerprints files are stored
     */
    SimpleGuesser(const char* confFile, const char* prefix);

    /** Compares the current Simpleguesser with an other
     * @param SimpleGuesser& sg the other guesser to compare
     */
    void operator=(SimpleGuesser& sg);

    /**
     * destroy the object
     */
    ~SimpleGuesser();

    /**
     * Analyze a text and return the most probable languages of the text
     * @param char* text is the text to analyze
     * @return the list of guess
     */
    vector<Guess> GuessLanguage(char* text);

    /**
     * Analyze a text and return the most probable language of the text
     * @param char* text is the text to analyze
     * @return the guess (containing language)
     */
    Guess GuessPrimaryLanguage(char* text);

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
    void EnableLanguage(string lang);

    /**
     * Mark a language disabled
     * @param string lang the language to disable (build like language-COUNTRY-encoding)
     */
    void DisableLanguage(string lang);

    /**
     * Load a new DB of fingerprints
     * @param const char* thePathOfConfFile self explaining
     * @param const char* prefix is the path where the directory witch contains fingerprint files is stored
    */
    void SetDBPath(const char* thePathOfConfFile, const char* prefix);

protected:

    //Where typical fingerprints (n-gram tables) are stored
    void* h;

    //Is used to select languages into the fingerprints DB, the mask is used to indicate if we want enabled disabled or both
    vector<Guess> GetManagedLanguages(const char mask);

    //Like getManagedLanguages, this function enable or disable a language and it depends of the mask
    void XableLanguage(string lang, char mask);
};

#endif
