/*************************************************************************
 *
 *  $RCSfile: ScriptInfo.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: lkovacs $ $Date: 2002-09-23 14:08:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef __SCRIPT_FRAMEWORK_STORAGE_SCRIPTINFO_HXX_
#define __SCRIPT_FRAMEWORK_STORAGE_SCRIPTINFO_HXX_

#include <cppuhelper/implbase4.hxx> // helper for component factory

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <drafts/com/sun/star/script/framework/storage/XScriptInfo.hpp>
#include <drafts/com/sun/star/script/framework/storage/ScriptImplInfo.hpp>
#ifndef _DRAFTS_COM_SUN_STAR_SCRIPT_FRAMEWORK_XSCRIPTINVOCATIONPREP_HPP_
#include <drafts/com/sun/star/script/framework/storage/XScriptInvocationPrep.hpp>
#endif

namespace scripting_impl {

class ScriptInfo : public ::cppu::WeakImplHelper4< ::com::sun::star::lang::XServiceInfo, ::com::sun::star::lang::XInitialization, ::drafts::com::sun::star::script::framework::storage::XScriptInfo, ::drafts::com::sun::star::script::framework::storage::XScriptInvocationPrep >
{
    // private member
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext> m_xContext;

    ::osl::Mutex     m_mutex;

    ::drafts::com::sun::star::script::framework::storage::ScriptImplInfo m_scriptImplInfo;
    sal_uInt16 m_storageID;

    // public interface
public:
    explicit ScriptInfo( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& );
    virtual ~ScriptInfo();

    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL initialize(::com::sun::star::uno::Sequence < ::com::sun::star::uno::Any > const & args) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::uno::Exception);

    // XScriptInfo
    virtual ::rtl::OUString SAL_CALL getLogicalName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setLogicalName( const ::rtl::OUString& name ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getDescription(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDescription( const ::rtl::OUString& desc ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getLanguage(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getScriptLocation(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setLanguage( const ::rtl::OUString& language ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasSource(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getLanguageSpecificName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setLanguageSpecificName( const ::rtl::OUString& langName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getRoot(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getDependencies(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getLocation(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL extraProperties(  ) throw (::com::sun::star::uno::RuntimeException);
    /**
        This function prepares the script for invocation and returns the full path
       to the prepared parcel folder

       @return
        <type>::rtl::OUString</type> file URI to the prepared parcel

    */
    virtual ::rtl::OUString SAL_CALL prepareForInvocation() throw (::com::sun::star::uno::RuntimeException);

};

}
#endif // define __SCRIPTING_STORAGE...
