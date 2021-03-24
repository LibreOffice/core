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

#if defined(_WIN32)
#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include <osl/thread.h>
#include <osl/file.hxx>
#include <osl/process.h>
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <unotools/bootstrap.hxx>
#include <unotools/lingucfg.hxx>
#include <unotools/pathoptions.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/ustring.hxx>
#include <rtl/string.hxx>
#include <rtl/tencinfo.h>
#include <linguistic/misc.hxx>

#include <set>
#include <vector>
#include <string.h>

#include "lingutil.hxx"

#include <sal/macros.h>

using namespace ::com::sun::star;

#if defined(_WIN32)
OString Win_AddLongPathPrefix( const OString &rPathName )
{
#define WIN32_LONG_PATH_PREFIX "\\\\?\\"
  if (!rPathName.match(WIN32_LONG_PATH_PREFIX)) return WIN32_LONG_PATH_PREFIX + rPathName;
  return rPathName;
}
#endif //defined(_WIN32)

#if defined SYSTEM_DICTS || defined IOS
// find old style dictionaries in system directories
static void GetOldStyleDicsInDir(
    OUString const & aSystemDir, OUString const & aFormatName,
    OUString const & aSystemSuffix, OUString const & aSystemPrefix,
    std::set< OUString >& aDicLangInUse,
    std::vector< SvtLinguConfigDictionaryEntry >& aRes )
{
    osl::Directory aSystemDicts(aSystemDir);
    if (aSystemDicts.open() != osl::FileBase::E_None)
        return;

    osl::DirectoryItem aItem;
    osl::FileStatus aFileStatus(osl_FileStatus_Mask_FileURL);
    while (aSystemDicts.getNextItem(aItem) == osl::FileBase::E_None)
    {
        aItem.getFileStatus(aFileStatus);
        OUString sPath = aFileStatus.getFileURL();
        if (sPath.endsWith(aSystemSuffix))
        {
            sal_Int32 nStartIndex = sPath.lastIndexOf('/') + 1;
            if (!sPath.match(aSystemPrefix, nStartIndex))
                continue;
            OUString sChunk = sPath.copy(nStartIndex + aSystemPrefix.getLength(),
                sPath.getLength() - aSystemSuffix.getLength() -
                nStartIndex - aSystemPrefix.getLength());
            if (sChunk.isEmpty())
                continue;

            // We prefer (now) to use language tags.
            // Avoid feeding in the older LANG_REGION scheme to the BCP47
            // ctor as that triggers use of liblangtag and initializes its
            // database which we do not want during startup. Convert
            // instead.
            sChunk = sChunk.replace( '_', '-');

            // There's a known exception to the rule, the dreaded
            // hu_HU_u8.dic of the myspell-hu package, see
            // http://packages.debian.org/search?arch=any&searchon=contents&keywords=hu_HU_u8.dic
            // This was ignored because unknown in the old implementation,
            // truncate to the known locale and either insert because hu_HU
            // wasn't encountered yet, or skip because it was. It doesn't
            // really matter because the proper new-style hu_HU dictionary
            // will take precedence anyway if installed with a Hungarian
            // languagepack. Again, this is only to not pull in all
            // liblangtag and stuff during startup, the result would be
            // !isValidBcp47() and the dictionary ignored.
            if (sChunk == "hu-HU-u8")
                sChunk = "hu-HU";

            LanguageTag aLangTag(sChunk, true);
            if (!aLangTag.isValidBcp47())
                continue;

            // Thus we first get the language of the dictionary
            const OUString& aLocaleName(aLangTag.getBcp47());

            if (aDicLangInUse.insert(aLocaleName).second)
            {
                // add the dictionary to the resulting vector
                SvtLinguConfigDictionaryEntry aDicEntry;
                aDicEntry.aLocations.realloc(1);
                aDicEntry.aLocaleNames.realloc(1);
                aDicEntry.aLocations[0] = sPath;
                aDicEntry.aFormatName = aFormatName;
                aDicEntry.aLocaleNames[0] = aLocaleName;
                aRes.push_back( aDicEntry );
            }
        }
    }
}
#endif

