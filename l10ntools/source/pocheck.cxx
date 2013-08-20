/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <iostream>
#include <map>
#include <rtl/string.hxx>
#include "po.hxx"

// Translated style names must be unique
static void checkStyleNames(OString aLanguage)
{
    std::map<OString,sal_uInt16> aLocalizedStyleNames;
    std::map<OString,sal_uInt16> aLocalizedNumStyleNames;
    OString aPoPath = OString(getenv("SRC_ROOT")) +
                      "/translations/source/"
                      aLanguage + "/sw/source/ui/utlui.po";
    PoIfstream aPoInput;
    aPoInput.open(aPoPath);
    if( !aPoInput.isOpen() )
        std::cerr << "Warning: Cannot open " << aPoPath << std::endl;

    for(;;)
    {
        PoEntry aPoEntry;
        aPoInput.readEntry(aPoEntry);
        if( aPoInput.eof() )
            break;
        if( !aPoEntry.isFuzzy() && aPoEntry.getSourceFile() == "poolfmt.src" &&
            aPoEntry.getGroupId().startsWith("STR_POOLCOLL") )
        {
            OString aMsgStr = aPoEntry.getMsgStr();
            if( aMsgStr.isEmpty() )
                continue;
            if( aLocalizedStyleNames.find(aMsgStr) == aLocalizedStyleNames.end() )
                aLocalizedStyleNames[aMsgStr] = 1;
            else
                aLocalizedStyleNames[aMsgStr]++;
        }
        if( !aPoEntry.isFuzzy() && aPoEntry.getSourceFile() == "poolfmt.src" &&
            aPoEntry.getGroupId().startsWith("STR_POOLNUMRULE") )
        {
            OString aMsgStr = aPoEntry.getMsgStr();
            if( aMsgStr.isEmpty() )
                continue;
            if( aLocalizedNumStyleNames.find(aMsgStr) == aLocalizedNumStyleNames.end() )
                aLocalizedNumStyleNames[aMsgStr] = 1;
            else
                aLocalizedNumStyleNames[aMsgStr]++;
        }
    }
    aPoInput.close();

    for( std::map<OString,sal_uInt16>::iterator it=aLocalizedStyleNames.begin(); it!=aLocalizedStyleNames.end(); ++it)
    {
        if( it->second > 1 )
        {
            std::cout << "ERROR: Style name translations must be unique in:\n" <<
                aPoPath << "\nLanguage: " << aLanguage << "\nDuplicated translation is: " << it->first <<
                "\nSee STR_POOLCOLL_*\n\n";
        }
    }
    for( std::map<OString,sal_uInt16>::iterator it=aLocalizedNumStyleNames.begin(); it!=aLocalizedNumStyleNames.end(); ++it)
    {
        if( it->second > 1 )
        {
            std::cout << "ERROR: Style name translations must be unique in:\n" <<
                aPoPath << "\nLanguage: " << aLanguage << "\nDuplicated translation is: " << it->first <<
                "\nSee STR_POOLNUMRULE_*\n\n";
        }
    }
}

