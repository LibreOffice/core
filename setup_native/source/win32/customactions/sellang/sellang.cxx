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

/* Currently the "all" installer has a bit over 100 UI languages, and
 * I doubt it will grow a lot over that.
 */
#define MAX_LANGUAGES 200

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>
#include <malloc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sal/macros.h>
#include <systools/win32/uwinapi.h>
#include <algorithm>

#include "spellchecker_selection.hxx"

BOOL GetMsiProp( MSIHANDLE hMSI, const char* pPropName, char** ppValue )
{
    DWORD sz = 0;
    if ( MsiGetProperty( hMSI, pPropName, const_cast<char *>(""), &sz ) == ERROR_MORE_DATA ) {
        sz++;
        DWORD nbytes = sz * sizeof( char );
        char* buff = reinterpret_cast<char*>( malloc( nbytes ) );
        ZeroMemory( buff, nbytes );
        MsiGetProperty( hMSI, pPropName, buff, &sz );
        *ppValue = buff;
        return ( strlen(buff) > 0 );
    }
    return FALSE;
}

static const char *
langid_to_string( LANGID langid )
{
    /* Map from LANGID to string. The languages below are now in
     * alphabetical order of codes as in
     * l10ntools/source/ulfconv/msi-encodinglist.txt. Only the
     * language part is returned in the string.
     */
    switch (PRIMARYLANGID (langid)) {
    case LANG_AFRIKAANS: return "af";
    case LANG_AMHARIC: return "am";
    case LANG_ARABIC: return "ar";
    case LANG_ASSAMESE: return "as";
    case LANG_BELARUSIAN: return "be";
    case LANG_BULGARIAN: return "bg";
    case LANG_BENGALI: return "bn";
    case LANG_BRETON: return "br";
    case LANG_CATALAN: return "ca";
    case LANG_CZECH: return "cs";
    case LANG_WELSH: return "cy";
    case LANG_DANISH: return "da";
    case LANG_GERMAN: return "de";
    case LANG_GREEK: return "el";
    case LANG_ENGLISH: return "en";
    case LANG_SPANISH: return "es";
    case LANG_ESTONIAN: return "et";
    case LANG_BASQUE: return "eu";
    case LANG_FARSI: return "fa";
    case LANG_FINNISH: return "fi";
    case LANG_FAEROESE: return "fo";
    case LANG_FRENCH: return "fr";
    case LANG_IRISH: return "ga";
    case LANG_GALICIAN: return "gl";
    case LANG_GUJARATI: return "gu";
    case LANG_HEBREW: return "he";
    case LANG_HINDI: return "hi";
    case LANG_HUNGARIAN: return "hu";
    case LANG_ARMENIAN: return "hy";
    case LANG_INDONESIAN: return "id";
    case LANG_ICELANDIC: return "is";
    case LANG_ITALIAN: return "it";
    case LANG_JAPANESE: return "ja";
    case LANG_GEORGIAN: return "ka";
    case LANG_KHMER: return "km";
    case LANG_KANNADA: return "kn";
    case LANG_KOREAN: return "ko";
    case LANG_KASHMIRI: return "ks";
    case LANG_LAO: return "lo";
    case LANG_LITHUANIAN: return "lt";
    case LANG_LATVIAN: return "lv";
    case LANG_MACEDONIAN: return "mk";
    case LANG_MALAYALAM: return "ml";
    case LANG_MONGOLIAN: return "mn";
    case LANG_MARATHI: return "mr";
    case LANG_MALAY: return "ms";
    case LANG_MALTESE: return "mt";
    case LANG_NEPALI: return "ne";
    case LANG_DUTCH: return "nl";
    case LANG_SOTHO: return "ns";
    case LANG_ORIYA: return "or";
    case LANG_PUNJABI: return "pa";
    case LANG_POLISH: return "pl";
    case LANG_PORTUGUESE: return "pt";
    case LANG_ROMANSH: return "rm";
    case LANG_ROMANIAN: return "ro";
    case LANG_RUSSIAN: return "ru";
    case LANG_KINYARWANDA: return "rw";
    case LANG_SANSKRIT: return "sa";
    case LANG_UPPER_SORBIAN: return "sb";
    case LANG_SINDHI: return "sd";
    case LANG_SLOVAK: return "sk";
    case LANG_SLOVENIAN: return "sl";
    case LANG_ALBANIAN: return "sq";
    case LANG_SWEDISH: return "sv";
    case LANG_SWAHILI: return "sw";
    case LANG_TAMIL: return "ta";
    case LANG_TELUGU: return "te";
    case LANG_TAJIK: return "tg";
    case LANG_THAI: return "th";
    case LANG_TIGRIGNA: return "ti";
    case LANG_TSWANA: return "tn";
    case LANG_TURKISH: return "tr";
    case LANG_TATAR: return "tt";
    case LANG_UKRAINIAN: return "uk";
    case LANG_URDU: return "ur";
    case LANG_UZBEK: return "uz";
    case LANG_VIETNAMESE: return "vi";
    case LANG_XHOSA: return "xh";
    case LANG_CHINESE: return "zh";
    case LANG_ZULU: return "zu";
    /* Special cases */
    default:
        switch (langid) {
        case MAKELANGID(LANG_SERBIAN, 0x05): return "bs";
        case MAKELANGID(LANG_SERBIAN, SUBLANG_DEFAULT): return "hr";
        case MAKELANGID(LANG_NORWEGIAN, SUBLANG_NORWEGIAN_BOKMAL): return "nb";
        case MAKELANGID(LANG_NORWEGIAN, SUBLANG_NORWEGIAN_NYNORSK): return "nn";
        case MAKELANGID(LANG_SERBIAN, SUBLANG_SERBIAN_LATIN): return "sh";
        case MAKELANGID(LANG_SERBIAN, SUBLANG_SERBIAN_CYRILLIC): return "sr";
        default: return 0;
        }
    }
}

