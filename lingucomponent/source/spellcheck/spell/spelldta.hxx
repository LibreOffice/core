/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: spelldta.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _LINGUISTIC_SPELLDTA_HXX_
#define _LINGUISTIC_SPELLDTA_HXX_


#include <com/sun/star/linguistic2/XSpellAlternatives.hpp>

#include <tools/solar.h>

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase1.hxx> // helper for implementations

namespace com { namespace sun { namespace star {
    namespace linguistic2 {
        class XDictionaryList;
    }
} } }


namespace linguistic
{

///////////////////////////////////////////////////////////////////////////

::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XSpellAlternatives >
        MergeProposals(
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::linguistic2::XSpellAlternatives > &rxAlt1,
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::linguistic2::XSpellAlternatives > &rxAlt2 );

::com::sun::star::uno::Sequence< ::rtl::OUString >
        MergeProposalSeqs(
                ::com::sun::star::uno::Sequence< ::rtl::OUString > &rAlt1,
                ::com::sun::star::uno::Sequence< ::rtl::OUString > &rAlt2,
                BOOL bAllowDuplicates );

void    SeqRemoveNegEntries(
                ::com::sun::star::uno::Sequence< ::rtl::OUString > &rSeq,
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::linguistic2::XDictionaryList > &rxDicList,
                INT16 nLanguage );

BOOL    SeqHasEntry(
                const ::com::sun::star::uno::Sequence< ::rtl::OUString > &rSeq,
                const ::rtl::OUString &rTxt);

///////////////////////////////////////////////////////////////////////////


class SpellAlternatives :
    public cppu::WeakImplHelper1
    <
        ::com::sun::star::linguistic2::XSpellAlternatives
    >
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString >  aAlt;   // list of alternatives, may be empty.
    ::rtl::OUString         aWord;
    INT16                   nType;          // type of failure
    INT16                   nLanguage;

    // disallow copy-constructor and assignment-operator for now
    SpellAlternatives(const SpellAlternatives &);
    SpellAlternatives & operator = (const SpellAlternatives &);

public:
    SpellAlternatives();
    SpellAlternatives(const ::rtl::OUString &rWord, INT16 nLang, INT16 nFailureType,
                      const ::rtl::OUString &rRplcWord );
    SpellAlternatives(const ::rtl::OUString &rWord, INT16 nLang, INT16 nFailureType,
                      const ::com::sun::star::uno::Sequence< ::rtl::OUString > &rAlternatives );
    virtual ~SpellAlternatives();

    // XSpellAlternatives
    virtual ::rtl::OUString SAL_CALL
        getWord()
            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::lang::Locale SAL_CALL
        getLocale()
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL
        getFailureType()
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL
        getAlternativesCount()
            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
        getAlternatives()
            throw(::com::sun::star::uno::RuntimeException);

    // non-interface specific functions
    void    SetWordLanguage(const ::rtl::OUString &rWord, INT16 nLang);
    void    SetFailureType(INT16 nTypeP);
    void    SetAlternatives(
                const ::com::sun::star::uno::Sequence< ::rtl::OUString > &rAlt );
};


///////////////////////////////////////////////////////////////////////////

}   // namespace linguistic

#endif

