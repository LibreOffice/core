/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _LINGUISTIC_HYPHDTA_HXX_
#define _LINGUISTIC_HYPHDTA_HXX_


#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>
#include <com/sun/star/linguistic2/XPossibleHyphens.hpp>

#include <tools/solar.h>

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase1.hxx> // helper for implementations


namespace linguistic
{

///////////////////////////////////////////////////////////////////////////

class HyphenatedWord :
    public cppu::WeakImplHelper1
    <
        ::com::sun::star::linguistic2::XHyphenatedWord
    >
{
    ::rtl::OUString     aWord;
    ::rtl::OUString     aHyphenatedWord;
    sal_Int16               nHyphPos;
    sal_Int16               nHyphenationPos;
    sal_Int16               nLanguage;
    sal_Bool                bIsAltSpelling;

    // disallow copy-constructor and assignment-operator for now
    HyphenatedWord(const HyphenatedWord &);
    HyphenatedWord & operator = (const HyphenatedWord &);

public:
    HyphenatedWord(const ::rtl::OUString &rWord, sal_Int16 nLang, sal_Int16 nHyphenationPos,
                   const ::rtl::OUString &rHyphenatedWord, sal_Int16 nHyphenPos );
    virtual ~HyphenatedWord();

    // XHyphenatedWord
    virtual ::rtl::OUString SAL_CALL
        getWord()
            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::lang::Locale SAL_CALL
        getLocale()
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL
        getHyphenationPos()
            throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL
        getHyphenatedWord()
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL
        getHyphenPos()
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL
        isAlternativeSpelling()
            throw(::com::sun::star::uno::RuntimeException);

    ::rtl::OUString GetWord()           { return aWord; }
    ::rtl::OUString GetHyphenatedWord() { return aHyphenatedWord; }
    sal_Int16           GetLanguage()       { return nLanguage; }
    void            SetWord( ::rtl::OUString &rTxt )            { aWord = rTxt; }
    void            SetHyphenatedWord( ::rtl::OUString &rTxt )  { aHyphenatedWord = rTxt; }
    void            SetLanguage( sal_Int16 nLang )                  { nLanguage = nLang; }
};


///////////////////////////////////////////////////////////////////////////

class PossibleHyphens :
    public cppu::WeakImplHelper1
    <
        ::com::sun::star::linguistic2::XPossibleHyphens
    >
{
    ::rtl::OUString             aWord;
    ::rtl::OUString             aWordWithHyphens;
    ::com::sun::star::uno::Sequence< sal_Int16 >    aOrigHyphenPos;
    sal_Int16                       nLanguage;

    // disallow copy-constructor and assignment-operator for now
    PossibleHyphens(const PossibleHyphens &);
    PossibleHyphens & operator = (const PossibleHyphens &);

public:
    PossibleHyphens(const ::rtl::OUString &rWord, sal_Int16 nLang,
            const ::rtl::OUString &rHyphWord,
            const ::com::sun::star::uno::Sequence< sal_Int16 > &rPositions);
    virtual ~PossibleHyphens();

    // XPossibleHyphens
    virtual ::rtl::OUString SAL_CALL
        getWord()
            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::lang::Locale SAL_CALL
        getLocale()
            throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL
        getPossibleHyphens()
            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int16 > SAL_CALL
        getHyphenationPositions()
            throw(::com::sun::star::uno::RuntimeException);

    ::rtl::OUString GetWord()       { return aWord; }
    sal_Int16           GetLanguage()   { return nLanguage; }
    void            SetWord( ::rtl::OUString &rTxt )    { aWord = rTxt; }
    void            SetLanguage( sal_Int16 nLang )          { nLanguage = nLang; }
};


///////////////////////////////////////////////////////////////////////////

} // namespace linguistic

#endif

