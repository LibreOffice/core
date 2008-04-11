/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: flushcode.cxx,v $
 * $Revision: 1.5 $
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
#include "precompiled_bridges.hxx"

#include "sal/config.h"

#include "flushcode.hxx"

extern "C" void doFlushCode(unsigned long address, unsigned long count);

namespace bridges { namespace cpp_uno { namespace cc50_solaris_sparc {

void flushCode(void const * begin, void const * end) {
    unsigned long n =
        static_cast< char const * >(end) - static_cast< char const * >(begin);
    if (n != 0) {
        unsigned long adr = reinterpret_cast< unsigned long >(begin);
        unsigned long off = adr & 7;
        doFlushCode(adr - off, (n + off + 7) >> 3);
    }
}

} } }
