/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
************************************************************************/


#ifndef _FRAMEWORK_SCRIPT_SCRIPTSECURITYMANAGER_HXX_
#define _FRAMEWORK_SCRIPT_SCRIPTSECURITYMANAGER_HXX_

#include <boost/unordered_map.hpp>
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

typedef ::boost::unordered_map< ::rtl::OUString, StoragePerm, ::rtl::OUStringHash,
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
