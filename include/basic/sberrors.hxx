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

#ifndef INCLUDED_BASIC_SBERRORS_HXX
#define INCLUDED_BASIC_SBERRORS_HXX

#include <basic/sbxdef.hxx>

#ifndef __RSC
typedef sal_uIntPtr SbError;
#endif

// Mapping to SbxError
#define ERRCODE_BASIC_SYNTAX                ERRCODE_SBX_SYNTAX                  // unspecified syntax error
#define ERRCODE_BASIC_BAD_ARGUMENT          ERRCODE_SBX_NOTIMP                  // Invalid procedure call
#define ERRCODE_BASIC_MATH_OVERFLOW         ERRCODE_SBX_OVERFLOW                // Overflow
#define ERRCODE_BASIC_OUT_OF_RANGE          ERRCODE_SBX_BOUNDS                  // Subscript out of range
#define ERRCODE_BASIC_ZERODIV               ERRCODE_SBX_ZERODIV                 // Division by zero
#define ERRCODE_BASIC_CONVERSION            ERRCODE_SBX_CONVERSION              // Type mismatch
#define ERRCODE_BASIC_BAD_PARAMETER         ERRCODE_SBX_BAD_PARAMETER           // Invalid Parameter
#define ERRCODE_BASIC_PROC_UNDEFINED        ERRCODE_SBX_PROC_UNDEFINED          // Sub or Function not defined
#define ERRCODE_BASIC_INTERNAL_ERROR        ERRCODE_SBX_ERROR                   // internal error
#define ERRCODE_BASIC_NO_OBJECT             ERRCODE_SBX_NO_OBJECT               // Object variable not set
#define ERRCODE_BASIC_CANNOT_LOAD           ERRCODE_SBX_CANNOT_LOAD             // Can't load module
#define ERRCODE_BASIC_BAD_INDEX             ERRCODE_SBX_BAD_INDEX               // Invalid object index
#define ERRCODE_BASIC_NO_ACTIVE_OBJECT      ERRCODE_SBX_NO_ACTIVE_OBJECT        // No active view or document
#define ERRCODE_BASIC_BAD_PROP_VALUE        ERRCODE_SBX_BAD_PROP_VALUE          // Bad property value
#define ERRCODE_BASIC_PROP_READONLY         ERRCODE_SBX_PROP_READONLY           // Property is read only
#define ERRCODE_BASIC_PROP_WRITEONLY        ERRCODE_SBX_PROP_WRITEONLY          // Property is write only
#define ERRCODE_BASIC_INVALID_OBJECT        ERRCODE_SBX_INVALID_OBJECT          // Invalid object reference
#define ERRCODE_BASIC_NO_METHOD             ERRCODE_SBX_NO_METHOD               // Property or method not found
#define ERRCODE_BASIC_INVALID_USAGE_OBJECT  ERRCODE_SBX_INVALID_USAGE_OBJECT    // Invalid use of object
#define ERRCODE_BASIC_NO_OLE                ERRCODE_SBX_NO_OLE                  // Class does not support OLE
#define ERRCODE_BASIC_BAD_METHOD            ERRCODE_SBX_BAD_METHOD              // Object doesn't support method
#define ERRCODE_BASIC_OLE_ERROR             ERRCODE_SBX_OLE_ERROR               // OLE Automation error
#define ERRCODE_BASIC_BAD_ACTION            ERRCODE_SBX_BAD_ACTION              // Object doesn't support this action
#define ERRCODE_BASIC_NO_NAMED_ARGS         ERRCODE_SBX_NO_NAMED_ARGS           // Object doesn't support named args
#define ERRCODE_BASIC_BAD_LOCALE            ERRCODE_SBX_BAD_LOCALE              // Object doesn't support current locale setting
#define ERRCODE_BASIC_NAMED_NOT_FOUND       ERRCODE_SBX_NAMED_NOT_FOUND         // Named argument not found
#define ERRCODE_BASIC_NOT_OPTIONAL          ERRCODE_SBX_NOT_OPTIONAL            // Argument not optional
#define ERRCODE_BASIC_WRONG_ARGS            ERRCODE_SBX_WRONG_ARGS              // Wrong number of arguments
#define ERRCODE_BASIC_NOT_A_COLL            ERRCODE_SBX_NOT_A_COLL              // Object not a collection

