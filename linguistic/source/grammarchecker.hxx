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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
