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

// Mapping to ErrCode
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
#define ERRCODE_BASIC_NO_GOSUB              ErrCode((LAST_SBX_ERROR_ID+1) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Return without Gosub
#define ERRCODE_BASIC_REDO_FROM_START       ErrCode((LAST_SBX_ERROR_ID+2) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Redo form start (SB internal)
#define ERRCODE_BASIC_NO_MEMORY             ErrCode((LAST_SBX_ERROR_ID+3) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Out of memory
#define ERRCODE_BASIC_ALREADY_DIM           ErrCode((LAST_SBX_ERROR_ID+4) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Array already dimensioned
#define ERRCODE_BASIC_DUPLICATE_DEF         ErrCode((LAST_SBX_ERROR_ID+5) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Duplicate definition
#define ERRCODE_BASIC_VAR_UNDEFINED         ErrCode((LAST_SBX_ERROR_ID+6) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)          // Variable undefined (SB)
#define ERRCODE_BASIC_USER_ABORT            ErrCode((LAST_SBX_ERROR_ID+7) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // User interrupt occurred
#define ERRCODE_BASIC_BAD_RESUME            ErrCode((LAST_SBX_ERROR_ID+8) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Resume without error
#define ERRCODE_BASIC_STACK_OVERFLOW        ErrCode((LAST_SBX_ERROR_ID+9) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Out of stack space
#define ERRCODE_BASIC_BAD_DLL_LOAD          ErrCode((LAST_SBX_ERROR_ID+10) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Error in loading DLL
#define ERRCODE_BASIC_BAD_DLL_CALL          ErrCode((LAST_SBX_ERROR_ID+11) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Bad DLL calling convention
#define ERRCODE_BASIC_BAD_CHANNEL           ErrCode((LAST_SBX_ERROR_ID+12) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Bad file name or number
#define ERRCODE_BASIC_FILE_NOT_FOUND        ErrCode((LAST_SBX_ERROR_ID+13) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // File not found
#define ERRCODE_BASIC_BAD_FILE_MODE         ErrCode((LAST_SBX_ERROR_ID+14) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Bad file mode
#define ERRCODE_BASIC_FILE_ALREADY_OPEN     ErrCode((LAST_SBX_ERROR_ID+15) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // File already open
#define ERRCODE_BASIC_IO_ERROR              ErrCode((LAST_SBX_ERROR_ID+16) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)          // Device I/O error
#define ERRCODE_BASIC_FILE_EXISTS           ErrCode((LAST_SBX_ERROR_ID+17) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // File already exists
#define ERRCODE_BASIC_BAD_RECORD_LENGTH     ErrCode((LAST_SBX_ERROR_ID+18) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // bad record length
#define ERRCODE_BASIC_DISK_FULL             ErrCode((LAST_SBX_ERROR_ID+19) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // disk full
#define ERRCODE_BASIC_READ_PAST_EOF         ErrCode((LAST_SBX_ERROR_ID+20) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Input past end of file
#define ERRCODE_BASIC_BAD_RECORD_NUMBER     ErrCode((LAST_SBX_ERROR_ID+21) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Bad record number
#define ERRCODE_BASIC_TOO_MANY_FILES        ErrCode((LAST_SBX_ERROR_ID+22) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)          // Too many files
#define ERRCODE_BASIC_NO_DEVICE             ErrCode((LAST_SBX_ERROR_ID+23) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Device not available
#define ERRCODE_BASIC_ACCESS_DENIED         ErrCode((LAST_SBX_ERROR_ID+24) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Permission denied
#define ERRCODE_BASIC_NOT_READY             ErrCode((LAST_SBX_ERROR_ID+25) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Disk not ready
#define ERRCODE_BASIC_NOT_IMPLEMENTED       ErrCode((LAST_SBX_ERROR_ID+26) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Feature not implemented
#define ERRCODE_BASIC_DIFFERENT_DRIVE       ErrCode((LAST_SBX_ERROR_ID+27) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // No rename with different drive
#define ERRCODE_BASIC_ACCESS_ERROR          ErrCode((LAST_SBX_ERROR_ID+28) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Path/File access error
#define ERRCODE_BASIC_PATH_NOT_FOUND        ErrCode((LAST_SBX_ERROR_ID+29) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Path not found
#define ERRCODE_BASIC_BAD_PATTERN           ErrCode((LAST_SBX_ERROR_ID+30) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Invalid pattern string
#define ERRCODE_BASIC_IS_NULL               ErrCode((LAST_SBX_ERROR_ID+31) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Invalid use of Null

