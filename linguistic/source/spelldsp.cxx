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
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#include <com/sun/star/linguistic2/SpellFailure.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

#include <cppuhelper/factory.hxx>   // helper for factories
#include <unotools/localedatawrapper.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/debug.hxx>
#include <svl/lngmisc.hxx>
#include <osl/mutex.hxx>

#include <vector>

#include "spelldsp.hxx"
#include "linguistic/spelldta.hxx"
#include "lngsvcmgr.hxx"
#include "linguistic/lngprops.hxx"

using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;


// ProposalList: list of proposals for misspelled words
//   The order of strings in the array should be left unchanged because the
// spellchecker should have put the more likely suggestions at the top.
// New entries will be added to the end but duplicates are to be avoided.
// Removing entries is done by assigning the empty string.
// The sequence is constructed from all non empty strings in the original
// while maintaining the order.
class ProposalList
{
    std::vector< OUString > aVec;

    sal_Bool    HasEntry( const OUString &rText ) const;

    // make copy c-tor and assignment operator private
    ProposalList( const ProposalList & );
    ProposalList & operator = ( const ProposalList & );

public:
    ProposalList()  {}

    size_t  Count() const;
    void    Prepend( const OUString &rText );
    void    Append( const OUString &rNew );
    void    Append( const std::vector< OUString > &rNew );
    void    Append( const Sequence< OUString > &rNew );
    Sequence< OUString >    GetSequence() const;
};


sal_Bool ProposalList::HasEntry( const OUString &rText ) const
{
    sal_Bool bFound = sal_False;
    size_t nCnt = aVec.size();
    for (size_t i = 0;  !bFound && i < nCnt;  ++i)
    {
        if (aVec[i] == rText)
            bFound = sal_True;
    }
    return bFound;
}

void ProposalList::Prepend( const OUString &rText )
{
    if (!HasEntry( rText ))
        aVec.insert( aVec.begin(), rText );
}

void ProposalList::Append( const OUString &rText )
{
    if (!HasEntry( rText ))
        aVec.push_back( rText );
}

void ProposalList::Append( const std::vector< OUString > &rNew )
{
    size_t nLen = rNew.size();
    for ( size_t i = 0;  i < nLen;  ++i)
    {
        const OUString &rText = rNew[i];
        if (!HasEntry( rText ))
            Append( rText );
    }
}

void ProposalList::Append( const Sequence< OUString > &rNew )
{
    sal_Int32 nLen = rNew.getLength();
    const OUString *pNew = rNew.getConstArray();
    for (sal_Int32 i = 0;  i < nLen;  ++i)
    {
        const OUString &rText = pNew[i];
        if (!HasEntry( rText ))
            Append( rText );
    }
}

size_t ProposalList::Count() const
{
    // returns the number of non-empty strings in the vector

    size_t nRes = 0;
    size_t nLen = aVec.size();
    for (size_t i = 0;  i < nLen;  ++i)
    {
        if (!aVec[i].isEmpty())
            ++nRes;
    }
    return nRes;
}

Sequence< OUString > ProposalList::GetSequence() const
{
    sal_Int32 nCount = Count();
    sal_Int32 nIdx = 0;
    Sequence< OUString > aRes( nCount );
    OUString *pRes = aRes.getArray();
    sal_Int32 nLen = aVec.size();
    for (sal_Int32 i = 0;  i < nLen;  ++i)
    {
        const OUString &rText = aVec[i];
        DBG_ASSERT( nIdx < nCount, "index our of range" );
        if (nIdx < nCount && !rText.isEmpty())
            pRes[ nIdx++ ] = rText;
    }
    return aRes;
}

