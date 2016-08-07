/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _LINGUISTIC_SPELLDTA_HXX_
#define _LINGUISTIC_SPELLDTA_HXX_

#include <vector>
#include <com/sun/star/linguistic2/XSpellAlternatives.hpp>
#include <com/sun/star/linguistic2/XSetSpellAlternatives.hpp>

#include <tools/solar.h>

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase2.hxx>
#include <linguistic/lngdllapi.h>

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
                sal_Bool bAllowDuplicates );

void    SeqRemoveNegEntries(
                ::com::sun::star::uno::Sequence< ::rtl::OUString > &rSeq,
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::linguistic2::XDictionaryList > &rxDicList,
                sal_Int16 nLanguage );

sal_Bool    SeqHasEntry(
                const ::com::sun::star::uno::Sequence< ::rtl::OUString > &rSeq,
                const ::rtl::OUString &rTxt);

///////////////////////////////////////////////////////////////////////////

void SearchSimilarText( const rtl::OUString &rText, sal_Int16 nLanguage,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XDictionaryList > &xDicList,
        std::vector< rtl::OUString > & rDicListProps );

///////////////////////////////////////////////////////////////////////////


class SpellAlternatives :
    public cppu::WeakImplHelper2
    <
        ::com::sun::star::linguistic2::XSpellAlternatives,
        ::com::sun::star::linguistic2::XSetSpellAlternatives
    >
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString >  aAlt;   // list of alternatives, may be empty.
    ::rtl::OUString         aWord;
    sal_Int16                   nType;          // type of failure
    sal_Int16                   nLanguage;

    // disallow copy-constructor and assignment-operator for now
    SpellAlternatives(const SpellAlternatives &);
    SpellAlternatives & operator = (const SpellAlternatives &);

public:
    SpellAlternatives();
    SpellAlternatives(const ::rtl::OUString &rWord, sal_Int16 nLang, sal_Int16 nFailureType,
                      const ::rtl::OUString &rRplcWord );
    SpellAlternatives(const ::rtl::OUString &rWord, sal_Int16 nLang, sal_Int16 nFailureType,
                      const ::com::sun::star::uno::Sequence< ::rtl::OUString > &rAlternatives );
    virtual ~SpellAlternatives();

    // XSpellAlternatives
    virtual ::rtl::OUString SAL_CALL getWord(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int16 SAL_CALL getFailureType(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int16 SAL_CALL getAlternativesCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAlternatives(  ) throw (::com::sun::star::uno::RuntimeException);

    // XSetSpellAlternatives
    virtual void SAL_CALL setAlternatives( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aAlternatives ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setFailureType( ::sal_Int16 nFailureType ) throw (::com::sun::star::uno::RuntimeException);

    // non-interface specific functions
    void    SetWordLanguage(const ::rtl::OUString &rWord, sal_Int16 nLang);
    void    SetFailureType(sal_Int16 nTypeP);
    void    SetAlternatives( const ::com::sun::star::uno::Sequence< ::rtl::OUString > &rAlt );
    static com::sun::star::uno::Reference < com::sun::star::linguistic2::XSpellAlternatives > LNG_DLLPUBLIC CreateSpellAlternatives(
        const ::rtl::OUString &rWord, sal_Int16 nLang, sal_Int16 nTypeP, const ::com::sun::star::uno::Sequence< ::rtl::OUString > &rAlt );
};


///////////////////////////////////////////////////////////////////////////

}   // namespace linguistic

#endif

