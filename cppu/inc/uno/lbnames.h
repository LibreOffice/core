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
#ifndef _UNO_LBNAMES_H_
#define _UNO_LBNAMES_H_

#ifdef __cplusplus

#ifdef CPPU_ENV

#define CPPU_STRINGIFY_EX( x ) #x
#define CPPU_STRINGIFY( x ) CPPU_STRINGIFY_EX( x )

/** Name for C++ compiler/ platform, e.g. "gcc3", "msci" */
#define CPPU_CURRENT_LANGUAGE_BINDING_NAME CPPU_STRINGIFY( CPPU_ENV )

#else

#error "No supported C++ compiler environment."
provoking error here, because PP ignores #error

#endif /* CPPU_ENV */

#endif /* __cplusplus */

/** Environment type name for binary C UNO. */
#define UNO_LB_UNO "uno"
/** Environment type name for ANSI C compilers. */
#define UNO_LB_C "c"
/** Environment type name for Java 1.3.1 compatible virtual machine. */
#define UNO_LB_JAVA "java"
/** Environment type name for CLI (Common Language Infrastructure). */
#define UNO_LB_CLI "cli"

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