sal_Bool SvcListHasLanguage(
        const LangSvcEntries_Spell &rEntry,
        LanguageType nLanguage )
{
    sal_Bool bHasLanguage = sal_False;
    Locale aTmpLocale;

    const Reference< XSpellChecker >  *pRef  = rEntry.aSvcRefs .getConstArray();
    sal_Int32 nLen = rEntry.aSvcRefs.getLength();
    for (sal_Int32 k = 0;  k < nLen  &&  !bHasLanguage;  ++k)
    {
        if (pRef[k].is())
        {
            if (aTmpLocale.Language.isEmpty())
                aTmpLocale = LanguageTag::convertToLocale( nLanguage );
            bHasLanguage = pRef[k]->hasLocale( aTmpLocale );
        }
    }

    return bHasLanguage;
}

SpellCheckerDispatcher::SpellCheckerDispatcher( LngSvcMgr &rLngSvcMgr ) :
    rMgr    (rLngSvcMgr)
{
    pCache = NULL;
    pCharClass = NULL;
}


SpellCheckerDispatcher::~SpellCheckerDispatcher()
{
    ClearSvcList();
    delete pCache;
    delete pCharClass;
}


void SpellCheckerDispatcher::ClearSvcList()
{
    // release memory for each table entry
    SpellSvcByLangMap_t aTmp;
    aSvcMap.swap( aTmp );
}


Sequence< Locale > SAL_CALL SpellCheckerDispatcher::getLocales()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    Sequence< Locale > aLocales( static_cast< sal_Int32 >(aSvcMap.size()) );
    Locale *pLocales = aLocales.getArray();
    SpellSvcByLangMap_t::const_iterator aIt;
    for (aIt = aSvcMap.begin();  aIt != aSvcMap.end();  ++aIt)
    {
        *pLocales++ = LanguageTag::convertToLocale( aIt->first );
    }
    return aLocales;
}


sal_Bool SAL_CALL SpellCheckerDispatcher::hasLocale( const Locale& rLocale )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    SpellSvcByLangMap_t::const_iterator aIt( aSvcMap.find( LinguLocaleToLanguage( rLocale ) ) );
    return aIt != aSvcMap.end();
}


sal_Bool SAL_CALL
    SpellCheckerDispatcher::isValid( const OUString& rWord, const Locale& rLocale,
            const PropertyValues& rProperties )
        throw(IllegalArgumentException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return isValid_Impl( rWord, LinguLocaleToLanguage( rLocale ), rProperties, sal_True );
}


Reference< XSpellAlternatives > SAL_CALL
    SpellCheckerDispatcher::spell( const OUString& rWord, const Locale& rLocale,
            const PropertyValues& rProperties )
        throw(IllegalArgumentException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return spell_Impl( rWord, LinguLocaleToLanguage( rLocale ), rProperties, sal_True );
}


// returns the overall result of cross-checking with all user-dictionaries
// including the IgnoreAll list
static Reference< XDictionaryEntry > lcl_GetRulingDictionaryEntry(
    const OUString &rWord,
    LanguageType nLanguage )
{
    Reference< XDictionaryEntry > xRes;

    // the order of winning from top to bottom is:
    // 1) IgnoreAll list will always win
    // 2) Negative dictionaries will win over positive dictionaries
    Reference< XDictionary > xIgnoreAll( GetIgnoreAllList() );
    if (xIgnoreAll.is())
        xRes = xIgnoreAll->getEntry( rWord );
    if (!xRes.is())
    {
        Reference< XSearchableDictionaryList > xDList( GetDictionaryList() );
        Reference< XDictionaryEntry > xNegEntry( SearchDicList( xDList,
                rWord, nLanguage, sal_False, sal_True ) );
        if (xNegEntry.is())
            xRes = xNegEntry;
        else
        {
            Reference< XDictionaryEntry > xPosEntry( SearchDicList( xDList,
                    rWord, nLanguage, sal_True, sal_True ) );
            if (xPosEntry.is())
                xRes = xPosEntry;
        }
    }

    return xRes;
}