// Append Basic specific error messages to ERRCODE_AREA_SBX
#define ERRCODE_BASIC_NO_GOSUB              ((LAST_SBX_ERROR_ID+1UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Return without Gosub
#define ERRCODE_BASIC_REDO_FROM_START       ((LAST_SBX_ERROR_ID+2UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Redo form start (SB internal)
#define ERRCODE_BASIC_NO_MEMORY             ((LAST_SBX_ERROR_ID+3UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Out of memory
#define ERRCODE_BASIC_ALREADY_DIM           ((LAST_SBX_ERROR_ID+4UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Array already dimensioned
#define ERRCODE_BASIC_DUPLICATE_DEF         ((LAST_SBX_ERROR_ID+5UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Duplicate definition
#define ERRCODE_BASIC_VAR_UNDEFINED         ((LAST_SBX_ERROR_ID+6UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)          // Variable undefined (SB)
#define ERRCODE_BASIC_USER_ABORT            ((LAST_SBX_ERROR_ID+7UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // User interrupt occurred
#define ERRCODE_BASIC_BAD_RESUME            ((LAST_SBX_ERROR_ID+8UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Resume without error
#define ERRCODE_BASIC_STACK_OVERFLOW        ((LAST_SBX_ERROR_ID+9UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Out of stack space
#define ERRCODE_BASIC_BAD_DLL_LOAD          ((LAST_SBX_ERROR_ID+10UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Error in loading DLL
#define ERRCODE_BASIC_BAD_DLL_CALL          ((LAST_SBX_ERROR_ID+11UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Bad DLL calling convention
#define ERRCODE_BASIC_BAD_CHANNEL           ((LAST_SBX_ERROR_ID+12UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Bad file name or number
#define ERRCODE_BASIC_FILE_NOT_FOUND        ((LAST_SBX_ERROR_ID+13UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // File not found
#define ERRCODE_BASIC_BAD_FILE_MODE         ((LAST_SBX_ERROR_ID+14UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Bad file mode
#define ERRCODE_BASIC_FILE_ALREADY_OPEN     ((LAST_SBX_ERROR_ID+15UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // File already open
#define ERRCODE_BASIC_IO_ERROR              ((LAST_SBX_ERROR_ID+16UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)          // Device I/O error
#define ERRCODE_BASIC_FILE_EXISTS           ((LAST_SBX_ERROR_ID+17UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // File already exists
#define ERRCODE_BASIC_BAD_RECORD_LENGTH     ((LAST_SBX_ERROR_ID+18UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // bad record length
#define ERRCODE_BASIC_DISK_FULL             ((LAST_SBX_ERROR_ID+19UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // disk full
#define ERRCODE_BASIC_READ_PAST_EOF         ((LAST_SBX_ERROR_ID+20UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Input past end of file
#define ERRCODE_BASIC_BAD_RECORD_NUMBER     ((LAST_SBX_ERROR_ID+21UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Bad record number
#define ERRCODE_BASIC_TOO_MANY_FILES        ((LAST_SBX_ERROR_ID+22UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)          // Too many files
#define ERRCODE_BASIC_NO_DEVICE             ((LAST_SBX_ERROR_ID+23UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Device not available
#define ERRCODE_BASIC_ACCESS_DENIED         ((LAST_SBX_ERROR_ID+24UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Permission denied
#define ERRCODE_BASIC_NOT_READY             ((LAST_SBX_ERROR_ID+25UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Disk not ready
#define ERRCODE_BASIC_NOT_IMPLEMENTED       ((LAST_SBX_ERROR_ID+26UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Feature not implemented
#define ERRCODE_BASIC_DIFFERENT_DRIVE       ((LAST_SBX_ERROR_ID+27UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // No rename with different drive
#define ERRCODE_BASIC_ACCESS_ERROR          ((LAST_SBX_ERROR_ID+28UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Path/File access error
#define ERRCODE_BASIC_PATH_NOT_FOUND        ((LAST_SBX_ERROR_ID+29UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Path not found
#define ERRCODE_BASIC_BAD_PATTERN           ((LAST_SBX_ERROR_ID+30UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Invalid pattern string
#define ERRCODE_BASIC_IS_NULL               ((LAST_SBX_ERROR_ID+31UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Invalid use of Null

