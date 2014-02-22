/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#undef OSL_DEBUG_LEVEL


#include <osl/diagnose.h>

#include "internal/config.hxx"
#include "internal/utilities.hxx"





const size_t MAX_RES_STRING = 1024;
const wchar_t SPACE_CHAR = _T(' ');

static std::wstring StringToWString(const std::string& String, int codepage)
{
    int len = MultiByteToWideChar(
        codepage, 0, String.c_str(), -1, 0, 0);

    wchar_t* buff = reinterpret_cast<wchar_t*>(
        _alloca(len * sizeof(wchar_t)));

    MultiByteToWideChar(
        codepage, 0, String.c_str(), -1, buff, len);

    return std::wstring(buff);
}

static std::string WStringToString(const std::wstring& String, int codepage)
{
    int len = WideCharToMultiByte(
        codepage, 0, String.c_str(), -1, 0, 0, 0, 0);

    char* buff = reinterpret_cast<char*>(
        _alloca(len * sizeof(char)));

    WideCharToMultiByte(
        codepage, 0, String.c_str(), -1, buff, len, 0, 0);

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

    OutputDebugStringFormat( "GetResString: read %d chars\n", rc );
    OSL_ENSURE(rc, "String resource not found");

    return std::wstring(szResStr);
}


/**
*/
bool is_windows_xp_or_above()
{
    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionEx(&osvi);

    
    if (osvi.dwMajorVersion > 5 ||
        (5 == osvi.dwMajorVersion && osvi.dwMinorVersion >= 1))
    {
        return true;
    }
    return false;
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
    long         length    = GetShortPathName( aLongName.c_str(), NULL, 0 );

    if ( length != 0 )
    {
        TCHAR* buffer = new TCHAR[ length+1 ];
        length = GetShortPathName( aLongName.c_str(), buffer, length );
        if ( length != 0 )
            shortName = std::wstring( buffer );
        delete [] buffer;
    }
    return shortName;
}


