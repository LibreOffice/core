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
    String  aURL;
    com::sun::star::uno::Any    aViewData;

    HelpHistoryEntry_Impl( const String& rURL, const com::sun::star::uno::Any& rViewData ) :
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
    sal_uIntPtr                     m_nCurPos;
    String                      m_aCurrentURL;
    com::sun::star::uno::Any    m_aViewData;

    void                        addURL( const String& rURL );

public:
    HelpInterceptor_Impl();
    ~HelpInterceptor_Impl();

    void                    setInterception( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xFrame );
    void                    SetStartURL( const String& rURL );
    String                  GetCurrentURL() const { return m_aCurrentURL; }



    const com::sun::star::uno::Any&     GetViewData()const {return m_aViewData;}

    sal_Bool                HasHistoryPred() const;     // is there a predecessor for the current in the history
    sal_Bool                HasHistorySucc() const;     // is there a successor for the current in the history

    // XDispatchProvider
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SAL_CALL
                            queryDispatch( const ::com::sun::star::util::URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags ) throw(::com::sun::star::uno::RuntimeException);
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
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
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
    String                  aFactory;

public:
    HelpListener_Impl( HelpInterceptor_Impl* pInter );

    virtual void SAL_CALL   statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event )
                                throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL   disposing( const ::com::sun::star::lang::EventObject& obj )
                                throw( ::com::sun::star::uno::RuntimeException );

    void                    SetChangeHdl( const Link& rLink ) { aChangeLink = rLink; }
    String                  GetFactory() const { return aFactory; }
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