sal_Bool SpellCheckerDispatcher::isValid_Impl(
            const OUString& rWord,
            LanguageType nLanguage,
            const PropertyValues& rProperties,
            sal_Bool bCheckDics)
        throw( RuntimeException, IllegalArgumentException )
{
    MutexGuard  aGuard( GetLinguMutex() );

    sal_Bool bRes = sal_True;

    if (LinguIsUnspecified( nLanguage) || rWord.isEmpty())
        return bRes;

    // search for entry with that language
    SpellSvcByLangMap_t::iterator    aIt( aSvcMap.find( nLanguage ) );
    LangSvcEntries_Spell    *pEntry = aIt != aSvcMap.end() ? aIt->second.get() : NULL;

    if (pEntry)
    {
        OUString aChkWord( rWord );
        Locale aLocale( LanguageTag::convertToLocale( nLanguage ) );

        // replace typographical apostroph by ascii apostroph
        OUString aSingleQuote( GetLocaleDataWrapper( nLanguage ).getQuotationMarkEnd() );
        DBG_ASSERT( 1 == aSingleQuote.getLength(), "unexpectend length of quotation mark" );
        if (!aSingleQuote.isEmpty())
            aChkWord = aChkWord.replace( aSingleQuote[0], '\'' );

        RemoveHyphens( aChkWord );
        if (IsIgnoreControlChars( rProperties, GetPropSet() ))
            RemoveControlChars( aChkWord );

        sal_Int32 nLen = pEntry->aSvcRefs.getLength();
        DBG_ASSERT( nLen == pEntry->aSvcImplNames.getLength(),
                "lng : sequence length mismatch");
        DBG_ASSERT( pEntry->nLastTriedSvcIndex < nLen,
                "lng : index out of range");

        sal_Int32 i = 0;
        sal_Bool bTmpRes = sal_True;
        sal_Bool bTmpResValid = sal_False;

        // try already instantiated services first
        {
            const Reference< XSpellChecker >  *pRef  =
                    pEntry->aSvcRefs.getConstArray();
            while (i <= pEntry->nLastTriedSvcIndex
                   &&  (!bTmpResValid  ||  sal_False == bTmpRes))
            {
                bTmpResValid = sal_True;
                if (pRef[i].is()  &&  pRef[i]->hasLocale( aLocale ))
                {
                    bTmpRes = GetCache().CheckWord( aChkWord, nLanguage );
                    if (!bTmpRes)
                    {
                        bTmpRes = pRef[i]->isValid( aChkWord, aLocale, rProperties );

                        // Add correct words to the cache.
                        // But not those that are correct only because of
                        // the temporary supplied settings.
                        if (bTmpRes  &&  0 == rProperties.getLength())
                            GetCache().AddWord( aChkWord, nLanguage );
                    }
                }
                else
                    bTmpResValid = sal_False;

                if (bTmpResValid)
                    bRes = bTmpRes;

                ++i;
            }
        }

        // if still no result instantiate new services and try those
        if ((!bTmpResValid  ||  sal_False == bTmpRes)
            &&  pEntry->nLastTriedSvcIndex < nLen - 1)
        {
            const OUString *pImplNames = pEntry->aSvcImplNames.getConstArray();
            Reference< XSpellChecker >  *pRef  = pEntry->aSvcRefs .getArray();

            Reference< XComponentContext > xContext(
                comphelper::getProcessComponentContext() );

            // build service initialization argument
            Sequence< Any > aArgs(2);
            aArgs.getArray()[0] <<= GetPropSet();

            while (i < nLen  &&  (!bTmpResValid  ||  sal_False == bTmpRes))
            {
                // create specific service via it's implementation name
                Reference< XSpellChecker > xSpell;
                try
                {
                    xSpell = Reference< XSpellChecker >(
                                xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                                    pImplNames[i], aArgs, xContext ),
                                UNO_QUERY );
                }
                catch (uno::Exception &)
                {
                    DBG_ASSERT( 0, "createInstanceWithArguments failed" );
                }
                pRef [i] = xSpell;

                Reference< XLinguServiceEventBroadcaster >
                        xBroadcaster( xSpell, UNO_QUERY );
                if (xBroadcaster.is())
                    rMgr.AddLngSvcEvtBroadcaster( xBroadcaster );

                bTmpResValid = sal_True;
                if (xSpell.is()  &&  xSpell->hasLocale( aLocale ))
                {
                    bTmpRes = GetCache().CheckWord( aChkWord, nLanguage );
                    if (!bTmpRes)
                    {
                        bTmpRes = xSpell->isValid( aChkWord, aLocale, rProperties );
                         // Add correct words to the cache.
                        // But not those that are correct only because of
                        // the temporary supplied settings.
                        if (bTmpRes  &&  0 == rProperties.getLength())
                            GetCache().AddWord( aChkWord, nLanguage );
                    }
                }
                else
                    bTmpResValid = sal_False;
                if (bTmpResValid)
                    bRes = bTmpRes;

                pEntry->nLastTriedSvcIndex = (sal_Int16) i;
                ++i;
            }

            // if language is not supported by any of the services
            // remove it from the list.
            if (i == nLen)
            {
                if (!SvcListHasLanguage( *pEntry, nLanguage ))
                    aSvcMap.erase( nLanguage );
            }
        }

        // cross-check against results from dictionaries which have precedence!
        if (bCheckDics  &&
            GetDicList().is()  &&  IsUseDicList( rProperties, GetPropSet() ))
        {
            Reference< XDictionaryEntry > xTmp( lcl_GetRulingDictionaryEntry( aChkWord, nLanguage ) );
            if (xTmp.is()) {
                bRes = !xTmp->isNegative();
            } else {
                setCharClass(LanguageTag(nLanguage));
                sal_uInt16 ct = capitalType(aChkWord, pCharClass);
                if (ct == CAPTYPE_INITCAP || ct == CAPTYPE_ALLCAP) {
                    Reference< XDictionaryEntry > xTmp2( lcl_GetRulingDictionaryEntry( makeLowerCase(aChkWord, pCharClass), nLanguage ) );
                    if (xTmp2.is()) {
                        bRes = !xTmp2->isNegative();
                    }
                }
            }
        }
    }

    return bRes;
}


