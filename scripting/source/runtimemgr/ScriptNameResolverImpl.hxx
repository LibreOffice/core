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

#ifndef _FRAMEWORK_SCRIPT_SCRIPTNAMERESOLVERIMPL_HXX_
#define  _FRAMEWORK_SCRIPT_SCRIPTNAMERESOLVERIMPL_HXX_

#include <cppuhelper/implbase1.hxx> // helper for XInterface, XTypeProvider etc.

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/script/CannotConvertException.hpp>
#include <com/sun/star/reflection/InvocationTargetException.hpp>

#include <drafts/com/sun/star/script/framework/runtime/XScriptNameResolver.hpp>
#include <drafts/com/sun/star/script/framework/storage/XScriptInfoAccess.hpp>
#include <drafts/com/sun/star/script/framework/storage/XScriptInfo.hpp>

namespace scripting_runtimemgr
{
// for simplification
#define css ::com::sun::star
#define dcsssf ::drafts::com::sun::star::script::framework

class ScriptNameResolverImpl : public
    ::cppu::WeakImplHelper1 < dcsssf::runtime::XScriptNameResolver >
{
public:
    /**********************************************
     ScriptNameResolverImpl Constructor
     @param  the current context
    */
    ScriptNameResolverImpl(
        const css::uno::Reference< css::uno::XComponentContext > & xContext );
    ~ScriptNameResolverImpl();

    // XServiceInfo implementation
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw( css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw( css::uno::RuntimeException );
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw( css::uno::RuntimeException );

    /**********************************************
     resolve method
     @param  scriptURI this is the given ScriptURI
     @param invocationCtx  the invocation context contains the
      documentStorageID and document reference for use in script name
      resolving. On full name resolution it sets the resolvedScriptStorageID to
      the actual storage location of the fully resolved script. May or may not * be the
      same as the documentStorageID.
     @exception CannotResolveScriptNameException
     @exception IllegalArgumentException
     @exception NullPointerException
     @return  the resolved XScriptURI
    */
    css::uno::Reference < dcsssf::storage::XScriptInfo > SAL_CALL resolve(
        const ::rtl::OUString & scriptURI,
        css::uno::Any& invocationCtx )
        throw( css::script::CannotConvertException, css::lang::IllegalArgumentException,
           css::uno::RuntimeException );
private:
    css::uno::Reference < dcsssf::storage::XScriptInfo >
    resolveURIFromStorageID( sal_Int32 sid, const rtl::OUString & docURI,
        const ::rtl::OUString & nameToResolve )
        SAL_THROW ( ( css::lang::IllegalArgumentException, css::uno::RuntimeException ) );
    css::uno::Reference< dcsssf::storage::XScriptInfoAccess >
    getStorageInstance( sal_Int32 sid, const rtl::OUString & permissionURI)
        SAL_THROW ( ( css::uno::RuntimeException ) );
    ::rtl::OUString
    ScriptNameResolverImpl::getFilesysURL( const ::rtl::OUString & scriptURI )
        throw( css::lang::IllegalArgumentException );

    /**********************************************
     Reference< XComponentContext > m_xContext
        to obtain other services if needed
    */
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::lang::XMultiComponentFactory > m_xMultiComFac;
    ::osl::Mutex m_mutex;

};
} // scripting_runtimemgr

#endif //_FRAMEWORK_SCRIPT_SCRIPTNAMERESOLVERIMPL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
