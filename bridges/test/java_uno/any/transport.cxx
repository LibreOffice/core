/*************************************************************************
 *
 *  $RCSfile: transport.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-18 19:07:19 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "jni.h"

#include "uno/mapping.hxx"
#include "uno/environment.hxx"
#include "jvmaccess/virtualmachine.hxx"
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
    JNIEnv * jni_env, jclass )
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
    ::rtl::Reference< ::jvmaccess::VirtualMachine > vm(
        new ::jvmaccess::VirtualMachine( java_vm, JNI_VERSION_1_2, false, jni_env ) );
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
