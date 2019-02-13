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

#ifndef INCLUDED_LINGUISTIC_SPELLDTA_HXX
#define INCLUDED_LINGUISTIC_SPELLDTA_HXX

#include <vector>
#include <com/sun/star/linguistic2/XSpellAlternatives.hpp>
#include <com/sun/star/linguistic2/XSetSpellAlternatives.hpp>
#include <cppuhelper/implbase.hxx>
#include <linguistic/lngdllapi.h>
#include <i18nlangtag/lang.h>

namespace com::sun::star::linguistic2 { class XSearchableDictionaryList; }


namespace linguistic
{

std::vector< OUString >
        MergeProposalSeqs(
                std::vector< OUString > &rAlt1,
                std::vector< OUString > &rAlt2 );

void    SeqRemoveNegEntries(
                std::vector< OUString > &rSeq,
                css::uno::Reference< css::linguistic2::XSearchableDictionaryList > const &rxDicList,
                LanguageType nLanguage );

void SearchSimilarText( const OUString &rText, LanguageType nLanguage,
        css::uno::Reference< css::linguistic2::XSearchableDictionaryList > const &xDicList,
        std::vector< OUString > & rDicListProps );


class SpellAlternatives
    : public cppu::WeakImplHelper
    <
        css::linguistic2::XSpellAlternatives,
        css::linguistic2::XSetSpellAlternatives
    >
{
    css::uno::Sequence< OUString >  aAlt;   // list of alternatives, may be empty.
    OUString                        aWord;
    sal_Int16                       nType;          // type of failure
    LanguageType                    nLanguage;

public:
    LNG_DLLPUBLIC SpellAlternatives();
    SpellAlternatives(const OUString &rWord, LanguageType nLang,
                      const css::uno::Sequence< OUString > &rAlternatives );
    virtual ~SpellAlternatives() override;
    SpellAlternatives(const SpellAlternatives&) = delete;
    SpellAlternatives& operator=( const SpellAlternatives& ) = delete;

    // XSpellAlternatives
    virtual OUString SAL_CALL getWord(  ) override;
    virtual css::lang::Locale SAL_CALL getLocale(  ) override;
    virtual ::sal_Int16 SAL_CALL getFailureType(  ) override;
    virtual ::sal_Int16 SAL_CALL getAlternativesCount(  ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getAlternatives(  ) override;

    // XSetSpellAlternatives
    virtual void SAL_CALL setAlternatives( const css::uno::Sequence< OUString >& aAlternatives ) override;
    virtual void SAL_CALL setFailureType( ::sal_Int16 nFailureType ) override;

    // non-interface specific functions
    void    LNG_DLLPUBLIC SetWordLanguage(const OUString &rWord, LanguageType nLang);
    void    LNG_DLLPUBLIC SetFailureType(sal_Int16 nTypeP);
    void    LNG_DLLPUBLIC SetAlternatives( const css::uno::Sequence< OUString > &rAlt );
    static css::uno::Reference < css::linguistic2::XSpellAlternatives > LNG_DLLPUBLIC CreateSpellAlternatives(
        const OUString &rWord, LanguageType nLang, sal_Int16 nTypeP, const css::uno::Sequence< OUString > &rAlt );
};

}   // namespace linguistic

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
