/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: lingutil.cxx,v $
 * $Revision: 1.3 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_lingucomponent.hxx"

#if defined(WNT)
#include <tools/prewin.h>
#endif

#if defined(WNT)
#include <Windows.h>
#endif

#if defined(WNT)
#include <tools/postwin.h>
#endif


#include <osl/thread.h>
#ifndef _OSL_FILE_H_
#include <osl/file.hxx>
#endif
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <i18npool/mslangid.hxx>
#include <svtools/lingucfg.hxx>
#include <svtools/pathoptions.hxx>
#include <rtl/ustring.hxx>
#include <rtl/string.hxx>
#include <linguistic/misc.hxx>

#include <set>
#include <vector>

#include <lingutil.hxx>
#include <dictmgr.hxx>


using ::com::sun::star::lang::Locale;
using namespace ::com::sun::star;

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


rtl::OString Win_GetShortPathName( const rtl::OUString &rLongPathName )
{
    (void) rLongPathName;
    rtl::OString aRes;

#if defined(WNT)

    sal_Unicode aShortBuffer[1024] = {0};
    sal_Int32   nShortBufSize = sizeof( aShortBuffer ) / sizeof( aShortBuffer[0] );

    // use the version of 'GetShortPathName' that can deal with Unicode...
    sal_Int32 nShortLen = GetShortPathNameW(
            reinterpret_cast<LPCWSTR>( rLongPathName.getStr() ),
            reinterpret_cast<LPWSTR>( aShortBuffer ),
            nShortBufSize );

    if (nShortLen < nShortBufSize) // conversion successful?
        aRes = rtl::OString( OU2ENC( rtl::OUString( aShortBuffer, nShortLen ), osl_getThreadTextEncoding()) );
    else
        DBG_ERROR( "Win_GetShortPathName: buffer to short" );
#else
    DBG_ERROR( "Win_GetShortPathName: functions should nor be called in non-Windows builds" );
#endif //defined(WNT)

    return aRes;
}

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
    bool bSpell = false;
    bool bHyph  = false;
    bool bThes  = false;
    if (strcmp( pDicType, "DICT" ) == 0)
    {
        aFormatName     = A2OU("DICT_SPELL");
        aDicExtension   = String::CreateFromAscii( ".dic" );
        bSpell = true;
    }
    else if (strcmp( pDicType, "HYPH" ) == 0)
    {
        aFormatName     = A2OU("DICT_HYPH");
        aDicExtension   = String::CreateFromAscii( ".dic" );
        bHyph = true;
    }
    else if (strcmp( pDicType, "THES" ) == 0)
    {
        aFormatName     = A2OU("DICT_THES");
        aDicExtension   = String::CreateFromAscii( ".dat" );
        bThes = true;
    }


    if (aFormatName.getLength() == 0 || aDicExtension.Len() == 0)
        return aRes;

    dictentry * pDict = NULL;  // shared dict entry pointer
    std::set< dictentry *, lt_dictentry > aAvailableDics;

    const sal_Int16 USER_LAYER = 0;
    for (int k = 0;  k < 2;  ++k)
    {
        // Search for 'dictionary.lst' file still in use.
        // First look in the user paths for downloaded dictionaries then
        // look in paths for shared installed dictionaries.
        // In each path sequence there should be at most one 'dictionary.lst' be found...
        const sal_Int16 nFlags = k == USER_LAYER ? PATH_FLAG_USER : PATH_FLAG_INTERNAL;
        const uno::Sequence< ::rtl::OUString > aPaths( linguistic::GetLinguisticPaths( nFlags ) );

        // invoke a dictionary manager to get the dictionary list
        String aLstFile( String::CreateFromAscii("dictionary.lst") );
        aLstFile = linguistic::SearchFileInPaths( aLstFile, aPaths );
        rtl::OUString aLstFileURL;
        osl::FileBase::getSystemPathFromFileURL( aLstFile, aLstFileURL );
        rtl::OString aSysPathToFile( OU2ENC( aLstFileURL, osl_getThreadTextEncoding() ) );
        DictMgr aDictMgr( aSysPathToFile.getStr(), pDicType );
        int nDicts = aDictMgr.get_list( &pDict );

        // set of languages to remember the language where it is already
        // decided to make use of the dictionary.
        std::set< LanguageType > aDicLangInUse;

        // Test for existence of the actual dictionary files
        // and remember the ones we like to use...
        for (int i = 0;  i < nDicts;  ++i)
        {
            // Note: the 'dictionary.lst' file and the actual dictionary files
            // need to reside in the very same directory!!
            String aDicFileName( String::CreateFromAscii( pDict[i].filename ) );
            aDicFileName += aDicExtension;
            aDicFileName = linguistic::SearchFileInPaths( aDicFileName, aPaths );

            // file not found?
            if (aDicFileName.Len() == 0)
                continue;


            //
            // Now, since the dictionary does exist add it to the resulting vector.
            // But don't make use of shared layer dictionaries if for the
            // same language user layer dictionaries do exist.
            // The user dictionaries must get precedence over shared layer
            // (system installed dictionaries) in order to let the user have
            // the choice. E.g. when he wants touse a newer version of a
            // shared layer installed dictionary...
            //

            // Thus we first get the language of the dictionary
            LanguageType nLang = MsLangId::convertIsoNamesToLanguage(
                    A2OU( pDict[i].lang ),
                    A2OU( pDict[i].region ) );

            // Don't add shared layer dictionary if there is already
            // a user layer dictionary...
            if (k == USER_LAYER || aDicLangInUse.count( nLang ) == 0)
            {
                // remember the new language in use
                aDicLangInUse.insert( nLang );

                // add the dictionary to the resulting vector
                SvtLinguConfigDictionaryEntry aDicEntry;
                aDicEntry.aLocations.realloc(1);
                aDicEntry.aLocaleNames.realloc(1);
                rtl::OUString aLocaleName( MsLangId::convertLanguageToIsoString( nLang ) );
                aDicEntry.aLocations[0]   = aDicFileName;
                aDicEntry.aFormatName     = aFormatName;
                aDicEntry.aLocaleNames[0] = aLocaleName;
                aRes.push_back( aDicEntry );
            }
        }
    }

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
        DBG_ASSERT( nOldStyleDics, "old style dictionary with more then one language found!")
        if (nOldStyleDics > 0)
        {
            LanguageType nLang = MsLangId::convertIsoStringToLanguage( aIt2->aLocaleNames[0] );

            // language not yet added?
            if (aNewStyleLanguages.count( nLang ) == 0)
            {
                rNewStyleDics.push_back( *aIt2 );
            }
        }
        else
        {
            DBG_ERROR( "old style dictionary with no language found!" );
        }
    }
}

//////////////////////////////////////////////////////////////////////

