/*************************************************************************
 *
 *  $RCSfile: hyphdta.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tl $ $Date: 2000-11-17 12:48:39 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _LINGUISTIC_HYPHDTA_HXX_
#define _LINGUISTIC_HYPHDTA_HXX_


#ifndef _COM_SUN_STAR_LINGUISTIC2_XHYPHENATEDWORD_HPP_
#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XPOSSIBLEHYPHENS_HPP_
#include <com/sun/star/linguistic2/XPossibleHyphens.hpp>
#endif

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
    INT16               nHyphPos;
    INT16               nHyphenationPos;
    INT16               nLanguage;
    BOOL                bIsAltSpelling;

    // disallow copy-constructor and assignment-operator for now
    HyphenatedWord(const HyphenatedWord &);
    HyphenatedWord & operator = (const HyphenatedWord &);

public:
    HyphenatedWord(const ::rtl::OUString &rWord, INT16 nLang, INT16 nHyphenationPos,
                   const ::rtl::OUString &rHyphenatedWord, INT16 nHyphenPos );
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
    ::com::sun::star::uno::Sequence< INT16 >    aOrigHyphenPos;
    INT16                       nLanguage;

    // disallow copy-constructor and assignment-operator for now
    PossibleHyphens(const PossibleHyphens &);
    PossibleHyphens & operator = (const PossibleHyphens &);

public:
    PossibleHyphens(const ::rtl::OUString &rWord, INT16 nLang,
            const ::rtl::OUString &rHyphWord,
            const ::com::sun::star::uno::Sequence< INT16 > &rPositions);
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
};


///////////////////////////////////////////////////////////////////////////

} // namespace linguistic

#endif

