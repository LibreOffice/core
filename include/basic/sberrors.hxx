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

// Append Basic specific error messages to ErrCodeArea::Sbx
#define ERRCODE_BASIC_NO_GOSUB              ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+1 \
                                            )              // Return without Gosub
#define ERRCODE_BASIC_REDO_FROM_START       ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+2 \
                                            )              // Redo form start (SB internal)
#define ERRCODE_BASIC_NO_MEMORY             ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+3 \
                                            )              // Out of memory
#define ERRCODE_BASIC_ALREADY_DIM           ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+4 \
                                            )              // Array already dimensioned
#define ERRCODE_BASIC_DUPLICATE_DEF         ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+5 \
                                            )              // Duplicate definition
#define ERRCODE_BASIC_VAR_UNDEFINED         ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+6 \
                                            )          // Variable undefined (SB)
#define ERRCODE_BASIC_USER_ABORT            ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+7 \
                                            )              // User interrupt occurred
#define ERRCODE_BASIC_BAD_RESUME            ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+8 \
                                            )              // Resume without error
#define ERRCODE_BASIC_STACK_OVERFLOW        ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+9 \
                                            )              // Out of stack space
#define ERRCODE_BASIC_BAD_DLL_LOAD          ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+10 \
                                            )              // Error in loading DLL
#define ERRCODE_BASIC_BAD_DLL_CALL          ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+11 \
                                            )              // Bad DLL calling convention
#define ERRCODE_BASIC_BAD_CHANNEL           ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+12 \
                                            )              // Bad file name or number
#define ERRCODE_BASIC_FILE_NOT_FOUND        ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+13 \
                                            )              // File not found
#define ERRCODE_BASIC_BAD_FILE_MODE         ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+14 \
                                            )              // Bad file mode
#define ERRCODE_BASIC_FILE_ALREADY_OPEN     ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+15 \
                                            )              // File already open
#define ERRCODE_BASIC_IO_ERROR              ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+16 \
                                            )          // Device I/O error
#define ERRCODE_BASIC_FILE_EXISTS           ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+17 \
                                            )              // File already exists
#define ERRCODE_BASIC_BAD_RECORD_LENGTH     ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+18 \
                                            )              // bad record length
#define ERRCODE_BASIC_DISK_FULL             ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+19 \
                                            )              // disk full
#define ERRCODE_BASIC_READ_PAST_EOF         ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+20 \
                                            )              // Input past end of file
#define ERRCODE_BASIC_BAD_RECORD_NUMBER     ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+21 \
                                            )              // Bad record number
#define ERRCODE_BASIC_TOO_MANY_FILES        ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+22 \
                                            )          // Too many files
#define ERRCODE_BASIC_NO_DEVICE             ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+23 \
                                            )              // Device not available
#define ERRCODE_BASIC_ACCESS_DENIED         ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+24 \
                                            )              // Permission denied
#define ERRCODE_BASIC_NOT_READY             ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+25 \
                                            )              // Disk not ready
#define ERRCODE_BASIC_NOT_IMPLEMENTED       ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+26 \
                                            )              // Feature not implemented
#define ERRCODE_BASIC_DIFFERENT_DRIVE       ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+27 \
                                            )              // No rename with different drive
#define ERRCODE_BASIC_ACCESS_ERROR          ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+28 \
                                            )              // Path/File access error
#define ERRCODE_BASIC_PATH_NOT_FOUND        ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+29 \
                                            )              // Path not found
#define ERRCODE_BASIC_BAD_PATTERN           ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+30 \
                                            )              // Invalid pattern string
#define ERRCODE_BASIC_IS_NULL               ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+31 \
                                            )              // Invalid use of Null

// DDE messages from 250-299
#define ERRCODE_BASIC_DDE_ERROR             ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+32 \
                                            )
#define ERRCODE_BASIC_DDE_WAITINGACK        ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+33 \
                                            )
#define ERRCODE_BASIC_DDE_OUTOFCHANNELS     ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+34 \
                                            )
#define ERRCODE_BASIC_DDE_NO_RESPONSE       ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+35 \
                                            )
#define ERRCODE_BASIC_DDE_MULT_RESPONSES    ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+36 \
                                            )
#define ERRCODE_BASIC_DDE_CHANNEL_LOCKED    ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+37 \
                                            )
#define ERRCODE_BASIC_DDE_NOTPROCESSED      ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+38 \
                                            )
