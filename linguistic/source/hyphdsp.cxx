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

#include <cppuhelper/factory.hxx>   // helper for factories
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>
#include <rtl/ustrbuf.hxx>
#include <i18npool/lang.h>
#include <unotools/localedatawrapper.hxx>
#include <tools/debug.hxx>
#include <svl/lngmisc.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/mutex.hxx>

#include "hyphdsp.hxx"
#include "linguistic/hyphdta.hxx"
#include "linguistic/lngprops.hxx"
#include "lngsvcmgr.hxx"

using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;


HyphenatorDispatcher::HyphenatorDispatcher( LngSvcMgr &rLngSvcMgr ) :
    rMgr    (rLngSvcMgr)
{
}


HyphenatorDispatcher::~HyphenatorDispatcher()
{
    ClearSvcList();
}


void HyphenatorDispatcher::ClearSvcList()
{
    // release memory for each table entry
    HyphSvcByLangMap_t aTmp;
    aSvcMap.swap( aTmp );
}


Reference<XHyphenatedWord>  HyphenatorDispatcher::buildHyphWord(
            const OUString rOrigWord,
            const Reference<XDictionaryEntry> &xEntry,
            sal_Int16 nLang, sal_Int16 nMaxLeading )
{
    MutexGuard  aGuard( GetLinguMutex() );

    Reference< XHyphenatedWord > xRes;

    if (xEntry.is())
    {
        OUString aText( xEntry->getDictionaryWord() );
        sal_Int32 nTextLen = aText.getLength();

        // trailing '=' means "hyphenation should not be possible"
        if (nTextLen > 0  &&  aText[ nTextLen - 1 ] != '=')
        {
            sal_Int16 nHyphenationPos = -1;

            OUStringBuffer aTmp( nTextLen );
            sal_Bool  bSkip = sal_False;
            sal_Int32 nHyphIdx = -1;
            sal_Int32 nLeading = 0;
            for (sal_Int32 i = 0;  i < nTextLen;  i++)
            {
                sal_Unicode cTmp = aText[i];
                if (cTmp != '=')
                {
                    aTmp.append( cTmp );
                    nLeading++;
                    bSkip = sal_False;
                    nHyphIdx++;
                }
                else
                {
                    if (!bSkip  &&  nHyphIdx >= 0)
                    {
                        if (nLeading <= nMaxLeading)
                            nHyphenationPos = (sal_Int16) nHyphIdx;
                    }
                    bSkip = sal_True;   //! multiple '=' should count as one only
                }
            }

            if (nHyphenationPos > 0)
            {
                aText = aTmp.makeStringAndClear();

#if OSL_DEBUG_LEVEL > 1
                {
                    if (aText != rOrigWord)
                    {
                        // both words should only differ by a having a trailing '.'
                        // character or not...
                        OUString aShorter, aLonger;
                        if (aText.getLength() <= rOrigWord.getLength())
                        {
                            aShorter = aText;
                            aLonger  = rOrigWord;
                        }
                        else
                        {
                            aShorter = rOrigWord;
                            aLonger  = aText;
                        }
                        xub_StrLen nS = sal::static_int_cast< xub_StrLen >( aShorter.getLength() );
                        xub_StrLen nL = sal::static_int_cast< xub_StrLen >( aLonger.getLength() );
                        if (nS > 0 && nL > 0)
                        {
                            DBG_ASSERT( (nS + 1 == nL) && aLonger[nL-1] == (sal_Unicode) '.',
                                "HyphenatorDispatcher::buildHyphWord: unexpected difference between words!" );
                        }
                    }
                }
#endif
                //! take care of #i22591#
                aText = rOrigWord;

                DBG_ASSERT( aText == rOrigWord, "failed to " );
                xRes = new HyphenatedWord( aText, nLang, nHyphenationPos,
                                aText, nHyphenationPos );
            }
        }
    }

    return xRes;
}