/** convert LocaleSet pair into Microsoft List of Locale ID (LCID)
    according to ISO-639 and ISO-3166.
    http:
    http:
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
        usPrimaryLang = LANG_ARABIC;                      

        if ( wsCountry == L"SA" )
            usSubLang = SUBLANG_ARABIC_SAUDI_ARABIA;          
        else if ( wsCountry == L"IQ" )
            usSubLang = SUBLANG_ARABIC_IRAQ;                  
        else if ( wsCountry == L"EG" )
            usSubLang = SUBLANG_ARABIC_EGYPT;                 
        else if ( wsCountry == L"LY" )
            usSubLang = SUBLANG_ARABIC_LIBYA;                 
        else if ( wsCountry == L"DZ" )
            usSubLang = SUBLANG_ARABIC_ALGERIA;               
        else if ( wsCountry == L"MA" )
            usSubLang = SUBLANG_ARABIC_MOROCCO;               
        else if ( wsCountry == L"TN" )
            usSubLang = SUBLANG_ARABIC_TUNISIA;               
        else if ( wsCountry == L"OM" )
            usSubLang = SUBLANG_ARABIC_OMAN;                  
        else if ( wsCountry == L"YE" )
            usSubLang = SUBLANG_ARABIC_YEMEN;                 
        else if ( wsCountry == L"SY" )
            usSubLang = SUBLANG_ARABIC_SYRIA;                 
        else if ( wsCountry == L"JO" )
            usSubLang = SUBLANG_ARABIC_JORDAN;                
        else if ( wsCountry == L"LB" )
            usSubLang = SUBLANG_ARABIC_LEBANON;               
        else if ( wsCountry == L"KW" )
            usSubLang = SUBLANG_ARABIC_KUWAIT;                
        else if ( wsCountry == L"AE" )
            usSubLang = SUBLANG_ARABIC_UAE;                   
        else if ( wsCountry == L"BH" )
            usSubLang = SUBLANG_ARABIC_BAHRAIN;               
        else if ( wsCountry == L"QA" )
            usSubLang = SUBLANG_ARABIC_QATAR;                 
        else
            usSubLang = SUBLANG_DEFAULT;                      
    }
    else if ( wsLanguage == L"bg" )
        usPrimaryLang = LANG_BULGARIAN;                   
    else if ( wsLanguage == L"ca" )
        usPrimaryLang = LANG_CATALAN;                     
    else if ( wsLanguage == L"zh" )
    {
        usPrimaryLang = LANG_CHINESE;                     
        if ( wsCountry == L"TW" )
            usSubLang = SUBLANG_CHINESE_TRADITIONAL;          
        else if ( wsCountry == L"CN" )
            usSubLang = SUBLANG_CHINESE_SIMPLIFIED;           
        else if ( wsCountry == L"HK" )
            usSubLang = SUBLANG_CHINESE_HONGKONG;             
        else if ( wsCountry == L"SG" )
            usSubLang = SUBLANG_CHINESE_SINGAPORE;            
        else if ( wsCountry == L"MO" )
            usSubLang = SUBLANG_CHINESE_MACAU;                
        else
            usSubLang = SUBLANG_DEFAULT;                      
    }
    else if ( wsLanguage == L"cs" )
        usPrimaryLang = LANG_CZECH;                       
    else if ( wsLanguage == L"da" )
        usPrimaryLang = LANG_DANISH;                      
    else if ( wsLanguage == L"de" )
    {
        usPrimaryLang = LANG_GERMAN;                      
        if ( wsCountry == L"DE" )
            usSubLang = SUBLANG_GERMAN;                       
        else if ( wsCountry == L"CH" )
            usSubLang = SUBLANG_GERMAN_SWISS;                 
        else if ( wsCountry == L"AT" )
            usSubLang = SUBLANG_GERMAN_AUSTRIAN;              
        else if ( wsCountry == L"LU" )
            usSubLang = SUBLANG_GERMAN_LUXEMBOURG;            
        else if ( wsCountry == L"LI" )
            usSubLang = SUBLANG_GERMAN_LIECHTENSTEIN;         
        else
            usSubLang = SUBLANG_DEFAULT;                      
    }
    else if ( wsLanguage == L"el" )
        usPrimaryLang = LANG_GREEK;                       
    else if ( wsLanguage == L"en" )
    {
        usPrimaryLang = LANG_ENGLISH;                         
        if ( wsCountry == L"US" )
            usSubLang = SUBLANG_ENGLISH_US;                   
        else if ( wsCountry == L"GB" )
            usSubLang = SUBLANG_ENGLISH_UK;                   
        else if ( wsCountry == L"AU" )
            usSubLang = SUBLANG_ENGLISH_AUS;                  
        else if ( wsCountry == L"CA" )
            usSubLang = SUBLANG_ENGLISH_CAN;                  
        else if ( wsCountry == L"NZ" )
            usSubLang = SUBLANG_ENGLISH_NZ;                   
        else if ( wsCountry == L"IE" )
            usSubLang = SUBLANG_ENGLISH_EIRE;                 
        else if ( wsCountry == L"ZA" )
            usSubLang = SUBLANG_ENGLISH_SOUTH_AFRICA;         
        else if ( wsCountry == L"JM" )
            usSubLang = SUBLANG_ENGLISH_JAMAICA;              
        else if ( wsCountry == L"GD" )
            usSubLang = SUBLANG_ENGLISH_CARIBBEAN;            
        else if ( wsCountry == L"BZ" )
            usSubLang = SUBLANG_ENGLISH_BELIZE;               
        else if ( wsCountry == L"TT" )
            usSubLang = SUBLANG_ENGLISH_TRINIDAD;             
        else if ( wsCountry == L"ZW" )
            usSubLang = SUBLANG_ENGLISH_ZIMBABWE;             
        else if ( wsCountry == L"PH" )
            usSubLang = SUBLANG_ENGLISH_PHILIPPINES;          
        else
            usSubLang = SUBLANG_DEFAULT;                      
    }
    else if ( wsLanguage == L"es" )
    {
        usPrimaryLang = LANG_SPANISH;                     
        if ( wsCountry == L"MX" )
            usSubLang = SUBLANG_SPANISH_MEXICAN;              
        else if ( wsCountry == L"ES" )
            usSubLang = SUBLANG_SPANISH_MODERN;               
        else if ( wsCountry == L"GT" )
            usSubLang = SUBLANG_SPANISH_GUATEMALA;            
        else if ( wsCountry == L"CR" )
            usSubLang = SUBLANG_SPANISH_COSTA_RICA;           
        else if ( wsCountry == L"PA" )
            usSubLang = SUBLANG_SPANISH_PANAMA;               
        else if ( wsCountry == L"DO" )
            usSubLang = SUBLANG_SPANISH_DOMINICAN_REPUBLIC;   
        else if ( wsCountry == L"VE" )
            usSubLang = SUBLANG_SPANISH_VENEZUELA;            
        else if ( wsCountry == L"CO" )
            usSubLang = SUBLANG_SPANISH_COLOMBIA;             
        else if ( wsCountry == L"PE" )
            usSubLang = SUBLANG_SPANISH_PERU;                 
        else if ( wsCountry == L"AR" )
            usSubLang = SUBLANG_SPANISH_ARGENTINA;            
        else if ( wsCountry == L"EC" )
            usSubLang = SUBLANG_SPANISH_ECUADOR;              
        else if ( wsCountry == L"CL" )
            usSubLang = SUBLANG_SPANISH_CHILE;                
        else if ( wsCountry == L"UY" )
            usSubLang = SUBLANG_SPANISH_URUGUAY;              
        else if ( wsCountry == L"PY" )
            usSubLang = SUBLANG_SPANISH_PARAGUAY;             
        else if ( wsCountry == L"BO" )
            usSubLang = SUBLANG_SPANISH_BOLIVIA;              
        else if ( wsCountry == L"SV" )
            usSubLang = SUBLANG_SPANISH_EL_SALVADOR;          
        else if ( wsCountry == L"HN" )
            usSubLang = SUBLANG_SPANISH_HONDURAS;             
        else if ( wsCountry == L"NI" )
            usSubLang = SUBLANG_SPANISH_NICARAGUA;            
        else if ( wsCountry == L"PR" )
            usSubLang = SUBLANG_SPANISH_PUERTO_RICO;          
        else
            usSubLang = SUBLANG_DEFAULT;                      
    }
    else if ( wsLanguage == L"fi" )
        usPrimaryLang = LANG_FINNISH;                     
    else if ( wsLanguage == L"fr" )
    {
        usPrimaryLang = LANG_FRENCH;                      
        if ( wsCountry == L"FR" )
            usSubLang = SUBLANG_FRENCH;                        
        else if ( wsCountry == L"BE" )
            usSubLang = SUBLANG_FRENCH_BELGIAN;                
        else if ( wsCountry == L"CA" )
            usSubLang = SUBLANG_FRENCH_CANADIAN;               
        else if ( wsCountry == L"CH" )
            usSubLang = SUBLANG_FRENCH_SWISS;                  
        else if ( wsCountry == L"LU" )
            usSubLang = SUBLANG_FRENCH_LUXEMBOURG;             
        else if ( wsCountry == L"MC" )
            usSubLang = SUBLANG_FRENCH_MONACO;                 
        else
            usSubLang = SUBLANG_DEFAULT;                       
    }
    else if ( wsLanguage == L"iw" )
        usPrimaryLang = LANG_HEBREW;                      
    else if ( wsLanguage == L"hu" )
        usPrimaryLang = LANG_HUNGARIAN;                        
    else if ( wsLanguage == L"is" )
        usPrimaryLang = LANG_ICELANDIC;                        
    else if ( wsLanguage == L"it" )
    {
        usPrimaryLang = LANG_ITALIAN;                     
        if ( wsCountry == L"IT" )
            usSubLang = SUBLANG_ITALIAN;                       
        else if ( wsCountry == L"CH" )
            usSubLang = SUBLANG_ITALIAN_SWISS;                 
        else
            usSubLang = SUBLANG_DEFAULT;                       
    }
    else if ( wsLanguage == L"ja" )
        usPrimaryLang = LANG_JAPANESE;                    
    else if ( wsLanguage == L"ko" )
    {
        usPrimaryLang = LANG_KOREAN;                      
        if ( wsCountry == L"KR" )
            usSubLang = SUBLANG_KOREAN;                       
        else
            usSubLang = SUBLANG_DEFAULT;                      
    }
    else if ( wsLanguage == L"nl" )
    {
        usPrimaryLang = LANG_DUTCH;                       
        if ( wsCountry == L"NL" )
            usSubLang = SUBLANG_DUTCH;                        
        else if ( wsCountry == L"BE" )
            usSubLang = SUBLANG_DUTCH_BELGIAN;                
        else
            usSubLang = SUBLANG_DEFAULT;                      
    }
    else if ( wsLanguage == L"no" )
    {
        usPrimaryLang = LANG_NORWEGIAN;                   
        if ( wsCountry == L"NO" )
            usSubLang = SUBLANG_NORWEGIAN_BOKMAL;             
        else
            usSubLang = SUBLANG_DEFAULT;                      
    }
    else if ( wsLanguage == L"pl" )
        usPrimaryLang = LANG_POLISH;                      
    else if ( wsLanguage == L"pt" )
    {
        usPrimaryLang = LANG_PORTUGUESE;                  
        if ( wsCountry == L"BR" )
            usSubLang = SUBLANG_PORTUGUESE_BRAZILIAN;         
        else if ( wsCountry == L"PT" )
            usSubLang = SUBLANG_PORTUGUESE;                   
        else
            usSubLang = SUBLANG_DEFAULT;                      
    }
    else if ( wsLanguage == L"ro" )
        usPrimaryLang = LANG_ROMANIAN;                    
    else if ( wsLanguage == L"ru" )
        usPrimaryLang = LANG_RUSSIAN;                     
    else if ( wsLanguage == L"hr" )
        usPrimaryLang = LANG_CROATIAN;                    
    else if ( wsLanguage == L"sr" )
    {
        usPrimaryLang = LANG_SERBIAN;                     
        if ( wsCountry == L"VA" )
            usSubLang = SUBLANG_SERBIAN_LATIN;                 
        else if ( wsCountry == L"HR" )
            usSubLang = SUBLANG_SERBIAN_CYRILLIC;              
        else
            usSubLang = SUBLANG_DEFAULT;                       
    }
    else if ( wsLanguage == L"sk" )
        usPrimaryLang = LANG_SLOVAK;                      
    else if ( wsLanguage == L"sq" )
        usPrimaryLang = LANG_ALBANIAN;                    
    else if ( wsLanguage == L"sv" )
    {
        usPrimaryLang = LANG_SWEDISH;                     
        if ( wsCountry == L"SE" )
            usSubLang = SUBLANG_SWEDISH;                       
        else if ( wsCountry == L"FI" )
            usSubLang = SUBLANG_SWEDISH_FINLAND;               
        else
            usSubLang = SUBLANG_DEFAULT;                       
    }
    else if ( wsLanguage == L"th" )
        usPrimaryLang = LANG_THAI;                        
    else if ( wsLanguage == L"tr" )
        usPrimaryLang = LANG_TURKISH;                     
    else if ( wsLanguage == L"ur" )
    {
        usPrimaryLang = LANG_URDU;                        
        if ( wsCountry == L"PK" )
            usSubLang = SUBLANG_URDU_PAKISTAN;                 
        else if ( wsCountry == L"IN" )
            usSubLang = SUBLANG_URDU_INDIA;                    
        else
            usSubLang = SUBLANG_DEFAULT;                       
    }
    else if ( wsLanguage == L"in" )
        usPrimaryLang = LANG_INDONESIAN;                  
    else if ( wsLanguage == L"uk" )
        usPrimaryLang = LANG_UKRAINIAN;                   
    else if ( wsLanguage == L"be" )
        usPrimaryLang = LANG_BELARUSIAN;                  
    else if ( wsLanguage == L"sl" )
        usPrimaryLang = LANG_SLOVENIAN;                   
    else if ( wsLanguage == L"et" )
        usPrimaryLang = LANG_ESTONIAN;                    
    else if ( wsLanguage == L"lv" )
        usPrimaryLang = LANG_LATVIAN;                     
    else if ( wsLanguage == L"lt" )
    {
        usPrimaryLang = LANG_LITHUANIAN;                  
        if ( wsCountry == L"LT" )
            usSubLang = SUBLANG_LITHUANIAN;                    
        else
            usSubLang = SUBLANG_DEFAULT;                       
    }
    else if ( wsLanguage == L"fa" )
        usPrimaryLang = LANG_FARSI;                       
    else if ( wsLanguage == L"vi" )
        usPrimaryLang = LANG_VIETNAMESE;                  
    else if ( wsLanguage == L"hy" )
        usPrimaryLang = LANG_ARMENIAN;                    
    else if ( wsLanguage == L"az" )
        usPrimaryLang = LANG_AZERI;                       
    else if ( wsLanguage == L"eu" )
        usPrimaryLang = LANG_BASQUE;                      
    else if ( wsLanguage == L"mk" )
        usPrimaryLang = LANG_MACEDONIAN;                  
    else if ( wsLanguage == L"af" )
        usPrimaryLang = LANG_AFRIKAANS;                   
    else if ( wsLanguage == L"ka" )
        usPrimaryLang = LANG_GEORGIAN;                    
    else if ( wsLanguage == L"fo" )
        usPrimaryLang = LANG_FAEROESE;                    
    else if ( wsLanguage == L"hi" )
        usPrimaryLang = LANG_HINDI;                       
    else if ( wsLanguage == L"ms" )
    {
        usPrimaryLang = LANG_MALAY;                       
        if ( wsCountry == L"MY" )
            usSubLang = SUBLANG_MALAY_MALAYSIA;                
        else if ( wsCountry == L"BN" )
            usSubLang = SUBLANG_MALAY_BRUNEI_DARUSSALAM;       
        else
            usSubLang = SUBLANG_DEFAULT;                       
    }
    else if ( wsLanguage == L"kk" )
        usPrimaryLang = LANG_KAZAK;                       
    else if ( wsLanguage == L"ky" )
        usPrimaryLang = LANG_KYRGYZ;                      
    else if ( wsLanguage == L"sw" )
        usPrimaryLang = LANG_SWAHILI;                     
    else if ( wsLanguage == L"uz" )
    {
        usPrimaryLang = LANG_UZBEK;                       
        if ( wsCountry == L"UZ" )
            usSubLang = SUBLANG_UZBEK_LATIN;                   
        else if ( wsCountry == L"DE" )
            usSubLang = SUBLANG_UZBEK_CYRILLIC;                
        else
            usSubLang = SUBLANG_DEFAULT;                       
    }
    else if ( wsLanguage == L"tt" )
        usPrimaryLang = LANG_TATAR;                       
    else if ( wsLanguage == L"bn" )
        usPrimaryLang = LANG_BENGALI;                     
    else if ( wsLanguage == L"pa" )
        usPrimaryLang = LANG_PUNJABI;                     
    else if ( wsLanguage == L"gu" )
        usPrimaryLang = LANG_GUJARATI;                    
    else if ( wsLanguage == L"or" )
        usPrimaryLang = LANG_ORIYA;                       
    else if ( wsLanguage == L"ta" )
        usPrimaryLang = LANG_TAMIL;                       
    else if ( wsLanguage == L"te" )
        usPrimaryLang = LANG_TELUGU;                      
    else if ( wsLanguage == L"kn" )
        usPrimaryLang = LANG_KANNADA;                     
    else if ( wsLanguage == L"ml" )
        usPrimaryLang = LANG_MALAYALAM;                   
    else if ( wsLanguage == L"as" )
        usPrimaryLang = LANG_ASSAMESE;                    
    else if ( wsLanguage == L"mr" )
        usPrimaryLang = LANG_MARATHI;                     
    else if ( wsLanguage == L"sa" )
        usPrimaryLang = LANG_SANSKRIT;                    
    else if ( wsLanguage == L"mn" )
        usPrimaryLang = LANG_MONGOLIAN;                   
    else if ( wsLanguage == L"gl" )
        usPrimaryLang = LANG_GALICIAN;                    
    else if ( wsLanguage == L"sd" )
        usPrimaryLang = LANG_SINDHI;                      
    else if ( wsLanguage == L"ks" )
        usPrimaryLang = LANG_KASHMIRI;                    
    else if ( wsLanguage == L"ne" )
        usPrimaryLang = LANG_NEPALI;                      
    else
        return GetSystemDefaultLCID();                    

    return MAKELCID( MAKELANGID( usPrimaryLang, usSubLang ), SORT_DEFAULT );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
