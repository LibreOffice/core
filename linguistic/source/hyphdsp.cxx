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

#include <cppuhelper/factory.hxx>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>
#include <rtl/ustrbuf.hxx>
#include <i18nlangtag/lang.h>
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

        
        if (nTextLen > 0  &&  aText[ nTextLen - 1 ] != '=' && aText[ nTextLen - 1 ] != '[')
        {
            sal_Int16 nHyphenationPos = -1;
            sal_Int32 nHyphenPos = -1;
            sal_Int16 nOrigHyphPos = -1;

            OUStringBuffer aTmp( nTextLen );
            sal_Bool  bSkip = sal_False;
            sal_Bool  bSkip2 = sal_False;
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
                    bSkip = sal_False;
                    nHyphIdx++;
                }
                else
                {
                    if (!bSkip  &&  nHyphIdx >= 0)
                    {
                        if (nLeading <= nMaxLeading) {
                            nHyphenationPos = (sal_Int16) nHyphIdx;
                            nOrigHyphPos = i;
                        }
                    }
                    bSkip = sal_True;   
                }
            }

            if (nHyphenationPos > 0)
            {

#if OSL_DEBUG_LEVEL > 1
                {
                    if (aTmp.toString() != rOrigWord)
                    {
                        
                        
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
                            DBG_ASSERT( (nS + 1 == nL) && aLonger[nL-1] == '.',
                                "HyphenatorDispatcher::buildHyphWord: unexpected difference between words!" );
                        }
                    }
                }
