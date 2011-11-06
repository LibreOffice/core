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
#include "precompiled_desktop.hxx"

#define COMPHELPER_SERVICEDECL_COMPONENT_HELPER_MAX_ARGS 12
#include "comphelper/servicedecl.hxx"

using namespace com::sun::star;
namespace sdecl = comphelper::service_decl;

namespace dp_registry {
namespace backend {

namespace configuration {
extern sdecl::ServiceDecl const serviceDecl;
}

namespace component {
extern sdecl::ServiceDecl const serviceDecl;
}

namespace script {
extern sdecl::ServiceDecl const serviceDecl;
}

namespace sfwk {
extern sdecl::ServiceDecl const serviceDecl;
}

namespace help {
extern sdecl::ServiceDecl const serviceDecl;
}

namespace executable {
extern sdecl::ServiceDecl const serviceDecl;
}

} // namespace backend
} // namespace dp_registry

namespace dp_manager {
namespace factory {
extern sdecl::ServiceDecl const serviceDecl;
bool singleton_entries( uno::Reference<registry::XRegistryKey> const& );
}
extern sdecl::ServiceDecl const serviceDecl;
bool singleton_entries( uno::Reference<registry::XRegistryKey> const& );
}

namespace dp_log {
extern sdecl::ServiceDecl const serviceDecl;
}

namespace dp_info {
extern sdecl::ServiceDecl const serviceDecl;
bool singleton_entries( uno::Reference<registry::XRegistryKey> const& );
}

extern "C" {

void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

void * SAL_CALL component_getFactory(
    sal_Char const * pImplName,
    lang::XMultiServiceFactory * pServiceManager,
    registry::XRegistryKey * pRegistryKey )
{
    return component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey,
        dp_registry::backend::configuration::serviceDecl,
        dp_registry::backend::component::serviceDecl,
        dp_registry::backend::help::serviceDecl,
        dp_registry::backend::script::serviceDecl,
        dp_registry::backend::sfwk::serviceDecl,
        dp_registry::backend::executable::serviceDecl,
        dp_manager::factory::serviceDecl,
        dp_log::serviceDecl,
        dp_info::serviceDecl,
        dp_manager::serviceDecl);
}

} // extern "C"

