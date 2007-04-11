/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoctitm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:31:30 $
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
#ifndef _SFX_UNOCTITM_HXX
#define _SFX_UNOCTITM_HXX

#include <functional>

#ifndef _COM_SUN_STAR_FRAME_XNOTIFYINGDISPATCH_HPP_
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHRESULTLISTENER_HPP_
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTOR_HPP_
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTATUSLISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTION_HPP_
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FEATURESTATEEVENT_HPP_
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_DISPATCHDESCRIPTOR_HPP_
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

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

//#if 0 // _SOLAR__PRIVATE
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
//#endif

#endif

