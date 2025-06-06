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

#pragma once

#include <cppuhelper/implbase.hxx>
#include <comphelper/interfacecontainer3.hxx>

#include <comphelper/lok.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/linguistic2/XLinguServiceManager2.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <unotools/configitem.hxx>
#include <rtl/ref.hxx>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
#include <vector>
#include <optional>

class SpellCheckerDispatcher;
class HyphenatorDispatcher;
class ThesaurusDispatcher;
class GrammarCheckingIterator;
class LngSvcMgrListenerHelper;
struct SvcInfo;

namespace com::sun::star::linguistic2 {
    class XLinguServiceEventBroadcaster;
}


class LngSvcMgr :
    public cppu::WeakImplHelper
    <
        css::linguistic2::XLinguServiceManager2,
        css::lang::XServiceInfo,
        css::util::XModifyListener
    >,
    private utl::ConfigItem,
    public comphelper::LibreOfficeKit::ThreadJoinable
{
    friend class LngSvcMgrListenerHelper;

    ::comphelper::OInterfaceContainerHelper3<css::lang::XEventListener>  aEvtListeners;

    css::uno::Reference<
        css::util::XModifyBroadcaster>                  xMB;

    Idle                                                aUpdateIdle;


    css::uno::Sequence<
        css::lang::Locale >                             aAvailSpellLocales;
    css::uno::Sequence<
        css::lang::Locale >                             aAvailGrammarLocales;
    css::uno::Sequence<
        css::lang::Locale >                             aAvailHyphLocales;
    css::uno::Sequence<
        css::lang::Locale >                             aAvailThesLocales;

    rtl::Reference<SpellCheckerDispatcher>              mxSpellDsp;
    rtl::Reference<GrammarCheckingIterator>             mxGrammarDsp;
    rtl::Reference<HyphenatorDispatcher>                mxHyphDsp;
    rtl::Reference<ThesaurusDispatcher>                 mxThesDsp;

    rtl::Reference<LngSvcMgrListenerHelper>             mxListenerHelper;

    typedef std::vector< SvcInfo > SvcInfoArray;
    std::optional<SvcInfoArray>                        pAvailSpellSvcs;
    std::optional<SvcInfoArray>                        pAvailGrammarSvcs;
    std::optional<SvcInfoArray>                        pAvailHyphSvcs;
    std::optional<SvcInfoArray>                        pAvailThesSvcs;

    bool bDisposing;

    LngSvcMgr(const LngSvcMgr &) = delete;
    LngSvcMgr & operator = (const LngSvcMgr &) = delete;

    void    GetAvailableSpellSvcs_Impl();
    void    GetAvailableGrammarSvcs_Impl();
    void    GetAvailableHyphSvcs_Impl();
    void    GetAvailableThesSvcs_Impl();
    void    GetListenerHelper_Impl();

    void    GetSpellCheckerDsp_Impl( bool bSetSvcList = true );
    void    GetGrammarCheckerDsp_Impl( bool bSetSvcList = true );
    void    GetHyphenatorDsp_Impl( bool bSetSvcList = true );
    void    GetThesaurusDsp_Impl( bool bSetSvcList = true );

    void    SetCfgServiceLists( SpellCheckerDispatcher &rSpellDsp );
    void    SetCfgServiceLists( GrammarCheckingIterator &rGrammarDsp );
    void    SetCfgServiceLists( HyphenatorDispatcher &rHyphDsp );
    void    SetCfgServiceLists( ThesaurusDispatcher &rThesDsp );

    bool    SaveCfgSvcs( std::u16string_view rServiceName );

    // utl::ConfigItem (to allow for listening of changes of relevant properties)
    virtual void    Notify( const css::uno::Sequence< OUString > &rPropertyNames ) override;
    virtual void    ImplCommit() override;

    void UpdateAll();
    void stopListening();
    DECL_LINK( updateAndBroadcast, Timer*, void );

public:
    LngSvcMgr();
    virtual ~LngSvcMgr() override;

    // XLinguServiceManager
    virtual css::uno::Reference< css::linguistic2::XSpellChecker > SAL_CALL getSpellChecker(  ) override;
    virtual css::uno::Reference< css::linguistic2::XHyphenator > SAL_CALL getHyphenator(  ) override;
    virtual css::uno::Reference< css::linguistic2::XThesaurus > SAL_CALL getThesaurus(  ) override;
    virtual sal_Bool SAL_CALL addLinguServiceManagerListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual sal_Bool SAL_CALL removeLinguServiceManagerListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getAvailableServices( const OUString& aServiceName, const css::lang::Locale& aLocale ) override;
    virtual void SAL_CALL setConfiguredServices( const OUString& aServiceName, const css::lang::Locale& aLocale, const css::uno::Sequence< OUString >& aServiceImplNames ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getConfiguredServices( const OUString& aServiceName, const css::lang::Locale& aLocale ) override;

    // XAvailableLocales
    virtual css::uno::Sequence< css::lang::Locale > SAL_CALL getAvailableLocales( const OUString& aServiceName ) override;

    // XComponent
    virtual void SAL_CALL dispose(  ) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& rSource ) override;

    // XModifyListener
    virtual void SAL_CALL modified( const css::lang::EventObject& rEvent ) override;

    // comphelper::LibreOfficeKit::ThreadJoinable
    virtual bool joinThreads() override;

    bool    AddLngSvcEvtBroadcaster(
                const css::uno::Reference< css::linguistic2::XLinguServiceEventBroadcaster > &rxBroadcaster );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
