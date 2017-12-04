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
#ifndef INCLUDED_SFX2_UNOCTITM_HXX
#define INCLUDED_SFX2_UNOCTITM_HXX

#include <memory>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.hxx>

#include <sfx2/ctrlitem.hxx>
#include <osl/mutex.hxx>

class SfxBindings;
class SfxFrame;
class SfxDispatcher;

typedef cppu::OMultiTypeInterfaceContainerHelperVar<OUString>
    SfxStatusDispatcher_Impl_ListenerContainer;

class SfxStatusDispatcher   :   public cppu::WeakImplHelper<css::frame::XNotifyingDispatch>
{
    ::osl::Mutex        aMutex;
    SfxStatusDispatcher_Impl_ListenerContainer  aListeners;

public:

    SfxStatusDispatcher();

    // XDispatch
    virtual void SAL_CALL dispatchWithNotification( const css::util::URL& aURL,
                const css::uno::Sequence< css::beans::PropertyValue >& aArgs,
                const css::uno::Reference< css::frame::XDispatchResultListener >& rListener ) override;
    virtual void SAL_CALL dispatch( const css::util::URL& aURL, const css::uno::Sequence< css::beans::PropertyValue >& aArgs ) override;
    virtual void SAL_CALL addStatusListener(const css::uno::Reference< css::frame::XStatusListener > & xControl, const css::util::URL& aURL) override;
    virtual void SAL_CALL removeStatusListener(const css::uno::Reference< css::frame::XStatusListener > & xControl, const css::util::URL& aURL) override;

    // Something else
    void                ReleaseAll();
    SfxStatusDispatcher_Impl_ListenerContainer& GetListeners()
                        { return aListeners; }
};

class SfxSlotServer;
class SfxDispatchController_Impl;
class SfxOfficeDispatch : public cppu::ImplInheritanceHelper<SfxStatusDispatcher, css::lang::XUnoTunnel>
{
friend class SfxDispatchController_Impl;
    std::unique_ptr<SfxDispatchController_Impl>  pImpl;
public:
                                SfxOfficeDispatch( SfxBindings& rBind,
                                                   SfxDispatcher* pDispat,
                                                   const SfxSlot* pSlot,
                                                   const css::util::URL& rURL );
                                SfxOfficeDispatch( SfxDispatcher* pDispat,
                                                   const SfxSlot* pSlot,
                                                   const css::util::URL& rURL );
                                virtual ~SfxOfficeDispatch() override;

    virtual void SAL_CALL       dispatchWithNotification( const css::util::URL& aURL,
                                                          const css::uno::Sequence< css::beans::PropertyValue >& aArgs,
                                                          const css::uno::Reference< css::frame::XDispatchResultListener >& rListener ) override;
    virtual void   SAL_CALL     dispatch( const css::util::URL& aURL,
                                          const css::uno::Sequence< css::beans::PropertyValue >& aArgs ) override;
    virtual void   SAL_CALL     addStatusListener( const css::uno::Reference< css::frame::XStatusListener > & xControl,
                                                   const css::util::URL& aURL) override;

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override ;
    static const css::uno::Sequence< sal_Int8 >& impl_getStaticIdentifier();

    static bool             IsMasterUnoCommand( const css::util::URL& aURL );
    static OUString         GetMasterUnoCommand( const css::util::URL& aURL );

    void                    SetFrame(const css::uno::Reference< css::frame::XFrame >& xFrame);

    void                    SetMasterUnoCommand( bool bSet );

    SfxDispatcher*          GetDispatcher_Impl();
};

class SfxDispatchController_Impl : public SfxControllerItem
{
    css::util::URL              aDispatchURL;
    SfxDispatcher*              pDispatcher;
    SfxBindings*                pBindings;
    const SfxPoolItem*          pLastState;
    sal_uInt16                  nSlot;
    SfxOfficeDispatch*          pDispatch;
    bool                        bMasterSlave;
    bool                        bVisible;
    const char*                 pUnoName;
    css::uno::WeakReference< css::frame::XFrame > xFrame;

    static void         addParametersToArgs( const css::util::URL& aURL,
                                             css::uno::Sequence< css::beans::PropertyValue >& rArgs );
    static MapUnit      GetCoreMetric( SfxItemPool const & rPool, sal_uInt16 nSlot );

    void                sendStatusChanged(const OUString& rURL, const css::frame::FeatureStateEvent& rEvent);

public:
                        SfxDispatchController_Impl( SfxOfficeDispatch*                 pDisp,
                                                    SfxBindings*                       pBind,
                                                    SfxDispatcher*                     pDispat,
                                                    const SfxSlot*                     pSlot,
                                                    const css::util::URL& rURL );
                        virtual ~SfxDispatchController_Impl() override;

    static OUString getSlaveCommand( const css::util::URL& rURL );

    void                StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState, SfxSlotServer const * pServ );
    virtual void        StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState ) override;
    void                setMasterSlaveCommand( bool bSet );
    /// @throws css::uno::RuntimeException
    void       dispatch( const css::util::URL& aURL,
                                  const css::uno::Sequence< css::beans::PropertyValue >& aArgs,
                                  const css::uno::Reference< css::frame::XDispatchResultListener >& rListener );
    /// @throws css::uno::RuntimeException
    void       addStatusListener(const css::uno::Reference< css::frame::XStatusListener > & xControl, const css::util::URL& aURL);
    void                UnBindController();
    SfxDispatcher*      GetDispatcher();
    void                SetFrame(const css::uno::Reference< css::frame::XFrame >& xFrame);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
