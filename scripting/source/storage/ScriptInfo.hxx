/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ScriptInfo.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:36:22 $
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
