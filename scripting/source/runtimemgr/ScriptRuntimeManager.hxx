/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#ifndef _FRAMEWORK_SCRIPT_SCRIPTRUNTIMEMANAGER_HXX_
#define _FRAMEWORK_SCRIPT_SCRIPTRUNTIMEMANAGER_HXX_

#include <osl/mutex.hxx>

#include <cppuhelper/implbase3.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/script/CannotConvertException.hpp>
#include <com/sun/star/reflection/InvocationTargetException.hpp>

#include <drafts/com/sun/star/script/framework/runtime/XScriptInvocation.hpp>
#include <drafts/com/sun/star/script/framework/runtime/XScriptNameResolver.hpp>

namespace css = ::com::sun::star;

namespace scripting_runtimemgr
{

/**
 * Class responsible for managing the various ScriptRuntime implementations.
 */
class ScriptRuntimeManager : public
    ::cppu::WeakImplHelper3< ::drafts::com::sun::star::script::framework::runtime::XScriptInvocation, css::lang::XServiceInfo,
    ::drafts::com::sun::star::script::framework::runtime::XScriptNameResolver >
{
public:
    explicit ScriptRuntimeManager(
        const css::uno::Reference< css::uno::XComponentContext > & xContext );
    ~ScriptRuntimeManager();


    // XServiceInfo implementation
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw( css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw( css::uno::RuntimeException );
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw( css::uno::RuntimeException );

    /**
     * implements XScriptInvocation, invokes the script named in scriptURI
     * (resolving it first, if necessary), with the args passed.
     *
     * @param scriptURI the URI, which may not be fully qualified, for the
     *  script to be invoked
     *
     *************************************************************
      Invoke
        @param scriptUri script uri describing the script
        @param invocationCtx
             the invocation context contains the documentStorageID and document reference
             for use in script name resolving

        @param aParams all parameters; pure, out params are undefined in sequence,
            i.e., the value has to be ignored by the callee
        @param aOutParamIndex out indices
        @param aOutParam out parameters

        @returns
            the value returned from the function being invoked

        @throws IllegalArgumentException
            if there is no matching script name

        @throws CannotConvertException
            if args do not match or cannot be converted the those
            of the invokee

        @throws InvocationTargetException
            if the running script throws an exception this information is captured and
            rethrown as this exception type.

    */
    virtual css::uno::Any SAL_CALL invoke(
        const ::rtl::OUString & scriptUri,
        const css::uno::Any& invocationCtx,
        const css::uno::Sequence< css::uno::Any >& aParams,
        css::uno::Sequence< sal_Int16 >& aOutParamIndex,
        css::uno::Sequence< css::uno::Any >& aOutParam )
        throw ( css::lang::IllegalArgumentException,
                css::script::CannotConvertException,
                css::reflection::InvocationTargetException,
                css::uno::RuntimeException );

    /**
     * implements  XScriptNameResolver, attempts to resolve the script URI
     * passed in
     *
     * @param scriptURI the URI to be resolved
     * @param invocationCtx  the invocation context contains the
     * documentStorageID and document reference for use in script name
     * resolving. On full name resolution it sets the resolvedScriptStorageID to
     * the actual storage location of the fully resolved script. May or may not * be the
     same as the documentStorageID.
     * @return the resolved URI
     */
    virtual css::uno::Reference< ::drafts::com::sun::star::script::framework::storage::XScriptInfo > SAL_CALL resolve(
        const ::rtl::OUString& scriptUri,
        css::uno::Any& invocationCtx )
        throw( css::lang::IllegalArgumentException, css::script::CannotConvertException,
           css::uno::RuntimeException );

private:
    css::uno::Reference< ::drafts::com::sun::star::script::framework::runtime::XScriptInvocation > SAL_CALL getScriptRuntime(
        const css::uno::Reference< css::uno::XInterface > & scriptInfo )
        throw( css::uno::RuntimeException );
    css::uno::Reference< ::drafts::com::sun::star::script::framework::runtime::XScriptNameResolver > SAL_CALL getScriptNameResolver()
        throw( css::uno::RuntimeException );

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::lang::XMultiComponentFactory > m_xMgr;
    ::osl::Mutex m_mutex;
};
} // scripting_runtimemgr

#endif //_FRAMEWORK_SCRIPT_SCRIPTRUNTIMEMANAGER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
