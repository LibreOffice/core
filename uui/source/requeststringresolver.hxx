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



#ifndef UUI_REQUESTSTRINGRESOLVER_HXX
#define UUI_REQUESTSTRINGRESOLVER_HXX

#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/task/XInteractionRequestStringResolver.hpp"
#include "cppuhelper/implbase2.hxx"

class UUIInteractionHelper;

class UUIInteractionRequestStringResolver:
    public cppu::WeakImplHelper2<
        com::sun::star::lang::XServiceInfo,
        com::sun::star::task::XInteractionRequestStringResolver >
{
public:
    static char const m_aImplementationName[];

    static com::sun::star::uno::Sequence< rtl::OUString >
    getSupportedServiceNames_static();

    static com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
    SAL_CALL
    createInstance(
        com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory > const &
    rServiceFactory)
        SAL_THROW((com::sun::star::uno::Exception));

private:
    com::sun::star::uno::Reference<
    com::sun::star::lang::XMultiServiceFactory > m_xServiceFactory;
    UUIInteractionHelper * m_pImpl;

    UUIInteractionRequestStringResolver(UUIInteractionRequestStringResolver &); // not implemented
    void operator =(UUIInteractionRequestStringResolver); // not implemented

    UUIInteractionRequestStringResolver(
        com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory >
                const & rServiceFactory)
        SAL_THROW(());

    virtual ~UUIInteractionRequestStringResolver() SAL_THROW(());

    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const &
                          rServiceName)
        throw (com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
    getSupportedServiceNames()
        throw (com::sun::star::uno::RuntimeException);

    virtual com::sun::star::beans::Optional< rtl::OUString > SAL_CALL
    getStringFromInformationalRequest(
        const com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionRequest >& Request )
        throw (com::sun::star::uno::RuntimeException);
};

#endif // UUI_REQUESTSTRINGRESOLVER_HXX
