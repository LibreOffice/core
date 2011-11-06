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



#ifndef _FRAMEWORK_SCRIPT_PROVIDER_XSCRIPTURIHELPER_HXX_
#define _FRAMEWORK_SCRIPT_PROVIDER_XSCRIPTURIHELPER_HXX_

#include <com/sun/star/script/provider/XScriptURIHelper.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>

#include <rtl/ustring.hxx>
#include <cppuhelper/implbase3.hxx>

namespace func_provider
{

#define css ::com::sun::star

class ScriptingFrameworkURIHelper :
    public ::cppu::WeakImplHelper3<
        css::script::provider::XScriptURIHelper,
        css::lang::XServiceInfo,
        css::lang::XInitialization >
{
private:

    css::uno::Reference< css::ucb::XSimpleFileAccess > m_xSimpleFileAccess;
    css::uno::Reference<css::uri::XUriReferenceFactory> m_xUriReferenceFactory;

    ::rtl::OUString m_sLanguage;
    ::rtl::OUString m_sLocation;
    ::rtl::OUString m_sBaseURI;

    ::rtl::OUString SCRIPTS_PART;

    bool initBaseURI();
    ::rtl::OUString getLanguagePart(const ::rtl::OUString& rStorageURI);
    ::rtl::OUString getLanguagePath(const ::rtl::OUString& rLanguagePart);

public:

    ScriptingFrameworkURIHelper(
        const css::uno::Reference< css::uno::XComponentContext >& xContext )
            throw( css::uno::RuntimeException );

    ~ScriptingFrameworkURIHelper();

    virtual void SAL_CALL
        initialize( const css::uno::Sequence < css::uno::Any > & args )
            throw ( css::uno::Exception, css::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL
        getRootStorageURI()
            throw ( ::com::sun::star::uno::RuntimeException );

    virtual ::rtl::OUString SAL_CALL
        getScriptURI( const ::rtl::OUString& rStorageURI )
            throw( css::lang::IllegalArgumentException,
                   css::uno::RuntimeException );

    virtual ::rtl::OUString SAL_CALL
        getStorageURI( const ::rtl::OUString& rScriptURI )
            throw( css::lang::IllegalArgumentException,
                   css::uno::RuntimeException );

    virtual ::rtl::OUString SAL_CALL
        getImplementationName()
            throw( css::uno::RuntimeException );

    virtual sal_Bool SAL_CALL
        supportsService( const ::rtl::OUString& ServiceName )
            throw( css::uno::RuntimeException );

    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL
        getSupportedServiceNames()
            throw( css::uno::RuntimeException );
};

} // namespace func_provider
#endif //_FRAMEWORK_SCRIPT_PROVIDER_XSCRIPTURIHELPER_HXX_
