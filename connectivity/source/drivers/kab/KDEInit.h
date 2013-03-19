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

#ifndef CONNECTIVITY_KAB_KDEINIT_H
#define CONNECTIVITY_KAB_KDEINIT_H

// the address book driver's version
#define KAB_DRIVER_VERSION      "0.2"
#define KAB_DRIVER_VERSION_MAJOR    0
#define KAB_DRIVER_VERSION_MINOR    2

#include <config_vclplug.h>

#if ENABLE_TDE

// the minimum TDE version which is required at runtime
#define MIN_KDE_VERSION_MAJOR   14
#define MIN_KDE_VERSION_MINOR   0

#define MAX_KDE_VERSION_MAJOR   255
#define MAX_KDE_VERSION_MINOR   255

#else // ENABLE_TDE

// the minimum KDE version which is required at runtime
#define MIN_KDE_VERSION_MAJOR   3
#define MIN_KDE_VERSION_MINOR   2

#define MAX_KDE_VERSION_MAJOR   3
#define MAX_KDE_VERSION_MINOR   6

#endif // ENABLE_TDE


#endif // CONNECTIVITY_KAB_KDEINIT_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
