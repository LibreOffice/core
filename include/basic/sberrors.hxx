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

#pragma once

#include <comphelper/errcode.hxx>

#define ERRCODE_BASIC_SYNTAX                ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, 1)   // unspecified syntax error
#define ERRCODE_BASIC_BAD_ARGUMENT          ErrCode( ErrCodeArea::Sbx, ErrCodeClass::NotSupported, 2) // Invalid procedure call
#define ERRCODE_BASIC_MATH_OVERFLOW         ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Sbx, 3)        // overflow
#define ERRCODE_BASIC_OUT_OF_RANGE          ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Sbx, 4)        // Invalid array index / Subscript out of range
#define ERRCODE_BASIC_ZERODIV               ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Sbx, 5)        // Division by zero
#define ERRCODE_BASIC_CONVERSION            ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Sbx, 6)        // wrong data type
#define ERRCODE_BASIC_BAD_PARAMETER         ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 7)    // invalid Parameter
#define ERRCODE_BASIC_PROC_UNDEFINED        ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 8)    // Sub or Function not defined
#define ERRCODE_BASIC_INTERNAL_ERROR        ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Unknown, 9)    // generic object error
#define ERRCODE_BASIC_NO_OBJECT             ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 10)   // Object variable not set
#define ERRCODE_BASIC_CANNOT_LOAD           ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Create, 11)    // Can't load module
#define ERRCODE_BASIC_BAD_INDEX             ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Sbx, 12)       // Invalid object index
#define ERRCODE_BASIC_NO_ACTIVE_OBJECT      ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Access, 13)    // No active view or document
#define ERRCODE_BASIC_BAD_PROP_VALUE        ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 14)   // Bad property value
#define ERRCODE_BASIC_PROP_READONLY         ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Read, 15)      // Property is read only
#define ERRCODE_BASIC_PROP_WRITEONLY        ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Write, 16)   // Property is write only
#define ERRCODE_BASIC_INVALID_OBJECT        ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Access, 17)    // Invalid object reference
#define ERRCODE_BASIC_NO_METHOD             ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 18)   // Property or Method unknown
#define ERRCODE_BASIC_INVALID_USAGE_OBJECT  ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Access, 19)    // Invalid object usage
#define ERRCODE_BASIC_NO_OLE                ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Access, 20)    // Class does not support OLE
#define ERRCODE_BASIC_BAD_METHOD            ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 21)   // Method not supported
#define ERRCODE_BASIC_OLE_ERROR             ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 22)   // OLE Automation Error
#define ERRCODE_BASIC_BAD_ACTION            ErrCode( ErrCodeArea::Sbx, ErrCodeClass::NotSupported, 23)  // Object doesn't support this action
#define ERRCODE_BASIC_NO_NAMED_ARGS         ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 24)   // Object doesn't support named args
#define ERRCODE_BASIC_BAD_LOCALE            ErrCode( ErrCodeArea::Sbx, ErrCodeClass::NotSupported, 25)  // Object doesn't support current locale setting
#define ERRCODE_BASIC_NAMED_NOT_FOUND       ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 26)   // Named argument not found
#define ERRCODE_BASIC_NOT_OPTIONAL          ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 27)   // Argument not optional
#define ERRCODE_BASIC_WRONG_ARGS            ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Sbx, 28)       // Invalid number of arguments
#define ERRCODE_BASIC_NOT_A_COLL            ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 29)   // Object not a collection

