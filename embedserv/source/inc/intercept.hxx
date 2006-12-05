/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: intercept.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-05 12:53:56 $
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

#ifndef _INTERCEPT_HXX_
#define _INTERCEPT_HXX_

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef  _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTOR_HPP_
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XINTERCEPTORINFO_HPP_
#include <com/sun/star/frame/XInterceptorInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif

#include <rtl/ref.hxx>
#include "embeddocaccess.hxx"


class StatusChangeListenerContainer;
class EmbedDocument_Impl;
class DocumentHolder;

class Interceptor
    : public ::cppu::WeakImplHelper3<
                   ::com::sun::star::frame::XDispatchProviderInterceptor,
                   ::com::sun::star::frame::XInterceptorInfo,
                   ::com::sun::star::frame::XDispatch>
{
public:

    Interceptor(
        const ::rtl::Reference< EmbeddedDocumentInstanceAccess_Impl >& xOleAccess,
        DocumentHolder* pDocH,
        sal_Bool bLink );

    ~Interceptor();

    void DisconnectDocHolder();

    void generateFeatureStateEvent();

    // overwritten to release the statuslistner.


    // XComponent
    virtual void SAL_CALL
    addEventListener(
        const com::sun::star::uno::Reference< com::sun::star::lang::XEventListener >& xListener )
        throw( com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
    removeEventListener( const com::sun::star::uno::Reference< com::sun::star::lang::XEventListener >& aListener )
        throw( com::sun::star::uno::RuntimeException );

    void SAL_CALL
    dispose() throw(::com::sun::star::uno::RuntimeException);



    //XDispatch
    virtual void SAL_CALL
    dispatch(
        const ::com::sun::star::util::URL& URL,
        const ::com::sun::star::uno::Sequence<
        ::com::sun::star::beans::PropertyValue >& Arguments )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
    addStatusListener(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XStatusListener >& Control,
        const ::com::sun::star::util::URL& URL )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    virtual void SAL_CALL
    removeStatusListener(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XStatusListener >& Control,
        const ::com::sun::star::util::URL& URL )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    //XInterceptorInfo
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >
    SAL_CALL getInterceptedURLs(  )
        throw (
            ::com::sun::star::uno::RuntimeException
        );


    //XDispatchProvider ( inherited by XDispatchProviderInterceptor )
    virtual ::com::sun::star::uno::Reference<
    ::com::sun::star::frame::XDispatch > SAL_CALL
    queryDispatch(
        const ::com::sun::star::util::URL& URL,
        const ::rtl::OUString& TargetFrameName,
        sal_Int32 SearchFlags )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    virtual ::com::sun::star::uno::Sequence<
    ::com::sun::star::uno::Reference<
    ::com::sun::star::frame::XDispatch > > SAL_CALL
    queryDispatches(
        const ::com::sun::star::uno::Sequence<
        ::com::sun::star::frame::DispatchDescriptor >& Requests )
        throw (
            ::com::sun::star::uno::RuntimeException
        );


    //XDispatchProviderInterceptor
    virtual ::com::sun::star::uno::Reference<
    ::com::sun::star::frame::XDispatchProvider > SAL_CALL
    getSlaveDispatchProvider(  )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    virtual void SAL_CALL
    setSlaveDispatchProvider(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XDispatchProvider >& NewDispatchProvider )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    virtual ::com::sun::star::uno::Reference<
    ::com::sun::star::frame::XDispatchProvider > SAL_CALL
    getMasterDispatchProvider(  )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    virtual void SAL_CALL
    setMasterDispatchProvider(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XDispatchProvider >& NewSupplier )
        throw (
            ::com::sun::star::uno::RuntimeException
        );


private:

    osl::Mutex   m_aMutex;

    ::rtl::Reference< EmbeddedDocumentInstanceAccess_Impl > m_xOleAccess;

    ::com::sun::star::uno::WeakReference< ::com::sun::star::uno::XInterface > m_xDocHLocker;
    DocumentHolder*       m_pDocH;

    ::com::sun::star::uno::Reference<
    ::com::sun::star::frame::XDispatchProvider > m_xSlaveDispatchProvider;

    ::com::sun::star::uno::Reference<
    ::com::sun::star::frame::XDispatchProvider > m_xMasterDispatchProvider;

    static ::com::sun::star::uno::Sequence< ::rtl::OUString >
    m_aInterceptedURL;

    cppu::OInterfaceContainerHelper*    m_pDisposeEventListeners;
    StatusChangeListenerContainer*    m_pStatCL;

    sal_Bool m_bLink;
};

#endif
