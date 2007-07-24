/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cellkeytranslator.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-24 09:23:06 $
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

#include "cellkeytranslator.hxx"
#include "rtl/ustring.hxx"

using ::com::sun::star::lang::Locale;
using ::std::list;
using ::std::hash_map;
using ::rtl::OUString;

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

static void lclMatchKeyword(String& rName, const ScCellKeywordHashMap& aMap, OpCode eOpCode = ocNone, const Locale* pLocale = NULL)
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

    lclMatchKeyword(rName, spInstance->maStringNameMap, eOpCode, pLocale);
}

ScCellKeywordTranslator::ScCellKeywordTranslator()
{
    init();
}

ScCellKeywordTranslator::~ScCellKeywordTranslator()
{
}

struct TransItem
{
    const sal_Char* from;
    const sal_Char* to;
    OpCode          func;
};

void ScCellKeywordTranslator::init()
{
    // 1. Keywords must be all uppercase.
    // 2. Mapping must be <localized string name> to <English string name>.

    // French language locale.

    static const Locale aFr(OUString::createFromAscii("fr"), OUString(), OUString());
    static const TransItem pFr[] =
    {
        // CELL
        {"ADRESSE",    "ADDRESS",   ocCell},
        {"COLONNE",    "COL",       ocCell},
        {"CONTENU",    "CONTENTS",  ocCell},
        {"COULEUR",    "COLOR",     ocCell},
        {"LARGEUR",    "WIDTH",     ocCell},
        {"LIGNE",      "ROW",       ocCell},
        {"NOMFICHIER", "FILENAME",  ocCell},
        {"PREFIXE",    "PREFIX",    ocCell},
        {"PROTEGE",    "PROTECT",   ocCell},

        // INFO
        {"NBFICH",     "NUMFILE",   ocInfo},
        {"RECALCUL",   "RECALC",    ocInfo},
        {"SYSTEXPL",   "SYSTEM",    ocInfo},
        {"VERSION",    "RELEASE",   ocInfo},
        {"VERSIONSE",  "OSVERSION", ocInfo},

        {NULL, NULL, ocNone}
    };
    addToMap(pFr, aFr);
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
        addToMap(String::CreateFromAscii(pItems[i].from), pItems[i].to, rLocale, pItems[i].func);
}
