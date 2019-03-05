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

#include <sal/config.h>
#include <sal/log.hxx>

#include <algorithm>

#include <cppuhelper/factory.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventBroadcaster.hpp>
#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>
#include <rtl/ustrbuf.hxx>
#include <i18nlangtag/lang.h>
#include <unotools/localedatawrapper.hxx>
#include <tools/debug.hxx>
#include <svl/lngmisc.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/mutex.hxx>

#include "hyphdsp.hxx"
#include <linguistic/hyphdta.hxx>
#include <linguistic/lngprops.hxx>
#include "lngsvcmgr.hxx"

using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;


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
            const OUString& rOrigWord,
            const Reference<XDictionaryEntry> &xEntry,
            LanguageType nLang, sal_Int16 nMaxLeading )
{
    MutexGuard  aGuard( GetLinguMutex() );

    Reference< XHyphenatedWord > xRes;

    if (xEntry.is())
    {
        OUString aText( xEntry->getDictionaryWord() );
        sal_Int32 nTextLen = aText.getLength();

        // trailing '=' means "hyphenation should not be possible"
        if (nTextLen > 0  &&  aText[ nTextLen - 1 ] != '=' && aText[ nTextLen - 1 ] != '[')
        {
            sal_Int16 nHyphenationPos = -1;
            sal_Int32 nHyphenPos = -1;
            sal_Int16 nOrigHyphPos = -1;

            OUStringBuffer aTmp( nTextLen );
            bool  bSkip = false;
            bool  bSkip2 = false;
            sal_Int32 nHyphIdx = -1;
            sal_Int32 nLeading = 0;
            for (sal_Int32 i = 0;  i < nTextLen;  i++)
            {
                sal_Unicode cTmp = aText[i];
                if (cTmp == '[' || cTmp == ']')
                    bSkip2 = !bSkip2;
                if (cTmp != '=' && !bSkip2 && cTmp != ']')
                {
                    aTmp.append( cTmp );
                    nLeading++;
                    bSkip = false;
                    nHyphIdx++;
                }
                else
                {
                    if (!bSkip  &&  nHyphIdx >= 0)
                    {
                        if (nLeading <= nMaxLeading) {
                            nHyphenationPos = static_cast<sal_Int16>(nHyphIdx);
                            nOrigHyphPos = i;
                        }
                    }
                    bSkip = true;   //! multiple '=' should count as one only
                }
            }

            if (nHyphenationPos > 0)
            {
#if OSL_DEBUG_LEVEL > 0
                {
                    if (aTmp.toString() != rOrigWord)
                    {
                        // both words should only differ by a having a trailing '.'
                        // character or not...
                        OUString aShorter, aLonger;
                        if (aTmp.getLength() <= rOrigWord.getLength())
                        {
                            aShorter = aTmp.toString();
                            aLonger  = rOrigWord;
                        }
                        else
                        {
                            aShorter = rOrigWord;
                            aLonger  = aTmp.toString();
                        }
                        sal_Int32 nS = aShorter.getLength();
                        sal_Int32 nL = aLonger.getLength();
                        if (nS > 0 && nL > 0)
                        {
                            assert( ((nS + 1 == nL) && aLonger[nL-1] == '.') && "HyphenatorDispatcher::buildHyphWord: unexpected difference between words!" );
                        }
                    }
                }
#endif
                if (aText[ nOrigHyphPos ] == '[')  // alternative hyphenation
                {
                    sal_Int16 split = 0;
                    sal_Unicode c = aText [ nOrigHyphPos + 1 ];
                    sal_Int32 endhyphpat = aText.indexOf( ']', nOrigHyphPos );
                    if ('0' <= c && c <= '9')
                    {
                        split = c - '0';
                        nOrigHyphPos++;
                    }
                    if (endhyphpat > -1)
                    {
                        OUStringBuffer aTmp2 ( aTmp.copy(0, std::max (nHyphenationPos + 1 - split, 0) ) );
                        aTmp2.append( aText.copy( nOrigHyphPos + 1, endhyphpat - nOrigHyphPos - 1) );
                        nHyphenPos = aTmp2.getLength();
                        aTmp2.append( aTmp.copy( nHyphenationPos + 1 ) );
                        //! take care of #i22591#
                        if (rOrigWord[ rOrigWord.getLength() - 1 ] == '.')
                            aTmp2.append( '.' );
                        aText = aTmp2.makeStringAndClear();
                    }
                }
                if (nHyphenPos == -1)
                    aText = rOrigWord;

                xRes = new HyphenatedWord( rOrigWord, nLang, nHyphenationPos,
                                aText, (nHyphenPos > -1) ? nHyphenPos - 1 : nHyphenationPos);
            }
        }
    }

    return xRes;
}


