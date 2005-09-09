/*************************************************************************
*
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ScriptSecurityManager.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:37:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
************************************************************************/


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
