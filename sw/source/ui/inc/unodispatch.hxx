/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unodispatch.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 10:12:40 $
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
#ifndef _UNODISPATCH_HXX
#define _UNODISPATCH_HXX

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTION_HPP_
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTOR_HPP_
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONCHANGELISTENER_HPP_
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#include <list>
//#ifndef _OSL_MUTEX_HXX_
//#include <osl/mutex.hxx>
//#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

class SwView;
//---------------------------------------------------------------------------------------------------------------------
class SwXDispatchProviderInterceptor : public cppu::WeakImplHelper3
<
    ::com::sun::star::frame::XDispatchProviderInterceptor,
    ::com::sun::star::lang::XEventListener,
    ::com::sun::star::lang::XUnoTunnel
>
{
    class DispatchMutexLock_Impl
    {
        //::osl::MutexGuard   aGuard; #102295# solar mutex has to be used currently
        vos::OGuard         aGuard;
        DispatchMutexLock_Impl();
    public:
        DispatchMutexLock_Impl(SwXDispatchProviderInterceptor&);
        ~DispatchMutexLock_Impl();
    };
    friend class DispatchMutexLock_Impl;

//    ::osl::Mutex                     m_aMutex;#102295# solar mutex has to be used currently

    // the component which's dispatches we're intercepting
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterception>   m_xIntercepted;

    // chaining
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider>           m_xSlaveDispatcher;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider>           m_xMasterDispatcher;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch>                   m_xDispatch;

    SwView* m_pView;

public:
    SwXDispatchProviderInterceptor(SwView& rView);
    ~SwXDispatchProviderInterceptor();

    //XDispatchProvider
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SAL_CALL queryDispatch( const ::com::sun::star::util::URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > > SAL_CALL queryDispatches( const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor >& aDescripts ) throw(::com::sun::star::uno::RuntimeException);

    //XDispatchProviderInterceptor
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > SAL_CALL getSlaveDispatchProvider(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setSlaveDispatchProvider( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xNewDispatchProvider ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > SAL_CALL getMasterDispatchProvider(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setMasterDispatchProvider( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xNewSupplier ) throw(::com::sun::star::uno::RuntimeException);

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    //XUnoTunnel
    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    // view destroyed
    void    Invalidate();
};
//---------------------------------------------------------------------------------------------------------------------
struct StatusStruct_Impl
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener> xListener;
    ::com::sun::star::util::URL                                                 aURL;
};
typedef std::list< StatusStruct_Impl > StatusListenerList;
class SwXDispatch : public cppu::WeakImplHelper2
<
    ::com::sun::star::frame::XDispatch,
    ::com::sun::star::view::XSelectionChangeListener
>
{
    SwView*             m_pView;
    StatusListenerList  m_aListenerList;
    sal_Bool            m_bOldEnable;
    sal_Bool            m_bListenerAdded;
public:
    SwXDispatch(SwView& rView);
    ~SwXDispatch();

    virtual void SAL_CALL dispatch( const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xControl, const ::com::sun::star::util::URL& aURL ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xControl, const ::com::sun::star::util::URL& aURL ) throw(::com::sun::star::uno::RuntimeException);

    //XSelectionChangeListener
    virtual void SAL_CALL selectionChanged( const ::com::sun::star::lang::EventObject& aEvent ) throw(::com::sun::star::uno::RuntimeException);

    //XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    static const sal_Char* GetDBChangeURL();
};

#endif
