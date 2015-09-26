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


#include <comphelper/servicedecl.hxx>

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
}
extern sdecl::ServiceDecl const serviceDecl;
}

namespace dp_log {
extern sdecl::ServiceDecl const serviceDecl;
}

namespace dp_info {
extern sdecl::ServiceDecl const serviceDecl;
}

extern "C" {

SAL_DLLPUBLIC_EXPORT void * SAL_CALL deployment_component_getFactory(
    sal_Char const * pImplName, void *, void *)
{
    return component_getFactoryHelper(
        pImplName,
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