Reference< XPossibleHyphens > HyphenatorDispatcher::buildPossHyphens(
            const Reference< XDictionaryEntry > &xEntry, sal_Int16 nLanguage )
{
    MutexGuard  aGuard( GetLinguMutex() );

    Reference<XPossibleHyphens> xRes;

    if (xEntry.is())
    {
        // text with hyphenation info
        OUString aText( xEntry->getDictionaryWord() );
        sal_Int32 nTextLen = aText.getLength();

        // trailing '=' means "hyphenation should not be possible"
        if (nTextLen > 0  &&  aText[ nTextLen - 1 ] != '=')
        {
            // sequence to hold hyphenation positions
            Sequence< sal_Int16 > aHyphPos( nTextLen );
            sal_Int16 *pPos = aHyphPos.getArray();
            sal_Int32 nHyphCount = 0;

            OUStringBuffer aTmp( nTextLen );
            sal_Bool  bSkip = sal_False;
            sal_Int32 nHyphIdx = -1;
            for (sal_Int32 i = 0;  i < nTextLen;  i++)
            {
                sal_Unicode cTmp = aText[i];
                if (cTmp != '=')
                {
                    aTmp.append( cTmp );
                    bSkip = sal_False;
                    nHyphIdx++;
                }
                else
                {
                    if (!bSkip  &&  nHyphIdx >= 0)
                        pPos[ nHyphCount++ ] = (sal_Int16) nHyphIdx;
                    bSkip = sal_True;   //! multiple '=' should count as one only
                }
            }

            // ignore (multiple) trailing '='
            if (bSkip  &&  nHyphIdx >= 0)
            {
                nHyphCount--;
            }
            DBG_ASSERT( nHyphCount >= 0, "lng : invalid hyphenation count");

            if (nHyphCount > 0)
            {
                aHyphPos.realloc( nHyphCount );
                xRes = new PossibleHyphens( aTmp.makeStringAndClear(), nLanguage,
                                aText, aHyphPos );
            }
        }
    }

    return xRes;
}


Sequence< Locale > SAL_CALL HyphenatorDispatcher::getLocales()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    Sequence< Locale > aLocales( static_cast< sal_Int32 >(aSvcMap.size()) );
    Locale *pLocales = aLocales.getArray();
    HyphSvcByLangMap_t::const_iterator aIt;
    for (aIt = aSvcMap.begin();  aIt != aSvcMap.end();  ++aIt)
    {
        *pLocales++ = CreateLocale( aIt->first );
    }
    return aLocales;
}


sal_Bool SAL_CALL HyphenatorDispatcher::hasLocale(const Locale& rLocale)
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    HyphSvcByLangMap_t::const_iterator aIt( aSvcMap.find( LocaleToLanguage( rLocale ) ) );
    return aIt != aSvcMap.end();
}


