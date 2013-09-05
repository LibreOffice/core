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
#include <list>
#include <vector>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <osl/file.hxx>
#include "po.hxx"

// Translated style names must be unique
static void checkStyleNames(OString aLanguage)
{
    std::map<OString,sal_uInt16> aLocalizedStyleNames;
    std::map<OString,sal_uInt16> aLocalizedNumStyleNames;
    std::list<PoEntry*> repeatedEntries;

    OString aPoPath = OString(getenv("SRC_ROOT")) +
                      "/translations/source/" +
                      aLanguage + "/sw/source/ui/utlui.po";
    PoIfstream aPoInput;
    aPoInput.open(aPoPath);
    if( !aPoInput.isOpen() )
        std::cerr << "Warning: Cannot open " << aPoPath << std::endl;

    for(;;)
    {
        PoEntry* aPoEntry = new PoEntry();
        aPoInput.readEntry(*aPoEntry);
        bool bRepeated = false;
        if( aPoInput.eof() )
            break;
        if( !aPoEntry->isFuzzy() && aPoEntry->getSourceFile() == "poolfmt.src" &&
            aPoEntry->getGroupId().startsWith("STR_POOLCOLL") )
        {
            OString aMsgStr = aPoEntry->getMsgStr();
            if( aMsgStr.isEmpty() )
                continue;
            if( aLocalizedStyleNames.find(aMsgStr) == aLocalizedStyleNames.end() )
                aLocalizedStyleNames[aMsgStr] = 1;
            else {
                aLocalizedStyleNames[aMsgStr]++;
                bRepeated = true;
            }
        }
        if( !aPoEntry->isFuzzy() && aPoEntry->getSourceFile() == "poolfmt.src" &&
            aPoEntry->getGroupId().startsWith("STR_POOLNUMRULE") )
        {
            OString aMsgStr = aPoEntry->getMsgStr();
            if( aMsgStr.isEmpty() )
                continue;
            if( aLocalizedNumStyleNames.find(aMsgStr) == aLocalizedNumStyleNames.end() )
                aLocalizedNumStyleNames[aMsgStr] = 1;
            else {
                aLocalizedNumStyleNames[aMsgStr]++;
                bRepeated = true;
            }
        }
        if (bRepeated)
            repeatedEntries.push_back(aPoEntry);
         else
            delete aPoEntry;

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
    aPoInput.open(aPoPath);
    if( !aPoInput.isOpen() )
        std::cerr << "Warning: Cannot open " << aPoPath << std::endl;
    PoOfstream aPoOutput;
    aPoOutput.open(aPoPath+".new");
    PoHeader aTmp("sw/source/ui/utlui");
    aPoOutput.writeHeader(aTmp);
    bool bAnyError = false;

    for(;;)
    {
        PoEntry aPoEntry;
        bool bError = false;
        aPoInput.readEntry(aPoEntry);
        if( aPoInput.eof() )
            break;
        for ( std::list<PoEntry*>::iterator it=repeatedEntries.begin(); it!=repeatedEntries.end(); ++it) {
            if ((*it)->getMsgId() == aPoEntry.getMsgId() && (*it)->getGroupId() == aPoEntry.getGroupId()) {
                bError = true;
                break;
            }
        }
        if (bError) {
            bAnyError = true;
        } else {
            aPoOutput.writeEntry(aPoEntry);
        }
    }
    aPoInput.close();
    aPoOutput.close();
    OUString aPoPathURL;
    osl::FileBase::getFileURLFromSystemPath(OStringToOUString(aPoPath, RTL_TEXTENCODING_UTF8), aPoPathURL);
    if( bAnyError )
        osl::File::move(aPoPathURL + ".new", aPoPathURL);
    else
        osl::File::remove(aPoPathURL + ".new");

}

// Translated spreadsheet function names must be unique
static void checkFunctionNames(OString aLanguage)
{
    std::map<OString,sal_uInt16> aLocalizedFunctionNames;
    std::map<OString,sal_uInt16> aLocalizedCoreFunctionNames;
    //
    std::list<PoEntry*> repeatedEntries;

    OString aPoPaths[4];
    OUString aPoPathURL;

    aPoPaths[0] = OString(getenv("SRC_ROOT")) +
                      "/translations/source/" +
                      aLanguage +
                      "/formula/source/core/resource.po";
    PoIfstream aPoInput;
    aPoInput.open(aPoPaths[0]);
    if( !aPoInput.isOpen() )
        std::cerr << "Warning: Cannot open " << aPoPaths[0] << std::endl;

    for(;;)
    {
        PoEntry* aPoEntry = new PoEntry();
        aPoInput.readEntry(*aPoEntry);
        if( aPoInput.eof() )
            break;
        if( !aPoEntry->isFuzzy() && aPoEntry->getGroupId() == "RID_STRLIST_FUNCTION_NAMES" )
        {
            OString aMsgStr = aPoEntry->getMsgStr();
            if( aMsgStr.isEmpty() )
                continue;
            if( aLocalizedCoreFunctionNames.find(aMsgStr) == aLocalizedCoreFunctionNames.end() )
                aLocalizedCoreFunctionNames[aMsgStr] = 1;
            if( aLocalizedFunctionNames.find(aMsgStr) == aLocalizedFunctionNames.end() ) {
                aLocalizedFunctionNames[aMsgStr] = 1;
                delete aPoEntry;
            } else {
                aLocalizedFunctionNames[aMsgStr]++;
                repeatedEntries.push_back(aPoEntry);
            }
        }
    }
    aPoInput.close();

    aPoPaths[1] = OString(getenv("SRC_ROOT")) +
        "/translations/source/" +
        aLanguage +
        "/scaddins/source/analysis.po";
    aPoInput.open(aPoPaths[1]);
    if( !aPoInput.isOpen() )
        std::cerr << "Warning: Cannot open " << aPoPaths[1] << std::endl;

    for(;;)
    {
        PoEntry* aPoEntry = new PoEntry();
        aPoInput.readEntry(*aPoEntry);
        if( aPoInput.eof() )
            break;
        if( !aPoEntry->isFuzzy() && aPoEntry->getGroupId() == "RID_ANALYSIS_FUNCTION_NAMES" )
        {
            OString aMsgStr = aPoEntry->getMsgStr();
            if( aMsgStr.isEmpty() )
                continue;
            if( aLocalizedCoreFunctionNames.find(aMsgStr) != aLocalizedCoreFunctionNames.end() )
                aMsgStr += "_ADD";
            if( aLocalizedFunctionNames.find(aMsgStr) == aLocalizedFunctionNames.end() ) {
                aLocalizedFunctionNames[aMsgStr] = 1;
                delete aPoEntry;
            } else {
                aLocalizedFunctionNames[aMsgStr]++;
                repeatedEntries.push_back(aPoEntry);
            }
        }
    }
    aPoInput.close();


    aPoPaths[2] = OString(getenv("SRC_ROOT")) +
              "/translations/source/" +
               aLanguage +
              "/scaddins/source/datefunc.po";
    aPoInput.open(aPoPaths[2]);
    if( !aPoInput.isOpen() )
        std::cerr << "Warning: Cannot open " << aPoPaths[2] << std::endl;

    for(;;)
    {
        PoEntry* aPoEntry = new PoEntry();
        aPoInput.readEntry(*aPoEntry);
        if( aPoInput.eof() )
            break;
        if( !aPoEntry->isFuzzy() && aPoEntry->getGroupId() == "RID_DATE_FUNCTION_NAMES" )
        {
            OString aMsgStr = aPoEntry->getMsgStr();
            if( aMsgStr.isEmpty() )
                continue;
            if( aLocalizedCoreFunctionNames.find(aMsgStr) != aLocalizedCoreFunctionNames.end() )
                aMsgStr += "_ADD";
            if( aLocalizedFunctionNames.find(aMsgStr) == aLocalizedFunctionNames.end() ) {
                aLocalizedFunctionNames[aMsgStr] = 1;
                delete aPoEntry;
            } else {
                aLocalizedFunctionNames[aMsgStr]++;
                repeatedEntries.push_back(aPoEntry);
            }
        }
    }
    aPoInput.close();

    aPoPaths[3] = OString(getenv("SRC_ROOT")) +
              "/translations/source/" +
               aLanguage +
              "/scaddins/source/pricing.po";
    aPoInput.open(aPoPaths[3]);
    if( !aPoInput.isOpen() )
        std::cerr << "Warning: Cannot open " << aPoPaths[3] << std::endl;

    for(;;)
    {
        PoEntry* pPoEntry = new PoEntry();
        aPoInput.readEntry(*pPoEntry);
        if( aPoInput.eof() )
        {
            delete pPoEntry;
            break;
        }

        if( !pPoEntry->isFuzzy() && pPoEntry->getGroupId() == "RID_PRICING_FUNCTION_NAMES" )
        {
            OString aMsgStr = pPoEntry->getMsgStr();
            if( aMsgStr.isEmpty() )
                continue;
            if( aLocalizedCoreFunctionNames.find(aMsgStr) != aLocalizedCoreFunctionNames.end() )
                aMsgStr += "_ADD";
            if( aLocalizedFunctionNames.find(aMsgStr) == aLocalizedFunctionNames.end() ) {
                aLocalizedFunctionNames[aMsgStr] = 1;
                delete pPoEntry;
            } else {
                aLocalizedFunctionNames[aMsgStr]++;
                repeatedEntries.push_back(pPoEntry);
            }
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
    //
    for (int i=0;i<4;i++)
    {
        aPoInput.open(aPoPaths[i]);
        if( !aPoInput.isOpen() )
            std::cerr << "Warning: Cannot open " << aPoPaths[i] << std::endl;
        PoOfstream aPoOutput;
        aPoOutput.open(aPoPaths[i]+".new");

        switch (i)
        {
        case 0:
        {
            PoHeader hd(OString("formula/source/core/resource"));
            aPoOutput.writeHeader(hd);
            break;
        }
        case 1:
        {
            PoHeader hd(OString("scaddins/source/analysis"));
            aPoOutput.writeHeader(hd);
            break;
        }
        case 2:
        {
            PoHeader hd(OString("scaddins/source/datefunc"));
            aPoOutput.writeHeader(hd);
            break;
        }
        case 3:
        {
            PoHeader hd(OString("scaddins/source/pricing"));
            aPoOutput.writeHeader(hd);
            break;
        }
        }
        bool bAnyError = false;

        for(;;)
        {
            PoEntry aPoEntry;
            bool bError = false;
            aPoInput.readEntry(aPoEntry);
            if( aPoInput.eof() )
                break;
            for ( std::list<PoEntry*>::iterator it=repeatedEntries.begin(); it!=repeatedEntries.end(); ++it)
            {
                if ((*it)->getMsgId() == aPoEntry.getMsgId() && (*it)->getGroupId() == aPoEntry.getGroupId())
                {
                    bError = true;
                    break;
                }
            }
            if (bError)
            {
                bAnyError = true;
            }
            else
            {
                aPoOutput.writeEntry(aPoEntry);
            }
        }
        aPoInput.close();
        aPoOutput.close();
        osl::FileBase::getFileURLFromSystemPath(OStringToOUString(aPoPaths[i], RTL_TEXTENCODING_UTF8), aPoPathURL);
        if( bAnyError )
            osl::File::move(aPoPathURL + ".new", aPoPathURL);
        else
            osl::File::remove(aPoPathURL + ".new");
    }
}

// In instsetoo_native/inc_openoffice/windows/msi_languages.po
// where an en-US string ends with '|', translation must end
// with '|', too.
static void checkVerticalBar(OString aLanguage)
{
    OString aPoPath = OString(getenv("SRC_ROOT")) +
                      "/translations/source/" +
                      aLanguage +
                      "/instsetoo_native/inc_openoffice/windows/msi_languages.po";
    PoIfstream aPoInput;
    aPoInput.open(aPoPath);
    PoOfstream aPoOutput;
    aPoOutput.open(aPoPath+".new");
    if( !aPoInput.isOpen() )
        std::cerr << "Warning: Cannot open " << aPoPath << std::endl;
    PoHeader aTmp("instsetoo_native/inc_openoffice/windows/msi_languages");
    aPoOutput.writeHeader(aTmp);
    bool bError = false;

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
            bError = true;
        }
        else
            aPoOutput.writeEntry(aPoEntry);
    }
    aPoInput.close();
    aPoOutput.close();
    OUString aPoPathURL;
    osl::FileBase::getFileURLFromSystemPath(OStringToOUString(aPoPath, RTL_TEXTENCODING_UTF8), aPoPathURL);
    if( bError )
        osl::File::move(aPoPathURL + ".new", aPoPathURL);
    else
        osl::File::remove(aPoPathURL + ".new");
}

// In starmath/source.po Math symbol names (from symbol.src)
// must not contain spaces
static void checkMathSymbolNames(OString aLanguage)
{
    OString aPoPath = OString(getenv("SRC_ROOT")) +
                      "/translations/source/" +
                      aLanguage +
                      "/starmath/source.po";
    PoIfstream aPoInput;
    aPoInput.open(aPoPath);
    PoOfstream aPoOutput;
    aPoOutput.open(aPoPath+".new");
    if( !aPoInput.isOpen() )
        std::cerr << "Warning: Cannot open " << aPoPath << std::endl;
    PoHeader aTmp("starmath/source");
    aPoOutput.writeHeader(aTmp);
    bool bError = false;

    for(;;)
    {
        PoEntry aPoEntry;
        aPoInput.readEntry(aPoEntry);
        if( aPoInput.eof() )
            break;
        if( !aPoEntry.isFuzzy() && aPoEntry.getGroupId() == "RID_UI_SYMBOL_NAMES" &&
            !aPoEntry.getMsgStr().isEmpty() && (aPoEntry.getMsgStr().indexOf(" ") != -1) )
        {
            std::cout << "ERROR: Math symbol names must not contain spaces.\n" <<
                "File: " << aPoPath << std::endl <<
                "Language: " << aLanguage << std::endl <<
                "English:   " << aPoEntry.getMsgId() << std::endl <<
                "Localized: " << aPoEntry.getMsgStr() << std::endl << std::endl;
            bError = true;
        }
        else
            aPoOutput.writeEntry(aPoEntry);
    }
    aPoInput.close();
    aPoOutput.close();
    OUString aPoPathURL;
    osl::FileBase::getFileURLFromSystemPath(OStringToOUString(aPoPath, RTL_TEXTENCODING_UTF8), aPoPathURL);
    if( bError )
        osl::File::move(aPoPathURL + ".new", aPoPathURL);
    else
        osl::File::remove(aPoPathURL + ".new");
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
         checkMathSymbolNames(aLanguage);
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
