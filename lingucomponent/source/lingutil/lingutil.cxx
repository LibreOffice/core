/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#if defined(WNT)
#include <windows.h>
#endif

#include <osl/thread.h>
#include <osl/file.hxx>
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <i18npool/languagetag.hxx>
#include <i18npool/mslangid.hxx>
#include <unotools/lingucfg.hxx>
#include <unotools/pathoptions.hxx>
#include <rtl/ustring.hxx>
#include <rtl/string.hxx>
#include <rtl/tencinfo.h>
#include <linguistic/misc.hxx>

#include <set>
#include <vector>
#include <string.h>

#include <lingutil.hxx>
#include <dictmgr.hxx>

#include <sal/macros.h>


using ::com::sun::star::lang::Locale;
using namespace ::com::sun::star;

#if 0
//////////////////////////////////////////////////////////////////////

String GetDirectoryPathFromFileURL( const String &rFileURL )
{
    // get file URL
    INetURLObject aURLObj;
    aURLObj.SetSmartProtocol( INET_PROT_FILE );
    aURLObj.SetSmartURL( rFileURL );
    aURLObj.removeSegment();
    DBG_ASSERT( !aURLObj.HasError(), "invalid URL" );
    String aRes = aURLObj.GetMainURL( INetURLObject::DECODE_TO_IURI );
    return aRes;
}
#endif

#if defined(WNT)
rtl::OString Win_GetShortPathName( const rtl::OUString &rLongPathName )
{
    rtl::OString aRes;

    sal_Unicode aShortBuffer[1024] = {0};
    sal_Int32   nShortBufSize = SAL_N_ELEMENTS( aShortBuffer );

    // use the version of 'GetShortPathName' that can deal with Unicode...
    sal_Int32 nShortLen = GetShortPathNameW(
            reinterpret_cast<LPCWSTR>( rLongPathName.getStr() ),
            reinterpret_cast<LPWSTR>( aShortBuffer ),
            nShortBufSize );

    if (nShortLen < nShortBufSize) // conversion successful?
        aRes = rtl::OString( OU2ENC( rtl::OUString( aShortBuffer, nShortLen ), osl_getThreadTextEncoding()) );
    else
        OSL_FAIL( "Win_GetShortPathName: buffer to short" );

    return aRes;
}
#endif //defined(WNT)

//////////////////////////////////////////////////////////////////////