Reference< XHyphenatedWord > SAL_CALL
    HyphenatorDispatcher::hyphenate(
            const OUString& rWord, const Locale& rLocale, sal_Int16 nMaxLeading,
            const PropertyValues& rProperties )
        throw(IllegalArgumentException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    Reference< XHyphenatedWord >    xRes;

    sal_Int32 nWordLen = rWord.getLength();
    sal_Int16 nLanguage = LocaleToLanguage( rLocale );
    if (nLanguage == LANGUAGE_NONE  || !nWordLen ||
        nMaxLeading == 0 || nMaxLeading == nWordLen)
        return xRes;

    // search for entry with that language
    HyphSvcByLangMap_t::iterator    aIt( aSvcMap.find( nLanguage ) );
    LangSvcEntries_Hyph     *pEntry = aIt != aSvcMap.end() ? aIt->second.get() : NULL;

    bool bWordModified = false;
    if (!pEntry || (nMaxLeading < 0 || nMaxLeading > nWordLen))
    {
#ifdef LINGU_EXCEPTIONS
        throw IllegalArgumentException();
#else
        return NULL;
#endif
    }
    else
    {
        OUString aChkWord( rWord );

        // replace typographical apostroph by ascii apostroph
        String aSingleQuote( GetLocaleDataWrapper( nLanguage ).getQuotationMarkEnd() );
        DBG_ASSERT( 1 == aSingleQuote.Len(), "unexpectend length of quotation mark" );
        if (aSingleQuote.Len())
            aChkWord = aChkWord.replace( aSingleQuote.GetChar(0), '\'' );

        bWordModified |= RemoveHyphens( aChkWord );
        if (IsIgnoreControlChars( rProperties, GetPropSet() ))
            bWordModified |= RemoveControlChars( aChkWord );
        sal_Int16 nChkMaxLeading = (sal_Int16) GetPosInWordToCheck( rWord, nMaxLeading );

        // check for results from (positive) dictionaries which have precedence!
        Reference< XDictionaryEntry > xEntry;

        if (GetDicList().is()  &&  IsUseDicList( rProperties, GetPropSet() ))
        {
            xEntry = GetDicList()->queryDictionaryEntry( aChkWord, rLocale,
                        sal_True, sal_False );
        }

        if (xEntry.is())
        {
            //! because queryDictionaryEntry (in the end DictionaryNeo::getEntry)
            //! does not distinguish betwee "XYZ" and "XYZ." in order to avoid
            //! to require them as different entry we have to supply the
            //! original word here as well so it can be used in th result
            //! otherwise a strange effect may occur (see #i22591#)
            xRes = buildHyphWord( rWord, xEntry, nLanguage, nChkMaxLeading );
        }
        else
        {
            sal_Int32 nLen = pEntry->aSvcImplNames.getLength() > 0 ? 1 : 0;
            DBG_ASSERT( pEntry->nLastTriedSvcIndex < nLen,
                    "lng : index out of range");

            sal_Int32 i = 0;
            Reference< XHyphenator > xHyph;
            if (pEntry->aSvcRefs.getLength() > 0)
                xHyph = pEntry->aSvcRefs[0];

            // try already instantiated service
            if (i <= pEntry->nLastTriedSvcIndex)
            {
                if (xHyph.is()  &&  xHyph->hasLocale( rLocale ))
                    xRes = xHyph->hyphenate( aChkWord, rLocale, nChkMaxLeading,
                                            rProperties );
                ++i;
            }
            else if (pEntry->nLastTriedSvcIndex < nLen - 1)
            // instantiate services and try it
            {
                Reference< XHyphenator > *pRef = pEntry->aSvcRefs.getArray();

                Reference< XMultiServiceFactory > xMgr(
                    comphelper::getProcessServiceFactory() );
                if (xMgr.is())
                {
                    // build service initialization argument
                    Sequence< Any > aArgs(2);
                    aArgs.getArray()[0] <<= GetPropSet();

                    // create specific service via it's implementation name
                    try
                    {
                        xHyph = Reference< XHyphenator >(
                                xMgr->createInstanceWithArguments(
                                pEntry->aSvcImplNames[0], aArgs ), UNO_QUERY );
                    }
                    catch (uno::Exception &)
                    {
                        DBG_ASSERT( 0, "createInstanceWithArguments failed" );
                    }
                    pRef [i] = xHyph;

                    Reference< XLinguServiceEventBroadcaster >
                            xBroadcaster( xHyph, UNO_QUERY );
                    if (xBroadcaster.is())
                        rMgr.AddLngSvcEvtBroadcaster( xBroadcaster );

                    if (xHyph.is()  &&  xHyph->hasLocale( rLocale ))
                        xRes = xHyph->hyphenate( aChkWord, rLocale, nChkMaxLeading,
                                                rProperties );

                    pEntry->nLastTriedSvcIndex = (sal_Int16) i;
                    ++i;

                    // if language is not supported by the services
                    // remove it from the list.
                    if (xHyph.is()  &&  !xHyph->hasLocale( rLocale ))
                        aSvcMap.erase( nLanguage );
                }
            }
        }   // if (xEntry.is())
    }

    if (bWordModified  &&  xRes.is())
        xRes = RebuildHyphensAndControlChars( rWord, xRes );

    if (xRes.is()  &&  xRes->getWord() != rWord)
    {
        xRes = new HyphenatedWord( rWord, nLanguage, xRes->getHyphenationPos(),
                                   xRes->getHyphenatedWord(),
                                   xRes->getHyphenPos() );
    }

    return xRes;
}


