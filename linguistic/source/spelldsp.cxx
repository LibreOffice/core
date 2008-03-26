/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: spelldsp.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-26 09:06:23 $
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
#include "precompiled_linguistic.hxx"

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XSEARCHABLEDICTIONARYLIST_HPP_
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#endif

#include <com/sun/star/linguistic2/SpellFailure.hpp>
#include <cppuhelper/factory.hxx>   // helper for factories
#include <com/sun/star/registry/XRegistryKey.hpp>

#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ASSERT
#include <tools/debug.hxx>
#endif
#ifndef _SVTOOLS_LNGMISC_HXX_
#include <svtools/lngmisc.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif

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

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif


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
        const SeqLangSvcEntry_Spell &rEntry,
        INT16 nLanguage )
{
    BOOL bHasLanguage = FALSE;
    Locale aTmpLocale;

    const Reference< XSpellChecker >  *pRef  = rEntry.aSvcRefs .getConstArray();
    const Reference< XSpellChecker1 > *pRef1 = rEntry.aSvc1Refs.getConstArray();
    sal_Int32 nLen = rEntry.aSvcRefs.getLength();
    DBG_ASSERT( nLen == rEntry.aSvc1Refs.getLength(),
            "sequence length mismatch" );
    for (INT32 k = 0;  k < nLen  &&  !bHasLanguage;  ++k)
    {
        if (pRef1[k].is())
            bHasLanguage = pRef1[k]->hasLanguage( nLanguage );
        else if (pRef[k].is())
        {
            if (0 == aTmpLocale.Language.getLength())
                aTmpLocale = CreateLocale( nLanguage );
            bHasLanguage = pRef[k]->hasLocale( aTmpLocale );
        }
    }

    return bHasLanguage;
}

///////////////////////////////////////////////////////////////////////////


SeqLangSvcEntry_Spell::~SeqLangSvcEntry_Spell()
{
}


SeqLangSvcEntry_Spell::SeqLangSvcEntry_Spell(
        const Sequence< OUString > &rSvcImplNames ) :
    aSvcImplNames   ( rSvcImplNames ),
    aSvcRefs        ( rSvcImplNames.getLength() ),
    aSvc1Refs       ( rSvcImplNames.getLength() )
{
}

///////////////////////////////////////////////////////////////////////////

SpellCheckerDispatcher::SpellCheckerDispatcher( LngSvcMgr &rLngSvcMgr ) :
    rMgr    (rLngSvcMgr)
{
    pExtCache = NULL;
}


SpellCheckerDispatcher::~SpellCheckerDispatcher()
{
    ClearSvcList();
    delete pExtCache;
}


void SpellCheckerDispatcher::ClearSvcList()
{
    // release memory for each table entry
    SeqLangSvcEntry_Spell *pItem = aSvcList.First();
    while (pItem)
    {
        SeqLangSvcEntry_Spell *pTmp = pItem;
        pItem = aSvcList.Next();
        delete pTmp;
    }
}


Sequence< Locale > SAL_CALL SpellCheckerDispatcher::getLocales()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return LangSeqToLocaleSeq( getLanguages() );
}


sal_Bool SAL_CALL SpellCheckerDispatcher::hasLocale( const Locale& rLocale )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return hasLanguage( LocaleToLanguage(rLocale) );
}


sal_Bool SAL_CALL
    SpellCheckerDispatcher::isValid( const OUString& rWord, const Locale& rLocale,
            const PropertyValues& rProperties )
        throw(IllegalArgumentException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return isValid( rWord, LocaleToLanguage(rLocale), rProperties );
}


Reference< XSpellAlternatives > SAL_CALL
    SpellCheckerDispatcher::spell( const OUString& rWord, const Locale& rLocale,
            const PropertyValues& rProperties )
        throw(IllegalArgumentException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return spell( rWord, LocaleToLanguage(rLocale), rProperties );
}


Sequence< sal_Int16 > SAL_CALL SpellCheckerDispatcher::getLanguages()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    ULONG nCnt = aSvcList.Count();
    Sequence< INT16 > aLanguages( nCnt );
    INT16 *pLang = aLanguages.getArray();
    SeqLangSvcEntry_Spell *pEntry = aSvcList.First();
    for (ULONG i = 0;  i < nCnt;  i++)
    {
        DBG_ASSERT( pEntry, "lng : pEntry is NULL pointer" );
        pLang[i] = (INT16) aSvcList.GetKey( pEntry );
        pEntry = aSvcList.Next();
    }
    return aLanguages;
}


sal_Bool SAL_CALL SpellCheckerDispatcher::hasLanguage( sal_Int16 nLanguage )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return 0 != aSvcList.Get( nLanguage );
}