// DDE messages from 250-299
#define ERRCODE_BASIC_DDE_ERROR             ErrCode((LAST_SBX_ERROR_ID+32) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_WAITINGACK        ErrCode((LAST_SBX_ERROR_ID+33) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_OUTOFCHANNELS     ErrCode((LAST_SBX_ERROR_ID+34) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_NO_RESPONSE       ErrCode((LAST_SBX_ERROR_ID+35) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_MULT_RESPONSES    ErrCode((LAST_SBX_ERROR_ID+36) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_CHANNEL_LOCKED    ErrCode((LAST_SBX_ERROR_ID+37) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_NOTPROCESSED      ErrCode((LAST_SBX_ERROR_ID+38) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_TIMEOUT           ErrCode((LAST_SBX_ERROR_ID+39) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_USER_INTERRUPT    ErrCode((LAST_SBX_ERROR_ID+40) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_BUSY              ErrCode((LAST_SBX_ERROR_ID+41) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_NO_DATA           ErrCode((LAST_SBX_ERROR_ID+42) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_WRONG_DATA_FORMAT ErrCode((LAST_SBX_ERROR_ID+43) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_PARTNER_QUIT      ErrCode((LAST_SBX_ERROR_ID+44) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_CONV_CLOSED       ErrCode((LAST_SBX_ERROR_ID+45) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_NO_CHANNEL        ErrCode((LAST_SBX_ERROR_ID+46) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_INVALID_LINK      ErrCode((LAST_SBX_ERROR_ID+47) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_QUEUE_OVERFLOW    ErrCode((LAST_SBX_ERROR_ID+48) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_LINK_ALREADY_EST  ErrCode((LAST_SBX_ERROR_ID+49) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_LINK_INV_TOPIC    ErrCode((LAST_SBX_ERROR_ID+50) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)
#define ERRCODE_BASIC_DDE_DLL_NOT_FOUND     ErrCode((LAST_SBX_ERROR_ID+51) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)

#define ERRCODE_BASIC_NEEDS_OBJECT          ErrCode((LAST_SBX_ERROR_ID+52) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Object required
#define ERRCODE_BASIC_BAD_ORDINAL           ErrCode((LAST_SBX_ERROR_ID+53) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Invalid ordinal
#define ERRCODE_BASIC_DLLPROC_NOT_FOUND     ErrCode((LAST_SBX_ERROR_ID+54) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Specified DLL function not found
#define ERRCODE_BASIC_BAD_CLIPBD_FORMAT     ErrCode((LAST_SBX_ERROR_ID+55) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Invalid clipboard format

// Debugger messages from 700-799

#define ERRCODE_BASIC_PROPERTY_NOT_FOUND    ErrCode((LAST_SBX_ERROR_ID+56) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Class not have property
#define ERRCODE_BASIC_METHOD_NOT_FOUND      ErrCode((LAST_SBX_ERROR_ID+57) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Class does not have method
#define ERRCODE_BASIC_ARG_MISSING           ErrCode((LAST_SBX_ERROR_ID+58) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Missing required argument
#define ERRCODE_BASIC_BAD_NUMBER_OF_ARGS    ErrCode((LAST_SBX_ERROR_ID+59) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Bad number of arguments
#define ERRCODE_BASIC_METHOD_FAILED         ErrCode((LAST_SBX_ERROR_ID+60) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Method failed
#define ERRCODE_BASIC_SETPROP_FAILED        ErrCode((LAST_SBX_ERROR_ID+61) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Unable to set property
#define ERRCODE_BASIC_GETPROP_FAILED        ErrCode((LAST_SBX_ERROR_ID+62) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)              // Unable to get property

