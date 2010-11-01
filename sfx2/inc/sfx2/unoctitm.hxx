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
#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.hxx>

#include <sfx2/sfxuno.hxx>
#include <sfx2/ctrlitem.hxx>
#include <osl/mutex.hxx>

class SfxBindings;
class SfxFrame;
class SfxDispatcher;

class SfxUnoControllerItem :    public ::com::sun::star::frame::XStatusListener ,
                                public ::com::sun::star::lang::XTypeProvider    ,
                                public ::cppu::OWeakObject
{
    ::com::sun::star::util::URL                         aCommand;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >              xDispatch;
    SfxControllerItem*          pCtrlItem;
    SfxBindings*                pBindings;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >              TryGetDispatch( SfxFrame* pFrame );

public:
    SFX_DECL_XINTERFACE_XTYPEPROVIDER


                                SfxUnoControllerItem( SfxControllerItem*, SfxBindings&, const String& );
                                ~SfxUnoControllerItem();

    const ::com::sun::star::util::URL&                  GetCommand() const
                                { return aCommand; }
    void                        Execute();

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
    size_t operator()(const ::rtl::OUString& s) const
        { return s.hashCode(); }
};

typedef ::cppu::OMultiTypeInterfaceContainerHelperVar< ::rtl::OUString, SfxStatusDispatcher_Impl_hashType, std::equal_to< ::rtl::OUString > >   SfxStatusDispatcher_Impl_ListenerContainer ;

class SfxStatusDispatcher   :   public ::com::sun::star::frame::XNotifyingDispatch,
                                public ::com::sun::star::lang::XTypeProvider,
                                public ::cppu::OWeakObject
{
    ::osl::Mutex        aMutex;
    SfxStatusDispatcher_Impl_ListenerContainer  aListeners;

public:
    SFX_DECL_XINTERFACE_XTYPEPROVIDER

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
class SfxOfficeDispatch : public SfxStatusDispatcher
                        , public ::com::sun::star::lang::XUnoTunnel
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

    SFX_DECL_XINTERFACE_XTYPEPROVIDER

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
    static rtl::OUString    GetMasterUnoCommand( const ::com::sun::star::util::URL& aURL );

    void                    SetFrame(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame);

    void                    SetMasterUnoCommand( sal_Bool bSet );
    sal_Bool                IsMasterUnoCommand() const;

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
    using SfxControllerItem::GetCoreMetric;
    SfxMapUnit          GetCoreMetric( SfxItemPool& rPool, sal_uInt16 nSlot );

public:
                        SfxDispatchController_Impl( SfxOfficeDispatch*                 pDisp,
                                                    SfxBindings*                       pBind,
                                                    SfxDispatcher*                     pDispat,
                                                    const SfxSlot*                     pSlot,
                                                    const ::com::sun::star::util::URL& rURL );
                        ~SfxDispatchController_Impl();

    static rtl::OUString getSlaveCommand( const ::com::sun::star::util::URL& rURL );

    void                StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState, SfxSlotServer* pServ );
    virtual void        StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );
    void                setMasterSlaveCommand( sal_Bool bSet );
    sal_Bool            isMasterSlaveCommand() const;
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
