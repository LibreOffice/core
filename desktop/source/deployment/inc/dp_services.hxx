/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_SERVICES_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_SERVICES_HXX

#include <sal/config.h>

namespace comphelper { namespace service_decl { class ServiceDecl; } }

namespace dp_info {
extern comphelper::service_decl::ServiceDecl const serviceDecl;
}

namespace dp_log {
extern comphelper::service_decl::ServiceDecl const serviceDecl;
}

namespace dp_manager {

namespace factory {
extern comphelper::service_decl::ServiceDecl const serviceDecl;
}

extern comphelper::service_decl::ServiceDecl const serviceDecl;

}

namespace dp_registry { namespace backend {

namespace component {
extern comphelper::service_decl::ServiceDecl const serviceDecl;
}

namespace configuration {
extern comphelper::service_decl::ServiceDecl const serviceDecl;
}

namespace executable {
extern comphelper::service_decl::ServiceDecl const serviceDecl;
}

namespace help {
extern comphelper::service_decl::ServiceDecl const serviceDecl;
}

namespace script {
extern comphelper::service_decl::ServiceDecl const serviceDecl;
}

namespace sfwk {
extern comphelper::service_decl::ServiceDecl const serviceDecl;
}

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
