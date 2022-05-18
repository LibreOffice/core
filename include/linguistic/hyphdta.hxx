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

#ifndef INCLUDED_LINGUISTIC_HYPHDTA_HXX
#define INCLUDED_LINGUISTIC_HYPHDTA_HXX

#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>
#include <com/sun/star/linguistic2/XPossibleHyphens.hpp>
#include <cppuhelper/implbase.hxx>
#include <linguistic/lngdllapi.h>
#include <i18nlangtag/lang.h>
#include <config_options.h>

namespace linguistic
{

class HyphenatedWord final :
    public cppu::WeakImplHelper< css::linguistic2::XHyphenatedWord >
{
    OUString     aWord;
    OUString     aHyphenatedWord;
    sal_Int16    nHyphPos;
    sal_Int16    nHyphenationPos;
    LanguageType nLanguage;
    bool         bIsAltSpelling;

    HyphenatedWord(const HyphenatedWord &) = delete;
    HyphenatedWord & operator = (const HyphenatedWord &) = delete;

public:
    HyphenatedWord(const OUString &rWord, LanguageType nLang, sal_Int16 nHyphenationPos,
                   const OUString &rHyphenatedWord, sal_Int16 nHyphenPos );
    virtual ~HyphenatedWord() override;

    // XHyphenatedWord
    virtual OUString SAL_CALL
        getWord() override;
    virtual css::lang::Locale SAL_CALL
        getLocale() override;
    virtual sal_Int16 SAL_CALL
        getHyphenationPos() override;
    virtual OUString SAL_CALL
        getHyphenatedWord() override;
    virtual sal_Int16 SAL_CALL
        getHyphenPos() override;
    virtual sal_Bool SAL_CALL
        isAlternativeSpelling() override;

    static css::uno::Reference <css::linguistic2::XHyphenatedWord> UNLESS_MERGELIBS(LNG_DLLPUBLIC) CreateHyphenatedWord(
        const OUString &rWord, LanguageType nLang, sal_Int16 nHyphenationPos,
        const OUString &rHyphenatedWord, sal_Int16 nHyphenPos );
};


class PossibleHyphens final :
    public cppu::WeakImplHelper
    <
        css::linguistic2::XPossibleHyphens
    >
{
    OUString             aWord;
    OUString             aWordWithHyphens;
    css::uno::Sequence< sal_Int16 > aOrigHyphenPos;
    LanguageType         nLanguage;

    PossibleHyphens(const PossibleHyphens &) = delete;
    PossibleHyphens & operator = (const PossibleHyphens &) = delete;

public:
    PossibleHyphens(OUString aWord, LanguageType nLang,
            OUString aHyphWord,
            const css::uno::Sequence< sal_Int16 > &rPositions);
    virtual ~PossibleHyphens() override;

    // XPossibleHyphens
    virtual OUString SAL_CALL
        getWord() override;
    virtual css::lang::Locale SAL_CALL
        getLocale() override;
    virtual OUString SAL_CALL
        getPossibleHyphens() override;
    virtual css::uno::Sequence< sal_Int16 > SAL_CALL
        getHyphenationPositions() override;

    static css::uno::Reference < css::linguistic2::XPossibleHyphens > UNLESS_MERGELIBS(LNG_DLLPUBLIC) CreatePossibleHyphens
        (const OUString &rWord, LanguageType nLang,
         const OUString &rHyphWord,
         const css::uno::Sequence< sal_Int16 > &rPositions);
};
} // namespace linguistic

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