sal_Bool SAL_CALL
    SpellCheckerDispatcher::isValid( const OUString& rWord, sal_Int16 nLanguage,
            const PropertyValues& rProperties )
        throw(IllegalArgumentException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    BOOL bRes = TRUE;
    if (LANGUAGE_NONE != nLanguage  &&  rWord.getLength())
    {
        if (aOpt.IsSpellInAllLanguages())
            bRes = isValidInAny( rWord, getLanguages(), rProperties );
        else
            bRes = isValid_Impl( rWord, nLanguage, rProperties, TRUE );
    }
    return bRes;
}


BOOL SpellCheckerDispatcher::isValidInAny(
            const OUString& rWord,
            const Sequence< INT16 >& aLanguages,
            const PropertyValues& rProperties )
        throw( RuntimeException, IllegalArgumentException )
{
    MutexGuard  aGuard( GetLinguMutex() );

    BOOL bRes = TRUE;

    INT32         nNumLang = aLanguages.getLength();
    const INT16  *pLang    = aLanguages.getConstArray();
    for (int i = 0;  i < nNumLang;  i++)
    {
        if( LANGUAGE_NONE != pLang[i] )
        {
            if (sal_True == (bRes = isValid_Impl( rWord, pLang[i], rProperties, TRUE )))
                break;
        }
    }

    return bRes;
}


