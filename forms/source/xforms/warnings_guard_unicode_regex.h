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

#ifndef INCLUDED_WARNINGS_GUARD_UNICODE_REGEX_H
#define INCLUDED_WARNINGS_GUARD_UNICODE_REGEX_H

// Because the GCC system_header mechanism doesn't work in .c/.cxx compilation
// units and more important affects the rest of the current include file, the
// warnings guard is separated into this header file on its own.

// External unicode includes (from icu) cause warning C4668 on Windows.
// We want to minimize the patches to external headers, so the warnings are
// disabled here instead of in the header file itself.
#ifdef _MSC_VER
#pragma warning(push, 1)
#elif defined __GNUC__
#pragma GCC system_header
#endif
#include <unicode/regex.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // INCLUDED_WARNINGS_GUARD_UNICODE_REGEX_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
