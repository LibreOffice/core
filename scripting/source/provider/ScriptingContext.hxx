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

#ifndef _FRAMEWORK_SCRIPT_PROTOCOLHANDLER_SCRIPTING_CONTEXT_HXX_
#define _FRAMEWORK_SCRIPT_PROTOCOLHANDLER_SCRIPTING_CONTEXT_HXX_


#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/proparrhlp.hxx>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/broadcasthelper.hxx>
namespace func_provider
{
// for simplification
#define css ::com::sun::star

//Typedefs
//=============================================================================
//typedef ::cppu::WeakImplHelper1< css::beans::XPropertySet > ScriptingContextImpl_BASE;

class ScriptingContext : public ::comphelper::OMutexAndBroadcastHelper, public ::comphelper::OPropertyContainer,
                         public ::comphelper::OPropertyArrayUsageHelper< ScriptingContext >,  public css::lang::XTypeProvider, public ::cppu::OWeakObject
{

public:
    ScriptingContext( const css::uno::Reference< css::uno::XComponentContext > & xContext );
    ~ScriptingContext();
    // XInterface

    css::uno::Any SAL_CALL queryInterface( const css::uno::Type& rType )
        throw( css::uno::RuntimeException )
    {
        css::uno::Any aRet( OPropertySetHelper::queryInterface( rType ) );
        return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
    }
    void SAL_CALL acquire() throw() { ::cppu::OWeakObject::acquire(); }
    void SAL_CALL release() throw() { ::cppu::OWeakObject::release(); }
    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  )
            throw ( css::uno::RuntimeException );
    //XTypeProvider
    DECLARE_XTYPEPROVIDER( )

protected:

    // OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper(  );

    // OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper(  ) const;
private:
    css::uno::Reference< css::uno::XComponentContext > m_xContext;


};
} // func_provider
#endif //_FRAMEWORK_SCRIPT_PROVIDER_XFUNCTIONPROVIDER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
