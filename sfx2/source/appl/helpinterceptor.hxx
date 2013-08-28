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
#ifndef INCLUDED_SFX_HELPINTERCEPTOR_HXX
#define INCLUDED_SFX_HELPINTERCEPTOR_HXX

#include <cppuhelper/implbase3.hxx>
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#include <com/sun/star/frame/XInterceptorInfo.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <tools/string.hxx>
#include <tools/link.hxx>
#include <vector>

struct HelpHistoryEntry_Impl
{
    OUString  aURL;
    com::sun::star::uno::Any    aViewData;

    HelpHistoryEntry_Impl( const OUString& rURL, const com::sun::star::uno::Any& rViewData ) :
        aURL( rURL ), aViewData(rViewData) {}
};

typedef ::std::vector< HelpHistoryEntry_Impl* > HelpHistoryList_Impl;

class SfxHelpWindow_Impl;
class HelpInterceptor_Impl : public ::cppu::WeakImplHelper3<

        ::com::sun::star::frame::XDispatchProviderInterceptor,
        ::com::sun::star::frame::XInterceptorInfo,
        ::com::sun::star::frame::XDispatch >

{
private:
friend class HelpDispatch_Impl;
friend class SfxHelpWindow_Impl;

    // the component which's dispatches we're intercepting
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterception > m_xIntercepted;

    // chaining
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > m_xSlaveDispatcher;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > m_xMasterDispatcher;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > m_xListener;

    HelpHistoryList_Impl*       m_pHistory;
    SfxHelpWindow_Impl*         m_pWindow;
    sal_uIntPtr                 m_nCurPos;
    OUString                    m_aCurrentURL;
    com::sun::star::uno::Any    m_aViewData;

    void                        addURL( const OUString& rURL );

public:
    HelpInterceptor_Impl();
    ~HelpInterceptor_Impl();

    void                    setInterception( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xFrame );
    OUString                GetCurrentURL() const { return m_aCurrentURL; }



    const com::sun::star::uno::Any&     GetViewData()const {return m_aViewData;}

    sal_Bool                HasHistoryPred() const;     // is there a predecessor for the current in the history
    sal_Bool                HasHistorySucc() const;     // is there a successor for the current in the history

    // XDispatchProvider
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SAL_CALL
                            queryDispatch( const ::com::sun::star::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > > SAL_CALL
                            queryDispatches( const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor >& aDescripts ) throw(::com::sun::star::uno::RuntimeException);

    // XDispatchProviderInterceptor
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > SAL_CALL
                            getSlaveDispatchProvider(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setSlaveDispatchProvider( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xNewSlave ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > SAL_CALL
                            getMasterDispatchProvider(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setMasterDispatchProvider( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xNewMaster ) throw(::com::sun::star::uno::RuntimeException);

    // XInterceptorInfo
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
                            getInterceptedURLs(  ) throw(::com::sun::star::uno::RuntimeException);

    // XDispatch
    virtual void SAL_CALL   dispatch( const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xControl, const ::com::sun::star::util::URL& aURL ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xControl, const ::com::sun::star::util::URL& aURL ) throw(::com::sun::star::uno::RuntimeException);

    // extras
    void                    InitWaiter( SfxHelpWindow_Impl* pWindow )
                                { m_pWindow = pWindow; }
    SfxHelpWindow_Impl*     GetHelpWindow() const { return m_pWindow; }
};

// HelpListener_Impl -----------------------------------------------------

class HelpListener_Impl : public ::cppu::WeakImplHelper1< ::com::sun::star::frame::XStatusListener >
{
private:
    HelpInterceptor_Impl*   pInterceptor;
    Link                    aChangeLink;
    OUString                aFactory;

public:
    HelpListener_Impl( HelpInterceptor_Impl* pInter );

    virtual void SAL_CALL   statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event )
                                throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL   disposing( const ::com::sun::star::lang::EventObject& obj )
                                throw( ::com::sun::star::uno::RuntimeException );

    void                    SetChangeHdl( const Link& rLink ) { aChangeLink = rLink; }
    OUString                GetFactory() const { return aFactory; }
};
// HelpStatusListener_Impl -----------------------------------------------------

class HelpStatusListener_Impl : public
::cppu::WeakImplHelper1< ::com::sun::star::frame::XStatusListener >
{
private:
    ::com::sun::star::uno::Reference < ::com::sun::star::frame::XDispatch > xDispatch;
    ::com::sun::star::frame::FeatureStateEvent                              aStateEvent;

public:
    HelpStatusListener_Impl(
        ::com::sun::star::uno::Reference < ::com::sun::star::frame::XDispatch > xDispatch,
        com::sun::star::util::URL& rURL);
    ~HelpStatusListener_Impl();

    virtual void SAL_CALL   statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event )
                                throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL   disposing( const ::com::sun::star::lang::EventObject& obj )
                                throw( ::com::sun::star::uno::RuntimeException );
    const ::com::sun::star::frame::FeatureStateEvent&
                            GetStateEvent() const {return aStateEvent;}
};


#endif // #ifndef INCLUDED_SFX_HELPINTERCEPTOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
