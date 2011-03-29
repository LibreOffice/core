/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_linguistic.hxx"

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
///////////////////////////////////////////////////////////////////////////


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


///////////////////////////////////////////////////////////////////////////


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

///////////////////////////////////////////////////////////////////////////

}   // namespace linguistic

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
