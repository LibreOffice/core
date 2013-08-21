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
#ifndef _SFX_UNOCTITM_HXX
#define _SFX_UNOCTITM_HXX

#include <functional>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/interfacecontainer.hxx>

#include <sfx2/ctrlitem.hxx>
#include <osl/mutex.hxx>

class SfxBindings;
class SfxFrame;
class SfxDispatcher;

class SfxUnoControllerItem :    public ::cppu::WeakImplHelper1< css::frame::XStatusListener >
{
    ::com::sun::star::util::URL                         aCommand;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >              xDispatch;
    SfxControllerItem*          pCtrlItem;
    SfxBindings*                pBindings;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >              TryGetDispatch( SfxFrame* pFrame );

public:

                                SfxUnoControllerItem( SfxControllerItem*, SfxBindings&, const OUString& );
                                ~SfxUnoControllerItem();

    const ::com::sun::star::util::URL&                  GetCommand() const
                                { return aCommand; }

    // XStatusListener
    virtual void SAL_CALL statusChanged(const ::com::sun::star::frame::FeatureStateEvent& Event) throw( ::com::sun::star::uno::RuntimeException );

    // Something else
    virtual void    SAL_CALL            disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException );
    void                        UnBind();
    void                        GetNewDispatch();
    void                        ReleaseDispatch();
    void                        ReleaseBindings();
};

struct SfxStatusDispatcher_Impl_hashType
{
    size_t operator()(const OUString& s) const
        { return s.hashCode(); }
};

typedef ::cppu::OMultiTypeInterfaceContainerHelperVar< OUString, SfxStatusDispatcher_Impl_hashType, std::equal_to< OUString > >   SfxStatusDispatcher_Impl_ListenerContainer ;

class SfxStatusDispatcher   :   public ::cppu::WeakImplHelper1< css::frame::XNotifyingDispatch >
{
    ::osl::Mutex        aMutex;
    SfxStatusDispatcher_Impl_ListenerContainer  aListeners;

public:

    SfxStatusDispatcher();

    // XDispatch
    virtual void SAL_CALL dispatchWithNotification( const ::com::sun::star::util::URL& aURL,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs,
                const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchResultListener >& rListener ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL dispatch( const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xControl, const ::com::sun::star::util::URL& aURL) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xControl, const ::com::sun::star::util::URL& aURL) throw( ::com::sun::star::uno::RuntimeException );

    // Something else
    void                ReleaseAll();
    SfxStatusDispatcher_Impl_ListenerContainer& GetListeners()
                        { return aListeners; }
};

class SfxSlotServer;
class SfxDispatchController_Impl;
class SfxOfficeDispatch : public ::cppu::ImplInheritanceHelper1< SfxStatusDispatcher, css::lang::XUnoTunnel >
{
friend class SfxDispatchController_Impl;
    SfxDispatchController_Impl*  pControllerItem;
public:
                                SfxOfficeDispatch( SfxBindings& rBind,
                                                   SfxDispatcher* pDispat,
                                                   const SfxSlot* pSlot,
                                                   const ::com::sun::star::util::URL& rURL );
                                SfxOfficeDispatch( SfxDispatcher* pDispat,
                                                   const SfxSlot* pSlot,
                                                   const ::com::sun::star::util::URL& rURL );
                                ~SfxOfficeDispatch();

    virtual void SAL_CALL       dispatchWithNotification( const ::com::sun::star::util::URL& aURL,
                                                          const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs,
                                                          const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchResultListener >& rListener )
                                throw( ::com::sun::star::uno::RuntimeException );
    virtual void   SAL_CALL     dispatch( const ::com::sun::star::util::URL& aURL,
                                          const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs )
                                throw( ::com::sun::star::uno::RuntimeException );
    virtual void   SAL_CALL     addStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xControl,
                                                   const ::com::sun::star::util::URL& aURL)
                                throw( ::com::sun::star::uno::RuntimeException );

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException) ;
    static const ::com::sun::star::uno::Sequence< sal_Int8 >& impl_getStaticIdentifier();

    static sal_Bool         IsMasterUnoCommand( const ::com::sun::star::util::URL& aURL );
    static OUString    GetMasterUnoCommand( const ::com::sun::star::util::URL& aURL );

    void                    SetFrame(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame);

    void                    SetMasterUnoCommand( sal_Bool bSet );

    SfxDispatcher*          GetDispatcher_Impl();
};

class SfxDispatchController_Impl : public SfxControllerItem
{
    ::com::sun::star::util::URL aDispatchURL;
    SfxDispatcher*              pDispatcher;
    SfxBindings*                pBindings;
    const SfxPoolItem*          pLastState;
    sal_uInt16                  nSlot;
    SfxOfficeDispatch*          pDispatch;
    sal_Bool                    bMasterSlave;
    sal_Bool                    bVisible;
    const char*                 pUnoName;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XFrame > xFrame;

    void                addParametersToArgs( const com::sun::star::util::URL& aURL,
                                             ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rArgs ) const;
    SfxMapUnit          GetCoreMetric( SfxItemPool& rPool, sal_uInt16 nSlot );

public:
                        SfxDispatchController_Impl( SfxOfficeDispatch*                 pDisp,
                                                    SfxBindings*                       pBind,
                                                    SfxDispatcher*                     pDispat,
                                                    const SfxSlot*                     pSlot,
                                                    const ::com::sun::star::util::URL& rURL );
                        ~SfxDispatchController_Impl();

    static OUString getSlaveCommand( const ::com::sun::star::util::URL& rURL );

    void                StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState, SfxSlotServer* pServ );
    virtual void        StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );
    void                setMasterSlaveCommand( sal_Bool bSet );
    void SAL_CALL       dispatch( const ::com::sun::star::util::URL& aURL,
                                  const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs,
                                  const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchResultListener >& rListener ) throw( ::com::sun::star::uno::RuntimeException );
    void SAL_CALL       addStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xControl, const ::com::sun::star::util::URL& aURL) throw( ::com::sun::star::uno::RuntimeException );
    void                UnBindController();
    SfxDispatcher*      GetDispatcher();
    void                    SetFrame(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
