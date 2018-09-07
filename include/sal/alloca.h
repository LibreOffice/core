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

#ifndef INCLUDED_SAL_ALLOCA_H
#define INCLUDED_SAL_ALLOCA_H

#if defined (__sun) || defined (LINUX) || defined(AIX) || defined(ANDROID) || defined(HAIKU)

#ifndef INCLUDED_ALLOCA_H
#include <alloca.h>
#define INCLUDED_ALLOCA_H
#endif

#elif defined (FREEBSD) || defined(NETBSD) || defined(OPENBSD) || defined(DRAGONFLY)

#ifndef INCLUDED_STDLIB_H
#include <stdlib.h>
#define INCLUDED_STDLIB_H
#endif

#elif defined (MACOSX)

#ifndef INCLUDED_SYS_TYPES_H
#include <sys/types.h>
#define INCLUDED_SYS_TYPES_H
#endif

#elif defined (IOS)

#ifndef INCLUDED_SYS_TYPES_H
#include <sys/types.h>
#define INCLUDED_SYS_TYPES_H
#endif

#elif defined (_WIN32)

#ifndef INCLUDED_MALLOC_H
#include <malloc.h>
#define INCLUDED_MALLOC_H
#endif

#else

#error "unknown platform: please check for alloca"

#endif

#endif  /* INCLUDED_SAL_ALLOCA_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
