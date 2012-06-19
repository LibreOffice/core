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

#ifndef INCLUDED_BRIDGES_SOURCE_CPP_UNO_CC5_SOLARIS_SPARC64_EXCEPTIONS_HXX
#define INCLUDED_BRIDGES_SOURCE_CPP_UNO_CC5_SOLARIS_SPARC64_EXCEPTIONS_HXX

#include "sal/config.h"

#include <cstddef>

#include "typelib/typedescription.h"
#include "uno/any2.h"
#include "uno/mapping.h"

// Private CC5 structures and functions:
namespace __Crun {
    struct class_base_descr {
        int type_hash[4];
        std::size_t offset;
    };
    struct static_type_info {
        std::ptrdiff_t ty_name;
        std::ptrdiff_t reserved;
        std::ptrdiff_t base_table;
        int type_hash[4];
        unsigned int flags;
        unsigned int cv_qualifiers;
    };
    void * ex_alloc(unsigned long);
    void ex_throw(void *, static_type_info const *, void (*)(void *));
    void * ex_get();
    void ex_rethrow_q() throw ();
}
namespace __Cimpl {
    char const * ex_name();
}

namespace bridges { namespace cpp_uno { namespace cc5_solaris_sparc64 {

void raiseException(uno_Any * exception, uno_Mapping * unoToCpp);

void fillUnoException(
    void * cppException, char const * cppName, uno_Any * unoException,
    uno_Mapping * cppToUno);

} } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
