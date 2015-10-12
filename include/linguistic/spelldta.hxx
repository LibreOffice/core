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
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#include <cppuhelper/implbase.hxx>
#include <linguistic/lngdllapi.h>
#include <boost/noncopyable.hpp>

namespace com { namespace sun { namespace star {
    namespace linguistic2 {
        class XDictionaryList;
    }
} } }


namespace linguistic
{

::com::sun::star::uno::Sequence< OUString >
        MergeProposalSeqs(
                ::com::sun::star::uno::Sequence< OUString > &rAlt1,
                ::com::sun::star::uno::Sequence< OUString > &rAlt2,
                bool bAllowDuplicates );

void    SeqRemoveNegEntries(
                ::com::sun::star::uno::Sequence< OUString > &rSeq,
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::linguistic2::XSearchableDictionaryList > &rxDicList,
                sal_Int16 nLanguage );

bool    SeqHasEntry(
                const ::com::sun::star::uno::Sequence< OUString > &rSeq,
                const OUString &rTxt);

void SearchSimilarText( const OUString &rText, sal_Int16 nLanguage,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XSearchableDictionaryList > &xDicList,
        std::vector< OUString > & rDicListProps );


class SpellAlternatives
    : public cppu::WeakImplHelper
    <
        ::com::sun::star::linguistic2::XSpellAlternatives,
        ::com::sun::star::linguistic2::XSetSpellAlternatives
    >
    , private ::boost::noncopyable
{
    ::com::sun::star::uno::Sequence< OUString >  aAlt;   // list of alternatives, may be empty.
    OUString         aWord;
    sal_Int16                   nType;          // type of failure
    sal_Int16                   nLanguage;

public:
    LNG_DLLPUBLIC SpellAlternatives();
    SpellAlternatives(const OUString &rWord, sal_Int16 nLang, sal_Int16 nFailureType,
                      const ::com::sun::star::uno::Sequence< OUString > &rAlternatives );
    virtual ~SpellAlternatives();

    // XSpellAlternatives
    virtual OUString SAL_CALL getWord(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int16 SAL_CALL getFailureType(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int16 SAL_CALL getAlternativesCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getAlternatives(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XSetSpellAlternatives
    virtual void SAL_CALL setAlternatives( const ::com::sun::star::uno::Sequence< OUString >& aAlternatives ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setFailureType( ::sal_Int16 nFailureType ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // non-interface specific functions
    void    LNG_DLLPUBLIC SetWordLanguage(const OUString &rWord, sal_Int16 nLang);
    void    LNG_DLLPUBLIC SetFailureType(sal_Int16 nTypeP);
    void    LNG_DLLPUBLIC SetAlternatives( const ::com::sun::star::uno::Sequence< OUString > &rAlt );
    static com::sun::star::uno::Reference < com::sun::star::linguistic2::XSpellAlternatives > LNG_DLLPUBLIC CreateSpellAlternatives(
        const OUString &rWord, sal_Int16 nLang, sal_Int16 nTypeP, const ::com::sun::star::uno::Sequence< OUString > &rAlt );
};

}   // namespace linguistic

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
