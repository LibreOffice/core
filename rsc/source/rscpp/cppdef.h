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

#ifndef INCLUDED_RSC_SOURCE_RSCPP_CPPDEF_H
#define INCLUDED_RSC_SOURCE_RSCPP_CPPDEF_H

/*
 * This redundant definition of TRUE and FALSE works around
 * a limitation of Decus C.
 */
#ifndef TRUE
#define TRUE                    1
#define FALSE                   0
#endif

/*
 * Define the HOST operating system.  This is needed so that
 * cpp can use appropriate filename conventions.
 */
#define SYS_UNKNOWN             0
#define SYS_UNIX                1

#ifndef HOST
#ifdef  unix
#define HOST                    SYS_UNIX
#else
#endif
#endif

#ifndef HOST
#define HOST                    SYS_UNKNOWN
#endif

/*
 * We assume that the target is the same as the host system
 */
#ifndef TARGET
#define TARGET                  HOST
#endif

/*
 * In order to predefine machine-dependent constants,
 * several strings are defined here:
 *
 * MACHINE      defines the target cpu (by name)
 * SYSTEM       defines the target operating system
 * COMPILER     defines the target compiler
 *
 *      The above may be #defined as "" if they are not wanted.
 *      They should not be #defined as NULL.
 *
 * LINE_PREFIX  defines the # output line prefix, if not "line"
 *              This should be defined as "" if cpp is to replace
 *              the "standard" C pre-processor.
 *
 * OK_DATE      Predefines the compilation date if set TRUE.
 *              Not permitted by the Nov. 12, 1984 Draft Standard.
 *
 * S_CHAR etc.  Define the sizeof the basic TARGET machine word types.
 *              By default, sizes are set to the values for the HOST
 *              computer.  If this is inappropriate, see the code in
 *              cpp3.c for details on what to change.  Also, if you
 *              have a machine where sizeof (signed int) differs from
 *              sizeof (unsigned int), you will have to edit code and
 *              tables in cpp3.c (and extend the -S option definition.)
 *
 * CPP_LIBRARY  May be defined if you have a site-specific include directory
 *              which is to be searched *before* the operating-system
 *              specific directories.
 */

#if     TARGET == SYS_UNIX
#define SYSTEM                  "unix"
#endif

/*
 * defaults
 */

#ifndef MSG_PREFIX
#define MSG_PREFIX              "cpp: "
#endif

#ifndef LINE_PREFIX
#define LINE_PREFIX             ""
#endif

/*
 * RECURSION_LIMIT may be set to -1 to disable the macro recursion test.
 */
#ifndef RECURSION_LIMIT
#define RECURSION_LIMIT 1000
#endif

/*
 * BITS_CHAR may be defined to set the number of bits per character.
 * it is needed only for multi-byte character constants.
 */
#ifndef BITS_CHAR
#define BITS_CHAR               8
#endif

/*
 * OK_DATE may be enabled to predefine today's date as a string
 * at the start of each compilation.  This is apparently not permitted
 * by the Draft Ansi Standard.
 */
#ifndef OK_DATE
#define OK_DATE         TRUE
#endif

/*
 * The following definitions are used to allocate memory for
 * work buffers.  In general, they should not be modified
 * by implementors.
 *
 * PAR_MAC      The maximum number of #define parameters (31 per Standard)
 *              Note: we need another one for strings.
 * IDMAX        The longest identifier, 31 per Ansi Standard
 * NBUFF        Input buffer size
 * NWORK        Work buffer size -- the longest macro
 *              must fit here after expansion.
 * NEXP         The nesting depth of #if expressions
 * NINCLUDE     The number of directories that may be specified
 *              on a per-system basis, or by the -I option.
 * BLK_NEST     The number of nested #if's permitted.
 * NFWORK       FileNameWorkBuffer (added by erAck, was NWORK)
 */

#ifndef IDMAX
#define IDMAX                    127
#endif
#ifdef SOLAR
#define PAR_MAC           (253 + 1)
#else
#define PAR_MAC            (31 + 1)
#endif
/* NWORK wg. grooossen Makros in *.src erhoeht,
 da wir bald 10 Sprachen haben werden gleich ordentlich reingehauen.. */
#define NWORK                   128000
#define NBUFF                   NWORK
#define NFWORK                  1024
#define NEXP                    128
#define NINCLUDE        100
#define NPARMWORK               (NWORK * 2)
#define BLK_NEST                32


#ifndef ALERT
#define ALERT                   '\007'          /* '\a' is "Bell"       */
#endif

#ifndef VT
#define VT                      '\013'          /* Vertical Tab CTRL/K  */
#endif


#endif // INCLUDED_RSC_SOURCE_RSCPP_CPPDEF_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
