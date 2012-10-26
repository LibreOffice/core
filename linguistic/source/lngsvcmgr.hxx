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

#ifndef _LINGUISTIC_LNGSVCMGR_HXX_
#define _LINGUISTIC_LNGSVCMGR_HXX_

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase3.hxx> // helper for implementations
#include <cppuhelper/interfacecontainer.h>  //OMultiTypeInterfaceContainerHelper


#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/linguistic2/XLinguServiceManager2.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <unotools/configitem.hxx>
#include <vcl/timer.hxx>
#include <boost/ptr_container/ptr_vector.hpp>

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
    class XProofreadingIterator;
    class XHyphenator;
    class XThesaurus;
} } } }



class LngSvcMgr :
    public cppu::WeakImplHelper3
    <
        com::sun::star::linguistic2::XLinguServiceManager2,
        com::sun::star::lang::XServiceInfo,
        com::sun::star::util::XModifyListener
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

    com::sun::star::uno::Reference<
        ::com::sun::star::util::XModifyBroadcaster>     xMB;

    Timer                                               aUpdateTimer;


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

    typedef boost::ptr_vector< SvcInfo >    SvcInfoArray;
    SvcInfoArray *                                      pAvailSpellSvcs;
    SvcInfoArray *                                      pAvailGrammarSvcs;
    SvcInfoArray *                                      pAvailHyphSvcs;
    SvcInfoArray *                                      pAvailThesSvcs;

    sal_Bool bDisposing;

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

    static void clearSvcInfoArray(SvcInfoArray *&rpInfo);

    // utl::ConfigItem (to allow for listening of changes of relevant properties)
    virtual void    Notify( const com::sun::star::uno::Sequence< rtl::OUString > &rPropertyNames );
    virtual void    Commit();

    void UpdateAll();
    void stopListening();
    DECL_LINK( updateAndBroadcast, void* );

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

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& rSource ) throw(::com::sun::star::uno::RuntimeException);

    // XModifyListener
    virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& rEvent ) throw(::com::sun::star::uno::RuntimeException);

    static inline ::rtl::OUString   getImplementationName_Static();
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static() throw();

    sal_Bool    AddLngSvcEvtBroadcaster(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::linguistic2::XLinguServiceEventBroadcaster > &rxBroadcaster );
};


inline ::rtl::OUString LngSvcMgr::getImplementationName_Static()
{
    return A2OU( "com.sun.star.lingu2.LngSvcMgr" );
}



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
