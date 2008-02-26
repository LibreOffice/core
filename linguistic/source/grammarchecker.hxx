/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: grammarchecker.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 09:49:53 $
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

#ifndef _LINGUISTIC_GRAMMARCHECKER_HXX_
#define _LINGUISTIC_GRAMMARCHECKER_HXX_


#include <cppuhelper/implbase4.hxx>
#include <com/sun/star/linguistic2/XGrammarChecker.hpp>
#include <com/sun/star/linguistic2/XGrammarCheckingResultBroadcaster.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/interfacecontainer.h>
#include <map>
#include <com/sun/star/linguistic2/SingleGrammarError.hpp>
#include <com/sun/star/linguistic2/XSupportedLocales.hpp>


class GrammarChecker:
    public cppu::WeakImplHelper4
    <
        ::com::sun::star::linguistic2::XGrammarChecker,
        ::com::sun::star::linguistic2::XGrammarCheckingResultBroadcaster,
        ::com::sun::star::lang::XComponent,
        ::com::sun::star::lang::XServiceInfo
    >
{
//    com::sun::star::uno::Reference< uno::XComponentContext > m_xContext;
    ::cppu::OInterfaceContainerHelper m_aGCListeners;
    // disallow use of copy c-tor and assignment operator
    GrammarChecker( const GrammarChecker & );
    GrammarChecker & operator = ( const GrammarChecker & );

public:
    explicit GrammarChecker( /* uno::Reference< uno::XComponentContext > const & rXContext */ );
    virtual ~GrammarChecker();

    //sal_Int32 m_Doc;

    // XSupportedLocales
    virtual ::sal_Bool SAL_CALL hasLocale( const ::com::sun::star::lang::Locale & aLocale) throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale > SAL_CALL getLocales(  ) throw ( ::com::sun::star::uno::RuntimeException );

    // XGrammarChecker
    virtual ::sal_Bool SAL_CALL isSpellChecker( const ::com::sun::star::lang::Locale& aLocale ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL startDocument( ::sal_Int32 nDocId ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL startParagraph( ::sal_Int32 nDocId ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endParagraph( ::sal_Int32 nDocId ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endDocument( ::sal_Int32 nDocId ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL doGrammarChecking( ::sal_Int32 nDocId, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XFlatParagraph >& xFlatPara, const ::com::sun::star::lang::Locale& aLocale, ::sal_Int32 nStartOfSentencePos, ::sal_Int32 nSuggestedSentenceEndPos ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getEndOfSentencePos( ::sal_Int32 nDocId, const ::rtl::OUString& aFlatParaText, const ::com::sun::star::lang::Locale& aLocale, ::sal_Int32 nStartOfSentencePos ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getStartOfSentencePos( ::sal_Int32 nDocId, const ::rtl::OUString& aFlatParaText, const ::com::sun::star::lang::Locale& aLocale ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL requiresPreviousText(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasCheckingDialog(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasOptionsDialog(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL runCheckingDialog( ::sal_Int32 nDocId ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL runOptionsDialog( ::sal_Int32 nDocId ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);


    // XGrammarCheckingResultBroadcaster
    virtual ::sal_Bool SAL_CALL addGrammarCheckingResultListener( const ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XGrammarCheckingResultListener >& xLstnr ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL removeGrammarCheckingResultListener( const ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XGrammarCheckingResultListener >& xLstnr ) throw (::com::sun::star::uno::RuntimeException);

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
        ::sal_Int32 nDocId, const ::rtl::OUString aFlatParaText, const ::com::sun::star::lang::Locale & rLocale,
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