#define ERRCODE_BASIC_DDE_TIMEOUT           ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+39 \
                                            )
#define ERRCODE_BASIC_DDE_USER_INTERRUPT    ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+40 \
                                            )
#define ERRCODE_BASIC_DDE_BUSY              ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+41 \
                                            )
#define ERRCODE_BASIC_DDE_NO_DATA           ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+42 \
                                            )
#define ERRCODE_BASIC_DDE_WRONG_DATA_FORMAT ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+43 \
                                            )
#define ERRCODE_BASIC_DDE_PARTNER_QUIT      ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+44 \
                                            )
#define ERRCODE_BASIC_DDE_CONV_CLOSED       ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+45 \
                                            )
#define ERRCODE_BASIC_DDE_NO_CHANNEL        ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+46 \
                                            )
#define ERRCODE_BASIC_DDE_INVALID_LINK      ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+47 \
                                            )
#define ERRCODE_BASIC_DDE_QUEUE_OVERFLOW    ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+48 \
                                            )
#define ERRCODE_BASIC_DDE_LINK_ALREADY_EST  ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+49 \
                                            )
#define ERRCODE_BASIC_DDE_LINK_INV_TOPIC    ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+50 \
                                            )
#define ERRCODE_BASIC_DDE_DLL_NOT_FOUND     ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+51 \
                                            )

#define ERRCODE_BASIC_NEEDS_OBJECT          ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+52 \
                                            )              // Object required
#define ERRCODE_BASIC_BAD_ORDINAL           ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+53 \
                                            )              // Invalid ordinal
#define ERRCODE_BASIC_DLLPROC_NOT_FOUND     ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+54 \
                                            )              // Specified DLL function not found
#define ERRCODE_BASIC_BAD_CLIPBD_FORMAT     ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+55 \
                                            )              // Invalid clipboard format

// Debugger messages from 700-799

#define ERRCODE_BASIC_PROPERTY_NOT_FOUND    ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+56 \
                                            )              // Class not have property
#define ERRCODE_BASIC_METHOD_NOT_FOUND      ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+57 \
                                            )              // Class does not have method
#define ERRCODE_BASIC_ARG_MISSING           ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+58 \
                                            )              // Missing required argument
#define ERRCODE_BASIC_BAD_NUMBER_OF_ARGS    ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+59 \
                                            )              // Bad number of arguments
#define ERRCODE_BASIC_METHOD_FAILED         ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+60 \
                                            )              // Method failed
#define ERRCODE_BASIC_SETPROP_FAILED        ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+61 \
                                            )              // Unable to set property
#define ERRCODE_BASIC_GETPROP_FAILED        ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+62 \
                                            )              // Unable to get property

// Compiler Errors (do not occur at runtime)
// These IDs can shift at any time

#define ERRCODE_BASIC_UNEXPECTED            ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+63 \
                                            )             // Unexpected symbol: xx
#define ERRCODE_BASIC_EXPECTED              ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+64 \
                                            )             // Expected: xx
#define ERRCODE_BASIC_SYMBOL_EXPECTED       ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+65 \
                                            )             // Symbol expected
#define ERRCODE_BASIC_VAR_EXPECTED          ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+66 \
                                            )             // Variable expected
#define ERRCODE_BASIC_LABEL_EXPECTED        ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+67 \
                                            )             // Label expected
#define ERRCODE_BASIC_LVALUE_EXPECTED       ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+68 \
                                            )             // Lvalue expected
#define ERRCODE_BASIC_VAR_DEFINED           ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+69 \
                                            )             // Variable xxx already defined
#define ERRCODE_BASIC_PROC_DEFINED          ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+70 \
                                            )             // Procedure xx already defined
#define ERRCODE_BASIC_LABEL_DEFINED         ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+71 \
                                            )             // Label xxx already defined
#define ERRCODE_BASIC_UNDEF_VAR             ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+72 \
                                            )             // Variable xx undefined
#define ERRCODE_BASIC_UNDEF_ARRAY           ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+73 \
                                            )             // Array or function xx undefined
#define ERRCODE_BASIC_UNDEF_PROC            ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+74 \
                                            )             // Procedure xxx undefined
#define ERRCODE_BASIC_UNDEF_LABEL           ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+75 \
                                            )             // Label xxx undefined
#define ERRCODE_BASIC_UNDEF_TYPE            ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+76 \
                                            )             // Unknown user defined type xxx
#define ERRCODE_BASIC_BAD_EXIT              ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+77 \
                                            )             // Exit XXX expected
