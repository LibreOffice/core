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



#ifndef __OWNVIEW_HXX_
#define __OWNVIEW_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XCloseListener.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <cppuhelper/implbase2.hxx>

#include <osl/mutex.hxx>

class OwnView_Impl : public ::cppu::WeakImplHelper2 < ::com::sun::star::util::XCloseListener,
                                                      ::com::sun::star::document::XEventListener >
{
    ::osl::Mutex m_aMutex;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > m_xModel;

    ::rtl::OUString m_aTempFileURL;
    ::rtl::OUString m_aNativeTempURL;

    ::rtl::OUString m_aFilterName;

    sal_Bool m_bBusy;

    sal_Bool m_bUseNative;

private:
    sal_Bool CreateModelFromURL( const ::rtl::OUString& aFileURL );

    sal_Bool CreateModel( sal_Bool bUseNative );

    sal_Bool ReadContentsAndGenerateTempFile( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xStream, sal_Bool bParseHeader );

    void CreateNative();

public:
    static ::rtl::OUString GetFilterNameFromExtentionAndInStream(
                                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory,
                                const ::rtl::OUString& aNameWithExtention,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInputStream );

    OwnView_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory,
                  const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xStream );
    virtual ~OwnView_Impl();

    sal_Bool Open();

    void Close();

    virtual void SAL_CALL notifyEvent( const ::com::sun::star::document::EventObject& Event ) throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL queryClosing( const ::com::sun::star::lang::EventObject& Source, sal_Bool GetsOwnership ) throw (::com::sun::star::util::CloseVetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL notifyClosing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
};

#endif

