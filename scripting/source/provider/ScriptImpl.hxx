/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _FRAMEWORK_SCRIPT_PROVIDER_FUNCTIONIMPL_HXX_
#define  _FRAMEWORK_SCRIPT_PROVIDER_FUNCTIONIMPL_HXX_

#include <cppuhelper/implbase1.hxx> // helper for XInterface, XTypeProvider etc.
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
// for simplification
#define css ::com::sun::star
#define dcsssf ::drafts::com::sun::star::script::framework


class ScriptImpl :
            public ::cppu::WeakImplHelper1 < dcsssf::provider::XScript >
{

public:
    /*************************************************************
      ScriptImpl Constructor
      @param runtimeMgr which is a service that implement a XScriptInvocation
      @param scriptURI the received ScriptURI that needs to be resolve and invoked
    */
    ScriptImpl(
        const css::uno::Reference< css::beans::XPropertySet > & scriptingContext,
        const css::uno::Reference< dcsssf::runtime::XScriptInvocation > & runtimeMgr,
        const ::rtl::OUString& scriptURI )
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
            of the invokee

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
    css::uno::Reference < dcsssf::runtime::XScriptInvocation > m_RunTimeManager;
    ::rtl::OUString m_ScriptURI;

    /* copy ctor disabled, i.e. not defined */
    ScriptImpl( const ScriptImpl& );
    /* assignment disabled,  i.e. not defined */
    ScriptImpl& operator = ( const ScriptImpl& );
};
} // namespace func_provider
#endif //_FRAMEWORK_SCRIPT_PROVIDER_FUNCTIONIMPL_HXX_