BOOL SpellCheckerDispatcher::isValid_Impl(
            const OUString& rWord,
            INT16 nLanguage,
            const PropertyValues& rProperties,
            BOOL bCheckDics)
        throw( RuntimeException, IllegalArgumentException )
{
    MutexGuard  aGuard( GetLinguMutex() );

    BOOL bRes = TRUE;

    if (nLanguage == LANGUAGE_NONE  || !rWord.getLength())
        return bRes;

    // search for entry with that language
    SeqLangSvcEntry_Spell *pEntry = aSvcList.Get( nLanguage );

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
        DBG_ASSERT( pEntry->aFlags.nLastTriedSvcIndex < nLen,
                "lng : index out of range");

        INT32 i = 0;
        BOOL bTmpRes = TRUE;
        BOOL bTmpResValid = FALSE;

        // try already instantiated services first
        {
            const Reference< XSpellChecker1 > *pRef1 =
                    pEntry->aSvc1Refs.getConstArray();
            const Reference< XSpellChecker >  *pRef  =
                    pEntry->aSvcRefs.getConstArray();
            while (i <= pEntry->aFlags.nLastTriedSvcIndex
                   &&  (!bTmpResValid  ||  FALSE == bTmpRes))
            {
                bTmpResValid = TRUE;
                if (pRef1[i].is()  &&  pRef1[i]->hasLanguage( nLanguage ))
                    bTmpRes = pRef1[i]->isValid( aChkWord, nLanguage, rProperties );
                else if (pRef[i].is()  &&  pRef[i]->hasLocale( aLocale ))
                {
                    bTmpRes = GetExtCache().CheckWord( aChkWord, nLanguage, FALSE );
                    if (!bTmpRes)
                    {
                        bTmpRes = pRef[i]->isValid( aChkWord, aLocale, rProperties );

                        // Add correct words to the cache.
                        // But not those that are correct only because of
                        // the temporary supplied settings.
                        if (bTmpRes  &&  0 == rProperties.getLength())
                            GetExtCache().AddWord( aChkWord, nLanguage );
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
            &&  pEntry->aFlags.nLastTriedSvcIndex < nLen - 1)
        {
            const OUString *pImplNames = pEntry->aSvcImplNames.getConstArray();
            Reference< XSpellChecker1 > *pRef1 = pEntry->aSvc1Refs.getArray();
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
                        DBG_ERROR( "createInstanceWithArguments failed" );
                    }
                    Reference< XSpellChecker1 > xSpell1( xSpell, UNO_QUERY );
                    pRef [i] = xSpell;
                    pRef1[i] = xSpell1;

                    Reference< XLinguServiceEventBroadcaster >
                            xBroadcaster( xSpell, UNO_QUERY );
                    if (xBroadcaster.is())
                        rMgr.AddLngSvcEvtBroadcaster( xBroadcaster );

                    bTmpResValid = TRUE;
                    if (xSpell1.is()  &&  xSpell1->hasLanguage( nLanguage ))
                        bTmpRes = xSpell1->isValid( aChkWord, nLanguage, rProperties );
                    else if (xSpell.is()  &&  xSpell->hasLocale( aLocale ))
                    {
                        bTmpRes = GetExtCache().CheckWord( aChkWord, nLanguage, FALSE );
                        if (!bTmpRes)
                        {
                            bTmpRes = xSpell->isValid( aChkWord, aLocale, rProperties );

                            // Add correct words to the cache.
                            // But not those that are correct only because of
                            // the temporary supplied settings.
                            if (bTmpRes  &&  0 == rProperties.getLength())
                                GetExtCache().AddWord( aChkWord, nLanguage );
                        }
                    }
                    else
                        bTmpResValid = FALSE;

                    if (bTmpResValid)
                        bRes = bTmpRes;

                    pEntry->aFlags.nLastTriedSvcIndex = (INT16) i;
                    ++i;
                }

                // if language is not supported by any of the services
                // remove it from the list.
                if (i == nLen)
                {
                    if (!SvcListHasLanguage( *pEntry, nLanguage ))
                        aSvcList.Remove( nLanguage );
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


Reference< XSpellAlternatives > SAL_CALL
    SpellCheckerDispatcher::spell( const OUString& rWord, sal_Int16 nLanguage,
               const PropertyValues& rProperties )
        throw(IllegalArgumentException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    Reference< XSpellAlternatives > xAlt;
    if (LANGUAGE_NONE != nLanguage  &&  rWord.getLength())
    {
        if (aOpt.IsSpellInAllLanguages())
            xAlt = spellInAny( rWord, getLanguages(), rProperties, nLanguage );
        else
            xAlt = spell_Impl( rWord, nLanguage, rProperties, TRUE );
    }
    return xAlt;
}


Reference< XSpellAlternatives > SpellCheckerDispatcher::spellInAny(
            const OUString& aWord,
            const Sequence< INT16 >& aLanguages,
            const PropertyValues& rProperties,
            INT16 nPreferredResultLang)
        throw( RuntimeException, IllegalArgumentException )
{
    MutexGuard  aGuard( GetLinguMutex() );

    Reference< XSpellAlternatives > xAlt;

    // check preferred language first
    BOOL bPrefLangChecked = FALSE;
    if (    LANGUAGE_NONE != nPreferredResultLang
         && hasLanguage( nPreferredResultLang ) )
    {
        xAlt = spell_Impl( aWord, nPreferredResultLang, rProperties, TRUE );
        bPrefLangChecked = TRUE;
    }

    // if word is incorrect (or not checked) try the other languages
    if (xAlt.is() || !bPrefLangChecked)
    {
        INT32         nNumLang = aLanguages.getLength();
        const INT16  *pLang    = aLanguages.getConstArray();
        for (int i = 0;  i < nNumLang;  ++i)
        {
            INT16 nLang = pLang[i];
            if (nLang == nPreferredResultLang)  // already checked!
                continue;

            // Bug 71632
            if( LANGUAGE_NONE != nLang && hasLanguage( nLang ) )
            {
                Reference< XSpellAlternatives >
                        xLast( spell_Impl( aWord, nLang, rProperties, TRUE ));

                // remember first spelling alternatives found
                if (xLast.is() && !xAlt.is())
                    xAlt = xLast;

                // did we finally find a language in which the word is correct?
                if (!xLast.is())
                {
                    xAlt = NULL;    // don't return any spelling alternatives
                    break;
                }
            }
        }
    }

    return xAlt;
}


Reference< XSpellAlternatives > SpellCheckerDispatcher::spell_Impl(
            const OUString& rWord,
            sal_Int16 nLanguage,
            const PropertyValues& rProperties,
            BOOL bCheckDics )
        throw(IllegalArgumentException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    Reference< XSpellAlternatives > xRes;

    if (nLanguage == LANGUAGE_NONE  || !rWord.getLength())
        return xRes;

    // search for entry with that language
    SeqLangSvcEntry_Spell *pEntry = aSvcList.Get( nLanguage );

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
        DBG_ASSERT( pEntry->aFlags.nLastTriedSvcIndex < nLen,
                "lng : index out of range");

        INT32 i = 0;
        Reference< XSpellAlternatives > xTmpRes;
        BOOL bTmpResValid = FALSE;

        // try already instantiated services first
        {
            const Reference< XSpellChecker1 > *pRef1 =
                    pEntry->aSvc1Refs.getConstArray();
            const Reference< XSpellChecker >  *pRef  =
                    pEntry->aSvcRefs.getConstArray();
            while (i <= pEntry->aFlags.nLastTriedSvcIndex
                   &&  (!bTmpResValid || xTmpRes.is()) )
            {
                bTmpResValid = TRUE;
                if (pRef1[i].is()  &&  pRef1[i]->hasLanguage( nLanguage ))
                    xTmpRes = pRef1[i]->spell( aChkWord, nLanguage, rProperties );
                else if (pRef[i].is()  &&  pRef[i]->hasLocale( aLocale ))
                {
                    BOOL bOK = GetExtCache().CheckWord( aChkWord, nLanguage, FALSE );
                    if (bOK)
                        xTmpRes = NULL;
                    else
                    {
                        xTmpRes = pRef[i]->spell( aChkWord, aLocale, rProperties );

                        // Add correct words to the cache.
                        // But not those that are correct only because of
                        // the temporary supplied settings.
                        if (!xTmpRes.is()  &&  0 == rProperties.getLength())
                            GetExtCache().AddWord( aChkWord, nLanguage );
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
            &&  pEntry->aFlags.nLastTriedSvcIndex < nLen - 1)
        {
            const OUString *pImplNames = pEntry->aSvcImplNames.getConstArray();
            Reference< XSpellChecker1 > *pRef1 = pEntry->aSvc1Refs.getArray();
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
                        DBG_ERROR( "createInstanceWithArguments failed" );
                    }
                    Reference< XSpellChecker1 > xSpell1( xSpell, UNO_QUERY );
                    pRef [i] = xSpell;
                    pRef1[i] = xSpell1;

                    Reference< XLinguServiceEventBroadcaster >
                            xBroadcaster( xSpell, UNO_QUERY );
                    if (xBroadcaster.is())
                        rMgr.AddLngSvcEvtBroadcaster( xBroadcaster );

                    bTmpResValid = TRUE;
                    if (xSpell1.is()  &&  xSpell1->hasLanguage( nLanguage ))
                        xTmpRes = xSpell1->spell( aChkWord, nLanguage, rProperties );
                    else if (xSpell.is()  &&  xSpell->hasLocale( aLocale ))
                    {
                        BOOL bOK = GetExtCache().CheckWord( aChkWord, nLanguage, FALSE );
                        if (bOK)
                            xTmpRes = NULL;
                        else
                        {
                            xTmpRes = xSpell->spell( aChkWord, aLocale, rProperties );

                            // Add correct words to the cache.
                            // But not those that are correct only because of
                            // the temporary supplied settings.
                            if (!xTmpRes.is()  &&  0 == rProperties.getLength())
                                GetExtCache().AddWord( aChkWord, nLanguage );
                        }
                    }
                    else
                        bTmpResValid = FALSE;

                    // remember first found alternatives only
                    if (!xRes.is() && bTmpResValid)
                        xRes = xTmpRes;

                    pEntry->aFlags.nLastTriedSvcIndex = (INT16) i;
                    ++i;
                }

                // if language is not supported by any of the services
                // remove it from the list.
                if (i == nLen)
                {
                    if (!SvcListHasLanguage( *pEntry, nLanguage ))
                        aSvcList.Remove( nLanguage );
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

            xRes = new SpellAlternatives( aChkWord, nLanguage,
                            eFailureType, aProposals );
        }
    }

    return xRes;
}


void SpellCheckerDispatcher::SetServiceList( const Locale &rLocale,
        const Sequence< OUString > &rSvcImplNames )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (pExtCache)
        pExtCache->Flush(); // new services may spell differently...

    INT16 nLanguage = LocaleToLanguage( rLocale );

    if (0 == rSvcImplNames.getLength())
        // remove entry
        aSvcList.Remove( nLanguage );
    else
    {
        // modify/add entry
        SeqLangSvcEntry_Spell *pEntry = aSvcList.Get( nLanguage );
        if (pEntry)
        {
            INT32 nLen = rSvcImplNames.getLength();
            pEntry->aSvcImplNames = rSvcImplNames;
            pEntry->aSvcRefs  = Sequence< Reference < XSpellChecker > > ( nLen );
            pEntry->aSvc1Refs = Sequence< Reference < XSpellChecker1 > >( nLen );
            pEntry->aFlags = SvcFlags();
        }
        else
        {
            pEntry = new SeqLangSvcEntry_Spell( rSvcImplNames );
            aSvcList.Insert( nLanguage, pEntry );
            DBG_ASSERT( aSvcList.Get( nLanguage ), "lng : Insert failed" );
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
    const SeqLangSvcEntry_Spell *pEntry = pThis->aSvcList.Get( nLanguage );
    if (pEntry)
        aRes = pEntry->aSvcImplNames;

    return aRes;
}


SpellCheckerDispatcher::DspType
    SpellCheckerDispatcher::GetDspType() const
{
    return DSP_SPELL;
}


///////////////////////////////////////////////////////////////////////////