Reference< XHyphenatedWord > SAL_CALL
    HyphenatorDispatcher::queryAlternativeSpelling(
            const OUString& rWord, const Locale& rLocale, sal_Int16 nIndex,
            const PropertyValues& rProperties )
        throw(IllegalArgumentException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    Reference< XHyphenatedWord >    xRes;

    sal_Int32 nWordLen = rWord.getLength();
    sal_Int16 nLanguage = LocaleToLanguage( rLocale );
    if (nLanguage == LANGUAGE_NONE  || !nWordLen)
        return xRes;

    // search for entry with that language
    HyphSvcByLangMap_t::iterator    aIt( aSvcMap.find( nLanguage ) );
    LangSvcEntries_Hyph     *pEntry = aIt != aSvcMap.end() ? aIt->second.get() : NULL;

    bool bWordModified = false;
    if (!pEntry || !(0 <= nIndex && nIndex <= nWordLen - 2))
    {
#ifdef LINGU_EXCEPTIONS
        throw IllegalArgumentException();
#else
        return NULL;
#endif
    }
    else
    {
        OUString aChkWord( rWord );

        // replace typographical apostroph by ascii apostroph
        String aSingleQuote( GetLocaleDataWrapper( nLanguage ).getQuotationMarkEnd() );
        DBG_ASSERT( 1 == aSingleQuote.Len(), "unexpectend length of quotation mark" );
        if (aSingleQuote.Len())
            aChkWord = aChkWord.replace( aSingleQuote.GetChar(0), '\'' );

        bWordModified |= RemoveHyphens( aChkWord );
        if (IsIgnoreControlChars( rProperties, GetPropSet() ))
            bWordModified |= RemoveControlChars( aChkWord );
        sal_Int16 nChkIndex = (sal_Int16) GetPosInWordToCheck( rWord, nIndex );

        // check for results from (positive) dictionaries which have precedence!
        Reference< XDictionaryEntry > xEntry;

        if (GetDicList().is()  &&  IsUseDicList( rProperties, GetPropSet() ))
        {
            xEntry = GetDicList()->queryDictionaryEntry( aChkWord, rLocale,
                        sal_True, sal_False );
        }

        if (xEntry.is())
        {
            //! alternative spellings not yet supported by dictionaries
        }
        else
        {
            sal_Int32 nLen = pEntry->aSvcImplNames.getLength() > 0 ? 1 : 0;
            DBG_ASSERT( pEntry->nLastTriedSvcIndex < nLen,
                    "lng : index out of range");

            sal_Int32 i = 0;
            Reference< XHyphenator > xHyph;
            if (pEntry->aSvcRefs.getLength() > 0)
                xHyph = pEntry->aSvcRefs[0];

            // try already instantiated service
            if (i <= pEntry->nLastTriedSvcIndex)
            {
                if (xHyph.is()  &&  xHyph->hasLocale( rLocale ))
                    xRes = xHyph->queryAlternativeSpelling( aChkWord, rLocale,
                                nChkIndex, rProperties );
                ++i;
            }
            else if (pEntry->nLastTriedSvcIndex < nLen - 1)
            // instantiate services and try it
            {
                Reference< XHyphenator > *pRef = pEntry->aSvcRefs.getArray();

                Reference< XMultiServiceFactory > xMgr(
                    comphelper::getProcessServiceFactory() );
                if (xMgr.is())
                {
                    // build service initialization argument
                    Sequence< Any > aArgs(2);
                    aArgs.getArray()[0] <<= GetPropSet();

                    // create specific service via it's implementation name
                    try
                    {
                        xHyph = Reference< XHyphenator >(
                                xMgr->createInstanceWithArguments(
                                pEntry->aSvcImplNames[0], aArgs ), UNO_QUERY );
                    }
                    catch (uno::Exception &)
                    {
                        DBG_ASSERT( 0, "createInstanceWithArguments failed" );
                    }
                    pRef [i] = xHyph;

                    Reference< XLinguServiceEventBroadcaster >
                            xBroadcaster( xHyph, UNO_QUERY );
                    if (xBroadcaster.is())
                        rMgr.AddLngSvcEvtBroadcaster( xBroadcaster );

                    if (xHyph.is()  &&  xHyph->hasLocale( rLocale ))
                        xRes = xHyph->queryAlternativeSpelling( aChkWord, rLocale,
                                    nChkIndex, rProperties );

                    pEntry->nLastTriedSvcIndex = (sal_Int16) i;
                    ++i;

                    // if language is not supported by the services
                    // remove it from the list.
                    if (xHyph.is()  &&  !xHyph->hasLocale( rLocale ))
                        aSvcMap.erase( nLanguage );
                }
            }
        }   // if (xEntry.is())
    }

    if (bWordModified  &&  xRes.is())
        xRes = RebuildHyphensAndControlChars( rWord, xRes );

    if (xRes.is()  &&  xRes->getWord() != rWord)
    {
        xRes = new HyphenatedWord( rWord, nLanguage, xRes->getHyphenationPos(),
                                   xRes->getHyphenatedWord(),
                                   xRes->getHyphenPos() );
    }

    return xRes;
}


