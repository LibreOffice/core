/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cppdef.h,v $
 * $Revision: 1.9 $
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
#define SYS_VMS                 2
#define SYS_RSX                 3
#define SYS_RT11                4
#define SYS_LATTICE             5
#define SYS_ONYX                6
#define SYS_68000               7

#ifndef HOST
#ifdef  unix
#define HOST                    SYS_UNIX
#else
#ifdef  vms
#define HOST                    SYS_VMS
#else
#ifdef  rsx
#define HOST                    SYS_RSX
#else
#ifdef  rt11
#define HOST                    SYS_RT11
#endif
#endif
#endif
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
 * FILE_LOCAL   marks functions which are referenced only in the
 *              file they reside.  Some C compilers allow these
 *              to be marked "static" even though they are referenced
 *              by "extern" statements elsewhere.
 *
 * OK_DOLLAR    Should be set TRUE if $ is a valid alphabetic character
 *              in identifiers (default), or zero if $ is invalid.
 *              Default is TRUE.
 *
 * OK_CONCAT    Should be set TRUE if # may be used to concatenate
 *              tokens in macros (per the Ansi Draft Standard) or
 *              FALSE for old-style # processing (needed if cpp is
 *              to process assembler source code).
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

#if TARGET == SYS_LATTICE
/*
 * We assume the operating system is pcdos for the IBM-PC.
 * We also assume the small model (just like the PDP-11)
 */
#define MACHINE                 "i8086"
#define SYSTEM                  "pcdos"
#endif

#if TARGET == SYS_ONYX
#define MACHINE                 "z8000"
#define SYSTEM                  "unix"
#endif

#if TARGET == SYS_VMS
#define MACHINE                 "vax"
#define SYSTEM                  "vms"
#define COMPILER                "vax11c"
#endif

#if TARGET == SYS_RSX
#define MACHINE                 "pdp11"
#define SYSTEM                  "rsx"
#define COMPILER                "decus"
#endif

#if TARGET == SYS_RT11
#define MACHINE                 "pdp11"
#define SYSTEM                  "rt11"
#define COMPILER                "decus"
#endif

#if TARGET == SYS_68000 || defined(M68000) || defined(m68000) || defined(m68k)
/*
 * All three machine designators have been seen in various systems.
 * Warning -- compilers differ as to sizeof (int).  cpp3 assumes that
 * sizeof (int) == 2
 */
#define MACHINE                 "M68000", "m68000", "m68k"
#define SYSTEM                  "unix"
#endif

#if     TARGET == SYS_UNIX
#define SYSTEM                  "unix"
#ifdef  pdp11
#define MACHINE                 "pdp11"
#endif
#ifdef  vax
#define MACHINE                 "vax"
#endif
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
 * OLD_PREPROCESSOR forces the definition of OK_DOLLAR, OK_CONCAT,
 * COMMENT_INVISIBLE, and STRING_FORMAL to values appropriate for
 * an old-style preprocessor.
 */

#ifndef OLD_PREPROCESSOR
#define OLD_PREPROCESSOR        FALSE
#endif

#if     OLD_PREPROCESSOR
#define OK_DOLLAR               FALSE
#define OK_CONCAT               TRUE
#define COMMENT_INVISIBLE       TRUE
#define STRING_FORMAL           TRUE
#define IDMAX                   63      /* actually, seems to be unlimited */
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
 * BIG_ENDIAN is set TRUE on machines (such as the IBM 360 series)
 * where 'ab' stores 'a' in the high-bits and 'b' in the low-bits.
 * It is set FALSE on machines (such as the PDP-11 and Vax-11)
 * where 'ab' stores 'a' in the low-bits and 'b' in the high-bits.
 * (Or is it the other way around?) -- Warning: BIG_ENDIAN code is untested.
 */
#ifndef BIG_ENDIAN
#define BIG_ENDIAN              FALSE
#endif

/*
 * COMMENT_INVISIBLE may be defined to allow "old-style" comment
 * processing, whereby the comment becomes a zero-length token
 * delimiter.  This permitted tokens to be concatenated in macro
 * expansions.  This was removed from the Draft Ansi Standard.
 */
#ifndef COMMENT_INVISIBLE
#define COMMENT_INVISIBLE       FALSE
#endif

/*
 * STRING_FORMAL may be defined to allow recognition of macro parameters
 * anywhere in replacement strings.  This was removed from the Draft Ansi
 * Standard and a limited recognition capability added.
 */
#ifndef STRING_FORMAL
#define STRING_FORMAL           FALSE
#endif

/*
 * OK_DOLLAR enables use of $ as a valid "letter" in identifiers.
 * This is a permitted extension to the Ansi Standard and is required
 * for e.g., VMS, RSX-11M, etc.   It should be set FALSE if cpp is
 * used to preprocess assembler source on Unix systems.  OLD_PREPROCESSOR
 * sets OK_DOLLAR FALSE for that reason.
 */
#ifndef OK_DOLLAR
#define OK_DOLLAR               TRUE
#endif

/*
 * OK_CONCAT enables (one possible implementation of) token concatenation.
 * If cpp is used to preprocess Unix assembler source, this should be
 * set FALSE as the concatenation character, #, is used by the assembler.
 */
#ifndef OK_CONCAT
#define OK_CONCAT               TRUE
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
/* ER 13.06.95 19:33
 da Makros im file->buffer expandiert werden, muss NBUFF mindestens NWORK sein
#define NWORK                   4096
#define NBUFF                   4096
 */
/* ER 13.06.95 20:05  NWORK wg. grooossen Makros in *.src erhoeht,
 da wir bald 10 Sprachen haben werden gleich ordentlich reingehauen.. */
#define NWORK                   128000
#define NBUFF                   NWORK
#define NFWORK                  1024
#define NEXP                    128
#define NINCLUDE        100
#define NPARMWORK               (NWORK * 2)
#define BLK_NEST                32


#ifndef ALERT
#ifdef EBCDIC
#define ALERT                   '\057'
#else
#define ALERT                   '\007'          /* '\a' is "Bell"       */
#endif
#endif

#ifndef VT
#define VT                      '\013'          /* Vertical Tab CTRL/K  */
#endif


#ifndef FILE_LOCAL
#ifdef  decus
#define FILE_LOCAL              static
#else
#ifdef  vax11c
#define FILE_LOCAL              static
#else
#define FILE_LOCAL                              /* Others are global    */
#endif
#endif
#endif

