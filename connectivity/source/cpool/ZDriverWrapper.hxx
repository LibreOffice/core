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



#ifndef _CONNECTIVITY_CPOOL_ZDRIVERWRAPPER_HXX_
#define _CONNECTIVITY_CPOOL_ZDRIVERWRAPPER_HXX_

#include <com/sun/star/sdbc/XDriver.hpp>
#include <cppuhelper/implbase1.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/uno/XAggregation.hpp>

//........................................................................
namespace connectivity
{
//........................................................................

    class OConnectionPool;
    //====================================================================
    //= ODriverWrapper
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::sdbc::XDriver
                                    >   ODriverWrapper_BASE;

    class ODriverWrapper : public ODriverWrapper_BASE
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >
                            m_xDriverAggregate;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver >
                            m_xDriver;
        OConnectionPool*    m_pConnectionPool;

    public:
        /** creates a new wrapper for a driver
            @param _rxAggregateDriver
                the driver to aggregate. The object will be reset to <NULL/> when returning from the ctor.
        */
        ODriverWrapper(
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >& _rxAggregateDriver,
            OConnectionPool* _pPool
            );



        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        /// dtor
        virtual ~ODriverWrapper();
        // XDriver
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL connect( const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL acceptsURL( const ::rtl::OUString& url ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sdbc::DriverPropertyInfo > SAL_CALL getPropertyInfo( const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getMajorVersion(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getMinorVersion(  ) throw (::com::sun::star::uno::RuntimeException);
    };

//........................................................................
}   // namespace connectivity
//........................................................................

#endif // _CONNECTIVITY_CPOOL_ZDRIVERWRAPPER_HXX_


