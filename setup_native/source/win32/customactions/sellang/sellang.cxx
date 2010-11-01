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

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0500
#undef WINVER
#define WINVER 0x0500

#pragma warning(push, 1) /* disable warnings within system headers as
                          * warnings are now treated as errors...
                          */
#include <windows.h>
#include <msiquery.h>
#include <malloc.h>

#ifdef UNICODE
#define _UNICODE
#define _tstring    wstring
#else
#define _tstring    string
#endif
#include <tchar.h>
#include <string>
#include <stdio.h>
#pragma warning(pop)

#include <sal/macros.h>
#include <systools/win32/uwinapi.h>

/* #define VERBOSE_DEBUG_OUTPUT 1 */

static const char *
langid_to_string( LANGID langid, int *have_default_lang )
{
    /* Map from LANGID to string. The languages below are in the same
     * seemingly random order as in
     * setup_native/source/win32/msi-encodinglist.txt.
     * Only the language part is returned in the string.
     */
    switch (PRIMARYLANGID (langid)) {
    case LANG_ENGLISH:
        if (have_default_lang != NULL &&
            langid == MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT))
            *have_default_lang = 1;
        return "en";
#define CASE(primary, name) \
        case LANG_##primary: return #name
    CASE(PORTUGUESE, pt);
    CASE(RUSSIAN, ru);
    CASE(GREEK, el);
    CASE(DUTCH, nl);
    CASE(FRENCH, fr);
    CASE(SPANISH, es);
    CASE(FINNISH, fi);
    CASE(HUNGARIAN, hu);
    CASE(CATALAN, ca);
    CASE(ITALIAN, it);
    CASE(CZECH, cs);
    CASE(SLOVAK, sk);
    CASE(DANISH, da);
    CASE(SWEDISH, sv);
    CASE(POLISH, pl);
    CASE(GERMAN, de);
    CASE(THAI, th);
    CASE(ESTONIAN, et);
    CASE(JAPANESE, ja);
    CASE(KOREAN, ko);
    // CASE(KHMER, km);
    // CASE(WELSH, cy);
    CASE(CHINESE, zh);
    CASE(TURKISH, tr);
    CASE(HINDI, hi);
    CASE(PUNJABI, pa);
    CASE(TAMIL, ta);
    CASE(ARABIC, ar);
    CASE(HEBREW, he);
    CASE(AFRIKAANS, af);
    CASE(ALBANIAN, sq);
    CASE(ARMENIAN, hy);
    CASE(BASQUE, eu);
    CASE(BELARUSIAN, be);
    CASE(BENGALI, bn);
    CASE(BULGARIAN, bg);
    CASE(ICELANDIC, is);
    CASE(INDONESIAN, id);
    // CASE(LAO, lo);
    CASE(LATVIAN, lv);
    CASE(LITHUANIAN, lt);
    // CASE(MALTESE, mt);
    // CASE(ROMANSH, rm);
    CASE(ROMANIAN, ro);
    // CASE(KINYARWANDA, rw);
    CASE(SANSKRIT, sa);
    // CASE(SETSWANA, tn);
    CASE(FARSI, fa);
    CASE(FAEROESE, fo);
    CASE(SLOVENIAN, sl);
    // CASE(SORBIAN, sb);
    // CASE(SUTU, st);
    CASE(SWAHILI, sw);
    CASE(TATAR, tt);
    // CASE(TSONGA, ts);
    CASE(UKRAINIAN, uk);
    CASE(URDU, ur);
    CASE(VIETNAMESE, vi);
    // CASE(XHOSA, xh);
    // CASE(YIDDISH, yi);
    // CASE(ZULU, zu);
    CASE(GUJARATI, gu);
    // CASE(BRETON, br);
    CASE(NEPALI, ne);
    // CASE(NDEBELE, nr);
    // CASE(SWAZI, ss);
    // CASE(VENDA, ve);
    // CASE(IRISH, ga);
    CASE(MACEDONIAN, mk);
    CASE(TELUGU, te);
    CASE(MALAYALAM, ml);
    CASE(MARATHI, mr);
    CASE(ORIYA, or);
    // CASE(KURDISH, ku);
    // CASE(TAGALOG, tg);
    // CASE(TIGRINYA, ti);
    CASE(GALICIAN, gl);
    CASE(KANNADA, kn);
    CASE(MALAY, ms);
    // CASE(TAJIK, tg);
    CASE(GEORGIAN, ka);
    // CASE(ESPERANTO, eo);
#undef CASE
    /* Special cases */
    default:
        switch (langid) {
        case MAKELANGID(LANG_SERBIAN, 0x05): return "bs";
#define CASE(primary, sub, name) \
        case MAKELANGID(LANG_##primary, SUBLANG_##sub): return #name

        CASE(NORWEGIAN, NORWEGIAN_BOKMAL, nb);
        CASE(NORWEGIAN, NORWEGIAN_NYNORSK, nn);
        CASE(SERBIAN, DEFAULT, hr);
        CASE(SERBIAN, SERBIAN_LATIN, sh);
        CASE(SERBIAN, SERBIAN_CYRILLIC, sr);
        // CASE(SOTHO, DEFAULT, ns);
        // CASE(SOTHO, SOTHO_SOUTHERN, st);
#undef CASE
        default: return "";
        }
    }
}

static const char *ui_langs[100];
static int num_ui_langs = 0;