Reference< XPossibleHyphens > HyphenatorDispatcher::buildPossHyphens(
            const Reference< XDictionaryEntry > &xEntry, LanguageType nLanguage )
{
    MutexGuard  aGuard( GetLinguMutex() );

    Reference<XPossibleHyphens> xRes;

    if (xEntry.is())
    {
        // text with hyphenation info
        OUString aText( xEntry->getDictionaryWord() );
        sal_Int32 nTextLen = aText.getLength();

        // trailing '=' means "hyphenation should not be possible"
        if (nTextLen > 0  &&  aText[ nTextLen - 1 ] != '=' && aText[ nTextLen - 1 ] != '[')
        {
            // sequence to hold hyphenation positions
            Sequence< sal_Int16 > aHyphPos( nTextLen );
            sal_Int16 *pPos = aHyphPos.getArray();
            sal_Int32 nHyphCount = 0;

            OUStringBuffer aTmp( nTextLen );
            bool  bSkip = false;
            bool  bSkip2 = false;
            sal_Int32 nHyphIdx = -1;
            for (sal_Int32 i = 0;  i < nTextLen;  i++)
            {
                sal_Unicode cTmp = aText[i];
                if (cTmp == '[' || cTmp == ']')
                    bSkip2 = !bSkip2;
                if (cTmp != '=' && !bSkip2 && cTmp != ']')
                {
                    aTmp.append( cTmp );
                    bSkip = false;
                    nHyphIdx++;
                }
                else
                {
                    if (!bSkip  &&  nHyphIdx >= 0)
                        pPos[ nHyphCount++ ] = static_cast<sal_Int16>(nHyphIdx);
                    bSkip = true;   //! multiple '=' should count as one only
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
{
    MutexGuard  aGuard( GetLinguMutex() );

    Sequence< Locale > aLocales( static_cast< sal_Int32 >(aSvcMap.size()) );
    Locale *pLocales = aLocales.getArray();
    for (auto const& elem : aSvcMap)
    {
        *pLocales++ = LanguageTag::convertToLocale(elem.first);
    }
    return aLocales;
}


sal_Bool SAL_CALL HyphenatorDispatcher::hasLocale(const Locale& rLocale)
{
    MutexGuard  aGuard( GetLinguMutex() );
    HyphSvcByLangMap_t::const_iterator aIt( aSvcMap.find( LinguLocaleToLanguage( rLocale ) ) );
    return aIt != aSvcMap.end();
}


Reference< XHyphenatedWord > SAL_CALL
    HyphenatorDispatcher::hyphenate(
            const OUString& rWord, const Locale& rLocale, sal_Int16 nMaxLeading,
            const css::uno::Sequence< ::css::beans::PropertyValue >& rProperties )
{
    MutexGuard  aGuard( GetLinguMutex() );

    Reference< XHyphenatedWord >    xRes;

    sal_Int32 nWordLen = rWord.getLength();
    LanguageType nLanguage = LinguLocaleToLanguage( rLocale );
    if (LinguIsUnspecified(nLanguage) || !nWordLen ||
        nMaxLeading == 0 || nMaxLeading == nWordLen)
        return xRes;

    // search for entry with that language
    HyphSvcByLangMap_t::iterator    aIt( aSvcMap.find( nLanguage ) );
    LangSvcEntries_Hyph     *pEntry = aIt != aSvcMap.end() ? aIt->second.get() : nullptr;

    bool bWordModified = false;
    if (!pEntry || (nMaxLeading < 0 || nMaxLeading > nWordLen))
    {
        return nullptr;
    }
    else
    {
        OUString aChkWord( rWord );

        // replace typographical apostroph by ascii apostroph
        OUString aSingleQuote( GetLocaleDataWrapper( nLanguage ).getQuotationMarkEnd() );
        DBG_ASSERT( 1 == aSingleQuote.getLength(), "unexpected length of quotation mark" );
        if (!aSingleQuote.isEmpty())
            aChkWord = aChkWord.replace( aSingleQuote[0], '\'' );

        bWordModified |= RemoveHyphens( aChkWord );
        if (IsIgnoreControlChars( rProperties, GetPropSet() ))
            bWordModified |= RemoveControlChars( aChkWord );
        sal_Int16 nChkMaxLeading = static_cast<sal_Int16>(GetPosInWordToCheck( rWord, nMaxLeading ));

        // check for results from (positive) dictionaries which have precedence!
        Reference< XDictionaryEntry > xEntry;

        if (GetDicList().is()  &&  IsUseDicList( rProperties, GetPropSet() ))
        {
            xEntry = GetDicList()->queryDictionaryEntry( aChkWord, rLocale,
                        true, false );
        }

        if (xEntry.is())
        {
            //! because queryDictionaryEntry (in the end DictionaryNeo::getEntry)
            //! does not distinguish between "XYZ" and "XYZ." in order to avoid
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

                Reference< XComponentContext > xContext(
                    comphelper::getProcessComponentContext() );

                // build service initialization argument
                Sequence< Any > aArgs(2);
                aArgs.getArray()[0] <<= GetPropSet();

                // create specific service via it's implementation name
                try
                {
                    xHyph = Reference< XHyphenator >(
                                xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                                    pEntry->aSvcImplNames[0], aArgs, xContext ),
                                UNO_QUERY );
                }
                catch (uno::Exception &)
                {
                    SAL_WARN( "linguistic", "createInstanceWithArguments failed" );
                }
                pRef [i] = xHyph;

                Reference< XLinguServiceEventBroadcaster >
                        xBroadcaster( xHyph, UNO_QUERY );
                if (xBroadcaster.is())
                    rMgr.AddLngSvcEvtBroadcaster( xBroadcaster );

                if (xHyph.is()  &&  xHyph->hasLocale( rLocale ))
                    xRes = xHyph->hyphenate( aChkWord, rLocale, nChkMaxLeading,
                                            rProperties );

                pEntry->nLastTriedSvcIndex = static_cast<sal_Int16>(i);
                ++i;

                // if language is not supported by the services
                // remove it from the list.
                if (xHyph.is()  &&  !xHyph->hasLocale( rLocale ))
                    aSvcMap.erase( nLanguage );
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
            const css::uno::Sequence< ::css::beans::PropertyValue >& rProperties )
{
    MutexGuard  aGuard( GetLinguMutex() );

    Reference< XHyphenatedWord >    xRes;

    sal_Int32 nWordLen = rWord.getLength();
    LanguageType nLanguage = LinguLocaleToLanguage( rLocale );
    if (LinguIsUnspecified(nLanguage) || !nWordLen)
        return xRes;

    // search for entry with that language
    HyphSvcByLangMap_t::iterator    aIt( aSvcMap.find( nLanguage ) );
    LangSvcEntries_Hyph     *pEntry = aIt != aSvcMap.end() ? aIt->second.get() : nullptr;

    bool bWordModified = false;
    if (!pEntry || !(0 <= nIndex && nIndex <= nWordLen - 2))
    {
        return nullptr;
    }
    else
    {
        OUString aChkWord( rWord );

        // replace typographical apostroph by ascii apostroph
        OUString aSingleQuote( GetLocaleDataWrapper( nLanguage ).getQuotationMarkEnd() );
        DBG_ASSERT( 1 == aSingleQuote.getLength(), "unexpected length of quotation mark" );
        if (!aSingleQuote.isEmpty())
            aChkWord = aChkWord.replace( aSingleQuote[0], '\'' );

        bWordModified |= RemoveHyphens( aChkWord );
        if (IsIgnoreControlChars( rProperties, GetPropSet() ))
            bWordModified |= RemoveControlChars( aChkWord );
        sal_Int16 nChkIndex = static_cast<sal_Int16>(GetPosInWordToCheck( rWord, nIndex ));

        // check for results from (positive) dictionaries which have precedence!
        Reference< XDictionaryEntry > xEntry;

        if (GetDicList().is()  &&  IsUseDicList( rProperties, GetPropSet() ))
        {
            xEntry = GetDicList()->queryDictionaryEntry( aChkWord, rLocale,
                        true, false );
        }

        if (xEntry.is())
        {
            xRes = buildHyphWord(aChkWord, xEntry, nLanguage, nIndex + 1);
            if (xRes.is() && xRes->isAlternativeSpelling() && xRes->getHyphenationPos() == nIndex)
                    return xRes;
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

                Reference< XComponentContext > xContext(
                    comphelper::getProcessComponentContext() );

                // build service initialization argument
                Sequence< Any > aArgs(2);
                aArgs.getArray()[0] <<= GetPropSet();

                // create specific service via it's implementation name
                try
                {
                    xHyph = Reference< XHyphenator >(
                                xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                                    pEntry->aSvcImplNames[0], aArgs, xContext ), UNO_QUERY );
                }
                catch (uno::Exception &)
                {
                    SAL_WARN( "linguistic", "createInstanceWithArguments failed" );
                }
                pRef [i] = xHyph;

                Reference< XLinguServiceEventBroadcaster >
                        xBroadcaster( xHyph, UNO_QUERY );
                if (xBroadcaster.is())
                    rMgr.AddLngSvcEvtBroadcaster( xBroadcaster );

                if (xHyph.is()  &&  xHyph->hasLocale( rLocale ))
                    xRes = xHyph->queryAlternativeSpelling( aChkWord, rLocale,
                                nChkIndex, rProperties );

                pEntry->nLastTriedSvcIndex = static_cast<sal_Int16>(i);
                ++i;

                // if language is not supported by the services
                // remove it from the list.
                if (xHyph.is()  &&  !xHyph->hasLocale( rLocale ))
                    aSvcMap.erase( nLanguage );
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
            const css::uno::Sequence< ::css::beans::PropertyValue >& rProperties )
{
    MutexGuard  aGuard( GetLinguMutex() );

    Reference< XPossibleHyphens >   xRes;

    LanguageType nLanguage = LinguLocaleToLanguage( rLocale );
    if (LinguIsUnspecified(nLanguage) || rWord.isEmpty())
        return xRes;

    // search for entry with that language
    HyphSvcByLangMap_t::iterator    aIt( aSvcMap.find( nLanguage ) );
    LangSvcEntries_Hyph     *pEntry = aIt != aSvcMap.end() ? aIt->second.get() : nullptr;

    if (pEntry)
    {
        OUString aChkWord( rWord );

        // replace typographical apostroph by ascii apostroph
        OUString aSingleQuote( GetLocaleDataWrapper( nLanguage ).getQuotationMarkEnd() );
        DBG_ASSERT( 1 == aSingleQuote.getLength(), "unexpected length of quotation mark" );
        if (!aSingleQuote.isEmpty())
            aChkWord = aChkWord.replace( aSingleQuote[0], '\'' );

        RemoveHyphens( aChkWord );
        if (IsIgnoreControlChars( rProperties, GetPropSet() ))
            RemoveControlChars( aChkWord );

        // check for results from (positive) dictionaries which have precedence!
        Reference< XDictionaryEntry > xEntry;

        if (GetDicList().is()  &&  IsUseDicList( rProperties, GetPropSet() ))
        {
            xEntry = GetDicList()->queryDictionaryEntry( aChkWord, rLocale,
                        true, false );
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

                Reference< XComponentContext > xContext(
                    comphelper::getProcessComponentContext() );

                // build service initialization argument
                Sequence< Any > aArgs(2);
                aArgs.getArray()[0] <<= GetPropSet();

                // create specific service via it's implementation name
                try
                {
                    xHyph.set( xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                                   pEntry->aSvcImplNames[0], aArgs, xContext ),
                               UNO_QUERY );
                }
                catch (uno::Exception &)
                {
                    SAL_WARN( "linguistic", "createWithArguments failed" );
                }
                pRef [i] = xHyph;

                Reference< XLinguServiceEventBroadcaster >
                        xBroadcaster( xHyph, UNO_QUERY );
                if (xBroadcaster.is())
                    rMgr.AddLngSvcEvtBroadcaster( xBroadcaster );

                if (xHyph.is()  &&  xHyph->hasLocale( rLocale ))
                    xRes = xHyph->createPossibleHyphens( aChkWord, rLocale, rProperties );

                pEntry->nLastTriedSvcIndex = static_cast<sal_Int16>(i);
                ++i;

                // if language is not supported by the services
                // remove it from the list.
                if (xHyph.is()  &&  !xHyph->hasLocale( rLocale ))
                    aSvcMap.erase( nLanguage );
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

    LanguageType nLanguage = LinguLocaleToLanguage( rLocale );

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
            std::shared_ptr< LangSvcEntries_Hyph > pTmpEntry( new LangSvcEntries_Hyph( rSvcImplNames[0] ) );
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
    LanguageType nLanguage = LinguLocaleToLanguage( rLocale );
    const HyphSvcByLangMap_t::const_iterator  aIt( aSvcMap.find( nLanguage ) );
    const LangSvcEntries_Hyph       *pEntry = aIt != aSvcMap.end() ? aIt->second.get() : nullptr;
    if (pEntry)
    {
        aRes = pEntry->aSvcImplNames;
        if (aRes.getLength() > 0)
            aRes.realloc(1);
    }

    return aRes;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
