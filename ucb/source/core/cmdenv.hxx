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



#ifndef INCLUDED_CMDENV_HXX
#define INCLUDED_CMDENV_HXX

#include "cppuhelper/implbase3.hxx"

#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/lang/XSingleServiceFactory.hpp"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"

namespace ucb_cmdenv {

class UcbCommandEnvironment :
        public cppu::WeakImplHelper3< com::sun::star::lang::XInitialization,
                                      com::sun::star::lang::XServiceInfo,
                                      com::sun::star::ucb::XCommandEnvironment >
{
    com::sun::star::uno::Reference<
        com::sun::star::task::XInteractionHandler > m_xIH;
    com::sun::star::uno::Reference<
        com::sun::star::ucb::XProgressHandler >     m_xPH;

public:
    UcbCommandEnvironment(
        const com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory >& rXSMgr );
    virtual ~UcbCommandEnvironment();

    // XInitialization
    virtual void SAL_CALL
    initialize( const com::sun::star::uno::Sequence<
                        com::sun::star::uno::Any >& aArguments )
        throw( com::sun::star::uno::Exception,
               com::sun::star::uno::RuntimeException );

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw ( com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL
    supportsService( const ::rtl::OUString& ServiceName )
        throw ( com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
    getSupportedServiceNames()
        throw ( com::sun::star::uno::RuntimeException );

    // XCommandEnvironment
    virtual com::sun::star::uno::Reference<
        com::sun::star::task::XInteractionHandler > SAL_CALL
    getInteractionHandler()
        throw ( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Reference<
        com::sun::star::ucb::XProgressHandler > SAL_CALL
    getProgressHandler()
        throw ( com::sun::star::uno::RuntimeException );

    // Non-UNO interfaces
    static rtl::OUString
    getImplementationName_Static();
    static com::sun::star::uno::Sequence< rtl::OUString >
    getSupportedServiceNames_Static();

    static com::sun::star::uno::Reference<
            com::sun::star::lang::XSingleServiceFactory >
    createServiceFactory( const com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory > & rxServiceMgr );
private:
    //com::sun::star::uno::Reference<
    //    com::sun::star::lang::XMultiServiceFactory > m_xSMgr;
};

} // namespace ucb_cmdenv

#endif // INCLUDED_CMDENV_HXX
