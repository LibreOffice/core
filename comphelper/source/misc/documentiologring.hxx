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



#ifndef __DOCUMENTIOLOGRING_HXX_
#define __DOCUMENTIOLOGRING_HXX_

#include <com/sun/star/logging/XSimpleLogRing.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

#include <osl/mutex.hxx>
#include <cppuhelper/implbase3.hxx>

#define SIMPLELOGRING_SIZE 256

namespace comphelper
{

class OSimpleLogRing : public ::cppu::WeakImplHelper3< ::com::sun::star::logging::XSimpleLogRing,
                                                           ::com::sun::star::lang::XInitialization,
                                                           ::com::sun::star::lang::XServiceInfo >
{
    ::osl::Mutex m_aMutex;
    ::com::sun::star::uno::Sequence< ::rtl::OUString > m_aMessages;

    sal_Bool m_bInitialized;
    sal_Bool m_bFull;
    sal_Int32 m_nPos;

public:
    OSimpleLogRing( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext );
    virtual ~OSimpleLogRing();

    static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
            getSupportedServiceNames_static();

    static ::rtl::OUString SAL_CALL getImplementationName_static();

    static ::rtl::OUString SAL_CALL getSingletonName_static();

    static ::rtl::OUString SAL_CALL getServiceName_static();

    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        Create( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );

// XSimpleLogRing
    virtual void SAL_CALL logString( const ::rtl::OUString& aMessage ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getCollectedLog() throw (::com::sun::star::uno::RuntimeException);

// XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

// XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

};

} // namespace comphelper

#endif

