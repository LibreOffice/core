/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ScriptStorageManager.hxx,v $
 * $Revision: 1.19 $
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


#ifndef _DRAFTS_COM_SUN_STAR_SCRIPT_FRAMEWORK_STORAGE_SCRIPTSTORAGEMANAGER_HXX_
#define _DRAFTS_COM_SUN_STAR_SCRIPT_FRAMEWORK_STORAGE_SCRIPTSTORAGEMANAGER_HXX_

#include <hash_map>
#include <map>

#include <osl/mutex.hxx>
#include <cppuhelper/implbase4.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/security/AccessControlException.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

#include <drafts/com/sun/star/script/framework/storage/XScriptStorageManager.hpp>
#include <drafts/com/sun/star/script/framework/storage/XScriptStorageRefresh.hpp>
#include <drafts/com/sun/star/script/framework/storage/XScriptInfo.hpp>
#include <drafts/com/sun/star/script/framework/security/XScriptSecurity.hpp>
#include "ScriptSecurityManager.hxx"


namespace scripting_impl
{
// for simplification
#define css ::com::sun::star
#define dcsssf ::drafts::com::sun::star::script::framework

// Define a map used to store the ScriptingStorages key;d by ID
typedef ::std::map < sal_Int32, css::uno::Reference < css::uno::XInterface > >
    ScriptStorage_map;

typedef ::std::hash_map < ::rtl::OUString, sal_Int32, ::rtl::OUStringHash>
    StorageId_hash;

class ScriptStorageManager : public
    ::cppu::WeakImplHelper4 < dcsssf::storage::XScriptStorageManager,
    dcsssf::security::XScriptSecurity, css::lang::XServiceInfo,
    css::lang::XEventListener >
{
public:
    explicit ScriptStorageManager(
        const css::uno::Reference< css::uno::XComponentContext > & xContext )
        SAL_THROW ( ( css::uno::RuntimeException ) );


    ~ScriptStorageManager() SAL_THROW ( () );

    // XServiceInfo implementation
    //======================================================================
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw( css::uno::RuntimeException );
    //----------------------------------------------------------------------
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw( css::uno::RuntimeException );
    //----------------------------------------------------------------------
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw( css::uno::RuntimeException );
    //======================================================================

    //XScriptStorageManager
    //======================================================================
    /**
        create a ScriptStorage using the XSimpleFileAccess passed as an
        argument, and return a ID for getting the associated ScriptStorage

        @params xSFA
        an implementation of XSimpleFileAccess that knows its own base URL
        and can thus take URLs relative to that base.

        @returns an unsigned short ScriptStorage ID, which can be used in the
        getScriptStorage method
    */
    virtual sal_Int32 SAL_CALL createScriptStorage(
            const css::uno::Reference< css::ucb::XSimpleFileAccess > & xSFA )
            throw ( css::uno::RuntimeException );
    //----------------------------------------------------------------------
    /**
        create a ScriptStorage using the XSimpleFileAccess, and a string URL
        and return a ID for getting the associated ScriptStorage

        @params xSFA
        a standard implementation of XSimpleFileAccess

        @params stringURI
        a string URI to the head of the script storage

        @returns an unsigned short ScriptStorage ID, which can be used in the
        getScriptStorage method
    */
    virtual sal_Int32 SAL_CALL
        createScriptStorageWithURI(
            const css::uno::Reference< css::ucb::XSimpleFileAccess >& xSFA,
            const ::rtl::OUString& stringURI )
            throw ( css::uno::RuntimeException );
    //----------------------------------------------------------------------
    /**
        get a ScriptStorage component using its scriptStorageID

        @params scriptStorageID
        the usigned short returned by one of the methods above. ID=0 is
        reserved for the application/share scripts, and ID=1 is reserved
        for the application/user scripts

        @returns an XInterface to a component that implements the ScriptStorage
        service
    */
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getScriptStorage(
        sal_Int32 scriptStorageID )
        throw ( css::uno::RuntimeException );

    /**
        get a ScriptStorage ID using storage URI

        @param scriptStorageURI
        the file URL for the document will retrieve storage id for the document,        special treatment is reserved for the strings "USER"and "SHARE" which
        retrieve storage id for application/user application/share areas respectively.


        @returns as long ScriptStorage ID (-1 if no storage exists), which can be used in getScriptStorage method.

    */

    virtual sal_Int32 SAL_CALL getScriptStorageID( const ::rtl::OUString& scriptStorageURI )
        throw (::com::sun::star::uno::RuntimeException);

    /**
        refresh a storage component using its URI

        @param StringURI
        URI of storage area, for documents, url to document eg, file:///home/user/ADocument.sxw To refresh the storage for user or share area, use USER or SHARE respectively instead or a url.

    */

    virtual void SAL_CALL refreshScriptStorage(const ::rtl::OUString & stringURI)
    throw ( css::uno::RuntimeException );
    //======================================================================

    //XScriptSecurity
    //======================================================================
    //----------------------------------------------------------------------
    /**
        the language independent interface for invocation

        @param scriptURI
            a string containing the script URI

        @returns
            the value returned from the function being invoked

        @throws IllegalArgumentException
            if there is no matching script name

    */
    virtual void SAL_CALL ScriptStorageManager::checkPermission(
        const rtl::OUString & scriptStorageURI,
        const rtl::OUString & permissionRequest )
        throw ( css::uno::RuntimeException, css::lang::IllegalArgumentException,
            css::security::AccessControlException );
    //======================================================================

    //XEventListener
    //======================================================================

    virtual void SAL_CALL disposing( const css::lang::EventObject& Source )
        throw ( css::uno::RuntimeException );

private:
    ScriptStorageManager( const ScriptStorageManager & );
    ScriptStorageManager& operator= ( const ScriptStorageManager & );

    void removeScriptDocURIHashEntry( const ::rtl::OUString & origURI );

    // to obtain other services if needed
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::lang::XMultiComponentFactory > m_xMgr;
    ::osl::Mutex m_mutex;
    ScriptStorage_map m_ScriptStorageMap;
    StorageId_hash m_StorageIdOrigURIHash;
    sal_Int32 m_count;
    scripting_securitymgr::ScriptSecurityManager m_securityMgr;

    void setupAppStorage( const css::uno::Reference< css::util::XMacroExpander > & xME,
        const ::rtl::OUString & storageStr,
        const ::rtl::OUString & appStr)
        SAL_THROW ( ( css::uno::RuntimeException ) );

    sal_Int32 setupAnyStorage(
        const css::uno::Reference< css::ucb::XSimpleFileAccess> & xSFA,
        const ::rtl::OUString & storageStr,
        const ::rtl::OUString & origStringURI )
        SAL_THROW ( ( css::uno::RuntimeException ) );

};
} // scripting_impl

#endif //_COM_SUN_STAR_SCRIPTING_STORAGE_SCRIPTSTORAGEMANAGER_HXX_