// build list of old style diuctionaries (not as extensions) to use.
// User installed dictionaries (the ones residing in the user paths)
// will get precedence over system installed ones for the same language.
std::vector< SvtLinguConfigDictionaryEntry > GetOldStyleDics( const char *pDicType )
{
    std::vector< SvtLinguConfigDictionaryEntry > aRes;

    if (!pDicType)
        return aRes;

    rtl::OUString aFormatName;
    String aDicExtension;
#ifdef SYSTEM_DICTS
    rtl::OUString aSystemDir;
    rtl::OUString aSystemPrefix;
    rtl::OUString aSystemSuffix;
#endif
    if (strcmp( pDicType, "DICT" ) == 0)
    {
        aFormatName     = A2OU("DICT_SPELL");
        aDicExtension   = rtl::OUString( ".dic" );
#ifdef SYSTEM_DICTS
        aSystemDir      = A2OU( DICT_SYSTEM_DIR );
        aSystemSuffix       = aDicExtension;
#endif
    }
    else if (strcmp( pDicType, "HYPH" ) == 0)
    {
        aFormatName     = A2OU("DICT_HYPH");
        aDicExtension   = rtl::OUString( ".dic" );
#ifdef SYSTEM_DICTS
        aSystemDir      = A2OU( HYPH_SYSTEM_DIR );
        aSystemPrefix       = A2OU( "hyph_" );
        aSystemSuffix       = aDicExtension;
#endif
    }
    else if (strcmp( pDicType, "THES" ) == 0)
    {
        aFormatName     = A2OU("DICT_THES");
        aDicExtension   = rtl::OUString( ".dat" );
#ifdef SYSTEM_DICTS
        aSystemDir      = A2OU( THES_SYSTEM_DIR );
        aSystemPrefix       = A2OU( "th_" );
        aSystemSuffix       = A2OU( "_v2.dat" );
#endif
    }


    if (aFormatName.isEmpty() || aDicExtension.Len() == 0)
        return aRes;

#ifdef SYSTEM_DICTS
    osl::Directory aSystemDicts(aSystemDir);
    if (aSystemDicts.open() == osl::FileBase::E_None)
    {
        // set of languages to remember the language where it is already
        // decided to make use of the dictionary.
        std::set< OUString > aDicLangInUse;

        osl::DirectoryItem aItem;
        osl::FileStatus aFileStatus(osl_FileStatus_Mask_FileURL);
        while (aSystemDicts.getNextItem(aItem) == osl::FileBase::E_None)
        {
            aItem.getFileStatus(aFileStatus);
            OUString sPath = aFileStatus.getFileURL();
            if (sPath.lastIndexOf(aSystemSuffix) == sPath.getLength()-aSystemSuffix.getLength())
            {
                sal_Int32 nStartIndex = sPath.lastIndexOf(sal_Unicode('/')) + 1;
                if (!sPath.match(aSystemPrefix, nStartIndex))
                    continue;
                OUString sChunk = sPath.copy(nStartIndex + aSystemPrefix.getLength(),
                    sPath.getLength() - aSystemSuffix.getLength() -
                    nStartIndex - aSystemPrefix.getLength());
                if (sChunk.isEmpty())
                    continue;
                //We prefer (now) to use language tags
                LanguageTag aLangTag(sChunk, true);
                //On failure try older basic LANG_REGION scheme
                if (!aLangTag.isValidBcp47())
                {
                    sal_Int32 nIndex = 0;
                    OUString sLang = sChunk.getToken(0, '_', nIndex);
                    if (!sLang.getLength())
                        continue;
                    OUString sRegion;
                    if (nIndex != -1)
                       sRegion = sChunk.copy(nIndex);
                    aLangTag = LanguageTag(sLang, sRegion);
                }
                if (!aLangTag.isValidBcp47())
                    continue;

                // Thus we first get the language of the dictionary
                OUString aLocaleName(aLangTag.getBcp47());

                if (aDicLangInUse.count(aLocaleName) == 0)
                {
                    // remember the new language in use
                    aDicLangInUse.insert(aLocaleName);

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

    return aRes;
}


void MergeNewStyleDicsAndOldStyleDics(
    std::list< SvtLinguConfigDictionaryEntry > &rNewStyleDics,
    const std::vector< SvtLinguConfigDictionaryEntry > &rOldStyleDics )
{
    // get list of languages supported by new style dictionaries
    std::set< LanguageType > aNewStyleLanguages;
    std::list< SvtLinguConfigDictionaryEntry >::const_iterator aIt;
    for (aIt = rNewStyleDics.begin() ;  aIt != rNewStyleDics.end();  ++aIt)
    {
        const uno::Sequence< rtl::OUString > aLocaleNames( aIt->aLocaleNames );
        sal_Int32 nLocaleNames = aLocaleNames.getLength();
        for (sal_Int32 k = 0;  k < nLocaleNames; ++k)
        {
            LanguageType nLang = MsLangId::convertIsoStringToLanguage( aLocaleNames[k] );
            aNewStyleLanguages.insert( nLang );
        }
    }

    // now check all old style dictionaries if they will add a not yet
    // added language. If so add them to the resulting vector
    std::vector< SvtLinguConfigDictionaryEntry >::const_iterator aIt2;
    for (aIt2 = rOldStyleDics.begin();  aIt2 != rOldStyleDics.end();  ++aIt2)
    {
        sal_Int32 nOldStyleDics = aIt2->aLocaleNames.getLength();

        // old style dics should only have one language listed...
        DBG_ASSERT( nOldStyleDics, "old style dictionary with more then one language found!");
        if (nOldStyleDics > 0)
        {
            LanguageType nLang = MsLangId::convertIsoStringToLanguage( aIt2->aLocaleNames[0] );

            if (nLang == LANGUAGE_DONTKNOW || nLang == LANGUAGE_NONE)
            {
                OSL_FAIL( "old style dictionary with invalid language found!" );
                continue;
            }

            // language not yet added?
            if (aNewStyleLanguages.count( nLang ) == 0)
                rNewStyleDics.push_back( *aIt2 );
        }
        else
        {
            OSL_FAIL( "old style dictionary with no language found!" );
        }
    }
}


rtl_TextEncoding getTextEncodingFromCharset(const sal_Char* pCharset)
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

//////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
