/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
