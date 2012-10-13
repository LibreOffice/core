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


#include "linguistic/hyphdta.hxx"
#include "linguistic/lngprops.hxx"
#include "linguistic/misc.hxx"
#include <osl/mutex.hxx>


#include <rtl/ustrbuf.hxx>
#include <tools/debug.hxx>
#include <svl/lngmisc.hxx>
#include <unotools/localedatawrapper.hxx>

using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;

using ::rtl::OUString;

namespace linguistic
{


HyphenatedWord::HyphenatedWord(const OUString &rWord, sal_Int16 nLang, sal_Int16 nHPos,
                               const OUString &rHyphWord, sal_Int16 nPos ) :
    aWord           (rWord),
    aHyphenatedWord (rHyphWord),
    nHyphPos        (nPos),
    nHyphenationPos (nHPos),
    nLanguage       (nLang)
{
    String aSingleQuote( GetLocaleDataWrapper( nLanguage ).getQuotationMarkEnd() );
    DBG_ASSERT( 1 == aSingleQuote.Len(), "unexpectend length of quotation mark" );
    if (aSingleQuote.Len())
    {
        // ignore typographical apostrophes (which got replaced in original
        // word when being checked for hyphenation) in results.
        OUString aTmpWord( rWord );
        OUString aTmpHyphWord( rHyphWord );
        aTmpWord        = aTmpWord    .replace( aSingleQuote.GetChar(0), '\'' );
        aTmpHyphWord    = aTmpHyphWord.replace( aSingleQuote.GetChar(0), '\'' );
        bIsAltSpelling  = aTmpWord != aTmpHyphWord;
    }
    else
        bIsAltSpelling = rWord != rHyphWord;
}


HyphenatedWord::~HyphenatedWord()
{
}


OUString SAL_CALL HyphenatedWord::getWord()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return aWord;
}


Locale SAL_CALL HyphenatedWord::getLocale()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    Locale aRes;
    return LanguageToLocale( aRes, nLanguage );
}


sal_Int16 SAL_CALL HyphenatedWord::getHyphenationPos()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return nHyphenationPos;
}


OUString SAL_CALL HyphenatedWord::getHyphenatedWord()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return aHyphenatedWord;
}


sal_Int16 SAL_CALL HyphenatedWord::getHyphenPos()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return nHyphPos;
}


sal_Bool SAL_CALL HyphenatedWord::isAlternativeSpelling()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return bIsAltSpelling;
}




PossibleHyphens::PossibleHyphens(const OUString &rWord, sal_Int16 nLang,
            const OUString &rHyphWord,
            const Sequence< sal_Int16 > &rPositions) :
    aWord           (rWord),
    aWordWithHyphens(rHyphWord),
    aOrigHyphenPos  (rPositions),
    nLanguage       (nLang)
{
}


PossibleHyphens::~PossibleHyphens()
{
}


OUString SAL_CALL PossibleHyphens::getWord()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return aWord;
}


Locale SAL_CALL PossibleHyphens::getLocale()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return CreateLocale( nLanguage );
}


OUString SAL_CALL PossibleHyphens::getPossibleHyphens()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return aWordWithHyphens;
}


Sequence< sal_Int16 > SAL_CALL PossibleHyphens::getHyphenationPositions()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return aOrigHyphenPos;
}

com::sun::star::uno::Reference <com::sun::star::linguistic2::XHyphenatedWord> HyphenatedWord::CreateHyphenatedWord(
        const ::rtl::OUString &rWord, sal_Int16 nLang, sal_Int16 nHyphenationPos,
        const ::rtl::OUString &rHyphenatedWord, sal_Int16 nHyphenPos )
{
    return new HyphenatedWord( rWord, nLang, nHyphenationPos, rHyphenatedWord, nHyphenPos );
}

com::sun::star::uno::Reference < com::sun::star::linguistic2::XPossibleHyphens > PossibleHyphens::CreatePossibleHyphens
        (const ::rtl::OUString &rWord, sal_Int16 nLang,
         const ::rtl::OUString &rHyphWord,
         const ::com::sun::star::uno::Sequence< sal_Int16 > &rPositions)
{
    return new PossibleHyphens( rWord, nLang, rHyphWord, rPositions );
}



}   // namespace linguistic

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
