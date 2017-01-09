/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_SOURCE_UI_VBA_SERVICE_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_SERVICE_HXX

#include <sal/config.h>

namespace comphelper { namespace service_decl { class ServiceDecl; } }

namespace document {
extern comphelper::service_decl::ServiceDecl const serviceDecl;
}

namespace globals {
extern comphelper::service_decl::ServiceDecl const serviceDecl;
}

namespace vbaeventshelper {
extern comphelper::service_decl::ServiceDecl const serviceDecl;
}

namespace wrapformat {
extern comphelper::service_decl::ServiceDecl const serviceDecl;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
