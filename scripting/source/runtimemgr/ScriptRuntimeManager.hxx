/*************************************************************************
 *
 *  $RCSfile: ScriptRuntimeManager.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dfoster $ $Date: 2002-09-20 14:33:32 $
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


#ifndef _DRAFTS_COM_SUN_STAR_SCRIPT_FRAMEWORK_SCRIPTRUNTIMEMANAGER_HXX_
#define _DRAFTS_COM_SUN_STAR_SCRIPT_FRAMEWORK_SCRIPTRUNTIMEMANAGER_HXX_

#include <osl/mutex.hxx>

#include <cppuhelper/implbase3.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/script/CannotConvertException.hpp>
#include <com/sun/star/reflection/InvocationTargetException.hpp>

#include <drafts/com/sun/star/script/framework/XScriptInvocation.hpp>
#include <drafts/com/sun/star/script/framework/XScriptNameResolver.hpp>
#include <drafts/com/sun/star/script/framework/scripturi/XScriptURI.hpp>

namespace scripting_runtimemgr
{
/**
 * Class responsible for managing the various ScriptRuntime implementations.
 */
class ScriptRuntimeManager : public ::cppu::WeakImplHelper3< ::drafts::com::sun::star::script::framework::XScriptInvocation, ::com::sun::star::lang::XServiceInfo, ::drafts::com::sun::star::script::framework::XScriptNameResolver >
{
    // to obtain other services if needed
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;
    ::osl::Mutex     m_mutex;
public:
    ScriptRuntimeManager( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & xContext );
    ~ScriptRuntimeManager();


    // XServiceInfo implementation
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    /**
     * implements XScriptInvocation, invokes the script named in scriptURI
     * (resolving it first, if necessary), with the args passed.
     *
     * @param scriptURI the URI, which may not be fully qualified, for the
     *  script to be invoked
     *
    /*************************************************************
      Invoke
        @param scriptUri script uri describing the script
        @param invocationCtx
             the invocation context contains the documentStorageID and document reference for use in script name resolving

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
            if the running script throws an exception this information is captured and rethrown as this exception type.

    */
    virtual ::com::sun::star::uno::Any SAL_CALL invoke(
        const ::com::sun::star::uno::Reference < ::drafts::com::sun::star::script::framework::scripturi::XScriptURI > & scriptUri,
        const ::com::sun::star::uno::Any& invocationCtx, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aParams, ::com::sun::star::uno::Sequence< sal_Int16 >& aOutParamIndex, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aOutParam ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::script::CannotConvertException, ::com::sun::star::reflection::InvocationTargetException, ::com::sun::star::uno::RuntimeException);

    /**
     * implements  XScriptNameResolver, attempts to resolve the script URI
     * passed in
     *
     * @param scriptURI the URI to be resolved
     * @param invocationCtx  the invocation context contains the
     * documentStorageID and document reference for use in script name
     * resolving. On full name resolution it sets the resolvedScriptStorageID to
     * the actual storage location of the fully resolved script. May or may not * be the same as the documentStorageID.
     * @return the resolved URI
     */
    virtual ::com::sun::star::uno::Reference < ::drafts::com::sun::star::script::framework::scripturi::XScriptURI > SAL_CALL
    resolve( const ::com::sun::star::uno::Reference < ::drafts::com::sun::star::script::framework::scripturi::XScriptURI >& scriptUri ,::com::sun::star::uno::Any& invocationCtx)
    throw(::com::sun::star::lang::IllegalArgumentException,
          ::com::sun::star::script::CannotConvertException,
          ::com::sun::star::uno::RuntimeException);

private:
    ::com::sun::star::uno::Reference < ::drafts::com::sun::star::script::framework::XScriptInvocation > SAL_CALL getScriptRuntime(const ::com::sun::star::uno::Reference < ::drafts::com::sun::star::script::framework::scripturi::XScriptURI > & scriptUri) throw( ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference < ::drafts::com::sun::star::script::framework::XScriptNameResolver > SAL_CALL getScriptNameResolver() throw( ::com::sun::star::uno::RuntimeException);
};
} // scripting_runtimemgr

#endif //_COM_SUN_STAR_SCRIPTING_SCRIPTRUNTIMEMANAGER_HXX_
