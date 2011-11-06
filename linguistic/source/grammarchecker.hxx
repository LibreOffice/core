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



#ifndef _LINGUISTIC_GRAMMARCHECKER_HXX_
#define _LINGUISTIC_GRAMMARCHECKER_HXX_


#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/linguistic2/XGrammarChecker.hpp>
#include <com/sun/star/linguistic2/SingleGrammarError.hpp>
#include <com/sun/star/linguistic2/XSupportedLocales.hpp>

#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/interfacecontainer.h>

#include <map>


class GrammarChecker:
    public cppu::WeakImplHelper3
    <
        ::com::sun::star::linguistic2::XGrammarChecker,
        ::com::sun::star::lang::XComponent,
        ::com::sun::star::lang::XServiceInfo
    >
{
//    com::sun::star::uno::Reference< uno::XComponentContext > m_xContext;

    // disallow use of copy c-tor and assignment operator
    GrammarChecker( const GrammarChecker & );
    GrammarChecker & operator = ( const GrammarChecker & );

public:
    explicit GrammarChecker( /* uno::Reference< uno::XComponentContext > const & rXContext */ );
    virtual ~GrammarChecker();

    // XSupportedLocales
    virtual ::sal_Bool SAL_CALL hasLocale( const ::com::sun::star::lang::Locale & aLocale) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale > SAL_CALL getLocales(  ) throw ( ::com::sun::star::uno::RuntimeException );

    // XGrammarChecker
    virtual ::sal_Bool SAL_CALL isSpellChecker(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL startDocument( ::sal_Int32 nDocId ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL startParagraph( ::sal_Int32 nDocId ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endParagraph( ::sal_Int32 nDocId ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endDocument( ::sal_Int32 nDocId ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::linguistic2::GrammarCheckingResult SAL_CALL doGrammarChecking( ::sal_Int32 nDocId, const ::rtl::OUString& aText, const ::com::sun::star::lang::Locale& aLocale, ::sal_Int32 nStartOfSentencePos, ::sal_Int32 nSuggestedEndOfSentencePos, const ::com::sun::star::uno::Sequence< ::sal_Int32 >& aLanguagePortions, const ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale >& aLanguagePortionsLocales ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasOptionsDialog(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL runOptionsDialog(  ) throw (::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw ( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);

    static inline ::rtl::OUString getImplementationName_Static();
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static() throw();

    ::com::sun::star::uno::Sequence< ::com::sun::star::linguistic2::SingleGrammarError > GrammarCheckingInDummy(
        ::sal_Int32 nDocId, const ::rtl::OUString &rFlatParaText, const ::com::sun::star::lang::Locale & rLocale,
        ::sal_Int32 nStartOfSentencePos, ::sal_Int32 nSuggestedSentenceEndPos );


    inline ::osl::Mutex & GetMutex()
    {
        static osl::Mutex aMutex;
        return aMutex;
    }
};

inline ::rtl::OUString GrammarChecker::getImplementationName_Static()
{
    return A2OU( "com.sun.star.lingu2.GrammarChecker" );
}


#endif