// DDE messages from 250-299
#define ERRCODE_BASIC_DDE_ERROR             ((LAST_SBX_ERROR_ID+32UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_WAITINGACK        ((LAST_SBX_ERROR_ID+33UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_OUTOFCHANNELS     ((LAST_SBX_ERROR_ID+34UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_NO_RESPONSE       ((LAST_SBX_ERROR_ID+35UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_MULT_RESPONSES    ((LAST_SBX_ERROR_ID+36UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_CHANNEL_LOCKED    ((LAST_SBX_ERROR_ID+37UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_NOTPROCESSED      ((LAST_SBX_ERROR_ID+38UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_TIMEOUT           ((LAST_SBX_ERROR_ID+39UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_USER_INTERRUPT    ((LAST_SBX_ERROR_ID+40UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_BUSY              ((LAST_SBX_ERROR_ID+41UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_NO_DATA           ((LAST_SBX_ERROR_ID+42UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_WRONG_DATA_FORMAT ((LAST_SBX_ERROR_ID+43UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_PARTNER_QUIT      ((LAST_SBX_ERROR_ID+44UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_CONV_CLOSED       ((LAST_SBX_ERROR_ID+45UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_NO_CHANNEL        ((LAST_SBX_ERROR_ID+46UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_INVALID_LINK      ((LAST_SBX_ERROR_ID+47UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_QUEUE_OVERFLOW    ((LAST_SBX_ERROR_ID+48UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_LINK_ALREADY_EST  ((LAST_SBX_ERROR_ID+49UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_LINK_INV_TOPIC    ((LAST_SBX_ERROR_ID+50UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_DLL_NOT_FOUND     ((LAST_SBX_ERROR_ID+51UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)

#define ERRCODE_BASIC_NEEDS_OBJECT          ((LAST_SBX_ERROR_ID+52UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Object required
#define ERRCODE_BASIC_BAD_ORDINAL           ((LAST_SBX_ERROR_ID+53UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Invalid ordinal
#define ERRCODE_BASIC_DLLPROC_NOT_FOUND     ((LAST_SBX_ERROR_ID+54UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Specified DLL function not found
#define ERRCODE_BASIC_BAD_CLIPBD_FORMAT     ((LAST_SBX_ERROR_ID+55UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Invalid clipboard format

// Debugger messages from 700-799

#define ERRCODE_BASIC_PROPERTY_NOT_FOUND    ((LAST_SBX_ERROR_ID+56UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Class not have property
#define ERRCODE_BASIC_METHOD_NOT_FOUND      ((LAST_SBX_ERROR_ID+57UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Class does not have method
#define ERRCODE_BASIC_ARG_MISSING           ((LAST_SBX_ERROR_ID+58UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Missing required argument
#define ERRCODE_BASIC_BAD_NUMBER_OF_ARGS    ((LAST_SBX_ERROR_ID+59UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Bad number of arguments
#define ERRCODE_BASIC_METHOD_FAILED         ((LAST_SBX_ERROR_ID+60UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Method failed
#define ERRCODE_BASIC_SETPROP_FAILED        ((LAST_SBX_ERROR_ID+61UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Unable to set property
#define ERRCODE_BASIC_GETPROP_FAILED        ((LAST_SBX_ERROR_ID+62UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Unable to get property

// Compiler Errors (do not occur at runtime)
// These IDs can shift at any time

#define ERRCODE_BASIC_UNEXPECTED            ((LAST_SBX_ERROR_ID+63UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Unexpected symbol: xx
#define ERRCODE_BASIC_EXPECTED              ((LAST_SBX_ERROR_ID+64UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Expected: xx
#define ERRCODE_BASIC_SYMBOL_EXPECTED       ((LAST_SBX_ERROR_ID+65UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Symbol expected
#define ERRCODE_BASIC_VAR_EXPECTED          ((LAST_SBX_ERROR_ID+66UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Variable expected
#define ERRCODE_BASIC_LABEL_EXPECTED        ((LAST_SBX_ERROR_ID+67UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Label expected
#define ERRCODE_BASIC_LVALUE_EXPECTED       ((LAST_SBX_ERROR_ID+68UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Lvalue expected
#define ERRCODE_BASIC_VAR_DEFINED           ((LAST_SBX_ERROR_ID+69UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Variable xxx already defined
#define ERRCODE_BASIC_PROC_DEFINED          ((LAST_SBX_ERROR_ID+70UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Procedure xx already defined
#define ERRCODE_BASIC_LABEL_DEFINED         ((LAST_SBX_ERROR_ID+71UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Label xxx already defined
#define ERRCODE_BASIC_UNDEF_VAR             ((LAST_SBX_ERROR_ID+72UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Variable xx undefined
#define ERRCODE_BASIC_UNDEF_ARRAY           ((LAST_SBX_ERROR_ID+73UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Array or function xx undefined
#define ERRCODE_BASIC_UNDEF_PROC            ((LAST_SBX_ERROR_ID+74UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Procedure xxx undefined
#define ERRCODE_BASIC_UNDEF_LABEL           ((LAST_SBX_ERROR_ID+75UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Label xxx undefined
#define ERRCODE_BASIC_UNDEF_TYPE            ((LAST_SBX_ERROR_ID+76UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Unknown user defined type xxx
#define ERRCODE_BASIC_BAD_EXIT              ((LAST_SBX_ERROR_ID+77UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Exit XXX expected
#define ERRCODE_BASIC_BAD_BLOCK             ((LAST_SBX_ERROR_ID+78UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Unterminated statement block: missing XX
#define ERRCODE_BASIC_BAD_BRACKETS          ((LAST_SBX_ERROR_ID+79UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Parentheses do not match
#define ERRCODE_BASIC_BAD_DECLARATION       ((LAST_SBX_ERROR_ID+80UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Symbol xx defined differently
#define ERRCODE_BASIC_BAD_PARAMETERS        ((LAST_SBX_ERROR_ID+81UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Parameters do not match
#define ERRCODE_BASIC_BAD_CHAR_IN_NUMBER    ((LAST_SBX_ERROR_ID+82UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Bad character in number
#define ERRCODE_BASIC_MUST_HAVE_DIMS        ((LAST_SBX_ERROR_ID+83UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Array needs dimensioning
#define ERRCODE_BASIC_NO_IF                 ((LAST_SBX_ERROR_ID+84UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Else/Endif without If
#define ERRCODE_BASIC_NOT_IN_SUBR           ((LAST_SBX_ERROR_ID+85UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // xxx not allowed within a sub
#define ERRCODE_BASIC_NOT_IN_MAIN           ((LAST_SBX_ERROR_ID+86UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // xxx not allowed outside a sub
#define ERRCODE_BASIC_WRONG_DIMS            ((LAST_SBX_ERROR_ID+87UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Dimensions do not match
#define ERRCODE_BASIC_BAD_OPTION            ((LAST_SBX_ERROR_ID+88UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Unknown option: xxx
#define ERRCODE_BASIC_CONSTANT_REDECLARED   ((LAST_SBX_ERROR_ID+89UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Constant xx redeclared
#define ERRCODE_BASIC_PROG_TOO_LARGE        ((LAST_SBX_ERROR_ID+90UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Program is too large
#define ERRCODE_BASIC_NO_STRINGS_ARRAYS     ((LAST_SBX_ERROR_ID+91UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)

