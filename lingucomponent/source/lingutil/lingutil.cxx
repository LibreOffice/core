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


#if defined(WNT)
#include <windows.h>
#endif

#include <osl/thread.h>
#include <osl/file.hxx>
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/mslangid.hxx>
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

#include <sal/macros.h>


using ::com::sun::star::lang::Locale;
using namespace ::com::sun::star;

#if defined(WNT)
OString Win_GetShortPathName( const OUString &rLongPathName )
{
    OString aRes;

    sal_Unicode aShortBuffer[1024] = {0};
    sal_Int32   nShortBufSize = SAL_N_ELEMENTS( aShortBuffer );

    
    sal_Int32 nShortLen = GetShortPathNameW(
            reinterpret_cast<LPCWSTR>( rLongPathName.getStr() ),
            reinterpret_cast<LPWSTR>( aShortBuffer ),
            nShortBufSize );

    if (nShortLen < nShortBufSize) 
        aRes = OString( OU2ENC( OUString( aShortBuffer, nShortLen ), osl_getThreadTextEncoding()) );
    else
        OSL_FAIL( "Win_GetShortPathName: buffer to short" );

    return aRes;
}
#endif 






std::vector< SvtLinguConfigDictionaryEntry > GetOldStyleDics( const char *pDicType )
{
    std::vector< SvtLinguConfigDictionaryEntry > aRes;

    if (!pDicType)
        return aRes;

    OUString aFormatName;
    OUString aDicExtension;
#ifdef SYSTEM_DICTS
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
#endif
    }


    if (aFormatName.isEmpty() || aDicExtension.isEmpty())
        return aRes;

#ifdef SYSTEM_DICTS
    osl::Directory aSystemDicts(aSystemDir);
    if (aSystemDicts.open() == osl::FileBase::E_None)
    {
        
        
        std::set< OUString > aDicLangInUse;

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

                
                
                
                
                
                sChunk = sChunk.replace( '_', '-');

                
                
                
                
                
                
                
                
                
                
                
                if (sChunk == "hu-HU-u8")
                    sChunk = "hu-HU";

                LanguageTag aLangTag(sChunk, true);
                if (!aLangTag.isValidBcp47())
                    continue;

                
                OUString aLocaleName(aLangTag.getBcp47());

                if (aDicLangInUse.insert(aLocaleName).second)
                {
                    
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
    
    std::set< OUString > aNewStyleLanguages;
    std::list< SvtLinguConfigDictionaryEntry >::const_iterator aIt;
    for (aIt = rNewStyleDics.begin() ;  aIt != rNewStyleDics.end();  ++aIt)
    {
        const uno::Sequence< OUString > aLocaleNames( aIt->aLocaleNames );
        sal_Int32 nLocaleNames = aLocaleNames.getLength();
        for (sal_Int32 k = 0;  k < nLocaleNames; ++k)
        {
            aNewStyleLanguages.insert( aLocaleNames[k] );
        }
    }

    
    
    std::vector< SvtLinguConfigDictionaryEntry >::const_iterator aIt2;
    for (aIt2 = rOldStyleDics.begin();  aIt2 != rOldStyleDics.end();  ++aIt2)
    {
        sal_Int32 nOldStyleDics = aIt2->aLocaleNames.getLength();

        
        DBG_ASSERT( nOldStyleDics, "old style dictionary with more then one language found!");
        if (nOldStyleDics > 0)
        {
            if (linguistic::LinguIsUnspecified( aIt2->aLocaleNames[0]))
            {
                OSL_FAIL( "old style dictionary with invalid language found!" );
                continue;
            }

            
            if (aNewStyleLanguages.find( aIt2->aLocaleNames[0] ) == aNewStyleLanguages.end())
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
