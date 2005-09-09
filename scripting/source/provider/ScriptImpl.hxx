/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ScriptImpl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:30:55 $
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