#define ERRCODE_BASIC_BAD_BLOCK             ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+78 \
                                            )             // Unterminated statement block: missing XX
#define ERRCODE_BASIC_BAD_BRACKETS          ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+79 \
                                            )             // Parentheses do not match
#define ERRCODE_BASIC_BAD_DECLARATION       ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+80 \
                                            )             // Symbol xx defined differently
#define ERRCODE_BASIC_BAD_PARAMETERS        ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+81 \
                                            )             // Parameters do not match
#define ERRCODE_BASIC_BAD_CHAR_IN_NUMBER    ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+82 \
                                            )             // Bad character in number
#define ERRCODE_BASIC_MUST_HAVE_DIMS        ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+83 \
                                            )             // Array needs dimensioning
#define ERRCODE_BASIC_NO_IF                 ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+84 \
                                            )             // Else/Endif without If
#define ERRCODE_BASIC_NOT_IN_SUBR           ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+85 \
                                            )             // xxx not allowed within a sub
#define ERRCODE_BASIC_NOT_IN_MAIN           ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+86 \
                                            )             // xxx not allowed outside a sub
#define ERRCODE_BASIC_WRONG_DIMS            ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+87 \
                                            )             // Dimensions do not match
#define ERRCODE_BASIC_BAD_OPTION            ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+88 \
                                            )             // Unknown option: xxx
#define ERRCODE_BASIC_CONSTANT_REDECLARED   ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+89 \
                                            )             // Constant xx redeclared
#define ERRCODE_BASIC_PROG_TOO_LARGE        ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+90 \
                                            )             // Program is too large
#define ERRCODE_BASIC_NO_STRINGS_ARRAYS     ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+91 \
                                            )

#define ERRCODE_BASIC_EXCEPTION             ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+92 )

#define ERRCODE_BASMGR_STDLIBOPEN           ErrCode( ErrCodeArea::Sbx, LAST_SBX_ERROR_ID+93)
#define ERRCODE_BASMGR_STDLIBSAVE           ErrCode( ErrCodeArea::Sbx, LAST_SBX_ERROR_ID+94)
#define ERRCODE_BASMGR_LIBLOAD              ErrCode( ErrCodeArea::Sbx, LAST_SBX_ERROR_ID+95)
#define ERRCODE_BASMGR_LIBCREATE            ErrCode( ErrCodeArea::Sbx, LAST_SBX_ERROR_ID+96)
#define ERRCODE_BASMGR_LIBSAVE              ErrCode( ErrCodeArea::Sbx, LAST_SBX_ERROR_ID+97)
#define ERRCODE_BASMGR_LIBDEL               ErrCode( ErrCodeArea::Sbx, LAST_SBX_ERROR_ID+98)
#define ERRCODE_BASMGR_MGROPEN              ErrCode( ErrCodeArea::Sbx, LAST_SBX_ERROR_ID+99)
#define ERRCODE_BASMGR_MGRSAVE              ErrCode( ErrCodeArea::Sbx, LAST_SBX_ERROR_ID+100)
#define ERRCODE_BASMGR_REMOVELIB            ErrCode( ErrCodeArea::Sbx, LAST_SBX_ERROR_ID+101)
#define ERRCODE_BASMGR_UNLOADLIB            ErrCode( ErrCodeArea::Sbx, LAST_SBX_ERROR_ID+102)

#define ERRCODE_BASIC_ARRAY_FIX             ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+104 \
                                            )             // This array is fixed
#define ERRCODE_BASIC_STRING_OVERFLOW       ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+105 \
                                            )             // Out of string space
#define ERRCODE_BASIC_EXPR_TOO_COMPLEX      ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+106 \
                                            )             // Expression too complex
#define ERRCODE_BASIC_OPER_NOT_PERFORM      ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+107 \
                                            )             // Can't perform requested operation
#define ERRCODE_BASIC_TOO_MANY_DLL          ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+108 \
                                            )             // Too many dll application clients
#define ERRCODE_BASIC_LOOP_NOT_INIT         ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Compiler, LAST_SBX_ERROR_ID+109 \
                                            )             // For loop not initialized

#define ERRCODE_BASIC_COMPAT                ErrCode( ErrCodeArea::Sbx, ErrCodeClass::Runtime, LAST_SBX_ERROR_ID+103 )

// Grid messages from 30000-30999
// OLE messages from 31000-31999

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