#define ERRCODE_BASIC_NO_GOSUB              ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 30 )   // Return without Gosub
#define ERRCODE_BASIC_REDO_FROM_START       ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 31 )   // Redo form start (SB internal)
#define ERRCODE_BASIC_NO_MEMORY             ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 32 )   // Out of memory
#define ERRCODE_BASIC_ALREADY_DIM           ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 33 )   // Array already dimensioned
#define ERRCODE_BASIC_DUPLICATE_DEF         ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 34 )   // Duplicate definition
#define ERRCODE_BASIC_VAR_UNDEFINED         ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 35 )   // Variable undefined (SB)
#define ERRCODE_BASIC_USER_ABORT            ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 36 )   // User interrupt occurred
#define ERRCODE_BASIC_BAD_RESUME            ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 37 )   // Resume without error
#define ERRCODE_BASIC_STACK_OVERFLOW        ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 38 )   // Out of stack space
#define ERRCODE_BASIC_BAD_DLL_LOAD          ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 39 )   // Error in loading DLL
#define ERRCODE_BASIC_BAD_DLL_CALL          ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 40 )   // Bad DLL calling convention
#define ERRCODE_BASIC_BAD_CHANNEL           ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 41 )   // Bad file name or number
#define ERRCODE_BASIC_FILE_NOT_FOUND        ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 42 )   // File not found
#define ERRCODE_BASIC_BAD_FILE_MODE         ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 43 )   // Bad file mode
#define ERRCODE_BASIC_FILE_ALREADY_OPEN     ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 44 )   // File already open
#define ERRCODE_BASIC_IO_ERROR              ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 45 )   // Device I/O error
#define ERRCODE_BASIC_FILE_EXISTS           ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 46 )   // File already exists
#define ERRCODE_BASIC_BAD_RECORD_LENGTH     ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 47 )   // bad record length
#define ERRCODE_BASIC_DISK_FULL             ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 48 )   // disk full
#define ERRCODE_BASIC_READ_PAST_EOF         ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 49 )   // Input past end of file
#define ERRCODE_BASIC_BAD_RECORD_NUMBER     ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 50 )   // Bad record number
#define ERRCODE_BASIC_TOO_MANY_FILES        ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 51 )   // Too many files
#define ERRCODE_BASIC_NO_DEVICE             ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 52 )   // Device not available
#define ERRCODE_BASIC_ACCESS_DENIED         ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 53 )   // Permission denied
#define ERRCODE_BASIC_NOT_READY             ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 54 )   // Disk not ready
#define ERRCODE_BASIC_NOT_IMPLEMENTED       ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 55 )   // Feature not implemented
#define ERRCODE_BASIC_DIFFERENT_DRIVE       ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 56 )   // No rename with different drive
#define ERRCODE_BASIC_ACCESS_ERROR          ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 57 )   // Path/File access error
#define ERRCODE_BASIC_PATH_NOT_FOUND        ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 58 )   // Path not found
#define ERRCODE_BASIC_BAD_PATTERN           ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 59 )   // Invalid pattern string
#define ERRCODE_BASIC_IS_NULL               ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 60 )   // Invalid use of Null

#define ERRCODE_BASIC_DDE_ERROR             ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 61 )
#define ERRCODE_BASIC_DDE_WAITINGACK        ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 62 )
#define ERRCODE_BASIC_DDE_OUTOFCHANNELS     ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 63 )
#define ERRCODE_BASIC_DDE_NO_RESPONSE       ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 64 )
#define ERRCODE_BASIC_DDE_MULT_RESPONSES    ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 65 )
#define ERRCODE_BASIC_DDE_CHANNEL_LOCKED    ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 66 )
#define ERRCODE_BASIC_DDE_NOTPROCESSED      ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 67 )
#define ERRCODE_BASIC_DDE_TIMEOUT           ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 68 )
#define ERRCODE_BASIC_DDE_USER_INTERRUPT    ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 69 )
#define ERRCODE_BASIC_DDE_BUSY              ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 70 )
#define ERRCODE_BASIC_DDE_NO_DATA           ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 71 )
#define ERRCODE_BASIC_DDE_WRONG_DATA_FORMAT ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 72 )
#define ERRCODE_BASIC_DDE_PARTNER_QUIT      ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 73 )
#define ERRCODE_BASIC_DDE_CONV_CLOSED       ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 74 )
#define ERRCODE_BASIC_DDE_NO_CHANNEL        ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 75 )
#define ERRCODE_BASIC_DDE_INVALID_LINK      ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 76 )
#define ERRCODE_BASIC_DDE_QUEUE_OVERFLOW    ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 77 )
#define ERRCODE_BASIC_DDE_LINK_ALREADY_EST  ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 78 )
#define ERRCODE_BASIC_DDE_LINK_INV_TOPIC    ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 79 )
#define ERRCODE_BASIC_DDE_DLL_NOT_FOUND     ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 80 )

#define ERRCODE_BASIC_NEEDS_OBJECT          ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 81 )   // Object required
#define ERRCODE_BASIC_BAD_ORDINAL           ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 82 )   // Invalid ordinal
#define ERRCODE_BASIC_DLLPROC_NOT_FOUND     ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 83 )   // Specified DLL function not found
#define ERRCODE_BASIC_BAD_CLIPBD_FORMAT     ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 84 )   // Invalid clipboard format

#define ERRCODE_BASIC_PROPERTY_NOT_FOUND    ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 85 )   // Class not have property
#define ERRCODE_BASIC_METHOD_NOT_FOUND      ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 86 )   // Class does not have method
#define ERRCODE_BASIC_ARG_MISSING           ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 87 )   // Missing required argument
#define ERRCODE_BASIC_BAD_NUMBER_OF_ARGS    ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 88 )   // Bad number of arguments
#define ERRCODE_BASIC_METHOD_FAILED         ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 89 )   // Method failed
#define ERRCODE_BASIC_SETPROP_FAILED        ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 90 )   // Unable to set property
#define ERRCODE_BASIC_GETPROP_FAILED        ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 91 )   // Unable to get property

// Compiler Errors (do not occur at runtime)
// These IDs can shift at any time

