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

#include <sal/config.h>

#include <memory>

#include "config.hxx"
#include "utilities.hxx"

#ifdef _WIN32_WINNT_WINBLUE
#include <VersionHelpers.h>
#endif

// constants


const size_t MAX_RES_STRING = 1024;
const wchar_t SPACE_CHAR = L' ';

static std::wstring StringToWString(const std::string& String, int codepage)
{
    int len = MultiByteToWideChar(
        codepage, 0, String.c_str(), -1, nullptr, 0);

    wchar_t* buff = static_cast<wchar_t*>(
        _alloca(len * sizeof(wchar_t)));

    MultiByteToWideChar(
        codepage, 0, String.c_str(), -1, buff, len);

    return std::wstring(buff);
}

static std::string WStringToString(const std::wstring& String, int codepage)
{
    int len = WideCharToMultiByte(
        codepage, 0, String.c_str(), -1, nullptr, 0, nullptr, nullptr);

    char* buff = static_cast<char*>(
        _alloca(len * sizeof(char)));

    WideCharToMultiByte(
        codepage, 0, String.c_str(), -1, buff, len, nullptr, nullptr);

    return std::string(buff);
}


/**
*/
std::wstring StringToWString(const std::string& String)
{
    return StringToWString(String, CP_ACP);
}


/**
*/
std::string WStringToString(const std::wstring& String)
{
    return WStringToString(String, CP_ACP);
}


/**
*/
std::wstring UTF8ToWString(const std::string& String)
{
    return StringToWString(String, CP_UTF8);
}


/**
*/
std::wstring GetResString(int ResId)
{
    wchar_t szResStr[MAX_RES_STRING];

    int rc = LoadStringW( GetModuleHandleW(MODULE_NAME), ResId, szResStr, sizeof(szResStr) );

    OutputDebugStringFormatA( "GetResString: read %d chars\n", rc );
    // OSL_ENSURE(rc, "String resource not found");

    return std::wstring(szResStr);
}


/**
*/
bool is_windows_xp_or_above()
{
// the Win32 SDK 8.1 deprecates GetVersionEx()
#ifdef _WIN32_WINNT_WINBLUE
    return IsWindowsXPOrGreater();
#else
    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionEx(&osvi);

    // LLA: check for windows xp or above (Vista)
    if (osvi.dwMajorVersion > 5 ||
        (5 == osvi.dwMajorVersion && osvi.dwMinorVersion >= 1))
    {
        return true;
    }
    return false;
#endif
}


/** helper function to judge if the string is only has spaces.
    @returns
        <TRUE>if the provided string contains only but at least one space
        character else <FALSE/>.
*/

bool HasOnlySpaces(const std::wstring& String)
{
    if ( String.length() == 0 )
        return false;

    const wchar_t* p = String.c_str();

    while (*p)
    {
        if (*p++ != SPACE_CHAR)
            return false;
    }

    return true;
}


/** helper function to convert windows paths to short form.
    @returns
        shortend path.
*/

std::wstring getShortPathName( const std::wstring& aLongName )
{
    std::wstring shortName = aLongName;
    long         length    = GetShortPathNameW( aLongName.c_str(), nullptr, 0 );

    if ( length != 0 )
    {
        auto buffer = std::unique_ptr<WCHAR[]>(new WCHAR[ length+1 ]);
        length = GetShortPathNameW( aLongName.c_str(), buffer.get(), length );
        if ( length != 0 )
            shortName = std::wstring( buffer.get() );
    }
    return shortName;
}


/** convert LocaleSet pair into Microsoft List of Locale ID (LCID)
    according to ISO-639 and ISO-3166.
    http://etext.lib.virginia.edu/tei/iso639.html
    http://nl.ijs.si/gnusl/cee/std/ISO_3166.html
    @param
        Locale, LocaleSet
    @returns
        Windows Locale Identifier corresponding to input LocaleSet.
    @Usage Sample
        LocaleSet_t myDefaultLocale( ::std::wstring( L"zh" ),::std::wstring(L"HK") );
        DWORD myLCID = LocaleSetToLCID( myDefaultLocale );
        wchar_t  buffer[20];
        _ultow( myLCID, buffer, 16 );
        MessageBox( NULL, buffer,L"the LCID is:",MB_OK );
*/

