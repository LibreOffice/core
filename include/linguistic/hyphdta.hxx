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

#ifndef _LINGUISTIC_HYPHDTA_HXX_
#define _LINGUISTIC_HYPHDTA_HXX_

#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>
#include <com/sun/star/linguistic2/XPossibleHyphens.hpp>
#include <tools/solar.h>
#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase1.hxx> // helper for implementations
#include <linguistic/lngdllapi.h>

namespace linguistic
{

class HyphenatedWord :
    public cppu::WeakImplHelper1
    <
        ::com::sun::star::linguistic2::XHyphenatedWord
    >
{
    OUString     aWord;
    OUString     aHyphenatedWord;
    sal_Int16               nHyphPos;
    sal_Int16               nHyphenationPos;
    sal_Int16               nLanguage;
    sal_Bool                bIsAltSpelling;

    // disallow copy-constructor and assignment-operator for now
    HyphenatedWord(const HyphenatedWord &);
    HyphenatedWord & operator = (const HyphenatedWord &);

public:
    HyphenatedWord(const OUString &rWord, sal_Int16 nLang, sal_Int16 nHyphenationPos,
                   const OUString &rHyphenatedWord, sal_Int16 nHyphenPos );
    virtual ~HyphenatedWord();

    // XHyphenatedWord
    virtual OUString SAL_CALL
        getWord()
            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::lang::Locale SAL_CALL
        getLocale()
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL
        getHyphenationPos()
            throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL
        getHyphenatedWord()
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL
        getHyphenPos()
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL
        isAlternativeSpelling()
            throw(::com::sun::star::uno::RuntimeException);

    OUString GetWord()           { return aWord; }
    OUString GetHyphenatedWord() { return aHyphenatedWord; }
    sal_Int16           GetLanguage()       { return nLanguage; }
    void            SetWord( OUString &rTxt )            { aWord = rTxt; }
    void            SetHyphenatedWord( OUString &rTxt )  { aHyphenatedWord = rTxt; }
    void            SetLanguage( sal_Int16 nLang )                  { nLanguage = nLang; }
    static com::sun::star::uno::Reference <com::sun::star::linguistic2::XHyphenatedWord> LNG_DLLPUBLIC CreateHyphenatedWord(
        const OUString &rWord, sal_Int16 nLang, sal_Int16 nHyphenationPos,
        const OUString &rHyphenatedWord, sal_Int16 nHyphenPos );
};


class PossibleHyphens :
    public cppu::WeakImplHelper1
    <
        ::com::sun::star::linguistic2::XPossibleHyphens
    >
{
    OUString             aWord;
    OUString             aWordWithHyphens;
    ::com::sun::star::uno::Sequence< sal_Int16 >    aOrigHyphenPos;
    sal_Int16                       nLanguage;

    // disallow copy-constructor and assignment-operator for now
    PossibleHyphens(const PossibleHyphens &);
    PossibleHyphens & operator = (const PossibleHyphens &);

public:
    PossibleHyphens(const OUString &rWord, sal_Int16 nLang,
            const OUString &rHyphWord,
            const ::com::sun::star::uno::Sequence< sal_Int16 > &rPositions);
    virtual ~PossibleHyphens();

    // XPossibleHyphens
    virtual OUString SAL_CALL
        getWord()
            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::lang::Locale SAL_CALL
        getLocale()
            throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL
        getPossibleHyphens()
            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int16 > SAL_CALL
        getHyphenationPositions()
            throw(::com::sun::star::uno::RuntimeException);

    OUString GetWord()       { return aWord; }
    sal_Int16           GetLanguage()   { return nLanguage; }
    void            SetWord( OUString &rTxt )    { aWord = rTxt; }
    void            SetLanguage( sal_Int16 nLang )          { nLanguage = nLang; }

    static com::sun::star::uno::Reference < com::sun::star::linguistic2::XPossibleHyphens > LNG_DLLPUBLIC CreatePossibleHyphens
        (const OUString &rWord, sal_Int16 nLang,
         const OUString &rHyphWord,
         const ::com::sun::star::uno::Sequence< sal_Int16 > &rPositions);
};
} // namespace linguistic

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
