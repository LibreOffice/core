/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 3.
 *
 *
 *    GNU Lesser General Public License Version 3
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

/* Currently the "all" installer has a bit over 100 UI languages, and
 * I doubt it will grow a lot over that.
 */
#define MAX_LANGUAGES 200

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0500
#undef WINVER
#define WINVER 0x0500

#include <windows.h>
#include <msiquery.h>
#include <malloc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sal/macros.h>
#include <systools/win32/uwinapi.h>

#include "spellchecker_selection.hxx"

BOOL GetMsiProp( MSIHANDLE hMSI, const char* pPropName, char** ppValue )
{
    DWORD sz = 0;
    if ( MsiGetProperty( hMSI, pPropName, "", &sz ) == ERROR_MORE_DATA ) {
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
#define CASE(name, primary) \
        case LANG_##primary: return #name
    CASE(af, AFRIKAANS);
    CASE(am, AMHARIC);
    CASE(ar, ARABIC);
    CASE(as, ASSAMESE);
    CASE(be, BELARUSIAN);
    CASE(bg, BULGARIAN);
    CASE(bn, BENGALI);
    CASE(br, BRETON);
    CASE(ca, CATALAN);
    CASE(cs, CZECH);
    CASE(cy, WELSH);
    CASE(da, DANISH);
    CASE(de, GERMAN);
    CASE(el, GREEK);
    CASE(en, ENGLISH);
    CASE(es, SPANISH);
    CASE(et, ESTONIAN);
    CASE(eu, BASQUE);
    CASE(fa, FARSI);
    CASE(fi, FINNISH);
    CASE(fo, FAEROESE);
    CASE(fr, FRENCH);
    CASE(ga, IRISH);
    CASE(gl, GALICIAN);
    CASE(gu, GUJARATI);
    CASE(he, HEBREW);
    CASE(hi, HINDI);
    CASE(hu, HUNGARIAN);
    CASE(hy, ARMENIAN);
    CASE(id, INDONESIAN);
    CASE(is, ICELANDIC);
    CASE(it, ITALIAN);
    CASE(ja, JAPANESE);
    CASE(ka, GEORGIAN);
    CASE(km, KHMER);
    CASE(kn, KANNADA);
    CASE(ko, KOREAN);
    CASE(ks, KASHMIRI);
    CASE(lo, LAO);
    CASE(lt, LITHUANIAN);
    CASE(lv, LATVIAN);
    CASE(mk, MACEDONIAN);
    CASE(ml, MALAYALAM);
    CASE(mn, MONGOLIAN);
    CASE(mr, MARATHI);
    CASE(ms, MALAY);
    CASE(mt, MALTESE);
    CASE(ne, NEPALI);
    CASE(nl, DUTCH);
    CASE(ns, SOTHO);
    CASE(or, ORIYA);
    CASE(pa, PUNJABI);
    CASE(pl, POLISH);
    CASE(pt, PORTUGUESE);
    CASE(rm, ROMANSH);
    CASE(ro, ROMANIAN);
    CASE(ru, RUSSIAN);
    CASE(rw, KINYARWANDA);
    CASE(sa, SANSKRIT);
    CASE(sb, UPPER_SORBIAN);
    CASE(sd, SINDHI);
    CASE(sk, SLOVAK);
    CASE(sl, SLOVENIAN);
    CASE(sq, ALBANIAN);
    CASE(sv, SWEDISH);
    CASE(sw, SWAHILI);
    CASE(ta, TAMIL);
    CASE(te, TELUGU);
    CASE(tg, TAJIK);
    CASE(th, THAI);
    CASE(ti, TIGRIGNA);
    CASE(tn, TSWANA);
    CASE(tr, TURKISH);
    CASE(tt, TATAR);
    CASE(uk, UKRAINIAN);
    CASE(ur, URDU);
    CASE(uz, UZBEK);
    CASE(vi, VIETNAMESE);
    CASE(xh, XHOSA);
    CASE(zh, CHINESE);
    CASE(zu, ZULU);
#undef CASE
    /* Special cases */
    default:
        switch (langid) {
        case MAKELANGID(LANG_SERBIAN, 0x05): return "bs";
#define CASE(name, primary, sub) \
        case MAKELANGID(LANG_##primary, SUBLANG_##sub): return #name

        CASE(hr, SERBIAN, DEFAULT);
        CASE(nb, NORWEGIAN, NORWEGIAN_BOKMAL);
        CASE(nn, NORWEGIAN, NORWEGIAN_NYNORSK);
        CASE(sh, SERBIAN, SERBIAN_LATIN);
        CASE(sr, SERBIAN, SERBIAN_CYRILLIC);
#undef CASE
        default: return "";
        }
    }
}

/* Here we collect the UI languages present on the system;
 * MAX_LANGUAGES is certainly enough for that
 */
static const char *ui_langs[MAX_LANGUAGES];
static int num_ui_langs = 0;

BOOL CALLBACK
enum_ui_lang_proc (LPTSTR language, LONG_PTR /* unused_lParam */)
{
    long langid = strtol(language, NULL, 16);
    if (langid > 0xFFFF)
        return TRUE;
    ui_langs[num_ui_langs] = langid_to_string((LANGID) langid);
    num_ui_langs++;
    if (num_ui_langs == SAL_N_ELEMENTS(ui_langs) )
        return FALSE;
    return TRUE;
}

static BOOL
present_in_ui_langs(const char *lang)
{
    for (int i = 0; i < num_ui_langs; i++)
        if (memcmp (ui_langs[i], lang, ( strlen(ui_langs[i]) >= strlen(lang) ) ? strlen(lang) : strlen(ui_langs[i]) ) == 0)
            return TRUE;
    return FALSE;
}

namespace {

struct Dictionary {
    char lang[sizeof("xx_XX")];
    bool install;
};

void addMatchingDictionaries(char const * lang, Dictionary * dicts, int ndicts)
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
    char langs[MAX_LANGUAGES][6];
    int ndicts = 0;
    Dictionary dicts[MAX_LANGUAGES];

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

        /* Keep track of what languages are included in this installer, if
         * it is a multilanguage one.
         */
        if (strcmp(feature, "gm_Langpack_r_en_US") != 0)
            strcpy(langs[nlangs++], feature + strlen("gm_Langpack_r_"));

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

    if (nlangs > 0) {
        int i;
        char* pVal = NULL;
        if ( (GetMsiProp( handle, "UI_LANGS", &pVal )) && pVal ) {
            /* user gave UI languages explicitely with UI_LANGS property */
            int sel_ui_lang = 0;
            strcpy(langs[nlangs++], "en_US");
            char *str_ptr;
            str_ptr = strtok(pVal, ",");
            for(; str_ptr != NULL ;) {
                ui_langs[num_ui_langs] = str_ptr;
                num_ui_langs++;
                str_ptr = strtok(NULL, ",");
            }
            for (i = 0; i < nlangs; i++) {
                if (!present_in_ui_langs(langs[i])) {
                    UINT rc;
                    sprintf(feature, "gm_Langpack_r_%s", langs[i]);
                    rc = MsiSetFeatureStateA(handle, feature, INSTALLSTATE_ABSENT);
                }
                else {
                    addMatchingDictionaries(langs[i], dicts, ndicts);
                    sel_ui_lang++;
                }
            }
            if ( sel_ui_lang == 0 ) {
                /* When UI_LANG property contains only languages that are not present
                 * in the installer, install at least en_US localization.
                 */
                MsiSetFeatureStateA(handle, "gm_Langpack_r_en_US", INSTALLSTATE_LOCAL);
                addMatchingDictionaries("en_US", dicts, ndicts);
            }
        }
        else {
            /* Deselect those languages that don't match any of the UI languages
             * available on the system.
             */

            const char *system_default_lang = langid_to_string(GetSystemDefaultUILanguage());
            const char *user_locale_lang = langid_to_string(LANGIDFROMLCID(GetThreadLocale()));

            EnumUILanguagesA(enum_ui_lang_proc, 0, 0);

            /* If one of the alternative languages in a multi-language installer
             * is the system default UI language, deselect those languages that
             * aren't among the UI languages available on the system.
             * (On most Windows installations, just one UI language is present,
             * which obviously is the same as the default UI language. But
             * we want to be generic.)
             * If none of the languages in a multi-language installer is the
             * system default UI language (this happens now in 2.4.0 where we
             * cannot put as many UI languages into the installer as we would
             * like, but only half a dozen: en-US,de,es,fr,it,pt-BR), pretend
             * that English is the system default UI language,
             * so that we will by default deselect everything except
             * English. We don't want to by default install all half dozen
             * languages for an unsuspecting user of a Finnish Windows, for
             * instance. Sigh.
             */
            bool have_system_default_lang = false;
            if (system_default_lang[0]) {
                for (i = 0; i < nlangs; i++) {
                    if (memcmp (system_default_lang, langs[i], 2) == 0) {
                        have_system_default_lang = true;
                        break;
                    }
                }
            }
            if (!have_system_default_lang) {
                system_default_lang = "en";
            }

            for (i = 0; i < nlangs; i++) {
                if (memcmp(system_default_lang, langs[i], 2) != 0 &&
                    memcmp(user_locale_lang, langs[i], 2) != 0 &&
                    !present_in_ui_langs(langs[i])) {
                    UINT rc;
                    sprintf(feature, "gm_Langpack_r_%s", langs[i]);
                    rc = MsiSetFeatureStateA(handle, feature, INSTALLSTATE_ABSENT);
                } else {
                    addMatchingDictionaries(langs[i], dicts, ndicts);
                }
            }
        }
    }

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
