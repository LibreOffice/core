/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cassert>
#include <iostream>
#include <map>
#include <vector>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <osl/file.hxx>
#include <po.hxx>

// Translated style names must be unique
static void checkStyleNames(const OString& aLanguage)
{
    std::map<OString,sal_uInt16> aLocalizedStyleNames;
    std::map<OString,sal_uInt16> aLocalizedNumStyleNames;
    std::vector<PoEntry> repeatedEntries;

    OString aPoPath = OString::Concat(getenv("SRC_ROOT")) +
                      "/translations/source/" +
                      aLanguage + "/sw/messages.po";
    PoIfstream aPoInput;
    aPoInput.open(aPoPath);
    if( !aPoInput.isOpen() )
    {
        std::cerr << "Warning: Cannot open " << aPoPath << std::endl;
        return;
    }

    for(;;)
    {
        PoEntry aPoEntry;
        aPoInput.readEntry(aPoEntry);
        bool bRepeated = false;
        if( aPoInput.eof() )
        {
            break;
        }

        if( !aPoEntry.isFuzzy() && aPoEntry.getMsgCtxt().startsWith("STR_POOLCOLL") )
        {
            const OString& aMsgStr = aPoEntry.getMsgStr();
            if( aMsgStr.isEmpty() )
                continue;
            if( aLocalizedStyleNames.find(aMsgStr) == aLocalizedStyleNames.end() )
                aLocalizedStyleNames[aMsgStr] = 1;
            else {
                aLocalizedStyleNames[aMsgStr]++;
                bRepeated = true;
            }
        }
        if( !aPoEntry.isFuzzy() && aPoEntry.getMsgCtxt().startsWith("STR_POOLNUMRULE") )
        {
            const OString& aMsgStr = aPoEntry.getMsgStr();
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
    }
    aPoInput.close();

    for (auto const& localizedStyleName : aLocalizedStyleNames)
    {
        if( localizedStyleName.second > 1 )
        {
            std::cout << "ERROR: Style name translations must be unique in:\n" <<
                aPoPath << "\nLanguage: " << aLanguage << "\nDuplicated translation is: " << localizedStyleName.first <<
                "\nSee STR_POOLCOLL_*\n\n";
        }
    }
    for (auto const& localizedNumStyleName : aLocalizedNumStyleNames)
    {
        if( localizedNumStyleName.second > 1 )
        {
            std::cout << "ERROR: Style name translations must be unique in:\n" <<
                aPoPath << "\nLanguage: " << aLanguage << "\nDuplicated translation is: " << localizedNumStyleName.first <<
                "\nSee STR_POOLNUMRULE_*\n\n";
        }
    }
    OString sPoHdrMsg;
    aPoInput.open(aPoPath, sPoHdrMsg);
    if( !aPoInput.isOpen() )
    {
        std::cerr << "Warning: Cannot open " << aPoPath << std::endl;
        return;
    }
    PoOfstream aPoOutput;
    aPoOutput.open(aPoPath+".new");
    PoHeader aTmp("sw/inc", sPoHdrMsg);
    aPoOutput.writeHeader(aTmp);
    bool bAnyError = false;

    for(;;)
    {
        PoEntry aPoEntry;
        bool bError = false;
        aPoInput.readEntry(aPoEntry);
        if( aPoInput.eof() )
            break;
        for (auto const& repeatedEntry : repeatedEntries)
        {
            if (repeatedEntry.getMsgId() == aPoEntry.getMsgId() && repeatedEntry.getMsgCtxt() == aPoEntry.getMsgCtxt()) {
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
static void checkFunctionNames(const OString& aLanguage)
{
    std::map<OString,sal_uInt16> aLocalizedFunctionNames;
    std::map<OString,sal_uInt16> aLocalizedCoreFunctionNames;

    std::vector<PoEntry> repeatedEntries;

    OString aPoPaths[2];
    OUString aPoPathURL;

    aPoPaths[0] = OString::Concat(getenv("SRC_ROOT")) +
                      "/translations/source/" +
                      aLanguage +
                      "/formula/messages.po";
    PoIfstream aPoInput;
    OString sPoHdrMsg;
    aPoInput.open(aPoPaths[0], sPoHdrMsg);
    if( !aPoInput.isOpen() )
    {
        std::cerr << "Warning: Cannot open " << aPoPaths[0] << std::endl;
        return;
    }

    for(;;)
    {
        PoEntry aPoEntry;
        aPoInput.readEntry(aPoEntry);
        if( aPoInput.eof() )
            break;
        if( !aPoEntry.isFuzzy() && aPoEntry.getMsgCtxt() == "RID_STRLIST_FUNCTION_NAMES" )
        {
            const OString& aMsgStr = aPoEntry.getMsgStr();
            if( aMsgStr.isEmpty() )
                continue;
            if( aLocalizedCoreFunctionNames.find(aMsgStr) == aLocalizedCoreFunctionNames.end() )
                aLocalizedCoreFunctionNames[aMsgStr] = 1;
            if( aLocalizedFunctionNames.find(aMsgStr) == aLocalizedFunctionNames.end() ) {
                aLocalizedFunctionNames[aMsgStr] = 1;
            } else {
                aLocalizedFunctionNames[aMsgStr]++;
                repeatedEntries.push_back(aPoEntry);
            }
        }
    }
    aPoInput.close();

    aPoPaths[1] = OString::Concat(getenv("SRC_ROOT")) +
        "/translations/source/" +
        aLanguage +
        "/scaddins/messages.po";
    aPoInput.open(aPoPaths[1]);
    if( !aPoInput.isOpen() )
    {
        std::cerr << "Warning: Cannot open " << aPoPaths[1] << std::endl;
        return;
    }

    for(;;)
    {
        PoEntry aPoEntry;
        aPoInput.readEntry(aPoEntry);
        if( aPoInput.eof() )
            break;
        if( !aPoEntry.isFuzzy() && aPoEntry.getMsgCtxt().startsWith("ANALYSIS_FUNCNAME") )
        {
            OString aMsgStr = aPoEntry.getMsgStr();
            if( aMsgStr.isEmpty() )
                continue;
            if( aLocalizedCoreFunctionNames.find(aMsgStr) != aLocalizedCoreFunctionNames.end() )
                aMsgStr += "_ADD";
            if( aLocalizedFunctionNames.find(aMsgStr) == aLocalizedFunctionNames.end() ) {
                aLocalizedFunctionNames[aMsgStr] = 1;
            } else {
                aLocalizedFunctionNames[aMsgStr]++;
                repeatedEntries.push_back(aPoEntry);
            }
        }
    }
    aPoInput.close();

    for (auto const& localizedFunctionName : aLocalizedFunctionNames)
    {
        if( localizedFunctionName.second > 1 )
        {
            std::cout
                << ("ERROR: Spreadsheet function name translations must be"
                    " unique.\nLanguage: ")
                << aLanguage << "\nDuplicated translation is: " << localizedFunctionName.first
                << "\n\n";
        }
    }

    for (int i=0;i<2;i++)
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
            PoHeader hd("formula/inc", sPoHdrMsg);
            aPoOutput.writeHeader(hd);
            break;
        }
        case 1:
        {
            PoHeader hd("scaddins/inc", sPoHdrMsg);
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
            for (auto const& repeatedEntry : repeatedEntries)
            {
                if (repeatedEntry.getMsgId() == aPoEntry.getMsgId() && repeatedEntry.getMsgCtxt() == aPoEntry.getMsgCtxt())
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
static void checkVerticalBar(const OString& aLanguage)
{
    OString aPoPath = OString::Concat(getenv("SRC_ROOT")) +
                      "/translations/source/" +
                      aLanguage +
                      "/instsetoo_native/inc_openoffice/windows/msi_languages.po";
    PoIfstream aPoInput;
    aPoInput.open(aPoPath);
    if( !aPoInput.isOpen() )
    {
        std::cerr << "Warning: Cannot open " << aPoPath << std::endl;
        return;
    }
    PoOfstream aPoOutput;
    aPoOutput.open(aPoPath+".new");
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
            std::cout
                << ("ERROR: Missing '|' character at the end of translated"
                    " string.\nIt causes runtime error in installer.\nFile: ")
                << aPoPath << std::endl
                << "Language: " << aLanguage << std::endl
                << "English:   " << aPoEntry.getMsgId() << std::endl
                << "Localized: " << aPoEntry.getMsgStr() << std::endl
                << std::endl;
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
static void checkMathSymbolNames(const OString& aLanguage)
{
    OString aPoPath = OString::Concat(getenv("SRC_ROOT")) +
                      "/translations/source/" +
                      aLanguage +
                      "/starmath/messages.po";
    PoIfstream aPoInput;
    aPoInput.open(aPoPath);
    if( !aPoInput.isOpen() )
    {
        std::cerr << "Warning: Cannot open " << aPoPath << std::endl;
        return;
    }
    PoOfstream aPoOutput;
    aPoOutput.open(aPoPath+".new");
    PoHeader aTmp("starmath/inc");
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
            std::cout
                << "ERROR: Math symbol names must not contain spaces.\nFile: "
                << aPoPath << std::endl
                << "Language: " << aLanguage << std::endl
                << "English:   " << aPoEntry.getMsgId() << std::endl
                << "Localized: " << aPoEntry.getMsgStr() << std::endl
                << std::endl;
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
    try
    {
        char* const env = getenv("ALL_LANGS");
        assert(env != nullptr);
        OString aLanguages(env);
        if( aLanguages.isEmpty() )
        {
            std::cerr << "Usage: LD_LIBRARY_PATH=instdir/program make cmd cmd=workdir/LinkTarget/Executable/pocheck\n";
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
    catch (std::exception& e)
    {
        std::cerr << "pocheck: exception " << e.what() << std::endl;
        return 1;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
