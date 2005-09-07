/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hyphdta.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:52:28 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _LANG_HXX //autogen wg. LANGUAGE_ENGLISH_US
#include <tools/lang.hxx>
#endif

#include "hyphdta.hxx"
#include "lngprops.hxx"
#include "misc.hxx"

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif


#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ASSERT
#include <tools/debug.hxx>
#endif
#ifndef _SVTOOLS_LNGMISC_HXX_
#include <svtools/lngmisc.hxx>
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif

//using namespace utl;
using namespace osl;
using namespace rtl;
using namespace com::sun::star;
//using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;

namespace linguistic
{
///////////////////////////////////////////////////////////////////////////


HyphenatedWord::HyphenatedWord(const OUString &rWord, INT16 nLang, INT16 nHPos,
                               const OUString &rHyphWord, INT16 nPos ) :
    aWord           (rWord),
    nLanguage       (nLang),
    aHyphenatedWord (rHyphWord),
    nHyphenationPos (nHPos),
    nHyphPos        (nPos)
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


PossibleHyphens::PossibleHyphens(const OUString &rWord, INT16 nLang,
            const OUString &rHyphWord,
            const Sequence< INT16 > &rPositions) :
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