Reference< XSpellAlternatives > SpellCheckerDispatcher::spell_Impl(
            const OUString& rWord,
            LanguageType nLanguage,
            const PropertyValues& rProperties,
            sal_Bool bCheckDics )
        throw(IllegalArgumentException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    Reference< XSpellAlternatives > xRes;

    if (LinguIsUnspecified( nLanguage) || rWord.isEmpty())
        return xRes;

    // search for entry with that language
    SpellSvcByLangMap_t::iterator    aIt( aSvcMap.find( nLanguage ) );
    LangSvcEntries_Spell    *pEntry = aIt != aSvcMap.end() ? aIt->second.get() : NULL;

    if (pEntry)
    {
        OUString aChkWord( rWord );
        Locale aLocale( LanguageTag::convertToLocale( nLanguage ) );

        // replace typographical apostroph by ascii apostroph
        OUString aSingleQuote( GetLocaleDataWrapper( nLanguage ).getQuotationMarkEnd() );
        DBG_ASSERT( 1 == aSingleQuote.getLength(), "unexpectend length of quotation mark" );
        if (!aSingleQuote.isEmpty())
            aChkWord = aChkWord.replace( aSingleQuote[0], '\'' );

        RemoveHyphens( aChkWord );
        if (IsIgnoreControlChars( rProperties, GetPropSet() ))
            RemoveControlChars( aChkWord );

        sal_Int32 nLen = pEntry->aSvcRefs.getLength();
        DBG_ASSERT( nLen == pEntry->aSvcImplNames.getLength(),
                "lng : sequence length mismatch");
        DBG_ASSERT( pEntry->nLastTriedSvcIndex < nLen,
                "lng : index out of range");

        sal_Int32 i = 0;
        Reference< XSpellAlternatives > xTmpRes;
        sal_Bool bTmpResValid = sal_False;

        // try already instantiated services first
        {
            const Reference< XSpellChecker >  *pRef  = pEntry->aSvcRefs.getConstArray();
            sal_Int32 nNumSugestions = -1;
            while (i <= pEntry->nLastTriedSvcIndex
                   &&  (!bTmpResValid || xTmpRes.is()) )
            {
                bTmpResValid = sal_True;
                if (pRef[i].is()  &&  pRef[i]->hasLocale( aLocale ))
                {
                    sal_Bool bOK = GetCache().CheckWord( aChkWord, nLanguage );
                    if (bOK)
                        xTmpRes = NULL;
                    else
                    {
                        xTmpRes = pRef[i]->spell( aChkWord, aLocale, rProperties );

                        // Add correct words to the cache.
                        // But not those that are correct only because of
                        // the temporary supplied settings.
                        if (!xTmpRes.is()  &&  0 == rProperties.getLength())
                            GetCache().AddWord( aChkWord, nLanguage );
                    }
                }
                else
                    bTmpResValid = sal_False;

                // return first found result if the word is not known by any checker.
                // But if that result has no suggestions use the first one that does
                // provide suggestions for the misspelled word.
                if (!xRes.is() && bTmpResValid)
                {
                    xRes = xTmpRes;
                    nNumSugestions = 0;
                    if (xRes.is())
                        nNumSugestions = xRes->getAlternatives().getLength();
                }
                sal_Int32 nTmpNumSugestions = 0;
                if (xTmpRes.is() && bTmpResValid)
                    nTmpNumSugestions = xTmpRes->getAlternatives().getLength();
                if (xRes.is() && nNumSugestions == 0 && nTmpNumSugestions > 0)
                {
                    xRes = xTmpRes;
                    nNumSugestions = nTmpNumSugestions;
                }

                ++i;
            }
        }

        // if still no result instantiate new services and try those
        if ((!bTmpResValid || xTmpRes.is())
            &&  pEntry->nLastTriedSvcIndex < nLen - 1)
        {
            const OUString *pImplNames = pEntry->aSvcImplNames.getConstArray();
            Reference< XSpellChecker >  *pRef  = pEntry->aSvcRefs .getArray();

            Reference< XComponentContext > xContext(
                comphelper::getProcessComponentContext() );

            // build service initialization argument
            Sequence< Any > aArgs(2);
            aArgs.getArray()[0] <<= GetPropSet();

            sal_Int32 nNumSugestions = -1;
            while (i < nLen  &&  (!bTmpResValid || xTmpRes.is()))
            {
                // create specific service via it's implementation name
                Reference< XSpellChecker > xSpell;
                try
                {
                    xSpell = Reference< XSpellChecker >(
                                xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                                    pImplNames[i], aArgs, xContext ),
                                UNO_QUERY );
                }
                catch (uno::Exception &)
                {
                    DBG_ASSERT( 0, "createInstanceWithArguments failed" );
                }
                pRef [i] = xSpell;

                Reference< XLinguServiceEventBroadcaster >
                        xBroadcaster( xSpell, UNO_QUERY );
                if (xBroadcaster.is())
                    rMgr.AddLngSvcEvtBroadcaster( xBroadcaster );

                bTmpResValid = sal_True;
                if (xSpell.is()  &&  xSpell->hasLocale( aLocale ))
                {
                    sal_Bool bOK = GetCache().CheckWord( aChkWord, nLanguage );
                    if (bOK)
                        xTmpRes = NULL;
                    else
                    {
                        xTmpRes = xSpell->spell( aChkWord, aLocale, rProperties );

                        // Add correct words to the cache.
                        // But not those that are correct only because of
                        // the temporary supplied settings.
                        if (!xTmpRes.is()  &&  0 == rProperties.getLength())
                            GetCache().AddWord( aChkWord, nLanguage );
                    }
                }
                else
                    bTmpResValid = sal_False;

                // return first found result if the word is not known by any checker.
                // But if that result has no suggestions use the first one that does
                // provide suggestions for the misspelled word.
                if (!xRes.is() && bTmpResValid)
                {
                    xRes = xTmpRes;
                    nNumSugestions = 0;
                    if (xRes.is())
                        nNumSugestions = xRes->getAlternatives().getLength();
                }
                sal_Int32 nTmpNumSugestions = 0;
                if (xTmpRes.is() && bTmpResValid)
                    nTmpNumSugestions = xTmpRes->getAlternatives().getLength();
                if (xRes.is() && nNumSugestions == 0 && nTmpNumSugestions > 0)
                {
                    xRes = xTmpRes;
                    nNumSugestions = nTmpNumSugestions;
                }

                pEntry->nLastTriedSvcIndex = (sal_Int16) i;
                ++i;
            }

            // if language is not supported by any of the services
            // remove it from the list.
            if (i == nLen)
            {
                if (!SvcListHasLanguage( *pEntry, nLanguage ))
                    aSvcMap.erase( nLanguage );
            }
        }

        // if word is finally found to be correct
        // clear previously remembered alternatives
        if (bTmpResValid  &&  !xTmpRes.is())
            xRes = NULL;

        // list of proposals found (to be checked against entries of
        // neagtive dictionaries)
        ProposalList aProposalList;
        sal_Int16 eFailureType = -1;    // no failure
        if (xRes.is())
        {
            aProposalList.Append( xRes->getAlternatives() );
            eFailureType = xRes->getFailureType();
        }
        Reference< XSearchableDictionaryList > xDList;
        if (GetDicList().is()  &&  IsUseDicList( rProperties, GetPropSet() ))
            xDList = GetDicList();

        // cross-check against results from user-dictionaries which have precedence!
        if (bCheckDics  &&  xDList.is())
        {
            Reference< XDictionaryEntry > xTmp( lcl_GetRulingDictionaryEntry( aChkWord, nLanguage ) );
            if (xTmp.is())
            {
                if (xTmp->isNegative())    // positive entry found
                {
                    eFailureType = SpellFailure::IS_NEGATIVE_WORD;

                    // replacement text to be added to suggestions, if not empty
                    OUString aAddRplcTxt( xTmp->getReplacementText() );

                    // replacement text must not be in negative dictionary itself
                    if (!aAddRplcTxt.isEmpty() &&
                        !SearchDicList( xDList, aAddRplcTxt, nLanguage, sal_False, sal_True ).is())
                    {
                        aProposalList.Prepend( aAddRplcTxt );
                    }
                }
                else    // positive entry found
                {
                    xRes = NULL;
                    eFailureType = -1;  // no failure
                }
            }
        }

        if (eFailureType != -1)     // word misspelled or found in negative user-dictionary
        {
            // search suitable user-dictionaries for suggestions that are
            // similar to the misspelled word
            std::vector< OUString > aDicListProps;   // list of proposals from user-dictionaries
            SearchSimilarText( aChkWord, nLanguage, xDList, aDicListProps );
            aProposalList.Append( aDicListProps );
            Sequence< OUString > aProposals = aProposalList.GetSequence();

            // remove entries listed in negative dictionaries
            // (we don't want to display suggestions that will be regarded as misspelledlater on)
            if (bCheckDics  &&  xDList.is())
                SeqRemoveNegEntries( aProposals, xDList, nLanguage );

            uno::Reference< linguistic2::XSetSpellAlternatives > xSetAlt( xRes, uno::UNO_QUERY );
            if (xSetAlt.is())
            {
                xSetAlt->setAlternatives( aProposals );
                xSetAlt->setFailureType( eFailureType );
            }
            else
            {
                if (xRes.is())
                {
                    DBG_ASSERT( 0, "XSetSpellAlternatives not implemented!" );
                }
                else if (aProposals.getLength() > 0)
                {
                    // no xRes but Proposals found from the user-dictionaries.
                    // Thus we need to create an xRes...
                    xRes = new linguistic::SpellAlternatives( rWord, nLanguage,
                            SpellFailure::IS_NEGATIVE_WORD, aProposals );
                }
            }
        }
    }

    return xRes;
}