#endif
                if (aText[ nOrigHyphPos ] == '[')  
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
            const Reference< XDictionaryEntry > &xEntry, sal_Int16 nLanguage )
{
    MutexGuard  aGuard( GetLinguMutex() );

    Reference<XPossibleHyphens> xRes;

    if (xEntry.is())
    {
        
        OUString aText( xEntry->getDictionaryWord() );
        sal_Int32 nTextLen = aText.getLength();

        
        if (nTextLen > 0  &&  aText[ nTextLen - 1 ] != '=' && aText[ nTextLen - 1 ] != '[')
        {
            
            Sequence< sal_Int16 > aHyphPos( nTextLen );
            sal_Int16 *pPos = aHyphPos.getArray();
            sal_Int32 nHyphCount = 0;

            OUStringBuffer aTmp( nTextLen );
            sal_Bool  bSkip = sal_False;
            sal_Bool  bSkip2 = sal_False;
            sal_Int32 nHyphIdx = -1;
            for (sal_Int32 i = 0;  i < nTextLen;  i++)
            {
                sal_Unicode cTmp = aText[i];
                if (cTmp == '[' || cTmp == ']')
                    bSkip2 = !bSkip2;
                if (cTmp != '=' && !bSkip2 && cTmp != ']')
                {
                    aTmp.append( cTmp );
                    bSkip = sal_False;
                    nHyphIdx++;
                }
                else
                {
                    if (!bSkip  &&  nHyphIdx >= 0)
                        pPos[ nHyphCount++ ] = (sal_Int16) nHyphIdx;
                    bSkip = sal_True;   
                }
            }

            
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
        *pLocales++ = LanguageTag::convertToLocale( aIt->first );
    }
    return aLocales;
}


sal_Bool SAL_CALL HyphenatorDispatcher::hasLocale(const Locale& rLocale)
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    HyphSvcByLangMap_t::const_iterator aIt( aSvcMap.find( LinguLocaleToLanguage( rLocale ) ) );
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
    sal_Int16 nLanguage = LinguLocaleToLanguage( rLocale );
    if (LinguIsUnspecified(nLanguage) || !nWordLen ||
        nMaxLeading == 0 || nMaxLeading == nWordLen)
        return xRes;

    
    HyphSvcByLangMap_t::iterator    aIt( aSvcMap.find( nLanguage ) );
    LangSvcEntries_Hyph     *pEntry = aIt != aSvcMap.end() ? aIt->second.get() : NULL;

    bool bWordModified = false;
    if (!pEntry || (nMaxLeading < 0 || nMaxLeading > nWordLen))
    {
        return NULL;
    }
    else
    {
        OUString aChkWord( rWord );

        
        OUString aSingleQuote( GetLocaleDataWrapper( nLanguage ).getQuotationMarkEnd() );
        DBG_ASSERT( 1 == aSingleQuote.getLength(), "unexpectend length of quotation mark" );
        if (!aSingleQuote.isEmpty())
            aChkWord = aChkWord.replace( aSingleQuote[0], '\'' );

        bWordModified |= RemoveHyphens( aChkWord );
        if (IsIgnoreControlChars( rProperties, GetPropSet() ))
            bWordModified |= RemoveControlChars( aChkWord );
        sal_Int16 nChkMaxLeading = (sal_Int16) GetPosInWordToCheck( rWord, nMaxLeading );

        
        Reference< XDictionaryEntry > xEntry;

        if (GetDicList().is()  &&  IsUseDicList( rProperties, GetPropSet() ))
        {
            xEntry = GetDicList()->queryDictionaryEntry( aChkWord, rLocale,
                        sal_True, sal_False );
        }

        if (xEntry.is())
        {
            
            
            
            
            
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

            
            if (i <= pEntry->nLastTriedSvcIndex)
            {
                if (xHyph.is()  &&  xHyph->hasLocale( rLocale ))
                    xRes = xHyph->hyphenate( aChkWord, rLocale, nChkMaxLeading,
                                            rProperties );
                ++i;
            }
            else if (pEntry->nLastTriedSvcIndex < nLen - 1)
            
            {
                Reference< XHyphenator > *pRef = pEntry->aSvcRefs.getArray();

                Reference< XComponentContext > xContext(
                    comphelper::getProcessComponentContext() );

                
                Sequence< Any > aArgs(2);
                aArgs.getArray()[0] <<= GetPropSet();

                
                try
                {
                    xHyph = Reference< XHyphenator >(
                                xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                                    pEntry->aSvcImplNames[0], aArgs, xContext ),
                                UNO_QUERY );
                }
                catch (uno::Exception &)
                {
                    DBG_ASSERT( false, "createInstanceWithArguments failed" );
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

                
                
                if (xHyph.is()  &&  !xHyph->hasLocale( rLocale ))
                    aSvcMap.erase( nLanguage );
            }
        }   
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
    sal_Int16 nLanguage = LinguLocaleToLanguage( rLocale );
    if (LinguIsUnspecified(nLanguage) || !nWordLen)
        return xRes;

    
    HyphSvcByLangMap_t::iterator    aIt( aSvcMap.find( nLanguage ) );
    LangSvcEntries_Hyph     *pEntry = aIt != aSvcMap.end() ? aIt->second.get() : NULL;

    bool bWordModified = false;
    if (!pEntry || !(0 <= nIndex && nIndex <= nWordLen - 2))
    {
        return NULL;
    }
    else
    {
        OUString aChkWord( rWord );

        
        OUString aSingleQuote( GetLocaleDataWrapper( nLanguage ).getQuotationMarkEnd() );
        DBG_ASSERT( 1 == aSingleQuote.getLength(), "unexpectend length of quotation mark" );
        if (!aSingleQuote.isEmpty())
            aChkWord = aChkWord.replace( aSingleQuote[0], '\'' );

        bWordModified |= RemoveHyphens( aChkWord );
        if (IsIgnoreControlChars( rProperties, GetPropSet() ))
            bWordModified |= RemoveControlChars( aChkWord );
        sal_Int16 nChkIndex = (sal_Int16) GetPosInWordToCheck( rWord, nIndex );

        
        Reference< XDictionaryEntry > xEntry;

        if (GetDicList().is()  &&  IsUseDicList( rProperties, GetPropSet() ))
        {
            xEntry = GetDicList()->queryDictionaryEntry( aChkWord, rLocale,
                        sal_True, sal_False );
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

            
            if (i <= pEntry->nLastTriedSvcIndex)
            {
                if (xHyph.is()  &&  xHyph->hasLocale( rLocale ))
                    xRes = xHyph->queryAlternativeSpelling( aChkWord, rLocale,
                                nChkIndex, rProperties );
                ++i;
            }
            else if (pEntry->nLastTriedSvcIndex < nLen - 1)
            
            {
                Reference< XHyphenator > *pRef = pEntry->aSvcRefs.getArray();

                Reference< XComponentContext > xContext(
                    comphelper::getProcessComponentContext() );

                
                Sequence< Any > aArgs(2);
                aArgs.getArray()[0] <<= GetPropSet();

                
                try
                {
                    xHyph = Reference< XHyphenator >(
                                xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                                    pEntry->aSvcImplNames[0], aArgs, xContext ), UNO_QUERY );
                }
                catch (uno::Exception &)
                {
                    DBG_ASSERT( false, "createInstanceWithArguments failed" );
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

                
                
                if (xHyph.is()  &&  !xHyph->hasLocale( rLocale ))
                    aSvcMap.erase( nLanguage );
            }
        }   
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

    sal_Int16 nLanguage = LinguLocaleToLanguage( rLocale );
    if (LinguIsUnspecified(nLanguage) || rWord.isEmpty())
        return xRes;

    
    HyphSvcByLangMap_t::iterator    aIt( aSvcMap.find( nLanguage ) );
    LangSvcEntries_Hyph     *pEntry = aIt != aSvcMap.end() ? aIt->second.get() : NULL;

    if (pEntry)
    {
        OUString aChkWord( rWord );

        
        OUString aSingleQuote( GetLocaleDataWrapper( nLanguage ).getQuotationMarkEnd() );
        DBG_ASSERT( 1 == aSingleQuote.getLength(), "unexpectend length of quotation mark" );
        if (!aSingleQuote.isEmpty())
            aChkWord = aChkWord.replace( aSingleQuote[0], '\'' );

        RemoveHyphens( aChkWord );
        if (IsIgnoreControlChars( rProperties, GetPropSet() ))
            RemoveControlChars( aChkWord );

        
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

            
            if (i <= pEntry->nLastTriedSvcIndex)
            {
                if (xHyph.is()  &&  xHyph->hasLocale( rLocale ))
                    xRes = xHyph->createPossibleHyphens( aChkWord, rLocale,
                                rProperties );
                ++i;
            }
            else if (pEntry->nLastTriedSvcIndex < nLen - 1)
            
            {
                Reference< XHyphenator > *pRef = pEntry->aSvcRefs.getArray();

                Reference< XComponentContext > xContext(
                    comphelper::getProcessComponentContext() );

                
                Sequence< Any > aArgs(2);
                aArgs.getArray()[0] <<= GetPropSet();

                
                try
                {
                    xHyph = Reference< XHyphenator >(
                                xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                                    pEntry->aSvcImplNames[0], aArgs, xContext ),
                                UNO_QUERY );
                }
                catch (uno::Exception &)
                {
                    DBG_ASSERT( false, "createWithArguments failed" );
                }
                pRef [i] = xHyph;

                Reference< XLinguServiceEventBroadcaster >
                        xBroadcaster( xHyph, UNO_QUERY );
                if (xBroadcaster.is())
                    rMgr.AddLngSvcEvtBroadcaster( xBroadcaster );

                if (xHyph.is()  &&  xHyph->hasLocale( rLocale ))
                    xRes = xHyph->createPossibleHyphens( aChkWord, rLocale, rProperties );

                pEntry->nLastTriedSvcIndex = (sal_Int16) i;
                ++i;

                
                
                if (xHyph.is()  &&  !xHyph->hasLocale( rLocale ))
                    aSvcMap.erase( nLanguage );
            }
        }   
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

    sal_Int16 nLanguage = LinguLocaleToLanguage( rLocale );

    sal_Int32 nLen = rSvcImplNames.getLength();
    if (0 == nLen)
        
        aSvcMap.erase( nLanguage );
    else
    {
        
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

    
    sal_Int16 nLanguage = LinguLocaleToLanguage( rLocale );
    const HyphSvcByLangMap_t::const_iterator  aIt( aSvcMap.find( nLanguage ) );
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
