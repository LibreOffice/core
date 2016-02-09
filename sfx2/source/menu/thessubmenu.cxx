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


#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/linguistic2/XThesaurus.hpp>
#include <com/sun/star/linguistic2/XMeaning.hpp>
#include <com/sun/star/linguistic2/LinguServiceManager.hpp>

#include <comphelper/processfactory.hxx>
#include <svl/stritem.hxx>
#include <tools/debug.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>


#include <vector>

#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewsh.hxx>
#include "thessubmenu.hxx"

using namespace ::com::sun::star;

OUString SfxThesSubMenuHelper::GetText(
    const OUString &rLookUpString,
    sal_Int32 nDelimPos )
{
    return rLookUpString.copy( 0, nDelimPos );
}


void SfxThesSubMenuHelper::GetLocale(
    lang::Locale /*out */ &rLocale,
    const OUString &rLookUpString,
    sal_Int32 nDelimPos  )
{
    OUString aIsoLang( rLookUpString.copy( nDelimPos + 1) );
    rLocale = LanguageTag::convertToLocale( aIsoLang);
}


SfxThesSubMenuHelper::SfxThesSubMenuHelper():
    m_xLngMgr(
        linguistic2::LinguServiceManager::create(
            comphelper::getProcessComponentContext())),
    m_xThesarus(m_xLngMgr->getThesaurus())
{
}


SfxThesSubMenuHelper::~SfxThesSubMenuHelper()
{
}


bool SfxThesSubMenuHelper::IsSupportedLocale( const lang::Locale & rLocale ) const
{
    return m_xThesarus.is() && m_xThesarus->hasLocale( rLocale );
}


bool SfxThesSubMenuHelper::GetMeanings(
    std::vector< OUString > & rSynonyms,
    const OUString & rWord,
    const lang::Locale & rLocale,
    sal_Int16 nMaxSynonms )
{
    bool bHasMoreSynonyms = false;
    rSynonyms.clear();
    if (IsSupportedLocale( rLocale ) && !rWord.isEmpty() && nMaxSynonms > 0)
    {
        try
        {
            // get all meannings
            const uno::Sequence< uno::Reference< linguistic2::XMeaning > > aMeaningSeq(
                    m_xThesarus->queryMeanings( rWord, rLocale, uno::Sequence< beans::PropertyValue >() ));
            const uno::Reference< linguistic2::XMeaning > *pxMeaning = aMeaningSeq.getConstArray();
            const sal_Int32 nMeanings = aMeaningSeq.getLength();

            // iterate over all meanings until nMaxSynonms are found or all meanings are processed
            sal_Int32 nCount = 0;
            sal_Int32 i = 0;
            for ( ;  i < nMeanings && nCount < nMaxSynonms;  ++i)
            {
                const uno::Sequence< OUString > aSynonymSeq( pxMeaning[i]->querySynonyms() );
                const OUString *pSynonyms = aSynonymSeq.getConstArray();
                const sal_Int32 nSynonyms = aSynonymSeq.getLength();
                sal_Int32 k = 0;
                for ( ;  k < nSynonyms && nCount < nMaxSynonms;  ++k)
                {
                    rSynonyms.push_back( pSynonyms[k] );
                    ++nCount;
                }
                bHasMoreSynonyms = k < nSynonyms;    // any synonym from this meaning skipped?
            }

            bHasMoreSynonyms |= i < nMeanings;   // any meaning skipped?
        }
        catch (const uno::Exception &)
        {
            DBG_ASSERT( false, "failed to get synonyms" );
        }
    }
    return bHasMoreSynonyms;
}


OUString SfxThesSubMenuHelper::GetThesImplName( const lang::Locale &rLocale ) const
{
    OUString aRes;
    uno::Sequence< OUString > aServiceNames = m_xLngMgr->getConfiguredServices(
            "com.sun.star.linguistic2.Thesaurus", rLocale );
    // there should be at most one thesaurus configured for each language
    DBG_ASSERT( aServiceNames.getLength() <= 1, "more than one thesaurus found. Should not be possible" );
    if (aServiceNames.getLength() == 1)
        aRes = aServiceNames[0];
    return aRes;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
