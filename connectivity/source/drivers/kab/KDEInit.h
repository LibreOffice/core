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

#ifndef CONNECTIVITY_KAB_KDEINIT_H
#define CONNECTIVITY_KAB_KDEINIT_H

// the address book driver's version
#define KAB_DRIVER_VERSION      "0.2"
#define KAB_DRIVER_VERSION_MAJOR    0
#define KAB_DRIVER_VERSION_MINOR    2

#ifdef ENABLE_TDE

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
