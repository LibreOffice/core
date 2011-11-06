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




#ifndef _FRAMEWORK_SCRIPT_SCRIPTSECURITYMANAGER_HXX_
#define _FRAMEWORK_SCRIPT_SCRIPTSECURITYMANAGER_HXX_

#include <hash_map>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/security/AccessControlException.hpp>
#include <drafts/com/sun/star/script/framework/storage/XScriptInfo.hpp>

namespace scripting_securitymgr
{
// for simplification
#define css ::com::sun::star
#define dcsssf ::drafts::com::sun::star::script::framework

struct StoragePerm {
    rtl::OUString scriptStorageURL;
    sal_Int32 storageID;
    sal_Bool execPermission;
};

typedef ::std::hash_map< ::rtl::OUString, StoragePerm, ::rtl::OUStringHash,
    ::std::equal_to< ::rtl::OUString > > Permission_Hash;
/**
 * Class responsible for managing the ScriptSecurity.
 */
class ScriptSecurityManager
{
public:
    explicit ScriptSecurityManager(
        const css::uno::Reference< css::uno::XComponentContext > & xContext )
        throw ( css::uno::RuntimeException );
    ~ScriptSecurityManager();
    void addScriptStorage( rtl::OUString scriptStorageURL, sal_Int32 storageID)
        throw ( css::uno::RuntimeException );
/**
 * checks to see if the requested permission can be granted
 * checks to see whether the requested ScriptPeremission is allowed.
 */
    void checkPermission( const rtl::OUString & scriptStorageURL,
        const rtl::OUString & permissionRequest )
        throw ( css::uno::RuntimeException, css::lang::IllegalArgumentException,
            css::security::AccessControlException );
    void removePermissionSettings ( ::rtl::OUString & scriptStorageURL );
private:
    void readConfiguration() throw (css::uno::RuntimeException);

    short executeDialog ( const rtl::OUString & path )
        throw (css::uno::RuntimeException);
    short executeStandardDialog()
        throw ( css::uno::RuntimeException );
    short executePathDialog(const rtl::OUString & path)
        throw ( css::uno::RuntimeException );

        void addToSecurePaths ( const rtl::OUString & path )
        throw (css::uno::RuntimeException);
    bool isSecureURL( const rtl::OUString & path );
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    sal_Bool m_confirmationRequired;
    sal_Bool m_warning;
    sal_Int32 m_runMacroSetting;
    css::uno::Reference< css::lang::XMultiServiceFactory > m_xConfigProvFactory;
    css::uno::Sequence< rtl::OUString > m_secureURL;
    Permission_Hash m_permissionSettings;

};
} // scripting_securitymgr

#endif //_FRAMEWORK_SCRIPT_SCRIPTSECURITYMANAGER_HXX_
