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



#ifndef INCLUDED_JAVAUNOHELPER_SOURCE_VM_HXX
#define INCLUDED_JAVAUNOHELPER_SOURCE_VM_HXX

#include "sal/config.h"

#include "jni.h"
#include "com/sun/star/uno/Reference.hxx"
#include "rtl/ref.hxx"

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }
namespace jvmaccess { class UnoVirtualMachine; }

namespace javaunohelper {

::rtl::Reference< ::jvmaccess::UnoVirtualMachine > create_vm_access(
    JNIEnv * jni_env, jobject loader );

::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
install_vm_singleton(
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
    const & xContext,
    ::rtl::Reference< ::jvmaccess::UnoVirtualMachine > const & vm_access );

}

#endif