// Compiler Errors (do not occur at runtime)
// These IDs can shift at any time

#define ERRCODE_BASIC_UNEXPECTED            ErrCode((LAST_SBX_ERROR_ID+63) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Unexpected symbol: xx
#define ERRCODE_BASIC_EXPECTED              ErrCode((LAST_SBX_ERROR_ID+64) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Expected: xx
#define ERRCODE_BASIC_SYMBOL_EXPECTED       ErrCode((LAST_SBX_ERROR_ID+65) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Symbol expected
#define ERRCODE_BASIC_VAR_EXPECTED          ErrCode((LAST_SBX_ERROR_ID+66) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Variable expected
#define ERRCODE_BASIC_LABEL_EXPECTED        ErrCode((LAST_SBX_ERROR_ID+67) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Label expected
#define ERRCODE_BASIC_LVALUE_EXPECTED       ErrCode((LAST_SBX_ERROR_ID+68) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Lvalue expected
#define ERRCODE_BASIC_VAR_DEFINED           ErrCode((LAST_SBX_ERROR_ID+69) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Variable xxx already defined
#define ERRCODE_BASIC_PROC_DEFINED          ErrCode((LAST_SBX_ERROR_ID+70) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Procedure xx already defined
#define ERRCODE_BASIC_LABEL_DEFINED         ErrCode((LAST_SBX_ERROR_ID+71) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Label xxx already defined
#define ERRCODE_BASIC_UNDEF_VAR             ErrCode((LAST_SBX_ERROR_ID+72) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Variable xx undefined
#define ERRCODE_BASIC_UNDEF_ARRAY           ErrCode((LAST_SBX_ERROR_ID+73) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Array or function xx undefined
#define ERRCODE_BASIC_UNDEF_PROC            ErrCode((LAST_SBX_ERROR_ID+74) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Procedure xxx undefined
#define ERRCODE_BASIC_UNDEF_LABEL           ErrCode((LAST_SBX_ERROR_ID+75) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Label xxx undefined
#define ERRCODE_BASIC_UNDEF_TYPE            ErrCode((LAST_SBX_ERROR_ID+76) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Unknown user defined type xxx
#define ERRCODE_BASIC_BAD_EXIT              ErrCode((LAST_SBX_ERROR_ID+77) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Exit XXX expected
#define ERRCODE_BASIC_BAD_BLOCK             ErrCode((LAST_SBX_ERROR_ID+78) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Unterminated statement block: missing XX
#define ERRCODE_BASIC_BAD_BRACKETS          ErrCode((LAST_SBX_ERROR_ID+79) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Parentheses do not match
#define ERRCODE_BASIC_BAD_DECLARATION       ErrCode((LAST_SBX_ERROR_ID+80) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Symbol xx defined differently
#define ERRCODE_BASIC_BAD_PARAMETERS        ErrCode((LAST_SBX_ERROR_ID+81) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Parameters do not match
#define ERRCODE_BASIC_BAD_CHAR_IN_NUMBER    ErrCode((LAST_SBX_ERROR_ID+82) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Bad character in number
#define ERRCODE_BASIC_MUST_HAVE_DIMS        ErrCode((LAST_SBX_ERROR_ID+83) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Array needs dimensioning
#define ERRCODE_BASIC_NO_IF                 ErrCode((LAST_SBX_ERROR_ID+84) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Else/Endif without If
#define ERRCODE_BASIC_NOT_IN_SUBR           ErrCode((LAST_SBX_ERROR_ID+85) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // xxx not allowed within a sub
#define ERRCODE_BASIC_NOT_IN_MAIN           ErrCode((LAST_SBX_ERROR_ID+86) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // xxx not allowed outside a sub
#define ERRCODE_BASIC_WRONG_DIMS            ErrCode((LAST_SBX_ERROR_ID+87) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Dimensions do not match
#define ERRCODE_BASIC_BAD_OPTION            ErrCode((LAST_SBX_ERROR_ID+88) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Unknown option: xxx
#define ERRCODE_BASIC_CONSTANT_REDECLARED   ErrCode((LAST_SBX_ERROR_ID+89) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Constant xx redeclared
#define ERRCODE_BASIC_PROG_TOO_LARGE        ErrCode((LAST_SBX_ERROR_ID+90) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Program is too large
#define ERRCODE_BASIC_NO_STRINGS_ARRAYS     ErrCode((LAST_SBX_ERROR_ID+91) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)

