/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef DBA_INTERCEPT_HXX
#define DBA_INTERCEPT_HXX

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
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
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTLISTENER_HPP_
#include <com/sun/star/document/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _DBA_COREDATAACCESS_DOCUMENTDEFINITION_HXX_
#include "documentdefinition.hxx"
#endif
#include <vcl/svapp.hxx>

namespace dbaccess
{


class OInterceptor : public ::cppu::WeakImplHelper4< ::com::sun::star::frame::XDispatchProviderInterceptor,
                                                       ::com::sun::star::frame::XInterceptorInfo,
                                                       ::com::sun::star::frame::XDispatch,
                                                    ::com::sun::star::document::XEventListener>
{
    DECL_LINK( OnDispatch, void* _aURL  );
protected:
    virtual ~OInterceptor();
public:

    OInterceptor( ODocumentDefinition* _pContentHolder,sal_Bool _bAllowEditDoc );

    void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

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

    // XEventListener
    virtual void SAL_CALL notifyEvent( const ::com::sun::star::document::EventObject& Event ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);


private:

    osl::Mutex   m_aMutex;

    ODocumentDefinition*   m_pContentHolder;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > m_xSlaveDispatchProvider;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > m_xMasterDispatchProvider;

    ::com::sun::star::uno::Sequence< ::rtl::OUString > m_aInterceptedURL;

    cppu::OInterfaceContainerHelper*    m_pDisposeEventListeners;
    PropertyChangeListenerContainer*    m_pStatCL;
    sal_Bool                            m_bAllowEditDoc;
};


//........................................................................
}   // namespace dbaccess
//........................................................................


#endif //DBA_INTERCEPT_HXX


