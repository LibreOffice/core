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


#include "jni.h"

#include "uno/mapping.hxx"
#include "uno/environment.hxx"
#include "jvmaccess/virtualmachine.hxx"
#include "jvmaccess/unovirtualmachine.hxx"
#include "cppuhelper/implbase1.hxx"

#include "test/java_uno/anytest/XTransport.hpp"
#include "test/java_uno/anytest/DerivedInterface.hpp"


using namespace ::com::sun::star::uno;
using ::test::java_uno::anytest::XTransport;
using ::rtl::OUString;

namespace
{
//==================================================================================================
class Transport : public ::cppu::WeakImplHelper1< XTransport >
{
public:
    virtual Any SAL_CALL mapAny( Any const & any )
        throw (RuntimeException);
};
//__________________________________________________________________________________________________
Any Transport::mapAny( Any const & any )
    throw (RuntimeException)
{
    return any;
}
}

//##################################################################################################
extern "C" JNIEXPORT jobject JNICALL Java_test_java_1uno_anytest_TestJni_create_1jni_1transport(
    JNIEnv * jni_env, jclass, jobject loader )
    SAL_THROW_EXTERN_C()
{
    // publish some idl types
    ::getCppuType( (Reference< XTransport > const *)0 );
    ::getCppuType( (Reference< ::test::java_uno::anytest::DerivedInterface > const *)0 );

    Reference< XTransport > xRet( new Transport() );

    // get java vm
    JavaVM * java_vm;
    OSL_VERIFY( 0 == jni_env->GetJavaVM( &java_vm ) );
    // create jvmaccess vm
    ::rtl::Reference< ::jvmaccess::UnoVirtualMachine > vm;
    try {
        vm = new ::jvmaccess::UnoVirtualMachine(
            new ::jvmaccess::VirtualMachine(
                java_vm, JNI_VERSION_1_2, false, jni_env ),
            loader );
    } catch ( ::jvmaccess::UnoVirtualMachine::CreationException & ) {
        OSL_ASSERT( false );
        throw;
    }
    // create uno envs
    OUString java_name( UNO_LB_JAVA );
    OUString cpp_name( CPPU_CURRENT_LANGUAGE_BINDING_NAME );
    Environment java_env, cpp_env;
    uno_getEnvironment( (uno_Environment **)&java_env, java_name.pData, vm.get() );
    OSL_ASSERT( java_env.is() );
    uno_getEnvironment( (uno_Environment **)&cpp_env, cpp_name.pData, 0 );
    OSL_ASSERT( cpp_env.is() );

    // map interface
    Mapping mapping( cpp_env.get(), java_env.get() );
    OSL_ASSERT( mapping.is() );
    jobject jo_global = (jobject)mapping.mapInterface( xRet.get(), ::getCppuType( &xRet ) );
    OSL_ASSERT( 0 != jo_global );

    // return
    jobject jo_ret = jni_env->NewLocalRef( jo_global );
    jni_env->DeleteGlobalRef( jo_global );
    return jo_ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