#define ERRCODE_BASIC_EXCEPTION             ((LAST_SBX_ERROR_ID+92UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)

#define ERRCODE_BASMGR_STDLIBOPEN           (LAST_SBX_ERROR_ID+93UL) | ERRCODE_AREA_SBX
#define ERRCODE_BASMGR_STDLIBSAVE           (LAST_SBX_ERROR_ID+94UL) | ERRCODE_AREA_SBX
#define ERRCODE_BASMGR_LIBLOAD              (LAST_SBX_ERROR_ID+95UL) | ERRCODE_AREA_SBX
#define ERRCODE_BASMGR_LIBCREATE            (LAST_SBX_ERROR_ID+96UL) | ERRCODE_AREA_SBX
#define ERRCODE_BASMGR_LIBSAVE              (LAST_SBX_ERROR_ID+97UL) | ERRCODE_AREA_SBX
#define ERRCODE_BASMGR_LIBDEL               (LAST_SBX_ERROR_ID+98UL) | ERRCODE_AREA_SBX
#define ERRCODE_BASMGR_MGROPEN              (LAST_SBX_ERROR_ID+99UL) | ERRCODE_AREA_SBX
#define ERRCODE_BASMGR_MGRSAVE              (LAST_SBX_ERROR_ID+100UL) | ERRCODE_AREA_SBX
#define ERRCODE_BASMGR_REMOVELIB            (LAST_SBX_ERROR_ID+101UL) | ERRCODE_AREA_SBX
#define ERRCODE_BASMGR_UNLOADLIB            (LAST_SBX_ERROR_ID+102UL) | ERRCODE_AREA_SBX

#define ERRCODE_BASIC_ARRAY_FIX             ((LAST_SBX_ERROR_ID+104UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // This array is fixed
#define ERRCODE_BASIC_STRING_OVERFLOW       ((LAST_SBX_ERROR_ID+105UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Out of string space
#define ERRCODE_BASIC_EXPR_TOO_COMPLEX      ((LAST_SBX_ERROR_ID+106UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Expression too complex
#define ERRCODE_BASIC_OPER_NOT_PERFORM      ((LAST_SBX_ERROR_ID+107UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Can't perform requested operation
#define ERRCODE_BASIC_TOO_MANY_DLL          ((LAST_SBX_ERROR_ID+108UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Too many dll application clients
#define ERRCODE_BASIC_LOOP_NOT_INIT         ((LAST_SBX_ERROR_ID+109UL) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // For loop not initialized

#define ERRCODE_BASIC_COMPAT                ((LAST_SBX_ERROR_ID+103UL)| ERRCODE_AREA_SBX | ERRCODE_CLASS_RUNTIME)

// Grid messages from 30000-30999
// OLE messages from 31000-31999

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
