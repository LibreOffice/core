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

#ifndef INCLUDED_BRIDGES_SOURCE_CPP_UNO_CC50_SOLARIS_SPARC_FLUSHCODE_HXX
#define INCLUDED_BRIDGES_SOURCE_CPP_UNO_CC50_SOLARIS_SPARC_FLUSHCODE_HXX

#include "sal/config.h"

extern "C" void sync_instruction_memory(caddr_t addr, int len); // from libc

namespace bridges { namespace cpp_uno { namespace cc50_solaris_sparc {

/**
 * Flush a region of memory into which code has been written dynamically.
 */
inline void flushCode(void const * begin, void const * end) {
    sync_instruction_memory(
        static_cast< caddr_t >(const_cast< void * >(begin)),
        static_cast< char const * >(end) - static_cast< char const * >(begin));
}

} } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
