/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: spelldsp.cxx,v $
 * $Revision: 1.23 $
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
#include "precompiled_linguistic.hxx"
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>

#include <com/sun/star/linguistic2/SpellFailure.hpp>
#include <cppuhelper/factory.hxx>   // helper for factories
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <unotools/localedatawrapper.hxx>
#include <tools/debug.hxx>
#include <svtools/lngmisc.hxx>
#include <unotools/processfactory.hxx>

#include <vector>

#ifndef _SPELLIMP_HXX
#include <spelldsp.hxx>
#endif
#ifndef _LNGPROPS_HXX
#include <lngprops.hxx>
#endif

#include "spelldsp.hxx"
#include "spelldta.hxx"
#include "lngsvcmgr.hxx"
#include <osl/mutex.hxx>


using namespace utl;
using namespace osl;
using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

///////////////////////////////////////////////////////////////////////////
// ProposalList: list of proposals for misspelled words
//   The order of strings in the array should be left unchanged because the
// spellchecker should have put the more likely suggestions at the top.
// New entries will be added to the end but duplicates are to be avoided.
// Removing entries is done by assigning the empty string.
// The sequence is constructed from all non empty strings in the original
// while maintaining the order.
//
class ProposalList
{
    std::vector< OUString > aVec;

    BOOL    HasEntry( const OUString &rText ) const;

    // make copy c-tor and assignment operator private
    ProposalList( const ProposalList & );
    ProposalList & operator = ( const ProposalList & );

public:
    ProposalList()  {}

    //size_t  Size() const   { return aVec.size(); }
    size_t  Count() const;
    void    Append( const OUString &rNew );
    void    Append( const std::vector< OUString > &rNew );
    void    Append( const Sequence< OUString > &rNew );
    void    Remove( const OUString &rText );
    Sequence< OUString >    GetSequence() const;
};


BOOL ProposalList::HasEntry( const OUString &rText ) const
{
    BOOL bFound = FALSE;
    size_t nCnt = aVec.size();
    for (size_t i = 0;  !bFound && i < nCnt;  ++i)
    {
        if (aVec[i] == rText)
            bFound = TRUE;
    }
    return bFound;
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
    INT32 nLen = rNew.getLength();
    const OUString *pNew = rNew.getConstArray();
    for (INT32 i = 0;  i < nLen;  ++i)
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
        if (aVec[i].getLength() != 0)
            ++nRes;
    }
    return nRes;
}

Sequence< OUString > ProposalList::GetSequence() const
{
    INT32 nCount = Count();
    INT32 nIdx = 0;
    Sequence< OUString > aRes( nCount );
    OUString *pRes = aRes.getArray();
    INT32 nLen = aVec.size();
    for (INT32 i = 0;  i < nLen;  ++i)
    {
        const OUString &rText = aVec[i];
        DBG_ASSERT( nIdx < nCount, "index our of range" );
        if (nIdx < nCount && rText.getLength() > 0)
            pRes[ nIdx++ ] = rText;
    }
    return aRes;
}

void ProposalList::Remove( const OUString &rText )
{
    size_t nLen = aVec.size();
    for (size_t i = 0;  i < nLen;  ++i)
    {
        OUString &rEntry = aVec[i];
        if (rEntry == rText)
        {
            rEntry = OUString();
            break;  // there should be only one matching entry
        }
    }
}


///////////////////////////////////////////////////////////////////////////

BOOL SvcListHasLanguage(
        const LangSvcEntries_Spell &rEntry,
        LanguageType nLanguage )
{
    BOOL bHasLanguage = FALSE;
    Locale aTmpLocale;

    const Reference< XSpellChecker >  *pRef  = rEntry.aSvcRefs .getConstArray();
    sal_Int32 nLen = rEntry.aSvcRefs.getLength();
    for (INT32 k = 0;  k < nLen  &&  !bHasLanguage;  ++k)
    {
        if (pRef[k].is())
        {
            if (0 == aTmpLocale.Language.getLength())
                aTmpLocale = CreateLocale( nLanguage );
            bHasLanguage = pRef[k]->hasLocale( aTmpLocale );
        }
    }

    return bHasLanguage;
}

///////////////////////////////////////////////////////////////////////////


SpellCheckerDispatcher::SpellCheckerDispatcher( LngSvcMgr &rLngSvcMgr ) :
    rMgr    (rLngSvcMgr)
{
    pCache = NULL;
}


SpellCheckerDispatcher::~SpellCheckerDispatcher()
{
    ClearSvcList();
    delete pCache;
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
        *pLocales++ = CreateLocale( aIt->first );
    }
    return aLocales;
}


