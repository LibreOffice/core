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

#ifndef INCLUDED_VCL_INC_VCLPLUGINAPI_H
#define INCLUDED_VCL_INC_VCLPLUGINAPI_H

#include "sal/config.h"
#include "sal/types.h"

#if defined VCLPLUG_GEN_IMPLEMENTATION
#define VCLPLUG_GEN_PUBLIC SAL_DLLPUBLIC_EXPORT
#else
#define VCLPLUG_GEN_PUBLIC SAL_DLLPUBLIC_IMPORT
#endif

#if defined VCLPLUG_GTK_IMPLEMENTATION
#define VCLPLUG_GTK_PUBLIC SAL_DLLPUBLIC_EXPORT
#else
#define VCLPLUG_GTK_PUBLIC SAL_DLLPUBLIC_IMPORT
#endif

#if defined VCLPLUG_TDE_IMPLEMENTATION
#define VCLPLUG_TDE_PUBLIC SAL_DLLPUBLIC_EXPORT
#else
#define VCLPLUG_TDE_PUBLIC SAL_DLLPUBLIC_IMPORT
#endif

#if defined VCLPLUG_KDE_IMPLEMENTATION
#define VCLPLUG_KDE_PUBLIC SAL_DLLPUBLIC_EXPORT
#else
#define VCLPLUG_KDE_PUBLIC SAL_DLLPUBLIC_IMPORT
#endif

#if defined VCLPLUG_KDE4_IMPLEMENTATION
#define VCLPLUG_KDE4_PUBLIC SAL_DLLPUBLIC_EXPORT
#else
#define VCLPLUG_KDE4_PUBLIC SAL_DLLPUBLIC_IMPORT
#endif

#if defined VCLPLUG_SVP_IMPLEMENTATION
#define VCLPLUG_SVP_PUBLIC SAL_DLLPUBLIC_EXPORT
#else
#define VCLPLUG_SVP_PUBLIC SAL_DLLPUBLIC_IMPORT
#endif

#if defined DESKTOP_DETECTOR_IMPLEMENTATION
#define DESKTOP_DETECTOR_PUBLIC SAL_DLLPUBLIC_EXPORT
#else
#define DESKTOP_DETECTOR_PUBLIC SAL_DLLPUBLIC_IMPORT
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
