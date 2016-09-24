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

#ifndef INCLUDED_SCRIPTING_SOURCE_PROVIDER_SCRIPTIMPL_HXX
#define INCLUDED_SCRIPTING_SOURCE_PROVIDER_SCRIPTIMPL_HXX

#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/script/CannotConvertException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/reflection/InvocationTargetException.hpp>

#include <drafts/com/sun/star/script/framework/provider/XScript.hpp>
#include <drafts/com/sun/star/script/framework/runtime/XScriptInvocation.hpp>

namespace func_provider
{

class ScriptImpl :
            public ::cppu::WeakImplHelper < ::draftscss::script::framework::provider::XScript >
{

public:
    /*************************************************************
      ScriptImpl Constructor
      @param runtimeMgr which is a service that implement a XScriptInvocation
      @param scriptURI the received ScriptURI that needs to be resolve and invoked
    */
    ScriptImpl(
        const css::uno::Reference< css::beans::XPropertySet > & scriptingContext,
        const css::uno::Reference< ::draftscss::script::framework::runtime::XScriptInvocation > & runtimeMgr,
        const OUString& scriptURI )
    throw ( css::uno::RuntimeException );

    /*************************************************************
      ScriptImpl Destructor
    */
    ~ScriptImpl();

    /*************************************************************
      Invoke
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
            of the invoked

      @throws InvocationTargetException
            if the running script throws an exception this information is captured and
            rethrown as this exception type.

    */
    virtual css::uno::Any SAL_CALL invoke(
        const css::uno::Sequence< css::uno::Any > & aParams,
        css::uno::Sequence< sal_Int16 > & aOutParamIndex,
        css::uno::Sequence< css::uno::Any > & aOutParam )
    throw ( css::lang::IllegalArgumentException,
            css::script::CannotConvertException,
            css::reflection::InvocationTargetException,
            css::uno::RuntimeException );

private:
    css::uno::Reference< css::beans::XPropertySet > m_XScriptingContext;
    css::uno::Reference < ::draftscss::script::framework::runtime::XScriptInvocation > m_RunTimeManager;
    OUString m_ScriptURI;

    /* copy ctor disabled, i.e. not defined */
    ScriptImpl( const ScriptImpl& );
    /* assignment disabled,  i.e. not defined */
    ScriptImpl& operator = ( const ScriptImpl& );
};
} // namespace func_provider
#endif // INCLUDED_SCRIPTING_SOURCE_PROVIDER_SCRIPTIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