sal_Bool SAL_CALL SpellCheckerDispatcher::hasLocale( const Locale& rLocale )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    SpellSvcByLangMap_t::const_iterator aIt( aSvcMap.find( LocaleToLanguage( rLocale ) ) );
    return aIt != aSvcMap.end();
}


sal_Bool SAL_CALL
    SpellCheckerDispatcher::isValid( const OUString& rWord, const Locale& rLocale,
            const PropertyValues& rProperties )
        throw(IllegalArgumentException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return isValid_Impl( rWord, LocaleToLanguage( rLocale ), rProperties, TRUE );
}


Reference< XSpellAlternatives > SAL_CALL
    SpellCheckerDispatcher::spell( const OUString& rWord, const Locale& rLocale,
            const PropertyValues& rProperties )
        throw(IllegalArgumentException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return spell_Impl( rWord, LocaleToLanguage( rLocale ), rProperties, TRUE );
}


BOOL SpellCheckerDispatcher::isValid_Impl(
            const OUString& rWord,
            LanguageType nLanguage,
            const PropertyValues& rProperties,
            BOOL bCheckDics)
        throw( RuntimeException, IllegalArgumentException )
{
    MutexGuard  aGuard( GetLinguMutex() );

    BOOL bRes = TRUE;

    if (nLanguage == LANGUAGE_NONE  || !rWord.getLength())
        return bRes;

    // search for entry with that language
    LangSvcEntries_Spell *pEntry = aSvcMap[ nLanguage ].get();

    if (!pEntry)
    {
#ifdef LINGU_EXCEPTIONS
        throw IllegalArgumentException();
#endif
    }
    else
    {
        OUString aChkWord( rWord );
        Locale aLocale( CreateLocale( nLanguage ) );

        // replace typographical apostroph by ascii apostroph
        String aSingleQuote( GetLocaleDataWrapper( nLanguage ).getQuotationMarkEnd() );
        DBG_ASSERT( 1 == aSingleQuote.Len(), "unexpectend length of quotation mark" );
        if (aSingleQuote.Len())
            aChkWord = aChkWord.replace( aSingleQuote.GetChar(0), '\'' );

        RemoveHyphens( aChkWord );
        if (IsIgnoreControlChars( rProperties, GetPropSet() ))
            RemoveControlChars( aChkWord );

        INT32 nLen = pEntry->aSvcRefs.getLength();
        DBG_ASSERT( nLen == pEntry->aSvcImplNames.getLength(),
                "lng : sequence length mismatch");
        DBG_ASSERT( pEntry->nLastTriedSvcIndex < nLen,
                "lng : index out of range");

        INT32 i = 0;
        BOOL bTmpRes = TRUE;
        BOOL bTmpResValid = FALSE;

        // try already instantiated services first
        {
            const Reference< XSpellChecker >  *pRef  =
                    pEntry->aSvcRefs.getConstArray();
            while (i <= pEntry->nLastTriedSvcIndex
                   &&  (!bTmpResValid  ||  FALSE == bTmpRes))
            {
                bTmpResValid = TRUE;
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
                    bTmpResValid = FALSE;

                if (bTmpResValid)
                    bRes = bTmpRes;

                ++i;
            }
        }

        // if still no result instantiate new services and try those
        if ((!bTmpResValid  ||  FALSE == bTmpRes)
            &&  pEntry->nLastTriedSvcIndex < nLen - 1)
        {
            const OUString *pImplNames = pEntry->aSvcImplNames.getConstArray();
            Reference< XSpellChecker >  *pRef  = pEntry->aSvcRefs .getArray();

            Reference< XMultiServiceFactory >  xMgr( getProcessServiceFactory() );
            if (xMgr.is())
            {
                // build service initialization argument
                Sequence< Any > aArgs(2);
                aArgs.getArray()[0] <<= GetPropSet();
                //! The dispatcher searches the dictionary-list
                //! thus the service needs not to now about it
                //aArgs.getArray()[1] <<= GetDicList();

                while (i < nLen  &&  (!bTmpResValid  ||  FALSE == bTmpRes))
                {
                    // create specific service via it's implementation name
                    Reference< XSpellChecker > xSpell;
                    try
                    {
                        xSpell = Reference< XSpellChecker >(
                                xMgr->createInstanceWithArguments(
                                pImplNames[i], aArgs ),  UNO_QUERY );
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

                    bTmpResValid = TRUE;
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
                        bTmpResValid = FALSE;

                    if (bTmpResValid)
                        bRes = bTmpRes;

                    pEntry->nLastTriedSvcIndex = (INT16) i;
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
        }

        // countercheck against results from dictionary which have precedence!
        if (bCheckDics  &&
            GetDicList().is()  &&  IsUseDicList( rProperties, GetPropSet() ))
        {
            Reference< XDictionaryList > xDList( GetDicList(), UNO_QUERY );
            Reference< XDictionaryEntry > xPosEntry( SearchDicList( xDList,
                    aChkWord, nLanguage, TRUE, TRUE ) );
            if (xPosEntry.is())
                bRes = TRUE;
            else
            {
                Reference< XDictionaryEntry > xNegEntry( SearchDicList( xDList,
                        aChkWord, nLanguage, FALSE, TRUE ) );
                if (xNegEntry.is())
                    bRes = FALSE;
            }
        }
    }

    return bRes;
}


Reference< XSpellAlternatives > SpellCheckerDispatcher::spell_Impl(
            const OUString& rWord,
            LanguageType nLanguage,
            const PropertyValues& rProperties,
            BOOL bCheckDics )
        throw(IllegalArgumentException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    Reference< XSpellAlternatives > xRes;

    if (nLanguage == LANGUAGE_NONE  || !rWord.getLength())
        return xRes;

    // search for entry with that language
    LangSvcEntries_Spell *pEntry = aSvcMap[ nLanguage ].get();

    if (!pEntry)
    {
#ifdef LINGU_EXCEPTIONS
        throw IllegalArgumentException();
#endif
    }
    else
    {
        OUString aChkWord( rWord );
        Locale aLocale( CreateLocale( nLanguage ) );

        // replace typographical apostroph by ascii apostroph
        String aSingleQuote( GetLocaleDataWrapper( nLanguage ).getQuotationMarkEnd() );
        DBG_ASSERT( 1 == aSingleQuote.Len(), "unexpectend length of quotation mark" );
        if (aSingleQuote.Len())
            aChkWord = aChkWord.replace( aSingleQuote.GetChar(0), '\'' );

        RemoveHyphens( aChkWord );
        if (IsIgnoreControlChars( rProperties, GetPropSet() ))
            RemoveControlChars( aChkWord );

        INT32 nLen = pEntry->aSvcRefs.getLength();
        DBG_ASSERT( nLen == pEntry->aSvcImplNames.getLength(),
                "lng : sequence length mismatch");
        DBG_ASSERT( pEntry->nLastTriedSvcIndex < nLen,
                "lng : index out of range");

        INT32 i = 0;
        Reference< XSpellAlternatives > xTmpRes;
        BOOL bTmpResValid = FALSE;

        // try already instantiated services first
        {
            const Reference< XSpellChecker >  *pRef  =
                    pEntry->aSvcRefs.getConstArray();
            while (i <= pEntry->nLastTriedSvcIndex
                   &&  (!bTmpResValid || xTmpRes.is()) )
            {
                bTmpResValid = TRUE;
                if (pRef[i].is()  &&  pRef[i]->hasLocale( aLocale ))
                {
                    BOOL bOK = GetCache().CheckWord( aChkWord, nLanguage );
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
                    bTmpResValid = FALSE;

                // remember first found alternatives only
                if (!xRes.is() && bTmpResValid)
                    xRes = xTmpRes;

                ++i;
            }
        }

        // if still no result instantiate new services and try those
        if ((!bTmpResValid || xTmpRes.is())
            &&  pEntry->nLastTriedSvcIndex < nLen - 1)
        {
            const OUString *pImplNames = pEntry->aSvcImplNames.getConstArray();
            Reference< XSpellChecker >  *pRef  = pEntry->aSvcRefs .getArray();

            Reference< XMultiServiceFactory >  xMgr( getProcessServiceFactory() );
            if (xMgr.is())
            {
                // build service initialization argument
                Sequence< Any > aArgs(2);
                aArgs.getArray()[0] <<= GetPropSet();
                //! The dispatcher searches the dictionary-list
                //! thus the service needs not to now about it
                //aArgs.getArray()[1] <<= GetDicList();

                while (i < nLen  &&  (!bTmpResValid || xTmpRes.is()))
                {
                    // create specific service via it's implementation name
                    Reference< XSpellChecker > xSpell;
                    try
                    {
                        xSpell = Reference< XSpellChecker >(
                                xMgr->createInstanceWithArguments(
                                pImplNames[i], aArgs ), UNO_QUERY );
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

                    bTmpResValid = TRUE;
                    if (xSpell.is()  &&  xSpell->hasLocale( aLocale ))
                    {
                        BOOL bOK = GetCache().CheckWord( aChkWord, nLanguage );
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
                        bTmpResValid = FALSE;

                    // remember first found alternatives only
                    if (!xRes.is() && bTmpResValid)
                        xRes = xTmpRes;

                    pEntry->nLastTriedSvcIndex = (INT16) i;
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
        }

        // if word is finally found to be correct
        // clear previously remembered alternatives
        if (bTmpResValid  &&  !xTmpRes.is())
            xRes = NULL;

        // list of proposals found (to be checked against entries of
        // neagtive dictionaries)
        ProposalList aProposalList;
//        Sequence< OUString > aProposals;
        INT16 eFailureType = -1;    // no failure
        if (xRes.is())
        {
            aProposalList.Append( xRes->getAlternatives() );
//            aProposals = xRes->getAlternatives();
            eFailureType = xRes->getFailureType();
        }
        Reference< XDictionaryList > xDList;
        if (GetDicList().is()  &&  IsUseDicList( rProperties, GetPropSet() ))
            xDList = Reference< XDictionaryList >( GetDicList(), UNO_QUERY );

        // countercheck against results from dictionary which have precedence!
        if (bCheckDics  &&  xDList.is())
        {
            Reference< XDictionaryEntry > xPosEntry( SearchDicList( xDList,
                    aChkWord, nLanguage, TRUE, TRUE ) );

            if (xPosEntry.is())
            {
                xRes = NULL;    // positive dictionaries have precedence over negative ones
                eFailureType = -1;  // no failure
            }
            else
            {
                Reference< XDictionaryEntry > xNegEntry( SearchDicList( xDList,
                        aChkWord, nLanguage, FALSE, TRUE ) );
                if (xNegEntry.is())
                {
                    eFailureType = SpellFailure::IS_NEGATIVE_WORD;

                    // replacement text to be added to suggestions, if not empty
                    OUString aAddRplcTxt( xNegEntry->getReplacementText() );

                    // replacement text must not be in negative dictionary itself
                    if (aAddRplcTxt.getLength() &&
                        !SearchDicList( xDList, aAddRplcTxt, nLanguage, FALSE, TRUE ).is())
                    {
                        aProposalList.Append( aAddRplcTxt );
//                        // add suggestion if not already part of proposals
//                        if (!SeqHasEntry( aProposals, aAddRplcTxt))
//                        {
//                            INT32 nLen = aProposals.getLength();
//                            aProposals.realloc( nLen + 1);
//                            aProposals.getArray()[ nLen ] = aAddRplcTxt;
//                        }
                    }
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
            if (bCheckDics  &&  xDList.is())
                SeqRemoveNegEntries( aProposals, xDList, nLanguage );

            uno::Reference< linguistic2::XSetSpellAlternatives > xSetAlt( xRes, uno::UNO_QUERY );
            if (xSetAlt.is())
            {
                xSetAlt->setAlternatives( aProposals );
                xSetAlt->setFailureType( eFailureType );
            }
            else
                DBG_ASSERT( 0, "XSetSpellAlternatives not implemented!" );
        }
    }

    return xRes;
}

uno::Sequence< sal_Int16 > SAL_CALL SpellCheckerDispatcher::getLanguages(  )
throw (uno::RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    uno::Sequence< Locale > aTmp( getLocales() );
    uno::Sequence< INT16 > aRes( LocaleSeqToLangSeq( aTmp ) );
    return aRes;
}


sal_Bool SAL_CALL SpellCheckerDispatcher::hasLanguage(
    sal_Int16 nLanguage )
throw (uno::RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    Locale aLocale( CreateLocale( nLanguage ) );
    return hasLocale( aLocale );
}


sal_Bool SAL_CALL SpellCheckerDispatcher::isValid(
    const OUString& rWord,
    sal_Int16 nLanguage,
    const uno::Sequence< beans::PropertyValue >& rProperties )
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    Locale aLocale( CreateLocale( nLanguage ) );
    return isValid( rWord, aLocale, rProperties);
}


uno::Reference< linguistic2::XSpellAlternatives > SAL_CALL SpellCheckerDispatcher::spell(
    const OUString& rWord,
    sal_Int16 nLanguage,
    const uno::Sequence< beans::PropertyValue >& rProperties )
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    Locale aLocale( CreateLocale( nLanguage ) );
    return spell( rWord, aLocale, rProperties);
}


void SpellCheckerDispatcher::SetServiceList( const Locale &rLocale,
        const Sequence< OUString > &rSvcImplNames )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (pCache)
        pCache->Flush();    // new services may spell differently...

    INT16 nLanguage = LocaleToLanguage( rLocale );

    INT32 nLen = rSvcImplNames.getLength();
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
    INT16 nLanguage = LocaleToLanguage( rLocale );
    SpellCheckerDispatcher      *pThis = (SpellCheckerDispatcher *) this;
    const LangSvcEntries_Spell *pEntry = pThis->aSvcMap[ nLanguage ].get();
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

///////////////////////////////////////////////////////////////////////////