Reference< XPossibleHyphens > SAL_CALL
    HyphenatorDispatcher::createPossibleHyphens(
            const OUString& rWord, const Locale& rLocale,
            const PropertyValues& rProperties )
        throw(IllegalArgumentException, RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    Reference< XPossibleHyphens >   xRes;

    sal_Int16 nLanguage = LocaleToLanguage( rLocale );
    if (nLanguage == LANGUAGE_NONE || rWord.isEmpty())
        return xRes;

    // search for entry with that language
    HyphSvcByLangMap_t::iterator    aIt( aSvcMap.find( nLanguage ) );
    LangSvcEntries_Hyph     *pEntry = aIt != aSvcMap.end() ? aIt->second.get() : NULL;

    if (!pEntry)
    {
#ifdef LINGU_EXCEPTIONS
        throw IllegalArgumentException();
#endif
    }
    else
    {
        OUString aChkWord( rWord );

        // replace typographical apostroph by ascii apostroph
        String aSingleQuote( GetLocaleDataWrapper( nLanguage ).getQuotationMarkEnd() );
        DBG_ASSERT( 1 == aSingleQuote.Len(), "unexpectend length of quotation mark" );
        if (aSingleQuote.Len())
            aChkWord = aChkWord.replace( aSingleQuote.GetChar(0), '\'' );

        RemoveHyphens( aChkWord );
        if (IsIgnoreControlChars( rProperties, GetPropSet() ))
            RemoveControlChars( aChkWord );

        // check for results from (positive) dictionaries which have precedence!
        Reference< XDictionaryEntry > xEntry;

        if (GetDicList().is()  &&  IsUseDicList( rProperties, GetPropSet() ))
        {
            xEntry = GetDicList()->queryDictionaryEntry( aChkWord, rLocale,
                        sal_True, sal_False );
        }

        if (xEntry.is())
        {
            xRes = buildPossHyphens( xEntry, nLanguage );
        }
        else
        {
            sal_Int32 nLen = pEntry->aSvcImplNames.getLength() > 0 ? 1 : 0;
            DBG_ASSERT( pEntry->nLastTriedSvcIndex < nLen,
                    "lng : index out of range");

            sal_Int32 i = 0;
            Reference< XHyphenator > xHyph;
            if (pEntry->aSvcRefs.getLength() > 0)
                xHyph = pEntry->aSvcRefs[0];

            // try already instantiated service
            if (i <= pEntry->nLastTriedSvcIndex)
            {
                if (xHyph.is()  &&  xHyph->hasLocale( rLocale ))
                    xRes = xHyph->createPossibleHyphens( aChkWord, rLocale,
                                rProperties );
                ++i;
            }
            else if (pEntry->nLastTriedSvcIndex < nLen - 1)
            // instantiate services and try it
            {
                Reference< XHyphenator > *pRef = pEntry->aSvcRefs.getArray();

                Reference< XMultiServiceFactory > xMgr(
                    comphelper::getProcessServiceFactory() );
                if (xMgr.is())
                {
                    // build service initialization argument
                    Sequence< Any > aArgs(2);
                    aArgs.getArray()[0] <<= GetPropSet();

                    // create specific service via it's implementation name
                    try
                    {
                        xHyph = Reference< XHyphenator >(
                                xMgr->createInstanceWithArguments(
                                pEntry->aSvcImplNames[0], aArgs ), UNO_QUERY );
                    }
                    catch (uno::Exception &)
                    {
                        DBG_ASSERT( 0, "createWithArguments failed" );
                    }
                    pRef [i] = xHyph;

                    Reference< XLinguServiceEventBroadcaster >
                            xBroadcaster( xHyph, UNO_QUERY );
                    if (xBroadcaster.is())
                        rMgr.AddLngSvcEvtBroadcaster( xBroadcaster );

                    if (xHyph.is()  &&  xHyph->hasLocale( rLocale ))
                    xRes = xHyph->createPossibleHyphens( aChkWord, rLocale,
                                rProperties );

                    pEntry->nLastTriedSvcIndex = (sal_Int16) i;
                    ++i;

                    // if language is not supported by the services
                    // remove it from the list.
                    if (xHyph.is()  &&  !xHyph->hasLocale( rLocale ))
                        aSvcMap.erase( nLanguage );
                }
            }
        }   // if (xEntry.is())
    }

    if (xRes.is()  &&  xRes->getWord() != rWord)
    {
        xRes = new PossibleHyphens( rWord, nLanguage,
                                    xRes->getPossibleHyphens(),
                                    xRes->getHyphenationPositions() );
    }

    return xRes;
}