uno::Sequence< sal_Int16 > SAL_CALL SpellCheckerDispatcher::getLanguages(  )
throw (uno::RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    uno::Sequence< Locale > aTmp( getLocales() );
    uno::Sequence< sal_Int16 > aRes( LocaleSeqToLangSeq( aTmp ) );
    return aRes;
}


sal_Bool SAL_CALL SpellCheckerDispatcher::hasLanguage(
    sal_Int16 nLanguage )
throw (uno::RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return hasLocale( LanguageTag::convertToLocale( nLanguage) );
}


sal_Bool SAL_CALL SpellCheckerDispatcher::isValid(
    const OUString& rWord,
    sal_Int16 nLanguage,
    const uno::Sequence< beans::PropertyValue >& rProperties )
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return isValid( rWord, LanguageTag::convertToLocale( nLanguage ), rProperties);
}


uno::Reference< linguistic2::XSpellAlternatives > SAL_CALL SpellCheckerDispatcher::spell(
    const OUString& rWord,
    sal_Int16 nLanguage,
    const uno::Sequence< beans::PropertyValue >& rProperties )
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return spell( rWord, LanguageTag::convertToLocale( nLanguage), rProperties);
}


void SpellCheckerDispatcher::SetServiceList( const Locale &rLocale,
        const Sequence< OUString > &rSvcImplNames )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (pCache)
        pCache->Flush();    // new services may spell differently...

    sal_Int16 nLanguage = LinguLocaleToLanguage( rLocale );

    sal_Int32 nLen = rSvcImplNames.getLength();
    if (0 == nLen)
        // remove entry
        aSvcMap.erase( nLanguage );
    else
    {
        // modify/add entry
        LangSvcEntries_Spell *pEntry = aSvcMap[ nLanguage ].get();
        if (pEntry)
        {
            pEntry->Clear();
            pEntry->aSvcImplNames = rSvcImplNames;
            pEntry->aSvcRefs = Sequence< Reference < XSpellChecker > > ( nLen );
        }
        else
        {
            boost::shared_ptr< LangSvcEntries_Spell > pTmpEntry( new LangSvcEntries_Spell( rSvcImplNames ) );
            pTmpEntry->aSvcRefs = Sequence< Reference < XSpellChecker > >( nLen );
            aSvcMap[ nLanguage ] = pTmpEntry;
        }
    }
}