BOOL CALLBACK
enum_ui_lang_proc (LPTSTR language, LONG_PTR /* unused_lParam */)
{
    long langid = strtol(language, NULL, 16);
    if (langid > 0xFFFF)
        return TRUE;
    ui_langs[num_ui_langs] = langid_to_string((LANGID) langid, NULL);
    num_ui_langs++;
    if (num_ui_langs == SAL_N_ELEMENTS(ui_langs) )
        return FALSE;
    return TRUE;
}

static BOOL
present_in_ui_langs(const char *lang)
{
    for (int i = 0; i < num_ui_langs; i++)
        if (memcmp (ui_langs[i], lang, 2) == 0)
            return TRUE;
    return FALSE;
}

extern "C" UINT __stdcall SelectLanguage( MSIHANDLE handle )
{
#ifdef VERBOSE_DEBUG_OUTPUT
    char tem[2000];
#endif
    char feature[100];
    MSIHANDLE database, view, record;
    DWORD length;
    int nlangs = 0;
    /* Keeping this code simple and stupid... won't bother with any
     * dynamic arrays or whatnot. 100 is more than enough for this purpose.
     */
    char langs[100][6];

    database = MsiGetActiveDatabase(handle);

    if (MsiDatabaseOpenViewA(database, "SELECT Feature from Feature WHERE Feature_Parent = 'gm_Langpack_Languageroot'", &view) != ERROR_SUCCESS) {
        MsiCloseHandle(database);
        return ERROR_SUCCESS;
    }

#ifdef VERBOSE_DEBUG_OUTPUT
    MessageBoxA(NULL, "MsiDatabaseOpenViewA success!", "SelectLanguage", MB_OK);
#endif

    if (MsiViewExecute(view, NULL) != ERROR_SUCCESS) {
        MsiCloseHandle(view);
        MsiCloseHandle(database);
        return ERROR_SUCCESS;
    }

#ifdef VERBOSE_DEBUG_OUTPUT
    MessageBoxA(NULL, "MsiViewExecute success!", "SelectLanguage", MB_OK);
#endif

    while (MsiViewFetch(view, &record) == ERROR_SUCCESS) {
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

    if (nlangs > 0) {
        /* Deselect those languages that don't match any of the UI languages
         * available on the system.
         */

        int i;
        int have_system_default_lang = 0;
        const char *system_default_lang = langid_to_string(GetSystemDefaultUILanguage(), &have_system_default_lang);
        const char *user_locale_lang = langid_to_string(LANGIDFROMLCID(GetThreadLocale()), NULL);

#ifdef VERBOSE_DEBUG_OUTPUT
        sprintf(tem, "GetSystemDefaultUILanguage(): %#x = %s", GetSystemDefaultUILanguage(), system_default_lang);
        MessageBoxA(NULL, tem, "SelectLanguage", MB_OK);
#endif

        EnumUILanguagesA(enum_ui_lang_proc, 0, 0);

#ifdef VERBOSE_DEBUG_OUTPUT
        sprintf(tem, "Have %d UI languages: ", num_ui_langs);
        for (i = 0; i < num_ui_langs; i++) {
            char *p = tem + strlen(tem);
            sprintf(p, "%s%s",
                    ui_langs[i],
                    (i + 1 < num_ui_langs) ? ", " : "");
        }
        MessageBoxA(NULL, tem, "SelectLanguage", MB_OK);
#endif

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
        if (system_default_lang[0]) {
            for (i = 0; i < nlangs; i++) {
                if (memcmp (system_default_lang, langs[i], 2) == 0) {
#ifdef VERBOSE_DEBUG_OUTPUT
                    sprintf(tem, "We have the system default language %s in the installer", system_default_lang);
                    MessageBoxA(NULL, tem, "SelectLanguage", MB_OK);
#endif
                    have_system_default_lang = 1;
                }
            }
        }

        if (!have_system_default_lang) {
#ifdef VERBOSE_DEBUG_OUTPUT
            sprintf(tem, "We don't have the system default language %s in the installer, so pretend that English is the system default, sigh.", system_default_lang);
            MessageBoxA(NULL, tem, "SelectLanguage", MB_OK);
#endif
            system_default_lang = "en";
            have_system_default_lang = 1;
        }
        if (have_system_default_lang) {
            for (i = 0; i < nlangs; i++) {
                if (memcmp(system_default_lang, langs[i], 2) != 0 &&
                    memcmp(user_locale_lang, langs[i], 2) != 0 &&
                    !present_in_ui_langs(langs[i])) {
                    UINT rc;
                    sprintf(feature, "gm_Langpack_r_%s", langs[i]);
                    rc = MsiSetFeatureStateA(handle, feature, INSTALLSTATE_ABSENT);
                    if (rc != ERROR_SUCCESS) {
#ifdef VERBOSE_DEBUG_OUTPUT
                        sprintf(tem, "MsiSetFeatureStateA %s failed: %d", feature, rc);
                        MessageBoxA(NULL, tem, "SelectLanguage", MB_OK);
#endif
                    } else {
#ifdef VERBOSE_DEBUG_OUTPUT
                        sprintf(tem, "MsiSetFeatureStateA %s OK!", feature);
                        MessageBoxA(NULL, tem, "SelectLanguage", MB_OK);
#endif
                    }
                }
            }
        }
    }

    MsiCloseHandle(database);

    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
