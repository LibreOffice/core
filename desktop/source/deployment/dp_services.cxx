/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_services.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-19 13:08:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#define COMPHELPER_SERVICEDECL_COMPONENT_HELPER_MAX_ARGS 10
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

} // namespace backend
} // namespace dp_registry

namespace dp_manager {
namespace factory {
extern sdecl::ServiceDecl const serviceDecl;
bool singleton_entries( uno::Reference<registry::XRegistryKey> const& );
}
}

namespace dp_log {
extern sdecl::ServiceDecl const serviceDecl;
}

namespace dp_migration {
extern sdecl::ServiceDecl const serviceDecl;
}

namespace dp_info {
extern sdecl::ServiceDecl const serviceDecl;
bool singleton_entries( uno::Reference<registry::XRegistryKey> const& );
}

extern "C" {

struct uno_Environment;

void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

sal_Bool SAL_CALL component_writeInfo(
    lang::XMultiServiceFactory * pServiceManager,
    registry::XRegistryKey * pRegistryKey )
{
    return component_writeInfoHelper(
        pServiceManager, pRegistryKey,
        dp_registry::backend::configuration::serviceDecl,
        dp_registry::backend::component::serviceDecl,
        dp_registry::backend::help::serviceDecl,
        dp_registry::backend::script::serviceDecl,
        dp_registry::backend::sfwk::serviceDecl,
        dp_manager::factory::serviceDecl,
        dp_log::serviceDecl,
        dp_migration::serviceDecl,
        dp_info::serviceDecl ) &&
        dp_manager::factory::singleton_entries( pRegistryKey ) &&
        dp_info::singleton_entries( pRegistryKey );
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
        dp_manager::factory::serviceDecl,
        dp_log::serviceDecl,
        dp_migration::serviceDecl,
        dp_info::serviceDecl );
}

} // extern "C"