Sequence< OUString >
    SpellCheckerDispatcher::GetServiceList( const Locale &rLocale ) const
{
    MutexGuard  aGuard( GetLinguMutex() );

    Sequence< OUString > aRes;

    // search for entry with that language and use data from that
    sal_Int16 nLanguage = LinguLocaleToLanguage( rLocale );
    SpellCheckerDispatcher          *pThis = (SpellCheckerDispatcher *) this;
    const SpellSvcByLangMap_t::iterator aIt( pThis->aSvcMap.find( nLanguage ) );
    const LangSvcEntries_Spell      *pEntry = aIt != aSvcMap.end() ? aIt->second.get() : NULL;
    if (pEntry)
        aRes = pEntry->aSvcImplNames;

    return aRes;
}


LinguDispatcher::DspType SpellCheckerDispatcher::GetDspType() const
{
    return DSP_SPELL;
}

void SpellCheckerDispatcher::FlushSpellCache()
{
    if (pCache)
        pCache->Flush();
}

void SpellCheckerDispatcher::setCharClass(const LanguageTag& rLanguageTag)
{
    if (!pCharClass)
        pCharClass = new CharClass(rLanguageTag);
    pCharClass->setLanguageTag(rLanguageTag);
}



OUString SAL_CALL SpellCheckerDispatcher::makeLowerCase(const OUString& aTerm, CharClass * pCC)
{
    if (pCC)
        return pCC->lowercase(aTerm);
    return aTerm;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
