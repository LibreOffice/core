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

#ifndef _VCLPLUGINAPI_H
#define _VCLPLUGINAPI_H

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