LCID LocaleSetToLCID( const LocaleSet_t & Locale )
{
    if ( EMPTY_LOCALE == Locale )
        return GetSystemDefaultLCID();

    USHORT usPrimaryLang= LANG_NEUTRAL;
    USHORT usSubLang=SUBLANG_DEFAULT;

    ::std::wstring wsLanguage(Locale.first);
    ::std::wstring wsCountry(Locale.second);

    if  ( wsLanguage == L"ar" )
    {
        usPrimaryLang = LANG_ARABIC;                      // Arabic 01

        if ( wsCountry == L"SA" )
            usSubLang = SUBLANG_ARABIC_SAUDI_ARABIA;          // Arabic (Saudi Arabia)
        else if ( wsCountry == L"IQ" )
            usSubLang = SUBLANG_ARABIC_IRAQ;                  // Arabic (Iraq)
        else if ( wsCountry == L"EG" )
            usSubLang = SUBLANG_ARABIC_EGYPT;                 // Arabic (Egypt)
        else if ( wsCountry == L"LY" )
            usSubLang = SUBLANG_ARABIC_LIBYA;                 // Arabic (Libya)
        else if ( wsCountry == L"DZ" )
            usSubLang = SUBLANG_ARABIC_ALGERIA;               // Arabic (Algeria)
        else if ( wsCountry == L"MA" )
            usSubLang = SUBLANG_ARABIC_MOROCCO;               // Arabic (Morocco)
        else if ( wsCountry == L"TN" )
            usSubLang = SUBLANG_ARABIC_TUNISIA;               // Arabic (Tunisia)
        else if ( wsCountry == L"OM" )
            usSubLang = SUBLANG_ARABIC_OMAN;                  // Arabic (Oman)
        else if ( wsCountry == L"YE" )
            usSubLang = SUBLANG_ARABIC_YEMEN;                 // Arabic (Yemen)
        else if ( wsCountry == L"SY" )
            usSubLang = SUBLANG_ARABIC_SYRIA;                 // Arabic (Syria)
        else if ( wsCountry == L"JO" )
            usSubLang = SUBLANG_ARABIC_JORDAN;                // Arabic (Jordan)
        else if ( wsCountry == L"LB" )
            usSubLang = SUBLANG_ARABIC_LEBANON;               // Arabic (Lebanon)
        else if ( wsCountry == L"KW" )
            usSubLang = SUBLANG_ARABIC_KUWAIT;                // Arabic (Kuwait)
        else if ( wsCountry == L"AE" )
            usSubLang = SUBLANG_ARABIC_UAE;                   // Arabic (U.A.E.)
        else if ( wsCountry == L"BH" )
            usSubLang = SUBLANG_ARABIC_BAHRAIN;               // Arabic (Bahrain)
        else if ( wsCountry == L"QA" )
            usSubLang = SUBLANG_ARABIC_QATAR;                 // Arabic (Qatar)
        else
            usSubLang = SUBLANG_DEFAULT;                      //default sub language
    }
    else if ( wsLanguage == L"bg" )
        usPrimaryLang = LANG_BULGARIAN;                   //Bulgarian 02
    else if ( wsLanguage == L"ca" )
        usPrimaryLang = LANG_CATALAN;                     //Catalan 03
    else if ( wsLanguage == L"zh" )
    {
        usPrimaryLang = LANG_CHINESE;                     //Chinese
        if ( wsCountry == L"TW" )
            usSubLang = SUBLANG_CHINESE_TRADITIONAL;          // Chinese (Traditional)
        else if ( wsCountry == L"CN" )
            usSubLang = SUBLANG_CHINESE_SIMPLIFIED;           // Chinese (Simplified)
        else if ( wsCountry == L"HK" )
            usSubLang = SUBLANG_CHINESE_HONGKONG;             // Chinese (Hong Kong SAR, PRC)
        else if ( wsCountry == L"SG" )
            usSubLang = SUBLANG_CHINESE_SINGAPORE;            // Chinese (Singapore)
        else if ( wsCountry == L"MO" )
            usSubLang = SUBLANG_CHINESE_MACAU;                // Chinese (Macau SAR)
        else
            usSubLang = SUBLANG_DEFAULT;                      //default sub language
    }
    else if ( wsLanguage == L"cs" )
        usPrimaryLang = LANG_CZECH;                       //Czech
    else if ( wsLanguage == L"da" )
        usPrimaryLang = LANG_DANISH;                      //Danish
    else if ( wsLanguage == L"de" )
    {
        usPrimaryLang = LANG_GERMAN;                      //German
        if ( wsCountry == L"DE" )
            usSubLang = SUBLANG_GERMAN;                       // German
        else if ( wsCountry == L"CH" )
            usSubLang = SUBLANG_GERMAN_SWISS;                 // German (Swiss)
        else if ( wsCountry == L"AT" )
            usSubLang = SUBLANG_GERMAN_AUSTRIAN;              // German (Austrian)
        else if ( wsCountry == L"LU" )
            usSubLang = SUBLANG_GERMAN_LUXEMBOURG;            // German (Luxembourg)
        else if ( wsCountry == L"LI" )
            usSubLang = SUBLANG_GERMAN_LIECHTENSTEIN;         // German (Liechtenstein)
        else
            usSubLang = SUBLANG_DEFAULT;                      //default sub language
    }
    else if ( wsLanguage == L"el" )
        usPrimaryLang = LANG_GREEK;                       //Greek
    else if ( wsLanguage == L"en" )
    {
        usPrimaryLang = LANG_ENGLISH;                         //English
        if ( wsCountry == L"US" )
            usSubLang = SUBLANG_ENGLISH_US;                   // English (US)
        else if ( wsCountry == L"GB" )
            usSubLang = SUBLANG_ENGLISH_UK;                   // English (UK)
        else if ( wsCountry == L"AU" )
            usSubLang = SUBLANG_ENGLISH_AUS;                  // English (Australian)
        else if ( wsCountry == L"CA" )
            usSubLang = SUBLANG_ENGLISH_CAN;                  // English (Canadian)
        else if ( wsCountry == L"NZ" )
            usSubLang = SUBLANG_ENGLISH_NZ;                   // English (New Zealand)
        else if ( wsCountry == L"IE" )
            usSubLang = SUBLANG_ENGLISH_EIRE;                 // English (Ireland)
        else if ( wsCountry == L"ZA" )
            usSubLang = SUBLANG_ENGLISH_SOUTH_AFRICA;         // English (South Africa)
        else if ( wsCountry == L"JM" )
            usSubLang = SUBLANG_ENGLISH_JAMAICA;              // English (Jamaica)
        else if ( wsCountry == L"GD" )
            usSubLang = SUBLANG_ENGLISH_CARIBBEAN;            // English (Caribbean) Grenada
        else if ( wsCountry == L"BZ" )
            usSubLang = SUBLANG_ENGLISH_BELIZE;               // English (Belize)
        else if ( wsCountry == L"TT" )
            usSubLang = SUBLANG_ENGLISH_TRINIDAD;             // English (Trinidad)
        else if ( wsCountry == L"ZW" )
            usSubLang = SUBLANG_ENGLISH_ZIMBABWE;             // English (Zimbabwe)
        else if ( wsCountry == L"PH" )
            usSubLang = SUBLANG_ENGLISH_PHILIPPINES;          // English (Philippines)
        else
            usSubLang = SUBLANG_DEFAULT;                      //default sub language
    }
    else if ( wsLanguage == L"es" )
    {
        usPrimaryLang = LANG_SPANISH;                     //Spanish
        if ( wsCountry == L"MX" )
            usSubLang = SUBLANG_SPANISH_MEXICAN;              // Spanish (Mexican)
        else if ( wsCountry == L"ES" )
            usSubLang = SUBLANG_SPANISH_MODERN;               // Spanish (Spain)
        else if ( wsCountry == L"GT" )
            usSubLang = SUBLANG_SPANISH_GUATEMALA;            // Spanish (Guatemala)
        else if ( wsCountry == L"CR" )
            usSubLang = SUBLANG_SPANISH_COSTA_RICA;           // Spanish (Costa Rica)
        else if ( wsCountry == L"PA" )
            usSubLang = SUBLANG_SPANISH_PANAMA;               // Spanish (Panama)
        else if ( wsCountry == L"DO" )
            usSubLang = SUBLANG_SPANISH_DOMINICAN_REPUBLIC;   // Spanish (Dominican Republic)
        else if ( wsCountry == L"VE" )
            usSubLang = SUBLANG_SPANISH_VENEZUELA;            // Spanish (Venezuela)
        else if ( wsCountry == L"CO" )
            usSubLang = SUBLANG_SPANISH_COLOMBIA;             // Spanish (Colombia)
        else if ( wsCountry == L"PE" )
            usSubLang = SUBLANG_SPANISH_PERU;                 // Spanish (Peru)
        else if ( wsCountry == L"AR" )
            usSubLang = SUBLANG_SPANISH_ARGENTINA;            // Spanish (Argentina)
        else if ( wsCountry == L"EC" )
            usSubLang = SUBLANG_SPANISH_ECUADOR;              // Spanish (Ecuador)
        else if ( wsCountry == L"CL" )
            usSubLang = SUBLANG_SPANISH_CHILE;                // Spanish (Chile)
        else if ( wsCountry == L"UY" )
            usSubLang = SUBLANG_SPANISH_URUGUAY;              // Spanish (Uruguay)
        else if ( wsCountry == L"PY" )
            usSubLang = SUBLANG_SPANISH_PARAGUAY;             // Spanish (Paraguay)
        else if ( wsCountry == L"BO" )
            usSubLang = SUBLANG_SPANISH_BOLIVIA;              // Spanish (Bolivia)
        else if ( wsCountry == L"SV" )
            usSubLang = SUBLANG_SPANISH_EL_SALVADOR;          // Spanish (El Salvador)
        else if ( wsCountry == L"HN" )
            usSubLang = SUBLANG_SPANISH_HONDURAS;             // Spanish (Honduras)
        else if ( wsCountry == L"NI" )
            usSubLang = SUBLANG_SPANISH_NICARAGUA;            // Spanish (Nicaragua)
        else if ( wsCountry == L"PR" )
            usSubLang = SUBLANG_SPANISH_PUERTO_RICO;          // Spanish (Puerto Rico)
        else
            usSubLang = SUBLANG_DEFAULT;                      //default sub language
    }
    else if ( wsLanguage == L"fi" )
        usPrimaryLang = LANG_FINNISH;                     //Finnish
    else if ( wsLanguage == L"fr" )
    {
        usPrimaryLang = LANG_FRENCH;                      //French
        if ( wsCountry == L"FR" )
            usSubLang = SUBLANG_FRENCH;                        // French
        else if ( wsCountry == L"BE" )
            usSubLang = SUBLANG_FRENCH_BELGIAN;                // French (Belgian)
        else if ( wsCountry == L"CA" )
            usSubLang = SUBLANG_FRENCH_CANADIAN;               // French (Canadian)
        else if ( wsCountry == L"CH" )
            usSubLang = SUBLANG_FRENCH_SWISS;                  // French (Swiss)
        else if ( wsCountry == L"LU" )
            usSubLang = SUBLANG_FRENCH_LUXEMBOURG;             // French (Luxembourg)
        else if ( wsCountry == L"MC" )
            usSubLang = SUBLANG_FRENCH_MONACO;                 // French (Monaco)
        else
            usSubLang = SUBLANG_DEFAULT;                       //default sub language
    }
    else if ( wsLanguage == L"iw" )
        usPrimaryLang = LANG_HEBREW;                      //Hebrew
    else if ( wsLanguage == L"hu" )
        usPrimaryLang = LANG_HUNGARIAN;                        //Hungarian
    else if ( wsLanguage == L"is" )
        usPrimaryLang = LANG_ICELANDIC;                        //Icelandic
    else if ( wsLanguage == L"it" )
    {
        usPrimaryLang = LANG_ITALIAN;                     //Italian
        if ( wsCountry == L"IT" )
            usSubLang = SUBLANG_ITALIAN;                       // Italian
        else if ( wsCountry == L"CH" )
            usSubLang = SUBLANG_ITALIAN_SWISS;                 // Italian (Swiss)
        else
            usSubLang = SUBLANG_DEFAULT;                       //default sub language
    }
    else if ( wsLanguage == L"ja" )
        usPrimaryLang = LANG_JAPANESE;                    //Japanese
    else if ( wsLanguage == L"ko" )
    {
        usPrimaryLang = LANG_KOREAN;                      //Korean
        if ( wsCountry == L"KR" )
            usSubLang = SUBLANG_KOREAN;                       // Korean
        else
            usSubLang = SUBLANG_DEFAULT;                      //default sub language
    }
    else if ( wsLanguage == L"nl" )
    {
        usPrimaryLang = LANG_DUTCH;                       //Dutch
        if ( wsCountry == L"NL" )
            usSubLang = SUBLANG_DUTCH;                        // Dutch
        else if ( wsCountry == L"BE" )
            usSubLang = SUBLANG_DUTCH_BELGIAN;                // Dutch (Belgian)
        else
            usSubLang = SUBLANG_DEFAULT;                      //default sub language
    }
    else if ( wsLanguage == L"no" )
    {
        usPrimaryLang = LANG_NORWEGIAN;                   //Norwegian
        if ( wsCountry == L"NO" )
            usSubLang = SUBLANG_NORWEGIAN_BOKMAL;             // Norwegian (Bokmal)
        else
            usSubLang = SUBLANG_DEFAULT;                      //default sub language
    }
    else if ( wsLanguage == L"pl" )
        usPrimaryLang = LANG_POLISH;                      //Polish
    else if ( wsLanguage == L"pt" )
    {
        usPrimaryLang = LANG_PORTUGUESE;                  //Portuguese
        if ( wsCountry == L"BR" )
            usSubLang = SUBLANG_PORTUGUESE_BRAZILIAN;         // Portuguese (Brazil)
        else if ( wsCountry == L"PT" )
            usSubLang = SUBLANG_PORTUGUESE;                   // Portuguese (Portugal)
        else
            usSubLang = SUBLANG_DEFAULT;                      //default sub language
    }
    else if ( wsLanguage == L"ro" )
        usPrimaryLang = LANG_ROMANIAN;                    //Romanian
    else if ( wsLanguage == L"ru" )
        usPrimaryLang = LANG_RUSSIAN;                     //Russian
    else if ( wsLanguage == L"hr" )
        usPrimaryLang = LANG_CROATIAN;                    //Croatian
    else if ( wsLanguage == L"sr" )
    {
        usPrimaryLang = LANG_SERBIAN;                     //Serbian
        if ( wsCountry == L"VA" )
            usSubLang = SUBLANG_SERBIAN_LATIN;                 // Serbian (Latin)
        else if ( wsCountry == L"HR" )
            usSubLang = SUBLANG_SERBIAN_CYRILLIC;              // Serbian (Cyrillic)
        else
            usSubLang = SUBLANG_DEFAULT;                       //default sub language
    }
    else if ( wsLanguage == L"sk" )
        usPrimaryLang = LANG_SLOVAK;                      //Slovak
    else if ( wsLanguage == L"sq" )
        usPrimaryLang = LANG_ALBANIAN;                    //Albanian
    else if ( wsLanguage == L"sv" )
    {
        usPrimaryLang = LANG_SWEDISH;                     //Swedish
        if ( wsCountry == L"SE" )
            usSubLang = SUBLANG_SWEDISH;                       // Swedish
        else if ( wsCountry == L"FI" )
            usSubLang = SUBLANG_SWEDISH_FINLAND;               // Swedish (Finland)
        else
            usSubLang = SUBLANG_DEFAULT;                       //default sub language
    }
    else if ( wsLanguage == L"th" )
        usPrimaryLang = LANG_THAI;                        //Thai
    else if ( wsLanguage == L"tr" )
        usPrimaryLang = LANG_TURKISH;                     //Turkish
    else if ( wsLanguage == L"ur" )
    {
        usPrimaryLang = LANG_URDU;                        //Urdu
        if ( wsCountry == L"PK" )
            usSubLang = SUBLANG_URDU_PAKISTAN;                 // Urdu (Pakistan)
        else if ( wsCountry == L"IN" )
            usSubLang = SUBLANG_URDU_INDIA;                    // Urdu (India)
        else
            usSubLang = SUBLANG_DEFAULT;                       //default sub language
    }
    else if ( wsLanguage == L"in" )
        usPrimaryLang = LANG_INDONESIAN;                  //Indonesian
    else if ( wsLanguage == L"uk" )
        usPrimaryLang = LANG_UKRAINIAN;                   //Ukrainian
    else if ( wsLanguage == L"be" )
        usPrimaryLang = LANG_BELARUSIAN;                  //Belarusian
    else if ( wsLanguage == L"sl" )
        usPrimaryLang = LANG_SLOVENIAN;                   //Slovenian
    else if ( wsLanguage == L"et" )
        usPrimaryLang = LANG_ESTONIAN;                    //Estonian
    else if ( wsLanguage == L"lv" )
        usPrimaryLang = LANG_LATVIAN;                     //Latvian
    else if ( wsLanguage == L"lt" )
    {
        usPrimaryLang = LANG_LITHUANIAN;                  //Lithuanian
        if ( wsCountry == L"LT" )
            usSubLang = SUBLANG_LITHUANIAN;                    // Lithuanian
        else
            usSubLang = SUBLANG_DEFAULT;                       //default sub language
    }
    else if ( wsLanguage == L"fa" )
        usPrimaryLang = LANG_FARSI;                       //Farsi
    else if ( wsLanguage == L"vi" )
        usPrimaryLang = LANG_VIETNAMESE;                  //Vietnamese
    else if ( wsLanguage == L"hy" )
        usPrimaryLang = LANG_ARMENIAN;                    //Armenian
    else if ( wsLanguage == L"az" )
        usPrimaryLang = LANG_AZERI;                       //Azeri
    else if ( wsLanguage == L"eu" )
        usPrimaryLang = LANG_BASQUE;                      //Basque
    else if ( wsLanguage == L"mk" )
        usPrimaryLang = LANG_MACEDONIAN;                  //FYRO Macedonian
    else if ( wsLanguage == L"af" )
        usPrimaryLang = LANG_AFRIKAANS;                   //Afrikaans
    else if ( wsLanguage == L"ka" )
        usPrimaryLang = LANG_GEORGIAN;                    //Georgian
    else if ( wsLanguage == L"fo" )
        usPrimaryLang = LANG_FAEROESE;                    //Faeroese
    else if ( wsLanguage == L"hi" )
        usPrimaryLang = LANG_HINDI;                       //Hindi
    else if ( wsLanguage == L"ms" )
    {
        usPrimaryLang = LANG_MALAY;                       //Malay
        if ( wsCountry == L"MY" )
            usSubLang = SUBLANG_MALAY_MALAYSIA;                // Malay (Malaysia)
        else if ( wsCountry == L"BN" )
            usSubLang = SUBLANG_MALAY_BRUNEI_DARUSSALAM;       // Malay (Brunei Darassalam)
        else
            usSubLang = SUBLANG_DEFAULT;                       //default sub language
    }
    else if ( wsLanguage == L"kk" )
        usPrimaryLang = LANG_KAZAK;                       //Kazakh
    else if ( wsLanguage == L"ky" )
        usPrimaryLang = LANG_KYRGYZ;                      //Kyrgyz
    else if ( wsLanguage == L"sw" )
        usPrimaryLang = LANG_SWAHILI;                     //Swahili
    else if ( wsLanguage == L"uz" )
    {
        usPrimaryLang = LANG_UZBEK;                       //Uzbek
        if ( wsCountry == L"UZ" )
            usSubLang = SUBLANG_UZBEK_LATIN;                   // Uzbek (Latin)
        else if ( wsCountry == L"DE" )
            usSubLang = SUBLANG_UZBEK_CYRILLIC;                // Uzbek (Cyrillic)
        else
            usSubLang = SUBLANG_DEFAULT;                       //default sub language
    }
    else if ( wsLanguage == L"tt" )
        usPrimaryLang = LANG_TATAR;                       //Tatar
    else if ( wsLanguage == L"bn" )
        usPrimaryLang = LANG_BENGALI;                     //Not supported.
    else if ( wsLanguage == L"pa" )
        usPrimaryLang = LANG_PUNJABI;                     //Punjabi
    else if ( wsLanguage == L"gu" )
        usPrimaryLang = LANG_GUJARATI;                    //Gujarati
    else if ( wsLanguage == L"or" )
        usPrimaryLang = LANG_ORIYA;                       //Not supported.
    else if ( wsLanguage == L"ta" )
        usPrimaryLang = LANG_TAMIL;                       //Tamil
    else if ( wsLanguage == L"te" )
        usPrimaryLang = LANG_TELUGU;                      //Telugu
    else if ( wsLanguage == L"kn" )
        usPrimaryLang = LANG_KANNADA;                     //Kannada
    else if ( wsLanguage == L"ml" )
        usPrimaryLang = LANG_MALAYALAM;                   //Not supported.
    else if ( wsLanguage == L"as" )
        usPrimaryLang = LANG_ASSAMESE;                    //Not supported.
    else if ( wsLanguage == L"mr" )
        usPrimaryLang = LANG_MARATHI;                     //Marathi
    else if ( wsLanguage == L"sa" )
        usPrimaryLang = LANG_SANSKRIT;                    //Sanskrit
    else if ( wsLanguage == L"mn" )
        usPrimaryLang = LANG_MONGOLIAN;                   //Mongolian
    else if ( wsLanguage == L"gl" )
        usPrimaryLang = LANG_GALICIAN;                    //Galician
    else if ( wsLanguage == L"sd" )
        usPrimaryLang = LANG_SINDHI;                      //Not supported.
    else if ( wsLanguage == L"ks" )
        usPrimaryLang = LANG_KASHMIRI;                    //Not supported.
    else if ( wsLanguage == L"ne" )
        usPrimaryLang = LANG_NEPALI;                      //Not supported.
    else
        return GetSystemDefaultLCID();                    //System Default Locale

    return MAKELCID( MAKELANGID( usPrimaryLang, usSubLang ), SORT_DEFAULT );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