// build list of old style dictionaries (not as extensions) to use.
// User installed dictionaries (the ones residing in the user paths)
// will get precedence over system installed ones for the same language.
std::vector< SvtLinguConfigDictionaryEntry > GetOldStyleDics( const char *pDicType )
{
    std::vector< SvtLinguConfigDictionaryEntry > aRes;

    if (!pDicType)
        return aRes;

    OUString aFormatName;
    OUString aDicExtension;
#if defined SYSTEM_DICTS || defined IOS
    OUString aSystemDir;
    OUString aSystemPrefix;
    OUString aSystemSuffix;
#endif
    if (strcmp( pDicType, "DICT" ) == 0)
    {
        aFormatName     = "DICT_SPELL";
        aDicExtension   = ".dic";
#ifdef SYSTEM_DICTS
        aSystemDir      = DICT_SYSTEM_DIR;
        aSystemSuffix   = aDicExtension;
#elif defined IOS
        aSystemDir      = "$BRAND_BASE_DIR/share/spell";
        rtl::Bootstrap::expandMacros(aSystemDir);
        aSystemSuffix   = ".dic";
#endif
    }
    else if (strcmp( pDicType, "HYPH" ) == 0)
    {
        aFormatName     = "DICT_HYPH";
        aDicExtension   = ".dic";
#ifdef SYSTEM_DICTS
        aSystemDir      = HYPH_SYSTEM_DIR;
        aSystemPrefix   = "hyph_";
        aSystemSuffix   = aDicExtension;
#endif
    }
    else if (strcmp( pDicType, "THES" ) == 0)
    {
        aFormatName     = "DICT_THES";
        aDicExtension   = ".dat";
#ifdef SYSTEM_DICTS
        aSystemDir      = THES_SYSTEM_DIR;
        aSystemPrefix   = "th_";
        aSystemSuffix   = "_v2.dat";
#elif defined IOS
        aSystemDir      = "$BRAND_BASE_DIR/share/thes";
        rtl::Bootstrap::expandMacros(aSystemDir);
        aSystemPrefix   = "th_";
        aSystemSuffix   = "_v2.dat";
#endif
    }

    if (aFormatName.isEmpty() || aDicExtension.isEmpty())
        return aRes;

#if defined SYSTEM_DICTS || defined IOS
    // set of languages to remember the language where it is already
    // decided to make use of the dictionary.
    std::set< OUString > aDicLangInUse;

#ifndef IOS
    // follow the hunspell tool's example and check DICPATH for preferred dictionaries
    rtl_uString * pSearchPath = nullptr;
    osl_getEnvironment(OUString("DICPATH").pData, &pSearchPath);

    if (pSearchPath)
    {
        OUString aSearchPath(pSearchPath);
        rtl_uString_release(pSearchPath);

        sal_Int32 nIndex = 0;
        do
        {
            OUString aSystem = aSearchPath.getToken(0, ':', nIndex);
            OUString aCWD;
            OUString aRelative;
            OUString aAbsolute;

            if (!utl::Bootstrap::getProcessWorkingDir(aCWD))
                continue;
            if (osl::FileBase::getFileURLFromSystemPath(aSystem, aRelative)
                    != osl::FileBase::E_None)
                continue;
            if (osl::FileBase::getAbsoluteFileURL(aCWD, aRelative, aAbsolute)
                    != osl::FileBase::E_None)
                continue;

            // GetOldStyleDicsInDir will make sure the dictionary is the right
            // type based on its prefix, that way hyphen, mythes and regular
            // dictionaries can live in one directory
            GetOldStyleDicsInDir(aAbsolute, aFormatName, aSystemSuffix,
                aSystemPrefix, aDicLangInUse, aRes);
        }
        while (nIndex != -1);
    }
#endif

    // load system directories last so that DICPATH prevails
    GetOldStyleDicsInDir(aSystemDir, aFormatName, aSystemSuffix, aSystemPrefix,
        aDicLangInUse, aRes);
#endif

    return aRes;
}

void MergeNewStyleDicsAndOldStyleDics(
    std::vector< SvtLinguConfigDictionaryEntry > &rNewStyleDics,
    const std::vector< SvtLinguConfigDictionaryEntry > &rOldStyleDics )
{
    // get list of languages supported by new style dictionaries
    std::set< OUString > aNewStyleLanguages;
    for (auto const& newStyleDic : rNewStyleDics)
    {
        const uno::Sequence< OUString > aLocaleNames(newStyleDic.aLocaleNames);
        sal_Int32 nLocaleNames = aLocaleNames.getLength();
        for (sal_Int32 k = 0;  k < nLocaleNames; ++k)
        {
            aNewStyleLanguages.insert( aLocaleNames[k] );
        }
    }

    // now check all old style dictionaries if they will add a not yet
    // added language. If so add them to the resulting vector
    for (auto const& oldStyleDic : rOldStyleDics)
    {
        sal_Int32 nOldStyleDics = oldStyleDic.aLocaleNames.getLength();

        // old style dics should only have one language listed...
        DBG_ASSERT( nOldStyleDics, "old style dictionary with more than one language found!");
        if (nOldStyleDics > 0)
        {
            if (linguistic::LinguIsUnspecified( oldStyleDic.aLocaleNames[0]))
            {
                OSL_FAIL( "old style dictionary with invalid language found!" );
                continue;
            }

            // language not yet added?
            if (aNewStyleLanguages.find( oldStyleDic.aLocaleNames[0] ) == aNewStyleLanguages.end())
                rNewStyleDics.push_back(oldStyleDic);
        }
        else
        {
            OSL_FAIL( "old style dictionary with no language found!" );
        }
    }
}

rtl_TextEncoding getTextEncodingFromCharset(const char* pCharset)
{
    // default result: used to indicate that we failed to get the proper encoding
    rtl_TextEncoding eRet = RTL_TEXTENCODING_DONTKNOW;

    if (pCharset)
    {
        eRet = rtl_getTextEncodingFromMimeCharset(pCharset);
        if (eRet == RTL_TEXTENCODING_DONTKNOW)
            eRet = rtl_getTextEncodingFromUnixCharset(pCharset);
        if (eRet == RTL_TEXTENCODING_DONTKNOW)
        {
            if (strcmp("ISCII-DEVANAGARI", pCharset) == 0)
                eRet = RTL_TEXTENCODING_ISCII_DEVANAGARI;
        }
    }
    return eRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