#define ERRCODE_BASIC_EXCEPTION             ErrCode((LAST_SBX_ERROR_ID+92) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_RUNTIME)

#define ERRCODE_BASMGR_STDLIBOPEN           ErrCode((LAST_SBX_ERROR_ID+93) | ERRCODE_AREA_SBX)
#define ERRCODE_BASMGR_STDLIBSAVE           ErrCode((LAST_SBX_ERROR_ID+94) | ERRCODE_AREA_SBX)
#define ERRCODE_BASMGR_LIBLOAD              ErrCode((LAST_SBX_ERROR_ID+95) | ERRCODE_AREA_SBX)
#define ERRCODE_BASMGR_LIBCREATE            ErrCode((LAST_SBX_ERROR_ID+96) | ERRCODE_AREA_SBX)
#define ERRCODE_BASMGR_LIBSAVE              ErrCode((LAST_SBX_ERROR_ID+97) | ERRCODE_AREA_SBX)
#define ERRCODE_BASMGR_LIBDEL               ErrCode((LAST_SBX_ERROR_ID+98) | ERRCODE_AREA_SBX)
#define ERRCODE_BASMGR_MGROPEN              ErrCode((LAST_SBX_ERROR_ID+99) | ERRCODE_AREA_SBX)
#define ERRCODE_BASMGR_MGRSAVE              ErrCode((LAST_SBX_ERROR_ID+100) | ERRCODE_AREA_SBX)
#define ERRCODE_BASMGR_REMOVELIB            ErrCode((LAST_SBX_ERROR_ID+101) | ERRCODE_AREA_SBX)
#define ERRCODE_BASMGR_UNLOADLIB            ErrCode((LAST_SBX_ERROR_ID+102) | ERRCODE_AREA_SBX)

#define ERRCODE_BASIC_ARRAY_FIX             ErrCode((LAST_SBX_ERROR_ID+104) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // This array is fixed
#define ERRCODE_BASIC_STRING_OVERFLOW       ErrCode((LAST_SBX_ERROR_ID+105) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Out of string space
#define ERRCODE_BASIC_EXPR_TOO_COMPLEX      ErrCode((LAST_SBX_ERROR_ID+106) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Expression too complex
#define ERRCODE_BASIC_OPER_NOT_PERFORM      ErrCode((LAST_SBX_ERROR_ID+107) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Can't perform requested operation
#define ERRCODE_BASIC_TOO_MANY_DLL          ErrCode((LAST_SBX_ERROR_ID+108) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // Too many dll application clients
#define ERRCODE_BASIC_LOOP_NOT_INIT         ErrCode((LAST_SBX_ERROR_ID+109) | ERRCODE_AREA_SBX | \
                                            ERRCODE_CLASS_COMPILER)             // For loop not initialized

#define ERRCODE_BASIC_COMPAT                ErrCode((LAST_SBX_ERROR_ID+103)| ERRCODE_AREA_SBX | ERRCODE_CLASS_RUNTIME)

// Grid messages from 30000-30999
// OLE messages from 31000-31999

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
