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