void HyphenatorDispatcher::SetServiceList( const Locale &rLocale,
        const Sequence< OUString > &rSvcImplNames )
{
    MutexGuard  aGuard( GetLinguMutex() );

    sal_Int16 nLanguage = LocaleToLanguage( rLocale );

    sal_Int32 nLen = rSvcImplNames.getLength();
    if (0 == nLen)
        // remove entry
        aSvcMap.erase( nLanguage );
    else
    {
        // modify/add entry
        LangSvcEntries_Hyph *pEntry = aSvcMap[ nLanguage ].get();
        if (pEntry)
        {
            pEntry->Clear();
            pEntry->aSvcImplNames = rSvcImplNames;
            pEntry->aSvcImplNames.realloc(1);
            pEntry->aSvcRefs  = Sequence< Reference < XHyphenator > > ( 1 );
        }
        else
        {
            boost::shared_ptr< LangSvcEntries_Hyph > pTmpEntry( new LangSvcEntries_Hyph( rSvcImplNames[0] ) );
            pTmpEntry->aSvcRefs = Sequence< Reference < XHyphenator > >( 1 );
            aSvcMap[ nLanguage ] = pTmpEntry;
        }
    }
}


Sequence< OUString >
    HyphenatorDispatcher::GetServiceList( const Locale &rLocale ) const
{
    MutexGuard  aGuard( GetLinguMutex() );

    Sequence< OUString > aRes;

    // search for entry with that language and use data from that
    sal_Int16 nLanguage = LocaleToLanguage( rLocale );
    HyphenatorDispatcher            *pThis = (HyphenatorDispatcher *) this;
    const HyphSvcByLangMap_t::iterator  aIt( pThis->aSvcMap.find( nLanguage ) );
    const LangSvcEntries_Hyph       *pEntry = aIt != aSvcMap.end() ? aIt->second.get() : NULL;
    if (pEntry)
    {
        aRes = pEntry->aSvcImplNames;
        if (aRes.getLength() > 0)
            aRes.realloc(1);
    }

    return aRes;
}


LinguDispatcher::DspType HyphenatorDispatcher::GetDspType() const
{
    return DSP_HYPH;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
