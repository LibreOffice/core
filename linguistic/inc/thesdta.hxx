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



#ifndef _LINGUISTIC_THESDTA_HXX_
#define _LINGUISTIC_THESDTA_HXX_


#include <com/sun/star/linguistic2/XMeaning.hpp>

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase1.hxx> // helper for implementations


namespace linguistic
{

///////////////////////////////////////////////////////////////////////////


class ThesaurusMeaning :
    public cppu::WeakImplHelper1
    <
        ::com::sun::star::linguistic2::XMeaning
    >
{

protected:
    ::rtl::OUString aText;              // one of the found 'meanings' for the looked up text
    ::rtl::OUString aLookUpText;        // text that was looked up in the thesaurus
    sal_Int16           nLookUpLanguage;    // language of the text that was looked up

    // disallow copy-constructor and assignment-operator for now
    ThesaurusMeaning(const ThesaurusMeaning &);
    ThesaurusMeaning & operator = (const ThesaurusMeaning &);

public:
    ThesaurusMeaning(const ::rtl::OUString &rText,
            const ::rtl::OUString &rLookUpText, sal_Int16 nLookUpLang );
    virtual ~ThesaurusMeaning();

    // XMeaning
    virtual ::rtl::OUString SAL_CALL
        getMeaning()
            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
        querySynonyms()
            throw(::com::sun::star::uno::RuntimeException) = 0;

    // non-interface specific functions
    const ::rtl::OUString & getLookUpText() const       { return aLookUpText; }
    short                   getLookUpLanguage() const   { return nLookUpLanguage; }
};


///////////////////////////////////////////////////////////////////////////

} // namespace linguistic

#endif

