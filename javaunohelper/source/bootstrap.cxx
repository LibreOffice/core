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

#include <sal/config.h>
#include <sal/log.hxx>

#include <osl/diagnose.h>

#include <rtl/alloc.h>
#include <rtl/bootstrap.hxx>
#include <rtl/string.hxx>

#include <uno/lbnames.h>
#include <uno/mapping.hxx>
#include <uno/environment.hxx>

#include <cppuhelper/bootstrap.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>

#include <jni.h>
#include <jvmaccess/virtualmachine.hxx>
#include <jvmaccess/unovirtualmachine.hxx>
#include <tools/diagnose_ex.h>

#include "juhx-export-functions.hxx"
#include "vm.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace javaunohelper
{

static OUString jstring_to_oustring( jstring jstr, JNIEnv * jni_env )
{
    OSL_ASSERT( sizeof (sal_Unicode) == sizeof (jchar) );
    jsize len = jni_env->GetStringLength( jstr );
    rtl_uString * ustr =
        static_cast<rtl_uString *>(std::malloc( sizeof (rtl_uString) + (len * sizeof (sal_Unicode)) ));
    jni_env->GetStringRegion( jstr, 0, len, reinterpret_cast<jchar *>(ustr->buffer) );
    OSL_ASSERT( !jni_env->ExceptionCheck() );
    ustr->refCount = 1;
    ustr->length = len;
    ustr->buffer[ len ] = '\0';
    return OUString( ustr, SAL_NO_ACQUIRE );
}

}


jobject Java_com_sun_star_comp_helper_Bootstrap_cppuhelper_1bootstrap(
    JNIEnv * jni_env, SAL_UNUSED_PARAMETER jclass, jstring juno_rc, jobjectArray jpairs,
    jobject loader )
{
    try
    {
        if (nullptr != jpairs)
        {
            jsize nPos = 0, len = jni_env->GetArrayLength( jpairs );
            while (nPos < len)
            {
                // name
                jstring jstr = static_cast<jstring>(jni_env->GetObjectArrayElement( jpairs, nPos ));
                if (jni_env->ExceptionCheck())
                {
                    jni_env->ExceptionClear();
                    throw RuntimeException( "index out of bounds?!" );
                }
                if (nullptr != jstr)
                {
                    OUString name( ::javaunohelper::jstring_to_oustring( jstr, jni_env ) );
                    // value
                    jstr = static_cast<jstring>(jni_env->GetObjectArrayElement( jpairs, nPos +1 ));
                    if (jni_env->ExceptionCheck())
                    {
                        jni_env->ExceptionClear();
                        throw RuntimeException( "index out of bounds?!" );
                    }
                    if (nullptr != jstr)
                    {
                        OUString value( ::javaunohelper::jstring_to_oustring( jstr, jni_env ) );

                        // set bootstrap parameter
                        ::rtl::Bootstrap::set( name, value );
                    }
                }
                nPos += 2;
            }
        }

        // bootstrap uno
        Reference< XComponentContext > xContext;
        if (nullptr == juno_rc)
        {
            xContext = ::cppu::defaultBootstrap_InitialComponentContext();
        }
        else
        {
            OUString uno_rc( ::javaunohelper::jstring_to_oustring( juno_rc, jni_env ) );
            xContext = ::cppu::defaultBootstrap_InitialComponentContext( uno_rc );
        }

        // create vm access
        ::rtl::Reference< ::jvmaccess::UnoVirtualMachine > vm_access(
            ::javaunohelper::create_vm_access( jni_env, loader ) );
        // wrap vm singleton entry
        xContext = ::javaunohelper::install_vm_singleton( xContext, vm_access );

        // get uno envs
        OUString cpp_env_name = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
        OUString java_env_name = UNO_LB_JAVA;
        Environment java_env, cpp_env;
        uno_getEnvironment(reinterpret_cast<uno_Environment **>(&cpp_env), cpp_env_name.pData, nullptr);
        uno_getEnvironment(reinterpret_cast<uno_Environment **>(&java_env), java_env_name.pData, vm_access.get() );

        // map to java
        Mapping mapping( cpp_env.get(), java_env.get() );
        if (! mapping.is())
        {
            Reference< lang::XComponent > xComp( xContext, UNO_QUERY );
            if (xComp.is())
                xComp->dispose();
            throw RuntimeException("cannot get mapping C++ <-> Java!" );
        }

        jobject jret = static_cast<jobject>(mapping.mapInterface( xContext.get(), cppu::UnoType<decltype(xContext)>::get() ));
        jobject jlocal = jni_env->NewLocalRef( jret );
        jni_env->DeleteGlobalRef( jret );

        return jlocal;
    }
    catch (const RuntimeException & exc)
    {
        css::uno::Any exAny( cppu::getCaughtException() );
        jclass c = jni_env->FindClass( "com/sun/star/uno/RuntimeException" );
        if (nullptr != c)
        {
            SAL_WARN("javaunohelper", "forwarding RuntimeException: " << exceptionToString(exAny) );
            OString cstr( OUStringToOString(
                              exc.Message, RTL_TEXTENCODING_JAVA_UTF8 ) );
            jni_env->ThrowNew( c, cstr.getStr() );
        }
    }
    catch (const Exception & exc)
    {
        css::uno::Any ex( cppu::getCaughtException() );
        jclass c = jni_env->FindClass( "com/sun/star/uno/Exception" );
        if (nullptr != c)
        {
            SAL_WARN("javaunohelper",  "forwarding Exception: " << exceptionToString(ex) );
            OString cstr( OUStringToOString(
                              exc.Message, RTL_TEXTENCODING_JAVA_UTF8 ) );
            jni_env->ThrowNew( c, cstr.getStr() );
        }
    }

    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
