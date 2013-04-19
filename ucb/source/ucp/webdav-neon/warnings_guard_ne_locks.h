/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WARNINGS_GUARD_NE_LOCKS_H
#define INCLUDED_WARNINGS_GUARD_NE_LOCKS_H

// Because the GCC system_header mechanism doesn't work in .c/.cxx compilation
// units and more important affects the rest of the current include file, the
// warnings guard is separated into this header file on its own.

/*
    Silence down this WaE:
    /usr/include/neon/ne_locks.h:125:51: warning: 'int ne_lock(ne_session*, ne_lock*)'
    hides constructor for 'struct ne_lock' [-Wshadow]
*/

#ifdef _MSC_VER
#pragma warning(push, 1)
#elif defined __GNUC__
#pragma GCC system_header
#endif
#include <ne_locks.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // INCLUDED_WARNINGS_GUARD_NE_LOCKS_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
