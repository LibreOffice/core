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


#include "osl/diagnose.h"

#include "rtl/alloc.h"
#include "rtl/bootstrap.hxx"
#include "rtl/string.hxx"

#include "uno/mapping.hxx"
#include "uno/environment.hxx"

#include "cppuhelper/bootstrap.hxx"

#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/lang/XSingleComponentFactory.hpp"

#include "jni.h"
#include "jvmaccess/virtualmachine.hxx"
#include "jvmaccess/unovirtualmachine.hxx"

#include "vm.hxx"

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OString;
using ::rtl::OUString;

namespace javaunohelper
{

inline ::rtl::OUString jstring_to_oustring( jstring jstr, JNIEnv * jni_env )
{
    OSL_ASSERT( sizeof (sal_Unicode) == sizeof (jchar) );
    jsize len = jni_env->GetStringLength( jstr );
    rtl_uString * ustr =
        (rtl_uString *)rtl_allocateMemory( sizeof (rtl_uString) + (len * sizeof (sal_Unicode)) );
    jni_env->GetStringRegion( jstr, 0, len, ustr->buffer );
    OSL_ASSERT( JNI_FALSE == jni_env->ExceptionCheck() );
    ustr->refCount = 1;
    ustr->length = len;
    ustr->buffer[ len ] = '\0';
    return ::rtl::OUString( ustr, SAL_NO_ACQUIRE );
}

}

//==================================================================================================
extern "C" SAL_DLLPUBLIC_EXPORT jobject JNICALL Java_com_sun_star_comp_helper_Bootstrap_cppuhelper_1bootstrap(
    JNIEnv * jni_env, SAL_UNUSED_PARAMETER jclass, jstring juno_rc, jobjectArray jpairs,
    jobject loader )
{
    try
    {
        if (0 != jpairs)
        {
            jsize nPos = 0, len = jni_env->GetArrayLength( jpairs );
            while (nPos < len)
            {
                // name
                jstring jstr = (jstring)jni_env->GetObjectArrayElement( jpairs, nPos );
                if (JNI_FALSE != jni_env->ExceptionCheck())
                {
                    jni_env->ExceptionClear();
                    throw RuntimeException(
                        OUSTR("index out of bounds?!"), Reference< XInterface >() );
                }
                if (0 != jstr)
                {
                    OUString name( ::javaunohelper::jstring_to_oustring( jstr, jni_env ) );
                    // value
                    jstr = (jstring)jni_env->GetObjectArrayElement( jpairs, nPos +1 );
                    if (JNI_FALSE != jni_env->ExceptionCheck())
                    {
                        jni_env->ExceptionClear();
                        throw RuntimeException(
                            OUSTR("index out of bounds?!"), Reference< XInterface >() );
                    }
                    if (0 != jstr)
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
        if (0 == juno_rc)
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
        OUString cpp_env_name = OUSTR(CPPU_CURRENT_LANGUAGE_BINDING_NAME);
        OUString java_env_name = OUSTR(UNO_LB_JAVA);
        Environment java_env, cpp_env;
        uno_getEnvironment((uno_Environment **)&cpp_env, cpp_env_name.pData, NULL);
        uno_getEnvironment( (uno_Environment **)&java_env, java_env_name.pData, vm_access.get() );

        // map to java
        Mapping mapping( cpp_env.get(), java_env.get() );
        if (! mapping.is())
        {
            Reference< lang::XComponent > xComp( xContext, UNO_QUERY );
            if (xComp.is())
                xComp->dispose();
            throw RuntimeException(
                OUSTR("cannot get mapping C++ <-> Java!"),
                Reference< XInterface >() );
        }

        jobject jret = (jobject)mapping.mapInterface( xContext.get(), ::getCppuType( &xContext ) );
        jobject jlocal = jni_env->NewLocalRef( jret );
        jni_env->DeleteGlobalRef( jret );

        return jlocal;
    }
    catch (const RuntimeException & exc)
    {
        jclass c = jni_env->FindClass( "com/sun/star/uno/RuntimeException" );
        if (0 != c)
        {
            OString cstr( ::rtl::OUStringToOString(
                              exc.Message, RTL_TEXTENCODING_JAVA_UTF8 ) );
            OSL_TRACE( __FILE__": forwarding RuntimeException: %s", cstr.getStr() );
            jni_env->ThrowNew( c, cstr.getStr() );
        }
    }
    catch (const Exception & exc)
    {
        jclass c = jni_env->FindClass( "com/sun/star/uno/Exception" );
        if (0 != c)
        {
            OString cstr( ::rtl::OUStringToOString(
                              exc.Message, RTL_TEXTENCODING_JAVA_UTF8 ) );
            OSL_TRACE( __FILE__": forwarding Exception: %s", cstr.getStr() );
            jni_env->ThrowNew( c, cstr.getStr() );
        }
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
