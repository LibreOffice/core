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



#ifndef _SMPLMAILSUPPL_HXX_
#define _SMPLMAILSUPPL_HXX_

#include <cppuhelper/compbase2.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/system/XSimpleMailClientSupplier.hpp>

#ifndef _SIMPLEMAPI_HXX_
#include "simplemapi.hxx"
#endif


class CSmplMailSupplBase
{
protected:
    osl::Mutex  m_aMutex;
};

class CSmplMailSuppl :
    public CSmplMailSupplBase,
    public  cppu::WeakComponentImplHelper2<
            com::sun::star::system::XSimpleMailClientSupplier,
            com::sun::star::lang::XServiceInfo >
{
public:
    CSmplMailSuppl( );
    ~CSmplMailSuppl( );

    // XSimpleMailClientSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::system::XSimpleMailClient > SAL_CALL querySimpleMailClient(  )
        throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
        throw(::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
        throw(::com::sun::star::uno::RuntimeException);
};

#endif
