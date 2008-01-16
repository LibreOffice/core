/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cellkeytranslator.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-16 14:29:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include "cellkeytranslator.hxx"
#include "comphelper/processfactory.hxx"
#include "i18npool/mslangid.hxx"
#include "i18npool/lang.h"
#include "rtl/ustring.hxx"

#include <com/sun/star/i18n/TransliterationModules.hpp>

using ::com::sun::star::lang::Locale;
using ::com::sun::star::uno::Sequence;
using ::std::list;
using ::std::hash_map;
using ::rtl::OUString;

using namespace ::com::sun::star;

enum LocaleMatch
{
    LOCALE_MATCH_NONE = 0,
    LOCALE_MATCH_LANG,
    LOCALE_MATCH_LANG_COUNTRY,
    LOCALE_MATCH_ALL
};

static LocaleMatch lclLocaleCompare(const Locale& rLocale1, const Locale& rLocale2)
{
    LocaleMatch eMatchLevel = LOCALE_MATCH_NONE;
    if ( !rLocale1.Language.compareTo(rLocale1.Language) )
        eMatchLevel = LOCALE_MATCH_LANG;
    else
        return eMatchLevel;

    if ( !rLocale1.Country.compareTo(rLocale2.Country) )
        eMatchLevel = LOCALE_MATCH_LANG_COUNTRY;
    else
        return eMatchLevel;

    if ( !rLocale1.Variant.compareTo(rLocale2.Variant) )
        eMatchLevel = LOCALE_MATCH_ALL;

    return eMatchLevel;
}

ScCellKeyword::ScCellKeyword(const sal_Char* pName, OpCode eOpCode, const Locale& rLocale) :
    mpName(pName),
    meOpCode(eOpCode),
    mrLocale(rLocale)
{
}

::std::auto_ptr<ScCellKeywordTranslator> ScCellKeywordTranslator::spInstance(NULL);

static void lclMatchKeyword(String& rName, const ScCellKeywordHashMap& aMap,
                            OpCode eOpCode = ocNone, const Locale* pLocale = NULL)
{
    ScCellKeywordHashMap::const_iterator itrEnd = aMap.end();
    ScCellKeywordHashMap::const_iterator itr = aMap.find(rName);

    if ( itr == itrEnd || itr->second.empty() )
        // No candidate strings exist.  Bail out.
        return;

    if ( eOpCode == ocNone && !pLocale )
    {
        // Since no locale nor opcode matching is needed, simply return
        // the first item on the list.
        rName = String::CreateFromAscii( itr->second.front().mpName );
        return;
    }

    const sal_Char* aBestMatchName = itr->second.front().mpName;
    LocaleMatch eLocaleMatchLevel = LOCALE_MATCH_NONE;
    bool bOpCodeMatched = false;

    list<ScCellKeyword>::const_iterator itrListEnd = itr->second.end();
    list<ScCellKeyword>::const_iterator itrList = itr->second.begin();
    for ( ; itrList != itrListEnd; ++itrList )
    {
        if ( eOpCode != ocNone && pLocale )
        {
            if ( itrList->meOpCode == eOpCode )
            {
                LocaleMatch eLevel = lclLocaleCompare(itrList->mrLocale, *pLocale);
                if ( eLevel == LOCALE_MATCH_ALL )
                {
                    // Name with matching opcode and locale found.
                    rName = String::CreateFromAscii( itrList->mpName );
                    return;
                }
                else if ( eLevel > eLocaleMatchLevel )
                {
                    // Name with a better matching locale.
                    eLocaleMatchLevel = eLevel;
                    aBestMatchName = itrList->mpName;
                }
                else if ( !bOpCodeMatched )
                    // At least the opcode matches.
                    aBestMatchName = itrList->mpName;

                bOpCodeMatched = true;
            }
        }
        else if ( eOpCode != ocNone && !pLocale )
        {
            if ( itrList->meOpCode == eOpCode )
            {
                // Name with a matching opcode preferred.
                rName = String::CreateFromAscii( itrList->mpName );
                return;
            }
        }
        else if ( !eOpCode && pLocale )
        {
            LocaleMatch eLevel = lclLocaleCompare(itrList->mrLocale, *pLocale);
            if ( eLevel == LOCALE_MATCH_ALL )
            {
                // Name with matching locale preferred.
                rName = String::CreateFromAscii( itrList->mpName );
                return;
            }
            else if ( eLevel > eLocaleMatchLevel )
            {
                // Name with a better matching locale.
                eLocaleMatchLevel = eLevel;
                aBestMatchName = itrList->mpName;
            }
        }
    }

    // No preferred strings found.  Return the best matching name.
    rName = String::CreateFromAscii(aBestMatchName);
}

void ScCellKeywordTranslator::transKeyword(String& rName, const Locale* pLocale, OpCode eOpCode)
{
    if ( !spInstance.get() )
        spInstance.reset( new ScCellKeywordTranslator );

    LanguageType eLang = pLocale ? MsLangId::convertLocaleToLanguageWithFallback(*pLocale) : LANGUAGE_SYSTEM;
    Sequence<sal_Int32> aOffsets;
    rName = spInstance->maTransWrapper.transliterate(rName, eLang, 0, rName.Len(), &aOffsets);
    lclMatchKeyword(rName, spInstance->maStringNameMap, eOpCode, pLocale);
}

ScCellKeywordTranslator::ScCellKeywordTranslator() :
    maTransWrapper( ::comphelper::getProcessServiceFactory(),
                    i18n::TransliterationModules_LOWERCASE_UPPERCASE )
{
    init();
}

ScCellKeywordTranslator::~ScCellKeywordTranslator()
{
}

struct TransItem
{
    const sal_Unicode*  from;
    const sal_Char*     to;
    OpCode              func;
};

void ScCellKeywordTranslator::init()
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );

    // The file below has been autogenerated by sc/workben/celltrans/parse.py.
    // To add new locale keywords, edit sc/workben/celltrans/keywords_utf16.txt
    // and re-run the parse.py script.
    //
    // All keywords must be uppercase, and the mapping must be from the
    // localized keyword to the English keyword.
    //
    // Make sure that the original keyword file (keywords_utf16.txt) is
    // encoded in UCS-2/UTF-16!

    #include "cellkeywords.inl"
}

void ScCellKeywordTranslator::addToMap(const String& rKey, const sal_Char* pName, const Locale& rLocale, OpCode eOpCode)
{
    ScCellKeyword aKeyItem( pName, eOpCode, rLocale );

    ScCellKeywordHashMap::iterator itrEnd = maStringNameMap.end();
    ScCellKeywordHashMap::iterator itr = maStringNameMap.find(rKey);

    if ( itr == itrEnd )
    {
        // New keyword.
        list<ScCellKeyword> aList;
        aList.push_back(aKeyItem);
        maStringNameMap.insert( ScCellKeywordHashMap::value_type(rKey, aList) );
    }
    else
        itr->second.push_back(aKeyItem);
}

void ScCellKeywordTranslator::addToMap(const TransItem* pItems, const Locale& rLocale)
{
    for (sal_uInt16 i = 0; pItems[i].from != NULL; ++i)
        addToMap(String(pItems[i].from), pItems[i].to, rLocale, pItems[i].func);
}
