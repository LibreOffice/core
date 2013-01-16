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


#include <vector>

#include "rtl/bootstrap.hxx"
#include "osl/diagnose.h"
#include "osl/process.h"
#include "cppuhelper/component_context.hxx"

#include "com/sun/star/uno/SecurityException.hpp"

#include "servicefactory_detail.hxx"

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace cppu
{

void add_access_control_entries( ::std::vector< ContextEntry_Init > * values )
    SAL_THROW( (Exception) )
{
    ContextEntry_Init entry;
    ::std::vector< ContextEntry_Init > & context_values = *values;

    // - ac prop: mode
    // { "off", "on", "dynamic-only", "single-user", "single-default-user" }
    entry.bLateInitService = false;
    entry.name = OUSTR("/services/com.sun.star.security.AccessController/mode");
    entry.value <<= OUSTR("off");
    context_values.push_back( entry );
    // - ac singleton
    entry.bLateInitService = true;
    entry.name = OUSTR("/singletons/com.sun.star.security.theAccessController");
    entry.value <<= OUSTR("com.sun.star.security.AccessController");
    context_values.push_back( entry );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
