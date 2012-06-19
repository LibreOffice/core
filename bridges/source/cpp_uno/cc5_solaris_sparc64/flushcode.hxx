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

#ifndef INCLUDED_BRIDGES_SOURCE_CPP_UNO_CC5_SOLARIS_SPARC64_FLUSHCODE_HXX
#define INCLUDED_BRIDGES_SOURCE_CPP_UNO_CC5_SOLARIS_SPARC64_FLUSHCODE_HXX

#include "sal/config.h"

extern "C" void sync_instruction_memory(caddr_t addr, int len); // from libc

namespace bridges { namespace cpp_uno { namespace cc5_solaris_sparc64 {

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