// Translated spreadsheet function names must be unique
static void checkFunctionNames(OString aLanguage)
{
    std::map<OString,sal_uInt16> aLocalizedFunctionNames;
    std::map<OString,sal_uInt16> aLocalizedCoreFunctionNames;
    OString aPoPath = OString(getenv("SRC_ROOT")) +
                      "/translations/source/"
                      aLanguage
                      "/formula/source/core/resource.po";
    PoIfstream aPoInput;
    aPoInput.open(aPoPath);
    if( !aPoInput.isOpen() )
        std::cerr << "Warning: Cannot open " << aPoPath << std::endl;

    for(;;)
    {
        PoEntry aPoEntry;
        aPoInput.readEntry(aPoEntry);
        if( aPoInput.eof() )
            break;
        if( !aPoEntry.isFuzzy() && aPoEntry.getGroupId() == "RID_STRLIST_FUNCTION_NAMES" )
        {
            OString aMsgStr = aPoEntry.getMsgStr();
            if( aMsgStr.isEmpty() )
                continue;
            if( aLocalizedCoreFunctionNames.find(aMsgStr) == aLocalizedCoreFunctionNames.end() )
                aLocalizedCoreFunctionNames[aMsgStr] = 1;
            if( aLocalizedFunctionNames.find(aMsgStr) == aLocalizedFunctionNames.end() )
                aLocalizedFunctionNames[aMsgStr] = 1;
            else
                aLocalizedFunctionNames[aMsgStr]++;
        }
    }
    aPoInput.close();

    aPoPath = OString(getenv("SRC_ROOT")) +
        "/translations/source/"
        aLanguage
        "/scaddins/source/analysis.po";
    aPoInput.open(aPoPath);
    if( !aPoInput.isOpen() )
        std::cerr << "Warning: Cannot open " << aPoPath << std::endl;

    for(;;)
    {
        PoEntry aPoEntry;
        aPoInput.readEntry(aPoEntry);
        if( aPoInput.eof() )
            break;
        if( !aPoEntry.isFuzzy() && aPoEntry.getGroupId() == "RID_ANALYSIS_FUNCTION_NAMES" )
        {
            OString aMsgStr = aPoEntry.getMsgStr();
            if( aMsgStr.isEmpty() )
                continue;
            if( aLocalizedCoreFunctionNames.find(aMsgStr) != aLocalizedCoreFunctionNames.end() )
                aMsgStr += "_ADD";
            if( aLocalizedFunctionNames.find(aMsgStr) == aLocalizedFunctionNames.end() )
                aLocalizedFunctionNames[aMsgStr] = 1;
            else
                aLocalizedFunctionNames[aMsgStr]++;
        }
    }
    aPoInput.close();

    aPoPath = OString(getenv("SRC_ROOT")) +
              "/translations/source/"
               aLanguage
              "/scaddins/source/datefunc.po";
    aPoInput.open(aPoPath);
    if( !aPoInput.isOpen() )
        std::cerr << "Warning: Cannot open " << aPoPath << std::endl;

    for(;;)
    {
        PoEntry aPoEntry;
        aPoInput.readEntry(aPoEntry);
        if( aPoInput.eof() )
            break;
        if( !aPoEntry.isFuzzy() && aPoEntry.getGroupId() == "RID_DATE_FUNCTION_NAMES" )
        {
            OString aMsgStr = aPoEntry.getMsgStr();
            if( aMsgStr.isEmpty() )
                continue;
            if( aLocalizedCoreFunctionNames.find(aMsgStr) != aLocalizedCoreFunctionNames.end() )
                aMsgStr += "_ADD";
            if( aLocalizedFunctionNames.find(aMsgStr) == aLocalizedFunctionNames.end() )
                aLocalizedFunctionNames[aMsgStr] = 1;
            else
                aLocalizedFunctionNames[aMsgStr]++;
        }
    }
    aPoInput.close();

    aPoPath = OString(getenv("SRC_ROOT")) +
              "/translations/source/"
               aLanguage
              "/scaddins/source/pricing.po";
    aPoInput.open(aPoPath);
    if( !aPoInput.isOpen() )
        std::cerr << "Warning: Cannot open " << aPoPath << std::endl;

    for(;;)
    {
        PoEntry aPoEntry;
        aPoInput.readEntry(aPoEntry);
        if( aPoInput.eof() )
            break;
        if( !aPoEntry.isFuzzy() && aPoEntry.getGroupId() == "RID_PRICING_FUNCTION_NAMES" )
        {
            OString aMsgStr = aPoEntry.getMsgStr();
            if( aMsgStr.isEmpty() )
                continue;
            if( aLocalizedCoreFunctionNames.find(aMsgStr) != aLocalizedCoreFunctionNames.end() )
                aMsgStr += "_ADD";
            if( aLocalizedFunctionNames.find(aMsgStr) == aLocalizedFunctionNames.end() )
                aLocalizedFunctionNames[aMsgStr] = 1;
            else
                aLocalizedFunctionNames[aMsgStr]++;
        }
    }
    aPoInput.close();
    for( std::map<OString,sal_uInt16>::iterator it=aLocalizedFunctionNames.begin(); it!=aLocalizedFunctionNames.end(); ++it)
    {
        if( it->second > 1 )
        {
            std::cout << "ERROR: Spreadsheet function name translations must be unique.\n" <<
                "Language: " << aLanguage <<
                "\nDuplicated translation is: " << it->first << "\n\n";
        }
    }
}

// In instsetoo_native/inc_openoffice/windows/msi_languages.po
// where an en-US string ends with '|', translation must end
// with '|', too.
static void checkVerticalBar(OString aLanguage)
{
    OString aPoPath = OString(getenv("SRC_ROOT")) +
                      "/translations/source/"
                      aLanguage
                      "/instsetoo_native/inc_openoffice/windows/msi_languages.po";
    PoIfstream aPoInput;
    aPoInput.open(aPoPath);
    if( !aPoInput.isOpen() )
        std::cerr << "Warning: Cannot open " << aPoPath << std::endl;

    for(;;)
    {
        PoEntry aPoEntry;
        aPoInput.readEntry(aPoEntry);
        if( aPoInput.eof() )
            break;
        if( !aPoEntry.isFuzzy() && aPoEntry.getMsgId().endsWith("|") &&
            !aPoEntry.getMsgStr().isEmpty() && !aPoEntry.getMsgStr().endsWith("|") )
        {
            std::cout << "ERROR: Missing '|' character at the end of translated string.\n" <<
                "It causes runtime error in installer.\n" <<
                "File: " << aPoPath << std::endl <<
                "Language: " << aLanguage << std::endl <<
                "English:   " << aPoEntry.getMsgId() << std::endl <<
                "Localized: " << aPoEntry.getMsgStr() << std::endl << std::endl;
        }
    }
    aPoInput.close();
}

int main()
{
    OString aLanguages(getenv("ALL_LANGS"));
    if( aLanguages.isEmpty() )
    {
        std::cerr << "Usage: make cmd cmd=solver/*/bin/pocheck\n";
        return 1;
    }
    for(sal_Int32 i = 1;;++i) // skip en-US
    {
         OString aLanguage = aLanguages.getToken(i,' ');
         if( aLanguage.isEmpty() )
             break;
         if( aLanguage == "qtz" )
             continue;
         checkStyleNames(aLanguage);
         checkFunctionNames(aLanguage);
         checkVerticalBar(aLanguage);
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
