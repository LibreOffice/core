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


#include "ScriptImpl.hxx"
#include "sal/log.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::script::framework;

namespace func_provider
{


ScriptImpl::ScriptImpl(
    const Reference< beans::XPropertySet > & scriptingContext,
    const Reference< runtime::XScriptInvocation > & runtimeMgr,
    const OUString& scriptURI )
throw ( RuntimeException ) :
        m_XScriptingContext( scriptingContext, UNO_SET_THROW ),
        m_RunTimeManager( runtimeMgr, UNO_SET_THROW ),
        m_ScriptURI( scriptURI )
{
    SAL_INFO("scripting.provider", "<!constucting a ScriptImpl>" );
}


ScriptImpl::~ScriptImpl()
{
    SAL_INFO("scripting.provider", "<Destructing a ScriptImpl>" );
}


Any SAL_CALL
ScriptImpl::invoke( const Sequence< Any >& aParams,
                      Sequence< sal_Int16 >& aOutParamIndex, Sequence< Any >& aOutParam )
throw ( lang::IllegalArgumentException, script::CannotConvertException,
        reflection::InvocationTargetException, RuntimeException )
{
    SAL_INFO("scripting.provider", "<ScriptImpl::invoke>" );
    Any result;
    Any anyScriptingContext;

    anyScriptingContext <<= m_XScriptingContext;
    try
    {
        result = m_RunTimeManager->invoke( m_ScriptURI, anyScriptingContext, aParams,
                                           aOutParamIndex, aOutParam );
    }
    catch ( const lang::IllegalArgumentException & iae )
    {
        OUString temp = "ScriptImpl::invoke IllegalArgumentException : ";
        throw lang::IllegalArgumentException( temp.concat( iae.Message ),
                                              Reference< XInterface > (),
                                              iae.ArgumentPosition );
    }
    catch ( const script::CannotConvertException & cce )
    {
        OUString temp = "ScriptImpl::invoke CannotConvertException : ";
        throw script::CannotConvertException( temp.concat( cce.Message ),
                                              Reference< XInterface > (),
                                              cce.DestinationTypeClass,
                                              cce.Reason,
                                              cce.ArgumentIndex );
    }
    catch ( const reflection::InvocationTargetException & ite )
    {
        OUString temp = "ScriptImpl::invoke InvocationTargetException : ";
        throw reflection::InvocationTargetException( temp.concat( ite.Message ),
                Reference< XInterface > (),
                ite.TargetException );
    }
    catch ( const RuntimeException & re )
    {
        OUString temp = "ScriptImpl::invoke RuntimeException : ";
        throw RuntimeException( temp.concat( re.Message ) );
    }
    return result;
}
} // namespace func_provider

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
