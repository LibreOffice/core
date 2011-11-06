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



#ifndef SVTOOLS_PRODUCTREGISTRATION_HXX
#define SVTOOLS_PRODUCTREGISTRATION_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/task/XJobExecutor.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <cppuhelper/implbase3.hxx>

//........................................................................
namespace svt
{
//........................................................................

    //====================================================================
    //= OProductRegistration
    //====================================================================
    typedef ::cppu::WeakImplHelper3 <   ::com::sun::star::lang::XServiceInfo
                                    ,   ::com::sun::star::task::XJobExecutor
                                    ,   ::com::sun::star::task::XJob
                                    >   OProductRegistration_Base;

    class OProductRegistration : public OProductRegistration_Base
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                    m_xORB;

    protected:
        OProductRegistration( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB );

    public:
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                    Create( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB );

        // XServiceInfo - static version
        static ::rtl::OUString SAL_CALL getImplementationName_Static( );
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_Static(  ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

        // XJobExecutor
        virtual void SAL_CALL trigger( const ::rtl::OUString& sEvent ) throw (::com::sun::star::uno::RuntimeException);

        // XJob
        virtual ::com::sun::star::uno::Any SAL_CALL execute( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& _rArgs ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    private:
        // types of events which can be handled by this component
        enum EventType
        {
            etRegistrationRequired,

            etUnknown
        };

        // classifies a event
        EventType classify( const ::rtl::OUString& _rEventDesc );

        // do the online registration
        void doOnlineRegistration( );
    };

//........................................................................
}   // namespace svt
//........................................................................

#endif // SVTOOLS_PRODUCTREGISTRATION_HXX

