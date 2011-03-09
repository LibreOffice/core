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

#ifndef _LINGUISTIC_LNGSVCMGR_HXX_
#define _LINGUISTIC_LNGSVCMGR_HXX_

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase4.hxx> // helper for implementations
#include <cppuhelper/interfacecontainer.h>  //OMultiTypeInterfaceContainerHelper


#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/linguistic2/XLinguServiceManager.hpp>
#include <com/sun/star/linguistic2/XAvailableLocales.hpp>
#include <unotools/configitem.hxx>

//#include <vcl/timer.hxx>

#include "linguistic/misc.hxx"
#include "defs.hxx"

class SpellCheckerDispatcher;
class HyphenatorDispatcher;
class ThesaurusDispatcher;
class GrammarCheckingIterator;
class LngSvcMgrListenerHelper;
struct SvcInfo;

namespace com { namespace sun { namespace star { namespace linguistic2 {
    class XLinguServiceEventBroadcaster;
    class XSpellChecker;
    class XProofreader;
    class XProofreadingIterator;
    class XHyphenator;
    class XThesaurus;
} } } }

///////////////////////////////////////////////////////////////////////////


class LngSvcMgr :
    public cppu::WeakImplHelper4
    <
        com::sun::star::linguistic2::XLinguServiceManager,
        com::sun::star::linguistic2::XAvailableLocales,
        com::sun::star::lang::XComponent,
        com::sun::star::lang::XServiceInfo
    >,
    private utl::ConfigItem
{
    friend class LngSvcMgrListenerHelper;

    ::cppu::OInterfaceContainerHelper                   aEvtListeners;

    com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker >              xSpellDsp;
    com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XProofreadingIterator >      xGrammarDsp;
    com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenator >                xHyphDsp;
    com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XThesaurus >                 xThesDsp;

    com::sun::star::uno::Reference<
        ::com::sun::star::lang::XEventListener >        xListenerHelper;

    com::sun::star::uno::Sequence<
        com::sun::star::lang::Locale >                  aAvailSpellLocales;
    com::sun::star::uno::Sequence<
        com::sun::star::lang::Locale >                  aAvailGrammarLocales;
    com::sun::star::uno::Sequence<
        com::sun::star::lang::Locale >                  aAvailHyphLocales;
    com::sun::star::uno::Sequence<
        com::sun::star::lang::Locale >                  aAvailThesLocales;

    SpellCheckerDispatcher *                            pSpellDsp;
    GrammarCheckingIterator *                           pGrammarDsp;
    HyphenatorDispatcher *                              pHyphDsp;
    ThesaurusDispatcher *                               pThesDsp;

    LngSvcMgrListenerHelper *                           pListenerHelper;

    typedef std::vector< SvcInfo * >    SvcInfoArray;
    SvcInfoArray *                                      pAvailSpellSvcs;
    SvcInfoArray *                                      pAvailGrammarSvcs;
    SvcInfoArray *                                      pAvailHyphSvcs;
    SvcInfoArray *                                      pAvailThesSvcs;

    sal_Bool bDisposing;
    sal_Bool bHasAvailSpellLocales;
    sal_Bool bHasAvailGrammarLocales;
    sal_Bool bHasAvailHyphLocales;
    sal_Bool bHasAvailThesLocales;

    // disallow copy-constructor and assignment-operator for now
    LngSvcMgr(const LngSvcMgr &);
    LngSvcMgr & operator = (const LngSvcMgr &);

    void    GetAvailableSpellSvcs_Impl();
    void    GetAvailableGrammarSvcs_Impl();
    void    GetAvailableHyphSvcs_Impl();
    void    GetAvailableThesSvcs_Impl();
    void    GetListenerHelper_Impl();

    void    GetSpellCheckerDsp_Impl( sal_Bool bSetSvcList = sal_True );
    void    GetGrammarCheckerDsp_Impl( sal_Bool bSetSvcList = sal_True );
    void    GetHyphenatorDsp_Impl( sal_Bool bSetSvcList = sal_True );
    void    GetThesaurusDsp_Impl( sal_Bool bSetSvcList = sal_True );

    void    SetCfgServiceLists( SpellCheckerDispatcher &rSpellDsp );
    void    SetCfgServiceLists( GrammarCheckingIterator &rGrammarDsp );
    void    SetCfgServiceLists( HyphenatorDispatcher &rHyphDsp );
    void    SetCfgServiceLists( ThesaurusDispatcher &rThesDsp );

    sal_Bool    SaveCfgSvcs( const String &rServiceName );

    void    SetAvailableCfgServiceLists( LinguDispatcher &rDispatcher,
                    const SvcInfoArray &rAvailSvcs );

    static void clearSvcInfoArray(SvcInfoArray *pInfo);

    // utl::ConfigItem (to allow for listening of changes of relevant properties)
    virtual void    Notify( const com::sun::star::uno::Sequence< rtl::OUString > &rPropertyNames );
    virtual void    Commit();

public:
    LngSvcMgr();
    virtual ~LngSvcMgr();

    // XLinguServiceManager
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSpellChecker > SAL_CALL getSpellChecker(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XHyphenator > SAL_CALL getHyphenator(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XThesaurus > SAL_CALL getThesaurus(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL addLinguServiceManagerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL removeLinguServiceManagerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAvailableServices( const ::rtl::OUString& aServiceName, const ::com::sun::star::lang::Locale& aLocale ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setConfiguredServices( const ::rtl::OUString& aServiceName, const ::com::sun::star::lang::Locale& aLocale, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aServiceImplNames ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getConfiguredServices( const ::rtl::OUString& aServiceName, const ::com::sun::star::lang::Locale& aLocale ) throw (::com::sun::star::uno::RuntimeException);

    // XAvailableLocales
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale > SAL_CALL getAvailableLocales( const ::rtl::OUString& aServiceName ) throw (::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);


    static inline ::rtl::OUString   getImplementationName_Static();
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static() throw();

    sal_Bool    AddLngSvcEvtBroadcaster(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::linguistic2::XLinguServiceEventBroadcaster > &rxBroadcaster );
    sal_Bool    RemoveLngSvcEvtBroadcaster(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::linguistic2::XLinguServiceEventBroadcaster > &rxBroadcaster );
};


inline ::rtl::OUString LngSvcMgr::getImplementationName_Static()
{
    return A2OU( "com.sun.star.lingu2.LngSvcMgr" );
}


///////////////////////////////////////////////////////////////////////////

#endif