#define ERRCODE_BASIC_UNEXPECTED            ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler,  92 )  // Unexpected symbol: xx
#define ERRCODE_BASIC_EXPECTED              ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler,  93 )  // Expected: xx
#define ERRCODE_BASIC_SYMBOL_EXPECTED       ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler,  94 )  // Symbol expected
#define ERRCODE_BASIC_VAR_EXPECTED          ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler,  95 )  // Variable expected
#define ERRCODE_BASIC_LABEL_EXPECTED        ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler,  96 )  // Label expected
#define ERRCODE_BASIC_LVALUE_EXPECTED       ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler,  97 )  // Lvalue expected
#define ERRCODE_BASIC_VAR_DEFINED           ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler,  98 )  // Variable xxx already defined
#define ERRCODE_BASIC_PROC_DEFINED          ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler,  99 )  // Procedure xx already defined
#define ERRCODE_BASIC_LABEL_DEFINED         ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, 100 )  // Label xxx already defined
#define ERRCODE_BASIC_UNDEF_VAR             ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, 101 )  // Variable xx undefined
#define ERRCODE_BASIC_UNDEF_ARRAY           ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, 102 )  // Array or function xx undefined
#define ERRCODE_BASIC_UNDEF_PROC            ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, 103 )  // Procedure xxx undefined
#define ERRCODE_BASIC_UNDEF_LABEL           ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, 104 )  // Label xxx undefined
#define ERRCODE_BASIC_UNDEF_TYPE            ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, 105 )  // Unknown user defined type xxx
#define ERRCODE_BASIC_BAD_EXIT              ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, 106 )  // Exit XXX expected
#define ERRCODE_BASIC_BAD_BLOCK             ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, 107 )  // Unterminated statement block: missing XX
#define ERRCODE_BASIC_BAD_BRACKETS          ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, 108 )  // Parentheses do not match
#define ERRCODE_BASIC_BAD_DECLARATION       ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, 109 )  // Symbol xx defined differently
#define ERRCODE_BASIC_BAD_PARAMETERS        ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, 110 )  // Parameters do not match
#define ERRCODE_BASIC_BAD_CHAR_IN_NUMBER    ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, 111 )  // Bad character in number
#define ERRCODE_BASIC_MUST_HAVE_DIMS        ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, 112 )  // Array needs dimensioning
#define ERRCODE_BASIC_NO_IF                 ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, 113 )  // Else/Endif without If
#define ERRCODE_BASIC_NOT_IN_SUBR           ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, 114 )  // xxx not allowed within a sub
#define ERRCODE_BASIC_NOT_IN_MAIN           ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, 115 )  // xxx not allowed outside a sub
#define ERRCODE_BASIC_WRONG_DIMS            ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, 116 )  // Dimensions do not match
#define ERRCODE_BASIC_BAD_OPTION            ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, 117 )  // Unknown option: xxx
#define ERRCODE_BASIC_CONSTANT_REDECLARED   ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, 118 )  // Constant xx redeclared
#define ERRCODE_BASIC_PROG_TOO_LARGE        ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, 119 )  // Program is too large
#define ERRCODE_BASIC_NO_STRINGS_ARRAYS     ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, 120 )
#define ERRCODE_BASIC_EXCEPTION             ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime,  121 )

#define ERRCODE_BASMGR_STDLIBOPEN           ErrCode( ErrCodeArea::Sbx, 122)
#define ERRCODE_BASMGR_STDLIBSAVE           ErrCode( ErrCodeArea::Sbx, 123)
#define ERRCODE_BASMGR_LIBLOAD              ErrCode( ErrCodeArea::Sbx, 124)
#define ERRCODE_BASMGR_LIBCREATE            ErrCode( ErrCodeArea::Sbx, 125)
#define ERRCODE_BASMGR_LIBSAVE              ErrCode( ErrCodeArea::Sbx, 126)
#define ERRCODE_BASMGR_LIBDEL               ErrCode( ErrCodeArea::Sbx, 127)
#define ERRCODE_BASMGR_MGROPEN              ErrCode( ErrCodeArea::Sbx, 128)
#define ERRCODE_BASMGR_MGRSAVE              ErrCode( ErrCodeArea::Sbx, 129)
#define ERRCODE_BASMGR_REMOVELIB            ErrCode( ErrCodeArea::Sbx, 130)
#define ERRCODE_BASMGR_UNLOADLIB            ErrCode( ErrCodeArea::Sbx, 131)

#define ERRCODE_BASIC_COMPAT                ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, 132 )
#define ERRCODE_BASIC_ARRAY_FIX             ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, 133 )  // This array is fixed
#define ERRCODE_BASIC_STRING_OVERFLOW       ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, 134 )  // Out of string space
#define ERRCODE_BASIC_EXPR_TOO_COMPLEX      ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, 135 )  // Expression too complex
#define ERRCODE_BASIC_OPER_NOT_PERFORM      ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, 136 )  // Can't perform requested operation
#define ERRCODE_BASIC_TOO_MANY_DLL          ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, 137 )  // Too many dll application clients
#define ERRCODE_BASIC_LOOP_NOT_INIT         ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, 138 )  // For loop not initialized

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
