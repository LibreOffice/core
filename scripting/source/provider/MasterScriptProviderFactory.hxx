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


#include "rtl/ustring.hxx"
#include "rtl/ref.hxx"
#include <cppuhelper/implbase2.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

#include <com/sun/star/script/provider/XScriptProviderFactory.hpp>
#include <com/sun/star/script/provider/XScriptProvider.hpp>

#include "ActiveMSPList.hxx"

namespace func_provider
{
// for simplification
#define css ::com::sun::star

class MasterScriptProviderFactory :
    public ::cppu::WeakImplHelper2 <
        css::script::provider::XScriptProviderFactory,
        css::lang::XServiceInfo >
{
private:

    mutable rtl::Reference< ActiveMSPList > m_MSPList;

    const css::uno::Reference< css::uno::XComponentContext > m_xComponentContext;

    const rtl::Reference< ActiveMSPList > & getActiveMSPList() const;

protected:
    virtual ~MasterScriptProviderFactory();

public:
    MasterScriptProviderFactory(
        css::uno::Reference< css::uno::XComponentContext > const & xComponentContext );

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw ( css::uno::RuntimeException );

    virtual sal_Bool SAL_CALL
        supportsService( ::rtl::OUString const & serviceName )
            throw ( css::uno::RuntimeException );

    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL
        getSupportedServiceNames()
            throw ( css::uno::RuntimeException );

    // XScriptProviderFactory
    virtual css::uno::Reference< css::script::provider::XScriptProvider >
        SAL_CALL createScriptProvider( const css::uno::Any& context )
            throw ( css::lang::IllegalArgumentException, css::uno::RuntimeException);
};


} // namespace func_provider
