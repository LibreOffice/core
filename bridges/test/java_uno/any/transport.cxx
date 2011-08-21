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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_bridges.hxx"

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
    OUString java_name( RTL_CONSTASCII_USTRINGPARAM(UNO_LB_JAVA) );
    OUString cpp_name( RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) );
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
