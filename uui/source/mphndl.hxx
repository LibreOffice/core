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



#ifndef UUI_IAHNDL_HXX
#define UUI_IAHNDL_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XPasswordContainer.hpp>
#include <cppuhelper/weak.hxx>

//============================================================================
class UUIInteractionHandler:
    public cppu::OWeakObject,
    public com::sun::star::lang::XServiceInfo,
    public com::sun::star::lang::XTypeProvider,
    public com::sun::star::task::XInteractionHandler
{
    ::com::sun::star::uno::Reference< ::com::sun::star::task::XPasswordContainer > mPContainer;

public:
    static sal_Char const m_aImplementationName[];

    UUIInteractionHandler( com::sun::star::uno::Reference<
                      com::sun::star::lang::XMultiServiceFactory > const & );

    virtual com::sun::star::uno::Any SAL_CALL
    queryInterface(com::sun::star::uno::Type const & rType)
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL acquire()
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL release()
        throw (com::sun::star::uno::RuntimeException);

    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const &
                                                  rServiceName)
        throw (com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
    getSupportedServiceNames() throw (com::sun::star::uno::RuntimeException);

    virtual
    com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL
    getTypes() throw (com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId() throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
    handle(com::sun::star::uno::Reference<
                   com::sun::star::task::XInteractionRequest > const &
               rRequest)
        throw (com::sun::star::uno::RuntimeException);

    static com::sun::star::uno::Sequence< rtl::OUString >
    getSupportedServiceNames_static();

    static com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
    SAL_CALL
    createInstance(com::sun::star::uno::Reference<
                       com::sun::star::lang::XMultiServiceFactory > const &);
};

#endif // UUI_IAHNDL_HXX

