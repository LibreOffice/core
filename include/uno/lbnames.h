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
#ifndef INCLUDED_UNO_LBNAMES_H
#define INCLUDED_UNO_LBNAMES_H

/* I assume "LB" means "Language Binding" */

#include "sal/config.h"

#include "sal/macros.h"

#ifdef __cplusplus

#ifdef CPPU_ENV

/** Name for C++ compiler/ platform, e.g. "gcc3", "msci" */
#define CPPU_CURRENT_LANGUAGE_BINDING_NAME SAL_STRINGIFY( CPPU_ENV )

#else

#error "No supported C++ compiler environment."
provoking error here, because PP ignores #error

#endif /* CPPU_ENV */

#endif /* __cplusplus */

/** Environment type name for binary C UNO. */
#define UNO_LB_UNO "uno"
/** Environment type name for Java 1.3.1 compatible virtual machine. */
#define UNO_LB_JAVA "java"
/** Environment type name for CLI (Common Language Infrastructure). */
#define UNO_LB_CLI "cli"

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