/* Here we collect the UI languages present on the system;
 * MAX_LANGUAGES is certainly enough for that
 */
static const char *ui_langs[MAX_LANGUAGES];
static int num_ui_langs = 0;

void add_ui_lang(char const * lang)
{
    if (lang != 0 && num_ui_langs != SAL_N_ELEMENTS(ui_langs)) {
        ui_langs[num_ui_langs++] = lang;
    }
}

BOOL CALLBACK
enum_ui_lang_proc (LPTSTR language, LONG_PTR /* unused_lParam */)
{
    long langid = strtol(language, NULL, 16);
    if (langid > 0xFFFF)
        return TRUE;
    add_ui_lang(langid_to_string((LANGID) langid));
    if (num_ui_langs == SAL_N_ELEMENTS(ui_langs) )
        return FALSE;
    return TRUE;
}

static BOOL
present_in_ui_langs(const char *lang)
{
    for (int i = 0; i < num_ui_langs; i++)
        if (memcmp (ui_langs[i], lang, std::min(strlen(ui_langs[i]), strlen(lang))) == 0)
            return TRUE;
    return FALSE;
}

namespace {

/* TODO-BCP47: unlimit this, and if possible change from '_' to '-' separator
 * to ease things. */
// XXX NOTE: the sizeof needs to follow what is accepted in
// setup_native/source/packinfo/spellchecker_selection.pl
struct InstallLocalized {
    char lang[sizeof("lll_Ssss_CC_vvvvvvvv")];
    bool install;
};

void addMatchingDictionaries(
    char const * lang, InstallLocalized * dicts, int ndicts)
{
    for (int i = 0; i != SAL_N_ELEMENTS(setup_native::languageDictionaries);
         ++i)
    {
        if (strcmp(lang, setup_native::languageDictionaries[i].language) == 0) {
            for (char const * const * p = setup_native::languageDictionaries[i].
                     dictionaries;
                 *p != NULL; ++p)
            {
                for (int j = 0; j != ndicts; ++j) {
                    if (_stricmp(*p, dicts[j].lang) == 0) {
                        dicts[j].install = true;
                        break;
                    }
                }
            }
            break;
        }
    }
}

}

