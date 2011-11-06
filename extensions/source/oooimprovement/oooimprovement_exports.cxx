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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#include "corecontroller.hxx"
#include "invite_job.hxx"
#include "onlogrotate_job.hxx"
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <cppuhelper/factory.hxx>
#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>


using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::uno;
using namespace ::oooimprovement;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;


namespace
{
    void writeInfo(const Reference<XRegistryKey>& reg_key,
        const OUString& implementation_name,
        const OUString& service_name)
    {
        OUStringBuffer buf(256);
        buf.append(implementation_name);
        buf.appendAscii("/UNO/SERVICES/");
        buf.append(service_name);
        reg_key->createKey(buf.makeStringAndClear());
    }
}

extern "C"
{
    void SAL_CALL component_getImplementationEnvironment(const sal_Char** env_type_name, uno_Environment**)
    { *env_type_name = CPPU_CURRENT_LANGUAGE_BINDING_NAME; }

    void* SAL_CALL component_getFactory(const sal_Char* pImplName, void* pServiceManager, void*)
    {
        if ( !pServiceManager || !pImplName ) return 0;

        Reference<XSingleServiceFactory> factory;
        Reference<XMultiServiceFactory>  sm(reinterpret_cast<XMultiServiceFactory*>(pServiceManager), UNO_QUERY);
        OUString impl_name = OUString::createFromAscii(pImplName);
        Sequence<OUString> names(1);
        names[0] = impl_name;

        if (impl_name.equals(CoreController::getImplementationName_static()))
            factory = ::cppu::createSingleFactory(sm, impl_name, CoreController::Create, names);
        if (impl_name.equals(OnLogRotateJob::getImplementationName_static()))
            factory = ::cppu::createSingleFactory(sm, impl_name, OnLogRotateJob::Create, names);
        if (impl_name.equals(InviteJob::getImplementationName_static()))
            factory = ::cppu::createSingleFactory(sm, impl_name, InviteJob::Create, names);
        if (!factory.is()) return 0;
        factory->acquire();
        return factory.get();
    }
}
