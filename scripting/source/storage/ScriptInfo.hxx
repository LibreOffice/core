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



#ifndef __SCRIPT_FRAMEWORK_STORAGE_SCRIPTINFO_HXX_
#define __SCRIPT_FRAMEWORK_STORAGE_SCRIPTINFO_HXX_

#include <cppuhelper/implbase1.hxx> // helper for component factory

#include <com/sun/star/beans/XPropertySet.hpp>

#include <drafts/com/sun/star/script/framework/storage/XScriptInfo.hpp>

#include "ScriptData.hxx"

namespace scripting_impl
{
// for simplification
#define css ::com::sun::star
#define dcsssf ::drafts::com::sun::star::script::framework

class ScriptInfo : public ::cppu::WeakImplHelper1< dcsssf::storage::XScriptInfo >
{
public:
    explicit ScriptInfo( const ScriptData & scriptData, sal_Int32 storageID );
    virtual ~ScriptInfo();

    // XScriptInfo
    virtual ::rtl::OUString SAL_CALL getLogicalName()
        throw ( css::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getDescription()
        throw ( css::uno::RuntimeException ) ;
    virtual ::rtl::OUString SAL_CALL getLanguage() throw ( css::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getFunctionName()
        throw ( css::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getParcelURI()
        throw ( css::uno::RuntimeException );
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL
        getLanguageProperties() throw ( css::uno::RuntimeException );
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getFileSetNames()
        throw ( css::uno::RuntimeException );
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL
        getFilesInFileSet( const ::rtl::OUString & fileSetName )
        throw ( css::uno::RuntimeException );

private:
    ScriptData m_scriptData;
    sal_Int32 m_storageID;
};

}
#endif // define __SCRIPTING_STORAGE...