extern "C" UINT __stdcall SelectLanguage( MSIHANDLE handle )
{
    char feature[100];
    MSIHANDLE database, view, record;
    DWORD length;
    int nlangs = 0;
    InstallLocalized langs[MAX_LANGUAGES];
    int ndicts = 0;
    InstallLocalized dicts[MAX_LANGUAGES];

    database = MsiGetActiveDatabase(handle);

    if (MsiDatabaseOpenViewA(database, "SELECT Feature from Feature WHERE Feature_Parent = 'gm_Langpack_Languageroot'", &view) != ERROR_SUCCESS) {
        MsiCloseHandle(database);
        return ERROR_SUCCESS;
    }

    if (MsiViewExecute(view, NULL) != ERROR_SUCCESS) {
        MsiCloseHandle(view);
        MsiCloseHandle(database);
        return ERROR_SUCCESS;
    }

    while (nlangs < MAX_LANGUAGES &&
           MsiViewFetch(view, &record) == ERROR_SUCCESS) {
        length = sizeof(feature);
        if (MsiRecordGetStringA(record, 1, feature, &length) != ERROR_SUCCESS) {
            MsiCloseHandle(record);
            MsiCloseHandle(view);
            MsiCloseHandle(database);
            return ERROR_SUCCESS;
        }

        /* Keep track of what langpacks are included in this installer.
         */
        strcpy(langs[nlangs].lang, feature + strlen("gm_Langpack_r_"));
        langs[nlangs].install = false;
        ++nlangs;

        MsiCloseHandle(record);
    }

    MsiCloseHandle(view);

    /* Keep track of what dictionaries are included in this installer:
     */
    if (MsiDatabaseOpenViewA(
            database,
            ("SELECT Feature from Feature WHERE"
             " Feature_Parent = 'gm_Dictionaries'"),
            &view)
        == ERROR_SUCCESS)
    {
        if (MsiViewExecute(view, NULL) == ERROR_SUCCESS) {
            while (ndicts < MAX_LANGUAGES &&
                   MsiViewFetch(view, &record) == ERROR_SUCCESS)
            {
                length = sizeof(feature);
                if (MsiRecordGetStringA(record, 1, feature, &length)
                    == ERROR_SUCCESS)
                {
                    if (strncmp(
                            feature, "gm_r_ex_Dictionary_",
                            strlen("gm_r_ex_Dictionary_"))
                        == 0)
                    {
                        strcpy(
                            dicts[ndicts].lang,
                            feature + strlen("gm_r_ex_Dictionary_"));
                        dicts[ndicts].install = false;
                        ++ndicts;
                    }
                }
                MsiCloseHandle(record);
            }
        }
        MsiCloseHandle(view);
    }

    /* Keep track of what UI languages are relevant, either the ones explicitly
     * requested with the UI_LANGS property, or all available on the system:
     */
    char* pVal = NULL;
    if ( (GetMsiProp( handle, "UI_LANGS", &pVal )) && pVal ) {
        char *str_ptr;
        str_ptr = strtok(pVal, ",");
        for(; str_ptr != NULL ;) {
            add_ui_lang(str_ptr);
            str_ptr = strtok(NULL, ",");
        }
    } else {
        add_ui_lang(langid_to_string(GetSystemDefaultUILanguage()));
        add_ui_lang(langid_to_string(LANGIDFROMLCID(GetThreadLocale())));
            //TODO: are the above two explicit additions necessary, or will
            // those values always be included in the below EnumUILanguages
            // anyway?
        EnumUILanguagesA(enum_ui_lang_proc, 0, 0);
    }

    // If the set of langpacks that match any of the relevant UI languages is
    // non-empty, select just those matching langpacks; otherwise, if an en_US
    // langpack is included, select just that langpack (this happens if, e.g.,
    // a multi-language en-US,de,es,fr,it,pt-BR installation set is installed on
    // a Finnish Windows); otherwise, select all langpacks (this happens if,
    // e.g., a single-language de installation set is installed on a Finnish
    // Windows):
    bool matches = false;
    for (int i = 0; i < nlangs; i++) {
        if (present_in_ui_langs(langs[i].lang)) {
            langs[i].install = true;
            matches = true;
        }
    }
    if (!matches) {
        for (int i = 0; i < nlangs; i++) {
            if (strcmp(langs[nlangs].lang, "en_US") == 0) {
                langs[i].install = true;
                matches = true;
                break;
            }
        }
        if (!matches) {
            for (int i = 0; i < nlangs; i++) {
                langs[i].install = true;
            }
        }
    }

    for (int i = 0; i < nlangs; i++) {
        if (langs[i].install) {
            addMatchingDictionaries(langs[i].lang, dicts, ndicts);
        } else {
            sprintf(feature, "gm_Langpack_r_%s", langs[i].lang);
            MsiSetFeatureStateA(handle, feature, INSTALLSTATE_ABSENT);
        }
    }

    // Select just those dictionaries that match any of the selected langpacks:
    for (int i = 0; i != ndicts; ++i) {
        if (!dicts[i].install) {
            sprintf(feature, "gm_r_ex_Dictionary_%s", dicts[i].lang);
            MsiSetFeatureStateA(handle, feature, INSTALLSTATE_ABSENT);
        }
    }

    MsiCloseHandle(database);

    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
