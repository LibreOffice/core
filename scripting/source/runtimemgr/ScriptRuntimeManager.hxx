/*************************************************************************
*
*  $RCSfile: ScriptRuntimeManager.hxx,v $
*
*  $Revision: 1.10 $
*
*  last change: $Author: dfoster $ $Date: 2003-07-23 10:19:55 $
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

namespace scripting_runtimemgr
{
// for simplification
#define css ::com::sun::star
#define dcsssf ::drafts::com::sun::star::script::framework

/**
 * Class responsible for managing the various ScriptRuntime implementations.
 */
class ScriptRuntimeManager : public
    ::cppu::WeakImplHelper3< dcsssf::runtime::XScriptInvocation, css::lang::XServiceInfo,
    dcsssf::runtime::XScriptNameResolver >
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
    virtual css::uno::Reference< dcsssf::storage::XScriptInfo > SAL_CALL resolve(
        const ::rtl::OUString& scriptUri,
        css::uno::Any& invocationCtx )
        throw( css::lang::IllegalArgumentException, css::script::CannotConvertException,
           css::uno::RuntimeException );

private:
    css::uno::Reference< dcsssf::runtime::XScriptInvocation > SAL_CALL getScriptRuntime(
        const css::uno::Reference< css::uno::XInterface > & scriptInfo )
        throw( css::uno::RuntimeException );
    css::uno::Reference< dcsssf::runtime::XScriptNameResolver > SAL_CALL getScriptNameResolver()
        throw( css::uno::RuntimeException );

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::lang::XMultiComponentFactory > m_xMgr;
    ::osl::Mutex m_mutex;
};
} // scripting_runtimemgr

#endif //_FRAMEWORK_SCRIPT_SCRIPTRUNTIMEMANAGER_HXX_
