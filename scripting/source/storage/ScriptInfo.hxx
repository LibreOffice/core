/*************************************************************************
 *
 *  $RCSfile: ScriptInfo.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: npower $ $Date: 2002-10-24 10:37:52 $
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

#include <cppuhelper/implbase2.hxx> // helper for component factory

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <drafts/com/sun/star/script/framework/storage/XScriptInfo.hpp>

#include "ScriptData.hxx"

namespace scripting_impl
{
// for simplification
#define css ::com::sun::star
#define dcsssf ::drafts::com::sun::star::script::framework

class ScriptInfo : public ::cppu::WeakImplHelper2< css::lang::XServiceInfo,
    dcsssf::storage::XScriptInfo >
{
public:
    explicit ScriptInfo(
        const css::uno::Reference< css::uno::XComponentContext >& xContext );
    explicit ScriptInfo(
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const ScriptData & scriptData, sal_Int32 storageID );
    virtual ~ScriptInfo();

    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw( css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw( css::uno::RuntimeException );
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
        throw( css::uno::RuntimeException );

    // XScriptInfo
    virtual ::rtl::OUString SAL_CALL getLogicalName()
        throw ( css::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getDescription()
        throw ( css::uno::RuntimeException ) ;
    virtual ::rtl::OUString SAL_CALL getLanguage() throw ( css::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getScriptLocation()
        throw ( css::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getFunctionName()
        throw ( css::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getLocation()
        throw ( css::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL getParcelURI()
        throw ( css::uno::RuntimeException );
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL
        getLanguageProperties() throw ( css::uno::RuntimeException );


private:
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::lang::XMultiComponentFactory > m_xMgr;

    ::osl::Mutex     m_mutex;

    ScriptData m_scriptData;
    sal_Int32 m_storageID;

};

}
#endif // define __SCRIPTING_STORAGE...
