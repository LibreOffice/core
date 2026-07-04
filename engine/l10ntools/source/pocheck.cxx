/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cassert>
#include <cstring>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <o3tl/string_view.hxx>
#include <rtl/character.hxx>
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
            if( !aLocalizedStyleNames.contains(aMsgStr) )
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
            if( !aLocalizedNumStyleNames.contains(aMsgStr) )
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
            if( !aLocalizedCoreFunctionNames.contains(aMsgStr) )
                aLocalizedCoreFunctionNames[aMsgStr] = 1;
            if( !aLocalizedFunctionNames.contains(aMsgStr) ) {
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
            if( aLocalizedCoreFunctionNames.contains(aMsgStr) )
                aMsgStr += "_ADD";
            if( !aLocalizedFunctionNames.contains(aMsgStr) ) {
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

static void printError(const OString& rPoPath, const OString& rLanguage, const PoEntry& rPoEntry, const OString& rError)
{
    std::cout << "ERROR: " << rError << std::endl
    << "File: " << rPoPath << std::endl
    << "Language: " << rLanguage << std::endl
    << "English:   " << rPoEntry.getMsgId() << std::endl
    << "Localized: " << rPoEntry.getMsgStr() << std::endl
    << std::endl;
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
            printError(aPoPath, aLanguage, aPoEntry, "Math symbol names must not contain spaces."_ostr);
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

// Some translated strings are rendered as HTML. A translation should only use
// the tags that appear in its English source; anything else is dropped.

// Collect the (lower-cased) HTML tag names used in a string. A tag is '<' or
// '</' immediately followed by a name, so text like "a < b" is not a tag.
static void collectHtmlTags(const OString& rText, std::set<OString>& rTags)
{
    const char* p = rText.getStr();
    const sal_Int32 nLen = rText.getLength();
    for (sal_Int32 i = 0; i < nLen; ++i)
    {
        if (p[i] != '<')
            continue;
        sal_Int32 j = i + 1;
        if (j < nLen && p[j] == '/')
            ++j;
        if (j >= nLen || !rtl::isAsciiAlpha(static_cast<unsigned char>(p[j])))
            continue;
        const sal_Int32 nStart = j;
        while (j < nLen && (rtl::isAsciiAlpha(static_cast<unsigned char>(p[j]))
                            || rtl::isAsciiDigit(static_cast<unsigned char>(p[j]))
                            || p[j] == '-'))
            ++j;
        rTags.insert(rText.copy(nStart, j - nStart).toAsciiLowerCase());
    }
}

// Detect script/iframe/... elements, on* handler attributes and
// javascript:/vbscript:/data: URL schemes in attribute values.
static bool containsActiveMarkup(const OString& rText)
{
    static const char* const aElements[]
        = { "script", "iframe", "object", "embed", "form", "meta",
            "link",   "style",  "svg",    "math",  "base", "applet" };
    static const char* const aSchemes[] = { "javascript", "vbscript", "data" };

    const char* p = rText.getStr();
    const sal_Int32 nLen = rText.getLength();
    for (sal_Int32 i = 0; i < nLen; ++i)
    {
        const char c = p[i];
        if (c == '<')
        {
            sal_Int32 j = i + 1;
            while (j < nLen && (p[j] == ' ' || p[j] == '\t'))
                ++j;
            if (j < nLen && p[j] == '/')
            {
                ++j;
                while (j < nLen && (p[j] == ' ' || p[j] == '\t'))
                    ++j;
            }
            for (const char* pElem : aElements)
            {
                if (rText.matchIgnoreAsciiCase(pElem, j))
                {
                    const sal_Int32 nEnd = j + static_cast<sal_Int32>(strlen(pElem));
                    const char cAfter = nEnd < nLen ? p[nEnd] : ' ';
                    if (!rtl::isAsciiAlpha(static_cast<unsigned char>(cAfter))
                        && !rtl::isAsciiDigit(static_cast<unsigned char>(cAfter)))
                        return true;
                }
            }
        }
        else if ((c == ' ' || c == '\t' || c == '\n' || c == '\r') && i + 2 < nLen
                 && (p[i + 1] == 'o' || p[i + 1] == 'O')
                 && (p[i + 2] == 'n' || p[i + 2] == 'N'))
        {
            sal_Int32 j = i + 3;
            while (j < nLen && rtl::isAsciiAlpha(static_cast<unsigned char>(p[j])))
                ++j;
            if (j > i + 3)
            {
                while (j < nLen && (p[j] == ' ' || p[j] == '\t'))
                    ++j;
                if (j < nLen && p[j] == '=')
                    return true;
            }
        }
        else if (c == '=')
        {
            sal_Int32 j = i + 1;
            while (j < nLen && (p[j] == ' ' || p[j] == '\t'))
                ++j;
            if (j < nLen && (p[j] == '"' || p[j] == '\''))
                ++j;
            while (j < nLen && (p[j] == ' ' || p[j] == '\t'))
                ++j;
            for (const char* pScheme : aSchemes)
            {
                if (rText.matchIgnoreAsciiCase(pScheme, j))
                {
                    sal_Int32 k = j + static_cast<sal_Int32>(strlen(pScheme));
                    while (k < nLen && (p[k] == ' ' || p[k] == '\t'))
                        ++k;
                    if (k < nLen && p[k] == ':')
                        return true;
                }
            }
        }
    }
    return false;
}

// Real HTML element names. The engine strings also use <word> placeholders
// like <none> or <AutoField> that translators legitimately translate; those
// are not HTML elements and are left alone.
static bool isHtmlElementName(const OString& rTag)
{
    static const std::set<OString> aElements = {
        "a"_ostr,        "abbr"_ostr,    "address"_ostr, "applet"_ostr,  "area"_ostr,
        "article"_ostr,  "aside"_ostr,   "audio"_ostr,   "b"_ostr,       "base"_ostr,
        "bdi"_ostr,      "bdo"_ostr,     "big"_ostr,     "blockquote"_ostr, "body"_ostr,
        "br"_ostr,       "button"_ostr,  "canvas"_ostr,  "caption"_ostr, "center"_ostr,
        "cite"_ostr,     "code"_ostr,    "col"_ostr,     "colgroup"_ostr, "data"_ostr,
        "datalist"_ostr, "dd"_ostr,      "del"_ostr,     "details"_ostr, "dfn"_ostr,
        "dialog"_ostr,   "div"_ostr,     "dl"_ostr,      "dt"_ostr,      "em"_ostr,
        "embed"_ostr,    "fieldset"_ostr,"figcaption"_ostr, "figure"_ostr, "font"_ostr,
        "footer"_ostr,   "form"_ostr,    "h1"_ostr,      "h2"_ostr,      "h3"_ostr,
        "h4"_ostr,       "h5"_ostr,      "h6"_ostr,      "head"_ostr,    "header"_ostr,
        "hr"_ostr,       "html"_ostr,    "i"_ostr,       "iframe"_ostr,  "img"_ostr,
        "input"_ostr,    "ins"_ostr,     "kbd"_ostr,     "label"_ostr,   "legend"_ostr,
        "li"_ostr,       "link"_ostr,    "main"_ostr,    "map"_ostr,     "mark"_ostr,
        "marquee"_ostr,  "math"_ostr,    "menu"_ostr,    "meta"_ostr,    "meter"_ostr,
        "nav"_ostr,      "noscript"_ostr,"object"_ostr,  "ol"_ostr,      "optgroup"_ostr,
        "option"_ostr,   "output"_ostr,  "p"_ostr,       "param"_ostr,   "picture"_ostr,
        "pre"_ostr,      "progress"_ostr,"q"_ostr,       "rp"_ostr,      "rt"_ostr,
        "ruby"_ostr,     "s"_ostr,       "samp"_ostr,    "script"_ostr,  "section"_ostr,
        "select"_ostr,   "slot"_ostr,    "small"_ostr,   "source"_ostr,  "span"_ostr,
        "strike"_ostr,   "strong"_ostr,  "style"_ostr,   "sub"_ostr,     "summary"_ostr,
        "sup"_ostr,      "svg"_ostr,     "table"_ostr,   "tbody"_ostr,   "td"_ostr,
        "template"_ostr, "textarea"_ostr,"tfoot"_ostr,   "th"_ostr,      "thead"_ostr,
        "time"_ostr,     "title"_ostr,   "tr"_ostr,      "track"_ostr,   "tt"_ostr,
        "u"_ostr,        "ul"_ostr,      "var"_ostr,     "video"_ostr,   "wbr"_ostr
    };
    return aElements.count(rTag) != 0;
}

// A source string that is a single angle-bracket token, e.g. <none> or
// <date/time>. Such placeholders are routinely translated (<kein>, <data/ora>)
// and their inner words may coincide with HTML element names, so the added-tag
// check is skipped for them (the active-markup check below still applies).
static bool isPlaceholderString(std::string_view aText)
{
    const std::string_view aTrimmed = o3tl::trim(aText);
    const size_t nLen = aTrimmed.size();
    if (nLen < 2 || aTrimmed.front() != '<' || aTrimmed.back() != '>')
        return false;
    for (size_t i = 1; i + 1 < nLen; ++i)
        if (aTrimmed[i] == '<' || aTrimmed[i] == '>')
            return false;
    return true;
}

// Return true (and a description) when the translation uses HTML that its
// English source does not.
static bool findHtmlDiscrepancy(const OString& rMsgId, const OString& rMsgStr, OString& rProblem)
{
    if (rMsgStr.isEmpty())
        return false;

    std::set<OString> aSrcTags;
    std::set<OString> aDstTags;
    collectHtmlTags(rMsgId, aSrcTags);
    collectHtmlTags(rMsgStr, aDstTags);

    OString aNewTags;
    if (!isPlaceholderString(rMsgId))
    {
        for (const OString& rTag : aDstTags)
        {
            if (!aSrcTags.count(rTag) && isHtmlElementName(rTag))
            {
                if (!aNewTags.isEmpty())
                    aNewTags += ", "_ostr;
                aNewTags += "<" + rTag + ">";
            }
        }
    }
    if (!aNewTags.isEmpty())
    {
        rProblem = "Translation adds HTML tag(s) not in the English source: " + aNewTags;
        return true;
    }

    if (containsActiveMarkup(rMsgStr) && !containsActiveMarkup(rMsgId))
    {
        rProblem = "Translation adds active markup not in the English source."_ostr;
        return true;
    }

    return false;
}

// Turn a po path into the "extracted from" hint used in the header comment.
static OString extractSourceHint(const OString& rPoPath, std::string_view aLanguage)
{
    const OString aPrefix
        = OString::Concat(getenv("SRC_ROOT")) + "/translations/source/" + aLanguage + "/";
    OString aHint = rPoPath.startsWith(aPrefix) ? rPoPath.copy(aPrefix.getLength()) : rPoPath;
    if (aHint.endsWith(".po"))
        aHint = aHint.copy(0, aHint.getLength() - 3);
    return aHint;
}

static void checkHtmlInPoFile(const OString& rPoPath, const OString& aLanguage)
{
    PoIfstream aPoInput;
    OString sPoHdrMsg;
    aPoInput.open(rPoPath, sPoHdrMsg);
    if (!aPoInput.isOpen())
    {
        std::cerr << "Warning: Cannot open " << rPoPath << std::endl;
        return;
    }

    std::vector<PoEntry> aBadEntries;
    for (;;)
    {
        PoEntry aPoEntry;
        aPoInput.readEntry(aPoEntry);
        if (aPoInput.eof())
            break;
        if (aPoEntry.isFuzzy() || aPoEntry.getMsgStr().isEmpty())
            continue;
        OString aProblem;
        if (findHtmlDiscrepancy(aPoEntry.getMsgId(), aPoEntry.getMsgStr(), aProblem))
        {
            printError(rPoPath, aLanguage, aPoEntry, aProblem);
            aBadEntries.push_back(aPoEntry);
        }
    }
    aPoInput.close();

    if (aBadEntries.empty())
        return;

    // Rewrite the file without the offending entries.
    PoOfstream aPoOutput;
    aPoOutput.open(rPoPath + ".new");
    PoHeader aHeader(extractSourceHint(rPoPath, aLanguage), sPoHdrMsg);
    aPoOutput.writeHeader(aHeader);

    aPoInput.open(rPoPath);
    if (!aPoInput.isOpen())
    {
        std::cerr << "Warning: Cannot reopen " << rPoPath << std::endl;
        aPoOutput.close();
        return;
    }
    for (;;)
    {
        PoEntry aPoEntry;
        aPoInput.readEntry(aPoEntry);
        if (aPoInput.eof())
            break;
        bool bDrop = false;
        for (const PoEntry& rBad : aBadEntries)
        {
            if (rBad.getMsgId() == aPoEntry.getMsgId() && rBad.getMsgCtxt() == aPoEntry.getMsgCtxt())
            {
                bDrop = true;
                break;
            }
        }
        if (!bDrop)
            aPoOutput.writeEntry(aPoEntry);
    }
    aPoInput.close();
    aPoOutput.close();

    OUString aPoPathURL;
    osl::FileBase::getFileURLFromSystemPath(OStringToOUString(rPoPath, RTL_TEXTENCODING_UTF8), aPoPathURL);
    osl::File::move(aPoPathURL + ".new", aPoPathURL);
}

// Recursively collect all .po files under rDirUrl, skipping helpcontent2.
static void collectPoFiles(const OUString& rDirUrl, std::vector<OString>& rPoPaths)
{
    osl::Directory aDir(rDirUrl);
    if (aDir.open() != osl::FileBase::E_None)
        return;
    for (;;)
    {
        osl::DirectoryItem aItem;
        if (aDir.getNextItem(aItem) != osl::FileBase::E_None)
            break;
        osl::FileStatus aStatus(osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileName
                                | osl_FileStatus_Mask_FileURL);
        if (aItem.getFileStatus(aStatus) != osl::FileBase::E_None)
            continue;
        const OUString aName = aStatus.getFileName();
        if (aStatus.getFileType() == osl::FileStatus::Directory)
        {
            if (aName == "helpcontent2")
                continue;
            collectPoFiles(aStatus.getFileURL(), rPoPaths);
        }
        else if (aName.endsWith(u".po"))
        {
            OUString aSysPath;
            if (osl::FileBase::getSystemPathFromFileURL(aStatus.getFileURL(), aSysPath)
                == osl::FileBase::E_None)
                rPoPaths.push_back(OUStringToOString(aSysPath, RTL_TEXTENCODING_UTF8));
        }
    }
    aDir.close();
}

static void checkUnexpectedHtml(const OString& aLanguage)
{
    const OString aLangRoot
        = OString::Concat(getenv("SRC_ROOT")) + "/translations/source/" + aLanguage;
    OUString aLangRootUrl;
    if (osl::FileBase::getFileURLFromSystemPath(
            OStringToOUString(aLangRoot, RTL_TEXTENCODING_UTF8), aLangRootUrl)
        != osl::FileBase::E_None)
        return;

    std::vector<OString> aPoPaths;
    collectPoFiles(aLangRootUrl, aPoPaths);
    for (const OString& rPoPath : aPoPaths)
        checkHtmlInPoFile(rPoPath, aLanguage);
}

int main()
{
    try
    {
        auto const env = getenv("ALL_LANGS");
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
             checkMathSymbolNames(aLanguage);
             checkUnexpectedHtml(aLanguage);
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
