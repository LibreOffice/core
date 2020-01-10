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

#include <i18nlangtag/lang.h>
#include <i18nlangtag/languagetag.hxx>
#include <tools/debug.hxx>
#include <svl/lngmisc.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <osl/mutex.hxx>
#include <sal/log.hxx>

#include "thesdsp.hxx"
#include <linguistic/misc.hxx>

using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;


static bool SvcListHasLanguage(
        const Sequence< Reference< XThesaurus > > &rRefs,
        const Locale &rLocale )
{
    return std::any_of(rRefs.begin(), rRefs.end(),
        [&rLocale](const Reference<XThesaurus>& rRef) {
            return rRef.is() && rRef->hasLocale( rLocale ); });
}


ThesaurusDispatcher::ThesaurusDispatcher()
{
}


ThesaurusDispatcher::~ThesaurusDispatcher()
{
    ClearSvcList();
}


void ThesaurusDispatcher::ClearSvcList()
{
    // release memory for each table entry
    ThesSvcByLangMap_t aTmp;
    aSvcMap.swap( aTmp );
}


Sequence< Locale > SAL_CALL
    ThesaurusDispatcher::getLocales()
{
    MutexGuard  aGuard( GetLinguMutex() );

    std::vector<Locale> aLocales;
    aLocales.reserve(aSvcMap.size());

    std::transform(aSvcMap.begin(), aSvcMap.end(), std::back_inserter(aLocales),
        [](ThesSvcByLangMap_t::const_reference elem) { return LanguageTag::convertToLocale(elem.first); });

    return comphelper::containerToSequence(aLocales);
}


sal_Bool SAL_CALL
    ThesaurusDispatcher::hasLocale( const Locale& rLocale )
{
    MutexGuard  aGuard( GetLinguMutex() );
    ThesSvcByLangMap_t::const_iterator aIt( aSvcMap.find( LinguLocaleToLanguage( rLocale ) ) );
    return aIt != aSvcMap.end();
}


Sequence< Reference< XMeaning > > SAL_CALL
    ThesaurusDispatcher::queryMeanings(
            const OUString& rTerm, const Locale& rLocale,
            const css::uno::Sequence< ::css::beans::PropertyValue >& rProperties )
{
    MutexGuard  aGuard( GetLinguMutex() );

    Sequence< Reference< XMeaning > >   aMeanings;

    LanguageType nLanguage = LinguLocaleToLanguage( rLocale );
    if (LinguIsUnspecified( nLanguage) || rTerm.isEmpty())
        return aMeanings;

    // search for entry with that language
    ThesSvcByLangMap_t::iterator    aIt( aSvcMap.find( nLanguage ) );
    LangSvcEntries_Thes     *pEntry = aIt != aSvcMap.end() ? aIt->second.get() : nullptr;

    if (pEntry)
    {
        OUString aChkWord = rTerm.replace( SVT_HARD_SPACE, ' ' );
        RemoveHyphens( aChkWord );
        if (IsIgnoreControlChars( rProperties, GetPropSet() ))
            RemoveControlChars( aChkWord );

        sal_Int32 nLen = pEntry->aSvcRefs.getLength();
        DBG_ASSERT( nLen == pEntry->aSvcImplNames.getLength(),
                "lng : sequence length mismatch");
        DBG_ASSERT( pEntry->nLastTriedSvcIndex < nLen,
                "lng : index out of range");

        sal_Int32 i = 0;

        // try already instantiated services first
        {
            const Reference< XThesaurus > *pRef = pEntry->aSvcRefs.getConstArray();
            while (i <= pEntry->nLastTriedSvcIndex
                   &&  !aMeanings.hasElements())
            {
                if (pRef[i].is()  &&  pRef[i]->hasLocale( rLocale ))
                    aMeanings = pRef[i]->queryMeanings( aChkWord, rLocale, rProperties );
                ++i;
            }
        }

        // if still no result instantiate new services and try those
        if (!aMeanings.hasElements()
            &&  pEntry->nLastTriedSvcIndex < nLen - 1)
        {
            const OUString *pImplNames = pEntry->aSvcImplNames.getConstArray();
            Reference< XThesaurus > *pRef = pEntry->aSvcRefs.getArray();

            Reference< XComponentContext > xContext(
                comphelper::getProcessComponentContext() );

            // build service initialization argument
            Sequence< Any > aArgs(1);
            aArgs.getArray()[0] <<= GetPropSet();

            while (i < nLen  &&  !aMeanings.hasElements())
            {
                // create specific service via it's implementation name
                Reference< XThesaurus > xThes;
                try
                {
                    xThes.set(  xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                                    pImplNames[i], aArgs, xContext ),
                                UNO_QUERY );
                }
                catch (uno::Exception &)
                {
                    SAL_WARN( "linguistic", "createInstanceWithArguments failed" );
                }
                pRef[i] = xThes;

                if (xThes.is()  &&  xThes->hasLocale( rLocale ))
                    aMeanings = xThes->queryMeanings( aChkWord, rLocale, rProperties );

                pEntry->nLastTriedSvcIndex = static_cast<sal_Int16>(i);
                ++i;
            }

            // if language is not supported by any of the services
            // remove it from the list.
            if (i == nLen  &&  !aMeanings.hasElements())
            {
                if (!SvcListHasLanguage( pEntry->aSvcRefs, rLocale ))
                    aSvcMap.erase( nLanguage );
            }
        }
    }

    return aMeanings;
}


void ThesaurusDispatcher::SetServiceList( const Locale &rLocale,
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
        LangSvcEntries_Thes *pEntry = aSvcMap[ nLanguage ].get();
        if (pEntry)
        {
            pEntry->Clear();
            pEntry->aSvcImplNames = rSvcImplNames;
            pEntry->aSvcRefs = Sequence< Reference < XThesaurus > >( nLen );
        }
        else
        {
            auto pTmpEntry = std::make_shared<LangSvcEntries_Thes>( rSvcImplNames );
            pTmpEntry->aSvcRefs = Sequence< Reference < XThesaurus > >( nLen );
            aSvcMap[ nLanguage ] = pTmpEntry;
        }
    }
}


Sequence< OUString >
    ThesaurusDispatcher::GetServiceList( const Locale &rLocale ) const
{
    MutexGuard  aGuard( GetLinguMutex() );

    Sequence< OUString > aRes;

    // search for entry with that language and use data from that
    LanguageType nLanguage = LinguLocaleToLanguage( rLocale );
    const ThesSvcByLangMap_t::const_iterator  aIt( aSvcMap.find( nLanguage ) );
    const LangSvcEntries_Thes       *pEntry = aIt != aSvcMap.end() ? aIt->second.get() : nullptr;
    if (pEntry)
        aRes = pEntry->aSvcImplNames;

    return aRes;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
