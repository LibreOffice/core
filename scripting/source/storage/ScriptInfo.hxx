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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
