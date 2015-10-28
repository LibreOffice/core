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
#ifndef INCLUDED_SFX2_SOURCE_APPL_HELPINTERCEPTOR_HXX
#define INCLUDED_SFX2_SOURCE_APPL_HELPINTERCEPTOR_HXX

#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#include <com/sun/star/frame/XInterceptorInfo.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <tools/link.hxx>
#include <vcl/vclptr.hxx>
#include <vector>

struct HelpHistoryEntry_Impl
{
    OUString  aURL;
    css::uno::Any    aViewData;

    HelpHistoryEntry_Impl( const OUString& rURL, const css::uno::Any& rViewData ) :
        aURL( rURL ), aViewData(rViewData) {}
};

typedef ::std::vector< HelpHistoryEntry_Impl* > HelpHistoryList_Impl;

class SfxHelpWindow_Impl;
class HelpInterceptor_Impl : public ::cppu::WeakImplHelper<
        css::frame::XDispatchProviderInterceptor,
        css::frame::XInterceptorInfo,
        css::frame::XDispatch >

{
private:
friend class HelpDispatch_Impl;
friend class SfxHelpWindow_Impl;

    // the component which's dispatches we're intercepting
    css::uno::Reference< css::frame::XDispatchProviderInterception > m_xIntercepted;

    // chaining
    css::uno::Reference< css::frame::XDispatchProvider > m_xSlaveDispatcher;
    css::uno::Reference< css::frame::XDispatchProvider > m_xMasterDispatcher;

    css::uno::Reference< css::frame::XStatusListener > m_xListener;

    HelpHistoryList_Impl*       m_pHistory;
    VclPtr<SfxHelpWindow_Impl>  m_pWindow;
    sal_uIntPtr                 m_nCurPos;
    OUString                    m_aCurrentURL;
    css::uno::Any               m_aViewData;

    void                        addURL( const OUString& rURL );

public:
    HelpInterceptor_Impl();
    virtual ~HelpInterceptor_Impl();

    void                    setInterception( css::uno::Reference< css::frame::XFrame > xFrame );
    OUString                GetCurrentURL() const { return m_aCurrentURL; }



    const css::uno::Any&     GetViewData()const {return m_aViewData;}

    bool                HasHistoryPred() const;     // is there a predecessor for the current in the history
    bool                HasHistorySucc() const;     // is there a successor for the current in the history

    // XDispatchProvider
    virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL
                            queryDispatch( const css::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL
                            queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& aDescripts ) throw(css::uno::RuntimeException, std::exception) override;

    // XDispatchProviderInterceptor
    virtual css::uno::Reference< css::frame::XDispatchProvider > SAL_CALL
                            getSlaveDispatchProvider(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setSlaveDispatchProvider( const css::uno::Reference< css::frame::XDispatchProvider >& xNewSlave ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::frame::XDispatchProvider > SAL_CALL
                            getMasterDispatchProvider(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setMasterDispatchProvider( const css::uno::Reference< css::frame::XDispatchProvider >& xNewMaster ) throw(css::uno::RuntimeException, std::exception) override;

    // XInterceptorInfo
    virtual css::uno::Sequence< OUString > SAL_CALL
                            getInterceptedURLs(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XDispatch
    virtual void SAL_CALL   dispatch( const css::util::URL& aURL, const css::uno::Sequence< css::beans::PropertyValue >& aArgs ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xControl, const css::util::URL& aURL ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xControl, const css::util::URL& aURL ) throw(css::uno::RuntimeException, std::exception) override;

    // extras
    void                    InitWaiter( SfxHelpWindow_Impl* pWindow )
                                { m_pWindow = pWindow; }
    SfxHelpWindow_Impl*     GetHelpWindow() const { return m_pWindow; }
};

// HelpListener_Impl -----------------------------------------------------

class HelpListener_Impl : public ::cppu::WeakImplHelper< css::frame::XStatusListener >
{
private:
    HelpInterceptor_Impl*   pInterceptor;
    Link<HelpListener_Impl&,void> aChangeLink;
    OUString                aFactory;

public:
    explicit HelpListener_Impl( HelpInterceptor_Impl* pInter );

    virtual void SAL_CALL   statusChanged( const css::frame::FeatureStateEvent& Event )
                                throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL   disposing( const css::lang::EventObject& obj )
                                throw( css::uno::RuntimeException, std::exception ) override;

    void                    SetChangeHdl( const Link<HelpListener_Impl&,void>& rLink ) { aChangeLink = rLink; }
    OUString                GetFactory() const { return aFactory; }
};
// HelpStatusListener_Impl -----------------------------------------------------

class HelpStatusListener_Impl : public cppu::WeakImplHelper< css::frame::XStatusListener >
{
private:
    css::uno::Reference < css::frame::XDispatch > xDispatch;
    css::frame::FeatureStateEvent                 aStateEvent;

public:
    HelpStatusListener_Impl(
        css::uno::Reference < css::frame::XDispatch > xDispatch,
        css::util::URL& rURL);
    virtual ~HelpStatusListener_Impl();

    virtual void SAL_CALL   statusChanged( const css::frame::FeatureStateEvent& Event )
                                throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL   disposing( const css::lang::EventObject& obj )
                                throw( css::uno::RuntimeException, std::exception ) override;
    const css::frame::FeatureStateEvent&
                            GetStateEvent() const {return aStateEvent;}
};


#endif // INCLUDED_SFX2_SOURCE_APPL_HELPINTERCEPTOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
