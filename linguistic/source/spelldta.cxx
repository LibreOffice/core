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

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/linguistic2/SpellFailure.hpp>
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#include <osl/mutex.hxx>
#include <i18nlangtag/languagetag.hxx>

#include <algorithm>
#include <vector>

#include <linguistic/spelldta.hxx>
#include "lngsvcmgr.hxx"


using namespace utl;
using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;


namespace linguistic
{

#define MAX_PROPOSALS   40

static bool SeqHasEntry(
        const std::vector< OUString > &rSeq,
        const OUString &rTxt)
{
    bool bRes = false;
    sal_Int32 nLen = rSeq.size();
    for (sal_Int32 i = 0;  i < nLen && !bRes;  ++i)
    {
        if (rTxt == rSeq[i])
            bRes = true;
    }
    return bRes;
}


void SearchSimilarText( const OUString &rText, LanguageType nLanguage,
        Reference< XSearchableDictionaryList > const &xDicList,
        std::vector< OUString > & rDicListProps )
{
    if (!xDicList.is())
        return;

    const uno::Sequence< Reference< XDictionary > >
            aDics( xDicList->getDictionaries() );
    const Reference< XDictionary >
            *pDic = aDics.getConstArray();
    sal_Int32 nDics = xDicList->getCount();

    for (sal_Int32 i = 0;  i < nDics;  i++)
    {
        Reference< XDictionary > xDic( pDic[i], UNO_QUERY );

        LanguageType nLang = LinguLocaleToLanguage( xDic->getLocale() );

        if ( xDic.is() && xDic->isActive()
            && (nLang == nLanguage  ||  LinguIsUnspecified( nLang)) )
        {
#if OSL_DEBUG_LEVEL > 0
            DictionaryType  eType = xDic->getDictionaryType();
            (void) eType;
            assert( eType != DictionaryType_MIXED && "unexpected dictionary type" );
#endif
            const Sequence< Reference< XDictionaryEntry > > aEntries = xDic->getEntries();
            const Reference< XDictionaryEntry > *pEntries = aEntries.getConstArray();
            sal_Int32 nLen = aEntries.getLength();
            for (sal_Int32 k = 0;  k < nLen;  ++k)
            {
                OUString aEntryTxt;
                if (pEntries[k].is())
                {
                    // remove characters used to determine hyphenation positions
                    aEntryTxt = pEntries[k]->getDictionaryWord().replaceAll("=", "");
                }
                if (!aEntryTxt.isEmpty()  &&  aEntryTxt.getLength() > 1  &&  LevDistance( rText, aEntryTxt ) <= 2)
                    rDicListProps.push_back( aEntryTxt );
            }
        }
    }
}


void SeqRemoveNegEntries( std::vector< OUString > &rSeq,
        Reference< XSearchableDictionaryList > const &rxDicList,
        LanguageType nLanguage )
{
    bool bSthRemoved = false;
    sal_Int32 nLen = rSeq.size();
    for (sal_Int32 i = 0;  i < nLen;  ++i)
    {
        Reference< XDictionaryEntry > xNegEntry( SearchDicList( rxDicList,
                    rSeq[i], nLanguage, false, true ) );
        if (xNegEntry.is())
        {
            rSeq[i].clear();
            bSthRemoved = true;
        }
    }
    if (bSthRemoved)
    {
        std::vector< OUString > aNew;
        // merge sequence without duplicates and empty strings in new empty sequence
        aNew = MergeProposalSeqs( aNew, rSeq );
        rSeq = aNew;
    }
}


std::vector< OUString > MergeProposalSeqs(
            std::vector< OUString > &rAlt1,
            std::vector< OUString > &rAlt2 )
{
    std::vector< OUString > aMerged;

    size_t nAltCount1 = rAlt1.size();
    size_t nAltCount2 = rAlt2.size();

    sal_Int32 nCountNew = std::min<sal_Int32>( nAltCount1 + nAltCount2, sal_Int32(MAX_PROPOSALS) );
    aMerged.resize( nCountNew );

    sal_Int32 nIndex = 0;
    sal_Int32 i = 0;
    for (int j = 0;  j < 2;  j++)
    {
        sal_Int32        nCount  = j == 0 ? nAltCount1 : nAltCount2;
        std::vector< OUString >& rAlt   = j == 0 ? rAlt1 : rAlt2;
        for (i = 0;  i < nCount  &&  nIndex < MAX_PROPOSALS;  i++)
        {
            if (!rAlt[i].isEmpty() &&
                !SeqHasEntry(aMerged, rAlt[i] ))
                aMerged[ nIndex++ ] = rAlt[ i ];
        }
    }
    aMerged.resize( nIndex );

    return aMerged;
}


SpellAlternatives::SpellAlternatives()
{
    nLanguage   = LANGUAGE_NONE;
    nType       = SpellFailure::IS_NEGATIVE_WORD;
}


SpellAlternatives::SpellAlternatives(
        const OUString &rWord, LanguageType nLang,
        const Sequence< OUString > &rAlternatives ) :
    aAlt        (rAlternatives),
    aWord       (rWord),
    nType       (SpellFailure::IS_NEGATIVE_WORD),
    nLanguage   (nLang)
{
}


SpellAlternatives::~SpellAlternatives()
{
}


OUString SAL_CALL SpellAlternatives::getWord()
{
    MutexGuard  aGuard( GetLinguMutex() );
    return aWord;
}


Locale SAL_CALL SpellAlternatives::getLocale()
{
    MutexGuard  aGuard( GetLinguMutex() );
    return LanguageTag::convertToLocale( nLanguage );
}


sal_Int16 SAL_CALL SpellAlternatives::getFailureType()
{
    MutexGuard  aGuard( GetLinguMutex() );
    return nType;
}


sal_Int16 SAL_CALL SpellAlternatives::getAlternativesCount()
{
    MutexGuard  aGuard( GetLinguMutex() );
    return static_cast<sal_Int16>(aAlt.getLength());
}


Sequence< OUString > SAL_CALL SpellAlternatives::getAlternatives()
{
    MutexGuard  aGuard( GetLinguMutex() );
    return aAlt;
}


void SAL_CALL SpellAlternatives::setAlternatives( const uno::Sequence< OUString >& rAlternatives )
{
    MutexGuard  aGuard( GetLinguMutex() );
    aAlt = rAlternatives;
}


void SAL_CALL SpellAlternatives::setFailureType( sal_Int16 nFailureType )
{
    MutexGuard  aGuard( GetLinguMutex() );
    nType = nFailureType;
}


void SpellAlternatives::SetWordLanguage(const OUString &rWord, LanguageType nLang)
{
    MutexGuard  aGuard( GetLinguMutex() );
    aWord = rWord;
    nLanguage = nLang;
}


void SpellAlternatives::SetFailureType(sal_Int16 nTypeP)
{
    MutexGuard  aGuard( GetLinguMutex() );
    nType = nTypeP;
}


void SpellAlternatives::SetAlternatives( const Sequence< OUString > &rAlt )
{
    MutexGuard  aGuard( GetLinguMutex() );
    aAlt = rAlt;
}


css::uno::Reference < css::linguistic2::XSpellAlternatives > SpellAlternatives::CreateSpellAlternatives(
        const OUString &rWord, LanguageType nLang, sal_Int16 nTypeP, const css::uno::Sequence< OUString > &rAlt )
{
    SpellAlternatives* pAlt = new SpellAlternatives;
    pAlt->SetWordLanguage( rWord, nLang );
    pAlt->SetFailureType( nTypeP );
    pAlt->SetAlternatives( rAlt );
    return Reference < XSpellAlternatives >(pAlt);
}


}   // namespace linguistic

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
